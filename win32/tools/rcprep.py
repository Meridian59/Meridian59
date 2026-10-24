#!/usr/bin/env python3
"""rcprep.py <in.rc> <out.rc>: copy of a resource script that wrc can read on
Linux and macOS.  File names in resource statements ("bitmap\\ICON1.ICO") are
resolved to the file on disk, matching case-insensitively and with '/' separators,
relative to <out.rc> (wrc looks for them from there); everything else is copied
byte for byte."""
import os, re, sys

src, dst = sys.argv[1], sys.argv[2]
base = os.path.dirname(os.path.abspath(src))

def resolve(name):
    parts = [p for p in re.split(r'[\\/]+', name) if p]
    path = base
    for part in parts:
        if part in ('.', '..'):
            path = os.path.normpath(os.path.join(path, part))
            continue
        try:
            match = next((e for e in os.listdir(path) if e.lower() == part.lower()), None)
        except OSError:
            match = None
        if match is None:
            return None
        path = os.path.join(path, match)
    return path

# <name> <TYPE> [options] "file"
statement = re.compile(rb'^(\s*\S+\s+(?:ICON|BITMAP|CURSOR|AVI|RCDATA|WAVE|FONT|HTML|PNG)\b[^"\r\n]*")([^"\r\n]+)(")', re.M)

def fix(m):
    name = m.group(2).decode('latin-1').replace('\\\\', '\\')
    path = resolve(name)
    if path is None:
        sys.exit(f"{src}: resource file not found: {name}")
    path = os.path.relpath(path, os.path.dirname(os.path.abspath(dst)))
    return m.group(1) + path.encode('latin-1') + m.group(3)

data = open(src, 'rb').read()
open(dst, 'wb').write(statement.sub(fix, data))
