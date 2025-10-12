#include <coolprj/pch.h>
#pragma hdrstop

#include <coolprj/cooledit.h>

using namespace owl;

//  C++ keywords (MSVC5.0 + POET5.0)
static LPCTSTR s_apszCppKeywordList[] =
{
  _T("NOP"),
  _T("OR"),
  _T("OUTP"),
  _T("INP"),
  _T("ERR"),
  _T("STOP"),
  _T("RET"),
  _T("CALL"),
  _T("MOD"),
  _T("DIV"),
  _T("MUL"),
  _T("SUB"),
  _T("ADD"),
  _T("NEQ"),
  _T("EQU"),
  _T("GEQ"),
  _T("GRT"),
  _T("LEQ"),
  _T("LES"),
  _T("FJP"),
  _T("UJP"),
  _T("RED"),
  _T("MVRX"),
  _T("STO"),
  _T("COPY"),
  _T("LODIX"),
  _T("LODI"),
  _T("LOD"),
  _T("AND"),
  _T("ENTER"),
  _T("LEAVE"),
  _T("CHGSFP"),
  _T("RSTSFR"),
  _T("LODA"),
  _T("LODAX"),
  _T("LODEA"),
  _T("USTO"),
  _T("PUSH"),
  _T("POP"),
  _T("NFJP"),
  NULL
};

static BOOL IsCppKeyword(LPCTSTR pszChars, int nLength)
{
  for (int L = 0; s_apszCppKeywordList[L] != NULL; L ++)
  {
      if (_tcsncmp(s_apszCppKeywordList[L], pszChars, nLength) == 0
        && s_apszCppKeywordList[L][nLength] == 0)
      {
        return TRUE;
      }
  }
  return FALSE;
}

static BOOL IsCppNumber(LPCTSTR pszChars, int nLength)
{
  if (nLength > 2 && pszChars[0] == '0' && pszChars[1] == 'x')
  {
    for (int I = 2; I < nLength; I++)
    {
      if (isdigit(pszChars[I]) || (pszChars[I] >= 'A' && pszChars[I] <= 'F') ||
                    (pszChars[I] >= 'a' && pszChars[I] <= 'f'))
        continue;
      return FALSE;
    }
    return TRUE;
  }
  if (! isdigit(pszChars[0]))
    return FALSE;
  for (int I = 1; I < nLength; I++)
  {
    if (! isdigit(pszChars[I]) && pszChars[I] != '+' &&
      pszChars[I] != '-' && pszChars[I] != '.' && pszChars[I] != 'e' &&
      pszChars[I] != 'E')
      return FALSE;
  }
  return TRUE;
}

#define DEFINE_BLOCK(pos, colorindex)  \
  CHECK((pos) >= 0 && (pos) <= nLength);\
  if (pBuf != NULL){\
    if (nActualItems == 0 || pBuf[nActualItems - 1].CharPos <= (pos)){\
    pBuf[nActualItems].CharPos = (pos);\
    pBuf[nActualItems].SyntaxIndex = (colorindex);\
    nActualItems++;}\
  }

#define COOKIE_COMMENT  0x0001
#define COOKIE_PREPROCESSOR  0x0002
#define COOKIE_EXT_COMMENT  0x0004
#define COOKIE_STRING  0x0008
#define COOKIE_CHAR  0x0010

