import { create } from 'domain';
import * as vscode from 'vscode';

// This method is called when your extension is activated
// Your extension is activated the very first time the command is executed
export function activate(context: vscode.ExtensionContext) {
	context.subscriptions.push(vscode.languages.registerDocumentSymbolProvider(
		{language: "kod"}, new KodDocumentSymbolProvider()
	));
}

class KodDocumentSymbolProvider implements vscode.DocumentSymbolProvider {
	public provideDocumentSymbols(
			document: vscode.TextDocument,
			token: vscode.CancellationToken): Promise<vscode.DocumentSymbol[]> {
			return new Promise((resolve, reject) => {

				let symbols: vscode.DocumentSymbol[] = [];
				let documentText = document.getText();

				// Find the class name and create a symbol for it
				// This symbol will serve as the parent for all other symbols
				let classMatch = /\n(\w+)/.exec(documentText);

				// If we found the class name, continue building the symbol tree
				if (classMatch) {
					let classSymbol = new vscode.DocumentSymbol(
						classMatch[1],
						"",
						vscode.SymbolKind.Class,
						new vscode.Range(
							document.positionAt(documentText.indexOf(classMatch[0]) + 1),
							document.positionAt(documentText.indexOf(classMatch[0]) + classMatch[0].length)
						),
						new vscode.Range(
							document.positionAt(documentText.indexOf(classMatch[0]) + 1),
							document.positionAt(documentText.indexOf(classMatch[0]) + classMatch[0].length)
						)
					);
					symbols.push(classSymbol);

					//
					// CONSTANTS:
					// 
					let sectionMatch = /(constants):\s*(.*?)\nresources:\n/s.exec(documentText);
					if (sectionMatch) {
						let sectionSymbol = new vscode.DocumentSymbol(
							sectionMatch[1],
							"",
							vscode.SymbolKind.Namespace,
							new vscode.Range(
								document.positionAt(documentText.indexOf(sectionMatch[0]) + 1),
								document.positionAt(documentText.indexOf(sectionMatch[0]) + sectionMatch[0].length)
							),
							new vscode.Range(
								document.positionAt(documentText.indexOf(sectionMatch[0]) + 1),
								document.positionAt(documentText.indexOf(sectionMatch[0]) + sectionMatch[0].length)
							)
						);
						classSymbol.children.push(sectionSymbol);					

						// Let's only look at the relevant text for this section
						let selectionText = sectionMatch[2];

						if (selectionText) {
							let childrenRegex = /^\s*([A-Z_]+)\s*=\s*(.*)\s*$/mg;
							let childrenMatch = childrenRegex.exec(selectionText);
							while(childrenMatch) {
								sectionSymbol.children.push(new vscode.DocumentSymbol(
									childrenMatch[1],
									childrenMatch[2],
									vscode.SymbolKind.Constant,
									new vscode.Range(
										document.positionAt(documentText.indexOf(childrenMatch[0]) + 1),
										document.positionAt(documentText.indexOf(childrenMatch[0]) + childrenMatch[0].length)
									),
									new vscode.Range(
										document.positionAt(documentText.indexOf(childrenMatch[0]) + 1),
										document.positionAt(documentText.indexOf(childrenMatch[0]) + childrenMatch[0].length)
									)
								));
								
								// next childrenMatch
								childrenMatch = childrenRegex.exec(selectionText);
							}
						}
					}

					//
					// RESOURCES:
					// 
					sectionMatch = /(resources):\s*(.*?)\nclassvars:\n/s.exec(documentText);
					if (sectionMatch) {
						let sectionSymbol = new vscode.DocumentSymbol(
							sectionMatch[1],
							"",
							vscode.SymbolKind.Namespace,
							new vscode.Range(
								document.positionAt(documentText.indexOf(sectionMatch[0]) + 1),
								document.positionAt(documentText.indexOf(sectionMatch[0]) + sectionMatch[0].length)
							),
							new vscode.Range(
								document.positionAt(documentText.indexOf(sectionMatch[0]) + 1),
								document.positionAt(documentText.indexOf(sectionMatch[0]) + sectionMatch[0].length)
							)
						);
						classSymbol.children.push(sectionSymbol);					

						// Let's only look at the relevant text for this section
						let selectionText = sectionMatch[2];

						if (selectionText) {
							let childrenRegex = /([A-Za-z0-9_]+)\s*=\s*\\?\s*(.*?)$/gm;
							let childrenMatch = childrenRegex.exec(selectionText);
							while(childrenMatch) {
								sectionSymbol.children.push(new vscode.DocumentSymbol(
									childrenMatch[1],
									childrenMatch[2],
									vscode.SymbolKind.Constant,
									new vscode.Range(
										document.positionAt(documentText.indexOf(childrenMatch[0]) + 1),
										document.positionAt(documentText.indexOf(childrenMatch[0]) + childrenMatch[0].length)
									),
									new vscode.Range(
										document.positionAt(documentText.indexOf(childrenMatch[0]) + 1),
										document.positionAt(documentText.indexOf(childrenMatch[0]) + childrenMatch[0].length)
									)
								));
								
								// next childrenMatch
								childrenMatch = childrenRegex.exec(selectionText);
							}
						}
					}

					//
					// CLASSVARS:
					// 
					sectionMatch = /(classvars):\s*(.*?)\nproperties:\n/s.exec(documentText);
					if (sectionMatch) {
						let sectionSymbol = new vscode.DocumentSymbol(
							sectionMatch[1],
							"",
							vscode.SymbolKind.Namespace,
							new vscode.Range(
								document.positionAt(documentText.indexOf(sectionMatch[0]) + 1),
								document.positionAt(documentText.indexOf(sectionMatch[0]) + sectionMatch[0].length)
							),
							new vscode.Range(
								document.positionAt(documentText.indexOf(sectionMatch[0]) + 1),
								document.positionAt(documentText.indexOf(sectionMatch[0]) + sectionMatch[0].length)
							)
						);
						classSymbol.children.push(sectionSymbol);					

						// Let's only look at the relevant text for this section
						let selectionText = sectionMatch[2];

						if (selectionText) {
							let childrenRegex = /([A-Za-z0-9_]+)\s*=\s*\\?\s*(.*?)$/gm;
							let childrenMatch = childrenRegex.exec(selectionText);
							while(childrenMatch) {
								sectionSymbol.children.push(new vscode.DocumentSymbol(
									childrenMatch[1],
									childrenMatch[2],
									vscode.SymbolKind.Constant,
									new vscode.Range(
										document.positionAt(documentText.indexOf(childrenMatch[0]) + 1),
										document.positionAt(documentText.indexOf(childrenMatch[0]) + childrenMatch[0].length)
									),
									new vscode.Range(
										document.positionAt(documentText.indexOf(childrenMatch[0]) + 1),
										document.positionAt(documentText.indexOf(childrenMatch[0]) + childrenMatch[0].length)
									)
								));
								
								// next childrenMatch
								childrenMatch = childrenRegex.exec(selectionText);
							}
						}
					}

					//
					// PROPERTIES:
					// 
					sectionMatch = /(properties):\s*(.*?)\nmessages:\n/s.exec(documentText);
					if (sectionMatch) {
						let sectionSymbol = new vscode.DocumentSymbol(
							sectionMatch[1],
							"",
							vscode.SymbolKind.Namespace,
							new vscode.Range(
								document.positionAt(documentText.indexOf(sectionMatch[0]) + 1),
								document.positionAt(documentText.indexOf(sectionMatch[0]) + sectionMatch[0].length)
							),
							new vscode.Range(
								document.positionAt(documentText.indexOf(sectionMatch[0]) + 1),
								document.positionAt(documentText.indexOf(sectionMatch[0]) + sectionMatch[0].length)
							)
						);
						classSymbol.children.push(sectionSymbol);					

						// Let's only look at the relevant text for this section
						let selectionText = sectionMatch[2];

						if (selectionText) {
							let childrenRegex = /([A-Za-z0-9_]+)\s*=\s*\\?\s*(.*?)$/gm;
							let childrenMatch = childrenRegex.exec(selectionText);
							while(childrenMatch) {
								sectionSymbol.children.push(new vscode.DocumentSymbol(
									childrenMatch[1],
									childrenMatch[2],
									vscode.SymbolKind.Constant,
									new vscode.Range(
										document.positionAt(documentText.indexOf(childrenMatch[0]) + 1),
										document.positionAt(documentText.indexOf(childrenMatch[0]) + childrenMatch[0].length)
									),
									new vscode.Range(
										document.positionAt(documentText.indexOf(childrenMatch[0]) + 1),
										document.positionAt(documentText.indexOf(childrenMatch[0]) + childrenMatch[0].length)
									)
								));
								
								// next childrenMatch
								childrenMatch = childrenRegex.exec(selectionText);
							}
						}
					}

					//
					// MESSAGES:
					// 
					sectionMatch = /^(messages):\s*(.*)(?=\nend)/gms.exec(documentText);
					if (sectionMatch) {
						let sectionSymbol = new vscode.DocumentSymbol(
							sectionMatch[1],
							"",
							vscode.SymbolKind.Namespace,
							new vscode.Range(
								document.positionAt(documentText.indexOf(sectionMatch[0]) + 1),
								document.positionAt(documentText.indexOf(sectionMatch[0]) + sectionMatch[0].length)
							),
							new vscode.Range(
								document.positionAt(documentText.indexOf(sectionMatch[0]) + 1),
								document.positionAt(documentText.indexOf(sectionMatch[0]) + sectionMatch[0].length)
							)
						);
						classSymbol.children.push(sectionSymbol);					

						// Let's only look at the relevant text for this section
						let selectionText = sectionMatch[2];

						if (selectionText) {
							let childrenRegex = /^\s*(?!.*[@#"])([A-Z]\w+)\s*\((\s*\w*\s*=.*)*\)\s*("[^"]*"\s*)*{/gm;
							let childrenMatch = childrenRegex.exec(selectionText);
							while(childrenMatch) {
								sectionSymbol.children.push(new vscode.DocumentSymbol(
									childrenMatch[1],
									childrenMatch[2],
									vscode.SymbolKind.Constant,
									new vscode.Range(
										document.positionAt(documentText.indexOf(childrenMatch[0]) + 1),
										document.positionAt(documentText.indexOf(childrenMatch[0]) + childrenMatch[0].length)
									),
									new vscode.Range(
										document.positionAt(documentText.indexOf(childrenMatch[0]) + 1),
										document.positionAt(documentText.indexOf(childrenMatch[0]) + childrenMatch[0].length)
									)
								));
								
								// next childrenMatch
								childrenMatch = childrenRegex.exec(selectionText);
							}
						}
					}
				}

				resolve(symbols);
			});
	}
}

// This method is called when your extension is deactivated
export function deactivate() {}
