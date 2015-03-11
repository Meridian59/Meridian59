//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
///\file
/// class TCmdLine implementation
//----------------------------------------------------------------------------
#include <owl/pch.h>                                                  
#include <owl/cmdline.h>
#include <owl/private/memory.h>

namespace owl {

const tchar whitespace[] = _T(" \t");
const tchar terminator[] = _T("=/ \t\"");  // remove /- to dissallow separating there

TCmdLine::TCmdLine(const tstring& cmdLine)
{
  Buffer = new tchar[cmdLine.length() + 1];
  ::_tcscpy(Buffer, cmdLine.c_str());
  Reset();
}

void TCmdLine::Reset()
{
  Token = TokenStart = Buffer;
  TokenLen = 0;
  Kind = Start;
}

TCmdLine::~TCmdLine()
{
  delete [] Buffer;
}

TCmdLine::TKind TCmdLine::NextToken(bool removeCurrent)
{
	// Done parsing, no more tokens
	//
	if (Kind == Done)
		return Kind;

	// Move Token ptr to next token, by copying over current token, or by ptr
	// adjustment. TokenStart stays right past previous token
	//
	if (removeCurrent) {
		::_tcscpy(TokenStart, Token+TokenLen);
		Token = TokenStart;
	}
	else {
		Token += TokenLen;
		TokenStart = Token;
	}

	// Adjust token ptr to begining of token & determine kind
	//
	Token += _tcsspn(Token, whitespace); // skip leading whitespace
	switch (*Token) {
		case 0:
			Kind = Done;
			break;
		case _T('='):
			Kind = Value;
			Token++;
			break;
		case _T('-'):
		case _T('/'):
			Kind = Option;
			Token++;
			Token += _tcsspn(Token, whitespace); // skip any more whitespace
			if (*Token == _T('\"')) {
				Token++; // skip the quote
				TokenLen = static_cast<int>(_tcscspn(Token, _T("\"")));
				Token[TokenLen] = 0; // remove quote
				TokenLen++; // next char
				return Kind;
			}
			break;
		case _T('\"'):
			Kind = Name;
			Token++; // skip the quote
			TokenLen = static_cast<int>(_tcscspn(Token, _T("\"")));
			Token[TokenLen] = 0; // remove quote
			TokenLen++; // next char
			return Kind;
		default:
			Kind = Name;
	}
	Token += _tcsspn(Token, whitespace); // skip any more whitespace
	TokenLen = static_cast<int>(_tcscspn(Token, terminator));
	return Kind;
}

tstring TCmdLine::GetToken() const
{
  return tstring(Token, TokenLen);
}



} // OWL namespace
/* ========================================================================== */

