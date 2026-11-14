import * as vscode from 'vscode';
import { KodSymbol, KodSymbolKind, parseFoldingRanges, parseKodDocument } from './kodSymbols';

export function activate(context: vscode.ExtensionContext) {
	context.subscriptions.push(vscode.languages.registerDocumentSymbolProvider(
		{ language: 'kod' }, new KodDocumentSymbolProvider()
	));
	context.subscriptions.push(vscode.languages.registerFoldingRangeProvider(
		{ language: 'kod' }, new KodFoldingProvider()
	));
}

class KodDocumentSymbolProvider implements vscode.DocumentSymbolProvider {
	public provideDocumentSymbols(
		document: vscode.TextDocument,
		token: vscode.CancellationToken
	): vscode.DocumentSymbol[] {
		if (token.isCancellationRequested) {
			return [];
		}
		return parseKodDocument(document.getText()).map(toDocumentSymbol);
	}
}

class KodFoldingProvider implements vscode.FoldingRangeProvider {
	public provideFoldingRanges(
		document: vscode.TextDocument,
		context: vscode.FoldingContext,
		token: vscode.CancellationToken
	): vscode.FoldingRange[] {
		void context;
		if (token.isCancellationRequested) {
			return [];
		}
		return parseFoldingRanges(document.getText()).map((fold) => new vscode.FoldingRange(fold.start, fold.end));
	}
}

const SYMBOL_KIND: { [kind in KodSymbolKind]: vscode.SymbolKind } = {
	class: vscode.SymbolKind.Class,
	namespace: vscode.SymbolKind.Namespace,
	constant: vscode.SymbolKind.Constant,
	string: vscode.SymbolKind.String,
	variable: vscode.SymbolKind.Variable,
	property: vscode.SymbolKind.Property,
	method: vscode.SymbolKind.Method,
	module: vscode.SymbolKind.Module
};

function toDocumentSymbol(symbol: KodSymbol): vscode.DocumentSymbol {
	const documentSymbol = new vscode.DocumentSymbol(
		symbol.name,
		symbol.detail,
		SYMBOL_KIND[symbol.kind],
		new vscode.Range(symbol.startLine, symbol.startChar, symbol.endLine, symbol.endChar),
		new vscode.Range(
			symbol.selectionStartLine,
			symbol.selectionStartChar,
			symbol.selectionEndLine,
			symbol.selectionEndChar
		)
	);
	documentSymbol.children = symbol.children.map(toDocumentSymbol);
	return documentSymbol;
}

export function deactivate() {}
