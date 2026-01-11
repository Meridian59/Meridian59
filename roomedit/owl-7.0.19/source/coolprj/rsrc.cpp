///////////////////////////////////////////////////////////////////////////
//
//  Copyright:  Ferdinand Prantl, portions by Stcherbatchenko Andrei
//  E-mail:     prantl@ff.cuni.cz
//
//  Windows resources syntax highlighing definition
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
static LPCTSTR s_apszRsrcKeywordList[] =
{
    _T("CURSOR"),
    _T("BITMAP"),
    _T("ICON"),
    _T("MENU"),
    _T("DIALOG"),
    _T("STRING"),
    _T("FONTDIR"),
    _T("FONT"),
    _T("ACCELERATOR"),
    _T("RCDATA"),
    _T("MESSAGETABLE"),
    _T("VERSION"),
    _T("DLGINCLUDE"),
    _T("PLUGPLAY"),
    _T("VXD"),
    _T("ANICURSOR"),
    _T("ANIICON"),
    _T("HTML"),
    _T("TOOLBAR"),
    _T("BITMAPEX"),
    _T("MENUEX"),
    _T("DIALOGEX"),
    _T("VERSIONINFO"),

    _T("FILEVERSION"),
    _T("PRODUCTVERSION"),
    _T("FILEFLAGSMASK"),
    _T("FILEFLAGS"),
    _T("FILEOS"),
    _T("FILETYPE"),
    _T("FILESUBTYPE"),
    _T("BLOCK"),
    _T("VALUE"),

    _T("POPUP"),
    _T("MENUITEM"),
    _T("SEPARATOR"),
    _T("GRAYED"),
    _T("BUTTON"),
    _T("BEGIN"),
    _T("END"),
    _T("FIXED"),
    _T("IMPURE"),
    _T("PRELOAD"),
    _T("DISCARDABLE"),
    _T("CONTROL"),
    _T("NOT"),
    _T("STYLE"),
    _T("CAPTION"),
    _T("FONT"),
    _T("DLGINCLUDE"),
    _T("CLASS"),
    _T("EXSTYLE"),
    _T("CHARACTERISTICS"),
    _T("VERSION"),
    _T("LANGUAGE"),
    _T("RADIOBUTTON"),
    _T("CHECKBOX"),
    _T("DEFPUSHBUTTON"),
    _T("PUSHBUTTON"),
    _T("EDITTEXT"),
    _T("GROUPBOX"),
    _T("RTEXT"),
    _T("CTEXT"),
    _T("LTEXT"),
    _T("LISTBOX"),
    _T("COMBOBOX"),
    _T("SCROLLBAR"),
    _T("AUTO3STATE"),
    _T("AUTOCHECKBOX"),
    _T("AUTORADIOBUTTON"),
    _T("STATE3"),
    _T("USERBUTTON"),
    _T("COMBOBOXEX"),

    _T("BS_PUSHBUTTON"),
    _T("BS_DEFPUSHBUTTON"),
    _T("BS_CHECKBOX"),
    _T("BS_AUTOCHECKBOX"),
    _T("BS_RADIOBUTTON"),
    _T("BS_3STATE"),
    _T("BS_AUTO3STATE"),
    _T("BS_GROUPBOX"),
    _T("BS_USERBUTTON"),
    _T("BS_AUTORADIOBUTTON")
    _T("BS_PUSHBOX"),
    _T("BS_OWNERDRAW"),
    _T("BS_LEFTTEXT"),
    _T("SBS_HORZ"),
    _T("SBS_VERT"),
    _T("SBS_TOPALIGN"),
    _T("SBS_LEFTALIGN"),
    _T("SBS_BOTTOMALIGN"),
    _T("SBS_RIGHTALIGN"),
    _T("SBS_SIZEBOXTOPLEFTALIGN"),
    _T("SBS_SIZEBOXBOTTOMRIGHTALIGN"),
    _T("SBS_SIZEBOX"),
    _T("ES_LEFT"),
    _T("ES_CENTER"),
    _T("ES_RIGHT"),
    _T("ES_MULTILINE"),
    _T("ES_UPPERCASE"),
    _T("ES_LOWERCASE"),
    _T("ES_PASSWORD"),
    _T("ES_AUTOVSCROLL"),
    _T("ES_AUTOHSCROLL"),
    _T("ES_NOHIDESEL"),
    _T("ES_OEMCONVERT"),
    _T("ES_READONLY"),
    _T("SS_LEFT"),
    _T("SS_CENTER"),
    _T("SS_RIGHT"),
    _T("SS_ICON"),
    _T("SS_BLACKRECT"),
    _T("SS_GRAYRECT"),
    _T("SS_WHITERECT"),
    _T("SS_BLACKFRAME"),
    _T("SS_GRAYFRAME"),
    _T("SS_WHITEFRAME"),
    _T("SS_USERITEM"),
    _T("SS_SIMPLE"),
    _T("SS_LEFTNOWORDWRAP"),
    _T("SS_OWNERDRAW"),
    _T("SS_BITMAP"),
    _T("SS_ENHMETAFILE"),
    _T("SS_ETCHEDHORZ"),
    _T("SS_ETCHEDVERT"),
    _T("SS_ETCHEDFRAME"),
    _T("SS_NOPREFIX"),
    _T("LBS_NOTIFY"),
    _T("LBS_SORT"),
    _T("LBS_NOREDRAW"),
    _T("LBS_MULTIPLESEL"),
    _T("LBS_OWNERDRAWFIXED"),
    _T("LBS_OWNERDRAWVARIABLE"),
    _T("LBS_HASSTRINGS"),
    _T("LBS_USETABSTOPS"),
    _T("LBS_NOINTEGRALHEIGHT"),
    _T("LBS_MULTICOLUMN"),
    _T("LBS_WANTKEYBOARDINPUT"),
    _T("LBS_EXTENDEDSEL"),
    _T("LBS_DISABLENOSCROLL"),
    _T("LBS_NODATA"),
    _T("LBS_NOSEL"),
    _T("CBS_SIMPLE"),
    _T("CBS_DROPDOWN"),
    _T("CBS_DROPDOWNLIST"),
    _T("CBS_OWNERDRAWFIXED"),
    _T("CBS_OWNERDRAWVARIABLE"),
    _T("CBS_AUTOHSCROLL"),
    _T("CBS_OEMCONVERT"),
    _T("CBS_SORT"),
    _T("CBS_HASSTRINGS"),
    _T("CBS_NOINTEGRALHEIGHT"),
    _T("CBS_DISABLENOSCROLL"),
    _T("CBS_UPPERCASE"),
    _T("CBS_LOWERCASE"),
    _T("DS_ABSALIGN"),
    _T("DS_SYSMODAL"),
    _T("DS_LOCALEDIT"),
    _T("DS_MODALFRAME"),
    _T("DS_NOIDLEMSG"),
    _T("WS_MINIMIZEBOX"),
    _T("WS_MAXIMIZEBOX"),
    _T("DS_SETFOREGROUND"),
    _T("DS_3DLOOK"),
    _T("DS_FIXEDSYS"),
    _T("DS_NOFAILCREATE"),
    _T("DS_CONTROL"),
    _T("DS_CENTER"),
    _T("DS_CENTERMOUSE"),
    _T("DS_CONTEXTHELP"),
    _T("WS_POPUP"),
    _T("WS_CHILD"),
    _T("WS_MINIMIZE"),
    _T("WS_VISIBLE"),
    _T("WS_DISABLED"),
    _T("WS_CLIPSIBLINGS"),
    _T("WS_CLIPCHILDREN"),
    _T("WS_MAXIMIZE"),
    _T("WS_CAPTION"),
    _T("WS_BORDER"),
    _T("WS_DLGFRAME"),
    _T("WS_VSCROLL"),
    _T("WS_HSCROLL"),
    _T("WS_SYSMENU"),
    _T("WS_THICKFRAME"),
    _T("WS_GROUP"),
    _T("WS_TABSTOP"),
    _T("WS_EX_OVERLAPPEDWINDOW"),
    _T("WS_EX_PALETTEWINDOW"),
    _T("WS_EX_DLGMODALFRAME"),
    _T("WS_EX_NOPARENTNOTIFY"),
    _T("WS_EX_TOPMOST"),
    _T("WS_EX_ACCEPTFILES"),
    _T("WS_EX_TRANSPARENT"),
    _T("WS_EX_TOOLWINDOW"),
    _T("WS_EX_WINDOWEDGE"),
    _T("WS_EX_CLIENTEDGE"),
    _T("WS_EX_CONTEXTHELP"),
    _T("WS_EX_RIGHT"),
    _T("WS_EX_LEFT"),
    _T("WS_EX_RTLREADING"),
    _T("WS_EX_LTRREADING"),
    _T("WS_EX_LEFTSCROLLBAR"),
    _T("WS_EX_RIGHTSCROLLBAR"),
    _T("WS_EX_CONTROLPARENT"),
    _T("WS_EX_STATICEDGE"),
    _T("WS_EX_APPWINDOW"),
    _T("LANG_NEUTRAL"),
    _T("LANG_ALBANIAN"),
    _T("LANG_ARABIC"),
    _T("LANG_BASQUE"),
    _T("LANG_BULGARIAN"),
    _T("LANG_CATALAN"),
    _T("LANG_CHINESE"),
    _T("LANG_CZECH"),
    _T("LANG_DANISH"),
    _T("LANG_DUTCH"),
    _T("LANG_ENGLISH"),
    _T("LANG_FINNISH"),
    _T("LANG_FRENCH"),
    _T("LANG_GERMAN"),
    _T("LANG_GREEK"),
    _T("LANG_HEBREW"),
    _T("LANG_HUNGARIAN"),
    _T("LANG_ICELANDIC"),
    _T("LANG_ITALIAN"),
    _T("LANG_JAPANESE"),
    _T("LANG_KOREAN"),
    _T("LANG_NORWEGIAN"),
    _T("LANG_POLISH"),
    _T("LANG_PORTUGUESE"),
    _T("LANG_ROMANIAN"),
    _T("LANG_RUSSIAN"),
    _T("LANG_SERBIAN"),
    _T("LANG_SLOVAK"),
    _T("LANG_SPANISH"),
    _T("LANG_SWEDISH"),
    _T("LANG_THAI"),
    _T("LANG_TURKISH"),
    _T("LANG_URDU"),
    _T("SUBLANG_DEFAULT"),
    _T("SUBLANG_NEUTRAL"),
    _T("SUBLANG_CHINESE_SIMPLIFIED"),
    _T("SUBLANG_CHINESE_TRADITIONAL"),
    _T("SUBLANG_DUTCH"),
    _T("SUBLANG_DUTCH_BELGIAN"),
    _T("SUBLANG_ENGLISH_US"),
    _T("SUBLANG_ENGLISH_UK"),
    _T("SUBLANG_ENGLISH_AUS"),
    _T("SUBLANG_ENGLISH_CAN"),
    _T("SUBLANG_FRENCH"),
    _T("SUBLANG_FRENCH_BELGIAN"),
    _T("SUBLANG_FRENCH_CANADIAN"),
    _T("SUBLANG_FRENCH_SWISS"),
    _T("SUBLANG_GERMAN"),
    _T("SUBLANG_GERMAN_SWISS"),
    _T("SUBLANG_ITALIAN"),
    _T("SUBLANG_ITALIAN_SWISS"),
    _T("SUBLANG_NORWEGIAN_BOKMAL"),
    _T("SUBLANG_NORWEGIAN_NYNORSK"),
    _T("SUBLANG_PORTUGUESE"),
    _T("SUBLANG_PORTUGUESE_BRAZILIAN"),
    _T("SUBLANG_SERBIAN_CYRILLIC"),
    _T("SUBLANG_SERBIAN_LATIN"),
    _T("SUBLANG_SPANISH"),
    _T("SUBLANG_SPANISH_MEXICAN"),
    _T("SUBLANG_SPANISH_MODERN"),

    _T("VIRTKEY"),
    _T("ASCII"),
    _T("ALT"),
    _T("SHIFT"),
    _T("NOINVERT"),
//    _T("CONTROL"),

    _T("VK_BACK"),
    _T("VK_TAB"),
    _T("VK_CLEAR"),
    _T("VK_RETURN"),
    _T("VK_SHIFT"),
    _T("VK_CONTROL"),
    _T("VK_MENU"),
    _T("VK_PAUSE"),
    _T("VK_CAPITAL"),
    _T("VK_KANA"),
    _T("VK_HANGEUL"),
    _T("VK_HANGUL"),
    _T("VK_JUNJA"),
    _T("VK_FINAL"),
    _T("VK_HANJA"),
    _T("VK_KANJI"),
    _T("VK_ESCAPE"),
    _T("VK_CONVERT"),
    _T("VK_NONCONVERT"),
    _T("VK_ACCEPT"),
    _T("VK_MODECHANGE"),
    _T("VK_SPACE"),
    _T("VK_PRIOR"),
    _T("VK_NEXT"),
    _T("VK_END"),
    _T("VK_HOME"),
    _T("VK_LEFT"),
    _T("VK_UP"),
    _T("VK_RIGHT"),
    _T("VK_DOWN"),
    _T("VK_SELECT"),
    _T("VK_PRINT"),
    _T("VK_EXECUTE"),
    _T("VK_SNAPSHOT"),
    _T("VK_INSERT"),
    _T("VK_DELETE"),
    _T("VK_HELP"),

    _T("VK_NUMPAD0"),
    _T("VK_NUMPAD1"),
    _T("VK_NUMPAD2"),
    _T("VK_NUMPAD3"),
    _T("VK_NUMPAD4"),
    _T("VK_NUMPAD5"),
    _T("VK_NUMPAD6"),
    _T("VK_NUMPAD7"),
    _T("VK_NUMPAD8"),
    _T("VK_NUMPAD9"),
    _T("VK_MULTIPLY"),
    _T("VK_ADD"),
    _T("VK_SEPARATOR"),
    _T("VK_SUBTRACT"),
    _T("VK_DECIMAL"),
    _T("VK_DIVIDE"),
    _T("VK_F1"),
    _T("VK_F2"),
    _T("VK_F3"),
    _T("VK_F4"),
    _T("VK_F5"),
    _T("VK_F6"),
    _T("VK_F7"),
    _T("VK_F8"),
    _T("VK_F9"),
    _T("VK_F10"),
    _T("VK_F11"),
    _T("VK_F12"),
    _T("VK_F13"),
    _T("VK_F14"),
    _T("VK_F15"),
    _T("VK_F16"),
    _T("VK_F17"),
    _T("VK_F18"),
    _T("VK_F19"),
    _T("VK_F20"),
    _T("VK_F21"),
    _T("VK_F22"),
    _T("VK_F23"),
    _T("VK_F24"),
    NULL
};

