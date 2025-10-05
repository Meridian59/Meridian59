///////////////////////////////////////////////////////////////////////////
//
//  Copyright:  Ferdinand Prantl, portions by Stcherbatchenko Andrei
//  E-mail:     prantl@ff.cuni.cz
//
//  TCL syntax highlighing definition
//
//  You are free to use or modify this code to the following restrictions:
//  - Acknowledge me somewhere in your about box, simple "Parts of code by.."
//  will be enough. If you can't (or don't want to), contact me personally.
//  - LEAVE THIS HEADER INTACT
////////////////////////////////////////////////////////////////////////////

#include <coolprj/pch.h>
#pragma hdrstop

#include <coolprj/cooledit.h>

using namespace owl;

//  C++ keywords (MSVC5.0 + POET5.0)
static LPCTSTR s_apszTclKeywordList[] =
  {
    _T ("case"),
    _T ("do"),
    _T ("done"),
    _T ("elif"),
    _T ("else"),
    _T ("esac"),
    _T ("fi"),
    _T ("for"),
    _T ("function"),
    _T ("if"),
    _T ("in"),
    _T ("then"),
    _T ("until"),
    _T ("while"),
    NULL
  };

static BOOL
IsXKeyword (LPCTSTR apszKeywords[], LPCTSTR pszChars, int nLength)
{
  for (int L = 0; apszKeywords[L] != NULL; L++)
    {
      if (_tcsnicmp (apszKeywords[L], pszChars, nLength) == 0
            && apszKeywords[L][nLength] == 0)
        return TRUE;
    }
  return FALSE;
}

static BOOL
IsTclKeyword (LPCTSTR pszChars, int nLength)
{
  return IsXKeyword (s_apszTclKeywordList, pszChars, nLength);
}

static BOOL
IsTclNumber (LPCTSTR pszChars, int nLength)
{
  if (nLength > 2 && pszChars[0] == '0' && pszChars[1] == 'x')
    {
      for (int I = 2; I < nLength; I++)
        {
          if (_istdigit (pszChars[I]) || (pszChars[I] >= 'A' && pszChars[I] <= 'F') ||
                (pszChars[I] >= 'a' && pszChars[I] <= 'f'))
            continue;
          return FALSE;
        }
      return TRUE;
    }
  if (!_istdigit (pszChars[0]))
    return FALSE;
  for (int I = 1; I < nLength; I++)
    {
      if (!_istdigit (pszChars[I]) && pszChars[I] != '+' &&
            pszChars[I] != '-' && pszChars[I] != '.' && pszChars[I] != 'e' &&
            pszChars[I] != 'E')
        return FALSE;
    }
  return TRUE;
}

#define DEFINE_BLOCK(pos, syntaxindex)  \
  CHECK((pos) >= 0 && (pos) <= nLength);\
  if (pBuf != NULL){\
    if (nActualItems == 0 || pBuf[nActualItems - 1].CharPos <= (pos)){\
      pBuf[nActualItems].CharPos     = (pos);\
      pBuf[nActualItems].SyntaxIndex = (syntaxindex);\
      nActualItems++;\
    }\
  }

#define COOKIE_COMMENT          0x0001
#define COOKIE_PREPROCESSOR     0x0002
#define COOKIE_EXT_COMMENT      0x0004
#define COOKIE_STRING           0x0008
#define COOKIE_CHAR             0x0010

