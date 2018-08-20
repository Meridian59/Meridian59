///////////////////////////////////////////////////////////////////////////
//
//  Copyright:  Ferdinand Prantl, portions by Stcherbatchenko Andrei
//  E-mail:     prantl@ff.cuni.cz
//
//  SGML syntax highlighing definition
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
static LPCTSTR s_apszSgmlKeywordList[] =
  {
    _T ("ABSTRACT"),
    _T ("ARTICLE"),
    _T ("AUTHOR"),
    _T ("BF"),
    _T ("BOOK"),
    _T ("CHAPT"),
    _T ("CODE"),
    _T ("COPYRIGHT"),
    _T ("DATE"),
    _T ("DEBIANDOC"),
    _T ("DESCRIP"),
    _T ("DOCTYPE"),
    _T ("EM"),
    _T ("EMAIL"),
    _T ("ENUM"),
    _T ("ENUMLIST"),
    _T ("EXAMPLE"),
    _T ("FOOTNOTE"),
    _T ("FTPSITE"),
    _T ("FTPPATH"),
    _T ("HEADING"),
    _T ("HTMLURL"),
    _T ("HTTPSITE"),
    _T ("HTTPPATH"),
    _T ("IT"),
    _T ("ITEM"),
    _T ("ITEMIZE"),
    _T ("LABEL"),
    _T ("LIST"),
    _T ("MANREF"),
    _T ("NAME"),
    _T ("P"),
    _T ("PRGN"),
    _T ("PUBLIC"),
    _T ("QREF"),
    _T ("QUOTE"),
    _T ("REF"),
    _T ("SECT"),
    _T ("SECT1"),
    _T ("SECT2"),
    _T ("SECT3"),
    _T ("SECT4"),
    _T ("STRONG"),
    _T ("SYSTEM"),
    _T ("TAG"),
    _T ("TAGLIST"),
    _T ("TITLE"),
    _T ("TITLEPAG"),
    _T ("TOC"),
    _T ("TSCREEN"),
    _T ("TT"),
    _T ("URL"),
    _T ("VAR"),
    _T ("VERB"),
    _T ("VERSION"),
    NULL
  };

static LPCTSTR s_apszUser1KeywordList[] =
  {
    _T ("COMPACT"),
    _T ("ID"),
    _T ("NAME"),
    _T ("SECTION"),
    NULL
  };