struct TAsmSyntaxParser: public TSyntaxParser {
  public:
    TAsmSyntaxParser(TCoolTextWnd* parent):TSyntaxParser(parent){}
    uint32 ParseLine(uint32 cookie, int index, TCoolTextWnd::TTextBlock* buf, int& items);
};
//
_COOLEDFUNC(TSyntaxParser*) AsmParserCreator(TCoolTextWnd* parent)
{
  return new TAsmSyntaxParser(parent);
}
//
uint32 TAsmSyntaxParser::ParseLine(uint32 dwCookie, int nLineIndex,
                                   TCoolTextWnd::TTextBlock* pBuf,
                                   int& nActualItems)
{
  int nLength = Parent->GetLineLength(nLineIndex);
  if (nLength <= 1)
    return dwCookie & COOKIE_EXT_COMMENT;

  LPCTSTR pszChars    = GetLineText(nLineIndex);
  BOOL bFirstChar     = (dwCookie & ~COOKIE_EXT_COMMENT) == 0;
  BOOL bRedefineBlock = TRUE;
  BOOL bDecIndex  = FALSE;
  int nIdentBegin = -1;
   int I;
  for (I = 0; ; I++){
    if (bRedefineBlock){
      int nPos = I;
      if (bDecIndex)
        nPos--;
      if (dwCookie & (COOKIE_COMMENT | COOKIE_EXT_COMMENT)){
        DEFINE_BLOCK(nPos, COLORINDEX_COMMENT);
      }
      else if (dwCookie & (COOKIE_CHAR | COOKIE_STRING)){
        DEFINE_BLOCK(nPos, COLORINDEX_STRING);
      }
      else if (dwCookie & COOKIE_PREPROCESSOR){
        DEFINE_BLOCK(nPos, COLORINDEX_PREPROCESSOR);
      }
      else{
        DEFINE_BLOCK(nPos, COLORINDEX_NORMALTEXT);
      }
      bRedefineBlock = FALSE;
      bDecIndex      = FALSE;
    }

    if (I == nLength)
      break;

    if (dwCookie & COOKIE_COMMENT){
      DEFINE_BLOCK(I, COLORINDEX_COMMENT);
      dwCookie |= COOKIE_COMMENT;
      break;
    }

    //  String constant "...."
    if (dwCookie & COOKIE_STRING){
      if (pszChars[I] == '"' && (I == 0 || pszChars[I - 1] != '\\')){
        dwCookie &= ~COOKIE_STRING;
        bRedefineBlock = TRUE;
      }
      continue;
    }

    //  Char constant '..'
    if (dwCookie & COOKIE_CHAR){
      if (pszChars[I] == '\'' && (I == 0 || pszChars[I - 1] != '\\')){
        dwCookie &= ~COOKIE_CHAR;
        bRedefineBlock = TRUE;
      }
      continue;
    }

    if (dwCookie & COOKIE_EXT_COMMENT){
      if (pszChars[I] == '}'){
        dwCookie &= ~COOKIE_EXT_COMMENT;
        bRedefineBlock = TRUE;
      }
      continue;
    }


    //  Extended comment /*....*/
    if (dwCookie & COOKIE_EXT_COMMENT){
      if (I > 0 && pszChars[I] == '/' && pszChars[I - 1] == '*'){
        dwCookie &= ~COOKIE_EXT_COMMENT;
        bRedefineBlock = TRUE;
      }
      continue;
    }

    //  Extended comment //
    if (I > 0 && pszChars[I] == '/' && pszChars[I - 1] == '/'){
      DEFINE_BLOCK(I - 1, COLORINDEX_COMMENT);
      dwCookie |= COOKIE_COMMENT;
      break;
    }

    if (I > 0 && pszChars[I - 1] == ';'){
      DEFINE_BLOCK(I - 1, COLORINDEX_COMMENT);
      dwCookie |= COOKIE_COMMENT;
      break;
    }

    //  Preprocessor directive #....
    if (dwCookie & COOKIE_PREPROCESSOR){
      if (I > 0 && pszChars[I] == '*' && pszChars[I - 1] == '/'){
        DEFINE_BLOCK(I - 1, COLORINDEX_COMMENT);
        dwCookie |= COOKIE_EXT_COMMENT;
      }
      continue;
    }

    //  Normal text
    if (pszChars[I] == '"'){
      DEFINE_BLOCK(I, COLORINDEX_STRING);
      dwCookie |= COOKIE_STRING;
      continue;
    }
    if (pszChars[I] == '\''){
      DEFINE_BLOCK(I, COLORINDEX_STRING);
      dwCookie |= COOKIE_CHAR;
      continue;
    }
    if (pszChars[I] == '{'){
      DEFINE_BLOCK(I, COLORINDEX_COMMENT);
      dwCookie |= COOKIE_EXT_COMMENT;
      continue;
    }

    if (I > 0 && pszChars[I] == '*' && pszChars[I - 1] == '/'){
      DEFINE_BLOCK(I - 1, COLORINDEX_COMMENT);
      dwCookie |= COOKIE_EXT_COMMENT;
      continue;
    }

    if (bFirstChar){
      if (pszChars[I] == '#'){
        DEFINE_BLOCK(I, COLORINDEX_PREPROCESSOR);
        dwCookie |= COOKIE_PREPROCESSOR;
        continue;
      }
      if (! isspace(pszChars[I]))
        bFirstChar = FALSE;
    }

    if (pBuf == NULL)
      continue;  //  We don't need to extract keywords,
            //  for faster parsing skip the rest of loop

    if (isalnum(pszChars[I]) || pszChars[I] == '_' || pszChars[I] == '.'){
      if (nIdentBegin == -1)
        nIdentBegin = I;
    }
    else{
      if (nIdentBegin >= 0){
        if (IsCppKeyword(pszChars + nIdentBegin, I - nIdentBegin)){
          DEFINE_BLOCK(nIdentBegin, COLORINDEX_KEYWORD);
        }
        else if (IsCppNumber(pszChars + nIdentBegin, I - nIdentBegin)){
          DEFINE_BLOCK(nIdentBegin, COLORINDEX_NUMBER);
        }
        bRedefineBlock = TRUE;
        bDecIndex = TRUE;
        nIdentBegin = -1;
      }
    }
  }

  if (nIdentBegin >= 0){
    if (IsCppKeyword(pszChars + nIdentBegin, I - nIdentBegin)){
      DEFINE_BLOCK(nIdentBegin, COLORINDEX_KEYWORD);
    }
    else if (IsCppNumber(pszChars + nIdentBegin, I - nIdentBegin)){
      DEFINE_BLOCK(nIdentBegin, COLORINDEX_NUMBER);
    }
  }

  if (pszChars[nLength - 1] != '\\')
    dwCookie &= COOKIE_EXT_COMMENT;
  return dwCookie;
}
//