static LPCTSTR s_apszUser1KeywordList[] =
  {
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
IsRsrcKeyword (LPCTSTR pszChars, int nLength)
{
  return IsXKeyword (s_apszRsrcKeywordList, pszChars, nLength);
}

static BOOL
IsUser1Keyword (LPCTSTR pszChars, int nLength)
{
  return IsXKeyword (s_apszUser1KeywordList, pszChars, nLength);
}

static BOOL
IsRsrcNumber (LPCTSTR pszChars, int nLength)
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
struct TRsrcSyntaxParser: public TSyntaxParser {
  public:
    TRsrcSyntaxParser(TCoolTextWnd* parent):TSyntaxParser(parent){}
    uint32 ParseLine(uint32 cookie, int index, TCoolTextWnd::TTextBlock* buf, int& items);
};
//
_COOLEDFUNC(TSyntaxParser*) RsrcParserCreator(TCoolTextWnd* parent)
{
  return  new TRsrcSyntaxParser(parent);
}
//
uint32 TRsrcSyntaxParser::ParseLine(uint32 dwCookie, int nLineIndex,
                                   TCoolTextWnd::TTextBlock* pBuf,
                                   int& nActualItems)
{
  int nLength = Parent->GetLineLength (nLineIndex);
  if (nLength <= 1)
    return dwCookie & COOKIE_EXT_COMMENT;

  LPCTSTR pszChars = GetLineText (nLineIndex);
  BOOL bFirstChar = (dwCookie & ~COOKIE_EXT_COMMENT) == 0;
  BOOL bRedefineBlock = TRUE;
  BOOL bWasCommentStart = FALSE;
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
          if (nPos >= nLength)
            break;
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

      //  Extended comment /*....*/
      if (dwCookie & COOKIE_EXT_COMMENT)
        {
          // if (I > 0 && pszChars[I] == '/' && pszChars[I - 1] == '*')
          if ((I > 1 && pszChars[I] == '/' && pszChars[I - 1] == '*' /*&& pszChars[I - 2] != '/'*/ && !bWasCommentStart) || (I == 1 && pszChars[I] == '/' && pszChars[I - 1] == '*'))
            {
              dwCookie &= ~COOKIE_EXT_COMMENT;
              bRedefineBlock = TRUE;
            }
          bWasCommentStart = FALSE;
          continue;
        }

      if (I > 0 && pszChars[I] == '/' && pszChars[I - 1] == '/')
        {
          DEFINE_BLOCK (I - 1, COLORINDEX_COMMENT);
          dwCookie |= COOKIE_COMMENT;
          break;
        }

      //  Preprocessor directive #....
      if (dwCookie & COOKIE_PREPROCESSOR)
        {
          if (I > 0 && pszChars[I] == '*' && pszChars[I - 1] == '/')
            {
              DEFINE_BLOCK (I - 1, COLORINDEX_COMMENT);
              dwCookie |= COOKIE_EXT_COMMENT;
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
      if (I > 0 && pszChars[I] == '*' && pszChars[I - 1] == '/')
        {
          DEFINE_BLOCK (I - 1, COLORINDEX_COMMENT);
          dwCookie |= COOKIE_EXT_COMMENT;
          bWasCommentStart = TRUE;
          continue;
        }

      bWasCommentStart = FALSE;

      if (bFirstChar)
        {
          if (pszChars[I] == '#')
            {
              DEFINE_BLOCK (I, COLORINDEX_PREPROCESSOR);
              dwCookie |= COOKIE_PREPROCESSOR;
              continue;
            }
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
              if (IsRsrcKeyword (pszChars + nIdentBegin, I - nIdentBegin))
                {
                  DEFINE_BLOCK (nIdentBegin, COLORINDEX_KEYWORD);
                }
              else if (IsUser1Keyword (pszChars + nIdentBegin, I - nIdentBegin))
                {
                  DEFINE_BLOCK (nIdentBegin, COLORINDEX_CUSTOM0);
                }
              else if (IsRsrcNumber (pszChars + nIdentBegin, I - nIdentBegin))
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
      if (IsRsrcKeyword (pszChars + nIdentBegin, I - nIdentBegin))
        {
          DEFINE_BLOCK (nIdentBegin, COLORINDEX_KEYWORD);
        }
      else if (IsUser1Keyword (pszChars + nIdentBegin, I - nIdentBegin))
        {
          DEFINE_BLOCK (nIdentBegin, COLORINDEX_CUSTOM0);
        }
      else if (IsRsrcNumber (pszChars + nIdentBegin, I - nIdentBegin))
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