//
//
//
struct TTclSyntaxParser: public TSyntaxParser {
  public:
    TTclSyntaxParser(TCoolTextWnd* parent):TSyntaxParser(parent){}
    uint32 ParseLine(uint32 cookie, int index, TCoolTextWnd::TTextBlock* buf, int& items);
};
//
_COOLEDFUNC(TSyntaxParser*) TclParserCreator(TCoolTextWnd* parent)
{
  return  new TTclSyntaxParser(parent);
}
//
uint32 TTclSyntaxParser::ParseLine(uint32 dwCookie, int nLineIndex,
                                   TCoolTextWnd::TTextBlock* pBuf, int& nActualItems)
{
  int nLength = Parent->GetLineLength (nLineIndex);
  if (nLength <= 1)
    return dwCookie & COOKIE_EXT_COMMENT;

  LPCTSTR pszChars = GetLineText(nLineIndex);
  BOOL bFirstChar = (dwCookie & ~COOKIE_EXT_COMMENT) == 0;
  BOOL bRedefineBlock = TRUE;
  BOOL bDecIndex = FALSE;
  int nIdentBegin = -1;
  int I;
  for (I = 0;; I++){
      if (bRedefineBlock)
        {
          int nPos = I;
          if (bDecIndex)
            nPos--;
          if (dwCookie & (COOKIE_COMMENT | COOKIE_EXT_COMMENT))
            {
              DEFINE_BLOCK (nPos, COLORINDEX_COMMENT);
            }
          else if (dwCookie & (COOKIE_CHAR | COOKIE_STRING))
            {
              DEFINE_BLOCK (nPos, COLORINDEX_STRING);
            }
          else
            {
              if (xisalnum (pszChars[nPos]) ||
                (
                  nPos > 0 &&
                  pszChars[nPos] == '.' &&
                  !xisalpha (pszChars[nPos - 1]) &&
                  !xisalpha (pszChars[nPos + 1])
                ))
                {
                  DEFINE_BLOCK (nPos, COLORINDEX_NORMALTEXT);
                }
              else
                {
                  DEFINE_BLOCK (nPos, COLORINDEX_OPERATOR);
                  bRedefineBlock = TRUE;
                  bDecIndex = TRUE;
                  goto out;
                }
            }
          bRedefineBlock = FALSE;
          bDecIndex = FALSE;
        }
out:

      if (I == nLength)
        break;

      if (dwCookie & COOKIE_COMMENT)
        {
          DEFINE_BLOCK (I, COLORINDEX_COMMENT);
          dwCookie |= COOKIE_COMMENT;
          break;
        }

      //  String constant "...."
      if (dwCookie & COOKIE_STRING)
        {
          if (pszChars[I] == '"' &&
            (
              I == 0 || // "...
              (I >= 1 && pszChars[I - 1] != '\\') || // ...?"...
              (I >= 2 && pszChars[I - 1] == '\\' && pszChars[I - 2] == '\\') // ...\\"...
              // TODO: What about ...\\\"...?
            ))
            {
              dwCookie &= ~COOKIE_STRING;
              bRedefineBlock = TRUE;
            }
          continue;
        }

      //  Char constant '..'
      if (dwCookie & COOKIE_CHAR)
        {
          if (pszChars[I] == '\'' &&
            (
              I == 0 || // '...
              (I >= 1 && pszChars[I - 1] != '\\') || // ...?'...
              (I >= 2 && pszChars[I - 1] == '\\' && pszChars[I - 2] == '\\') // ...\\'...
              // TODO: What about ...\\\'...?
            ))
            {
              dwCookie &= ~COOKIE_CHAR;
              bRedefineBlock = TRUE;
            }
          continue;
        }

      if (pszChars[I] == '#')
        {
          DEFINE_BLOCK (I, COLORINDEX_COMMENT);
          dwCookie |= COOKIE_COMMENT;
          break;
        }

      //  Normal text
      if (pszChars[I] == '"')
        {
          DEFINE_BLOCK (I, COLORINDEX_STRING);
          dwCookie |= COOKIE_STRING;
          continue;
        }
      if (pszChars[I] == '\'')
        {
          DEFINE_BLOCK (I, COLORINDEX_STRING);
          dwCookie |= COOKIE_CHAR;
          continue;
        }

      if (bFirstChar)
        {
          if (!isspace (pszChars[I]))
            bFirstChar = FALSE;
        }

      if (pBuf == NULL)
        continue;               //  We don't need to extract keywords,
      //  for faster parsing skip the rest of loop

      if (xisalnum (pszChars[I]) ||
        (
          I > 0 &&
          pszChars[I] == '.' &&
          !xisalpha (pszChars[I - 1]) &&
          !xisalpha (pszChars[I + 1])
        ))
        {
          if (nIdentBegin == -1)
            nIdentBegin = I;
        }
      else
        {
          if (nIdentBegin >= 0)
            {
              if (IsTclKeyword (pszChars + nIdentBegin, I - nIdentBegin))
                {
                  DEFINE_BLOCK (nIdentBegin, COLORINDEX_KEYWORD);
                }
              else if (IsTclNumber (pszChars + nIdentBegin, I - nIdentBegin))
                {
                  DEFINE_BLOCK (nIdentBegin, COLORINDEX_NUMBER);
                }
              else
                {
                  bool bFunction = FALSE;

                  for (int j = I; j < nLength; j++)
                    {
                      if (!isspace (pszChars[j]))
                        {
                          if (pszChars[j] == '(')
                            {
                              bFunction = TRUE;
                            }
                          break;
                        }
                    }
                  if (bFunction)
                    {
                      DEFINE_BLOCK (nIdentBegin, COLORINDEX_FUNCNAME);
                    }
                }
              bRedefineBlock = TRUE;
              bDecIndex = TRUE;
              nIdentBegin = -1;
            }
        }
    }

  if (nIdentBegin >= 0)
    {
      if (IsTclKeyword (pszChars + nIdentBegin, I - nIdentBegin))
        {
          DEFINE_BLOCK (nIdentBegin, COLORINDEX_KEYWORD);
        }
      else if (IsTclNumber (pszChars + nIdentBegin, I - nIdentBegin))
        {
          DEFINE_BLOCK (nIdentBegin, COLORINDEX_NUMBER);
        }
      else
        {
          bool bFunction = FALSE;

          for (int j = I; j < nLength; j++)
            {
              if (!isspace (pszChars[j]))
                {
                  if (pszChars[j] == '(')
                    {
                      bFunction = TRUE;
                    }
                  break;
                }
            }
          if (bFunction)
            {
              DEFINE_BLOCK (nIdentBegin, COLORINDEX_FUNCNAME);
            }
        }
    }

  if (pszChars[nLength - 1] != '\\')
    dwCookie &= COOKIE_EXT_COMMENT;
  return dwCookie;
}
