///////////////////////////////////////////////////////////////////////////
//
//  Copyright:  Stcherbatchenko Andrei
//  E-mail:     windfall@gmx.de
//
//  Implementation of the CCrystalEditView class, a part of the Crystal Edit -
//  syntax coloring text editor.
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
static LPCTSTR s_apszCppKeywordList[] =
{
  _T("__asm"),
  _T("__based"),
  _T("__cdecl"),
  _T("__declspec"),
  _T("__except"),
  _T("__export"),
  _T("__far16"),
  _T("__fastcall"),
  _T("__finally"),
  _T("__inline"),
  _T("__int16"),
  _T("__int32"),
  _T("__int64"),
  _T("__int8"),
  _T("__leave"),
  _T("__multiple_inheritance"),
  _T("__pascal"),
  _T("__single_inheritance"),
  _T("__stdcall"),
  _T("__syscall"),
  _T("__try"),
  _T("__uuidof"),
  _T("__virtual_inheritance"),
  _T("_asm"),
  _T("_cdecl"),
  _T("_export"),
  _T("_far16"),
  _T("_fastcall"),
  _T("_pascal"),
  _T("_persistent"),
  _T("_stdcall"),
  _T("_syscall"),
  _T("auto"),
  _T("bool"),
  _T("break"),
  _T("case"),
  _T("catch"),
  _T("char"),
  _T("class"),
  _T("const"),
  _T("const_cast"),
  _T("constexpr"),
  _T("continue"),
  _T("decltype"),
  _T("default"),
  _T("delete"),
  _T("depend"),
  _T("dllexport"),
  _T("dllimport"),
  _T("do"),
  _T("double"),
  _T("dynamic_cast"),
  _T("else"),
  _T("enum"),
  _T("explicit"),
  _T("extern"),
  _T("false"),
  _T("float"),
  _T("for"),
  _T("friend"),
  _T("goto"),
  _T("if"),
  _T("inline"),
  _T("int"),
  _T("interface"),
  _T("long"),
  _T("main"),
  _T("mutable"),
  _T("naked"),
  _T("namespace"),
  _T("new"),
  _T("ondemand"),
  _T("operator"),
  _T("override"),
  _T("persistent"),
  _T("private"),
  _T("protected"),
  _T("public"),
  _T("register"),
  _T("reinterpret_cast"),
  _T("return"),
  _T("short"),
  _T("signed"),
  _T("sizeof"),
  _T("static"),
  _T("static_cast"),
  _T("struct"),
  _T("switch"),
  _T("template"),
  _T("this"),
  _T("thread"),
  _T("throw"),
  _T("true"),
  _T("try"),
  _T("typedef"),
  _T("typeid"),
  _T("typename"),
  _T("union"),
  _T("unsigned"),
  _T("using"),
  _T("uuid"),
  _T("virtual"),
  _T("void"),
  _T("volatile"),
  _T("while"),
  _T("wmain"),
  _T("xalloc"),
  NULL
};


/*
static bool
IsUser1Keyword (LPCTSTR pszChars, int nLength)
{
  return IsXKeyword (s_apszUser1KeywordList, pszChars, nLength);
}
*/

static bool IsCppKeyword(LPCTSTR pszChars, int len)
{
  for (int i = 0; s_apszCppKeywordList[i] != NULL; i ++){
    if(_tcsncmp(s_apszCppKeywordList[i], pszChars, len) == 0
        && s_apszCppKeywordList[i][len] == 0)
      return true;
  }
  return false;
}

static bool
IsCppNumber(LPCTSTR pszChars, int len)
{
  if (len > 2 && pszChars[0] == _T('0') && pszChars[1] == _T('x')){
    for (int I = 2; I < len; I++){
      if (_istdigit(pszChars[I]) || (pszChars[I] >= _T('A') &&
           pszChars[I] <= _T('F')) || (pszChars[I] >= _T('a') &&
           pszChars[I] <= _T('f')))
        continue;
      return false;
    }
    return true;
  }
  if (!_istdigit(pszChars[0]))
    return false;
  for (int I = 1; I < len; I++){
    if (! _istdigit(pszChars[I]) && pszChars[I] != _T('+') &&
      pszChars[I] != _T('-') && pszChars[I] != _T('.') && pszChars[I] != _T('e') &&
      pszChars[I] != _T('E'))
      return false;
  }
  return true;
}

inline void DefineBlock(TCoolTextWnd::TTextBlock* pBuf, int& actualItems, int pos, int syntaxindex)
{
  CHECK(pos >= 0);
  if(pBuf){
    if (actualItems == 0 || pBuf[actualItems - 1].CharPos <= pos){
      pBuf[actualItems].CharPos     = pos;
      pBuf[actualItems].SyntaxIndex = syntaxindex;
      actualItems++;
    }
  }
}
#if 0
#define DEFINE_BLOCK(pos, syntaxindex)  \
  CHECK((pos) >= 0 && (pos) <= len);\
  if (pBuf != NULL){\
    if (actualItems == 0 || pBuf[actualItems - 1].CharPos <= (pos)){\
      pBuf[actualItems].CharPos     = (pos);\
      pBuf[actualItems].SyntaxIndex = (syntaxindex);\
      actualItems++;\
    }\
  }
