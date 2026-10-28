#!/usr/bin/env python3
"""
passivedump.py:  Filters the output of system's DumpPassiveObjects down to the
  passive objects a room's kod doesn't make (by DMs, say).

  Usage:  passivedump.py <kod directory> <debug log or dump> [output]

  It reads every room class in the kod and works out, from the source, where
  its CreateStandardObjects places what: it follows propagate, the room's own
  messages it sends itself, CreateOrnObjFromList and CreateObjFromList, both
  sides of every if and loops over lists it can work out.  A dumped object of a
  class the kod places at exactly that spot is left out; so is anything the
  kod places where the source doesn't say exactly (a coordinate from Random,
  say), when its class and the coordinates the source does give match.  What
  is left is written out, between the dump's begin and end lines.
"""

import os
import re
import sys

FINENESS = 64


class Unknown:
    """A value the source doesn't say."""

    def __repr__(self):
        return '?'


UNKNOWN = Unknown()
SELF = ('self',)


def known(v):
    return not isinstance(v, Unknown)


# ---------------------------------------------------------------- kod source

def strip_comments(s):
    out = []
    in_str = False
    i = 0
    while i < len(s):
        c = s[i]
        if in_str:
            out.append(c)
            if c == '\\' and i + 1 < len(s):
                out.append(s[i + 1])
                i += 1
            elif c == '"':
                in_str = False
        elif c == '"':
            in_str = True
            out.append(c)
        elif c == '%':
            while i < len(s) and s[i] != '\n':
                i += 1
            continue
        else:
            out.append(c)
        i += 1
    return ''.join(out)


TOKEN = re.compile(r'''
    (?P<num>0x[0-9a-fA-F]+|\d+)
  | (?P<str>"(?:[^"\\]|\\.)*")
  | (?P<id>[A-Za-z_][A-Za-z0-9_]*)
  | (?P<op><>|<=|>=|[-+*/=<>(){}\[\],;#@&|$!.])
  | (?P<ws>\s+)
''', re.X)


def tokenize(s):
    toks = []
    for m in TOKEN.finditer(s):
        kind = m.lastgroup
        if kind == 'ws':
            continue
        text = m.group()
        if kind == 'num':
            toks.append(('num', int(text, 16) if text[:2].lower() == '0x' else int(text)))
        elif kind == 'id':
            toks.append(('id', text.lower()))
        else:
            toks.append((kind, text))
    return toks


class KodClass:
    def __init__(self, name, parent):
        self.directory = None    # where its .kod is, for include
        self.name = name
        self.parent = parent
        self.constants = {}      # name -> expression text
        self.vars = {}           # classvars and properties: name -> expression text
        self.messages = {}       # name -> (parameters [(name, default text)], body text)


def parse_kod(text):
    text = strip_comments(text)
    # The first line of code: <Class> is <Parent>, or just <Class> for Object.
    m = re.match(r'\s*(\w+)(?:\s+is\s+(\w+))?\s*\n', text)
    if not m:
        return None
    c = KodClass(m.group(1).lower(), (m.group(2) or '').lower())
    sections = re.split(r'^(constants|resources|classvars|properties|messages)\s*:\s*$', text, flags=re.M)
    for name, body in zip(sections[1::2], sections[2::2]):
        if name == 'constants':
            for k, v in re.findall(r'^\s*(\w+)\s*=\s*(.+?)\s*$', body, re.M):
                c.constants[k.lower()] = v
        elif name in ('classvars', 'properties'):
            for k, v in re.findall(r'^\s*(\w+)\s*=\s*(.+?)\s*$', body, re.M):
                c.vars[k.lower()] = v
        elif name == 'messages':
            parse_messages(c, body)
    return c


def parse_messages(c, body):
    i = 0
    header = re.compile(r'^\s*(\w+)\s*\(([^)]*)\)', re.M)
    while True:
        m = header.search(body, i)
        if not m:
            return
        # The handler's block: the first { after the header (past its doc strings).
        j = body.find('{', m.end())
        if j < 0:
            return
        depth = 0
        k = j
        in_str = False
        while k < len(body):
            ch = body[k]
            if in_str:
                if ch == '\\':
                    k += 1
                elif ch == '"':
                    in_str = False
            elif ch == '"':
                in_str = True
            elif ch == '{':
                depth += 1
            elif ch == '}':
                depth -= 1
                if depth == 0:
                    break
            k += 1
        params = []
        for p in m.group(2).split(','):
            if '=' in p:
                pn, pv = p.split('=', 1)
                params.append((pn.strip().lower(), pv.strip()))
        c.messages[m.group(1).lower()] = (params, body[j + 1:k])
        i = k + 1