static LPCTSTR s_apszUser2KeywordList[] =
  {
    _T ("nbsp"),
    _T ("quot"),
    _T ("amp"),
    _T ("lt"),
    _T ("lt"),
    _T ("gt"),
    _T ("copy"),
    _T ("reg"),
    _T ("acute"),
    _T ("laquo"),
    _T ("raquo"),
    _T ("iexcl"),
    _T ("iquest"),
    _T ("Agrave"),
    _T ("agrave"),
    _T ("Aacute"),
    _T ("aacute"),
    _T ("Acirc"),
    _T ("acirc"),
    _T ("Atilde"),
    _T ("atilde"),
    _T ("Auml"),
    _T ("auml"),
    _T ("Aring"),
    _T ("aring"),
    _T ("AElig"),
    _T ("aelig"),
    _T ("Ccedil"),
    _T ("ccedil"),
    _T ("ETH"),
    _T ("eth"),
    _T ("Egrave"),
    _T ("egrave"),
    _T ("Eacute"),
    _T ("eacute"),
    _T ("Ecirc"),
    _T ("ecirc"),
    _T ("Euml"),
    _T ("euml"),
    _T ("Igrave"),
    _T ("igrave"),
    _T ("Iacute"),
    _T ("iacute"),
    _T ("Icirc"),
    _T ("icirc"),
    _T ("Iuml"),
    _T ("iuml"),
    _T ("Ntilde"),
    _T ("ntilde"),
    _T ("Ograve"),
    _T ("ograve"),
    _T ("Oacute"),
    _T ("oacute"),
    _T ("Ocirc"),
    _T ("ocirc"),
    _T ("Otilde"),
    _T ("otilde"),
    _T ("Ouml"),
    _T ("ouml"),
    _T ("Oslash"),
    _T ("oslash"),
    _T ("Ugrave"),
    _T ("ugrave"),
    _T ("Uacute"),
    _T ("uacute"),
    _T ("Ucirc"),
    _T ("ucirc"),
    _T ("Uuml"),
    _T ("uuml"),
    _T ("Yacute"),
    _T ("yacute"),
    _T ("yuml"),
    _T ("THORN"),
    _T ("thorn"),
    _T ("szlig"),
    _T ("sect"),
    _T ("para"),
    _T ("micro"),
    _T ("brvbar"),
    _T ("plusmn"),
    _T ("middot"),
    _T ("uml"),
    _T ("cedil"),
    _T ("ordf"),
    _T ("ordm"),
    _T ("not"),
    _T ("shy"),
    _T ("macr"),
    _T ("deg"),
    _T ("sup1"),
    _T ("sup2"),
    _T ("sup3"),
    _T ("frac14"),
    _T ("frac12"),
    _T ("frac34"),
    _T ("times"),
    _T ("divide"),
    _T ("cent"),
    _T ("pound"),
    _T ("curren"),
    _T ("yen"),
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
IsSgmlKeyword (LPCTSTR pszChars, int nLength)
{
  return IsXKeyword (s_apszSgmlKeywordList, pszChars, nLength);
}

static BOOL
IsUser1Keyword (LPCTSTR pszChars, int nLength)
{
  return IsXKeyword (s_apszUser1KeywordList, pszChars, nLength);
}

static BOOL
IsUser2Keyword (LPCTSTR pszChars, int nLength)
{
  return IsXKeyword (s_apszUser2KeywordList, pszChars, nLength);
}

static BOOL
IsSgmlNumber (LPCTSTR pszChars, int nLength)
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
#define COOKIE_USER1            0x0020

//
//
//
struct TSgmlSyntaxParser: public TSyntaxParser {
  public:
    TSgmlSyntaxParser(TCoolTextWnd* parent):TSyntaxParser(parent){}
    uint32 ParseLine(uint32 cookie, int index, TCoolTextWnd::TTextBlock* buf, int& items);
};
//
_COOLEDFUNC(TSyntaxParser*) SgmlParserCreator(TCoolTextWnd* parent)
{
  return  new TSgmlSyntaxParser(parent);
}
//
uint32 TSgmlSyntaxParser::ParseLine(uint32 dwCookie, int nLineIndex,
                                   TCoolTextWnd::TTextBlock* pBuf,
                                   int& nActualItems)
{
  int nLength = Parent->GetLineLength (nLineIndex);
  if (nLength <= 1)
    return dwCookie & COOKIE_EXT_COMMENT;

  LPCTSTR pszChars = GetLineText (nLineIndex);
  BOOL bFirstChar = (dwCookie & ~COOKIE_EXT_COMMENT) == 0;
  BOOL bRedefineBlock = TRUE;
  BOOL bDecIndex = FALSE;
  int nIdentBegin = -1;
  int I;
  for (I = 0;; I++)
    {
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
          else if (dwCookie & COOKIE_PREPROCESSOR)
            {
              DEFINE_BLOCK (nPos, COLORINDEX_PREPROCESSOR);
            }
          else
            {
              if (xisalnum (pszChars[nPos]) || pszChars[nPos] == '.')
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

      //  Extended comment /*....*/
      if (dwCookie & COOKIE_EXT_COMMENT)
        {
          if (I > 1 && pszChars[I] == '>' && pszChars[I - 1] == '-' && pszChars[I - 2] == '-')
            {
              dwCookie &= ~COOKIE_EXT_COMMENT;
              bRedefineBlock = TRUE;
            }
          continue;
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
      if (I < nLength - 3 && pszChars[I] == '<' && pszChars[I + 1] == '!' && pszChars[I + 2] == '-' && pszChars[I + 3] == '-')
        {
          DEFINE_BLOCK (I, COLORINDEX_COMMENT);
          I += 3;
          dwCookie |= COOKIE_EXT_COMMENT;
          dwCookie &= ~COOKIE_PREPROCESSOR;
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

      if (xisalnum (pszChars[I]) || pszChars[I] == '.')
        {
          if (nIdentBegin == -1)
            nIdentBegin = I;
        }
      else
        {
          if (nIdentBegin >= 0)
            {
              if (dwCookie & COOKIE_PREPROCESSOR)
                {
                  if (IsSgmlKeyword (pszChars + nIdentBegin, I - nIdentBegin))
                    {
                      DEFINE_BLOCK (nIdentBegin, COLORINDEX_KEYWORD);
                    }
                  else if (IsUser1Keyword (pszChars + nIdentBegin, I - nIdentBegin))
                    {
                      DEFINE_BLOCK (nIdentBegin, COLORINDEX_CUSTOM0);
                    }
                  else if (IsSgmlNumber (pszChars + nIdentBegin, I - nIdentBegin))
                    {
                      DEFINE_BLOCK (nIdentBegin, COLORINDEX_NUMBER);
                    }
                  else
                    {
                      goto next;
                    }
                }
              else if (dwCookie & COOKIE_USER1)
                {
                  if (IsUser2Keyword (pszChars + nIdentBegin, I - nIdentBegin))
                    {
                      DEFINE_BLOCK (nIdentBegin, COLORINDEX_CUSTOM1);
                    }
                  else
                    {
                      goto next;
                    }
                }
              else if (IsSgmlNumber (pszChars + nIdentBegin, I - nIdentBegin))
                {
                  DEFINE_BLOCK (nIdentBegin, COLORINDEX_NUMBER);
                }
              else
                {
                  goto next;
                }
              bRedefineBlock = TRUE;
              bDecIndex = TRUE;
              nIdentBegin = -1;
next:
              ;
            }
          //  Preprocessor start: < or bracket
          if (pszChars[I] == '{' ||
            (
              pszChars[I] == '<' &&
              !(I < nLength - 3 && pszChars[I + 1] == '!' && pszChars[I + 2] == '-' && pszChars[I + 3] == '-') // Not ...<!--...
            ))
            {
              DEFINE_BLOCK (I + 1, COLORINDEX_PREPROCESSOR);
              dwCookie |= COOKIE_PREPROCESSOR;
              nIdentBegin = -1;
              continue;
            }

          //  Preprocessor end: > or bracket
          if (dwCookie & COOKIE_PREPROCESSOR)
            {
              if (pszChars[I] == '>' || pszChars[I] == '}')
                {
                  dwCookie &= ~COOKIE_PREPROCESSOR;
                  nIdentBegin = -1;
                  bRedefineBlock = TRUE;
                  bDecIndex = TRUE;
                  continue;
                }
            }
          //  Preprocessor start: &
          if (pszChars[I] == '&')
            {
              dwCookie |= COOKIE_USER1;
              nIdentBegin = -1;
              continue;
            }

          //  Preprocessor end: ;
          if (dwCookie & COOKIE_USER1)
            {
              if (pszChars[I] == ';')
                {
                  dwCookie &= ~COOKIE_USER1;
                  nIdentBegin = -1;
                  continue;
                }
            }
        }
    }

  if (nIdentBegin >= 0 && (dwCookie & COOKIE_PREPROCESSOR))
    {
      if (IsSgmlKeyword (pszChars + nIdentBegin, I - nIdentBegin))
        {
          DEFINE_BLOCK (nIdentBegin, COLORINDEX_KEYWORD);
        }
      else if (IsUser1Keyword (pszChars + nIdentBegin, I - nIdentBegin))
        {
          DEFINE_BLOCK (nIdentBegin, COLORINDEX_CUSTOM0);
        }
      else if (IsUser2Keyword (pszChars + nIdentBegin, I - nIdentBegin))
        {
          DEFINE_BLOCK (nIdentBegin, COLORINDEX_CUSTOM1);
        }
      else if (IsSgmlNumber (pszChars + nIdentBegin, I - nIdentBegin))
        {
          DEFINE_BLOCK (nIdentBegin, COLORINDEX_NUMBER);
        }
    }
  //  Preprocessor start: < or {
  if (pszChars[I] == '{' ||
    (
      pszChars[I] == '<' &&
      !(I < nLength - 3 && pszChars[I + 1] == '!' && pszChars[I + 2] == '-' && pszChars[I + 3] == '-') // Not ...<!--...
    ))
    {
      DEFINE_BLOCK (I + 1, COLORINDEX_PREPROCESSOR);
      dwCookie |= COOKIE_PREPROCESSOR;
      nIdentBegin = -1;
      goto end;
    }

  //  Preprocessor end: > or }
  if (dwCookie & COOKIE_PREPROCESSOR)
    {
      if (pszChars[I] == '>' || pszChars[I] == '}')
        {
          dwCookie &= ~COOKIE_PREPROCESSOR;
          nIdentBegin = -1;
        }
    }
end:
  dwCookie &= (COOKIE_COMMENT | COOKIE_STRING | COOKIE_PREPROCESSOR);
  return dwCookie;
}