#else
#define DEFINE_BLOCK(pos, syntaxindex)\
  DefineBlock(pBuf, actualItems, pos, syntaxindex)
#endif

#define COOKIE_COMMENT        0x0001
#define COOKIE_PREPROCESSOR    0x0002
#define COOKIE_EXT_COMMENT    0x0004
#define COOKIE_STRING          0x0008
#define COOKIE_CHAR            0x0010
#define COOKIE_OPERATOR        0x0020

//
//
//
//
struct TCSyntaxParser: public TSyntaxParser {
  public:
    TCSyntaxParser(TCoolTextWnd* parent):TSyntaxParser(parent){}
    uint32 ParseLine(uint32 cookie, int index, TCoolTextWnd::TTextBlock* buf, int& items);
};
//
_COOLEDFUNC(TSyntaxParser*) CParserCreator(TCoolTextWnd* parent)
{
  return  new TCSyntaxParser(parent);
}
//
uint32 TCSyntaxParser::ParseLine(uint32 cookie, int index,
                                 TCoolTextWnd::TTextBlock* pBuf,
                                 int& actualItems)
{
  int len = Parent->GetLineLength(index);
  if (len <= 0)
    return cookie & COOKIE_EXT_COMMENT;

  LPCTSTR pszChars = GetLineText(index);
  bool bFirstChar     = (cookie & ~COOKIE_EXT_COMMENT) == 0;
  bool bRedefineBlock = true;
  bool bDecIndex      = false;
  bool bWasCommentStart = false;
  int nIdentBegin      = -1;
  int I;
  for(I = 0; ; I++){
    if (bRedefineBlock){
      int nPos = I;
      if (bDecIndex){
        nPos--;
        //actualItems--;
      }
      bRedefineBlock = false;
      bDecIndex      = false;
      if (cookie & (COOKIE_COMMENT | COOKIE_EXT_COMMENT)){
        DEFINE_BLOCK(nPos, COLORINDEX_COMMENT);
      }
      else if (cookie & COOKIE_STRING){
        DEFINE_BLOCK(nPos, COLORINDEX_STRING);
      }
      else if (cookie & COOKIE_CHAR){
        DEFINE_BLOCK(nPos, COLORINDEX_CHARACTER);
      }
      else if (cookie & COOKIE_PREPROCESSOR){
        DEFINE_BLOCK(nPos, COLORINDEX_PREPROCESSOR);
      }
      else{
        //DEFINE_BLOCK(nPos, COLORINDEX_NORMALTEXT);
        if (xisalnum(pszChars[nPos]) ||
          (
            nPos > 0 &&
            pszChars[nPos] == _T('.') &&
            !xisalpha(pszChars[nPos - 1]) &&
            !xisalpha(pszChars[nPos + 1])
          ))
        {
          DEFINE_BLOCK (nPos, COLORINDEX_NORMALTEXT);
        }
        else{
          DEFINE_BLOCK (nPos, COLORINDEX_OPERATOR);
          bRedefineBlock = true;
          bDecIndex = true;
        }
      }
    }

    if (I == len)
      break;

    if (cookie & COOKIE_COMMENT){
      DEFINE_BLOCK(I, COLORINDEX_COMMENT);
      cookie |= COOKIE_COMMENT;
      break;
    }

    //  String constant "...."
    if (cookie & COOKIE_STRING)
    {
      if (pszChars[I] == '"' &&
        (
          I == 0 || // "...
          (I >= 1 && pszChars[I - 1] != '\\') || // ...?"...
          (I >= 2 && pszChars[I - 1] == '\\' && pszChars[I - 2] == '\\') // ...\\"...
          // TODO: What about ...\\\"...?
        ))
      {
        cookie &= ~COOKIE_STRING;
        bRedefineBlock = true;
      }
      continue;
    }

    //  Char constant '..'
    if (cookie & COOKIE_CHAR)
    {
      if (pszChars[I] == '\'' &&
        (
          I == 0 || // '...
          (I >= 1 && pszChars[I - 1] != '\\') || // ...?'...
          (I >= 2 && pszChars[I - 1] == '\\' && pszChars[I - 2] == '\\') // ...\\'...
          // TODO: What about ...\\\'...?
        ))
      {
        cookie &= ~COOKIE_CHAR;
        bRedefineBlock = true;
      }
      continue;
    }

    //  Extended comment /*....*/
    if (cookie & COOKIE_EXT_COMMENT){
      //if (I > 0 && pszChars[I] == _T('/') && pszChars[I - 1] == _T('*')){
      if ((I > 1 && pszChars[I] == _T('/') && pszChars[I - 1] == _T('*') &&
           !bWasCommentStart) ||
           (I == 1 && pszChars[I] == _T('/') && pszChars[I - 1] == _T('*'))){

        cookie &= ~COOKIE_EXT_COMMENT;
        bRedefineBlock = true;
      }
      continue;
    }

    if (I > 0 && pszChars[I] == _T('/') && pszChars[I - 1] == _T('/')){
      DEFINE_BLOCK(I - 1, COLORINDEX_COMMENT);
      cookie |= COOKIE_COMMENT;
      break;
    }

    //  Preprocessor directive #....
    if (cookie & COOKIE_PREPROCESSOR){
      if (I > 0 && pszChars[I] == _T('*') && pszChars[I - 1] == _T('/')){
        DEFINE_BLOCK(I - 1, COLORINDEX_COMMENT);
        //bRedefineBlock = true; // let work in redefine block
        cookie |= COOKIE_EXT_COMMENT;
      }
      continue;
    }

    //  Normal text
    if(pszChars[I] == _T('"')){
      DEFINE_BLOCK(I, COLORINDEX_STRING);
      //bRedefineBlock = true; // let work in redefine block
      cookie |= COOKIE_STRING;
      continue;
    }
    if (pszChars[I] == _T('\'')){
      DEFINE_BLOCK(I, COLORINDEX_CHARACTER);
      //bRedefineBlock = true; // let work in redefine block
      cookie |= COOKIE_CHAR;
      continue;
    }
    if (I > 0 && pszChars[I] == _T('*') && pszChars[I - 1] == _T('/')){
      DEFINE_BLOCK(I - 1, COLORINDEX_COMMENT);
      //bRedefineBlock = true; // let work in redefine block
      cookie |= COOKIE_EXT_COMMENT;
      continue;
    }

    bWasCommentStart = false;

    if (bFirstChar){
      if (pszChars[I] == _T('#')){
        DEFINE_BLOCK(I, COLORINDEX_PREPROCESSOR);
        cookie |= COOKIE_PREPROCESSOR;
        continue;
      }
      if (!_istspace(pszChars[I]))
        bFirstChar = FALSE;
    }

    if (pBuf == NULL)
      continue;  //  We don't need to extract keywords,

    //  for faster parsing skip the rest of loop
    if (xisalnum (pszChars[I]) ||
      (
        I > 0 &&
        pszChars[I] == _T('.') &&
        !xisalpha (pszChars[I - 1]) &&
        !xisalpha (pszChars[I + 1])
      ))
    {
      if (nIdentBegin == -1)
        nIdentBegin = I;
    }
    else{
      if (nIdentBegin >= 0){
        if (IsCppKeyword(pszChars + nIdentBegin, I - nIdentBegin)){
          DEFINE_BLOCK(nIdentBegin, COLORINDEX_KEYWORD);
        }
//        else if (IsUser1Keyword (pszChars + nIdentBegin, I - nIdentBegin)){
//          DEFINE_BLOCK (nIdentBegin, COLORINDEX_USER1);
//        }
        else if (IsCppNumber(pszChars + nIdentBegin, I - nIdentBegin)){
          DEFINE_BLOCK(nIdentBegin, COLORINDEX_NUMBER);
        }
        else{
          bool bFunction = false;
          for (int j = I; j < len; j++){
            if(!_istspace(pszChars[j])){
              if(pszChars[j] == _T('('))
                bFunction = true;
              break;
            }
          }
          if(bFunction){
            DEFINE_BLOCK(nIdentBegin, COLORINDEX_FUNCNAME);
          }
        }
        bRedefineBlock = true;
        bDecIndex = true;
        nIdentBegin = -1;
      }
    }
  }

  if (nIdentBegin >= 0){
    if (IsCppKeyword(pszChars + nIdentBegin, I - nIdentBegin)){
      DEFINE_BLOCK(nIdentBegin, COLORINDEX_KEYWORD);
    }
//    else if (IsUser1Keyword (pszChars + nIdentBegin, I - nIdentBegin)){
//      DEFINE_BLOCK (nIdentBegin, COLORINDEX_USER1);
//    }
    else if (IsCppNumber(pszChars + nIdentBegin, I - nIdentBegin)){
      DEFINE_BLOCK(nIdentBegin, COLORINDEX_NUMBER);
    }
    else{
      bool bFunction = false;
      for(int j = I; j < len; j++){
        if(!_istspace(pszChars[j])){
          if(pszChars[j] == _T('('))
            bFunction = true;
          break;
        }
      }
      if (bFunction){
        DEFINE_BLOCK (nIdentBegin, COLORINDEX_FUNCNAME);
      }
    }
  }
  if (pszChars[len - 1] != _T('\\'))
    cookie &= COOKIE_EXT_COMMENT;
  return cookie;
}
//