def load_kod(kod_dir):
    constants = {}
    with open(os.path.join(kod_dir, 'include', 'blakston.khd'), encoding='latin-1') as f:
        for k, v in re.findall(r'^\s*(\w+)\s*=\s*(.+?)\s*$', strip_comments(f.read()), re.M):
            constants[k.lower()] = v
    classes = {}
    paths = [(kod_dir, 'object.kod')]
    for root, _, files in os.walk(os.path.join(kod_dir, 'object')):
        paths += [(root, f) for f in files if f.endswith('.kod')]
    for root, f in paths:
            if True:
                with open(os.path.join(root, f), encoding='latin-1') as fh:
                    c = parse_kod(fh.read())
                if c:
                    c.directory = root
                    classes[c.name] = c
    return constants, classes


# ---------------------------------------------------------- interpreting it

class Room:
    """Works out a room class's CreateStandardObjects from its source."""

    def __init__(self, constants, classes, cls):
        self.constants = constants
        self.classes = classes
        self.cls = cls
        self.placed = []         # (class, row, col, fine_row, fine_col)
        self.unread = []         # messages it couldn't read all of
        self.props = {}          # properties set while it's built

    def chain(self, start):
        c = self.classes.get(start)
        while c:
            yield c
            c = self.classes.get(c.parent)

    def find(self, start, message):
        for c in self.chain(start):
            if message in c.messages:
                return c
        return None

    def constant(self, name, depth=0):
        if depth > 20:
            return UNKNOWN
        for c in self.chain(self.cls):
            if name in c.constants:
                return self.eval_text(c.constants[name], {}, depth + 1)
        if name in self.constants:
            return self.eval_text(self.constants[name], {}, depth + 1)
        for c in self.chain(self.cls):
            if name in c.vars:
                return self.eval_text(c.vars[name], {}, depth + 1)
        return UNKNOWN

    def eval_text(self, text, env, depth=0):
        try:
            return Parser(tokenize(text), self, env, depth).expr()
        except (IndexError, ValueError):
            return UNKNOWN

    def run(self, message, owner, env, depth=0):
        c = self.find(owner, message)
        if c is None:
            return
        params, body = c.messages[message]
        scope = {}
        for name, default in params:
            scope[name] = env.get(name, UNKNOWN) if name in env else self.eval_text(default, {})
        try:
            Parser(tokenize(body), self, scope, depth, message, c.name).block_body()
        except (IndexError, ValueError, TypeError) as e:
            self.unread.append('%s.%s' % (c.name, message))

    def new_hold(self, args):
        what = args.get('what', UNKNOWN)
        if not (isinstance(what, tuple) and what[0] == 'object'):
            return
        def num(name, default):
            v = args.get(name, default)
            return v if isinstance(v, int) else UNKNOWN
        self.placed.append((what[1], num('new_row', 1), num('new_col', 1),
                            num('fine_row', FINENESS // 2), num('fine_col', FINENESS // 2)))


class Parser:
    def __init__(self, toks, room, env, depth, message=None, owner=None):
        self.t = toks
        self.i = 0
        self.room = room
        self.env = env
        self.depth = depth
        self.message = message
        self.owner = owner

    def peek(self, k=0):
        return self.t[self.i + k] if self.i + k < len(self.t) else (None, None)

    def take(self, text=None):
        tok = self.t[self.i]
        if text is not None and tok[1] != text:
            raise ValueError('expected %s, got %s' % (text, tok[1]))
        self.i += 1
        return tok

    # Statements.

    def block_body(self):
        while self.i < len(self.t):
            if self.peek()[1] == '}':
                return
            self.statement()

    def block(self):
        self.take('{')
        self.block_body()
        self.take('}')

    def skip_to(self, *ends):
        depth = 0
        while self.i < len(self.t):
            tok = str(self.peek()[1])
            if depth == 0 and tok in ends:
                return
            if tok in '([{':
                depth += 1
            elif tok in ')]}':
                if depth == 0:
                    return
                depth -= 1
            self.i += 1

    def statement(self):
        kind, text = self.peek()
        if text == ';':
            self.take()
        elif kind == 'id' and text in ('local',):
            self.take()
            while self.peek()[1] != ';':
                name = self.take()[1]
                if name != ',':
                    self.env[name] = UNKNOWN
            self.take(';')
        elif kind == 'id' and text == 'if':
            self.take()
            self.skip_to('{')
            self.block()
            # Both sides: an object either side makes is the kod's.
            if self.peek()[1] == 'else':
                self.take()
                if self.peek()[1] == 'if':
                    self.statement()
                else:
                    self.block()
        elif kind == 'id' and text == 'for':
            self.take()
            var = self.take()[1]
            self.take('in')
            start = self.i
            self.skip_to('{')
            items = self.sub(self.t[start:self.i]).expr_safe()
            body_start = self.i
            self.skip_block()
            body = self.t[body_start + 1:self.i - 1]
            for item in (items if isinstance(items, list) else [UNKNOWN]):
                env = dict(self.env)
                env[var] = item
                p = Parser(body, self.room, env, self.depth, self.message, self.owner)
                p.block_body()
                self.env.update({k: v for k, v in p.env.items() if k in self.env})
        elif kind == 'id' and text == 'include':
            self.take()
            name = ''
            while self.peek()[1] != ';':
                name += str(self.take()[1])
            self.take(';')
            directory = self.room.classes[self.owner].directory
            path = next((os.path.join(directory, f) for f in os.listdir(directory)
                         if f.lower() == name.lower()), None)
            if path:
                with open(path, encoding='latin-1') as f:
                    Parser(tokenize(strip_comments(f.read())), self.room, self.env, self.depth,
                           self.message, self.owner).block_body()
        elif kind == 'id' and text == 'while':
            # A counted loop runs as the source says (up to 10000 times); any
            # other runs its body once.
            self.take()
            start = self.i
            self.skip_to('{')
            cond = self.t[start:self.i]
            body_start = self.i
            self.skip_block()
            body = self.t[body_start + 1:self.i - 1]
            for _ in range(10000):
                c = self.sub(cond).expr_safe()
                if c == 0:
                    break
                Parser(body, self.room, self.env, self.depth, self.message, self.owner).block_body()
                if not isinstance(c, int):
                    break
        elif kind == 'id' and text == 'propagate':
            self.take()
            parent = self.room.classes[self.owner].parent
            self.room.run(self.message, parent, self.env, self.depth)
            if self.peek()[1] == ';':
                self.take()
        elif kind == 'id' and text in ('return', 'break', 'continue'):
            self.skip_to(';', '}')
            if self.peek()[1] == ';':
                self.take()
        elif kind == 'id' and self.peek(1)[1] == '=':
            self.take()
            self.take('=')
            start = self.i
            self.skip_to(';')
            value = self.sub(self.t[start:self.i]).expr_safe()
            if text in self.env:
                self.env[text] = value
            else:
                self.room.props[text] = value
            self.take(';')
        else:
            start = self.i
            self.skip_to(';', '}')
            self.sub(self.t[start:self.i]).expr_safe()
            if self.peek()[1] == ';':
                self.take()

    def skip_block(self):
        depth = 0
        while True:
            tok = self.take()[1]
            if tok == '{':
                depth += 1
            elif tok == '}':
                depth -= 1
                if depth == 0:
                    return

    def sub(self, toks):
        return Parser(toks, self.room, self.env, self.depth, self.message, self.owner)

    # Expressions.

    def expr_safe(self):
        try:
            return self.expr()
        except (IndexError, ValueError, TypeError, ZeroDivisionError):
            return UNKNOWN

    def expr(self):
        v = self.term()
        while self.peek()[1] in ('+', '-', '|', 'and', 'or', '&', '=', '<>', '<', '>', '<=', '>='):
            op = self.take()[1]
            w = self.term()
            if isinstance(v, int) and isinstance(w, int):
                v = {'+': v + w, '-': v - w, '|': v | w, '&': v & w,
                     '=': int(v == w), '<>': int(v != w), '<': int(v < w), '>': int(v > w),
                     '<=': int(v <= w), '>=': int(v >= w)}.get(op, UNKNOWN)
            else:
                v = UNKNOWN
        return v

    def term(self):
        v = self.unary()
        while self.peek()[1] in ('*', '/', 'mod'):
            op = self.take()[1]
            w = self.unary()
            if isinstance(v, int) and isinstance(w, int) and (op == '*' or w):
                v = v * w if op == '*' else (v // w if op == '/' else v % w)
            else:
                v = UNKNOWN
        return v

    def unary(self):
        if self.peek()[1] == '-':
            self.take()
            v = self.unary()
            return -v if isinstance(v, int) else UNKNOWN
        if self.peek()[1] == 'not':
            self.take()
            self.unary()
            return UNKNOWN
        return self.primary()

    def primary(self):
        kind, text = self.take()
        if kind == 'num':
            return text
        if kind == 'str':
            return UNKNOWN
        if text == '$':
            return None
        if text == '(':
            v = self.expr()
            self.take(')')
            return v
        if text == '&':
            return ('class', self.take()[1])
        if text == '[':
            items = []
            while self.peek()[1] != ']':
                items.append(self.expr())
                if self.peek()[1] == ',':
                    self.take()
            self.take(']')
            return items
        if kind == 'id':
            if self.peek()[1] == '(':
                return self.call(text)
            if text == 'self':
                return SELF
            if text in ('true',):
                return 1
            if text in ('false',):
                return 0
            if text in self.env:
                return self.env[text]
            if text in self.room.props:
                return self.room.props[text]
            return self.room.constant(text, self.depth)
        raise ValueError(text)

    def args(self):
        """The arguments of a call: positional ones and #name=value ones."""
        pos, named = [], {}
        self.take('(')
        while self.peek()[1] != ')':
            if self.peek()[1] == '#':
                self.take()
                name = self.take()[1]
                self.take('=')
                named[name] = self.expr_safe_inline()
            elif self.peek()[1] == '@':
                self.take()
                pos.append(('message', self.take()[1]))
            else:
                pos.append(self.expr_safe_inline())
            if self.peek()[1] == ',':
                self.take()
        self.take(')')
        return pos, named

    def expr_safe_inline(self):
        start = self.i
        try:
            return self.expr()
        except (IndexError, ValueError, TypeError, ZeroDivisionError):
            self.i = start
            depth = 0
            while True:
                tok = str(self.peek()[1])
                if tok in '([':
                    depth += 1
                elif tok in ')]':
                    if depth == 0:
                        return UNKNOWN
                    depth -= 1
                elif tok == ',' and depth == 0:
                    return UNKNOWN
                self.take()

    def call(self, name):
        pos, named = self.args()
        if name == 'create':
            cls = pos[0] if pos else UNKNOWN
            return ('object', cls[1]) if isinstance(cls, tuple) else UNKNOWN
        if name in ('nth',) and len(pos) == 2 and isinstance(pos[0], list) and isinstance(pos[1], int):
            return pos[0][pos[1] - 1] if 0 < pos[1] <= len(pos[0]) else UNKNOWN
        if name == 'first' and pos and isinstance(pos[0], list) and pos[0]:
            return pos[0][0]
        if name == 'length' and pos and isinstance(pos[0], list):
            return len(pos[0])
        if name == 'cons' and len(pos) == 2 and isinstance(pos[1], list):
            return [pos[0]] + pos[1]
        if name == 'cons' and len(pos) == 2 and pos[1] is None:
            return [pos[0]]
        if name in ('send', 'post') and len(pos) >= 2 and pos[1][0] == 'message':
            target, message = pos[0], pos[1][1]
            if target == SELF:
                self.send_self(message, named)
            return UNKNOWN
        return UNKNOWN

    def send_self(self, message, named):
        if message == 'newhold':
            self.room.new_hold(named)
        elif message == 'createornobjfromlist':
            for e in named.get('lobjlist') or []:
                if isinstance(e, list) and len(e) in (3, 5):
                    fine = e[2:4] if len(e) == 5 else [FINENESS // 2, FINENESS // 2]
                    self.room.placed.append(('ornamentalobject', e[0], e[1], fine[0], fine[1]))
        elif message == 'createobjfromlist':
            cls = named.get('oobjectclassname', ('class', 'skull'))
            for e in named.get('lobjlist') or []:
                if isinstance(e, list) and len(e) in (2, 4) and isinstance(cls, tuple):
                    fine = e[2:4] if len(e) == 4 else [FINENESS // 2, FINENESS // 2]
                    self.room.placed.append((cls[1], e[0], e[1], fine[0], fine[1]))
        elif self.depth < 8 and self.room.find(self.room.cls, message):
            self.room.run(message, self.room.cls, named, self.depth + 1)


def kod_placements(kod_dir):
    """room number -> list of (class, row, col, fine_row, fine_col) its kod places."""
    constants, classes = load_kod(kod_dir)

    def is_room(name):
        seen = set()
        while name in classes and name not in seen:
            if name == 'room':
                return True
            seen.add(name)
            name = classes[name].parent
        return False

    rooms = {}
    for name, c in classes.items():
        if not is_room(name):
            continue
        r = Room(constants, classes, name)
        num = r.constant('piroom_num')
        if not isinstance(num, int):
            continue
        # As Create does: the constructors, Object's sends Constructed, Room's
        # CreateStandardObjects.
        r.run('constructor', name, {})
        rooms.setdefault(num, []).extend(r.placed)

        # What the room's own other messages place (a quest's next item, say),
        # where the source gives the exact spot.
        for c in r.chain(name):
            if c.name == 'room':
                break
            for message in c.messages:
                if message in ('constructor', 'constructed', 'createstandardobjects'):
                    continue
                other = Room(constants, classes, name)
                other.props = dict(r.props)
                other.run(message, c.name, {})
                rooms[num].extend(p for p in other.placed if all(isinstance(x, int) for x in p[1:]))
    return rooms


# ---------------------------------------------------------------- the dump

OBJECT_HEAD = re.compile(r'^if NOT Send\(oRoom,@HasPassiveAt,#class=&(\w+),#row=(-?\d+),#col=(-?\d+),'
                         r'#fine_row=(-?\d+),#fine_col=(-?\d+)\)$')


NOT_RECREATED = re.compile(r'^% Not recreated: &(\w+) at row (-?\d+), col (-?\d+)$')


def read_dump(path):
    """The dump's lines between begin and end, from a debug log or a dump."""
    lines = []
    inside = False
    with open(path, encoding='latin-1') as f:
        for line in f:
            line = line.rstrip('\r\n')
            # A debug log's lines start with the time and where Debug was called.
            m = re.match(r'^.*?\|\[[^\]]*\] (.*)$', line)
            if m:
                line = m.group(1)
            if line == '% DumpPassiveObjects begin':
                inside, lines = True, []
            elif line == '% DumpPassiveObjects end':
                inside = False
            elif inside:
                lines.append(line)
    return lines


def matches(spot, obj):
    return all(not known(a) or a == b for a, b in zip(spot, obj))


def main():
    if len(sys.argv) not in (3, 4):
        sys.exit(__doc__)
    placements = kod_placements(sys.argv[1])
    lines = read_dump(sys.argv[2])

    out = ['% DumpPassiveObjects begin']
    room_header = []
    room = None
    kept = left = 0
    i = 0
    while i < len(lines):
        line = lines[i]
        if line.startswith('% Room '):
            room_header = [line]
            room = int(line.split()[2].rstrip(':'))
            i += 1
            continue
        if line.startswith('oRoom = '):
            room_header.append(line)
            i += 1
            continue
        block = [line]
        note = NOT_RECREATED.match(line)
        if note:
            obj = (note.group(1).lower(), int(note.group(2)), int(note.group(3)), UNKNOWN, UNKNOWN)
            spots = placements.get(room, [])
            hit = next((s for s in spots if matches(obj, s)), None)
            if hit is not None:
                spots.remove(hit)
                left += 1
                i += 1
                continue
        m = OBJECT_HEAD.match(line)
        if m:
            while lines[i] != '}':
                i += 1
                block.append(lines[i])
            obj = (m.group(1).lower(),) + tuple(int(x) for x in m.group(2, 3, 4, 5))
            spots = placements.get(room, [])
            hit = next((s for s in spots if matches(s, obj)), None)
            if hit is not None:
                spots.remove(hit)
                left += 1
                i += 1
                continue
        if room_header:
            out.extend(room_header)
            room_header = []
        out.extend(block)
        kept += 1 if m else 0
        i += 1
    out.append('% DumpPassiveObjects end')

    text = '\n'.join(out) + '\n'
    if len(sys.argv) == 4:
        with open(sys.argv[3], 'w', encoding='latin-1') as f:
            f.write(text)
    else:
        sys.stdout.write(text)
    sys.stderr.write('%d objects the kod makes left out, %d kept\n' % (left, kept))


if __name__ == '__main__':
    main()
