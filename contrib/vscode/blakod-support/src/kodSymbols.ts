export type KodSymbolKind =
	| 'class'
	| 'namespace'
	| 'constant'
	| 'string'
	| 'variable'
	| 'property'
	| 'method'
	| 'module';

export interface KodSymbol {
	name: string;
	detail: string;
	kind: KodSymbolKind;
	startLine: number;
	startChar: number;
	endLine: number;
	endChar: number;
	selectionStartLine: number;
	selectionStartChar: number;
	selectionEndLine: number;
	selectionEndChar: number;
	children: KodSymbol[];
}

export interface FoldRange {
	start: number;
	end: number;
}

const RESERVED = new Set([
	'and', 'break', 'classvars', 'constants', 'continue', 'else', 'end', 'for', 'if', 'in', 'is',
	'include', 'local', 'messages', 'mod', 'not', 'or', 'propagate', 'properties', 'resources',
	'return', 'while'
]);

const SECTION_KIND: { [name: string]: KodSymbolKind } = {
	constants: 'constant',
	resources: 'string',
	classvars: 'variable',
	properties: 'property'
};

function splitLines(text: string): string[] {
	return text.split(/\r?\n/);
}

function skipStringOnLine(text: string, col: number): number {
	col++;
	while (col < text.length) {
		if (text[col] === '\\') {
			col += 2;
			continue;
		}
		if (text[col] === '"') {
			return col + 1;
		}
		col++;
	}
	return text.length;
}

function stripCodeComment(line: string): string {
	let col = 0;
	while (col < line.length) {
		if (line[col] === '"') {
			col = skipStringOnLine(line, col);
			continue;
		}
		if (line[col] === '%') {
			return line.slice(0, col);
		}
		col++;
	}
	return line;
}

function compact(text: string): string {
	const flat = text.replace(/\s+/g, ' ').trim();
	if (flat.length <= 180) {
		return flat;
	}
	return flat.slice(0, 177) + '...';
}

function makeSymbol(
	name: string,
	detail: string,
	kind: KodSymbolKind,
	startLine: number,
	startChar: number,
	endLine: number,
	endChar: number,
	selectionStartChar: number,
	selectionEndChar: number,
	children: KodSymbol[] = []
): KodSymbol {
	const selectionStartLine = startLine;
	const selectionEndLine = startLine;
	if (endLine < selectionEndLine || (endLine === selectionEndLine && endChar < selectionEndChar)) {
		endLine = selectionEndLine;
		endChar = selectionEndChar;
	}
	if (endLine === startLine && endChar < startChar) {
		endChar = startChar;
	}
	return {
		name,
		detail,
		kind,
		startLine,
		startChar,
		endLine,
		endChar,
		selectionStartLine,
		selectionStartChar,
		selectionEndLine,
		selectionEndChar,
		children
	};
}

interface Pos {
	line: number;
	col: number;
}

function scanClosing(lines: string[], start: Pos, open: string, close: string): Pos | null {
	let line = start.line;
	let col = start.col;
	let depth = 1;
	while (line < lines.length) {
		const text = lines[line];
		while (col < text.length) {
			const c = text[col];
			if (c === '"') {
				col = skipStringOnLine(text, col);
				continue;
			}
			if (c === '%') {
				col = text.length;
				continue;
			}
			if (c === open) {
				depth++;
			} else if (c === close) {
				depth--;
				if (depth === 0) {
					return { line, col: col + 1 };
				}
			}
			col++;
		}
		line++;
		col = 0;
	}
	return null;
}

function findBrace(lines: string[], start: Pos): Pos | null {
	let line = start.line;
	let col = start.col;
	while (line < lines.length) {
		const text = lines[line];
		while (col < text.length) {
			const c = text[col];
			if (c === ' ' || c === '\t') {
				col++;
				continue;
			}
			if (c === '%') {
				col = text.length;
				continue;
			}
			if (c === '"') {
				col = skipStringOnLine(text, col);
				continue;
			}
			if (c === '{') {
				return { line, col };
			}
			return null;
		}
		line++;
		col = 0;
	}
	return null;
}

function paramsText(lines: string[], openParen: Pos, closeParen: Pos): string {
	let text = '';
	if (openParen.line === closeParen.line) {
		text = lines[openParen.line].slice(openParen.col + 1, closeParen.col - 1);
	} else {
		text = lines[openParen.line].slice(openParen.col + 1);
		for (let line = openParen.line + 1; line < closeParen.line; line++) {
			text += ' ' + lines[line];
		}
		text += ' ' + lines[closeParen.line].slice(0, closeParen.col - 1);
	}
	return compact(text);
}

function parseMessages(lines: string[], start: number, end: number): KodSymbol[] {
	const symbols: KodSymbol[] = [];
	let line = start;
	while (line < end) {
		const header = /^(\s*)([A-Za-z][A-Za-z0-9_]*)(\s*)\(/.exec(lines[line]);
		if (!header || RESERVED.has(header[2].toLowerCase())) {
			line++;
			continue;
		}
		const name = header[2];
		const nameCol = header[1].length;
		const parenCol = nameCol + name.length + header[3].length;
		const closeParen = scanClosing(lines, { line, col: parenCol + 1 }, '(', ')');
		const brace = closeParen === null ? null : findBrace(lines, closeParen);
		if (!closeParen || !brace || brace.line >= end) {
			line++;
			continue;
		}
		const closeBrace = scanClosing(lines, { line: brace.line, col: brace.col + 1 }, '{', '}');
		if (!closeBrace || closeBrace.line >= end) {
			line++;
			continue;
		}
		symbols.push(makeSymbol(
			name,
			paramsText(lines, { line, col: parenCol }, closeParen),
			'method',
			line,
			0,
			closeBrace.line,
			closeBrace.col,
			nameCol,
			nameCol + name.length
		));
		line = closeBrace.line + 1;
	}
	return symbols;
}

function visibleContinuation(line: string): string {
	return stripCodeComment(line).trim().replace(/\\$/, '').trim();
}

function isContinuationLine(prevRaw: string, currentRaw: string): boolean {
	if (stripCodeComment(prevRaw).trim().endsWith('\\')) {
		return true;
	}
	const currentTrim = currentRaw.trim();
	if (currentTrim.length === 0 || currentTrim.startsWith('%')) {
		return false;
	}
	return stripCodeComment(currentRaw).trim().startsWith('"');
}

function parseDeclarations(lines: string[], start: number, end: number, section: string): KodSymbol[] {
	const symbols: KodSymbol[] = [];
	let line = start;
	while (line < end) {
		const body = stripCodeComment(lines[line]).trim();
		if (body.length === 0) {
			line++;
			continue;
		}
		const includeMatch = /^include\s+(\S+)/i.exec(body);
		if (includeMatch) {
			const fileName = includeMatch[1];
			const startChar = Math.max(lines[line].indexOf(fileName), 0);
			symbols.push(makeSymbol(
				fileName,
				'include',
				'module',
				line,
				0,
				line,
				lines[line].length,
				startChar,
				startChar + fileName.length
			));
			line++;
			continue;
		}
		const assign = /^([A-Za-z][A-Za-z0-9_]*)\s*(?:=\s*(.*))?$/.exec(body);
		if (!assign) {
			line++;
			continue;
		}
		let endLine = line;
		while (endLine + 1 < end && isContinuationLine(lines[endLine], lines[endLine + 1])) {
			endLine++;
		}
		const parts: string[] = [];
		if (assign[2] !== undefined) {
			const first = assign[2].trim().replace(/\\$/, '').trim();
			if (first.length > 0) {
				parts.push(first);
			}
		}
		for (let extra = line + 1; extra <= endLine; extra++) {
			const piece = visibleContinuation(lines[extra]);
			if (piece.length > 0) {
				parts.push(piece);
			}
		}
		const detail = compact(parts.join(' '));
		const name = assign[1];
		const nameCol = Math.max(lines[line].indexOf(name), 0);
		let kind = SECTION_KIND[section] ?? 'constant';
		if (section === 'resources' && !detail.includes('"')) {
			kind = 'constant';
		}
		if (section === 'constants') {
			kind = 'constant';
		}
		symbols.push(makeSymbol(
			name,
			detail,
			kind,
			line,
			0,
			endLine,
			lines[endLine].length,
			nameCol,
			nameCol + name.length
		));
		line = endLine + 1;
	}
	return symbols;
}

function parseClass(lines: string[], classLine: number, match: RegExpExecArray): { symbol: KodSymbol; nextLine: number } {
	const sections: { name: string; line: number }[] = [];
	let endLine = lines.length - 1;
	let foundEnd = false;
	for (let line = classLine + 1; line < lines.length; line++) {
		const body = stripCodeComment(lines[line]).trim();
		if (/^end$/i.test(body)) {
			endLine = line;
			foundEnd = true;
			break;
		}
		const section = /^(constants|resources|classvars|properties|messages)\s*:$/i.exec(body);
		if (section) {
			sections.push({ name: section[1], line });
		}
	}

	const children: KodSymbol[] = [];
	for (let index = 0; index < sections.length; index++) {
		const section = sections[index];
		const bodyEnd = index + 1 < sections.length ? sections[index + 1].line : endLine;
		const sectionKey = section.name.toLowerCase();
		const members = sectionKey === 'messages'
			? parseMessages(lines, section.line + 1, bodyEnd)
			: parseDeclarations(lines, section.line + 1, bodyEnd, sectionKey);
		const lastLine = Math.max(section.line, bodyEnd - 1);
		const nameCol = Math.max(lines[section.line].toLowerCase().indexOf(sectionKey), 0);
		children.push(makeSymbol(
			section.name,
			'',
			'namespace',
			section.line,
			0,
			lastLine,
			lines[lastLine].length,
			nameCol,
			nameCol + section.name.length,
			members
		));
	}

	const className = match[1];
	const nameCol = Math.max(lines[classLine].indexOf(className), 0);
	const symbol = makeSymbol(
		className,
		match[2] ?? '',
		'class',
		classLine,
		0,
		endLine,
		lines[endLine].length,
		nameCol,
		nameCol + className.length,
		children
	);
	return { symbol, nextLine: foundEnd ? endLine + 1 : lines.length };
}

export function parseKodDocument(text: string): KodSymbol[] {
	const lines = splitLines(text);
	const symbols: KodSymbol[] = [];
	let line = 0;
	while (line < lines.length) {
		const body = stripCodeComment(lines[line]).trim();
		if (body.length === 0) {
			line++;
			continue;
		}
		const classMatch = /^([A-Za-z][A-Za-z0-9_]*)(?:\s+is\s+([A-Za-z][A-Za-z0-9_]*))?$/i.exec(body);
		if (!classMatch) {
			line++;
			continue;
		}
		const parsed = parseClass(lines, line, classMatch);
		symbols.push(parsed.symbol);
		line = parsed.nextLine;
	}
	return symbols;
}

export function parseBraceFolds(text: string): FoldRange[] {
	const lines = splitLines(text);
	const stack: number[] = [];
	const folds: FoldRange[] = [];
	for (let line = 0; line < lines.length; line++) {
		const raw = lines[line];
		let col = 0;
		while (col < raw.length) {
			const c = raw[col];
			if (c === '"') {
				col = skipStringOnLine(raw, col);
				continue;
			}
			if (c === '%') {
				break;
			}
			if (c === '{') {
				stack.push(line);
				col++;
				continue;
			}
			if (c === '}') {
				const start = stack.pop();
				if (start !== undefined && line > start) {
					folds.push({ start, end: line });
				}
				col++;
				continue;
			}
			col++;
		}
	}
	return folds;
}

export function parseFoldingRanges(text: string): FoldRange[] {
	const folds: FoldRange[] = [];
	const seen = new Set<string>();
	const add = (start: number, end: number) => {
		if (end <= start) {
			return;
		}
		const key = start + ':' + end;
		if (seen.has(key)) {
			return;
		}
		seen.add(key);
		folds.push({ start, end });
	};
	const walk = (symbols: KodSymbol[]) => {
		for (const symbol of symbols) {
			add(symbol.startLine, symbol.endLine);
			walk(symbol.children);
		}
	};
	walk(parseKodDocument(text));
	for (const fold of parseBraceFolds(text)) {
		add(fold.start, fold.end);
	}
	folds.sort((a, b) => a.start - b.start || b.end - a.end);
	return folds;
}
