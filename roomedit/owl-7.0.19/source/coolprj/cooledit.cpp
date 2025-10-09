//----------------------------------------------------------------------------
// ObjectWindow - OWL NExt
// Copyright 1999. Yura Bidus. All Rights reserved.
//
// For more information, including license details, see
// http://owlnext.sourceforge.net
//
//  OVERVIEW
//  ~~~~~~~~
//  Syntax coloring text editor.
//----------------------------------------------------------------------------
#include <coolprj/pch.h>
#pragma hdrstop

#include <owl/template.h>
#include <owl/file.h>
#include <owl/filename.h>
#include <owl/scroller.h>
#include <owl/celarray.h>
#include <owl/glyphbtn.h>
#include <owl/uimetric.h>
#include <owl/configfl.h>
#include <owl/checkbox.h>
#include <owl/radiobut.h>
#include <owl/memcbox.h>
#include <owl/editfile.rh>
#include <coolprj/cooledit.rh>
#include <coolprj/cooledit.h>

#include <algorithm>

using namespace owl;
using namespace std;

// Scroller->AutoOrg == true
//#define USE_AUTOORG

//------------------------------------------------------------------------------
void __Trace(LPCTSTR lpszFormat, ...);
//------------------------------------------------------------------------------
inline void __NoTrace(LPCTSTR /*lpszFormat*/, ...){}
//------------------------------------------------------------------------------

#if 0
#define _TRACE __Trace
#else
#define _TRACE __NoTrace
#endif


//  Line allocation granularity
#define CHAR_ALIGN            16
#define ALIGN_BUF_SIZE(size)  (((size)/CHAR_ALIGN)*CHAR_ALIGN+CHAR_ALIGN);
#define TAB_CHARACTER         _T('\xBB')
#define SPACE_CHARACTER       _T('\xB7')

#define DEFAULT_PRINT_MARGIN    1000        //  10 millimeters
//#define SMOOTH_SCROLL_FACTOR    6
#define COOL_TIMER_DEBOUNCE      1001

static const _TCHAR crlf[]  = _T("\r\n");
static const _TCHAR chTab   = _T('\t');
static const _TCHAR szTab[] = _T("\t");

#ifdef WIN32
const int MSB=0x80000000;
#else
const int MSB=0x8000;
#endif

#define NULL_POS TEditPos(-1,-1)

//const int ciDigitsInLine = 5;


static const _TCHAR*  streamSelectionId = _T("COOL000");
static const _TCHAR*  lineSelectionId   = _T("COOL001");
static const _TCHAR*  columnSelection   = _T("COOL002");
const  int            IdSelSize         = 7;

static const _TCHAR*  configSection     = _T("SyntaxFormat");
static const _TCHAR*  configBaseFont    = _T("BaseFont");
static const _TCHAR*  configFormatIdx   = _T("FormatIdx%d");
static const _TCHAR*  configOWLNExt     = _T("OWL NExt");

//--------------------------------------------------------
// class TEditRange
// ~~~~~ ~~~~~~~~~~
//
void TEditRange::Normalize()
{
  if(srow > erow){
    std::swap(srow, erow);
    std::swap(scol, ecol);
  }
  if(srow == erow && scol > ecol)
    std::swap(scol, ecol);
}
//////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
//{{TCoolTextBuffer Implementation}}
//
TCoolTextBuffer::TCoolTextBuffer(TCrLfStyle style)
:
UndoList(0),
UndoCnt(0),
UndoSize(100),
RedoList(0),
RedoCnt(0),
RedoSize(100),
SyntaxArray(0),
Flags(bfCreateBackup),
CRLFMode(style)
{
  memset(&BaseFont, 0, sizeof(BaseFont));
  _tcscpy(BaseFont.lfFaceName, _T("Consolas"));
  BaseFont.lfHeight         = -13;
  BaseFont.lfWeight         = FW_NORMAL;
  BaseFont.lfItalic         = FALSE;
  BaseFont.lfCharSet        = DEFAULT_CHARSET;
  BaseFont.lfOutPrecision   = OUT_DEFAULT_PRECIS;
  BaseFont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
  BaseFont.lfQuality        = DEFAULT_QUALITY;
  BaseFont.lfPitchAndFamily = FIXED_PITCH;

  //InsertLine(_T(""));
}
//
TCoolTextBuffer::~TCoolTextBuffer()
{
  ClearUndoBuffer();
  ClearRedoBuffer();
  delete SyntaxArray;
}
//
void
TCoolTextBuffer::Clear(bool addempty)
{
  ClearUndoBuffer();
  ClearRedoBuffer();
  if(addempty)
    InsertLine(_T("")); // alwas first line
}
//
static int Flag2Index(uint32 flag)
{
  int index = 0;
  while ((flag & 1) == 0){
    flag = flag >> 1;
    index++;
    if (index == 32)
      return -1;
  }
  //  flag = flag & 0xFFFFFFFE;
  //  if (flag != 0)
  //    return -1;
  return index;
}
//-----------------------------------------------------------------------------
int
TCoolTextBuffer::GetLineWithFlag(uint32 flag)
{
  int index = ::Flag2Index(flag);
  if (index < 0){
    CHECK(false);   //  Invalid flag passed in
    return -1;
  }
  return FindLineWithFlag(flag);
}
//-----------------------------------------------------------------------------
int
TCoolTextBuffer::SetLineFlag(int line, uint32 flag, bool bSet,
               bool bRemoveFromPreviousLine /*= true*/)
{
  int index = ::Flag2Index(flag);
  if (index < 0){
    CHECK(false);   //  Invalid flag passed in
    return -1;
  }
  int retval = -1;
  if(line == -1){
    CHECK(!bSet);
    retval = line = FindLineWithFlag(flag);
    if (line == -1)
      return -1;
    bRemoveFromPreviousLine = false;
  }

  uint32 oldFlags = GetLineFlags(line);
  uint32 newFlags = oldFlags;
  if (bSet)
    newFlags = newFlags | flag;
  else
    newFlags = newFlags & ~flag;

  if(oldFlags != newFlags){
    if(bRemoveFromPreviousLine){
      int prevLine = FindLineWithFlag(flag);
      if (bSet){
        if (prevLine >= 0){
          CHECK((GetLineFlags(prevLine) & flag) != 0);
          SetLineFlags(prevLine, GetLineFlags(prevLine) & ~flag);
          retval = prevLine;
        }
      }
      else{
        CHECK(prevLine == line);
      }
    }
    SetLineFlags(line, newFlags);
  }
  return retval;
}
//-----------------------------------------------------------------------------
bool
TCoolTextBuffer::Undo(TEditPos* newPos)
{
  TUndoNode* node = UndoList;
  if(!node)
    return false;
  UndoList = UndoList->Next;
  bool retval = node->Undo(*this,newPos);
  delete node;
  UndoCnt--;
  return retval;
}
//-----------------------------------------------------------------------------
bool
TCoolTextBuffer::Redo(TEditPos* newPos)
{
  TRedoNode* node = RedoList;
  if(!node)
    return false;
  RedoList = RedoList->Next;
  bool retval = node->Redo(*this,newPos);
  delete node;
  RedoCnt--;
  return retval;
}
//-----------------------------------------------------------------------------
void
TCoolTextBuffer::AdjustUndoSize()
{
  if(UndoCnt >= UndoSize){
    TUndoNode* uList = UndoList;
    if(!uList || !uList->Next)
      return;
    while(uList->Next->Next)
      uList = uList->Next;
    delete uList->Next;
    uList->Next = 0;
    UndoCnt--;
  }
}
//-----------------------------------------------------------------------------
void
TCoolTextBuffer::AdjustRedoSize()
{
  if(RedoCnt >= RedoSize){
    TRedoNode* rList = RedoList;
    if(!rList || !rList->Next)
      return;
    while(rList->Next->Next)
      rList = rList->Next;
    delete rList->Next;
    rList->Next = 0;
    RedoCnt--;
  }
}
//-----------------------------------------------------------------------------
bool
TCoolTextBuffer::AddUndoNode(TUndoNode* node)
{
  PRECONDITION(node);
  node->Next = UndoList;
  UndoList = node;
  UndoCnt++;
  AdjustUndoSize();
  return true;
}
//-----------------------------------------------------------------------------
bool
TCoolTextBuffer::AddRedoNode(TRedoNode* node)
{
  PRECONDITION(node);
  node->Next = RedoList;
  RedoList = node;
  RedoCnt++;
  AdjustRedoSize();
  return true;
}
//-----------------------------------------------------------------------------
void
TCoolTextBuffer::ClearUndoBuffer()
{
  while(UndoList){
    TUndoNode* tmp = UndoList->Next;
    delete UndoList;
    UndoList = tmp;
  }
  UndoCnt = 0;
}
//-----------------------------------------------------------------------------
void
TCoolTextBuffer::ClearRedoBuffer()
{
  while(RedoList){
    TRedoNode* tmp = RedoList->Next;
    delete RedoList;
    RedoList = tmp;
  }
  RedoCnt = 0;
}
//-----------------------------------------------------------------------------
bool
TCoolTextBuffer::GetUndoDescription(_TCHAR* buffer, int len, TModule* module)
{
  if(UndoList)
    return UndoList->GetDescription(buffer, len, module);
  return LoadStringIndex(buffer, len, IDS_COOLUNDOSTRINGS, 0, _T('|'), module);
}
//-----------------------------------------------------------------------------
bool
TCoolTextBuffer::GetRedoDescription(_TCHAR* buffer, int len, TModule* module)
{
  if(RedoList)
    return RedoList->GetDescription(buffer, len, module);
  return LoadStringIndex(buffer, len, IDS_COOLREDOSTRINGS, 0, _T('|'), module);
}
//-----------------------------------------------------------------------------
bool
TCoolTextBuffer::LoadStringIndex(_TCHAR* buffer, int len, int resId,
                 int index, _TCHAR delimCh, TModule* module)
{
  if(!module)
    module = &owl::GetGlobalModule();
  TTmpBuffer<_TCHAR> __ClnObj(512);
  LPTSTR ptr = __ClnObj;
  if(!module->LoadString(resId, ptr, 512))
    return false;
  LPTSTR p = _tcschr(ptr, delimCh);
  int lastIdx = 0;
  bool found = false;
  do{
    if(p){
      if(lastIdx>=index){
        _TCHAR c = *p;
        *p = 0;
        _tcsncpy(buffer, ptr, len);
        found = true;
        *p = c;
        break;
      }
      ptr = p+1;
      p = _tcschr(ptr, delimCh);
      lastIdx++;
    }
    else if(lastIdx>=index){
      _tcsncpy(buffer, ptr, len);
      found = _tcslen(ptr) > 0;
    }
  }
  while(p);
  buffer[len-1] = 0;
  return found;
}
//-----------------------------------------------------------------------------
int
TCoolTextBuffer::FindNextBookmarkLine(int curLine)
{
  bool bWrapIt = true;
  uint32 flags = GetLineFlags(curLine);
  if ((flags & lfBookMarks) != 0)
    curLine++;

  int size = GetLineCount();
  for (;;){
    while (curLine < size){
      if ((GetLineFlags(curLine) & lfBookMarks) != 0)
        return curLine;
      // Keep going
      curLine++;
    }
    // End of text reached
    if (!bWrapIt)
      return -1;

    // Start from the beginning of text
    bWrapIt = false;
    curLine = 0;
  }
  //return -1;
}
//-----------------------------------------------------------------------------
int TCoolTextBuffer::FindPrevBookmarkLine(int curLine)
{
  bool bWrapIt = true;
  uint32 flags = GetLineFlags(curLine);
  if((flags & lfBookMarks) != 0)
    curLine--;

  int size = GetLineCount();
  for (;;){
    while (curLine >= 0){
      if ((GetLineFlags(curLine) & lfBookMarks) != 0)
        return curLine;
      // Keep moving up
      curLine--;
    }
    // Beginning of text reached
    if (!bWrapIt)
      return -1;

    // Start from the end of text
    bWrapIt = false;
    curLine = size - 1;
  }
}
//-----------------------------------------------------------------------------
static void
SetSyntaxNode(TCoolTextBuffer::TSyntaxDescr& node,
        TCoolTextBuffer::TSyntaxDescr* descr)
{
  // white Space
  if(descr){
    node.TxColor    = descr->TxColor;
    node.BkColor    = descr->BkColor;
    node.FontIndex  = descr->FontIndex;
  }
}
//-----------------------------------------------------------------------------
TCoolTextBuffer::TSyntaxDescr*
TCoolTextBuffer::GetDefSyntaxDescr(int index)
{
 static TSyntaxDescr array[] = 
 {
   TSyntaxDescr(TColor::Black, TColor::White, 0), // COLORINDEX_WHITESPACE

   // Normal Text
   //
   TSyntaxDescr(TColor::Black, TColor::White, 0), // COLORINDEX_NORMALTEXT

   // Margin
   //
   TSyntaxDescr(TColor::SysGrayText, TColor::Sys3dFace, 0), // COLORINDEX_MARGIN

   // Selected Text
   //
   TSyntaxDescr(TColor::Black, TColor{173, 214, 255}, 0), // COLORINDEX_SELECTION

   // Syntax Check
   //
   TSyntaxDescr(TColor::LtRed, TColor::White, 0), // COLORINDEX_SYNTAXCHECK

   // Syntax colors
   //
   TSyntaxDescr(TColor{16, 16, 128}, TColor::White, 0), // COLORINDEX_KEYWORD
   TSyntaxDescr(TColor{0, 128, 0}, TColor::White, 0), // COLORINDEX_COMMENT
   TSyntaxDescr(TColor::Black, TColor::White, 0), // COLORINDEX_NUMBER
   TSyntaxDescr(TColor::Black, TColor::White, 0), // COLORINDEX_OPERATOR
   TSyntaxDescr(TColor{163, 21, 21}, TColor::White, 0), // COLORINDEX_STRING
   TSyntaxDescr(TColor{163, 21, 21}, TColor::White, 0), // COLORINDEX_CHARACTER
   TSyntaxDescr(TColor{128, 128, 128}, TColor::White, 0), // COLORINDEX_PREPROCESSOR
   TSyntaxDescr(TColor::Black, TColor::White, 0), // COLORINDEX_FUNCNAME

   // Compiler/debugger colors
   //
   TSyntaxDescr(TColor{255, 0, 0}, TColor::White, 0), // COLORINDEX_ERROR
   TSyntaxDescr(TColor::Black, TColor(255, 238, 98), 0), // COLORINDEX_EXECUTION
   TSyntaxDescr(TColor::White, TColor{150, 58, 70}, 0), // COLORINDEX_BREAKPOINT
   TSyntaxDescr(TColor(255, 219, 163), TColor{150, 58, 70}, 0), // COLORINDEX_INVALIDBREAK

   // Expandable (custom elements are allowed)
   //
   TSyntaxDescr(TColor::Black, TColor::White, 0), // COLORINDEX_CUSTOM0
   TSyntaxDescr(TColor::Black, TColor::White, 0), // COLORINDEX_CUSTOM1
   TSyntaxDescr(TColor::Black, TColor::White, 0), // COLORINDEX_CUSTOM2
   TSyntaxDescr(TColor::Black, TColor::White, 0), // COLORINDEX_CUSTOM3
   TSyntaxDescr(TColor::Black, TColor::White, 0), // COLORINDEX_CUSTOM4
   TSyntaxDescr(TColor::Black, TColor::White, 0), // COLORINDEX_CUSTOM5
   TSyntaxDescr(TColor::Black, TColor::White, 0), // COLORINDEX_CUSTOM6
   TSyntaxDescr(TColor::Black, TColor::White, 0), // COLORINDEX_CUSTOM7
   TSyntaxDescr(TColor::Black, TColor::White, 0), // COLORINDEX_CUSTOM8
   TSyntaxDescr(TColor::Black, TColor::White, 0), // COLORINDEX_CUSTOM9
 };
  return index < COUNTOF(array) ? &array[index] : 0;
}
//-----------------------------------------------------------------------------
void
TCoolTextBuffer::BuildDefSyntaxDescr()
{
  if(!SyntaxArray)
    SyntaxArray = new TSyntaxDescr[COLORINDEX_LAST];

  for(int i = 0; i < COLORINDEX_LAST; i++)
    SetSyntaxNode(SyntaxArray[i], GetDefSyntaxDescr(i));
}
//-----------------------------------------------------------------------------
void
TCoolTextBuffer::SetSyntaxDescr(int index, const TSyntaxDescr& node)
{
  if(!SyntaxArray)
    SyntaxArray = new TSyntaxDescr[COLORINDEX_LAST];
  SyntaxArray[index] = node;
}
//-----------------------------------------------------------------------------
void
TCoolTextBuffer::SetSyntaxArray(TSyntaxDescr* array)
{
  PRECONDITION(array);
  delete SyntaxArray;
  SyntaxArray = array;
}
//-----------------------------------------------------------------------------
void TCoolTextBuffer::SaveSyntaxDescr(TConfigFile& file)
{
  PRECONDITION(SyntaxDescrExist());

  TConfigFileSection section(file,configSection);
  // 1. Save Font Info
  section.WriteFont(configBaseFont, BaseFont);

  // 2. Write color data + special flag, so WriteColor not applicable
  // maybe better WriteInteger???
  _TCHAR buffer[40];
  for(int i = 0; i < COLORINDEX_LAST; i++){
    wsprintf(buffer, configFormatIdx, i);
    section.WriteData(buffer, (void*)&(SyntaxArray[i]), sizeof(SyntaxArray[i]));
  }
}
//-----------------------------------------------------------------------------
void
TCoolTextBuffer::RestoreSyntaxDescr(TConfigFile& file)
{
  if(!SyntaxArray)
    SyntaxArray = new TSyntaxDescr[COLORINDEX_LAST];
  if(!file.SectionExists(configSection))
    BuildDefSyntaxDescr();
  else{

    TConfigFileSection section(file,configSection);
    // 1. Restore Font Info
    LOGFONT lf;
    if(section.ReadFont(configBaseFont, lf))
      SetFont(lf);

    // 2. Read all Syntax format items
    _TCHAR buffer[40];
    TSyntaxDescr synVal;

    for(int i = 0; i < COLORINDEX_LAST; i++){
      wsprintf(buffer, configFormatIdx, i);
      if(section.ReadData(buffer, (void*)&synVal, sizeof(synVal)))
        SyntaxArray[i] = synVal;
    }
  }
}
//-----------------------------------------------------------------------------
//
// Similar to strstr(), but is case sensitive or insensitive, uses Windows
// string functions to work with different language drivers
//
static LPCTSTR
strstrcd(LPCTSTR str1, LPCTSTR str2, bool caseSens, uint len)
{
  PRECONDITION(str1 && str2);
  int len2 = static_cast<int>(_tcslen(str2));
  LPTSTR p = (LPTSTR)str1;
  LPCTSTR endp = str1 + len - len2 + 1;

  if (caseSens)
    while (p < endp) {
      _TCHAR c = p[len2];            // must term p to match str2 len
      p[len2] = 0;                 // for _tcscmp to work.
      if (_tcscmp(p, str2) == 0) {
        p[len2] = c;
        return p;
      }
      p[len2] = c;
#if defined(BI_DBCS_SUPPORT) && !defined(UNICODE)
      LPTSTR p2 = ::AnsiNext(p);
      if (p2 <= p)
        break;
      p = p2;
#else
      p++;
#endif
    }
  else
    while (p < endp) {
      TCHAR c = p[len2];
      p[len2] = 0;
      if (_tcsicmp(p, str2) == 0) {
        p[len2] = c;
        return p;
      }
      p[len2] = c;
#if defined(BI_DBCS_SUPPORT) && !defined(UNICODE)
      LPTSTR p2 = ::AnsiNext(p);
      if (p2 <= p)
        break;
      p = p2;
#else
      p++;
#endif
    }
    return 0;
}
//-----------------------------------------------------------------------------
// Similar to strstrcd(), but searches backwards. Needs the length of str1
// to know where to start.
//
static LPCTSTR
strrstrcd(LPCTSTR str1, uint len1, LPCTSTR str2, bool caseSens)
{
  PRECONDITION(str1 && str2);
  int len2 = static_cast<int>(_tcslen(str2));
  LPTSTR p = (LPTSTR)(str1 + len1 - len2);

#if defined(BI_DBCS_SUPPORT) && !defined(UNICODE)
  if (p >= str1)
    p = ::AnsiPrev(str1, p+1);
#endif
  if (caseSens)
    while (p >= str1) {
      TCHAR c = p[len2];            // must term p to match str2 len
      p[len2] = 0;                 // for _tcscmp to work.
      if (_tcscmp(p, str2) == 0) {
        p[len2] = c;
        return p;
      }
      p[len2] = c;
#if defined(BI_DBCS_SUPPORT) && !defined(UNICODE)
      LPTSTR p2 = ::AnsiPrev(str1, p);
      if (p2 >= p)
        break;
      p = p2;
#else
      p--;
#endif
    }
  else
    while (p >= str1) {
      TCHAR c = p[len2];
      p[len2] = 0;
      if (_tcsicmp(p, str2) == 0) {
        p[len2] = c;
        return p;
      }
      p[len2] = c;
#if defined(BI_DBCS_SUPPORT) && !defined(UNICODE)
      LPTSTR p2 = ::AnsiPrev(str1, p);
      if (p2 >= p)
        break;
      p = p2;
#else
      p--;
#endif
    }
    return 0;
}
//-----------------------------------------------------------------------------
TEditPos
TCoolTextBuffer::Search(const TEditRange& _searchRange, LPCTSTR text,
            bool caseSensitive, bool wholeWord, bool up)
{
  if (!text || !text[0])
    return TEditPos();

  TEditRange searchRange(_searchRange);
  if(!searchRange.Valid()){
    // if not valid -> all file
    searchRange.scol = 0;
    searchRange.srow = 0;
    searchRange.erow = GetLineCount()-1;
    searchRange.ecol = GetLineLength(searchRange.srow);
  }

  int textLen = static_cast<int>(_tcslen(text));

  // Lock the text buffer to get the pointer, and search thru it for the text
  //
  LPCTSTR pos = 0;
  int lineIndex = up ? searchRange.erow : searchRange.srow;
  int posIndex  = searchRange.scol;
  LPTSTR buffer = GetLineText(lineIndex);
  uint bufLen    = GetLineLength(lineIndex);

  while(true){
    if(up){
      if(posIndex < searchRange.srow){
        lineIndex--;
        if(lineIndex < searchRange.srow)
          break;
        buffer    = GetLineText(lineIndex);
        bufLen    = posIndex  = GetLineLength(lineIndex);
      }
      pos = strrstrcd(buffer+posIndex, posIndex, text, caseSensitive);
    }
    else{
      if(posIndex > (int)bufLen){
        lineIndex++;
        if(lineIndex>=GetLineCount() || lineIndex>=searchRange.erow)
          break;
        buffer    = GetLineText(lineIndex);
        bufLen    = GetLineLength(lineIndex);
        posIndex  = 0;
      }
      pos = strstrcd(buffer+posIndex, text, caseSensitive, bufLen-posIndex);
    }

    // If whole-word matching is enabled and we have a match so far, then make
    // sure the match is on word boundaries.
    //
    if (wholeWord && pos){
#if defined(BI_DBCS_SUPPORT)
      LPTSTR prevPos;
      if (pos > buffer)
        prevPos = ::AnsiPrev(buffer, pos);

      if (pos > buffer && _istalnum((_TCHAR)*prevPos) || // Match is in preceding word
        textLen < _tcslen(pos) && _istalnum((uint)pos[textLen])) {
          if(up)
            posIndex = (uint)(prevPos - buffer) + _tcslen(text);
          else
            posIndex = (uint)(::AnsiNext(pos) - buffer);
          continue;  // Skip this match and keep searching
      }
#else
      if ((pos > buffer && _istalnum(pos[-1])) || // Match is in preceding word
        (textLen < static_cast<int>(_tcslen(pos)) && _istalnum(pos[textLen])))
      {
          posIndex = (uint)(pos-buffer) + !up;
          continue;  // Skip this match and keep searching
      }
#endif
    }
    if(pos)
      break;
    else
      posIndex = up ? -1 : GetLineLength(lineIndex)+1;
  }

  // If we've got a match, select that text, cleanup & return.
  //
  if(pos)
    return TEditPos(static_cast<int>(pos - buffer), lineIndex);
  return TEditPos();
}
//-----------------------------------------------------------------------------
//
//
// class TCoolTextBuffer::TLineInfo
// ~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~
//
TCoolTextBuffer::TLineInfo::TLineInfo(LPCTSTR text, int len/* = -1*/)
:
Text(0),
Length(0),
MaxChar(0),
Flags(0)
{
  SetText(text, len);
}
//-----------------------------------------------------------------------------
TCoolTextBuffer::TLineInfo::~TLineInfo()
{
  delete [] Text;
}
//-----------------------------------------------------------------------------
void TCoolTextBuffer::TLineInfo::SetText(LPCTSTR text, int len)
{
  if(len == -1){
    if (!text)
      Length = 0;
    else
      Length = static_cast<int>(_tcslen(text));
  }
  else
    Length = len;

  int maxChar = ALIGN_BUF_SIZE(Length);

  CHECK(maxChar >= Length);

  if (maxChar > MaxChar){
    delete[] Text;
    MaxChar = maxChar;
    Text = new _TCHAR[MaxChar+1];
    CHECK(Text);
  }

  if (Length > 0)
    memcpy(Text, text, sizeof(_TCHAR) * Length);
}
//-----------------------------------------------------------------------------
bool
TCoolTextBuffer::TLineInfo::Append(LPCTSTR text, int len /*= -1*/)
{
  if (len == -1){
    if (text == NULL)
      return false;
    len = static_cast<int>(_tcslen(text));
  }

  if (len == 0)
    return false;

  int bufNeeded = Length + len;
  if (bufNeeded > MaxChar){
    MaxChar = ALIGN_BUF_SIZE(bufNeeded);
    CHECK(MaxChar >= Length + len);
    TCHAR *newBuf = new TCHAR[MaxChar];
    if (Length > 0)
      memcpy(newBuf, Text, sizeof(TCHAR) * Length);
    delete[] Text;
    Text = newBuf;
  }
  memcpy(Text + Length, text, sizeof(TCHAR) * len);
  Length += len;
  CHECK(Length <= MaxChar);
  return true;
}
//-----------------------------------------------------------------------------
void
TCoolFileBuffer::InsertLine(LPCTSTR text, int len /*= -1*/, int pos /*= -1*/)
{
  TLineInfo* li = new TLineInfo(text, len);
  if (pos == -1)
    LineArray.Add(li);
  else
    LineArray.AddAt(li, pos);

#ifdef __TRACE
  int lines = static_cast<int>(LineArray.Size());
  if (lines % 5000 == 0)
    TRACE("TCoolFileBuffer::InsertLine(): " << lines << " lines loaded!");
#endif
}
//-----------------------------------------------------------------------------
bool
TCoolFileBuffer::AppendText(int index, LPCTSTR text, int len /*= -1*/)
{
  PRECONDITION(index >=0 && index < (int)LineArray.Size());
  return LineArray[index]->Append(text, len);
}
//-----------------------------------------------------------------------------
void
TCoolFileBuffer::Clear(bool addempty)
{
  //  Free text
  LineArray.Flush(true);
  Inherited::Clear(addempty);
}
//-----------------------------------------------------------------------------
static const _TCHAR* crlfs[] =
{
  _T("\x0d\x0a"), //  DOS/Windows style
  _T("\x0a"),     //  UNIX style
  _T("\x0a")      //  Macintosh style
};
//-----------------------------------------------------------------------------
bool
TCoolFileBuffer::Load(LPCTSTR filename, TCrLfStyle style)
{
  TFile file(filename);
  if(file.IsOpen())
    return Load(file, style);
  return false;
}
//-----------------------------------------------------------------------------
bool
TCoolFileBuffer::Load(TFile& file, TCrLfStyle style /*= clStyleAutomatic*/)
{
  int   curMax = 256;
  TAPointer<_TCHAR> __ClnObj(new TCHAR[curMax]);
  _TCHAR* lineBuf = __ClnObj;

  int curLength = 0;

  const uint32 bufSize = 32768L;
#if defined(_DEBUG) // && (_DEBUG > 0)
  TAPointer<_TCHAR> __ClnObj1(new _TCHAR[bufSize]);
  _TCHAR* pcBuf = (_TCHAR*)__ClnObj1;
#else
  TTmpBuffer<_TCHAR> __tmpBuffer(bufSize);
  _TCHAR* pcBuf = (_TCHAR*)__tmpBuffer;
#endif
  uint32 curSize;

  if((curSize = file.Read(pcBuf, bufSize*sizeof(_TCHAR))) == TFILE_ERROR)
    return false;

  //  Free all data if file opened and readable
  LineArray.Flush(true);
  ClearUndoBuffer();
  ClearRedoBuffer();
  //

  if (style == clStyleAutomatic){
    //  Try to determine current CRLF mode
    int i = 0;
    for(; i < (int)curSize; i++){
      if(pcBuf[i] == _T('\x0a') || pcBuf[i] == _T('\x0d'))
        break;
    }
    if (i == (int)curSize){
      //  By default (or in the case of empty file), set DOS style
      style = clStyleDos;
    }
    else{
      //  Otherwise, analyse the first occurance of line-feed character
      if(pcBuf[i] == _T('\x0a'))
        style = clStyleUnix;
      else{
        if (i < int(curSize-1) && pcBuf[i+1] == _T('\x0a'))
          style = clStyleDos;
        else
          style = clStyleMac;
      }
    }
  }

  CHECK(style >= clStyleDos && style <= clStyleMac);
  CRLFMode = style;
  const _TCHAR* crlf = crlfs[style];

  //LineArray.Grow(4096);

  uint32 bufPtr = 0;
  int crlfPtr = 0;
  USES_CONVERSION;
  while (bufPtr < curSize){
    int c = pcBuf[bufPtr];
    bufPtr++;
    if (bufPtr == curSize && curSize == bufSize){
      if((curSize = file.Read(pcBuf, bufSize*sizeof(_TCHAR))) == TFILE_ERROR){
        if(GetLineCount()==0)
          InsertLine(_T("")); // always one line
        return false;
      }
      bufPtr = 0;
    }

    lineBuf[curLength] = (_TCHAR) c;
    curLength++;
    if(curLength == curMax){
      //  Reallocate line buffer
      curMax += 256;
      _TCHAR* newBuf = new _TCHAR[curMax];
      memcpy(newBuf, lineBuf, curLength * sizeof(_TCHAR));
      lineBuf   = newBuf;
      __ClnObj  = lineBuf;
    }

    if(crlf[crlfPtr] == (_TCHAR)c){
      crlfPtr ++;
      if (crlf[crlfPtr] == 0){
        lineBuf[curLength - crlfPtr] = 0;
        InsertLine(lineBuf,curLength - crlfPtr);
        curLength = 0;
        crlfPtr = 0;
      }
    }
    else
      crlfPtr = 0;
  }

  lineBuf[curLength] = 0;
  InsertLine(lineBuf, curLength);

  CHECK(LineArray.Size() > 0); //  At least one empty line must be present

  EnableReadOnly(false);
  TFileStatus status;
  if(file.GetStatus(status) && (status.attribute&TFile::RdOnly) !=0)
    EnableReadOnly(true);
  SetDirty(false);

  CHECK(GetLineCount());

  return true;
}
//-----------------------------------------------------------------------------
bool
TCoolFileBuffer::Save(LPCTSTR filename, TCrLfStyle style, bool clearDirty)
{
  // save to tmp file and create backup
  TFileName savename(filename);
  TFileName tmpname(TFileName::TempFile);

  TFile file;
  if(file.Open(tmpname.CanonicalStr(), TFile::WriteOnly | TFile::PermExclusive | TFile::CreateAlways)){
    if(Save(file, style, clearDirty)){
      if((Flags&bfCreateBackup)!=0 && savename.Exists()){
        TFileName bakname(savename);
        TFileName origname(savename);
        tstring backupExt = bakname.GetParts(TFileName::Ext);

        if (backupExt.length() < 2)
          backupExt = _T("~");
        else
          backupExt[1] = _T('~');
        bakname.SetPart(TFileName::Ext, backupExt);
        savename.Move(bakname, TFileName::ReplaceExisting|
          TFileName::CopyAllowed);
        savename = origname;
      }
      file.Close();
      tmpname.Move(savename, TFileName::ReplaceExisting|TFileName::CopyAllowed);
      return true;
    }
    file.Close();
  }
  return false;
}
//-----------------------------------------------------------------------------
bool
TCoolFileBuffer::Save(TFile& file, TCrLfStyle style, bool clearDirty)
{
  if(style == clStyleAutomatic)
    style = CRLFMode == clStyleAutomatic ? clStyleDos : CRLFMode;

  LPCTSTR pszCRLF = crlfs[style];
  int CRLFLength = static_cast<int>(_tcsclen(pszCRLF));

  USES_CONVERSION;
  for (int line = 0; line < (int)LineArray.Size(); line++){
    int length = LineArray[line]->Length;
    if (length > 0){
      if(!file.Write(_W2A(LineArray[line]->Text), length))
        return false;
    }
    if (line < (int)LineArray.Size() - 1){  //  Last line must not
      if(!file.Write(pszCRLF, CRLFLength))  //  end with CRLF
        return false;
    }
  }
  if(clearDirty)
    SetDirty(false);
  return true;
}
//-----------------------------------------------------------------------------
int
TCoolFileBuffer::FindLineWithFlag(uint32 flag)
{
  for(int i = 0; i < (int)LineArray.Size(); i++){
    if((LineArray[i]->Flags & flag) != 0)
      return i;
  }
  return -1;
}
//-----------------------------------------------------------------------------
void
TCoolFileBuffer::GetText(const TEditPos& startPos,const TEditPos& endPos,
             LPTSTR buffer, int buflen, LPCTSTR pszCRLF /*= NULL*/)
{
  PRECONDITION(startPos.row >= 0 && startPos.row < (int)LineArray.Size());
  PRECONDITION(startPos.col >= 0 && startPos.col <= LineArray[startPos.row]->Length);
  PRECONDITION(endPos.row >= 0 && endPos.row < (int)LineArray.Size());
  PRECONDITION(endPos.col >= 0 && endPos.col <= LineArray[endPos.row]->Length);
  PRECONDITION(startPos.row < endPos.row ||
    (startPos.row == endPos.row && startPos.col < endPos.col));

  if (pszCRLF == NULL)
    pszCRLF = crlf;
  int clLength = static_cast<int>(_tcslen(pszCRLF));
  CHECK(clLength > 0);

  int bufSize = 0;
  for (int i = startPos.row; i <= endPos.row; i++){
    bufSize += LineArray[i]->Length;
    bufSize += clLength;
  }

  TTmpBuffer<_TCHAR> __clnObj(bufSize+10);
  LPTSTR pszBuf    = __clnObj;

  if (startPos.row < endPos.row){
    int count = LineArray[startPos.row]->Length - startPos.col;
    if (count > 0){
      memcpy(pszBuf, LineArray[startPos.row]->Text + startPos.col,
        sizeof(_TCHAR)*count);
      pszBuf += count;
    }
    memcpy(pszBuf, pszCRLF, sizeof(TCHAR) * clLength);
    pszBuf += clLength;
    for(int j = startPos.row + 1; j < endPos.row; j++){
      count = LineArray[j]->Length;
      if (count > 0){
        memcpy(pszBuf, LineArray[j]->Text, sizeof(_TCHAR) * count);
        pszBuf += count;
      }
      memcpy(pszBuf, pszCRLF, sizeof(_TCHAR) * clLength);
      pszBuf += clLength;
    }
    if (endPos.col > 0){
      memcpy(pszBuf, LineArray[endPos.row]->Text, sizeof(_TCHAR) * endPos.col);
      pszBuf += endPos.col;
    }
  }
  else{
    int count = endPos.col - startPos.col;
    memcpy(pszBuf, LineArray[startPos.row]->Text + startPos.col,
      sizeof(_TCHAR) * count);
    pszBuf += count;
  }
  pszBuf[0] = 0;
  int count = endPos.col - startPos.col;
  if(count > buflen){
    memcpy(buffer,(_TCHAR*)__clnObj, sizeof(_TCHAR) * (buflen-1));
    buffer[buflen-1] = 0;
  }
  else
    _tcscpy(buffer,(_TCHAR*)__clnObj);
}
//-----------------------------------------------------------------------------
bool
TCoolFileBuffer::DeleteText(const TEditRange& range)
{
  PRECONDITION(range.srow >= 0 && range.srow < (int)LineArray.Size());
  PRECONDITION(range.scol >= 0 && range.scol <= LineArray[range.srow]->Length);
  PRECONDITION(range.erow >= 0 && range.erow < (int)LineArray.Size());
  PRECONDITION((range.ecol >= 0 && range.ecol <= LineArray[range.erow]->Length) ||
    (range.ecol == -1 && range.scol == 0));
  PRECONDITION(range.srow < range.erow || (range.srow == range.erow &&
    (range.scol < range.ecol || (range.scol == 0 && range.ecol == -1))));

  if(IsReadOnly())
    return false;

  if(range.srow == range.erow){
    // if endPos.col == -1 and startpos == 0
    if(range.ecol == -1 && range.scol == 0)
      LineArray.Destroy(range.srow);
    else {
      TLineInfo& li = *LineArray[range.srow];
      memcpy(li.Text + range.scol, li.Text + range.ecol,
        sizeof(TCHAR)*(li.Length - range.ecol));
      li.Length -= (range.ecol - range.scol);
    }
  }
  else{
    int restCount = LineArray[range.erow]->Length - range.ecol;
    TAPointer<_TCHAR> restChars;
    if (restCount > 0){
      restChars = new _TCHAR[restCount];
      memcpy((_TCHAR*)restChars, LineArray[range.erow]->Text + range.ecol,
        restCount * sizeof(TCHAR));
    }

    //for (int i = range.srow + 1; i <= range.erow; i++)
    for (int i = range.erow; i > range.srow; i--)
      LineArray.Destroy(i);

    //  endPos.row is no more valid
    LineArray[range.srow]->Length = range.scol;
    if (restCount > 0)
      AppendText(range.srow, (_TCHAR*)restChars, restCount);
  }
  SetDirty(true);
  return true;
}
//-----------------------------------------------------------------------------
bool
TCoolFileBuffer::InsertText(const TEditPos& startPos, LPCTSTR text,
              TEditPos& endPos)
{
  PRECONDITION(startPos.row >= 0 && startPos.row < (int)LineArray.Size());
  PRECONDITION(startPos.col >= 0 && startPos.col <= LineArray[startPos.row]->Length);
  if (IsReadOnly())
    return false;

  int restCount = LineArray[startPos.row]->Length - startPos.col;
  TAPointer<_TCHAR> restChars;
  if(restCount > 0){
    restChars = new TCHAR[restCount];
    memcpy((_TCHAR*)restChars, LineArray[startPos.row]->Text + startPos.col,
      restCount * sizeof(_TCHAR));
    LineArray[startPos.row]->Length = startPos.col;
  }

  int currentLine = startPos.row;
  int textPos;
  for (;;){
    textPos = 0;
    while (text[textPos] != 0 && text[textPos] != _T('\r'))
      textPos++;

    if (currentLine == startPos.row)
      AppendText(startPos.row, text, textPos);
    else{
      InsertLine(text, textPos, currentLine);
    }

    if (text[textPos] == 0 || text[textPos+1] == 0){
      endPos.row = currentLine;
      endPos.col = LineArray[currentLine]->Length;
      if(restCount)
        AppendText(currentLine, (_TCHAR*)restChars, restCount);
      break;
    }

    currentLine++;
    textPos++;

    if(text[textPos] == _T('\n'))
      textPos++;
    else{
      CHECK(false);     //  Invalid line-end format passed
    }
    text += textPos;
  }
  SetDirty(true);
  return true;
}
//-----------------------------------------------------------------------------
//
//
//
//{{TCoolTextWnd Implementation}}

UINT TCoolTextWnd::SelClipFormat = 0;

//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(TCoolTextWnd, TControl)
EV_WM_SIZE,
EV_WM_SETCURSOR,
EV_WM_KILLFOCUS,
EV_WM_SETFOCUS,
EV_WM_HSCROLL,
EV_WM_VSCROLL,
EV_WM_MOUSEMOVE,
EV_WM_LBUTTONDOWN,
EV_WM_LBUTTONUP,
EV_WM_KEYDOWN,
EV_WM_LBUTTONDBLCLK,
EV_COMMAND(CM_EDITCOPY, CmEditCopy),
EV_COMMAND_ENABLE(CM_EDITCOPY, CeSelectEnable),
EV_WM_SYSCOLORCHANGE,
EV_WM_MOUSEWHEEL,
#if !defined(COOL_NODRAGDROP)
EV_WM_TIMER,
#endif
END_RESPONSE_TABLE;

//-----------------------------------------------------------------------------
bool
TCoolTextWnd::TTextSelection::TextToClipboard(TClipboard& cb, LPCTSTR text,
                        int len)
{
  // Allocate a global memory object for the text.
  HGLOBAL hHandle = ::GlobalAlloc(GMEM_DDESHARE, len + 1);
  if (hHandle){
    // Lock the handle and copy the text to the buffer.
    char* pBlock = (char*)::GlobalLock(hHandle);
    memcpy(pBlock, text, len);
    pBlock[len] = 0;
    ::GlobalUnlock(hHandle);
    // Place the handle on the clipboard, as internal format.
    cb.SetClipboardData(GetFormat(), hHandle);
  }
  else
    return false;
  // Copy as Text Format
  // Allocate a global memory object for the text.
  HGLOBAL hHandle1 = ::GlobalAlloc(GMEM_DDESHARE, len-7+1);
  if (hHandle1){
    // Lock the handle and copy the text to the buffer.
    char* pBlock = (char*)::GlobalLock(hHandle1);
    memcpy(pBlock, text+7, len-7);
    pBlock[len-7]=0;
    ::GlobalUnlock(hHandle1);
    // Place the handle on the clipboard, as visible format.
#ifdef _UNICODE
    cb.SetClipboardData(CF_UNICODETEXT, hHandle1);
#else
    cb.SetClipboardData(CF_TEXT, hHandle1);
#endif
  }
  else
    return false;
  return true;
}
//-----------------------------------------------------------------------------
bool
TCoolTextWnd::TTextSelection::Copy()
{
  bool retval = false;
  TClipboard clipboard(*GetParent());
  if (clipboard.EmptyClipboard()){
    tostringstream ostrm;
    if(Copy(ostrm)){
      const tstring text = ostrm.str();
      retval = TextToClipboard(clipboard, text.c_str(), static_cast<int>(text.size()));
    }
  }
  return retval;
}
//-----------------------------------------------------------------------------
// TLineSelection
//
TCoolTextWnd::TLineSelection::TLineSelection(TCoolTextWnd* parent,
                       int start, int end)
                       :
TTextSelection(parent), Start(start), End(end)
{
}
//-----------------------------------------------------------------------------
bool
TCoolTextWnd::TLineSelection::GetSelection(int line, int& startChar,
                       int& endChar)
{
  if(HasSelection(line)){
    startChar = 0;
    endChar   = GetParent()->GetLineLength(line)+1;
  }
  return true;
}
//-----------------------------------------------------------------------------
bool
TCoolTextWnd::TLineSelection::Delete()
{
  int start = Start, end = End;
  if(start > end)
    std::swap(start,end);
  GetBuffer()->DeleteText(TEditRange(0, start, -1, end));
  return true;
}
//-----------------------------------------------------------------------------
bool
TCoolTextWnd::TLineSelection::Copy(tostream& os)
{
  TCoolTextBuffer* buffer = GetBuffer();
  int start = Start, end   = End;
  if(start > end)
    std::swap(start,end);

  os << lineSelectionId;
  for(int i = start; i <= end; i++){
    int length = buffer->GetLineLength(i);
    LPTSTR text = (LPTSTR)buffer->GetLineText(i);
    if(text){
      _TCHAR ch = text[length];
      text[length] = 0;
      os << text;
      text[length] = ch;
    }
    os << crlf;
  }
  os << ends;
  return true;
}
//-----------------------------------------------------------------------------
bool
TCoolTextWnd::TStreamSelection::GetSelection(int line, int& startChar,
                       int& endChar)
{
  if(HasSelection(line)){
    TEditRange range(Range);
    range.Normalize();
    startChar = line > range.srow ? 0 : std::min(range.ecol, range.scol);
    endChar   = line < range.erow ? GetParent()->GetLineLength(line)+1
      : std::max(range.ecol, range.scol);
  }
  return true;
}
//-----------------------------------------------------------------------------
bool
TCoolTextWnd::TStreamSelection::Delete()
{
  TEditRange range(Range);
  range.Normalize();
  GetBuffer()->DeleteText(range);
  return true;
}
//-----------------------------------------------------------------------------
bool TCoolTextWnd::TStreamSelection::Copy(tostream& os)
{
  TCoolTextBuffer* buffer = GetBuffer();
  TEditRange range(Range);
  range.Normalize();

  os << streamSelectionId;
  for(int i = range.srow; i <= range.erow; i++){
    LPTSTR text = (LPTSTR)buffer->GetLineText(i);
    int length = buffer->GetLineLength(i);
    if(i == range.srow){
      if(range.srow == range.erow && range.ecol < length)
        length = range.ecol;
      text += range.scol;
    }
    else if(i == range.erow && range.ecol < length)
      length = range.ecol;
    if(length)
      length -= range.scol;
    if(length < 0)
      length = 0;

    if(text){
      _TCHAR ch = text[length];
      text[length] = 0;
      os << text;
      text[length] = ch;
    }
    if(i < range.erow)
      os << crlf;
  }
  os << ends;
  return true;
}
//-----------------------------------------------------------------------------
TCoolTextWnd::TColumnSelection::TColumnSelection(TCoolTextWnd* parent,
                         const TEditRange& range)
                         :
TStreamSelection(parent, range)
{
}
//-----------------------------------------------------------------------------
bool
TCoolTextWnd::TColumnSelection::GetSelection(int line, int& startChar,
                       int& endChar)
{
  if(HasSelection(line)){
    TEditRange range(Range);
    range.Normalize();
    startChar = range.scol;
    endChar   = range.ecol;
  }
  return true;
}
//-----------------------------------------------------------------------------
bool
TCoolTextWnd::TColumnSelection::Delete()
{
  TCoolTextBuffer* buffer = GetBuffer();
  TEditRange range(Range);
  range.Normalize();

  for(int i = range.srow; i <= range.erow; i++){
    int length = buffer->GetLineLength(i);
    int startP =  range.scol < length ? range.scol : -1;
    int endP =  range.srow < length ? range.srow : length;
    if(startP > 0)
      buffer->DeleteText(TEditRange(startP, i, endP, i));
  }
  return true;
}
//-----------------------------------------------------------------------------
bool
TCoolTextWnd::TColumnSelection::Copy(tostream& os)
{
  TCoolTextBuffer* buffer = GetBuffer();
  TEditRange range(Range);
  range.Normalize();

  os << columnSelection;
  for(int i = range.srow; i <= range.erow; i++){
    LPTSTR text = (LPTSTR)buffer->GetLineText(i);
    int length = buffer->GetLineLength(i);
    int startP = range.scol;
    if(startP > length)
      os << crlf;
    else{
      int endP = range.erow;
      if(endP > length)
        endP = length;
      if(text){
        _TCHAR ch = text[endP];
        text[range.ecol] = 0;
        os << &text[startP];
        text[endP] = ch;
      }
      os << crlf;
    }
  }
  os << ends;
  return true;
}
//-----------------------------------------------------------------------------
// class TCoolScroller
// ~~~~~ ~~~~~~~~~~~~~
TCoolScroller::TCoolScroller(TWindow* window, int xUnit, int yUnit,
               long xRange, long yRange)
               :
TScroller(window,xUnit,yUnit,xRange,yRange)
{
}
#if 0 //OWLInternalVersion < 0x06001020L
//-----------------------------------------------------------------------------
void
TCoolScroller::SetSBarRange()
{
  PRECONDITION(Window);
  PRECONDITION(Window->GetHandle());

  if (Window && Window->GetHandle()) {
    if (HasHScrollBar) {
      int curMin, curMax;
      GetScrollRange(SB_HORZ, curMin, curMax);
      int newMax = std::max(0, std::min(XRange - 1, SHRT_MAX));
      if (newMax != curMax || curMin != 0)
        SetScrollRange(SB_HORZ, 0, newMax, true);
    }

    if (HasVScrollBar) {
      int curMin, curMax;
      GetScrollRange(SB_VERT, curMin, curMax);
      int newMax = std::max(0, std::min(YRange - 1, SHRT_MAX));
      if (newMax != curMax || curMin != 0)
        SetScrollRange(SB_VERT, 0, newMax, true);
    }
  }
}
#endif //
//-----------------------------------------------------------------------------
void TCoolScroller::ScrollTo(long x, long y)
{
  PRECONDITION(Window);
  PRECONDITION(Window->GetHandle());

  if (Window && Window->GetHandle()) {
    long  newXPos = std::max(0L, std::min(x, XRange - XPage));
    long  newYPos = std::max(0L, std::min(y, YRange - YPage));

    if (newXPos != XPos || newYPos != YPos) {
      //
      // scaling isn't needed here. if condition is met, ScrollWindow()
      // will succeed since XPage and YPage are ints
      //
      // if condition isn't met, ScrollWindow() is called in EndScroll()
      // as a result of calling UpdateWindow()
      //
      // EndScroll() performs the necessary scaling
      //
      TCoolTextWnd* wnd = TYPESAFE_DOWNCAST(Window, TCoolTextWnd);
      CHECK(wnd);
      if (AutoOrg || (abs(YPos - newYPos) < YPage && abs(XPos - newXPos) < XPage))
      {
        TRect clientRect;
        Window->GetClientRect(clientRect);
        if((YPos - newYPos) == 0)
          clientRect.left  += wnd->GetMarginWidth();
        Window->ScrollWindow((int)(XPos - newXPos) * XUnit,
          (int)(YPos - newYPos) * YUnit, &clientRect, &clientRect);
      }
      else
        wnd->InvalidateLines(0, -1);

      XPos = newXPos;
      YPos = newYPos;
      Window->UpdateWindow();
    }
  }
}
//-----------------------------------------------------------------------------
// class TCoolTextWnd
// ~~~~~ ~~~~~~~~~~~~
//
TCoolTextWnd::TCoolTextWnd(TWindow* parent,int id,LPCTSTR title,int x, int y,
               int w, int h, TModule* module)
               :
TControl(parent,id,title,x,y,w,h,module),
CursorPos(0,0),
Flags(cfSelMargin|cfSmartCursor|cfNoUseCaret|cfLookWrdUnderCur|cfAutoIntend),
TabSize(4),
LineHeight(-1),
CharWidth(-1),
ScreenLines(-1),
ScreenChars(-1),
MaxLineLength(-1),
IdealCharPos(-1),
MemoryBitmap(0),
CelArray(0),
SelType(stStreamSelection),
Selection(0),
SyntaxParser(0),
ParseCookies(0),
ParseArraySize(0),
ActualLengthArraySize(0),
ActualLineLength(0),
Cursor(0),
LastHit(-1,-1),
MarginWidth(16),
LineDigits(5),
CurbPos(80),
TmpCurbPos(0)
#if !defined(COOL_NODRAGDROP)
,DragDropProxy(new TTextDragDropProxy)
,DragDropSupport(DragDropProxy)
,DragSelTimer(0)
#endif
{
  ModifyStyle(0, WS_VSCROLL|WS_HSCROLL);
  SetBkgndColor(TColor::Transparent);

  memset(TextFonts,0,sizeof(TextFonts));

  if(!SelClipFormat)
    SelClipFormat = ::RegisterClipboardFormat(_T("CoolEdit Text Block"));
}
//-----------------------------------------------------------------------------
TCoolTextWnd::~TCoolTextWnd()
{
#if !defined(COOL_NODRAGDROP)
  delete DragDropProxy;
#endif
  for(int i = 0; i < 8; i++)
    delete TextFonts[i];
  delete SyntaxParser;
  delete MemoryBitmap;
  delete CelArray;
  delete Selection;
  delete[] ParseCookies;
  delete[] ActualLineLength;
  delete Cursor;
}
//
void
TCoolTextWnd::SetupWindow()
{
  if(!Scroller)
    Scroller = new TCoolScroller(this, 7, 16, 80, 60);
#if !defined(USE_AUTOORG)
  Scroller->AutoOrg = false;
#endif

  // Call base class function.
  Inherited::SetupWindow();

#if !defined(COOL_NODRAGDROP)
  DragDropSupport.Register(*this);
#endif
  if(!CelArray)
    CelArray = new TCelArray(new TBtnBitmap(*GetModule(),IDR_MARGIN_ICONS), 26);
  if(!Cursor)
    Cursor  = new TCursor(*GetModule(), TResId(IDR_MARGIN_CURSOR));
  SetCursor(0, IDC_IBEAM);

  TCoolTextBuffer* buffer = GetBuffer();
  if(!buffer->SyntaxDescrExist())
    buffer->BuildDefSyntaxDescr();

  // setup scroll page size
  Scroller->SetUnits(GetCharWidth(), GetLineHeight());
  AdjustScroller();
}
//-----------------------------------------------------------------------------
void
TCoolTextWnd::CleanupWindow()
{
#if !defined(COOL_NODRAGDROP)
  DragDropSupport.UnRegister(*this);

  if(DragSelTimer)
    KillTimer(DragSelTimer);
  DragSelTimer = 0;
#endif

  // Call base class function.
  Inherited::CleanupWindow();
}
//-----------------------------------------------------------------------------
void TCoolTextWnd::SetSyntaxParser(TSyntaxParser* parser)
{
  delete SyntaxParser;
  SyntaxParser = parser;
}
//-----------------------------------------------------------------------------
TConfigFile* TCoolTextWnd::CreateConfigFile()
{
  return new TRegConfigFile(configOWLNExt);
}
//-----------------------------------------------------------------------------
void TCoolTextWnd::FileNew()
{
  PRECONDITION(GetBuffer());
  GetBuffer()->Clear(true);// add empty line
  ResetView();
  AdjustScroller();
  InvalidateLines(0, -1, false);
}
//-----------------------------------------------------------------------------
bool TCoolTextWnd::FileOpen(LPCTSTR filename)
{
  PRECONDITION(GetBuffer());
  if(!GetBuffer()->Load(filename))
    return false;
  ResetView();
  if(GetHandle()){
    AdjustScroller();
    InvalidateLines(0, -1, false);
  }
  return true;
}
//-----------------------------------------------------------------------------
void TCoolTextWnd::GetWindowClass(WNDCLASS& wndClass)
{
  // Call base class function.
  Inherited::GetWindowClass(wndClass);
  wndClass.style |= CS_DBLCLKS;
}
//-----------------------------------------------------------------------------
auto TCoolTextWnd::GetWindowClassName() -> TWindowClassName
{
  return TWindowClassName{_T("COOLEDIT")};
}
//-----------------------------------------------------------------------------
void TCoolTextWnd::Paint(TDC& dc, bool /*erase*/, TRect& /*rect*/)
{
  TRect clientRect;
  GetClientRect(clientRect);
  int lineCount  = GetNumLines();
  int lineHeight = GetLineHeight();

  // must be member variables
  TMemoryDC  memDC(dc);
  if(!MemoryBitmap)
    MemoryBitmap = new TBitmap(dc, clientRect.Width(), lineHeight);
  memDC.SelectObject(*MemoryBitmap);

  TRect lineRect(clientRect.left, clientRect.top,
    clientRect.right, clientRect.top + lineHeight);
  TRect cacheMargin(0, 0, GetMarginWidth(), lineHeight);
  TRect cacheLine(GetMarginWidth(), 0, lineRect.Width(), lineHeight);

  int currentLine = GetTopLine();
#if !defined(USE_AUTOORG)
  int leftPos = CurbPos == -1 ? -1 : (cacheLine.left+(CurbPos-GetOffsetChar())*GetCharWidth());
#else
  int leftPos = CurbPos == -1 ? -1 : (cacheLine.left+CurbPos*GetCharWidth());
#endif
  while (lineRect.Top() < clientRect.Bottom()){
    if(currentLine < lineCount){
      DrawMargin(memDC, cacheMargin, currentLine);
      DrawLine(memDC, cacheLine, currentLine);
    }
    else{
      DrawMargin(memDC, cacheMargin, -1);
      DrawLine(memDC, cacheLine, -1);
    }
    // paint line 80 characters
    if(leftPos >= 0){
      memDC.SelectObject(TUIFace::GetDitherBrush());
      VERIFY(memDC.PatBlt(leftPos-1, cacheLine.top, 1, cacheLine.bottom, PATCOPY));
      memDC.RestoreBrush();
    }

    // Now transfer what was drawn on the (invisible) memory DC onto the
    // visible dc This one BitBlt transfer is much faster than the many
    // individual operations that were performed above.
    //
    VERIFY(dc.BitBlt(lineRect, memDC, TPoint(0,0)));
    currentLine++;
    lineRect.Offset(0, lineHeight);
  }
  // Restore original bitmap before leaving
  //
  memDC.RestoreBitmap();
}
//
bool
TCoolTextWnd::GetLine(LPTSTR str, int strSize, int lineNumber) const
{
  TCoolTextBuffer* buffer = CONST_CAST(TCoolTextWnd*,this)->GetBuffer();
  PRECONDITION(buffer);
  if(lineNumber >= GetNumLines())
    return false;
  int endPos = GetLineLength(lineNumber);
  if(endPos > strSize)
    endPos = strSize-1;
  buffer->GetText(TEditPos(lineNumber, 0), TEditPos(lineNumber, endPos), str, strSize);
  return true;
}
//
void
TCoolTextWnd::CalcLineCharDim()
{
  TClientDC  clientDC(*this);
  TSize extent = GetFont()->GetTextExtent(clientDC, _T("X"));
  LineHeight = extent.cy;
  if (LineHeight < 1)
    LineHeight = 1;
  CharWidth = extent.cx;
}
//
int
TCoolTextWnd::GetLineHeight() const
{
  if (LineHeight == -1)
    CONST_CAST(TCoolTextWnd*,this)->CalcLineCharDim();
  return LineHeight;
}
//
int
TCoolTextWnd::GetCharWidth() const
{
  if (CharWidth == -1)
    CONST_CAST(TCoolTextWnd*,this)->CalcLineCharDim();
  return CharWidth;
}
//
TFont*
TCoolTextWnd::CreateFont(int index)
{
  if(!TextFonts[index]){
    PRECONDITION(GetBuffer());
    LOGFONT lf;
    GetBuffer()->GetFont(lf);
    lf.lfWeight   = (index & 0x01) ? FW_BOLD : FW_NORMAL;
    lf.lfItalic   = (uint8)((index & 0x02)!=0);
    lf.lfUnderline= (uint8)((index & 0x04)!=0);
    TextFonts[index] = new TFont(lf);
  }
  return TextFonts[index];
}
//
TFont*
TCoolTextWnd::GetFont(bool italic, bool bold, bool underline)
{
  int index = 0;
  if(bold)
    index |= 0x0001;
  if(italic)
    index |= 0x0002;
  if(underline)
    index |= 0x0004;
  return CreateFont(index);
}
//
void
TCoolTextWnd::SetFont(const LOGFONT &lf)
{
  PRECONDITION(GetBuffer());
  GetBuffer()->SetFont(lf);

  LineHeight = -1;
  CharWidth = -1;
  ScreenLines = -1;
  ScreenChars = -1;
  delete MemoryBitmap;
  MemoryBitmap = 0;
  for (int i = 0; i < 8; i ++){
    if (TextFonts[i]){
      delete TextFonts[i];
      TextFonts[i] = 0;
    }
  }
  if(GetHandle()){
    // setup scroll page size
    Scroller->SetUnits(GetCharWidth(), GetLineHeight());
    AdjustScroller();
    UpdateCaret();
  }
}
//
void
TCoolTextWnd::SetSyntaxFormat(TDC& dc, int index)
{
  TCoolTextBuffer::TSyntaxDescr& node = GetSyntaxDescr(index);
  dc.SetTextColor(node.TxColor);
  dc.SetBkColor(node.BkColor);
  dc.SelectObject(*GetFont(node.FontIndex));
}
//
void
TCoolTextWnd::FormatLineNumber(LPTSTR buffer, int len, int index)
{
  if (index > 0)
    _stprintf_s(buffer, len, _T("%5u"), index);
  else
    buffer[0] = 0;
}
//
int
TCoolTextWnd::CellImageIndex(int lineIndex)
{
  int imageIndex = -1;
  if (lineIndex >= 0){
    PRECONDITION(GetBuffer());
    uint32 lineFlags = GetBuffer()->GetLineFlags(lineIndex);
    if(lineFlags==0)
      return -1;

    static const TCoolTextBuffer::TLineFlags adwFlags[] = {
      LF_BOOKMARKID(1),
      LF_BOOKMARKID(2),
      LF_BOOKMARKID(3),
      LF_BOOKMARKID(4),
      LF_BOOKMARKID(5),
      LF_BOOKMARKID(6),
      LF_BOOKMARKID(7),
      LF_BOOKMARKID(8),
      LF_BOOKMARKID(9),
      LF_BOOKMARKID(0),
      TCoolTextBuffer::lfBreakPoint,
      TCoolTextBuffer::lfCompError,
      TCoolTextBuffer::lfBookMarks,
      TCoolTextBuffer::lfInvalidBPoint,
      TCoolTextBuffer::lfDisabledBPoint,
      TCoolTextBuffer::lfReserved1,
      TCoolTextBuffer::lfReserved2,
      TCoolTextBuffer::lfExecution,
    };
    for (int i = 0; i <= COUNTOF(adwFlags); i++){
      if((lineFlags & adwFlags[i]) != 0){
        imageIndex = i;
        break;
      }
    }
    if((lineFlags & TCoolTextBuffer::lfExecution) != 0){

#define ALL_FLAGS (TCoolTextBuffer::lfBreakPoint|TCoolTextBuffer::lfCompError|\
  TCoolTextBuffer::lfBookMarks|TCoolTextBuffer::lfInvalidBPoint|\
  TCoolTextBuffer::lfDisabledBPoint|TCoolTextBuffer::lfReserved1|\
  TCoolTextBuffer::lfReserved2)

      if((lineFlags&ALL_FLAGS)!=0)
        imageIndex += 9;
      else if(imageIndex < COUNTOF(adwFlags)-1)
        imageIndex = 18;
    }
  }
  return imageIndex;
}
//
void
TCoolTextWnd::DrawMargin(TDC& dc, const TRect& rect, int lineIndex)
{
  if(!IsSelectionMargin()){
    dc.SetBkColor(GetBkColor(COLORINDEX_WHITESPACE));
    dc.TextRect(rect);
    return;
  }
  TRect imageRect(rect);
  if(IsLineNumbers())
    imageRect.right -= (LineDigits + 1) * GetCharWidth();

  dc.SetBkColor(GetBkColor(COLORINDEX_MARGIN));
  dc.TextRect(imageRect);

  int imageIndex = CellImageIndex(lineIndex);
  if (imageIndex >= 0)
    VERIFY(CelArray->BitBlt(imageIndex, dc, imageRect.left+2,
    imageRect.top+(imageRect.Height()-16)/2));

  // draw line number
  if(IsLineNumbers()){
    TTmpBuffer<_TCHAR> buffer(MAX_PATH);
    FormatLineNumber(buffer, MAX_PATH, lineIndex+1);
    TRect textRect(rect);
    textRect.left = textRect.right - (LineDigits + 1) * GetCharWidth();
    SetSyntaxFormat(dc, COLORINDEX_MARGIN);
    dc.ExtTextOut(textRect.left, textRect.top, ETO_OPAQUE, &textRect, &buffer[0], -1);
  }
  dc.SelectObject(TUIFace::GetDitherBrush());
  dc.PatBlt(rect.right-1, rect.top, 1, rect.bottom, PATCOPY);
  dc.RestoreBrush();
}
//
void
TCoolTextWnd::DrawLine(TDC& dc, const TRect& rect, int lineIndex)
{
  PRECONDITION(lineIndex >= -1 && lineIndex < GetNumLines());
  PRECONDITION(GetBuffer());

  if(lineIndex == -1){
    //  Draw line beyond the text
    dc.SetBkColor(GetBkColor(COLORINDEX_WHITESPACE));
    dc.TextRect(rect);
    return;
  }

  //  Acquire the background color for the current line
  uint32 lineFlags = GetBuffer()->GetLineFlags(lineIndex);
  bool drawWhitespace = false;
  int colorIdx;
  if(lineFlags&TCoolTextBuffer::lfExecution){
    drawWhitespace = true;
    colorIdx = COLORINDEX_EXECUTION;
  }
  else if(lineFlags&TCoolTextBuffer::lfBreakPoint){
    drawWhitespace = true;
    colorIdx = COLORINDEX_BREAKPOINT;
  }
  else if(lineFlags&TCoolTextBuffer::lfCompError){
    drawWhitespace = true;
    colorIdx = COLORINDEX_ERROR;
  }
  else if(lineFlags&TCoolTextBuffer::lfInvalidBPoint){
    drawWhitespace = true;
    colorIdx = COLORINDEX_INVALIDBREAK;
  }
  else
    colorIdx = COLORINDEX_NORMALTEXT;

  int length = GetLineLength(lineIndex);
  if (!length){
    //  Draw the empty line
    TRect r(rect);
    TEditPos editPos(0,lineIndex);
    if(IsFlagSet(cfFocused|cfShowInactiveSel) && IsInSelection(editPos)){
      dc.SetBkColor(GetBkColor(COLORINDEX_SELECTION));
      TRect rc(r.left, r.top, r.left + GetCharWidth(), r.bottom);
      dc.TextRect(rc);
      r.left += GetCharWidth();
    }
    dc.SetBkColor(GetBkColor(drawWhitespace ? colorIdx : COLORINDEX_WHITESPACE));
    dc.TextRect(r);
    return;
  }

  int blocks = 0;
  TAPointer<TTextBlock> __ClnObj;
  TTextBlock* pBuf = 0;
  LPCTSTR pszChars = GetLineText(lineIndex);

  if(!drawWhitespace){
    //  Parse the line
    uint32 cookie   = GetParseCookie(lineIndex - 1);

    GetParseCookie(lineIndex);  //Jogy - ensure that ParseCookies[lineIndex] is allocated!

    if(SyntaxParser){
      __ClnObj = pBuf = new TTextBlock[length*3];
      ParseCookies[lineIndex] =
        SyntaxParser->ParseLine(cookie, lineIndex, pBuf, blocks);
    }
    else
      ParseCookies[lineIndex] = 0;
    CHECK(ParseCookies[lineIndex] != (uint32)-1);
  }

  //  Draw the line text
#if !defined(USE_AUTOORG)
  TPoint origin(rect.left - GetOffsetChar() * GetCharWidth(), rect.top);
#else
  TPoint origin(rect.left, rect.top);
#endif
  if (blocks > 0){
    CHECK(pBuf[0].CharPos >= 0 && pBuf[0].CharPos <= length);
    if(pBuf[0].CharPos){
      SetSyntaxFormat(dc, colorIdx);
      DrawLineBlock(dc, origin, rect, COLORINDEX_NORMALTEXT, pszChars,
        0, pBuf[0].CharPos, TEditPos(0, lineIndex));
    }
    for (int i = 0; i < blocks - 1; i ++){
      CHECK(pBuf[i].CharPos >= 0 && pBuf[i].CharPos <= length);
      if(pBuf[i + 1].CharPos - pBuf[i].CharPos){
        SetSyntaxFormat(dc, pBuf[i].SyntaxIndex);
        DrawLineBlock(dc, origin, rect, pBuf[i].SyntaxIndex,
          pszChars,pBuf[i].CharPos,
          pBuf[i + 1].CharPos - pBuf[i].CharPos,
          TEditPos(pBuf[i].CharPos, lineIndex));
      }
    }
    CHECK(pBuf[blocks - 1].CharPos >= 0 && pBuf[blocks - 1].CharPos <= length);
    if(length - pBuf[blocks - 1].CharPos){
      SetSyntaxFormat(dc, pBuf[blocks - 1].SyntaxIndex); //<<----------------
      DrawLineBlock(dc, origin, rect, pBuf[blocks - 1].SyntaxIndex,
        pszChars,pBuf[blocks - 1].CharPos,
        length - pBuf[blocks - 1].CharPos,
        TEditPos(pBuf[blocks - 1].CharPos, lineIndex));
    }
  }
  else if(length){
    SetSyntaxFormat(dc, drawWhitespace ? colorIdx : COLORINDEX_NORMALTEXT);
    DrawLineBlock(dc, origin, rect, drawWhitespace ? colorIdx : COLORINDEX_NORMALTEXT,
      pszChars, 0, length, TEditPos(0, lineIndex));
  }

  //  Draw whitespaces to the left of the text
  TRect frect(rect);
  if (origin.x > frect.left)
    frect.left = origin.x;
  if (frect.right > frect.left){
    if((IsFlagSet(cfFocused|cfShowInactiveSel)) && IsInSelection(TEditPos(length, lineIndex))){
      dc.SetBkColor(GetBkColor(COLORINDEX_SELECTION));
      TRect rc(frect.left, frect.top, frect.left+GetCharWidth(), frect.bottom);
      dc.TextRect(rc);
      frect.left += GetCharWidth();
    }
    if(frect.right > frect.left){
      dc.SetBkColor(GetBkColor(drawWhitespace ? colorIdx : COLORINDEX_WHITESPACE));
      dc.TextRect(frect);
    }
  }
}
//
uint32
TCoolTextWnd::GetParseCookie(int lineIndex)
{
  if(!ParseCookies){
    int lineCount = GetNumLines();
    ParseArraySize = lineCount;
    ParseCookies = new uint32[lineCount];
    memset(ParseCookies, 0xff, lineCount * sizeof(uint32));
  }

  if(lineIndex < 0)
    return 0;

  if(lineIndex >= ParseArraySize){
    uint32* cookie = new uint32[lineIndex*2];
    uint size  = ParseArraySize * sizeof(uint32);
    memcpy(cookie, ParseCookies, size);
    memset((uint8*)cookie+size, 0xff, (lineIndex*2-ParseArraySize)*sizeof(uint32));
    delete[] ParseCookies;
    ParseCookies = cookie;
    ParseArraySize = lineIndex*2;
  }

  if (ParseCookies[lineIndex] != (uint32)-1)
    return ParseCookies[lineIndex];

  int l = lineIndex;
  while(l >= 0 && ParseCookies[l] == (uint32)-1)
    l--;
  l++;

  int blocks;
  while(l <= lineIndex){
    uint32 cookie = 0;
    if (l > 0)
      cookie = ParseCookies[l - 1];
    CHECK(cookie != (uint32) -1);
    if(SyntaxParser)
      ParseCookies[l] = SyntaxParser->ParseLine(cookie, l, 0, blocks);
    else
      ParseCookies[l] = 0l;
    //_TRACE("TCoolTextWnd::GetParseCookie() index: %d, cookie %ld\n", l, ParseCookies[l]);
    CHECK(ParseCookies[l] != (uint32)-1);
    l++;
  }
  return ParseCookies[lineIndex];
}
//
void
TCoolTextWnd::DrawLineBlock(TDC& dc, TPoint& origin,
              const TRect& clipRect, int clrIndex,
              LPCTSTR text, int offset, int count,
              const TEditPos& textPos)
{
  if (count>0){
    if(IsFlagSet(cfFocused|cfShowInactiveSel) && Selection){
      int selBegin = 0, selEnd = 0;
      if(Selection->HasSelection(textPos.row)){
        int startSel, endSel;
        if(Selection->GetSelection(textPos.row, startSel, endSel)){
          selBegin = startSel - textPos.col;
          if (selBegin < 0)
            selBegin = 0;
          if (selBegin > count)
            selBegin = count;
          selEnd = endSel - textPos.col;
          if (selEnd < 0)
            selEnd = 0;
          if (selEnd > count)
            selEnd = count;
        }
      }
      else{
        selBegin = count;
        selEnd = count;
      }

      CHECK(selBegin >= 0 && selBegin <= count);
      CHECK(selEnd >= 0 && selEnd <= count);
      CHECK(selBegin <= selEnd);

      //  Draw part of the text before selection
      if (selBegin > 0)
        DrawTextBlock(dc, origin, clipRect, text, offset, selBegin);
      // Draw selected block
      if (selBegin < selEnd){
        SetSyntaxFormat(dc, COLORINDEX_SELECTION);
        DrawTextBlock(dc, origin, clipRect, text,
          offset+selBegin, selEnd-selBegin);
        SetSyntaxFormat(dc, clrIndex);
      }
      // draw block after selection
      if (selEnd < count)
        DrawTextBlock(dc, origin, clipRect, text,
        offset + selEnd, count - selEnd);
    }
    else
      DrawTextBlock(dc, origin, clipRect, text, offset, count);
  }
}
//
void
TCoolTextWnd::DrawTextBlock(TDC& dc, TPoint& origin, const TRect& clipRect,
              LPCTSTR text, int offset, int len)
{
  PRECONDITION(len >= 0);
  if (len > 0){
    tstring line;
    ExpandText(text, offset, len, line);
    int width = clipRect.right - origin.x;
    int count = static_cast<int>(_tcslen(line.c_str()));
    if (width > 0){
      int countFit = width / GetCharWidth() + 1;
      if (count > countFit)
        count = countFit;
      VERIFY(dc.ExtTextOut(origin, ETO_CLIPPED, &clipRect,
        line.c_str(), count));
    }
    origin.x += GetCharWidth() * count;
  }
}
//
void
TCoolTextWnd::ExpandText(LPCTSTR text, int offset, int count, tstring& line)
{
  if(count <= 0){
    line = _T("");
    return;
  }

  int tabSize = GetTabSize();
  int actualOffset = 0;

  for(int i = 0; i < offset; i++){
    if (text[i] == chTab)
      actualOffset += (tabSize - actualOffset % tabSize);
    else
      actualOffset ++;
  }

  text += offset;
  int length = count;

  int tabCount = 0;

  for(int j = 0; j < length; j++){
    if (text[j] == chTab)
      tabCount++;
  }

  line.resize(length + tabCount * (tabSize - 1)+1);
  LPTSTR pszBuf = (LPTSTR)line.c_str();
  int curPos = 0;
  if (tabCount > 0 || IsFlagSet(cfViewTabs)){
    for (int i = 0; i < length; i++){
      if(text[i] == chTab){
        int spaces = tabSize - (actualOffset + curPos) % tabSize;
        if (IsFlagSet(cfViewTabs)){
          pszBuf[curPos++] = TAB_CHARACTER;
          spaces--;
        }
        while (spaces > 0){
          pszBuf[curPos++] = _T(' ');
          spaces--;
        }
      }
      else{
        if(text[i] == _T(' ') && IsFlagSet(cfViewTabs))
          pszBuf[curPos] = SPACE_CHARACTER;
        else
          pszBuf[curPos] = text[i];
        curPos++;
      }
    }
  }
  else{
    memcpy(pszBuf, text, sizeof(_TCHAR)*length);
    curPos = length;
  }
  pszBuf[curPos] = 0;
}
//
bool
TCoolTextWnd::IsInSelection(const TEditPos& textPos)
{
  //return Selection && Selection->HasSelection(textPos.row);
  if(Selection && Selection->HasSelection(textPos.row)){
    int startSel, endSel;
    if(Selection->GetSelection(textPos.row, startSel, endSel))
      return textPos.col >= startSel && textPos.col < endSel;
  }
  return false;
}
//
void
TCoolTextWnd::EnableTabs(bool viewTabs)
{
  if(viewTabs != IsFlagSet(cfViewTabs)){
    viewTabs ? SetFlag(cfViewTabs) : ClearFlag(cfViewTabs);
    if (IsWindow())
      InvalidateLines(0, -1);
  }
}
//
void
TCoolTextWnd::EnableCaret(bool enable)
{
  if(enable != IsCaretEnable()){
    enable ? ClearFlag(cfNoUseCaret) : SetFlag(cfNoUseCaret);
    if (IsWindow() && IsFlagSet(cfFocused)){
      if(IsFlagSet(cfNoUseCaret)){
        HideCaret();
        DestroyCaret();
      }
      else {
        ClearFlag(cfCursorHidden);
        ClearFlag(cfHasCaret);
        UpdateCaret();
        ShowCaret();
      }
    }
  }
}
//
void
TCoolTextWnd::EnableShowInactiveSel(bool enable)
{
  if(enable != IsShowInactiveSel()){
    enable ? SetFlag(cfShowInactiveSel) : ClearFlag(cfShowInactiveSel);
    if (IsWindow())
      InvalidateLines(0, -1);
  }
}
//
void TCoolTextWnd::EnableSmartCursor(bool enable)
{
  if(enable != IsSmartCursor()){
    enable ? SetFlag(cfSmartCursor) : ClearFlag(cfSmartCursor);
    if(IsWindow())
      InvalidateLines(0, -1);
  }
}
//
void
TCoolTextWnd::EnableSelectionMargin(bool selMargin)
{
  if(IsFlagSet(cfSelMargin) != selMargin){
    selMargin ? SetFlag(cfSelMargin) : ClearFlag(cfSelMargin);
    if(GetHandle()){
      ScreenChars = -1;
      InvalidateLines(0, -1);
      AdjustScroller();
    }
  }
}
//
void TCoolTextWnd::EnableDragDrop(bool enable)
{
  if(IsFlagSet(cfNoDragDrop) != enable){
    enable ? SetFlag(cfNoDragDrop) : ClearFlag(cfNoDragDrop);
  }
}
//
void TCoolTextWnd::EnableAutoIndent(bool enable)
{
  if(IsFlagSet(cfAutoIntend) != enable){
    enable ? SetFlag(cfAutoIntend) : ClearFlag(cfAutoIntend);
    if (IsWindow())
      InvalidateLines(0, -1);
  }
}

//
void
TCoolTextWnd::ShowLineNumbers(bool enable)
{
  if(IsFlagSet(cfShowLineNumber) != enable){
    enable ? SetFlag(cfShowLineNumber) : ClearFlag(cfShowLineNumber);
    if (IsWindow()){
      UpdateCaret();
      InvalidateLines(0, -1);
    }
  }
}

//
int
TCoolTextWnd::GetMarginWidth() const
{
  int retval = IsFlagSet(cfSelMargin) ? MarginWidth : 1;
  if(IsLineNumbers())
    retval += (LineDigits + 1) * GetCharWidth(); // 5 chars
  return retval;
}
//-----------------------------------------------------------------------------
void
TCoolTextWnd::SetMarginWidth(int width)
{
  if(MarginWidth != width){
    MarginWidth = width;
    if (IsWindow())
      InvalidateLines(0, -1);
  }
}
//-----------------------------------------------------------------------------
void
TCoolTextWnd::SetLineNumberWidth(int width)
{
  if(LineDigits != width){
    LineDigits = width;
    InvalidateLines(0, -1);
  }
}
//-----------------------------------------------------------------------------
void
TCoolTextWnd::SetTabSize(int tabSize)
{
  PRECONDITION(tabSize >= 0 && tabSize <= 64);
  if (TabSize != tabSize){
    TabSize = tabSize;
    if(ActualLineLength){
      delete ActualLineLength;
      ActualLineLength = 0;
    }
    ActualLengthArraySize = 0;
    MaxLineLength = -1;
    if(GetHandle()){
      AdjustScroller();
      UpdateCaret();
      InvalidateLines(0, -1);
    }
  }
}
//
void
TCoolTextWnd::AdjustScroller()
{
  static bool in_work = false;
  if(!Scroller || in_work)
    return;

  in_work = true;
  TRect clientRect;
  GetClientRect(clientRect);

  // Only show scrollbars when image is larger than
  // the client area and we are not stretching to fit.
  //
  //int countChars  = clientRect.Width()/GetCharWidth();
  //int countLines  = clientRect.Height()/GetLineHeight();
  TPoint range(10000, GetNumLines()-1);
  //MaxLineLength = -1;
  //TPoint range(GetMaxLineLength(), GetNumLines());
  //??????
  //Scroller->SetUnits(GetCharWidth(), GetLineHeight());

  Scroller->SetRange(range.x, range.y);

  //Scroller->ScrollTo(0, 0);
  if (!GetUpdateRect(clientRect, false))
    InvalidateLines(0, -1, false);
  in_work = false;
}
//
void
TCoolTextWnd::EvSize(uint sizeType, const TSize& size)
{
  Inherited::EvSize(sizeType, size);
  if (sizeType != SIZEICONIC) {
    delete MemoryBitmap;
    MemoryBitmap = 0;
    ScreenLines = -1;
    ScreenChars = -1;
    //AdjustScroller();
    //Invalidate(false);
  }
}
//
bool
TCoolTextWnd::EvSetCursor(HWND hWndCursor, uint hitTest, uint mouseMsg)
{
  if (hitTest == HTCLIENT){
    TPoint point;
    Inherited::GetCursorPos(point);
    ScreenToClient(point);
    int lineCurb = CurbPos==-1 ? -1 : ((CurbPos-GetOffsetChar())*GetCharWidth()+GetMarginWidth());
    if(point.x < GetMarginWidth())
      ::SetCursor(*Cursor);
    else if(lineCurb >= 0 && (point.x < (int)(lineCurb+3) && point.x > (int)(lineCurb-3)))
      ::SetCursor(::LoadCursor(0, TResId(IDC_SIZEWE))); // Set To Arrow Cursor
    else{
      if(Selection && IsInSelection(Client2Text(point))){
#if !defined(COOL_NODRAGDROP)
        if(!IsFlagSet(cfNoDragDrop))
#endif
          ::SetCursor(::LoadCursor(0, TResId(IDC_ARROW))); // Set To Arrow Cursor
      }
      else
        return Inherited::EvSetCursor(hWndCursor, hitTest, mouseMsg);
    }
    return true;
  }
  return Inherited::EvSetCursor(hWndCursor, hitTest, mouseMsg);
}
//
TEditPos
TCoolTextWnd::Client2Text(const TPoint& point)
{
  int lineCount = GetNumLines();

  TEditPos p(0, GetTopLine() + point.y / GetLineHeight());
  if(p.row >= lineCount)
    p.row = lineCount - 1;
  if(p.row < 0)
    p.row = 0;

  int length = 0;
  LPCTSTR text = 0;
  if(p.row >= 0 && p.row < lineCount){
    length = GetLineLength(p.row);
    text   = GetLineText(p.row);
  }

  int pos = GetOffsetChar() + (point.x - GetMarginWidth()) / GetCharWidth();
  if(pos < 0)
    pos = 0;

  int index = 0;
  int curPos = 0;
  int tabSize = GetTabSize();
  while(index < length){
    if (text[index] == chTab)
      curPos += (tabSize - curPos % tabSize);
    else
      curPos++;

    if (curPos > pos)
      break;

    index++;
  }

  CHECK(index >= 0 && index <= length);
  p.col = index;
  return p;
}
//
TPoint
TCoolTextWnd::Text2Client(const TEditPos& pos)
{
  PRECONDITION(GetBuffer());
  //  VERIFY_TEXTPOS(pos);
  LPCTSTR text = GetLineText(pos.row);

  TPoint p(0,(pos.row - GetTopLine())*GetLineHeight());
  int tabSize = GetTabSize();
  for(int index = 0; index < pos.col; index ++){
    if (text[index] == chTab)
      p.x += (tabSize - p.x % tabSize);
    else
      p.x++;
  }
  p.x = (p.x - GetOffsetChar()) * GetCharWidth() + GetMarginWidth();
  return p;
}
//
void
TCoolTextWnd::ShowCursor()
{
  if(!IsFlagSet(cfNoUseCaret)){
    ClearFlag(cfCursorHidden);
    UpdateCaret();
    ShowCaret();
  }
}
//
void
TCoolTextWnd::HideCursor()
{
  if(!IsFlagSet(cfNoUseCaret)){
    SetFlag(cfCursorHidden);
    UpdateCaret();
    HideCaret();
  }
}
//
void
TCoolTextWnd::SetCursorPos(const TEditPos& newPos)
{
  VERIFY_TEXTPOS(newPos);
  CursorPos = newPos;
  IdealCharPos = CalculateActualOffset(CursorPos.row, CursorPos.col);
  UpdateCaret();
}
//
bool
TCoolTextWnd::CreateCaret(bool overType)
{
  if(overType)
    Inherited::CreateCaret(false, GetCharWidth(), GetLineHeight());
  else
    Inherited::CreateCaret(false, 3, GetLineHeight()-1);
  return true;
}
//
void
TCoolTextWnd::UpdateCaret()
{
  if(IsFlagSet(cfNoUseCaret))
    return;

  if (IsFlagSet(cfFocused) && !IsFlagSet(cfCursorHidden) &&
    CalculateActualOffset(CursorPos.row, CursorPos.col) >= GetOffsetChar())
  {
    if(!IsFlagSet(cfHasCaret))
    {
      if(CreateCaret(IsFlagSet(cfOverType)))
        SetFlag(cfHasCaret);
    }
    if(IsFlagSet(cfHasCaret))
    {
      SetCaretPos(Text2Client(CursorPos));
    }
    DisplayCursorPosition(CursorPos.row, CursorPos.col);
  }
}
//
int
TCoolTextWnd::CalculateActualOffset(int lineIndex, int charIndex)
{
  int length = GetLineLength(lineIndex);
  //PRECONDITION(charIndex >= 0 && charIndex <= length);
  LPCTSTR text = GetLineText(lineIndex);
  int offset = 0;
  int tabSize = GetTabSize();
  int i = 0;
  // for text
  for (; i < charIndex && i < length; i++){
    if (text[i] == chTab)
      offset += (tabSize - offset % tabSize);
    else
      offset++;
  }
  // after text
  for (; i < charIndex; i++)
    offset++;

  return offset;
}
//
void
TCoolTextWnd::EvKillFocus(HWND hWndGetFocus)
{
  ClearFlag(cfFocused);
  ClearFlag(cfHasCaret);
  if(!IsFlagSet(cfNoUseCaret)){
    HideCursor();
    DestroyCaret();
  }

  if (Selection)
    InvalidateLines(Selection->GetStart().row, Selection->GetEnd().row);
#if !defined(COOL_NODRAGDROP)
  if(IsFlagSet(cfDragPending)){
    ReleaseCapture();
    if(DragSelTimer)
      KillTimer(DragSelTimer);
    DragSelTimer = 0;
    ClearFlag(cfDragPending);
  }
#endif

  Inherited::EvKillFocus(hWndGetFocus);
}
//
void
TCoolTextWnd::EvSysColorChange()
{
  Inherited::EvSysColorChange();
  InvalidateLines(0, -1);
}
//
#if !defined(COOL_NODRAGDROP)
void TCoolTextWnd::EvTimer(uint timerId)
{
  if(timerId == COOL_TIMER_DEBOUNCE){
    CHECK(DragSelTimer);
    if(IsFlagSet(cfDragPending)){
      //_TRACE("TCoolTextWnd::EvTimer()\n");
      ExecuteDragDrop();
    }
  }
}
#endif
//
bool
TCoolTextWnd::ToggleInsertStatus()
{
  if(IsFlagSet(cfOverType))
    ClearFlag(cfOverType);
  else
    SetFlag(cfOverType);
  return true;
}
//
void
TCoolTextWnd::EnableOverType(bool enable)
{
  if(IsFlagSet(cfOverType) != enable){
    enable ? SetFlag(cfOverType) : ClearFlag(cfOverType);
    ClearFlag(cfHasCaret);
    ShowCursor();
  }
}
//
void
TCoolTextWnd::EvSetFocus(HWND hWndLostFocus)
{
  Inherited::EvSetFocus(hWndLostFocus);

  SetFlag(cfFocused);
  if (Selection)
    InvalidateLines(Selection->GetStart().row, Selection->GetEnd().row);
  ShowCursor();
}
//
int
TCoolTextWnd::GetMaxLineLength() const
{
  if (MaxLineLength == -1){
    TCoolTextWnd* wnd = CONST_CAST(TCoolTextWnd*,this);
    wnd->MaxLineLength = 0;
    int lineCount = GetNumLines();
    for (int i = 0; i < lineCount; i++){
      int actualLength = wnd->GetLineActualLength(i);
      if (MaxLineLength < actualLength)
        wnd->MaxLineLength = actualLength;
    }
  }
  return MaxLineLength;
}
//
int
TCoolTextWnd::GetLineActualLength(int lineIndex)
{
  PRECONDITION(GetBuffer());
  int lineCount = GetNumLines();
  CHECK(lineCount > 0);
  CHECK(lineIndex >= 0 && lineIndex < lineCount);
  if(!ActualLineLength){
    ActualLineLength = new int[lineCount];
    memset(ActualLineLength, 0xff, sizeof(int) * lineCount);
    ActualLengthArraySize = lineCount;
  }

  if(ActualLineLength[lineIndex] >= 0)
    return ActualLineLength[lineIndex];

  //  Actual line length is not determined yet, let's calculate a little
  int actualLength = 0;
  int length = GetLineLength(lineIndex);
  if(length > 0){
    LPCTSTR text = GetLineText(lineIndex);
    //Jogy: _alloca() -> alloca()
    LPTSTR pszChars = (LPTSTR)alloca(sizeof(TCHAR) * (length + 1));
    memcpy(pszChars, text, sizeof(TCHAR) * length);
    pszChars[length] = 0;
    LPTSTR pszCurrent = pszChars;

    int tabSize = GetTabSize();
    for (;;){
      LPTSTR psz = _tcschr(pszCurrent, chTab);
      if (!psz){
        actualLength += static_cast<int>(pszChars + length - pszCurrent);
        break;
      }

      actualLength += static_cast<int>(psz - pszCurrent);
      actualLength += (tabSize - actualLength % tabSize);
      pszCurrent = psz + 1;
    }
  }

  ActualLineLength[lineIndex] = actualLength;
  return actualLength;
}
//
void
TCoolTextWnd::GoToLine(int lineIndex, bool relative)
{
  int lineCount = GetNumLines() - 1;
  TEditPos cursorPos(CursorPos);
  if(relative)
    lineIndex += cursorPos.row;
  if(lineIndex)
    lineIndex--;
  if(lineIndex > lineCount)
    lineIndex = lineCount;
  if(lineIndex >= 0){
    int charIndex = GetLineLength(lineIndex);
    if (charIndex)
      charIndex--;

    if(cursorPos.col > charIndex)
      cursorPos.col = charIndex;
    if (cursorPos.col >= 0){
      cursorPos.row = lineIndex;
      VERIFY_TEXTPOS(cursorPos);
      SetCursorPos(cursorPos);
      SetSelection(TEditRange(cursorPos, cursorPos));
      ScrollToCaret(cursorPos);
    }
  }
}
//
void
TCoolTextWnd::ClearSelection()
{
  if(Selection){
    InvalidateLines(Selection->GetStart().row, Selection->GetEnd().row);
    delete Selection;
    Selection = 0;
  }
}
//
void
TCoolTextWnd::SetSelection(const TEditRange& range)
{
  VERIFY_TEXTPOS(range.Start());
  VERIFY_TEXTPOS(range.End());
  if(Selection && Selection->GetStart().row == range.srow){
    if(Selection->GetStart().row != range.erow)
      InvalidateLines(range.erow, Selection->GetEnd().row);
  }
  else{
    InvalidateLines(range.srow, range.erow);
    if(Selection)
      InvalidateLines(Selection->GetStart().row, Selection->GetEnd().row);
  }
  delete Selection;
  Selection = 0;
  if(range.Start() == range.End())
    return;
  Selection = CreateSelection(range);
}
//
TCoolTextWnd::TTextSelection*
TCoolTextWnd::CreateSelection(const TEditRange& range)
{
  switch(SelType){
  case stStreamSelection:
    return new TStreamSelection(this, range);
  case stLineSelection:
    return new TLineSelection(this, range.srow, range.srow);
  case stColumnSelection  :
    return new TColumnSelection(this, range);
  }
  return 0;
}
//
void
TCoolTextWnd::InvalidateLines(int index1, int index2, bool invalidateMargin)
{
  invalidateMargin = true;
  if(index2 == -1){
    TRect rect;
    GetClientRect(rect);
    if (!invalidateMargin)
      rect.left += GetMarginWidth();
    rect.top = (index1 - GetTopLine()) * GetLineHeight();
    InvalidateRect(rect, false);
  }
  else{
    if (index2 < index1)
      std::swap(index1, index2);
    TRect rect;
    GetClientRect(rect);
    if (!invalidateMargin)
      rect.left += GetMarginWidth();
    rect.top = (index1 - GetTopLine()) * GetLineHeight();
    rect.bottom = (index2 - GetTopLine() + 1) * GetLineHeight();
    InvalidateRect(rect, false);
  }
}
//
int
TCoolTextWnd::GetScreenLines()
{
  if (ScreenLines == -1){
    TRect rect;
    GetClientRect(rect);
    ScreenLines = rect.Height() / GetLineHeight();
  }
  return ScreenLines;
}
//
int
TCoolTextWnd::GetScreenChars()
{
  if (ScreenChars == -1){
    TRect rect;
    GetClientRect(rect);
    ScreenChars = (rect.Width() - GetMarginWidth()) / GetCharWidth();
  }
  return ScreenChars;
}
//
void
TCoolTextWnd::ToggleBookmark(int bookmarkId)
{
  TCoolTextBuffer* buffer = GetBuffer();
  PRECONDITION(buffer);
  PRECONDITION(bookmarkId >= 0 && bookmarkId <= 9);
  uint32 flags = buffer->GetLineFlags(CursorPos.row);
  uint32 mask = LF_BOOKMARKID(bookmarkId);
  int retval = buffer->SetLineFlag(CursorPos.row, mask, (flags & mask)==0);
  if(retval >= 0)
    InvalidateLines(retval,retval,true);
  InvalidateLines(CursorPos.row,CursorPos.row,true);
}
//
void
TCoolTextWnd::GoBookmark(int bookmarkId)
{
  PRECONDITION(bookmarkId >= 0 && bookmarkId <= 9);
  PRECONDITION(GetBuffer());
  int lineIdx = GetBuffer()->GetLineWithFlag(LF_BOOKMARKID(bookmarkId));
  if(lineIdx >= 0){
    TEditPos pos(0, lineIdx);
    VERIFY_TEXTPOS(pos);
    SetCursorPos(pos);
    SetSelection(TEditRange(pos, pos));
    ScrollToCaret(CursorPos);
  }
}
//
void
TCoolTextWnd::ClearBookmarks()
{
  TCoolTextBuffer* buffer = GetBuffer();
  PRECONDITION(buffer);
  for(int bookmarkId = 0; bookmarkId <= 9; bookmarkId++){
    int lineIdx = buffer->GetLineWithFlag(LF_BOOKMARKID(bookmarkId));
    if (lineIdx >= 0){
      buffer->SetLineFlag(lineIdx, LF_BOOKMARKID(bookmarkId), false);
      InvalidateLines(CursorPos.row,CursorPos.row,true);
    }
  }
}
//
void
TCoolTextWnd::ToggleRandBookmark()
{
  TCoolTextBuffer* buffer = GetBuffer();
  PRECONDITION(buffer);
  uint32 flags = buffer->GetLineFlags(CursorPos.row);
  uint32 mask  = TCoolTextBuffer::lfBookMarks;
  buffer->SetLineFlag(CursorPos.row, mask, (flags & mask) == 0, false);

  int lineIdx = buffer->GetLineWithFlag(TCoolTextBuffer::lfBookMarks);
  if (lineIdx >= 0)
    SetFlag(cfBookmarkExist);
  else
    ClearFlag(cfBookmarkExist);
  InvalidateLines(CursorPos.row, CursorPos.row, true);
}
//
void
TCoolTextWnd::NextRandBookmark()
{
  PRECONDITION(GetBuffer());
  int lineIndex = GetBuffer()->FindNextBookmarkLine(CursorPos.row);
  if (lineIndex >= 0) {
    TEditPos pos(0, lineIndex);
    VERIFY_TEXTPOS(pos);
    SetCursorPos(pos);
    SetSelection(TEditRange(pos, pos));
    ScrollToCaret(CursorPos);
  }
}
//
void
TCoolTextWnd::PrevRandBookmark()
{
  PRECONDITION(GetBuffer());
  int lineIndex = GetBuffer()->FindPrevBookmarkLine(CursorPos.row);
  if (lineIndex >= 0){
    TEditPos pos(0, lineIndex);
    VERIFY_TEXTPOS(pos);
    SetCursorPos(pos);
    SetSelection(TEditRange(pos, pos));
    ScrollToCaret(CursorPos);
  }
}
//
void
TCoolTextWnd::ClearRandBookmarks()
{
  PRECONDITION(GetBuffer());
  TCoolTextBuffer* buffer = GetBuffer();
  int lineCount = GetNumLines();
  for (int i = 0; i < lineCount; i++){
    if(buffer->GetLineFlags(i) & TCoolTextBuffer::lfBookMarks)
      buffer->SetLineFlag(i, TCoolTextBuffer::lfBookMarks, false);
  }
  ClearFlag(cfBookmarkExist);
}
//
void
TCoolTextWnd::EnablerNextBookmark(TCommandEnabler& tce)
{
  tce.Enable(IsFlagSet(cfBookmarkExist));
}
//
void
TCoolTextWnd::EnablerPrevBookmark(TCommandEnabler& tce)
{
  tce.Enable(IsFlagSet(cfBookmarkExist));
}
//
void
TCoolTextWnd::EnablerClearRandBookmarks(TCommandEnabler& tce)
{
  tce.Enable(IsFlagSet(cfBookmarkExist));
}
//
int
TCoolTextWnd::GetTopLine()
{
  return Scroller->YPos;
}
//
int
TCoolTextWnd::GetOffsetChar()
{
  return Scroller->XPos;
}
//
void
TCoolTextWnd::EvHScroll(uint scrollCode, uint thumbPos, HWND hWndCtl)
{
  Inherited::EvHScroll(scrollCode,thumbPos,hWndCtl);
  if(IsFlagSet(cfHasCaret))
    SetCaretPos(Text2Client(CursorPos));
}
//
void
TCoolTextWnd::EvVScroll(uint scrollCode, uint thumbPos, HWND hWndCtl)
{
  Inherited::EvVScroll(scrollCode,thumbPos,hWndCtl);
  if(IsFlagSet(cfHasCaret))
    SetCaretPos(Text2Client(CursorPos));
}
//
void
TCoolTextWnd::EvMouseWheel(uint modKeys, int zDelta, const TPoint& point)
{
  Inherited::EvMouseWheel(modKeys, zDelta, point);
  if(IsFlagSet(cfHasCaret))
    SetCaretPos(Text2Client(CursorPos));
}
//
void
TCoolTextWnd::AdjustTextPoint(TPoint& point)
{
  point.x += GetCharWidth()/ 2; //todo
}
//
void
TCoolTextWnd::SelectAll()
{
  int lineCount = GetNumLines();
  CursorPos.col = GetLineLength(lineCount - 1);
  CursorPos.row = lineCount - 1;
  SetSelection(TEditRange(0, 0, CursorPos.col, CursorPos.row));
  UpdateCaret();
}
//
void
TCoolTextWnd::DrawDraggedCurb(int position, bool redraw)
{
  TRect rect;
  GetClientRect(rect);
  TClientDC dc(GetHandle());
  if(redraw){
    rect.left    = TmpCurbPos-1;
    rect.right  = TmpCurbPos+1;
    dc.DrawFocusRect(rect);
  }
  if(position < GetMarginWidth())
    position = GetMarginWidth();
  position = (position-GetMarginWidth())/GetCharWidth()*GetCharWidth()+GetMarginWidth();
  rect.left   = position-1;
  rect.right = position+1;
  dc.DrawFocusRect(rect);
  TmpCurbPos = position;
}
//
void
TCoolTextWnd::EvLButtonDown(uint modKeys, const TPoint& point)
{
  //_TRACE("TCoolTextWnd::EvLButtonDown()\n");
  bool needRedraw = false;
  bool bShift   = modKeys & MK_SHIFT;
  bool bControl = modKeys & MK_CONTROL;
  bool bAlt     = GetKeyState(VK_MENU)&0x8000;

  //We don't need to capture the mouse - TScroller does it.
  SetFlag(cfMouseDown);
  if(point.x < GetMarginWidth()){
    TPoint p(point);
    AdjustTextPoint(p);
    if(bControl){
      SelectAll();
      ClearFlag(cfMouseDown);
      return;
    }
    else{
      CursorPos   = Client2Text(p);
      CursorPos.col = 0; // set new position on start of line
      TEditPos endPos(CursorPos);
      endPos.col  = GetLineLength(CursorPos.row); //  Force full line
      SetSelection(TEditRange(CursorPos, endPos));
      UpdateCaret();
      Inherited::EvLButtonDown(modKeys, p);
      return;
    }
  }
  // check Curb
  int lineCurb = CurbPos == -1 ? -1 : ((CurbPos-GetOffsetChar())*GetCharWidth()+GetMarginWidth());
  if(lineCurb >=0 && (point.x < (int)(lineCurb+3) && point.x > (int)(lineCurb-3))){
    SetFlag(cfCurbDrag);
    DrawDraggedCurb(point.x);
    Inherited::EvLButtonDown(modKeys, point);
    return;
  }

  CursorPos = Client2Text(point);

  if(LastHit == point){
    if(!Selection){
      TEditPos startSel = StartWord(CursorPos);
      CursorPos         = EndWord(CursorPos);
      if(startSel.Valid())
        SetSelection(TEditRange(startSel, CursorPos));
    }
    else{
      CursorPos.col = 0; // set new position on start of line
      TEditPos endPos(CursorPos);
      endPos.col  = GetLineLength(CursorPos.row); //  Force full line
      SetSelection(TEditRange(CursorPos, endPos));
      LastHit = TPoint(-1,-1);
    }
    ClearFlag(cfMouseDown);
    UpdateCaret();
    InvalidateLines(CursorPos.row, CursorPos.row);
    return;
  }

  LastHit = point;

  if(Selection){
    if(bShift)
      Selection->Extend(CursorPos);
    else{
#if !defined(COOL_NODRAGDROP)
      TEditPos textPos = Client2Text(point);
      if(!IsFlagSet(cfNoDragDrop) && IsInSelection(textPos)){
        //_TRACE("TCoolTextWnd::EvLButtonDown() Start pending\n");
        SavedDragPos = point;
        SetFlag(cfDragPending);
        DragSelTimer = SetTimer(COOL_TIMER_DEBOUNCE, DragDropSupport.GetDelay());
      }
      else
#endif
      {
        delete Selection;
        Selection = 0;
      }
    }
    needRedraw = true;
  }
  // start selection
  if(!Selection){
    TSelType selType = SelType;
    SelType = stStreamSelection;
    if(bAlt)
      SelType = stColumnSelection;
    Selection = CreateSelection(TEditRange(CursorPos, CursorPos));
    SelType = selType;
  }
  UpdateCaret();
  if(needRedraw){
    InvalidateLines(0, -1, false);
  }
  Inherited::EvLButtonDown(modKeys, point);
}
//
#if !defined(COOL_NODRAGDROP)
void
TCoolTextWnd::ExecuteDragDrop()
{
  //_TRACE("TCoolTextWnd::ExecuteDragDrop() START\n");
  if(DragSelTimer)
    KillTimer(DragSelTimer);
  DragSelTimer = 0;
  ClearFlag(cfMouseDown);
  ClearFlag(cfDragPending);
  ReleaseCapture();
  if (SetDragData()){
    SetFlag(cfDraggingText);
    Scroller->AutoMode = false; //workaround
    //_TRACE("TCoolTextWnd::ExecuteDragDrop() Scroller->AutoMode = false\n");
    DROPEFFECT de = DragDropSupport.DoDragDrop(GetDropEffect());
    if (de != DROPEFFECT_NONE)
      DropSource(de);
    ClearFlag(cfDraggingText);
    DragDropProxy->ResetDragData();
    //_TRACE("TCoolTextWnd::ExecuteDragDrop() ResetDragData\n");
  }
  //_TRACE("TCoolTextWnd::ExecuteDragDrop() END\n");
}
//
TDragDropProxy*
TCoolTextWnd::SetDragDropProxy(TDragDropProxy* proxy)
{
  TDragDropProxy* tmp = DragDropProxy;
  DragDropProxy = proxy;
  DragDropSupport.SetProxy(DragDropProxy);
  return tmp;
}
#endif
//
void
TCoolTextWnd::EvMouseMove(uint modKeys, const TPoint& point)
{
  if(IsFlagSet(cfMouseDown)){
    if(IsFlagSet(cfCurbDrag)){
      DrawDraggedCurb(point.x, true);
      Inherited::EvMouseMove(modKeys, point);
      return;
    }

    TEditPos startPos = CursorPos;
    CursorPos = Client2Text(point);
    if(point.x < GetMarginWidth()){
      if(Selection){
        TEditRange range(Selection->GetStart(), CursorPos);
        range.ecol  = GetLineLength(CursorPos.row); //  Force full line
        CursorPos.col = 0;
        SetSelection(range);
      }
    }
    else{
#if !defined(COOL_NODRAGDROP)
      if(IsFlagSet(cfDragPending)){
        if(DragDropSupport.CanDragDrop(SavedDragPos, point))
          ExecuteDragDrop();
      }
      else
#endif
        if(Selection)
          Selection->Extend(CursorPos);
    }
    UpdateCaret();
    InvalidateLines(startPos.row, CursorPos.row);
    UpdateWindow();
  }
  Inherited::EvMouseMove(modKeys, point);
}
//
void
TCoolTextWnd::EvLButtonUp(uint modKeys, const TPoint& point)
{
  //_TRACE("TCoolTextWnd::EvLButtonUp() START\n");
#if !defined(COOL_NODRAGDROP)
  // Workaround in conflict Scroller->AutoMode  and Drag-and-Drop
  if(!Scroller->AutoMode){
    Scroller->AutoMode = true;
    return;
  }
#endif
  if(IsFlagSet(cfMouseDown)){
    ClearFlag(cfMouseDown);
    if(IsFlagSet(cfCurbDrag)){
      ClearFlag(cfCurbDrag);
      CurbPos = (((point.x-GetMarginWidth())/GetCharWidth()*GetCharWidth())/GetCharWidth())+GetOffsetChar();
      DrawDraggedCurb(TmpCurbPos);
      Inherited::EvLButtonUp(modKeys, point);
      InvalidateLines(0, -1, false);
      return;
    }

    InvalidateLines(0, -1, false);
    CursorPos = Client2Text(point);
    if(point.x < GetMarginWidth()){
      if(Selection){
        TEditRange range(Selection->GetStart(), CursorPos);
        range.ecol  = GetLineLength(CursorPos.row); //  Force full line
        CursorPos.col = 0;
        SetSelection(range);
      }
    }
    else{
#if !defined(COOL_NODRAGDROP)
      if(IsFlagSet(cfDragPending)){
        ClearFlag(cfDragPending);
        delete Selection;
        Selection = 0;
      }
      else
#endif
        if(Selection)
          Selection->Extend(CursorPos);
    }
#if !defined(COOL_NODRAGDROP)
    if(DragSelTimer)
      KillTimer(DragSelTimer);
    DragSelTimer = 0;
#endif

    PostCheckSelection();
    ReleaseCapture();
    UpdateCaret();
  }
  //_TRACE("TCoolTextWnd::EvLButtonUp() Scroller->AutoMode = true\n");
  Inherited::EvLButtonUp(modKeys, point);
}
//
void
TCoolTextWnd::EvLButtonDblClk(uint modKeys, const TPoint& point)
{
  LastHit    = point;
  CursorPos = Client2Text(point);
  TEditPos startSel = StartWord(CursorPos);
  CursorPos         = EndWord(CursorPos);
  if(startSel.Valid())
    SetSelection(TEditRange(startSel, CursorPos));
  ClearFlag(cfMouseDown);
  UpdateCaret();
  InvalidateLines(CursorPos.row, CursorPos.row);
  Inherited::EvLButtonDblClk(modKeys, point);
}
//
void
TCoolTextWnd::ResetView()
{
  ClearFlag(cfBookmarkExist);
  MaxLineLength     = -1;
  IdealCharPos      = -1;
  SelType           = stStreamSelection;
  delete Selection;
  Selection         = 0;
  delete[] ParseCookies;
  ParseCookies      = 0;
  ParseArraySize    = 0;
  ActualLengthArraySize = 0;
  ActualLineLength  = 0;
  CursorPos.row      = 0;
  CursorPos.col      = 0;
  LastHit.x          = -1;
  LastHit.y          = -1;
  if(Scroller){
    Scroller->XPos = 0;
    Scroller->YPos = 0;
  }

  if(GetHandle())
    UpdateCaret();
}
//
void
TCoolTextWnd::ScrollToCaret(const TEditPos& cursorPos)
{
  int dx=0,dy=0;

  VERIFY_TEXTPOS(cursorPos);
  TPoint caretPos = Text2Client(cursorPos);
  TRect  clientRect;
  GetClientRect(clientRect);
  if(clientRect.IsEmpty())
    return;

  //
  if(Scroller->YRange){
    clientRect.bottom -= TUIMetric::CyHScroll;
    if(clientRect.bottom%GetLineHeight())
      clientRect.bottom -= clientRect.bottom%GetLineHeight();
  }
  if(Scroller->XRange){
    clientRect.right -= TUIMetric::CyVScroll;
    if(clientRect.right%GetCharWidth())
      clientRect.right -= clientRect.right%GetCharWidth();
  }
  clientRect.left += GetMarginWidth();

  if(!clientRect.Contains(caretPos)){
    if(caretPos.x > clientRect.right)
      dx = caretPos.x - clientRect.right;
    else if (caretPos.x < clientRect.left)
      dx = caretPos.x - clientRect.left;
    if (caretPos.y > clientRect.bottom)
      dy = caretPos.y - clientRect.bottom;
    else if (caretPos.y < clientRect.top)
      dy = caretPos.y - clientRect.top;

    Scroller->ScrollBy(dx/GetCharWidth(),dy/GetLineHeight());

    VERIFY_TEXTPOS(cursorPos);
    caretPos = Text2Client(cursorPos);

  }
  SetCursorPos(Client2Text(caretPos));
}
//
#if __DEBUG > 0
void
TCoolTextWnd::VerifyTextPos(const TEditPos& pos)
{
  if (GetNumLines() > 0){
    CHECK(GetTopLine() >= 0 && GetOffsetChar() >= 0);
    CHECK(pos.row >= 0 && pos.row < GetNumLines());
    CHECK(pos.col >= 0 && (IsSmartCursor() && pos.col <= GetLineLength(pos.row)));
  }
}
#endif
//
void
TCoolTextWnd::EvKeyDown(uint key, uint repeatCount, uint /*flags*/)
{
#if 1
  bool is_shift = ::GetAsyncKeyState(VK_SHIFT) & MSB;
  bool is_ctrl  = ::GetAsyncKeyState(VK_CONTROL) & MSB;
#else
  bool is_shift = flags & MK_SHIFT;  //?????
  bool is_ctrl  = flags & MK_CONTROL;// ????
#endif
  switch (key){
  case VK_ESCAPE:
    KeyEscape(repeatCount);
    break;
  case VK_RIGHT:
    if(is_shift || IsColumnSelMode()){
      if(is_ctrl)
        ExtendWordRight(repeatCount);
      else
        ExtendRight(repeatCount);
    }
    else{
      ClearSelection();
      if(is_ctrl)
        MoveWordRight(repeatCount);
      else
        MoveRight(repeatCount);
    }
    break;
  case VK_LEFT:
    if(is_shift || IsColumnSelMode()){
      if(is_ctrl)
        ExtendWordLeft(repeatCount);
      else
        ExtendLeft(repeatCount);
    }
    else{
      ClearSelection();
      if(is_ctrl)
        MoveWordLeft(repeatCount);
      else{
        MoveLeft(repeatCount);
      }
    }
    break;
  case VK_DOWN:
    if(is_ctrl){
      if(!is_shift)
        ScrollDown(repeatCount);
    }
    else{
      if(is_shift || IsColumnSelMode())
        ExtendDown(repeatCount);
      else{
        ClearSelection();
        MoveDown(repeatCount);
      }
    }
    break;

  case VK_UP:
    if(is_ctrl){
      if(!is_shift)
        ScrollUp(repeatCount);
    }
    else{
      if(is_shift || IsColumnSelMode())
        ExtendUp(repeatCount);
      else{
        ClearSelection();
        MoveUp(repeatCount);
      }
    }
    break;

  case VK_NEXT: //Page Down
    if(!is_ctrl){
      if(is_shift || IsColumnSelMode())
        ExtendPgDown(repeatCount);
      else{
        ClearSelection();
        MovePgDown(repeatCount);
      }
    }
    break;

  case VK_PRIOR:  //Page up
    if(!is_ctrl){
      if(is_shift || IsColumnSelMode())
        ExtendPgUp(repeatCount);
      else{
        ClearSelection();
        MovePgUp(repeatCount);
      }
    }
    break;

  case VK_HOME:
    if(is_shift || IsColumnSelMode()){
      if(is_ctrl)
        ExtendCtrlHome(repeatCount);
      else
        ExtendHome(repeatCount);
    }
    else{
      ClearSelection();
      if(is_ctrl)
        MoveCtrlHome(repeatCount);
      else
        MoveHome(repeatCount);
    }
    break;

  case VK_END:
    if(is_shift || IsColumnSelMode()){
      if(is_ctrl)
        ExtendCtrlEnd(repeatCount);
      else
        ExtendEnd(repeatCount);
    }
    else{
      ClearSelection();
      if(is_ctrl)
        MoveCtrlEnd(repeatCount);
      else
        MoveEnd(repeatCount);
    }
    break;
    //Also, we toggle the insert mode bit here
  case VK_INSERT:
    if(ToggleInsertStatus()){
      ClearFlag(cfHasCaret);
      ShowCursor();
    }
    break;

    //Anything else is a character key - let windows have it
    //back and we will catch it in EvChar.
  default:
    DefaultProcessing();
    return; //NOTE - return not break!
  }
  //Inherited::EvKeyDown(key, repeatCount, flags); //????
}
//
void
TCoolTextWnd::KeyEscape(int /*repeatCount*/)
{
  if(!IsStreamSelMode())
    SelType = stStreamSelection;
  else if(Selection)
    ClearSelection();
  else
    DefaultProcessing();
}
//
void
TCoolTextWnd::ExtendWordRight(int repeatCount)
{
  int lastLine = GetNumLines()-1;
  if(CursorPos.row < lastLine ||
    (CursorPos.row == lastLine && CursorPos.col < GetLineLength(lastLine))){
      PreCheckSelection();
      MoveWordRight(repeatCount);
      Selection->Extend(CursorPos);
      PostCheckSelection();
  }
}
//
void
TCoolTextWnd::ExtendRight(int repeatCount)
{
  int lastLine = GetNumLines()-1;
  if(CursorPos.row < lastLine ||
    (CursorPos.row == lastLine && CursorPos.col < GetLineLength(lastLine))){
      PreCheckSelection();
      int startPos = CursorPos.row;
      CursorPos.col += repeatCount;
      if( IsFlagSet(cfSmartCursor) && !IsColumnSelMode() &&
        CursorPos.col > GetLineLength(CursorPos.row)){
          if(CursorPos.row < GetNumLines()-1){
            CursorPos.row++;
            CursorPos.col = 0;
          }
      }
      Selection->Extend(CursorPos);
      PostCheckSelection();
      ScrollToCaret(CursorPos);
      InvalidateLines(startPos, CursorPos.row);
  }
}
//
void
TCoolTextWnd::MoveWordRight(int repeatCount)
{
  int lastLine = GetNumLines()-1;
  if(CursorPos.row < lastLine ||
    (CursorPos.row == lastLine && CursorPos.col < GetLineLength(lastLine))){
      TEditPos pos(CursorPos);
      while(repeatCount--){
        if(pos.col == GetLineLength(pos.row)){
          pos.col = 0;
          if(pos.row < GetNumLines())
            pos.row++;
        }
        pos = NextBreak(pos);
      }

      InvalidateLines(CursorPos.row, pos.row);
      CursorPos = pos;

      IdealCharPos = CalculateActualOffset(CursorPos.row, CursorPos.col);
      ScrollToCaret(CursorPos);
  }
}
//
void
TCoolTextWnd::MoveRight(int repeatCount)
{
  int lastLine = GetNumLines()-1;
  if(CursorPos.row < lastLine ||
    (CursorPos.row == lastLine && CursorPos.col < GetLineLength(lastLine))){
      int startPos = CursorPos.row;
      CursorPos.col += repeatCount;
      if(IsFlagSet(cfSmartCursor) && CursorPos.col > GetLineLength(CursorPos.row)){
        if(CursorPos.row < GetNumLines()-1){
          CursorPos.row++;
          CursorPos.col = 0;
        }
      }
      IdealCharPos = CalculateActualOffset(CursorPos.row, CursorPos.col);
      ScrollToCaret(CursorPos);
      InvalidateLines(startPos, CursorPos.row);
  }
}
//
void
TCoolTextWnd::ExtendWordLeft(int repeatCount)
{
  if(CursorPos.row > 0 || (CursorPos.row == 0 && CursorPos.col > 0)){
    PreCheckSelection();
    MoveWordLeft(repeatCount);
    Selection->Extend(CursorPos);
    PostCheckSelection();
  }
}
//
void
TCoolTextWnd::PreCheckSelection()
{
  if(!Selection)
    Selection = CreateSelection(TEditRange(CursorPos, CursorPos));
}
//
void
TCoolTextWnd::PostCheckSelection()
{
  if(Selection && Selection->IsEmpty() && !IsColumnSelMode()){
    delete Selection;
    Selection = 0;
  }
}
//
void
TCoolTextWnd::ExtendLeft(int repeatCount)
{
  if(CursorPos.row > 0 || (CursorPos.row == 0 && CursorPos.col > 0)){
    int startPos = CursorPos.row;
    PreCheckSelection();
    CursorPos.col -= repeatCount;
    if(CursorPos.col < 0){
      if(CursorPos.row > 0 && !IsColumnSelMode() &&
        IsFlagSet(cfSmartCursor)){
          CursorPos.row--;
          CursorPos.col = GetLineLength(CursorPos.row);
      }
      else
        CursorPos.col = 0;
    }
    IdealCharPos = CalculateActualOffset(CursorPos.row, CursorPos.col);
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);

    Selection->Extend(CursorPos);
    PostCheckSelection();
  }
}
//
void
TCoolTextWnd::MoveWordLeft(int repeatCount)
{
  if(CursorPos.row>0 || (CursorPos.row==0 && CursorPos.col>0)){
    TEditPos pos = CursorPos;
    while(repeatCount--){
      if(pos.col <= 0){
        if(pos.row > 0)
          pos.row--;
        pos.col = GetLineLength(pos.row);
      }
      pos = PrevBreak(pos);
    }

    InvalidateLines(CursorPos.row, pos.row);
    CursorPos = pos;

    IdealCharPos = CalculateActualOffset(CursorPos.row, CursorPos.col);
    ScrollToCaret(CursorPos);
  }
}
//
void
TCoolTextWnd::MoveLeft(int repeatCount)
{
  if(CursorPos.row>0 || (CursorPos.row==0 && CursorPos.col>0)){
    int startPos = CursorPos.row;
    CursorPos.col -= repeatCount;
    if(CursorPos.col < 0){
      if(CursorPos.row > 0 && IsFlagSet(cfSmartCursor)){
        CursorPos.row--;
        CursorPos.col = GetLineLength(CursorPos.row);
      }
      else
        CursorPos.col = 0;
    }
    IdealCharPos = CalculateActualOffset(CursorPos.row, CursorPos.col);
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//
void
TCoolTextWnd::ExtendDown(int repeatCount)
{
  if(CursorPos.row < GetNumLines()-1){
    int startPos = CursorPos.row;
    PreCheckSelection();
    CursorPos.row += repeatCount;
    if(CursorPos.row > GetNumLines()-1)
      CursorPos.row = GetNumLines()-1;
    if(!IsColumnSelMode())
      AdjustXCurPos();
    Selection->Extend(CursorPos);
    PostCheckSelection();
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//
void
TCoolTextWnd::MoveDown(int repeatCount)
{
  if(CursorPos.row < GetNumLines()-1){
    int startPos = CursorPos.row;
    CursorPos.row += repeatCount;
    if(CursorPos.row > GetNumLines()-1)
      CursorPos.row = GetNumLines()-1;
    AdjustXCurPos();
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//
void
TCoolTextWnd::ExtendUp(int repeatCount)
{
  if(CursorPos.row > 0){
    int startPos = CursorPos.row;
    PreCheckSelection();
    CursorPos.row -= repeatCount;
    if(CursorPos.row < 0)
      CursorPos.row = 0;
    if(!IsColumnSelMode())
      AdjustXCurPos();
    Selection->Extend(CursorPos);
    PostCheckSelection();
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//
void
TCoolTextWnd::MoveUp(int repeatCount)
{
  if(CursorPos.row > 0){
    int startPos = CursorPos.row;
    CursorPos.row -= repeatCount;
    if(CursorPos.row < 0)
      CursorPos.row = 0;
    AdjustXCurPos();
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//
void
TCoolTextWnd::ExtendPgDown(int repeatCount)
{
  if(CursorPos.row < GetNumLines()-1){
    int startPos  = CursorPos.row;
    int topLine   = GetTopLine();
    int delta     = repeatCount*GetScreenLines() - 1;
    int newTopLine= topLine + delta;
    if(newTopLine >= GetNumLines())
      newTopLine = GetNumLines() - 1;

    if(topLine != newTopLine){
      PreCheckSelection();
      CursorPos.row += delta;
      if(CursorPos.row >= GetNumLines())
        CursorPos.row = GetNumLines() - 1;
      if(!IsColumnSelMode())
        AdjustXCurPos();
      Scroller->ScrollBy(0, newTopLine-topLine);
      Selection->Extend(CursorPos);
      PostCheckSelection();
      ScrollToCaret(CursorPos);
      InvalidateLines(startPos, CursorPos.row);
    }
  }
}
//
void
TCoolTextWnd::MovePgDown(int repeatCount)
{
  if(CursorPos.row < GetNumLines()-1){
    int startPos = CursorPos.row;
    int topLine   = GetTopLine();
    int delta     = repeatCount*GetScreenLines() - 1;
    int newTopLine= topLine + delta;
    if(newTopLine >= GetNumLines())
      newTopLine = GetNumLines() - 1;

    if(topLine != newTopLine){
      CursorPos.row += delta;
      if(CursorPos.row >= GetNumLines())
        CursorPos.row = GetNumLines() - 1;
      AdjustXCurPos();
      Scroller->ScrollBy(0, newTopLine-topLine);
      ScrollToCaret(CursorPos);
      InvalidateLines(startPos, CursorPos.row);
    }
  }
}
//
void
TCoolTextWnd::ExtendPgUp(int repeatCount)
{
  if(CursorPos.row > 0){
    int startPos = CursorPos.row;
    int topLine = GetTopLine();
    int delta   = repeatCount*GetScreenLines() - 1;
    int newTopLine = topLine - delta;

    if (newTopLine < 0)
      newTopLine = 0;

    if(topLine != newTopLine || CursorPos.row>0){
      PreCheckSelection();
      CursorPos.row -= delta;
      if(CursorPos.row < 0)
        CursorPos.row = 0;
      if(!IsColumnSelMode())
        AdjustXCurPos();
      Scroller->ScrollBy(0, newTopLine-topLine);
      Selection->Extend(CursorPos);
      PostCheckSelection();
      ScrollToCaret(CursorPos);
      InvalidateLines(startPos, CursorPos.row);
    }
  }
}
//
void
TCoolTextWnd::MovePgUp(int repeatCount)
{
  if(CursorPos.row > 0){
    int startPos = CursorPos.row;
    int topLine = GetTopLine();
    int delta   = repeatCount*GetScreenLines() - 1;
    int newTopLine = topLine - delta;

    if (newTopLine < 0)
      newTopLine = 0;

    if(topLine != newTopLine || CursorPos.row>0){
      CursorPos.row -= delta;
      if(CursorPos.row < 0)
        CursorPos.row = 0;
      AdjustXCurPos();
      Scroller->ScrollBy(0, newTopLine-topLine);
      ScrollToCaret(CursorPos);
      InvalidateLines(startPos, CursorPos.row);
    }
  }
}
//
void
TCoolTextWnd::ExtendCtrlHome(int /*repeatCount*/)
{
  if(CursorPos.col || CursorPos.row){
    int startPos = CursorPos.row;
    PreCheckSelection();
    CursorPos.col = 0;
    CursorPos.row = 0;
    Selection->Extend(CursorPos);
    PostCheckSelection();
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//
void
TCoolTextWnd::MoveCtrlHome(int /*repeatCount*/)
{
  if(CursorPos.col || CursorPos.row){
    int startPos = CursorPos.row;
    CursorPos.col = 0;
    CursorPos.row = 0;
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//
void
TCoolTextWnd::ExtendHome(int /*repeatCount*/)
{
  if(CursorPos.col){
    int startPos = CursorPos.row;
    PreCheckSelection();
    CursorPos.col = 0;
    Selection->Extend(CursorPos);
    PostCheckSelection();
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//
void
TCoolTextWnd::MoveHome(int /*repeatCount*/)
{
  if(CursorPos.col){
    int startPos = CursorPos.row;
    CursorPos.col = 0;
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//
void
TCoolTextWnd::ExtendCtrlEnd(int /*repeatCount*/)
{
  int endLine = GetNumLines()-1;
  TEditPos endPos(GetLineLength(endLine), endLine);
  if(CursorPos != endPos){
    int startPos = CursorPos.row;
    PreCheckSelection();
    CursorPos = endPos;
    Selection->Extend(CursorPos);
    PostCheckSelection();
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//
void
TCoolTextWnd::MoveCtrlEnd(int /*repeatCount*/)
{
  int endLine = GetNumLines()-1;
  TEditPos endPos(GetLineLength(endLine), endLine);
  if(CursorPos != endPos){
    int startPos = CursorPos.row;
    CursorPos = endPos;
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//
void
TCoolTextWnd::ExtendEnd(int /*repeatCount*/)
{
  int endPos = GetLineLength(CursorPos.row);
  if(CursorPos.col != endPos){
    PreCheckSelection();
    CursorPos.col = endPos;
    Selection->Extend(CursorPos);
    PostCheckSelection();
    ScrollToCaret(CursorPos);
    InvalidateLines(CursorPos.row, CursorPos.row);
  }
}
//
void
TCoolTextWnd::MoveEnd(int /*repeatCount*/)
{
  int endPos = GetLineLength(CursorPos.row);
  if(CursorPos.col != endPos){
    CursorPos.col = endPos;
    ScrollToCaret(CursorPos);
    InvalidateLines(CursorPos.row, CursorPos.row);
  }
}
//
void
TCoolTextWnd::ScrollDown(int repeatCount)
{
  if(Scroller->YPos < Scroller->YRange){
    if((int)repeatCount > (Scroller->YRange-Scroller->YPos))
      repeatCount = Scroller->YRange - Scroller->YPos;
    Scroller->ScrollBy(0,repeatCount);
    CursorPos.row += repeatCount;
    if(CursorPos.row >= GetNumLines())
      CursorPos.row = GetNumLines()-1;
    if(IsFlagSet(cfSmartCursor)){
      int len = GetLineLength(CursorPos.row);
      if(CursorPos.col > len)
        CursorPos.col = len;
    }
    SetCursorPos(CursorPos);
  }
}
//
void
TCoolTextWnd::ScrollUp(int repeatCount)
{
  if (Scroller->YPos > 0)
    {
    if ((int)repeatCount > Scroller->YPos)
      repeatCount = Scroller->YPos;
    Scroller->ScrollBy(0,-(int)repeatCount);
    CursorPos.row -= repeatCount;
    if(CursorPos.row < 0)
      CursorPos.row = 0;
    if(IsFlagSet(cfSmartCursor))
        {
      int len = GetLineLength(CursorPos.row);
      if(CursorPos.col > len)
        CursorPos.col = len;
    }
    SetCursorPos(CursorPos);
  }
}
//
LPCTSTR TCoolTextWnd::GetBreakChars() const
{
  static const _TCHAR* breakChars = _T("\"\\\t +-=*/~!@#$%^&(){}[]<>'?.,;:");
  return breakChars;
}
//
static bool isWordChar(_TCHAR c)
{
  return _istalnum(c) || c==_T('_');
}
//
TEditPos TCoolTextWnd::StartWord(const TEditPos& pos)
{
  //First skip over any non aski or digit char
  LPCTSTR pszChars  = GetLineText(pos.row);

  int nPos = pos.col;
  if(nPos == GetLineLength(pos.row))
    nPos--;

  // if on whitespace -> find start of word
  while(nPos > 0 && isWordChar(pszChars[nPos]))
    nPos--;

  if(nPos) // if not start position -> nPos points on first space
    nPos++;

  return TEditPos(nPos, pos.row);
}
//
TEditPos
TCoolTextWnd::EndWord(const TEditPos& pos)
{
  //First skip over any non aski or digit char
  LPCTSTR pszChars  = GetLineText(pos.row);
  int len           = GetLineLength(pos.row);
  int nPos          = pos.col;

  // if on whitespace -> find start of word
  while(nPos < len && isWordChar(pszChars[nPos]))
    nPos++;
  return TEditPos(nPos, pos.row);
}
//
TEditPos
TCoolTextWnd::NextBreak(const TEditPos& pos)
{
  //Find the first break char AFTER Col
  //First skip over any break chars we are sitting on
  LPCTSTR breakChars  = GetBreakChars();
  LPCTSTR pszChars    = GetLineText(pos.row);
  int len             = GetLineLength(pos.row);
  int nPos            = pos.col;
  bool isBreakChar    = nPos < len && _tcschr(breakChars, pszChars[nPos]);

  // if BreakChar -> skip until non break
  if(isBreakChar){
    while(nPos < len && _tcschr(breakChars, pszChars[nPos]))
      nPos++;
  }
  else{ // else find next break char
    int npos = len>0 ? static_cast<int>(_tcscspn(&pszChars[nPos], breakChars)) : 0;
    if(npos)
      nPos += npos;
    else
      nPos = GetLineLength(pos.row);
  }
  return TEditPos(nPos, pos.row);
}
//
TEditPos
TCoolTextWnd::PrevBreak(const TEditPos& pos)
{
  //Find the first break char BEFORE Col
  //First skip over any break chars we are sitting on
  LPCTSTR breakChars  = GetBreakChars();
  LPCTSTR pszChars    = GetLineText(pos.row);
  int len             = GetLineLength(pos.row);
  int nPos            = max(pos.col - 1, 0);
  bool isBreakChar    = nPos < len && _tcschr(breakChars, pszChars[nPos]);

  if(isBreakChar){
    // skip any break characters
    while(nPos > 0 && _tcschr(breakChars, pszChars[nPos]))
      nPos--;
  }
  else{
    //Then Skip over any non-break chars to the next break
    while(nPos > 0 && _tcsrchr(breakChars, pszChars[nPos])==0)
      nPos--;
  }
  if(nPos)
    nPos++;
  return TEditPos(nPos,pos.row);
}
//
void
TCoolTextWnd::AdjustXCurPos()
{
  if(IsFlagSet(cfSmartCursor)){
    int len = GetLineLength(CursorPos.row);
    if(CursorPos.col > len)
      CursorPos.col = len;
  }
}
//
void
TCoolTextWnd::SetSelMode(TSelType newType)
{
  if(newType != SelType){
    SelType = newType;
    if(Selection){
      TEditRange range(Selection->GetStart(),Selection->GetEnd());
      SetSelection(range);
    }
  }
}
//
bool
TCoolTextWnd::GetSelection(TEditRange& range)
{
  range = TEditRange();
  if(Selection){
    range = TEditRange(Selection->GetStart(), Selection->GetEnd());
    range.Normalize();
  }
  return true;
}
//
#if !defined(COOL_NODRAGDROP)
DROPEFFECT
TCoolTextWnd::GetDropEffect()
{
  //_TRACE("TCoolTextWnd::GetDropEffect\n");
  return DROPEFFECT_COPY;
}
//
void
TCoolTextWnd::DropSource(DROPEFFECT de)
{
  CHECK(de == DROPEFFECT_COPY); InUse(de);
  //_TRACE("TCoolTextWnd::DropSource\n");
}
//
void
TCoolTextWnd::DropText(IDataObject*, const TPoint&, DROPEFFECT)
{
  //_TRACE("TCoolTextWnd::DropText\n");
  // nothing
}
//
bool
TCoolTextWnd::SetDragData()
{
  if(Selection){
    tostringstream ostrm;
    Selection->Copy(ostrm);
    tstring s = ostrm.str();
    LPCTSTR text = s.c_str();
    uint len = static_cast<int>(_tcslen(text));

    // Allocate a global memory object for the text.
    HGLOBAL hHandle = ::GlobalAlloc(GMEM_DDESHARE, len);
    if (hHandle){
      // Lock the handle and copy the text to the buffer.
      void* pBlock = ::GlobalLock(hHandle);
      memcpy(pBlock, text, len);
      ::GlobalUnlock(hHandle);
      // Place the handle on the clipboard, as internal format.
      DragDropProxy->SetDragData(SelClipFormat, hHandle);
    }
    else
    {
      return false;
    }

    // Allocate a global memory object for the text.
    HGLOBAL hHandle1 = ::GlobalAlloc(GMEM_DDESHARE, len-7);
    if (hHandle1){
      // Lock the handle and copy the text to the buffer.
      void* pBlock = ::GlobalLock(hHandle1);
      memcpy(pBlock, text+7, len-7);
      ::GlobalUnlock(hHandle1);
      // Place the handle on the clipboard, as visible format.
#ifdef _UNICODE
      DragDropProxy->SetDragData(CF_UNICODETEXT, hHandle1);
#else
      DragDropProxy->SetDragData(CF_TEXT, hHandle1);
#endif
    }
    // Allocate a global memory object for the text.
    HGLOBAL hHandle2 = ::GlobalAlloc(GMEM_DDESHARE, len-7);
    if (hHandle1){
      // Lock the handle and copy the text to the buffer.
      void* pBlock = ::GlobalLock(hHandle2);
      USES_CONVERSION;
#ifdef _UNICODE
      memcpy(pBlock, W2A(text+7), len-7);
#else
      memcpy(pBlock, A2W(text+7), len-7);
#endif
      ::GlobalUnlock(hHandle2);
      // Place the handle on the clipboard, as visible format.
#ifdef _UNICODE
      DragDropProxy->SetDragData(CF_TEXT, hHandle2);
#else
      DragDropProxy->SetDragData(CF_UNICODETEXT, hHandle2);
#endif
    }
    return true;
  }
  return false;
}
#endif
//
void
TCoolTextWnd::WordUnderCursor(LPTSTR text, uint size)
{
  *text = 0;
  TEditRange range;
  if(IsSelection()){
    GetSelection(range);
  }
  else{
    range = TEditRange(StartWord(GetCursorPos()), EndWord(GetCursorPos()));
    range.Normalize();
  }
  if(range.Valid() && !range.Empty()){
    GetBuffer()->GetText(range.Start(), range.End(), text, size);
  }
}
//
bool
TCoolTextWnd::Search(const TEditPos& _startPos, LPCTSTR text,
           TFindFlags flags /*= ffNone*/)
{
  if (!text || !text[0])
    return false;

  TCoolTextBuffer* buffer = GetBuffer();
  TEditPos startPos(_startPos);
  if(!startPos.Valid())
    startPos = CursorPos;
  TEditPos endPos;

  if(flags&ffWholeDoc){
    startPos = TEditPos(-1,-1);
    endPos   = TEditPos(-1,-1);
  }
  else{
    if(flags&ffDirectionUp)
      endPos = TEditPos(0,0);
    else{
      int lastLine = buffer->GetLineCount()-1;
      TEditPos(buffer->GetLineLength(lastLine),lastLine);
    }
  }

  TEditRange searchRange(startPos,endPos);
  searchRange.Normalize();

  TEditPos resPos = buffer->Search(searchRange, text,
    ToBool(flags&ffMatchCase),
    ToBool(flags&ffWholeWord),
    ToBool(flags&ffDirectionUp));

  // If we've got a match, select that text, cleanup & return.
  //
  if(resPos.Valid()){
    TEditRange resRange(resPos.col, resPos.row, resPos.col + static_cast<int>(_tcslen(text)), resPos.row);
    SetSelection(resRange);
    CursorPos = resRange.End();
    ScrollToCaret(CursorPos);
    return true;
  }
  return false;
}
//-----------------------------------------------------------------------------
// class TBruteForceEngine
// ~~~~~ ~~~~~~~~~~~~~~~~~~
//
//
TEditRange
TBruteForceEngine::Search(TCoolTextBuffer& buffer,const TEditRange& searchRange)
{
  TEditPos resPos = buffer.Search(searchRange,Pattern.c_str(),CaseSens,
    WholeWord, DirectUp);
  return
    resPos.Valid()
    ?
    TEditRange(resPos.col, resPos.row, resPos.col + static_cast<int>(Pattern.length()), resPos.row)
    :
  TEditRange();
}
//-----------------------------------------------------------------------------
// class TBoyerMooreEngine
// ~~~~~ ~~~~~~~~~~~~~~~~~
//
//
const size_t TBoyerMooreEngine::DeltaSize = 256;
//
TBoyerMooreEngine::TBoyerMooreEngine(const tstring& pattern, bool caseSens,
                   bool wholeWord, bool up)
                   :
Pattern(pattern.c_str()),
CaseSens(caseSens),
WholeWord(wholeWord),
DirectUp(up)
{
  if(!CaseSens)
    _tcslwr((LPTSTR)Pattern.c_str());

  // allocate delta table
  DeltaTable = new uint[DeltaSize];

  // clear table
  int i;
  // get length of pattern
  int patlen = static_cast<int>(Pattern.length());

  for(i = 0; i < DeltaSize; i++)
    DeltaTable[i] = patlen;

  if(DirectUp){
    // set table values
    for (i = patlen-1; i >= 0; i--)
      DeltaTable[(size_t)Pattern[i]] = i;
    // set value for first pattern character
    //DeltaTable[(size_t)Pattern[patlen-1]] = patlen-1;
  }
  else{
    // set table values
    for (i = 1; i < patlen; i++)
      DeltaTable[(size_t)Pattern[i-1]] = patlen - i;
    // set value for last pattern character
    DeltaTable[(size_t)Pattern[patlen - 1]] = 1;
  }
}
//
TBoyerMooreEngine::~TBoyerMooreEngine()
{
  delete DeltaTable;
}
//
static bool
charCompare(_TCHAR ch1, _TCHAR ch2, bool caseSens)
{
  if(!caseSens)
    return ch1 == _totlower(ch2);
  return ch1 == ch2;
}
//
static LPCTSTR
BoyerMooreSearchFrw(int patlen, LPCTSTR pattern, LPCTSTR text, int strLength,
          uint* table, bool caseSens)
{
  // i is the index into the target
  int j, lastIndex, i = patlen;
  lastIndex = i;
  while(i <= strLength){
    // j is an index into pattern
    j = patlen;
    // while corresponding characters match
    while(charCompare(pattern[j - 1], text[i - 1], caseSens)){
      if(j > 1){// move left one character for next comparison
        j--,i--;
      }
      else// we've reached the beginning of the pattern pattern found!
        return &text[i - 1];
    }
    // move target index by delta value of mismatched character
    while(true){
      if(caseSens)
        i += table[text[i - 1]];
      else
        i += table[_totlower(text[i - 1])];
      if(lastIndex < i)
        break;
    }
    lastIndex = i;
  }
  return 0;
}
//
static LPCTSTR
BoyerMooreSearchBk(int patlen, LPCTSTR pattern, LPCTSTR text, int posIndex,
           uint* table, bool caseSens)
{
  // i is the index into the target
  int j, i = posIndex-patlen;
  while(i >= 0){
    // j is an index into pattern
    j = 0;
    // while corresponding characters match
    while(charCompare(pattern[j], text[i], caseSens)){
      if(j < patlen-1){
        // move right one character for next comparison
        j++;i++;
      }
      else{
        // we've reached the end of the pattern found!
        return &text[i - patlen+1];
      }
    }
    // move target index by delta value of mismatched character
    if(caseSens)
      i -= table[text[i]];
    else
      i -= table[_totlower(text[i])];
  }
  return 0;
}
//
TEditRange
TBoyerMooreEngine::Search(TCoolTextBuffer& buffer, const TEditRange& _range)
{
  // store pattern length locally (it gets used a lot)
  int patlen = static_cast<int>(Pattern.length());
  TEditRange searchRange(_range);
  if(!searchRange.Valid()){
    // if not valid -> all file
    searchRange.scol = 0;
    searchRange.srow = 0;
    searchRange.erow = buffer.GetLineCount()-1;
    searchRange.ecol = buffer.GetLineLength(searchRange.srow);
  }
  searchRange.Normalize();

  LPCTSTR pos = 0;
  int lineIndex = DirectUp ? searchRange.erow : searchRange.srow;
  int posIndex  = searchRange.scol;
  LPTSTR text   = buffer.GetLineText(lineIndex);

  while(true){
    if(DirectUp){
      if(posIndex < searchRange.srow){
        lineIndex--;
        if(lineIndex < searchRange.srow)
          break;
        text      = buffer.GetLineText(lineIndex);
        posIndex  = buffer.GetLineLength(lineIndex);
      }
      pos = BoyerMooreSearchBk(patlen,
        Pattern.c_str(),
        text,
        posIndex,
        DeltaTable,
        CaseSens);
    }
    else {
      if(posIndex > buffer.GetLineLength(lineIndex)){
        lineIndex++;
        if(lineIndex>=buffer.GetLineCount() || lineIndex>=searchRange.erow)
          break;
        text    = buffer.GetLineText(lineIndex);
        posIndex  = 0;
      }
      pos = BoyerMooreSearchFrw(patlen,
        Pattern.c_str(),
        text+posIndex,
        buffer.GetLineLength(lineIndex)-posIndex,
        DeltaTable,
        CaseSens);
    }

    // If whole-word matching is enabled and we have a match so far, then make
    // sure the match is on word boundaries.
    //
    if (WholeWord && pos){
#if defined(BI_DBCS_SUPPORT)
      LPTSTR prevPos;
      if (pos > text)
        prevPos = ::AnsiPrev(text, pos);

      if ((pos > text && _istalnum((_TCHAR)*prevPos)) || // Match is in preceding word
        (Pattern.length() < static_cast<int>(_tcslen(pos)) && _istalnum(pos[Pattern.length()])))
      {
          if(DirectUp)
            posIndex = (uint)(prevPos - text) + Pattern.length();
          else
            posIndex = (uint)(::AnsiNext(pos) - text);
          continue;  // Skip this match and keep searching
      }
#else
      if ((pos > text && _istalnum(pos[-1])) || // Match is in preceding word
        (Pattern.length() < static_cast<int>(_tcslen(pos)) && _istalnum(pos[Pattern.length()])))
      {
          //Jogy:  up -> DirectUp
          posIndex = (uint)(pos-text) + !DirectUp;
          continue;  // Skip this match and keep searching
      }
#endif
    }
    if(pos)
      break;
    else
      posIndex = DirectUp ? -1 : buffer.GetLineLength(lineIndex)+1;
  }

  // If we've got a match, select that text, cleanup & return.
  //
  if(pos)
    return TEditRange(static_cast<int>(pos - text), lineIndex, static_cast<int>(pos - text + Pattern.length()), lineIndex);
  return TEditRange();
}
//-----------------------------------------------------------------------------
// class TRegularExEngine
// ~~~~~ ~~~~~~~~~~~~~~~~
// regular expressions
typedef _TUCHAR PatternType;
//
static int makepat(const _TCHAR* exp, PatternType* pat, size_t maxpattern);
//
const size_t TRegularExEngine::MaxPat=128;
//
TRegularExEngine::TRegularExEngine(const tstring& pattern, bool caseSens,
                   bool wholeWord, bool up)
                   :
CaseSens(caseSens),
WholeWord(wholeWord),
DirectUp(up)
{
  Pattern = new PatternType[MaxPat];
  Status = makepat(pattern.c_str(), Pattern, MaxPat);
}
//
typedef _TUCHAR pattern;  /* pattern strings are unsigned char / t_wchar */
static const _TCHAR* matchs( const _TCHAR*, const _TCHAR*, const pattern*, _TCHAR**);
//
TEditRange
TRegularExEngine::Search(TCoolTextBuffer& buffer, const TEditRange& _sRange)
{
  TEditRange searchRange(_sRange);
  if(!searchRange.Valid()){
    // if not valid -> all file
    searchRange.scol = 0;
    searchRange.srow = 0;
    searchRange.erow = buffer.GetLineCount()-1;
    searchRange.ecol = buffer.GetLineLength(searchRange.srow);
  }
  searchRange.Normalize();

  PRECONDITION(Status==0);

  int lineIndex = DirectUp ? searchRange.erow : searchRange.srow;
  //int posIndex  = DirectUp ? searchRange.ecol : searchRange.scol;
  int posIndex  = searchRange.scol;
  uint numLines = buffer.GetLineCount();

  _TCHAR* startp = 0;
  const _TCHAR* endp = 0; InUse(endp);
  LPCTSTR text = 0; InUse(text);
  while(true){

    text  = buffer.GetLineText(lineIndex);
    int lineLen = buffer.GetLineLength(lineIndex);
    LPCTSTR endpos = text + lineLen;

    if(searchRange.erow == searchRange.srow && searchRange.ecol < lineLen)
      endpos = text + searchRange.ecol;

    endp = matchs(text+posIndex, endpos, Pattern, &startp);

    if(endp)
      break;
    if(DirectUp){
      lineIndex--;
      if(lineIndex < 0)
        break;
    }
    else{
      lineIndex++;
      if(lineIndex >= (int)numLines)
        break;
    }
  }

  if(endp)
    return TEditRange(static_cast<int>(startp - text), lineIndex, static_cast<int>(endp - text + 1), lineIndex);
  return TEditRange();
}


/*
*
* Author:    Allen I. Holub
*
* (c) C Gazette. May be used freely as long as author and publication are
* acknowledged
*
*/

/*
*
* Modified by Borland International to compile without warnings as C++.
*
* Modified by Yura Bidus to support UNICODE.
*/

/* Metacharacters in the input:         */
#define BOL     _T('^')     /* start-of-line anchor                 */
#define EOL     _T('$')     /* end-of-line anchor                   */
#define ANY     _T('.')     /* matches any character                */
#define CCL     _T('[')     /* start a character class              */
#define CCLEND  _T(']')     /* end a character class                */
#define NCCL    _T('^')     /* negates character class if 1st char. */
#define CLOSURE _T('*')     /* Kleene closure (matches 0 or more)   */
#define PCLOSE  _T('+')     /* Positive closure (1 or more)         */
#define OPT     _T('?')     /* Optional closure (0 or 1)            */

//
typedef enum action {    /* These are put in the pattern string  */
  /* to represent metacharacters.         */
  M_BOL =    (0x80 | _T('^')),
  M_EOL =    (0x80 | _T('$')),
  M_ANY =    (0x80 | _T('.')),
  M_CCL =    (0x80 | _T('[')),
  M_OPT =    (0x80 | _T('?')),
  M_CLOSE =  (0x80 | _T('*')),
  M_PCLOSE = (0x80 | _T('+'))
} action;

#define IS_ACTION(x) ((x)&0x80) /* true => element of pat. string is an     */
/* action that represents a metacharacter   */

/*--------------------------------------------------------------------------*/
#define MAPSIZE 16      /* need this many bytes for character class bit map */

/* Advance a pointer into the pattern template    */
/* to the next pattern element, this is a +1 for  */
/* all pattern elements but M_CCL, where you      */
/* to skip past both the M_CCL character and the  */
/* bitmap that follows that character             */

#define ADVANCE(pat) (pat += (*pat == (pattern)M_CCL) ? (MAPSIZE+1) : 1)

/* Bitmap functions. Set bit b in the map and  */
/* test bit b to see if it was set previously. */

#define SETBIT(b,map) ((map)[((b) & 0x7f) >>3] |= (1<< ((b) & 0x07)) )
#define TSTBIT(b,map) ((map)[((b) & 0x7f) >>3] &  (1<< ((b) & 0x07)) )
/*----------------------------------------------------------------------*/
#define E_NONE       0      /* Possible return values from pat_err.     */
#define E_ILLEGAL    1      /* Set in makepat() to indicate prob-       */
#define E_NOMEM      2      /* lems that came up while making the       */
#define E_PAT        3      /* pattern template.                        */
/*----------------------------------------------------------------------*/

static const _TCHAR* doccl(pattern*, const _TCHAR*);
static int           hex2bin(int);
static int           makepat( const _TCHAR*, pattern*, size_t);
static int           oct2bin(int);
static int           omatch(const _TCHAR**, const pattern*, const _TCHAR*);
static const _TCHAR* patcmp(const _TCHAR*, const pattern*, const _TCHAR*);
static int           esc( const _TCHAR**);

/*----------------------------------------------------------------------*/

static int
makepat(const _TCHAR* exp, pattern* pat, size_t maxpat)
//  char      *exp;      /* Regular expression         */
//  pattern   *pat;      /* Assembled compiled pattern */
//  int             maxpat;   /* Length of pat              */
{
  /* Make a pattern template from the string pointed to by exp. Stop when  */
  /* '\0' or '\n' is found in exp.  The pattern template is assembled      */
  /* in pat whose length is given by maxpat.                               */
  /*                                                                       */
  /* Return:                                                               */
  /* E_ILLEGAL       Illegal input pattern.                                */
  /* E_NOMEM         out of memory.                                        */
  /* E_PAT           pattern too long.                                     */

  pattern*  cur;       /* pointer to current pattern element  */
  pattern*  prev;      /* pointer to previous pattern element */
  int       Error = E_ILLEGAL;

  if(!*exp || *exp == _T('\n'))
    goto exit;

  if (*exp == CLOSURE || *exp == PCLOSE || *exp == OPT)
    goto exit;

  Error = E_NOMEM;
  if (!pat)
    goto exit;      /* Check for bad pat */

  prev = cur = pat;
  Error = E_PAT;

  while(*exp && *exp != _T('\n')) {

    if(cur >= &pat[maxpat - 1])
      goto exit;

    switch (*exp) {
    case ANY:
      *cur = (pattern)M_ANY;
      prev = cur++;
      ++exp;
      break;

    case BOL:
      *cur = (cur == pat) ? (pattern)M_BOL : *exp;
      prev = cur++;
      ++exp;
      break;

    case EOL:
      *cur = (!exp[1] || exp[1] == _T('\n')) ? (pattern)M_EOL : *exp;
      prev = cur++;
      ++exp;
      break;

    case CCL:
      if (uint((cur - pat) + MAPSIZE) >= maxpat)
        goto exit;      /* not enough room for bit map */
      prev = cur;
      *cur++ = (pattern)M_CCL;
      exp = doccl(cur, exp);
      cur += MAPSIZE;
      break;

    case OPT:
    case CLOSURE:
    case PCLOSE:
      switch (*prev){
    case M_BOL:
    case M_EOL:
    case M_OPT:
    case M_PCLOSE:
    case M_CLOSE:
      goto exit;
      }

      /* memmove( prev+1, prev, cur-prev ); */
      {
        pattern *p = cur;
        while (p > prev) {
          *p = *(p - 1);
          p--;
        }
      }
      *prev = (*exp == OPT) ? (pattern)M_OPT :
        ((*exp == PCLOSE) ? (pattern)M_PCLOSE : (pattern)M_CLOSE);
      ++cur;
      ++exp;
      break;

      default:
        prev = cur;
        *cur++ = (pattern)esc(&exp);
        break;
    }
  }

  *cur = _T('\0');
  Error = E_NONE;

exit:
  return Error;
}
/*----------------------------------------------------------------------*/
static const _TCHAR*
doccl( pattern* map, const _TCHAR* src)
{
  /* Set bits in the map corresponding to characters specified in the src */
  /* character class.                                                     */

  int first, last, negative;
  const _TCHAR* start;

  ++src;                    /* skip past the [              */
  negative = (*src == NCCL);
  if (negative)             /* check for negative ccl       */
    ++src;
  start = src;              /* start of characters in class */
  memset(map, 0, MAPSIZE);  /* bitmap initially empty       */

  while (*src && *src != CCLEND) {
    if (*src != _T('-')) {
      first = esc(&src);    /* Use temp. to avoid macro  */
      SETBIT(first, map);   /* side effects.             */
    }
    else if (src == start) {
      SETBIT(_T('-'), map);     /* literal dash at start or end */
      ++src;
    }
    else{
      ++src;            /* skip to end-of-sequence char */
      if (*src < src[-2]) {
        first = *src;
        last = src[-2];
      }
      else {
        first = src[-2];
        last = *src;
      }
      while (++first <= last)
        SETBIT(first, map);
      src++;
    }
  }

  if (*src == CCLEND)
    ++src;          /* Skip CCLEND */

  if (negative)
    for (first = MAPSIZE; --first >= 0;)
      *map++ ^= ~0;     /* invert all bits */

  return src;
}

/*----------------------------------------------------------------------*/
static const _TCHAR*
matchs( const _TCHAR* str, const _TCHAR* ends, const pattern* pat, _TCHAR** startpat)
{
  const _TCHAR * endp = 0;
  const _TCHAR * start;

  if (!pat)
    return NULL;

  if(str == ends){
    if ((*pat == (pattern)M_EOL) || (*pat == (pattern)M_BOL && (!pat[1] || pat[1] == (pattern)M_EOL)))
      endp = str;
  }
  else{
    start = str; /* Do a brute-force substring search,           */
    /* comparing a pattern against the input string */
    while( str != ends) {
      endp = patcmp(str, pat, start);
      if(endp)
        break;
      str++;
    }
  }
  *startpat = (_TCHAR*)str;
  return endp;
}
/*----------------------------------------------------------------------*/
static const _TCHAR*
patcmp( const _TCHAR* str, const pattern* pat, const _TCHAR* start)
{
  /* Like strcmp, but compares str against pat. Each element of str is        */
  /* compared with the template until either a mis-match is found or the end  */
  /* of the template is reached. In the former case a 0 is returned; in the   */
  /* latter, a pointer into str (pointing to the last character in the        */
  /* matched pattern) is returned. Strstart points at the first character in  */
  /* the string, which might not be the same thing as line if the search      */
  /* started in the middle of the string.                                     */

  const _TCHAR* bocl;     /* beginning of closure string.         */
  const _TCHAR* end=0;    /* return value: end-of-string pointer. */

  if (!pat)         /* make sure pattern is valid   */
    return (NULL);

  while (*pat) {
    if (*pat == (pattern)M_OPT) {
      /* Zero or one matches. It doesn't matter if omatch fails---it will  */
      /* advance str past the character on success, though. Always advance */
      /* the pattern past both the M_OPT and the operand.                  */

      omatch(&str, ++pat, start);
      ADVANCE(pat);
    }
    else if (!(*pat == (pattern)M_CLOSE || *pat == (pattern)M_PCLOSE)) {
      /* Do a simple match. Note that omatch() fails if there's still */
      /* something in pat but we're at end of string.                 */
      if (!omatch(&str, pat, start))
        return NULL;

      ADVANCE(pat);

    }
    else {            /* Process a Kleene or positive closure */
      if (*pat++ == (pattern)M_PCLOSE)  /* one match required */
        if (!omatch(&str, pat, start))
          return NULL;

      /* Match as many as possible, zero is okay */
      bocl = str;
      while (*str && omatch(&str, pat, start)) { /* do nothing */ }

      /* 'str' now points to the character that made made us fail. Try to     */
      /* process the rest of the string. If the character following the       */
      /* closure could have been in the closure (as in the pattern "[a-z]*t") */
      /* the final 't' will be sucked up in the while loop. So, if the match  */
      /* fails, back up a notch and try to match the rest of the string       */
      /* again, repeating this process recursively until we get back to the   */
      /* beginning of the closure. The recursion goes, at most, one levels    */
      /* deep.                                                                */
      if (*ADVANCE(pat)){
        for (; bocl <= str; --str){
          end = patcmp(str, pat, start);
          if (end)
            break;
        }
        return end;
      }
      break;
    }
  }

  /* omatch() advances str to point at the next character to be matched. So    */
  /* str points at the character following the last character matched when     */
  /* you reach the end of the template. The exceptions are templates           */
  /* containing only a BOLN or EOLN token. In these cases omatch doesn't       */
  /* advance. Since we must return a pointer to the last matched character,    */
  /* decrement str to make it point at the end of the matched string, making   */
  /* sure that the decrement hasn't gone past the beginning of the string.     */
  /*                                                                           */
  /* Note that $ is a position, not a character, but in the case of a pattern  */
  /* ^$, a pointer to the end of line character is returned. In ^xyz$, a       */
  /* pointer to the z is returned.                                             */
  /*                                                                           */
  /* The --str is done outside the return statement because __max() was a macro */
  /* with side-effects.                                                        */

  --str;
  return std::max(start, str);
}

/*----------------------------------------------------------------------*/
static int
omatch( const _TCHAR * * strp,
     const pattern * pat,
     const _TCHAR * start )
{
  /* Match one pattern element, pointed at by pat, against the character at   */
  /* **strp. Return 0 on a failure, 1 on success. *strp is advanced to skip   */
  /* over the matched character on a successful match. Closure is handled one */
  /* level up by patcmp().                                                    */
  /*                                                                          */
  /* "start" points at the character at the left edge of the line. This might */
  /* not be the same thing as *strp if the search is starting in the middle   */
  /* of the string. An end-of- line anchor matches '\n' or '\0'.              */

  int advance = -1; /* amount to advance *strp, -1 == error  */

  switch (*pat) {
  case M_BOL:           /* First char in string? */
    if (*strp == start) /* Only one star here.   */
      advance = 0;
    break;

  case M_ANY:           /* . = anything but newline */
    if (**strp != _T('\n'))
      advance = 1;
    break;

  case M_EOL:
    if (**strp == _T('\n') || **strp == _T('\0'))
      advance = 0;
    break;

  case M_CCL:
    if (TSTBIT(**strp, pat + 1))
      advance = 1;
    break;

  default:             /* literal match */
    if (**strp == *pat)
      advance = 1;
    break;
  }

  if (advance > 0)
    *strp += advance;

  return (advance + 1);
}

#define ISHEXDIGIT(x) (_istdigit(x)                       \
  || (_T('a')<=(x) && (x)<=_T('f'))   \
  || (_T('A')<=(x) && (x)<=_T('F'))   )

#define ISOCTDIGIT(x) (_T('0')<=(x) && (x)<=_T('7'))

static int hex2bin( int c )
{
  /* Convert the hex digit represented by 'c' to an int. 'c'  */
  /* must be one of: 0123456789abcdefABCDEF                   */
  return (_istdigit(c) ? (c)-_T('0') : ((_totupper(c))-_T('A'))+10) & 0xf;
}

static int oct2bin( int c )
{
  /* Convert the hex digit represented by 'c' to an int. 'c' */
  /* must be a digit in the range '0'-'7'.                   */
  return ( ((c)-_T('0'))  &  0x7 );
}

/*------------------------------------------------------------*/
static int
esc( const _TCHAR** s)
{
  /* Map escape sequences into their equivalent symbols. Return   */
  /* the equivalent ASCII character. *s is advanced past the      */
  /* escape sequence. If no escape sequence is present, the       */
  /* current character is returned and the string is advanced by  */
  /* one. The following are recognized:                           */
  /*                                                              */
  /*  \b     backspace                                            */
  /*  \f     formfeed                                             */
  /*  \n     newline                                              */
  /*  \r     carriage return                                      */
  /*  \s     space                                                */
  /*  \t     tab                                                  */
  /*  \e     ASCII ESC character ('\033')                         */
  /*  \DDD   number formed of 1-3 octal digits                    */
  /*  \xDDD  number formed of 1-3 hex digits                      */
  /*  \^C    C = any letter. Control code                         */

  int rval;

  if( **s != _T('\\') )
    rval = *( (*s)++ );
  else {
    ++(*s);                                 /* Skip the \ */
    switch( _totupper(**s) ) {
    case _T('\0'):  rval = _T('\\');            break;
    case _T('B'):   rval = _T('\b');            break;
    case _T('F'):   rval = _T('\f');            break;
    case _T('N'):   rval = _T('\n');            break;
    case _T('R'):   rval = _T('\r');            break;
    case _T('S'):   rval = _T(' ');             break;
    case _T('T'):   rval = _T('\t');            break;
    case _T('E'):   rval = _T('\033');          break;

    case _T('^'):
      rval = *++(*s) ;
      rval = _totupper(rval) - _T('@') ;
      break;

    case _T('X'):
      rval = 0;
      ++(*s);
      if( ISHEXDIGIT(**s) ) {
        rval  = hex2bin( *(*s)++ );
      }
      if( ISHEXDIGIT(**s) ) {
        rval <<= 4;
        rval  |= hex2bin( *(*s)++ );
      }
      if( ISHEXDIGIT(**s) ) {
        rval <<= 4;
        rval  |= hex2bin( *(*s)++ );
      }
      --(*s);
      break;

    default:
      if( !ISOCTDIGIT(**s) )
        rval = **s;
      else {
        ++(*s);
        rval = oct2bin( *(*s)++ );
        if( ISOCTDIGIT(**s) ) {
          rval <<= 3;
          rval  |= oct2bin( *(*s)++ );
        }
        if( ISOCTDIGIT(**s) ) {
          rval <<= 3;
          rval  |= oct2bin( *(*s)++ );
        }
        --(*s);
      }
      break;
    }
    ++(*s);
  }
  return rval;
}
//
//-----------------------------------------------------------------------------
// class TPhoneticEngine
// ~~~~~ ~~~~~~~~~~~~~~~~~~
//
/*
**  The Metaphone algorithm was developed by Lawrence Phillips. Like the
**  Soundex algorithm, it compares words that sound alike but are spelled
**  differently. Metaphone was designed to overcome difficulties encountered
**  with Soundex.
**
**  This implementation was written by Gary A. Parker and originally published
**  in the June/July, 1991 (vol. 5 nr. 4) issue of C Gazette. As published,
**  this code was explicitly placed in the public domain by the author.
*/
//
//const uint TPhoneticEngine::MaxMetaph = 5;
#define MAXMETAPH 5
typedef enum {COMPARE, GENERATE} metaphlag;

/*
**  Character coding array
*/

static _TCHAR vsvfn[26] = {
  1,16,4,16,9,2,4,16,9,2,0,2,2,2,1,4,0,2,4,4,1,0,0,0,8,0};
  /*    A  B C  D E F G  H I J K L M N O P Q R S T U V W X Y Z      */

  /*
  **  Macros to access the character coding array
  */

#define MT_vowel(x)  (vsvfn[(x) - _T('A')] & 1)  /* AEIOU    */
#define MT_same(x)   (vsvfn[(x) - _T('A')] & 2)  /* FJLMNR   */
#define MT_varson(x) (vsvfn[(x) - _T('A')] & 4)  /* CGPST    */
#define MT_frontv(x) (vsvfn[(x) - _T('A')] & 8)  /* EIY      */
#define MT_noghf(x)  (vsvfn[(x) - _T('A')] & 16) /* BDH      */

  /*
  **  metaphone()
  **
  **  Arguments: 1 - The word to be converted to a metaphone code.
  **             2 - A MAXMETAPH+1 char field for the result.
  **             3 - Function flag:
  **                 If 0: Compute the Metaphone code for the first argument,
  **                       then compare it to the Metaphone code passed in
  **                       the second argument.
  **                 If 1: Compute the Metaphone code for the first argument,
  **                       then store the result in the area pointed to by the
  **                       second argument.
  **
  **  Returns: If function code is 0, returns Success_ for a match, else Error_.
  **           If function code is 1, returns Success_.
  */
  bool metaphone(const _TCHAR* Word, _TCHAR* Metaph, metaphlag flag)
  {
    _TCHAR *n, *n_start, *n_end;    /* Pointers to string               */
    _TCHAR *metaph = 0, *metaph_end = 0;    /* Pointers to metaph               */
    InUse(metaph); InUse(metaph_end); // Eliminate warnings.
    _TCHAR ntrans[512];             /* Word with uppercase letters      */
    _TCHAR newm[MAXMETAPH + 4];     /* New metaph for comparison        */
    bool KSflag;                  /* State flag for X translation     */

    /** Copy word to internal buffer, dropping non-alphabetic characters */
    /** and converting to upper case.                                    */
    for (n = ntrans + 1, n_end = ntrans + COUNTOF(ntrans) - 2;
      *Word && n < n_end; ++Word){
        if (isalpha(*Word))
          *n++ = toupper(*Word);
    }

    if (n == ntrans + 1)
      return false;           /* Return if zero characters        */
    else
      n_end = n;              /* Set end of string pointer        */

    /* Pad with NULs, front and rear  */

    *n++ = NULL;
    *n   = NULL;
    n    = ntrans;
    *n++ = NULL;

    /* If doing comparison, redirect pointers */
    if (COMPARE == flag){
      metaph = Metaph;
      Metaph = newm;
    }

    /* Check for PN, KN, GN, WR, WH, and X at start */
    switch (*n){
    case _T('P'):
    case _T('K'):
    case _T('G'):
      if (_T('N') == *(n + 1))
        *n++ = NULL;
      break;

    case _T('A'):
      if (_T('E') == *(n + 1))
        *n++ = NULL;
      break;

    case _T('W'):
      if (_T('R') == *(n + 1))
        *n++ = NULL;
      else if (_T('H') == *(n + 1)){
        *(n + 1) = *n;
        *n++ = NULL;
      }
      break;
    case _T('X'):
      *n = _T('S');
      break;
    }

    /* Now loop through the string, stopping at the end of the string   */
    /* or when the computed Metaphone code is MAXMETAPH characters long.*/
    KSflag = false;              /* State flag for KStranslation     */
    for (metaph_end = Metaph + MAXMETAPH, n_start = n;
      n <= n_end && Metaph < metaph_end; ++n){
        if (KSflag){
          KSflag = false;
          *Metaph++ = *n;
        }
        else{
          /* Drop duplicates except for CC    */
          if (*(n - 1) == *n && *n != _T('C'))
            continue;

          /* Check for F J L M N R  or first letter vowel */
          if (MT_same(*n) || (n == n_start && MT_vowel(*n)))
            *Metaph++ = *n;
          else
            switch (*n){
          case _T('B'):
            if (n < n_end || *(n - 1) != _T('M'))
              *Metaph++ = *n;
            break;
          case _T('C'):
            if (*(n - 1) != _T('S') || !MT_frontv(*(n + 1))){
              if (_T('I') == *(n + 1) && _T('A') == *(n + 2))
                *Metaph++ = _T('X');
              else if (MT_frontv(*(n + 1)))
                *Metaph++ = _T('S');
              else if (_T('H') == *(n + 1))
                *Metaph++ = ((n == n_start && !MT_vowel(*(n + 2))) || _T('S') == *(n - 1)) ? _T('K') : _T('X');
              else
                *Metaph++ = _T('K');
            }
            break;
          case _T('D'):
            *Metaph++ = (_T('G') == *(n + 1) && MT_frontv(*(n + 2))) ? _T('J') : _T('T');
            break;

          case _T('G'):
            if ((*(n + 1) != _T('H') || MT_vowel(*(n + 2))) &&
              (*(n + 1) != _T('N') || ((n + 1) < n_end &&
              (*(n + 2) != _T('E') || *(n + 3) != _T('D')))) &&
              (*(n - 1) != _T('D') || !MT_frontv(*(n + 1)))){
                *Metaph++ = (MT_frontv(*(n + 1)) && *(n + 2) != _T('G')) ? _T('J') : _T('K');
            }
            else if (_T('H') == *(n + 1) && !MT_noghf(*(n - 3)) && *(n - 4) != _T('H')){
              *Metaph++ = _T('F');
            }
            break;
          case _T('H'):
            if (!MT_varson(*(n - 1)) && (!MT_vowel(*(n - 1)) || MT_vowel(*(n + 1)))){
              *Metaph++ = _T('H');
            }
            break;
          case _T('K'):
            if (*(n - 1) != _T('C'))
              *Metaph++ = _T('K');
            break;
          case _T('P'):
            *Metaph++ = (_T('H') == *(n + 1)) ? _T('F') : _T('P');
            break;
          case _T('Q'):
            *Metaph++ = _T('K');
            break;
          case _T('S'):
            *Metaph++ = (_T('H') == *(n + 1) || (_T('I') == *(n + 1) &&
              (_T('O') == *(n + 2) || _T('A') == *(n + 2)))) ? _T('X') : _T('S');
            break;
          case _T('T'):
            if(_T('I') == *(n + 1) && (_T('O') == *(n + 2) || _T('A') == *(n + 2))){
              *Metaph++ = _T('X');
            }
            else if (_T('H') == *(n + 1))
              *Metaph++ = _T('O');
            else if (*(n + 1) != _T('C')|| *(n + 2) != _T('H'))
              *Metaph++ = _T('T');
            break;
          case _T('V'):
            *Metaph++ = _T('F');
            break;
          case _T('W'):
          case _T('Y'):
            if (MT_vowel(*(n + 1)))
              *Metaph++ = *n;
            break;
          case _T('X'):
            if (n == n_start)
              *Metaph++ = _T('S');
            else{
              *Metaph++ = _T('K');
              KSflag = true;
            }
            break;
          case _T('Z'):
            *Metaph++ = _T('S');
            break;
          }
        }
        /* Compare new Metaphone code with old */
        if (COMPARE == flag && *(Metaph - 1) != metaph[(Metaph - newm) - 1]){
          return false;
        }
    }

    /* If comparing, check if Metaphone codes were equal in length */
    if (COMPARE == flag && metaph[Metaph - newm])
      return false;

    *Metaph = NULL;
    return true;
  }
  //
  TPhoneticEngine::TPhoneticEngine(const tstring& pattern, bool up)
    : Pattern(pattern),
  DirectUp(up)
  {
    // only first word excluding whitespace ??
    auto pos = Pattern.find_first_not_of(_T(' '));
    if (pos != Pattern.npos)
      Pattern.erase(0, pos);
    pos = Pattern.find_first_of(_T(' '));
    if (pos != Pattern.npos)
      Pattern.erase(pos);

    _TCHAR meta[MAXMETAPH + 4];
    metaphone(Pattern.c_str(), meta, GENERATE);
    Pattern = meta;
  }
  //
  static LPCTSTR meta_match_word(LPCTSTR text, int lineLen, const tstring& pattern, int& len)
  {
    LPTSTR curptr = (LPTSTR)text;
    LPTSTR endword = curptr;
    while(lineLen > 0){
      // if on whitespace -> find start of word
      while(lineLen > 0 && !isWordChar(*curptr)){
        curptr++;  lineLen--;
      }
      endword = curptr;
      while(lineLen > 0 && isWordChar(*endword)){
        endword++;  lineLen--;
      }
      if(lineLen > 0){
        _TCHAR ch = *endword;
        *endword = 0;
        if(metaphone(curptr, (LPTSTR)pattern.c_str(), COMPARE)){
          *endword = ch;
          break;
        }
        *endword = ch;
      }
      curptr = endword;
    }
    if(lineLen > 0){
      len = static_cast<int>(endword - curptr);
      return curptr;
    }
    return 0;
  }
  //
  TEditRange
    TPhoneticEngine::Search(TCoolTextBuffer& buffer,const TEditRange& _sRange)
  {
    TEditRange searchRange(_sRange);
    if(!searchRange.Valid()){
      // if not valid -> all file
      searchRange.scol = 0;
      searchRange.srow = 0;
      searchRange.erow = buffer.GetLineCount()-1;
      searchRange.ecol = buffer.GetLineLength(searchRange.srow);
    }
    searchRange.Normalize();

    TEditRange range;

    int lineIndex = DirectUp ? searchRange.erow : searchRange.srow;
    uint numLines = buffer.GetLineCount();
    while(true){
      LPCTSTR text    = buffer.GetLineText(lineIndex);
      int     lineLen  = buffer.GetLineLength(lineIndex);

      if(searchRange.erow == lineIndex && searchRange.ecol < lineLen)
        lineLen -= lineLen-searchRange.ecol;

      if(searchRange.srow==lineIndex && searchRange.scol < lineLen){
        text += searchRange.srow;
        lineLen -= searchRange.srow;
      }

      int wordLen;
      LPCTSTR endp = meta_match_word(text, lineLen, Pattern, wordLen);

      if(endp){
        range.srow = range.erow = lineIndex;
        range.scol = static_cast<int>(endp - buffer.GetLineText(lineIndex));
        range.ecol = range.scol + wordLen;
        break;
      }
      if(DirectUp){
        lineIndex--;
        if(lineIndex < 0)
          break;
      }
      else{
        lineIndex++;
        if(lineIndex >= (int)numLines)
          break;
      }
    }
    return range;
  }
  //
  //
  //
  //-----------------------------------------------------------------------------
  //
  void TFuzzyEngine::Next(_TCHAR **start, _TCHAR **end, int *howclose,
    int textlen, LPCTSTR Text)
  {
    int *temp, a, b, c, i;
    *start = NULL;
    LPCTSTR pat = Pattern.c_str();
    int Textloc = -1;
    while(*start == NULL){  /* start computing columns */
      if (++Textloc >= textlen) /* out of text to search! */
        break;

      temp  = Rdiff;  /* move right-hand column to left ... */
      Rdiff = Ldiff;  /* ... so that we can compute new ... */
      Ldiff = temp;   /* ... right-hand column */
      Rdiff[0] = 0;   /* top (boundary) row */

      temp = Roff;    /* and swap offset arrays, too */
      Roff = Loff;
      Loff = temp;
      Roff[1] = 0;

      for (i = 0; i < Plen; i++){   /* run through pattern   */
        /* compute a, b, & c as the three adjacent cells ... */
        if (pat[i] == Text[Textloc])
          a = Ldiff[i];
        else
          a = Ldiff[i] + 1;

        b = Ldiff[i+1] + 1;
        c = Rdiff[i] + 1;

        /* ... now pick minimum ... */
        if (b < a)
          a = b;
        if (c < a)
          a = c;

        /* ... and store */
        Rdiff[i+1] = a;
      }

      /* now update offset array                              */
      /* the values in the offset arrays are added to the     */
      /*   current location to determine the beginning of the */
      /*   mismatched substring. (see text for details)       */
      if (Plen > 1){
        for (i=2; i<=Plen; i++){
          if (Ldiff[i-1] < Rdiff[i])
            Roff[i] = Loff[i-1] - 1;
          else if (Rdiff[i-1] < Rdiff[i])
            Roff[i] = Roff[i-1];
          else if (Ldiff[i] < Rdiff[i])
            Roff[i] = Loff[i] - 1;
          else /* Ldiff[i-1] == Rdiff[i] */
            Roff[i] = Loff[i-1] - 1;
        }
      }

      /* now, do we have an approximate match? */
      if (Rdiff[Plen] <= Degree){    /* indeed so! */
        *end = (LPTSTR)Text + Textloc;
        *start = *end + Roff[Plen];
        *howclose = Rdiff[Plen];
      }
    }
  }
  //
  //
  TFuzzyEngine::TFuzzyEngine(const tstring& pattern, bool up, uint level)
    :
  DirectUp(up),
    Degree(level)
  {
    // only first word excluding whitespace ??
    Pattern = pattern;
    /* initialize */
    Plen = static_cast<int>(pattern.length());
    Ldiff  = new int[(Plen+1)*4];
    Rdiff  = Ldiff + Plen + 1;
    Loff   = Rdiff + Plen + 1;
    Roff   = Loff +  Plen + 1;
    for(int i = 0; i <= Plen; i++){
      Rdiff[i] = i;   /* initial values for right-hand column */
      Roff[i]  = 1;
    }
  }
  //
  TFuzzyEngine::~TFuzzyEngine()
  {
    delete[] Ldiff;
  }
  //
  TEditRange
    TFuzzyEngine::Search(TCoolTextBuffer& buffer,const TEditRange& _sRange)
  {
    TEditRange searchRange(_sRange);
    if(!searchRange.Valid()){
      // if not valid -> all file
      searchRange.scol = 0;
      searchRange.srow = 0;
      searchRange.erow = buffer.GetLineCount()-1;
      searchRange.ecol = buffer.GetLineLength(searchRange.srow);
    }
    searchRange.Normalize();

    TEditRange range;

    int howclose;

    int lineIndex = DirectUp ? searchRange.erow : searchRange.srow;
    uint numLines = buffer.GetLineCount();
    while(true){
      LPCTSTR text    = buffer.GetLineText(lineIndex);
      int     lineLen  = buffer.GetLineLength(lineIndex);

      if(searchRange.erow == lineIndex && searchRange.ecol < lineLen)
        lineLen -= lineLen-searchRange.ecol;

      if(searchRange.srow==lineIndex && searchRange.scol < lineLen){
        text += searchRange.srow;
        lineLen -= searchRange.srow;
      }

      _TCHAR *textBegin, *textEnd;
      Next(&textBegin, &textEnd, &howclose, lineLen, text);
      if(textBegin){
        range.srow = range.erow = lineIndex;
        range.scol = static_cast<int>(textBegin - buffer.GetLineText(lineIndex));
        range.ecol = static_cast<int>(range.scol + textEnd - textBegin);
        break;
      }
      if(DirectUp){
        lineIndex--;
        if(lineIndex < 0)
          break;
      }
      else{
        lineIndex++;
        if(lineIndex >= (int)numLines)
          break;
      }
    }
    return range;
  }
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////
  // class TCoolFindDlg::TData
  // ~~~~~ ~~~~~~~~~~~~~~~~~~~
  //
  //
  TCoolFindDlg::TData::TData(TFindFlags flags, int buffSize)
    :
  Flags(flags), BuffSize(buffSize), Error(0), EngineIndex(0)
  {
    FindWhat    = new _TCHAR[BuffSize];
    ReplaceWith = new _TCHAR[BuffSize];
    *FindWhat    = *ReplaceWith = 0;
  }
  //
  TCoolFindDlg::TData::~TData()
  {
    delete[] FindWhat;
    delete[] ReplaceWith;
  }
  //
  void
    TCoolFindDlg::TData::AddDefaultEngines(TData& data)
  {
    data.EngineDescr.Add(new TBruteForceDescr);
    data.EngineDescr.Add(new TBoyerMooreDescr);
    data.EngineDescr.Add(new TRegularExDescr);
    data.EngineDescr.Add(new TPhoneticDescr);
    data.EngineDescr.Add(new TFuzzyDescrLevel1);
    data.EngineDescr.Add(new TFuzzyDescrLevel2);
    data.EngineDescr.Add(new TFuzzyDescrLevel3);
  }
  /////////////////////////////////////////////////////////////////////////////
  //
  class TCoolHistComboBox: public TMemComboBox{
  public:
    TCoolHistComboBox(TWindow* parent, int resId, const tstring& name, uint textLimit = 255, uint itemLimit = 25, TModule* module = 0)
      :TMemComboBox(parent,resId,name,textLimit,itemLimit,module){}
    TConfigFile* CreateConfigFile();
  };
  //
  TConfigFile*
    TCoolHistComboBox::CreateConfigFile()
  {
    TCoolFindDlg* dlg = TYPESAFE_DOWNCAST(GetParentO(), TCoolFindDlg);
    if(dlg)
      return dlg->CreateConfigFile();
    return TMemComboBox::CreateConfigFile();
  }

  /////////////////////////////////////////////////////////////////////////////
  //
  //{{TCoolFindDlg Implementation}}

  //
  // Build a response table for all messages/commands handled
  // by the application.
  //
  DEFINE_RESPONSE_TABLE1(TCoolFindDlg, TDialog)
        EV_COMMAND_ENABLE(IDOK, EvCommandEnableOk),
    EV_COMMAND(IDOK, CmOk),
    EV_COMMAND_ENABLE(IDC_FINDWORD, EvCommandenableFindword),
    EV_COMMAND_ENABLE(IDC_FINDCASE, EvCommandenableFindcase),
    EV_COMMAND_ENABLE(IDC_FINDFRW, EvCommandenableFindfrw),
    EV_COMMAND_ENABLE(IDC_FINDBACK, EvCommandenableFindback),
    EV_COMMAND_ENABLE(IDC_FINDGLOBAL, EvCommandenableFindglobal),
        END_RESPONSE_TABLE;

  //--------------------------------------------------------
  // TCoolFindDlg Constructor
  //
  uint TCoolFindDlg::MessageId = 0;
  //
  TCoolFindDlg::TCoolFindDlg(TWindow* parent, TData& data, TResId resId, TModule* module)
    :
  TDialog(parent, resId ? resId : TResId(IDD_FINDTEXTDLG), module),
    Data(data)
  {
        FindTextBox = new TCoolHistComboBox(this, IDC_FINDTEXT, _T("FindText"));
    FindEngine  = new TComboBox(this, IDC_FINDENGINE, 1);
    FindWord    = new TCheckBox(this, IDC_FINDWORD, 0);
    FindCase    = new TCheckBox(this, IDC_FINDCASE, 0);
    FindGlobal  = new TCheckBox(this, IDC_FINDGLOBAL, 0);
    FindFrw     = new TRadioButton(this, IDC_FINDFRW, 0);
    FindBack    = new TRadioButton(this, IDC_FINDBACK, 0);

    
    //<--Jogy
    //  new TGlyphButton(this, IDOK, TGlyphButton::btOk);
    //  new TGlyphButton(this, IDCANCEL, TGlyphButton::btCancel);
    //Jogy-->
    //  new TGlyphButton(this, IDHELP, TGlyphButton::btHelp);

    if(!TCoolFindDlg::MessageId)
      TCoolFindDlg::MessageId = ::RegisterWindowMessage(FINDMSGSTRING);
  }
  //--------------------------------------------------------
  // TCoolFindDlg Destructor
  //
  TCoolFindDlg::~TCoolFindDlg()
  {
    Destroy(IDCANCEL);
  }
  //
  void
    TCoolFindDlg::SetupWindow()
  {
    TDialog::SetupWindow();

    Data.Flags &= ~TCoolTextWnd::ffMaskFlags;
    if(Data.FindWhat && Data.FindWhat[0])
      FindTextBox->SetText(Data.FindWhat);

    for(int i = 0; i < (int)Data.EngineDescr.Size(); i++)
      FindEngine->AddString(Data.EngineDescr[i]->GetName().c_str());
    FindEngine->SetSelIndex(Data.EngineIndex);

    Data.Error = 0;

    if(Data.Flags&TCoolTextWnd::ffMatchCase)
      FindCase->SetCheck(BF_CHECKED);

    if(Data.Flags&TCoolTextWnd::ffWholeWord)
      FindWord->SetCheck(BF_CHECKED);

    if(Data.Flags&TCoolTextWnd::ffWholeDoc)
      FindGlobal->SetCheck(BF_CHECKED);

    if(Data.Flags&TCoolTextWnd::ffDirectionUp){
      FindFrw->SetCheck(BF_UNCHECKED);
      FindBack->SetCheck(BF_CHECKED);
    }
    else{
      FindFrw->SetCheck(BF_CHECKED);
      FindBack->SetCheck(BF_UNCHECKED);
    }
  }
  //
  void
    TCoolFindDlg::CleanupWindow()
  {
    Data.Flags &= ~TCoolTextWnd::ffMaskFlags;
    Data.Flags |= TCoolTextWnd::ffDlgTerm;
    GetParentO()->SendMessage(MessageId);
    TDialog::CleanupWindow();
  }
  //
  TConfigFile*
    TCoolFindDlg::CreateConfigFile()
  {
    TCoolTextWnd* wnd = TYPESAFE_DOWNCAST(GetParentO(), TCoolTextWnd);
    if(wnd)
      return wnd->CreateConfigFile();
    return new TRegConfigFile(configOWLNExt);
  }
  //
  void
    TCoolFindDlg::GetSelection()
  {
    TTmpBuffer<_TCHAR> buffer(MAX_PATH);
    FindTextBox->GetText(buffer, MAX_PATH);
    if(_tcscmp(buffer,Data.FindWhat) != 0){
      _tcscpy(Data.FindWhat, buffer);
      Data.Flags |= TCoolTextWnd::ffSEChanged;
    }
    int selIdx = FindEngine->GetSelIndex();
    if(selIdx != CB_ERR && selIdx != Data.EngineIndex){
      Data.EngineIndex = selIdx;
      Data.Flags |= TCoolTextWnd::ffSEChanged;
    }

    bool checked    = FindWord->GetCheck() == BF_CHECKED;
    bool flagState  = Data.Flags&TCoolTextWnd::ffWholeWord;
    if(checked != flagState){
      if(checked)
        Data.Flags |= TCoolTextWnd::ffWholeWord;
      else
        Data.Flags &= ~TCoolTextWnd::ffWholeWord;
      Data.Flags |= TCoolTextWnd::ffSEChanged;
    }

    checked   = FindCase->GetCheck() == BF_CHECKED;
    flagState = Data.Flags&TCoolTextWnd::ffMatchCase;
    if(checked != flagState){
      if(checked)
        Data.Flags |= TCoolTextWnd::ffMatchCase;
      else
        Data.Flags &= ~TCoolTextWnd::ffMatchCase;
      Data.Flags |= TCoolTextWnd::ffSEChanged;
    }

    checked   = FindGlobal->GetCheck() == BF_CHECKED;
    flagState = Data.Flags&TCoolTextWnd::ffWholeDoc;
    if(checked != flagState){
      if(checked)
        Data.Flags |= TCoolTextWnd::ffWholeDoc;
      else
        Data.Flags &= ~TCoolTextWnd::ffWholeDoc;
    }

    checked   = FindBack->GetCheck() == BF_CHECKED;
    flagState = Data.Flags&TCoolTextWnd::ffDirectionUp;
    if(checked != flagState){
      if(checked)
        Data.Flags |= TCoolTextWnd::ffDirectionUp;
      else
        Data.Flags &= ~TCoolTextWnd::ffDirectionUp;
      Data.Flags |= TCoolTextWnd::ffSEChanged;
    }
  }
  //
  void
    TCoolFindDlg::CmOk()
  {
    Data.Flags &= ~TCoolTextWnd::ffMaskFlags;
    GetSelection();

    Data.Flags |= TCoolTextWnd::ffFindNext;
    GetParentO()->SendMessage(MessageId);
    Data.Flags &= ~TCoolTextWnd::ffMaskFlags;
  }
  //
  void
    TCoolFindDlg::EvCommandEnableOk(TCommandEnabler& tce)
  {
    TTmpBuffer<_TCHAR> buffer(MAX_PATH);
    FindTextBox->GetText(buffer, MAX_PATH);
    tce.Enable(_tcslen(buffer));
  }
  //
  void TCoolFindDlg::EvCommandenableFindword(TCommandEnabler& tce)
  {
    int selIdx = FindEngine->GetSelIndex();
    if(selIdx != -1)
      tce.Enable((Data.EngineDescr[selIdx]->GetCapability()&TCoolEngineDescr::ecNoWholeWord)==0);
    else
      tce.Enable(false);
  }
  //
  void TCoolFindDlg::EvCommandenableFindcase(TCommandEnabler& tce)
  {
    int selIdx = FindEngine->GetSelIndex();
    if(selIdx != -1)
      tce.Enable((Data.EngineDescr[selIdx]->GetCapability()&TCoolEngineDescr::ecNoCaseSens)==0);
    else
      tce.Enable(false);
  }
  //
  void TCoolFindDlg::EvCommandenableFindfrw(TCommandEnabler& tce)
  {
    int selIdx = FindEngine->GetSelIndex();
    if(selIdx != -1)
      tce.Enable((Data.EngineDescr[selIdx]->GetCapability()&TCoolEngineDescr::ecNoDirection)==0);
    else
      tce.Enable(false);
  }
  //
  void TCoolFindDlg::EvCommandenableFindback(TCommandEnabler& tce)
  {
    int selIdx = FindEngine->GetSelIndex();
    if(selIdx != -1)
      tce.Enable((Data.EngineDescr[selIdx]->GetCapability()&TCoolEngineDescr::ecNoDirection)==0);
    else
      tce.Enable(false);
  }
  //
  void TCoolFindDlg::EvCommandenableFindglobal(TCommandEnabler& tce)
  {
    int selIdx = FindEngine->GetSelIndex();
    if(selIdx != -1)
      tce.Enable((Data.EngineDescr[selIdx]->GetCapability()&TCoolEngineDescr::ecNoGlobal)==0);
    else
      tce.Enable(false);
  }
  //
  //
  //
  //{{TCoolReplaceDlg Implementation}}

  //
  // Build a response table for all messages/commands handled
  // by the application.
  //
  DEFINE_RESPONSE_TABLE1(TCoolReplaceDlg, TCoolFindDlg)
        EV_COMMAND(IDC_REPLACE, CmReplace),
    EV_COMMAND_ENABLE(IDC_REPLACEALL, CeReplace),
    EV_COMMAND(IDC_REPLACEALL, CmReplaceAll),
    EV_COMMAND_ENABLE(IDC_REPLACEALL, CeReplaceAll),
        END_RESPONSE_TABLE;
  //--------------------------------------------------------
  // TCoolReplaceDlg Constructor
  //
  TCoolReplaceDlg::TCoolReplaceDlg(TWindow* parent, TData& data, TResId resId, TModule* module)
    :
  TCoolFindDlg(parent, data, resId ? resId : TResId(IDD_REPLACEDLG), module)
  {
        ReplaceTextBox = new TCoolHistComboBox(this, IDC_REPLACETEXT, _T("ReplaceText"));
    
    new TGlyphButton(this, IDC_REPLACE, TGlyphButton::btEdit);
    new TGlyphButton(this, IDC_REPLACEALL, TGlyphButton::btRevert);
    // INSERT>> Your constructor code here.
  }
  //--------------------------------------------------------
  // TCoolReplaceDlg Destructor
  //
  TCoolReplaceDlg::~TCoolReplaceDlg()
  {
    Destroy(IDCANCEL);
  }
  //
  void
    TCoolReplaceDlg::CmReplace()
  {
    Data.Flags &= ~TCoolTextWnd::ffMaskFlags;
    GetSelection();

    TTmpBuffer<_TCHAR> buffer(MAX_PATH);
    ReplaceTextBox->GetText(buffer, MAX_PATH);
    if(_tcscmp(buffer,Data.ReplaceWith) != 0){
      _tcscpy(Data.ReplaceWith, buffer);
      Data.Flags |= TCoolTextWnd::ffSEChanged;
    }

    Data.Flags |= TCoolTextWnd::ffReplace;
    GetParentO()->SendMessage(MessageId);
    Data.Flags &= ~TCoolTextWnd::ffMaskFlags;
  }
  //
  void
    TCoolReplaceDlg::CeReplace(TCommandEnabler& tce)
  {
    TTmpBuffer<_TCHAR> buffer(MAX_PATH);
    ReplaceTextBox->GetText(buffer, MAX_PATH);
    tce.Enable(_tcslen(buffer));
  }
  //
  void
    TCoolReplaceDlg::CmReplaceAll()
  {
    Data.Flags &= ~TCoolTextWnd::ffMaskFlags;
    GetSelection();

    TTmpBuffer<_TCHAR> buffer(MAX_PATH);
    ReplaceTextBox->GetText(buffer, MAX_PATH);
    if(_tcscmp(buffer,Data.ReplaceWith) != 0){
      _tcscpy(Data.ReplaceWith, buffer);
      Data.Flags |= TCoolTextWnd::ffSEChanged;
    }

    Data.Flags |= TCoolTextWnd::ffReplaceAll;
    GetParentO()->SendMessage(MessageId);
    Data.Flags &= ~TCoolTextWnd::ffMaskFlags;
  }
  //
  void
    TCoolReplaceDlg::CeReplaceAll(TCommandEnabler& tce)
  {
    TTmpBuffer<_TCHAR> buffer(MAX_PATH);
    ReplaceTextBox->GetText(buffer, MAX_PATH);
    tce.Enable(_tcslen(buffer));
  }
  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////
  //
  //{{TCoolSearchWnd Implementation}}
  //

  // Build a response table for all messages/commands handled
  // by the application.
  //
  DEFINE_RESPONSE_TABLE1(TCoolSearchWnd, TCoolTextWnd)
        EV_COMMAND(CM_EDITFIND, CmEditFind),
    EV_COMMAND(CM_EDITFINDNEXT, CmEditFindNext),
    EV_COMMAND_ENABLE(CM_EDITFINDNEXT, CeEditFindNext),
    EV_REGISTERED(FINDMSGSTRING, EvFindMsg),
        END_RESPONSE_TABLE;

  //--------------------------------------------------------
  // TCoolSearchWnd
  //
  TCoolSearchWnd::TCoolSearchWnd(TWindow* parent, int id, LPCTSTR title,
    int x, int y,int w, int h, TModule* module)
    :
  TCoolTextWnd(parent,id,title,x,y,w,h,module),
    SearchCmd(0),
    SearchDialog(0),
    SearchEngine(0)
  {
    SearchData.AddDefaultEngines(SearchData);
  }
  //
  TCoolSearchWnd::~TCoolSearchWnd()
  {
    delete SearchDialog;
    delete SearchEngine;
  }
  //
  // Post a CM_EDITFIND or a CM_EDITREPLACE to re-open a previously open
  // find or replace modeless dialog
  //
  void
    TCoolSearchWnd::SetupWindow()
  {
    TCoolTextWnd::SetupWindow();
    if (SearchCmd)
      PostMessage(WM_COMMAND, SearchCmd);
  }
  //
  bool
    TCoolSearchWnd::Search(const TEditPos& _startPos, LPCTSTR text, TFindFlags flags)
  {
    if (!text || !text[0])
      return false;
    bool firstTime = false;
    if(!SearchEngine)
      firstTime = true;

    if(flags&ffSEChanged){
      delete SearchEngine;
      SearchEngine = 0;
    }

    if(!SearchEngine)
      SearchEngine = SearchData.EngineDescr[SearchData.EngineIndex]->Create(text,
      ToBool(flags&TCoolTextWnd::ffMatchCase),
      ToBool(flags&TCoolTextWnd::ffWholeWord),
      ToBool(flags&TCoolTextWnd::ffDirectionUp));

    CHECK(SearchEngine);

    TCoolTextBuffer& buffer = *GetBuffer();
    TEditPos startPos(_startPos);
    if(!startPos.Valid())
      startPos = CursorPos;
    TEditPos endPos;

    if(flags&ffWholeDoc){
      if(firstTime)
        startPos = TEditPos(-1,-1);
      endPos = TEditPos(-1,-1);
    }
    else{
      if(flags&ffDirectionUp)
        endPos = TEditPos(0,0);
      else{
        int lastLine = buffer.GetLineCount()-1;
        endPos = TEditPos(buffer.GetLineLength(lastLine),lastLine);
      }
    }

    TEditRange searchRange(startPos,endPos);
    searchRange.Normalize();

    TEditRange resRange = SearchEngine->Search(buffer, searchRange);

    // If we've got a match, select that text, cleanup & return.
    //
    if(resRange.Valid()){
      SetSelection(resRange);
      CursorPos = flags&TCoolTextWnd::ffDirectionUp ? resRange.Start() : resRange.End();
      ScrollToCaret(CursorPos);
      return true;
    }
    return false;
  }

  //
  // Perform a search or replace operation based on information in SearchData
  //
  void
    TCoolSearchWnd::DoSearch()
  {
    if(Search(NULL_POS, SearchData.FindWhat, (TFindFlags)SearchData.Flags)){
      ;// All for this
    }
    else {
      if(ToBool(SearchData.Flags&ffFindNext)){
        tstring errTemplate(GetModule()->LoadString(IDS_CANNOTFIND));
        TTmpBuffer<_TCHAR>  errMsg(MAX_PATH);
        _stprintf(errMsg, errTemplate.c_str(), (LPCTSTR)SearchData.FindWhat);
        SearchDialog->MessageBox(errMsg, 0, MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
      }
    }
  }

  //
  // Open the modeless Find commdlg
  //
  void
    TCoolSearchWnd::CmEditFind()
  {
    if(!SearchCmd) {
      SearchCmd = CM_EDITFIND;
      delete SearchDialog;
      delete SearchEngine;
      SearchEngine = 0;
      SetFlag(cfShowInactiveSel);
      if(IsFlagSet(cfLookWrdUnderCur))
        WordUnderCursor(SearchData.FindWhat, SearchData.BuffSize);
      SearchDialog = new TCoolFindDlg(this, SearchData);
      SearchDialog->Create();
      SearchDialog->ShowWindow(SW_SHOWDEFAULT);
    }
  }
  //
  void
    TCoolSearchWnd::CmEditFindNext()
  {
    SearchData.Flags |= ffFindNext;
    DoSearch();
  }
  //
  void
    TCoolSearchWnd::CeEditFindNext(TCommandEnabler& ce)
  {
    ce.Enable(SearchData.FindWhat && *SearchData.FindWhat);
  }
  //
  TResult
    TCoolSearchWnd::EvFindMsg(TParam1, TParam2)
  {
    PRECONDITION(SearchDialog);

    if(SearchData.Flags&ffDlgTerm){
      SearchCmd = 0;
      ClearFlag(cfShowInactiveSel);
      delete SearchEngine;
      SearchEngine = 0;
    }
    else
      DoSearch();
    return 0;
  }


  //-------------------------------------------------------------------------------------
  //
  // Undo/Redo handling
  // ~~~~~~~~~~~~~~~~~~
  // TODO>> Redo handling
  //
  //-----------------------------------------------------------
  // class TUndoInsertChar
  // ~~~~~ ~~~~~~~~~~~~~~~
  //
  class TUndoInsertChar: public TCoolTextBuffer::TUndoNode{
    typedef TCoolTextBuffer::TUndoNode TUNode;
  public:
    TUndoInsertChar(uint key, int endLine, int endChar,
      TUNode* undoSel, bool overType, bool dirtyFlag);
    ~TUndoInsertChar();
    virtual bool Undo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  private:
    TUndoInsertChar(const TUndoInsertChar&);
    TUndoInsertChar& operator=(const TUndoInsertChar&);

    _TCHAR  Char;
    TUNode* UndoSel;
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!111
  class TRedoInsertChar: public TCoolTextBuffer::TRedoNode{
    typedef TCoolTextBuffer::TUndoNode TUNode;
  public:
    TRedoInsertChar(TCoolEdit& wnd);
    virtual bool Redo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  private:
    TRedoInsertChar(const TRedoInsertChar&);
    TRedoInsertChar& operator=(const TRedoInsertChar&);
  };
  //
  TUndoInsertChar::TUndoInsertChar(uint key, int endLine, int endChar,
    TUNode* undoSel, bool overType, bool dirtyFlag)
    :
  TUNode(TEditPos(endChar,endLine),dirtyFlag),
    Char((_TCHAR)0xFF),
    UndoSel(undoSel)
  {
    if(overType)
      Char = (_TCHAR)key;
  }
  //
  TUndoInsertChar::~TUndoInsertChar()
  {
    delete UndoSel;
  }
  //
  bool
    TUndoInsertChar::Undo(TCoolTextBuffer& buffer, TEditPos* newPos)
  {
    // create redo node??
    //buffer->AddRedoNode(new TRedoInsertChar());

    if(Char != (_TCHAR)0xFF)
      buffer.GetLineText(SavedPos.row)[SavedPos.col] = Char;
    else{
      TEditRange range(SavedPos.col,SavedPos.row,SavedPos.col+1, SavedPos.row);
      buffer.DeleteText(range);
    }
    if(newPos)
      *newPos = SavedPos;
    buffer.SetDirty(Dirty);
    if(UndoSel)
      return UndoSel->Undo(buffer, newPos);
    return true;
  }
  //
  bool
    TUndoInsertChar::GetDescription(_TCHAR* buffer, int len, TModule* module)
  {
    return TCoolTextBuffer::LoadStringIndex(buffer, len, IDS_COOLUNDOSTRINGS, 1, _T('|'), module);
  }
  //
  TRedoInsertChar::TRedoInsertChar(TCoolEdit&)
  { }
  //
  bool
    TRedoInsertChar::Redo(TCoolTextBuffer&, TEditPos* /*newPos*/)
  { return true; }
  //
  bool
    TRedoInsertChar::GetDescription(_TCHAR* /*buffer*/, int /*len*/, TModule*)
  { return true; }
  //////////////////////////////////////////////////////////////////////////////////////
  //-----------------------------------------------------------
  // class TUndoReplaceText
  // ~~~~~ ~~~~~~~~~~~~~~~~
  //
  class TUndoReplaceText: public TCoolTextBuffer::TUndoNode {
    typedef TCoolTextBuffer::TUndoNode TUNode;
  public:
    TUndoReplaceText(const TEditPos& curPos, const TEditPos& endPos, TUNode* undoSel, bool dirtyFlag);
    ~TUndoReplaceText();
    virtual bool Undo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  protected:
    TEditPos EndPos;
  private:
    TUndoReplaceText(const TUndoReplaceText&);
    TUndoReplaceText& operator=(const TUndoReplaceText&);

    TUNode* UndoSel;
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  class TRedoReplaceText: public TCoolTextBuffer::TRedoNode{
  public:
    TRedoReplaceText(TCoolEdit& wnd);
    virtual bool Redo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  private:
    TRedoReplaceText(const TRedoReplaceText&);
    TRedoReplaceText& operator=(const TRedoReplaceText&);
  };
  //
  TUndoReplaceText::TUndoReplaceText(const TEditPos& curPos, const TEditPos& endPos,
    TUNode* undoSel, bool dirtyFlag)
    :
  TUNode(curPos,dirtyFlag),
    UndoSel(undoSel),
    EndPos(endPos)
  {
  }
  //
  TUndoReplaceText::~TUndoReplaceText()
  {
    delete UndoSel;
  }
  //
  bool
    TUndoReplaceText::Undo(TCoolTextBuffer& buffer, TEditPos* newPos)
  {
    // create redo node??
    //buffer->AddRedoNode(new TRedoInsertChar());

    TEditRange range(SavedPos, EndPos);
    buffer.DeleteText(range);
    if(newPos)
      *newPos = SavedPos;
    buffer.SetDirty(Dirty);
    if(UndoSel)
      return UndoSel->Undo(buffer, newPos);
    return true;
  }
  //
  bool
    TUndoReplaceText::GetDescription(_TCHAR* buffer, int len, TModule* module)
  {
    return TCoolTextBuffer::LoadStringIndex(buffer, len, IDS_COOLUNDOSTRINGS, 2, _T('|'), module);
  }
  //-----------------------------------------------------------
  // class TUndoKeyEnter
  // ~~~~~ ~~~~~~~~~~~~~
  //
  class TUndoKeyEnter: public TCoolTextBuffer::TUndoNode{
    typedef TCoolTextBuffer::TUndoNode TUNode;
  public:
    TUndoKeyEnter(int endLine, int endChar, TUNode* undoSel, bool dirtyFlag);
    virtual bool Undo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
    void         SetNewPos(int newPos);
  private:
    TUndoKeyEnter(const TUndoKeyEnter&);
    TUndoKeyEnter& operator=(const TUndoKeyEnter&);

    TUNode* UndoSel;
    int     SmartScip;
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  class TRedoKeyEnter: public TCoolTextBuffer::TRedoNode{
  public:
    TRedoKeyEnter(TCoolEdit& wnd);
    virtual bool Redo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  private:
    TRedoKeyEnter(const TRedoKeyEnter&);
    TRedoKeyEnter& operator=(const TRedoKeyEnter&);
  };
  //
  TUndoKeyEnter::TUndoKeyEnter(int endLine, int endChar, TUNode* undoSel,
    bool dirtyFlag)
    :
  TUNode(TEditPos(endChar,endLine),dirtyFlag),
    UndoSel(undoSel),
    SmartScip(0)
  {
  }
  //
  void
    TUndoKeyEnter::SetNewPos(int newPos)
  {
    SmartScip = newPos;
  }
  //
  bool
    TUndoKeyEnter::Undo(TCoolTextBuffer& buffer, TEditPos* newPos)
  {
    //buffer.AddRedoNode(new TRedoInsertChar());
    if(newPos)
      *newPos = SavedPos;
    TEditRange range(SavedPos.col,SavedPos.row, SmartScip, SavedPos.row+1);
    buffer.DeleteText(range);
    buffer.SetDirty(Dirty);
    if(UndoSel)
      return UndoSel->Undo(buffer, newPos);
    return true;
  }
  //
  bool
    TUndoKeyEnter::GetDescription(_TCHAR* buffer, int len, TModule* module)
  {
    return TCoolTextBuffer::LoadStringIndex(buffer, len, IDS_COOLUNDOSTRINGS, 3, _T('|'), module);
  }
  //
  TRedoKeyEnter::TRedoKeyEnter(TCoolEdit&)
  {}
  bool TRedoKeyEnter::Redo(TCoolTextBuffer&, TEditPos* /*newPos*/)
  { return true;}
  bool TRedoKeyEnter::GetDescription(_TCHAR* /*buffer*/, int /*len*/, TModule*)
  { return true;}
  //////////////////////////////////////////////////////////////////////////////////////////
  //TUndoKeyTab(this,repeatCount));
  // problems when undo lines
  class TUndoKeyTab: public TCoolTextBuffer::TUndoNode{
    typedef TCoolTextBuffer::TUndoNode TUNode;
  public:
    TUndoKeyTab(TCoolEdit& wnd, const TEditPos& pos, int count,
      bool ins, bool dirtyFlag);
    virtual bool Undo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  protected:
    int     Count;
    bool    Insert;
  private:
    TUndoKeyTab(const TUndoKeyTab&);
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  class TRedoKeyTab: public TCoolTextBuffer::TRedoNode{
  public:
    TRedoKeyTab(TCoolEdit& wnd);
    virtual bool Redo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  private:
    TRedoKeyTab(const TRedoKeyTab&);
  };
  //
  TUndoKeyTab::TUndoKeyTab(TCoolEdit&, const TEditPos& pos,
    int count, bool ins, bool dirtyFlag)
    :
  TUNode(pos, dirtyFlag),Count(count),Insert(ins)
  {
  }
  //
  bool
    TUndoKeyTab::Undo(TCoolTextBuffer& buffer, TEditPos* newPos)
  {
    //buffer.AddRedoNode(new TRedoInsertChar());
    if(newPos)
      *newPos = SavedPos;
    if(Insert){
      TEditRange range(SavedPos.col,SavedPos.row, SavedPos.col+Count, SavedPos.row);
      buffer.DeleteText(range);
      buffer.SetDirty(Dirty);
    }
    return true;
  }
  //
  bool
    TUndoKeyTab::GetDescription(_TCHAR* buffer, int len, TModule* module)
  {
    return TCoolTextBuffer::LoadStringIndex(buffer, len, IDS_COOLUNDOSTRINGS, 4, _T('|'), module);
  }
  //
  TRedoKeyTab::TRedoKeyTab(TCoolEdit&)
  {}
  bool TRedoKeyTab::Redo(TCoolTextBuffer& /*buffer*/, TEditPos* /*newPos*/)
  { return true;}
  bool TRedoKeyTab::GetDescription(_TCHAR* /*buffer*/, int /*len*/, TModule*)
  { return true;}
  //////////////////////////////////////////////////////////////////////////////////////////
  //TUndoKeyTabify(this,repeatCount));
  class TUndoKeyTabify: public TCoolTextBuffer::TUndoNode{
    typedef TCoolTextBuffer::TUndoNode TUNode;
  public:
    TUndoKeyTabify(int xPos, int yPos1, int yPos2, bool dirtyFlag);
    virtual bool Undo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  private:
    TUndoKeyTabify(const TUndoKeyTab&);
    int yPos;
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!111
  class TRedoKeyTabify: public TCoolTextBuffer::TRedoNode{
  public:
    TRedoKeyTabify(TCoolEdit& wnd);
    virtual bool Redo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  private:
    TRedoKeyTabify(const TRedoKeyTab&);
  };
  //
  TUndoKeyTabify::TUndoKeyTabify(int xPos, int yPos1, int yPos2, bool dirtyFlag)
    :
  TUNode(TEditPos(xPos,yPos1), dirtyFlag),
    yPos(yPos2)
  {
  }
  //
  bool TUndoKeyTabify::Undo(TCoolTextBuffer& buffer, TEditPos* newPos)
  {
    //buffer->AddRedoNode(new TRedoKeyTabify());
    if(newPos)
      *newPos = SavedPos;
    for(int curLine = SavedPos.row; curLine < yPos; curLine++){
      TEditRange range(SavedPos.col,curLine, SavedPos.col+1, curLine);
      buffer.DeleteText(range);
    }
    buffer.SetDirty(Dirty);
    return true;
  }
  //
  bool
    TUndoKeyTabify::GetDescription(_TCHAR* buffer, int len, TModule* module)
  {
    return TCoolTextBuffer::LoadStringIndex(buffer, len, IDS_COOLUNDOSTRINGS, 5, _T('|'), module);
  }
  //
  TRedoKeyTabify::TRedoKeyTabify(TCoolEdit&)
  {}
  bool TRedoKeyTabify::Redo(TCoolTextBuffer& /*buffer*/, TEditPos* /*newPos*/)
  { return true;}
  bool TRedoKeyTabify::GetDescription(_TCHAR* /*buffer*/, int /*len*/, TModule*)
  {   return true;}
  //////////////////////////////////////////////////////////////////////////////////////////
  //TUndoDeleteChar
  class TUndoDeleteChar: public TCoolTextBuffer::TUndoNode {
    typedef TCoolTextBuffer::TUndoNode TUNode;
  public:
    TUndoDeleteChar(TCoolTextBuffer& buffer, int pos_y, int pos_x,
      TUNode* sel);
    virtual bool Undo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  private:
    TUndoDeleteChar(const TUndoDeleteChar&);
    TUNode* UndoSel;
    _TCHAR  Char;
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!111
  class TRedoDeleteChar: public TCoolTextBuffer::TRedoNode{
  public:
    TRedoDeleteChar(TCoolEdit& wnd);
    virtual bool Redo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  private:
    TRedoDeleteChar(const TRedoDeleteChar&);
  };
  //
  TUndoDeleteChar::TUndoDeleteChar(TCoolTextBuffer& buffer, int pos_y, int pos_x,
    TUNode* /*sel*/)
    :
  TUNode(TEditPos(pos_x, pos_y), buffer.IsDirty())
  {
    if(pos_x == buffer.GetLineLength(pos_y))
      Char = (_TCHAR)0xFF;
    else
      Char = buffer.GetLineText(pos_y)[pos_x];
  }
  //
  bool
    TUndoDeleteChar::Undo(TCoolTextBuffer& buffer, TEditPos* newPos)
  {
    if(newPos)
      *newPos = SavedPos;
    TEditPos endPos;
    if(Char == (_TCHAR)0xFF)
      buffer.InsertText(SavedPos, crlf, endPos);
    else{
      _TCHAR buf[2] = {Char, 0};
      buffer.InsertText(SavedPos, buf, endPos);
    }
    buffer.SetDirty(Dirty);
    return true;
  }
  //
  bool
    TUndoDeleteChar::GetDescription(_TCHAR* buffer, int len, TModule* module)
  {
    return TCoolTextBuffer::LoadStringIndex(buffer, len, IDS_COOLUNDOSTRINGS, 6, _T('|'), module);
  }
  //
  TRedoDeleteChar::TRedoDeleteChar(TCoolEdit&)
  {}
  bool TRedoDeleteChar::Redo(TCoolTextBuffer& /*buffer*/, TEditPos* /*newPos*/)
  { return true;}
  bool TRedoDeleteChar::GetDescription(_TCHAR* /*buffer*/, int /*len*/, TModule* /*module*/)
  { return true;}
  //////////////////////////////////////////////////////////////////////////////////////////
  //TUndoDeleteWord(this,repeatCount);
  class TUndoDeleteWord: public TCoolTextBuffer::TUndoNode{
    typedef TCoolTextBuffer::TUndoNode TUNode;
  public:
    TUndoDeleteWord(TCoolTextBuffer& buffer, const TEditPos& savedPos,
      const TEditPos& endPos, TUNode* undoSel);
    virtual bool Undo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  private:
    TUndoDeleteWord(const TUndoDeleteWord&);
  protected:
    TUNode*     UndoSel;
    tstring  Text;
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!111
  class TRedoDeleteWord: public TCoolTextBuffer::TRedoNode{
  public:
    TRedoDeleteWord(TCoolEdit& wnd);
    virtual bool Redo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  private:
    TRedoDeleteWord(const TRedoDeleteWord&);
  };
  //
  TUndoDeleteWord::TUndoDeleteWord(TCoolTextBuffer& buffer,
    const TEditPos& savedPos, const TEditPos& endPos, TUNode* undoSel)
    :
  TUNode(savedPos, buffer.IsDirty()),UndoSel(undoSel)
  {
    TTmpBuffer<_TCHAR> buf(MAX_PATH);
    buffer.GetText(savedPos, endPos, (_TCHAR*)buf, MAX_PATH);

    //Jogy: buf -> (_TCHAR *)buf
    Text = (_TCHAR *)buf;
  }
  //
  bool
    TUndoDeleteWord::Undo(TCoolTextBuffer& buffer, TEditPos* newPos)
  {
    if(newPos)
      *newPos = SavedPos;
    TEditPos endPos;
    buffer.InsertText(SavedPos, Text.c_str(), endPos);
    buffer.SetDirty(Dirty);
    return true;
  }
  //
  bool
    TUndoDeleteWord::GetDescription(_TCHAR* buffer, int len, TModule* module)
  {
    return TCoolTextBuffer::LoadStringIndex(buffer, len, IDS_COOLUNDOSTRINGS, 7, _T('|'), module);
  }
  //
  TRedoDeleteWord::TRedoDeleteWord(TCoolEdit&)
  {}
  bool TRedoDeleteWord::Redo(TCoolTextBuffer& /*buffer*/, TEditPos* /*newPos*/)
  { return true;}
  bool TRedoDeleteWord::GetDescription(_TCHAR* /*buffer*/, int /*len*/, TModule*)
  { return true;}
  //////////////////////////////////////////////////////////////////////////////////////////
  // class TUndoDeleteCharBack
  // ~~~~~ ~~~~~~~~~~~~~~~~~~~
  //
  class TUndoDeleteCharBack: public TCoolTextBuffer::TUndoNode{
    typedef TCoolTextBuffer::TUndoNode TUNode;
  public:
    TUndoDeleteCharBack(TCoolTextBuffer& buffer, int pos_y, int pos_x,
      TUNode* sel);
    virtual bool Undo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  protected:
    TUNode* UndoSel;
    int     PrevLen;
    _TCHAR  Char;
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!111
  class TRedoDeleteCharBack: public TCoolTextBuffer::TRedoNode{
  public:
    TRedoDeleteCharBack(TCoolEdit& wnd);
    virtual bool Redo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  };
  //
  TUndoDeleteCharBack::TUndoDeleteCharBack(TCoolTextBuffer& buffer, int pos_y,
    int pos_x, TUNode* /*sel*/)
    :
  TUNode(TEditPos(pos_x, pos_y), buffer.IsDirty())
  {
    if(pos_x == 0){
      Char = (_TCHAR)0xFF;
      PrevLen = buffer.GetLineLength(pos_y-1);
    }else
      Char = buffer.GetLineText(pos_y)[pos_x-1];
  }
  //
  bool
    TUndoDeleteCharBack::Undo(TCoolTextBuffer& buffer, TEditPos* newPos)
  {
    if(newPos)
      *newPos = SavedPos;
    TEditPos endPos;
    if(Char == (_TCHAR)0xFF)
      buffer.InsertText(TEditPos(PrevLen,SavedPos.row-1), crlf, endPos);
    else{
      _TCHAR buf[2] = {Char, 0};
      buffer.InsertText(TEditPos(SavedPos.col-1,SavedPos.row), buf, endPos);
    }
    buffer.SetDirty(Dirty);
    return true;
  }
  //
  bool
    TUndoDeleteCharBack::GetDescription(_TCHAR* buffer, int len, TModule* module)
  {
    return TCoolTextBuffer::LoadStringIndex(buffer, len, IDS_COOLUNDOSTRINGS, 8, _T('|'), module);
  }
  //
  TRedoDeleteCharBack::TRedoDeleteCharBack(TCoolEdit&)
  {}
  bool TRedoDeleteCharBack::Redo(TCoolTextBuffer& /*buffer*/, TEditPos* /*newPos*/)
  { return true;}
  bool TRedoDeleteCharBack::GetDescription(_TCHAR* /*buffer*/, int /*len*/, TModule*)
  { return true;}
  //-----------------------------------------------------------
  // class TUndoDeleteWordBack
  // ~~~~~ ~~~~~~~~~~~~~~~~~~~
  //
  class TUndoDeleteWordBack: public TCoolTextBuffer::TUndoNode{
    typedef TCoolTextBuffer::TUndoNode TUNode;
  public:
    TUndoDeleteWordBack(TCoolTextBuffer& buffer, const TEditPos& savedPos,
      const TEditPos& startPos, TUNode* undoSel);
    virtual bool Undo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  protected:
    TUNode*     UndoSel;
    TEditPos      SPos;
    tstring  Text;
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!111
  class TRedoDeleteWordBack: public TCoolTextBuffer::TRedoNode{
  public:
    TRedoDeleteWordBack(TCoolEdit& wnd);
    virtual bool Redo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  };
  //
  TUndoDeleteWordBack::TUndoDeleteWordBack(TCoolTextBuffer& buffer,
    const TEditPos& savedPos, const TEditPos& startPos, TUNode* undoSel)
    :
  TUNode(savedPos, buffer.IsDirty()),UndoSel(undoSel),SPos(startPos)
  {
    TTmpBuffer<_TCHAR> buf(MAX_PATH);
    buffer.GetText(startPos, savedPos, buf, MAX_PATH);
    //Jogy: buf -> (_TCHAR *)buf
    Text = (_TCHAR *)buf;
  }
  //
  bool
    TUndoDeleteWordBack::Undo(TCoolTextBuffer& buffer, TEditPos* newPos)
  {
    if(newPos)
      *newPos = SavedPos;
    TEditPos endPos;
    buffer.InsertText(SPos, Text.c_str(), endPos);
    buffer.SetDirty(Dirty);
    return true;
  }
  //
  bool
    TUndoDeleteWordBack::GetDescription(_TCHAR* buffer, int len, TModule* module)
  {
    return TCoolTextBuffer::LoadStringIndex(buffer, len, IDS_COOLUNDOSTRINGS, 9, _T('|'), module);
  }
  //
  TRedoDeleteWordBack::TRedoDeleteWordBack(TCoolEdit&)
  {}
  bool TRedoDeleteWordBack::Redo(TCoolTextBuffer& /*buffer*/, TEditPos* /*newPos*/)
  { return true;}
  bool TRedoDeleteWordBack::GetDescription(_TCHAR* /*buffer*/, int /*len*/, TModule*)
  { return true;}
  //-----------------------------------------------------------
  // class TUndoDeleteSelection
  // ~~~~~ ~~~~~~~~~~~~~~~~~~~~
  //
  class TUndoDeleteSelection: public TCoolTextBuffer::TUndoNode{
    typedef TCoolTextBuffer::TUndoNode TUNode;
  public:
    TUndoDeleteSelection(TCoolEdit& wnd, bool dirtyFlag);
    virtual bool Undo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  protected:
    tostringstream  Os;
    TEditPos        StartPos;
  private:
    TUndoDeleteSelection(const TUndoDeleteSelection&);
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!111
  class TRedoDeleteSelection: public TCoolTextBuffer::TRedoNode{
  public:
    TRedoDeleteSelection(TCoolEdit& wnd);
    virtual bool Redo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);

  private:
    TRedoDeleteSelection(const TRedoDeleteSelection&);
  };
  //
  TUndoDeleteSelection::TUndoDeleteSelection(TCoolEdit& wnd, bool dirtyFlag)
    :
  TUNode(wnd.CursorPos, dirtyFlag)
  {
    PRECONDITION(wnd.Selection);
    wnd.Selection->Copy(Os);

    TEditPos startPos = wnd.Selection->GetStart();
    TEditPos endPos   = wnd.Selection->GetEnd();
    if(startPos.row < endPos.row)
      StartPos = startPos;
    else if(startPos.row == endPos.row)
      StartPos = startPos.col < endPos.col ? startPos : endPos;
    else
      StartPos = endPos;
  }
  //
  static void PasteFromMemoryImp(_TCHAR* pMemory, int size,
    TCoolEdit::TSelType selection,
    TEditPos& cursorPos, TCoolTextBuffer& buffer);
  //
  bool TUndoDeleteSelection::Undo(TCoolTextBuffer& buffer, TEditPos* newPos)
  {
    //buffer.AddRedoNode(new TRedoDelSelection());

    if(newPos)
      *newPos = SavedPos;

    tstring s = Os.str();
    _TCHAR* pMemory = (_TCHAR*)s.c_str();

    TCoolEdit::TSelType selection;
    if(memcmp(pMemory,streamSelectionId,IdSelSize)==0)
      selection = TCoolEdit::stStreamSelection;
    else if(memcmp(pMemory,lineSelectionId,IdSelSize)==0)
      selection = TCoolEdit::stLineSelection;
    else if(memcmp(pMemory,lineSelectionId,IdSelSize)==0)
      selection = TCoolEdit::stColumnSelection;
    else{
      CHECK(false);
      selection = TCoolEdit::stStreamSelection;
    }
    pMemory += IdSelSize;
    PasteFromMemoryImp(pMemory, static_cast<int>(_tcslen(pMemory)), selection, StartPos, buffer);

    buffer.SetDirty(Dirty);
    return true;
  }
  //
  bool
    TUndoDeleteSelection::GetDescription(_TCHAR* buffer, int len, TModule* module)
  {
    return TCoolTextBuffer::LoadStringIndex(buffer, len, IDS_COOLUNDOSTRINGS, 10, _T('|'), module);
  }
  //
  TRedoDeleteSelection::TRedoDeleteSelection(TCoolEdit&)
  {}
  bool TRedoDeleteSelection::Redo(TCoolTextBuffer& /*buffer*/, TEditPos* /*newPos*/)
  { return true;}
  bool TRedoDeleteSelection::GetDescription(_TCHAR* /*buffer*/, int /*len*/, TModule*)
  { return true;}
  //-----------------------------------------------------------
  // class TUndoPaste
  // ~~~~~ ~~~~~~~~~~
  //
  class TUndoPaste: public TCoolTextBuffer::TUndoNode{
    typedef TCoolTextBuffer::TUndoNode TUNode;
  public:
    TUndoPaste(const TEditPos& savedPos, const TEditPos& currPos,
      TUNode* undoSel, bool dirty);
    virtual bool Undo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  private:
    TUndoPaste(const TUndoPaste&);
    TUNode* UndoSel;
    TEditPos  LastPos;
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  class TRedoPaste: public TCoolTextBuffer::TRedoNode{
  public:
    TRedoPaste(TCoolEdit& end);
    virtual bool Redo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  private:
    TRedoPaste(const TRedoPaste&);
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  TUndoPaste::TUndoPaste(const TEditPos& savedPos, const TEditPos& currPos,
    TUNode* undoSel, bool dirty)
    :
  TUNode(savedPos, dirty),UndoSel(undoSel),LastPos(currPos)
  {
  }
  //
  bool
    TUndoPaste::Undo(TCoolTextBuffer& buffer, TEditPos* newPos)
  {
    if(newPos)
      *newPos = SavedPos;

    TEditRange range(SavedPos, LastPos);
    buffer.DeleteText(range);
    buffer.SetDirty(Dirty);
    if(UndoSel)
      return UndoSel->Undo(buffer, newPos);
    return true;
  }
  //
  bool
    TUndoPaste::GetDescription(_TCHAR* buffer, int len, TModule* module)
  {
    return TCoolTextBuffer::LoadStringIndex(buffer, len, IDS_COOLUNDOSTRINGS, 11, _T('|'), module);
  }
  //
  TRedoPaste::TRedoPaste(TCoolEdit&)
  {}
  bool TRedoPaste::Redo(TCoolTextBuffer& /*buffer*/, TEditPos* /*newPos*/)
  { return true;}
  bool TRedoPaste::GetDescription(_TCHAR* /*buffer*/, int /*len*/, TModule*)
  { return true;}
  //////////////////////////////////////////////////////////////////////////////////////////
  //TUndoClearAll(this);
  class TUndoClearAll: public TCoolTextBuffer::TUndoNode{
    typedef TCoolTextBuffer::TUndoNode TUNode;
  public:
    TUndoClearAll(TCoolEdit& wnd, TEditPos& pos, bool dirtyFlag);
    virtual bool Undo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  protected:
    TIPtrArray<TCoolTextBuffer::TLineInfo*> LineArray;
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  class TRedoClearAll: public TCoolTextBuffer::TRedoNode {
  public:
    TRedoClearAll(TCoolEdit& end);
    virtual bool Redo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  };
  //
  TUndoClearAll::TUndoClearAll(TCoolEdit& wnd, TEditPos& pos, bool dirtyFlag)
    :
  TUNode(pos, dirtyFlag)
  {
    PRECONDITION(wnd.GetBuffer());
    TCoolTextBuffer& buffer = *wnd.GetBuffer();
    int size = buffer.GetLineCount();
    for(int i = 0; i < size; i++)
      LineArray.Add(new TCoolTextBuffer::TLineInfo(buffer.GetLineText(i),
      buffer.GetLineLength(i)));
  }
  //
  bool
    TUndoClearAll::Undo(TCoolTextBuffer& buffer, TEditPos* newPos)
  {
    if(newPos)
      *newPos = SavedPos;
    // delete first line
    TEditRange range(0, 0, -1, 0);
    buffer.DeleteText(range);
    int size = LineArray.Size();
    for(int i = 0; i < size; i++)
      buffer.InsertLine(LineArray[i]->Text, LineArray[i]->Length);
    buffer.SetDirty(Dirty);
    return true;
  }
  //
  bool
    TUndoClearAll::GetDescription(_TCHAR* buffer, int len, TModule* module)
  {
    return TCoolTextBuffer::LoadStringIndex(buffer, len, IDS_COOLUNDOSTRINGS, 11, _T('|'), module);
  }
  //
  TRedoClearAll::TRedoClearAll(TCoolEdit&)
  {}
  bool TRedoClearAll::Redo(TCoolTextBuffer& /*buffer*/, TEditPos* /*newPos*/)
  { return true;}
  bool TRedoClearAll::GetDescription(_TCHAR* /*buffer*/, int /*len*/, TModule*)
  { return true;}
  /////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(COOL_NODRAGDROP)
  //-----------------------------------------------------------
  // class TUndoDragDropMoveExt
  // ~~~~~ ~~~~~~~~~~~~~~~~~~~~
  //
  class TUndoDragDropMoveExt: public TCoolTextBuffer::TUndoNode{
    typedef TCoolTextBuffer::TUndoNode TUNode;
  public:
    TUndoDragDropMoveExt(TCoolEdit& wnd, bool dirtyFlag);
    virtual bool Undo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  protected:
    tostringstream  Os;
    TEditPos      StartPos;
  private:
    TUndoDragDropMoveExt(const TUndoDragDropMoveExt&);
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!111
  class TRedoDragDropMoveExt: public TCoolTextBuffer::TRedoNode{
  public:
    TRedoDragDropMoveExt(TCoolEdit& wnd);
    virtual bool Redo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);

  private:
    TRedoDragDropMoveExt(const TRedoDragDropMoveExt&);
  };
  //
  TUndoDragDropMoveExt::TUndoDragDropMoveExt(TCoolEdit& wnd, bool dirtyFlag)
    :
  TUNode(wnd.CursorPos, dirtyFlag)
  {
    PRECONDITION(wnd.Selection);
    wnd.Selection->Copy(Os);

    TEditPos startPos = wnd.Selection->GetStart();
    TEditPos endPos   = wnd.Selection->GetEnd();
    if(startPos.row < endPos.row)
      StartPos = startPos;
    else if(startPos.row == endPos.row)
      StartPos = startPos.col < endPos.col ? startPos : endPos;
    else
      StartPos = endPos;
  }
  //
  bool TUndoDragDropMoveExt::Undo(TCoolTextBuffer& buffer, TEditPos* newPos)
  {
    //buffer.AddRedoNode(new TRedoDragDropMoveExt());

    if(newPos)
      *newPos = SavedPos;

    tstring s = Os.str();
    _TCHAR* pMemory = (_TCHAR*)s.c_str();

    TCoolEdit::TSelType selection;
    if(memcmp(pMemory,streamSelectionId,IdSelSize)==0)
      selection = TCoolEdit::stStreamSelection;
    else if(memcmp(pMemory,lineSelectionId,IdSelSize)==0)
      selection = TCoolEdit::stLineSelection;
    else if(memcmp(pMemory,lineSelectionId,IdSelSize)==0)
      selection = TCoolEdit::stColumnSelection;
    else{
      CHECK(false);
      selection = TCoolEdit::stStreamSelection;
    }
    pMemory += IdSelSize;
    PasteFromMemoryImp(pMemory, static_cast<int>(_tcslen(pMemory)), selection, StartPos, buffer);

    buffer.SetDirty(Dirty);
    return true;
  }
  //
  bool
    TUndoDragDropMoveExt::GetDescription(_TCHAR* buffer, int len, TModule* module)
  {
    return TCoolTextBuffer::LoadStringIndex(buffer, len, IDS_COOLUNDOSTRINGS, 12, _T('|'), module);
  }
  //
  TRedoDragDropMoveExt::TRedoDragDropMoveExt(TCoolEdit&)
  {}
  bool TRedoDragDropMoveExt::Redo(TCoolTextBuffer& /*buffer*/, TEditPos* /*newPos*/)
  { return true;}
  bool TRedoDragDropMoveExt::GetDescription(_TCHAR* /*buffer*/, int /*len*/, TModule*)
  { return true;}
  //-----------------------------------------------------------
  // class TUndoDragDropCopy
  // ~~~~~ ~~~~~~~~~~~~~~~~~
  //
  class TUndoDragDropCopy: public TCoolTextBuffer::TUndoNode{
    typedef TCoolTextBuffer::TUndoNode TUNode;
  public:
    TUndoDragDropCopy(const TEditPos& savedPos, const TEditPos& currPos, bool dirty);
    virtual bool Undo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  private:
    TUndoDragDropCopy(const TUndoDragDropCopy&);
    TEditPos  LastPos;
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  class TRedoDragDropCopy: public TCoolTextBuffer::TRedoNode{
  public:
    TRedoDragDropCopy(TCoolEdit& end);
    virtual bool Redo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  private:
    TRedoDragDropCopy(const TRedoDragDropCopy&);
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  TUndoDragDropCopy::TUndoDragDropCopy(const TEditPos& savedPos,
    const TEditPos& currPos,
    bool dirty)
    :
  TUNode(savedPos, dirty), LastPos(currPos)
  {
  }
  //
  bool
    TUndoDragDropCopy::Undo(TCoolTextBuffer& buffer, TEditPos* newPos)
  {
    if(newPos)
      *newPos = SavedPos;
    TEditRange range(SavedPos, LastPos);
    buffer.DeleteText(range);
    buffer.SetDirty(Dirty);
    return true;
  }
  //
  bool
    TUndoDragDropCopy::GetDescription(_TCHAR* buffer, int len, TModule* module)
  {
    return TCoolTextBuffer::LoadStringIndex(buffer, len, IDS_COOLUNDOSTRINGS, 13, _T('|'), module);
  }
  //
  TRedoDragDropCopy::TRedoDragDropCopy(TCoolEdit&)
  {}
  bool TRedoDragDropCopy::Redo(TCoolTextBuffer& /*buffer*/, TEditPos* /*newPos*/)
  { return true;}
  bool TRedoDragDropCopy::GetDescription(_TCHAR* /*buffer*/, int /*len*/, TModule*)
  { return true;}
  //-----------------------------------------------------------
  // class TUndoDragDropMove
  // ~~~~~ ~~~~~~~~~~~~~~~~~
  //
  class TUndoDragDropMove: public TCoolTextBuffer::TUndoNode{
    typedef TCoolTextBuffer::TUndoNode TUNode;
  public:
    TUndoDragDropMove(const TEditPos& savedPos, const TEditPos& currPos,
      TUNode* undoSel, bool dirty);
    virtual bool Undo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  private:
    TUndoDragDropMove(const TUndoDragDropMove&);
    TUNode*    UndoSel;
    TEditPos  LastPos;
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  class TRedoDragDropMove: public TCoolTextBuffer::TRedoNode{
  public:
    TRedoDragDropMove(TCoolEdit& end);
    virtual bool Redo(TCoolTextBuffer& buffer, TEditPos* newPos=0);
    virtual bool GetDescription(_TCHAR* buffer, int len, TModule* module = owl::Module);
  private:
    TRedoDragDropMove(const TRedoDragDropMove&);
  };
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  TUndoDragDropMove::TUndoDragDropMove(const TEditPos& savedPos,
    const TEditPos& currPos,
    TUNode* undoSel, bool dirty)
    :
  TUNode(savedPos, dirty), UndoSel(undoSel), LastPos(currPos)
  {
  }
  //
  bool
    TUndoDragDropMove::Undo(TCoolTextBuffer& buffer, TEditPos* newPos)
  {
    if(newPos)
      *newPos = SavedPos;

    TEditRange range(SavedPos, LastPos);
    buffer.DeleteText(range);
    buffer.SetDirty(Dirty);
    if(UndoSel)
      return UndoSel->Undo(buffer, newPos);
    return true;
  }
  //
  bool
    TUndoDragDropMove::GetDescription(_TCHAR* buffer, int len, TModule* module)
  {
    return TCoolTextBuffer::LoadStringIndex(buffer, len, IDS_COOLUNDOSTRINGS, 14, _T('|'), module);
  }
  //
  TRedoDragDropMove::TRedoDragDropMove(TCoolEdit&)
  {}
  bool TRedoDragDropMove::Redo(TCoolTextBuffer& /*buffer*/, TEditPos* /*newPos*/)
  { return true;}
  bool TRedoDragDropMove::GetDescription(_TCHAR* /*buffer*/, int /*len*/, TModule*)
  { return true;}
#endif
  /////////////////////////////////////////////////////////////////////////////////////////////
  //
#if !defined(COOL_NODRAGDROP)
  //-------------------------------------------------------------------------------------------
  // class TCoolEdit::TEditDragDropProxy
  // ~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  //
  TCoolEdit::TEditDragDropProxy::TEditDragDropProxy(TCoolEdit* wnd)
    :
  TDragDropProxy(wnd)
  {
  }
  //
  void TCoolEdit::TEditDragDropProxy::ShowDropIndicator(TPoint& point)
  {
    TYPESAFE_DOWNCAST(Parent, TCoolEdit)->ShowDropIndicator(point);
  }
  //
  void TCoolEdit::TEditDragDropProxy::HideDropIndicator()
  {
    TYPESAFE_DOWNCAST(Parent, TCoolEdit)->HideDropIndicator();
  }
  //
  uint
    TCoolEdit::TEditDragDropProxy::IsDroppable(const TPoint& point, const TRect& clientRect)
  {
    TRect rect(clientRect);
    rect.left += TYPESAFE_DOWNCAST(Parent, TCoolEdit)->GetMarginWidth();
    return TDragDropProxy::IsDroppable(point, rect);
  }
  //
  STDMETHODIMP
    TCoolEdit::TEditDragDropProxy::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
  {
    *pdwEffect=DROPEFFECT_NONE;

    if (DataObject == 0)
      return NOERROR;
    TCoolEdit* coolEdit = TYPESAFE_DOWNCAST(Parent, TCoolEdit);

    if(coolEdit->IsReadOnly() || !coolEdit->IsDragDrop()){
      if(ShowIndicator)
        HideDropIndicator();  // Hide Drop Caret
      ShowIndicator = false;
      return NOERROR;
    }
    return TDragDropProxy::DragOver(grfKeyState, pt, pdwEffect);
  }
  //
  bool
    TCoolEdit::TEditDragDropProxy::PerformDrop(IDataObject* dataObj, const TPoint& point, DROPEFFECT de)
  {
    TCoolEdit* coolEdit = TYPESAFE_DOWNCAST(Parent, TCoolEdit);
    coolEdit->DropText(dataObj, point, de);
    if(coolEdit->IsFlagSet(cfDraggingText))
      return true;
    return false;
  }
  //
#endif
  //-------------------------------------------------------------------------------------------
  //
  //{{TCoolEdit Implementation}}

  //
  // Build a response table for all messages/commands handled
  // by the application.
  //
  DEFINE_RESPONSE_TABLE1(TCoolEdit, TCoolSearchWnd)
        EV_COMMAND_ENABLE(CM_EDITFIND,    CeEditFindReplace),
    EV_COMMAND(CM_EDITREPLACE,        CmEditReplace),
    EV_COMMAND_ENABLE(CM_EDITREPLACE, CeEditFindReplace),
    EV_COMMAND(CM_EDITCUT,            CmEditCut),
    EV_COMMAND(CM_EDITPASTE,          CmEditPaste),
    //  EV_COMMAND(CM_EDITDELETE,         CmEditDelete), // no Delete
    EV_COMMAND(CM_EDITCLEAR,          CmEditClear),  // ???
    EV_COMMAND(CM_EDITUNDO,           CmEditUndo),
    EV_COMMAND(CM_EDITREDO,           CmEditRedo),
    //  EV_COMMAND_ENABLE(CM_EDITCUT,     CeSelectEnable),
    //  EV_COMMAND_ENABLE(CM_EDITDELETE,  CeSelectEnable), // No Delete
    EV_COMMAND_ENABLE(CM_EDITPASTE,   CePasteEnable),
    EV_COMMAND_ENABLE(CM_EDITCLEAR,   CeCharsEnable),
    EV_COMMAND_ENABLE(CM_EDITUNDO,    CeUndoEnable),
    EV_COMMAND_ENABLE(CM_EDITREDO,    CeRedoEnable),
    EV_WM_CHAR,
    EV_WM_KEYDOWN,
        END_RESPONSE_TABLE;
  //--------------------------------------------------------
  // TCoolEdit
  //
  TCoolEdit::TCoolEdit(TWindow* parent, int id, LPCTSTR title, int x, int y,
    int w, int h, TModule* module)
    :
  TCoolSearchWnd(parent,id,title,x,y,w,h,module)
  {
    EnableCaret();
#if !defined(COOL_NODRAGDROP)
    delete SetDragDropProxy(new TEditDragDropProxy(this));
#endif
  }
  //
  TCoolEdit::~TCoolEdit()
  {
    // INSERT>> Your destructor code here.
  }
  //
  // Perform a search or replace operation based on information in SearchData
  //
  void
    TCoolEdit::DoSearch()
  {
    do{
      if(Search(NULL_POS, SearchData.FindWhat, (TFindFlags)SearchData.Flags)){
        if(SearchData.Flags & (ffReplace|ffReplaceAll))
          ReplaceText(SearchData.ReplaceWith);
      }
      else {
        if (SearchData.Flags & (ffFindNext|ffReplace)) {
          tstring errTemplate(GetModule()->LoadString(IDS_CANNOTFIND));
          TCHAR  errMsg[81];
          _stprintf(errMsg, errTemplate.c_str(), (LPCTSTR)SearchData.FindWhat);
          /*SearchDialog->*/MessageBox(errMsg, 0, MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
        }
        else if (SearchData.Flags&ffReplaceAll)
          break;
      }
    }
    while (SearchData.Flags&ffReplaceAll);
  }
  //
  void
    TCoolEdit::InsertText(LPCTSTR text)
  {
    ClearSelection();
    ReplaceText(text);
  }
  //
  void
    TCoolEdit::ReplaceText(LPCTSTR text)
  {
    if(IsReadOnly())
      return;

    PRECONDITION(GetBuffer());
    TCoolTextBuffer& buffer = *GetBuffer();

    TUndoDeleteSelection* undoSel = 0;
    if(Selection)
      undoSel = new TUndoDeleteSelection(*this, buffer.IsDirty());

    DeleteSelection(1);

    TEditPos savedPos = CursorPos;
    buffer.InsertText(CursorPos, text, CursorPos);
    buffer.AddUndoNode(new TUndoReplaceText(savedPos, CursorPos,
      undoSel, buffer.IsDirty()));

    InvalidateLines(savedPos.row, CursorPos.row);
    AdjustScroller();
    ScrollToCaret(CursorPos);
  }
  //
#if !defined(COOL_NODRAGDROP)
  DROPEFFECT
    TCoolEdit::GetDropEffect()
  {
    //_TRACE("TCoolEdit::GetDropEffect\n");
    return DROPEFFECT_COPY|DROPEFFECT_MOVE;
  }
  //
  void
    TCoolEdit::DropSource(DROPEFFECT de)
  {
    //_TRACE("TCoolEdit::DropSource()\n");
    // if not our window and move -> delete selection
    if(de == DROPEFFECT_MOVE){
      TCoolTextBuffer& buffer = *GetBuffer();
      buffer.AddUndoNode(new TUndoDragDropMoveExt(*this, buffer.IsDirty()));
      DeleteSelection(1);
    }
  }
  //
  void
    TCoolEdit::PastFromDataObject(IDataObject* dataObj)
  {
    //_TRACE("TCoolEdit::PastFromDataObject() START\n");
    // Actual Paste
    HGLOBAL hGlobal = 0;
    _TCHAR* pMemory = 0;
    TSelType selection = stStreamSelection;
    TStorageMedium stm;

    TFormatEtc sfe(TCoolEdit::SelClipFormat, TYMED_HGLOBAL);
#ifdef _UNICODE
    TFormatEtc tfe(CF_UNICODETEXT, TYMED_HGLOBAL);
#else
    TFormatEtc tfe(CF_TEXT, TYMED_HGLOBAL);
#endif
    if(dataObj->QueryGetData(&sfe) == NOERROR){
      HRESULT hr = dataObj->GetData(&sfe, &stm);
      if (SUCCEEDED(hr))
        hGlobal = stm.hGlobal;
      if(hGlobal)
        pMemory = (_TCHAR*)::GlobalLock(hGlobal);
      if(pMemory){
        if(memcmp(pMemory,streamSelectionId,IdSelSize)==0)
          selection = stStreamSelection;
        else if(memcmp(pMemory,lineSelectionId,IdSelSize)==0)
          selection = stLineSelection;
        else if(memcmp(pMemory,lineSelectionId,IdSelSize)==0)
          selection = stColumnSelection;
        else{
          pMemory = 0;
        }
        if(pMemory)
          pMemory += IdSelSize;
      }
    }
    else if(dataObj->QueryGetData(&tfe) == NOERROR){
      HRESULT hr = dataObj->GetData(&tfe, &stm);
      if (SUCCEEDED(hr))
        hGlobal = stm.hGlobal;
      if(hGlobal)
        pMemory = (_TCHAR*)::GlobalLock(hGlobal);
    }
    else { // check additional source
#ifdef _UNICODE
      TFormatEtc afe(CF_TEXT, TYMED_HGLOBAL);
#else
      TFormatEtc afe(CF_UNICODETEXT, TYMED_HGLOBAL);
#endif
      if(dataObj->QueryGetData(&afe) == NOERROR){
        HRESULT hr = dataObj->GetData(&tfe, &stm);
        if (SUCCEEDED(hr))
          hGlobal = stm.hGlobal;
        if(hGlobal){
          USES_CONVERSION;
#ifdef _UNICODE
          LPTSTR ptr  = A2W((LPSTR)::GlobalLock(hGlobal));
#else
          LPTSTR ptr = W2A((LPWSTR)::GlobalLock(hGlobal));
#endif
          if(ptr)
            PasteFromMemory(ptr, static_cast<int>(_tcslen(ptr)), selection);
        }
        pMemory = 0;
      }
    }
    if(pMemory)
      PasteFromMemory(pMemory, static_cast<int>(_tcslen(pMemory)), selection);

    if(hGlobal)
      ::GlobalUnlock(hGlobal);
    ReleaseStgMedium(&stm);

    //_TRACE("TCoolEdit::PastFromDataObject() END\n");
  }
  //
  void
    TCoolEdit::DropText(IDataObject* src, const TPoint& point, DROPEFFECT de)
  {
    //_TRACE("TCoolEdit::DropText() START\n");
    bool our_source = IsFlagSet(cfDraggingText);

    //TEditPos pastePos = Client2Text(Text2Client(Client2Text(point)));
    // if cursor under active selection and our source -> do nothig
    if(our_source && IsInSelection(Client2Text(point))){
      ClearSelection();
      return;
    }

    TUndoDeleteSelection* undoSel = 0;
    if(de == DROPEFFECT_COPY || !our_source){
      ClearSelection();
    }

    TCoolTextBuffer& buffer = *GetBuffer();
    if(our_source && Selection){
      undoSel = new TUndoDeleteSelection(*this, buffer.IsDirty());
      DeleteSelection(1);
    }
    bool savedDirty   = buffer.IsDirty();
    TEditPos pastePos = Client2Text(point);

    SetCursorPos(pastePos); // new paste position
    PastFromDataObject(src);
    if(de == DROPEFFECT_COPY)
      buffer.AddUndoNode(new TUndoDragDropCopy(pastePos, CursorPos, savedDirty));
    else
      buffer.AddUndoNode(new TUndoDragDropMove(pastePos, CursorPos, undoSel, savedDirty));
    // and last Select new region
    SetSelection(TEditRange(pastePos, CursorPos));
    //_TRACE("TCoolEdit::DropText() END\n");
  }
  //
  void TCoolEdit::ShowDropIndicator(const TPoint& point)
  {
    if(!IsFlagSet(cfDropPosVisible)){
      HideCursor();
      SavedCaretPos = GetCursorPos();
      SetFlag(cfDropPosVisible);
      ::CreateCaret(GetHandle(), (HBITMAP)1, 2, GetLineHeight());
    }
    //if(dropPos.col >= GetOffsetChar()){
    if(point.x >= GetOffsetChar()){
      TEditPos dropPos = Client2Text(point);
      SetCaretPos(Text2Client(dropPos));
      ShowCaret();
    }
    else{
      HideCaret();
    }
  }
  //
  void TCoolEdit::HideDropIndicator()
  {
    if(IsFlagSet(cfDropPosVisible)){
      ClearFlag(cfHasCaret);
      SetCursorPos(SavedCaretPos);
      ClearFlag(cfDropPosVisible);
      ShowCursor();
    }
  }
#endif
  //
  void
    TCoolEdit::EvKeyDown(uint key, uint repeatCount, uint flags)
  {
    //We also need to handle delete & backspace here. Note that
    //we don't use the repeat count - it's a little safer that
    //way as Windows can stack up a *lot* of them if it was busy
    //elsewhere for a while.
    bool is_ctrl  = ::GetAsyncKeyState(VK_CONTROL) & MSB;
    switch (key){
  case VK_BACK:
    if(is_ctrl)
      DeleteWordBack(1 /*repeatCount*/);
    else
      DeleteCharBack(1 /*repeatCount*/);
    return;

  case VK_DELETE:
    if(is_ctrl)
      DeleteWord(1 /*repeatCount*/);
    else
      DeleteChar(1 /*repeatCount*/);
    return;
    }
    TCoolSearchWnd::EvKeyDown(key, repeatCount, flags);
  }
  //
  void
    TCoolEdit::EvChar(uint key, uint repeatCount, uint flags)
  {
    //We don't deal with console bells etc.
    if (key < VK_ESCAPE && key != VK_TAB && key != VK_RETURN)
      return;

    if(key > 31 && key < 127)
      InsertChar(key, repeatCount);
    else if(key == VK_RETURN)
      KeyEnter(repeatCount);
    else if(key == VK_TAB){
      bool is_shift = ::GetAsyncKeyState(VK_SHIFT) & MSB;
      if(is_shift)
        KeyUnTab(repeatCount);
      else
        KeyTab(repeatCount);
    }
    else
      TCoolSearchWnd::EvChar(key, repeatCount, flags);

    //ScrollToCaret();
    //UpdateWindow();//?????????? or Invaliadte(false)???
  }
  //
  int TCoolEdit::DeleteSelection(uint repeatCount)
  {
    if(Selection){
      // calculate  endpos
      TEditRange range(Selection->GetStart(), Selection->GetEnd());
      range.Normalize();

      CursorPos = range.Start();
      Selection->Delete();
      ClearSelection();
      AdjustScroller();
      ScrollToCaret(CursorPos);
      InvalidateLines(0, -1, false);
      repeatCount--;
    }
    return repeatCount;
  }
  //
  void
    TCoolEdit::InsertChar(uint key, uint repeatCount)
  {
    if(IsReadOnly())
      return;

    PRECONDITION(GetBuffer());
    TCoolTextBuffer& buffer = *GetBuffer();

    _TCHAR text[2] = {static_cast<_TCHAR>(key), 0};

    TUndoDeleteSelection* undoSel = 0;
    if(Selection)
      undoSel = new TUndoDeleteSelection(*this, buffer.IsDirty());
    DeleteSelection(repeatCount);

    TEditPos endPos(CursorPos);

    while(repeatCount--){
      buffer.AddUndoNode(new TUndoInsertChar(key, endPos.row, endPos.col,
        undoSel, IsFlagSet(cfOverType), buffer.IsDirty()));
      if(IsFlagSet(cfOverType)){
        if(buffer.GetLineLength(endPos.row) > endPos.col)
          buffer.GetLineText(endPos.row)[endPos.col++] = key;
        else
          buffer.InsertText(endPos, text, endPos);
      }
      else
        buffer.InsertText(endPos, text, endPos);
      undoSel = 0;
    }

    InvalidateLines(CursorPos.row, endPos.row);
    CursorPos = endPos;
    ScrollToCaret(CursorPos);
  }
  //
  void
    TCoolEdit::KeyEnter(uint repeatCount)
  {
    if(IsReadOnly())
      return;

    PRECONDITION(GetBuffer());
    TCoolTextBuffer& buffer = *GetBuffer();

    TUndoDeleteSelection* undoSel = 0;
    if(Selection && !IsFlagSet(cfOverType)){
      undoSel = new TUndoDeleteSelection(*this, buffer.IsDirty());
      DeleteSelection(repeatCount);
    }

    TEditPos endPos(CursorPos);
    while(repeatCount--){
      if(!IsFlagSet(cfOverType)){
        TUndoKeyEnter* node = new TUndoKeyEnter(endPos.row, endPos.col,
          undoSel, buffer.IsDirty());
        buffer.AddUndoNode(node);
        buffer.InsertText(endPos, crlf, endPos);

        endPos.col = GetSmartPosition(endPos);
        TEditPos pos(0, endPos.row);
        for(int i = 0; i < endPos.col; i++){
          buffer.InsertText(pos, _T(" "), pos);
        }
        node->SetNewPos(endPos.col);
      }
      else{
        endPos.row++;
        endPos.col = GetSmartPosition(endPos);
      }
      undoSel = 0;
    }
    CursorPos = endPos;
    AdjustScroller();
    ScrollToCaret(CursorPos);
    InvalidateLines(0, -1, false);
  }
  //
  void
    TCoolEdit::KeyTab(uint repeatCount)
  {
    if(IsReadOnly())
      return;
    // do nothing if selection and overtype
    if(Selection && IsFlagSet(cfOverType))
      return;

    PRECONDITION(GetBuffer());
    TCoolTextBuffer& buffer = *GetBuffer();
    int startLine=0, endLine=0; InUse(endLine);
    if(Selection){
      // adjust selection
      bool fixedCursor = false;
      TEditPos startSel(Selection->GetStart());
      TEditPos endSel(Selection->GetEnd());
      if(Selection->GetType() == stStreamSelection){
        // in case current SelType changed
        TSelType saveType = SelType;
        SelType = stStreamSelection;
        startSel.col = 0;
        endSel.col = endSel.col > 0 ? buffer.GetLineLength(endSel.row) : 0;
        SetSelection(TEditRange(startSel, endSel));
        SelType = saveType;
        if(endSel.col > 0)
          CursorPos.row = endSel.row+1;
        else{
          CursorPos.row = endSel.row;
          endSel.row--;
        }
        CursorPos.col = 0;
        fixedCursor = true;
        startLine = startSel.row;
      }
      int endChar = CursorPos.col;
      while(repeatCount--){
        buffer.AddUndoNode(new TUndoKeyTabify(endChar, startSel.row, CursorPos.row,
          buffer.IsDirty()));
        // shift all characters right
        TEditPos deltaPos;//deltaChar;
        for(deltaPos.row = startSel.row; deltaPos.row <= endSel.row; deltaPos.row++){
          deltaPos.col = endChar;
          buffer.InsertText(deltaPos, szTab, deltaPos);
        }
        endChar = deltaPos.col;
      }
      //CursorPos.row = endLine;
      if(!fixedCursor)
        CursorPos.col = endChar;
      endLine = endSel.row;
    }
    else{
      int tabSize = GetTabSize();
      int lineLength  = buffer.GetLineLength(CursorPos.row);
      LPCTSTR text    = buffer.GetLineText(CursorPos.row);
      startLine = CursorPos.row;
      while(repeatCount--){
        if(CursorPos.col>=lineLength || !IsFlagSet(cfOverType)){
          buffer.AddUndoNode(new TUndoKeyTab(*this, CursorPos, 1, true,
            buffer.IsDirty()));
          buffer.InsertText(CursorPos, szTab, CursorPos);
        }
        else{
          if(text[CursorPos.col] == chTab){
            buffer.AddUndoNode(new TUndoKeyTab(*this, CursorPos, 1, false,
              buffer.IsDirty()));
            CursorPos.col++;
          }
          else{
            buffer.AddUndoNode(new TUndoKeyTab(*this, CursorPos, tabSize, false,
              buffer.IsDirty()));
            CursorPos.col += tabSize;
          }
        }
      }
      endLine = CursorPos.row;
    }

    ScrollToCaret(CursorPos);
    InvalidateLines(startLine, endLine);
  }
  //
  void
    TCoolEdit::KeyUnTab(uint /*repeatCount*/)
  {
    ClearSelection();

    if(IsReadOnly() || CursorPos.col == 0)
      return;

    PRECONDITION(GetBuffer());

    TCoolTextBuffer& buffer = *GetBuffer();
    LPCTSTR text= buffer.GetLineText(CursorPos.row);
    int length  = buffer.GetLineLength(CursorPos.row);
    if(CursorPos.col < length && text[CursorPos.col-1] == chTab)
      CursorPos.col--;
    else
      CursorPos.col -= GetTabSize();

    ScrollToCaret(CursorPos);
    InvalidateLines(CursorPos.row, CursorPos.row);
  }
  //
  void
    TCoolEdit::DeleteWordBack(uint repeatCount)
  {
    if(IsReadOnly())
      return;

    PRECONDITION(GetBuffer());

    TCoolTextBuffer& buffer = *GetBuffer();

    int startLine = CursorPos.row;
    TUndoDeleteSelection* undoSel = 0;

    if(Selection)
      undoSel = new TUndoDeleteSelection(*this, buffer.IsDirty());

    DeleteSelection(repeatCount);

    if(CursorPos.row==0 && CursorPos.col==0){
      if(undoSel)
        buffer.AddUndoNode(undoSel);
      return;
    }

    while(repeatCount--){
      TEditPos savedPos = CursorPos;
      CursorPos.col--;
      if(CursorPos.col < 0){
        if(CursorPos.row==0){
          CursorPos.col = 0;
          break;
        }
        CursorPos.row--;
        CursorPos.col = buffer.GetLineLength(CursorPos.row);
      }
      TEditPos startSel = StartWord(CursorPos);
      buffer.AddUndoNode(new TUndoDeleteWordBack(buffer, savedPos, startSel, undoSel));
      undoSel = 0;
      buffer.DeleteText(TEditRange(startSel, savedPos));
      CursorPos = startSel;
    }

    AdjustScroller();
    ScrollToCaret(CursorPos);
    InvalidateLines(CursorPos.row, startLine);
  }
  //
  void
    TCoolEdit::DeleteCharBack(uint repeatCount)
  {
    if(IsReadOnly())
      return;

    PRECONDITION(GetBuffer());

    TCoolTextBuffer& buffer = *GetBuffer();

    TUndoDeleteSelection* undoSel = 0;
    if(Selection)
      undoSel = new TUndoDeleteSelection(*this, buffer.IsDirty());
    repeatCount = DeleteSelection(repeatCount);
    if(!repeatCount){
      if(undoSel)
        buffer.AddUndoNode(undoSel);
      return;
    }

    TEditPos curPos(CursorPos);
    while(repeatCount--){
      if(curPos.col==0){
        if(curPos.row==0)
          break;

        buffer.AddUndoNode(new TUndoDeleteCharBack(buffer,
          curPos.row, curPos.col, undoSel));
        undoSel = 0;
        //insert next line
        int len = buffer.GetLineLength(curPos.row);
        TTmpBuffer<_TCHAR> __tmpObj(len+1);
        memcpy((_TCHAR*)__tmpObj,buffer.GetLineText(curPos.row),len);
        ((_TCHAR*)__tmpObj)[len] = 0;

        curPos.row--;
        curPos.col = buffer.GetLineLength(curPos.row);
        TEditPos endPos;
        buffer.InsertText(curPos, __tmpObj, endPos);
        // delete entare line
        TEditRange range(0, curPos.row+1, -1, curPos.row+1);
        buffer.DeleteText(range);
        InvalidateLines(0, -1);
        continue;
      }
      buffer.AddUndoNode(new TUndoDeleteCharBack(buffer,
        curPos.row, curPos.col, undoSel));
      undoSel = 0;
      TEditRange range(curPos.col-1,curPos.row, curPos.col, curPos.row);
      buffer.DeleteText(range);
      curPos.col--;
    }
    InvalidateLines(curPos.row, CursorPos.row);
    CursorPos  = curPos;
    AdjustScroller();
    ScrollToCaret(CursorPos);
  }
  //
  void
    TCoolEdit::DeleteWord(uint repeatCount)
  {
    if(IsReadOnly())
      return;
    PRECONDITION(GetBuffer());
    TCoolTextBuffer& buffer = *GetBuffer();

    TUndoDeleteSelection* undoSel = 0;
    if(Selection)
      undoSel = new TUndoDeleteSelection(*this, buffer.IsDirty());

    DeleteSelection(repeatCount);

    while(repeatCount--){
      if(CursorPos.col == buffer.GetLineLength(CursorPos.row))
        break;

      LPCTSTR p = buffer.GetLineText(CursorPos.row);
      TEditPos endpos(CursorPos);
      if(isWordChar(p[endpos.col]))
        endpos = EndWord(endpos);
      else{
        while(!isWordChar(p[endpos.col]))
          endpos.col++;
      }

      buffer.AddUndoNode(new TUndoDeleteWord(buffer, CursorPos, endpos, undoSel));
      undoSel = 0;
      buffer.DeleteText(TEditRange(CursorPos, endpos));
      //CursorPos.col = endpos.col;
    }
    if(undoSel)
      buffer.AddUndoNode(undoSel);

    ScrollToCaret(CursorPos);
    InvalidateLines(CursorPos.row, CursorPos.row);
  }
  //
  void
    TCoolEdit::DeleteChar(uint repeatCount)
  {
    if(IsReadOnly())
      return;

    PRECONDITION(GetBuffer());
    TCoolTextBuffer& buffer = *GetBuffer();

    TUndoDeleteSelection* undoSel = 0;
    if(Selection)
      undoSel = new TUndoDeleteSelection(*this, buffer.IsDirty());
    repeatCount = DeleteSelection(repeatCount);
    if(!repeatCount){
      if(undoSel)
        buffer.AddUndoNode(undoSel);
      return;
    }

    TEditPos curPos(CursorPos);
    while(repeatCount--){
      if(curPos.col == buffer.GetLineLength(curPos.row)){
        if(curPos.row >= buffer.GetLineCount()-1)
          break;
        buffer.AddUndoNode(new TUndoDeleteChar(buffer, curPos.row, curPos.col, undoSel));
        undoSel = 0;
        //insert next line
        int len = buffer.GetLineLength(curPos.row+1);
        TTmpBuffer<_TCHAR> __tmpObj(len+1);

        memcpy((_TCHAR*)__tmpObj,buffer.GetLineText(curPos.row+1),len);
        ((_TCHAR*)__tmpObj)[len] = 0;

        TEditPos endPos;
        buffer.InsertText(curPos, __tmpObj, endPos);

        // delete entare line
        buffer.DeleteText(TEditRange(0,curPos.row+1, -1, curPos.row+1));
        InvalidateLines(0, -1);
        continue;
      }
      buffer.AddUndoNode(new TUndoDeleteChar(buffer, curPos.row, curPos.col, undoSel));
      undoSel = 0;
      buffer.DeleteText(TEditRange(curPos.col, curPos.row, curPos.col+1, curPos.row));
    }
    if(curPos.row != CursorPos.row)
      InvalidateLines(0, -1);
    else
      InvalidateLines(curPos.row, CursorPos.row);
    AdjustScroller();
  }
  //
  void
    TCoolEdit::CmEditReplace()
  {
    if(IsReadOnly())
      return;
    if (!SearchCmd) {
      SearchCmd = CM_EDITREPLACE;
      delete SearchDialog;
      SearchDialog = new TCoolReplaceDlg(this, SearchData);
      SetFlag(cfShowInactiveSel);
      SearchDialog->Create();
      SearchDialog->ShowWindow(SW_SHOWDEFAULT);
    }
  }
  //
  void
    TCoolEdit::CeEditFindReplace(TCommandEnabler& ce)
  {
    ce.Enable(!SearchCmd && !IsReadOnly());
  }
  //
  void
    TCoolEdit::Delete()
  {
    if(IsReadOnly())
      return;

    PRECONDITION(GetBuffer());
    TCoolTextBuffer& buffer = *GetBuffer();

    if(Selection)
      buffer.AddUndoNode(new TUndoDeleteSelection(*this, buffer.IsDirty()));
    DeleteSelection(1);
    //AdjustScroller();
    //Invalidate();
  }
  //
  void
    TCoolEdit::CmEditDelete()
  {
    Delete();
  }
  //
  static _TCHAR*
    FindEndLine(_TCHAR* text, int& size)
  {
    int textPos = 0;
    while(text[textPos] != 0 && text[textPos] != _T('\r'))
      textPos++;
    if(text[textPos]){
      if(text[textPos] == _T('\n'))
        textPos++;
      size -= textPos;
    }
    return &text[textPos];
  }
  //
  static void
    PasteFromMemoryImp(_TCHAR* pMemory, int size, TCoolEdit::TSelType selection,
    TEditPos& cursorPos, TCoolTextBuffer& buffer)
  {
    switch(selection){
  case TCoolEdit::stStreamSelection:
  case TCoolEdit::stLineSelection:{
    TEditPos curPos(cursorPos);
    buffer.InsertText(curPos, pMemory, curPos);
    cursorPos = curPos;
                  }
                  break;
  case TCoolEdit::stColumnSelection:{
    TEditPos curPos(cursorPos);
    _TCHAR* text = pMemory;
    while(size){
      _TCHAR* endTxt = FindEndLine(pMemory, size);
      if(endTxt){
        _TCHAR c = *endTxt;
        *endTxt = 0;
        buffer.InsertText(curPos, text, curPos);
        *endTxt = c;
        text = endTxt + 1; // next
        size--;
      }
      else{
        _TCHAR c = text[size-1];
        text[size-1] = 0;
        buffer.InsertText(curPos, text, curPos);
        text[size-1] = c;
        break;
      }
    }
    cursorPos = curPos;
                    }
                    break;
    }
  }
  //
  void
    TCoolEdit::PasteFromMemory(_TCHAR* pMemory, int size, TSelType selection)
  {

    PasteFromMemoryImp(pMemory, size, selection, CursorPos, *GetBuffer());
    AdjustScroller();
    ScrollToCaret(CursorPos);
    InvalidateLines(0, -1);
  }
  //
  bool
    TCoolEdit::Paste()
  {
    if(IsReadOnly())
      return false;

    TCoolTextBuffer& buffer = *GetBuffer();

    TUndoDeleteSelection* undoSel = 0;
    if(Selection)
      undoSel = new TUndoDeleteSelection(*this, buffer.IsDirty());
    DeleteSelection(1);

    TEditPos savedPos = CursorPos;
    bool savedDirty = buffer.IsDirty();

    TClipboard clipboard(*this);
    if (!clipboard)
      return false;
    HGLOBAL hGlobal = 0;
    _TCHAR* pMemory = 0;
    TSelType selection = stStreamSelection;
    if(clipboard.IsClipboardFormatAvailable(SelClipFormat)){
      hGlobal = clipboard.GetClipboardData(SelClipFormat);
      if(hGlobal)
        pMemory = (_TCHAR*)::GlobalLock(hGlobal);
      if(pMemory){
        if(memcmp(pMemory,streamSelectionId,IdSelSize)==0)
          selection = stStreamSelection;
        else if(memcmp(pMemory,lineSelectionId,IdSelSize)==0)
          selection = stLineSelection;
        else if(memcmp(pMemory,lineSelectionId,IdSelSize)==0)
          selection = stColumnSelection;
        else{
          pMemory = 0;
        }
        if(pMemory)
          pMemory += IdSelSize;
      }
    }
#ifdef _UNICODE
    else if(clipboard.IsClipboardFormatAvailable(CF_UNICODETEXT)){
      hGlobal = clipboard.GetClipboardData(CF_UNICODETEXT);
#else
    else if(clipboard.IsClipboardFormatAvailable(CF_TEXT)){
      hGlobal = clipboard.GetClipboardData(CF_TEXT);
#endif
      if(hGlobal)
        pMemory = (_TCHAR*)::GlobalLock(hGlobal);
    }
    if(pMemory){
      PasteFromMemory(pMemory, static_cast<int>(_tcslen(pMemory)), selection);
      buffer.AddUndoNode(new TUndoPaste(savedPos, CursorPos, undoSel, savedDirty));
    }
    if(hGlobal)
      ::GlobalUnlock(hGlobal);
    return pMemory!=0;
  }
  //
  void
    TCoolEdit::ClearAll()
  {
    if(IsReadOnly())
      return;
    PRECONDITION(GetBuffer());
    TCoolTextBuffer& buffer = *GetBuffer();
    TUndoClearAll* node = new TUndoClearAll(*this, CursorPos, buffer.IsDirty());
    buffer.Clear();
    buffer.SetDirty(true);
    buffer.AddUndoNode(node);
    CursorPos.col = 0;
    CursorPos.row = 0;

    AdjustScroller();
    ScrollToCaret(CursorPos);
    InvalidateLines(0, -1);
  }
  //
  void
    TCoolEdit::Cut()
  {
    if(IsReadOnly() || !IsSelection())
      return;

    PRECONDITION(GetBuffer());
    TCoolTextBuffer& buffer = *GetBuffer();
    buffer.AddUndoNode(new TUndoDeleteSelection(*this, buffer.IsDirty()));

    CopySelection();
    DeleteSelection(1);
    AdjustScroller();
    InvalidateLines(0, -1);
  }
  //
  void
    TCoolEdit::CePasteEnable(TCommandEnabler& tce)
  {
    TClipboard cb(*this, false);
    tce.Enable(!IsReadOnly() && (cb.IsClipboardFormatAvailable(SelClipFormat) ||
#ifdef _UNICODE
      cb.IsClipboardFormatAvailable(CF_UNICODETEXT)
#else
      cb.IsClipboardFormatAvailable(CF_TEXT)
#endif
      ));
  }
  //
  void
    TCoolEdit::CeCharsEnable(TCommandEnabler& tce)
  {
    tce.Enable((GetNumLines()>1 || GetLineLength(0)>0) && !IsReadOnly());
  }
  //
  void
    TCoolEdit::CmEditUndo()
  {
    PRECONDITION(GetBuffer());
    TEditPos pos;
    TCoolTextBuffer* buffer = GetBuffer();
    if(buffer->Undo(&pos)){
      InvalidateLines(0, -1); // Y.B. Can we query invalid region???
      CursorPos = pos;
      AdjustScroller();
      ScrollToCaret(CursorPos);
    }
  }
  //
  void
    TCoolEdit::CmEditRedo()
  {
    PRECONDITION(GetBuffer());
    GetBuffer()->Redo();
  }
  //
  void
    TCoolEdit::CeUndoEnable(TCommandEnabler& tce)
  {
    PRECONDITION(GetBuffer());
    tce.Enable(GetBuffer()->CanUndo());
    //can also set text
    //TCoolTextBuffer::TUndoNode* node = GetBuffer()->GetTopUndoNode();
    //if(node){
    //  _TCHAR buffer[256];
    //  if(node->GetDescription(buffer,256, GetApplication()))
    //    tce.SetTex(buffer)
    //}
  }
  //
  void
    TCoolEdit::CeRedoEnable(TCommandEnabler& tce)
  {
    PRECONDITION(GetBuffer());
    tce.Enable(GetBuffer()->CanRedo());
    //can also set text
    //TCoolTextBuffer::TRedoNode* node = GetBuffer()->GetTopRedoNode();
    //if(node){
    //  _TCHAR buffer[256];
    //  if(node->GetDescription(buffer,256,GetApplication()))
    //    tce.SetTex(buffer)
    //}
  }
  //
  // Smart AutoIntend
  //
  int
    TCoolEdit::GetSmartPosition(const TEditPos& pos)
  {
    int index = 0;
    if(IsAutoIndent() && !IsFlagSet(cfOverType)){
      //  Take indentation from the previos line
      TCoolTextBuffer* buffer = GetBuffer();
      int len = buffer->GetLineLength(pos.row - 1);
      LPCTSTR text = buffer->GetLineText(pos.row - 1);
      while(index < len && _istspace(text[index])){
        if(text[index] == chTab)
          index += GetTabSize();
        else
          index++;
      }
    }
    return index;
  }

  //----------------------------------------------------------------------------
  // TCoolEditFile Implementation
  //
  DEFINE_RESPONSE_TABLE1(TCoolEditFile, TCoolEdit)
    // !!! BUG  EV_COMMAND(CM_FILENEW,  CmFileNew),  // BUG !!!!!!!
    // !!! BUG  EV_COMMAND(CM_FILEOPEN, CmFileOpen), // BUG !!!!!!!
    EV_COMMAND(CM_FILESAVE, CmFileSave),
    EV_COMMAND(CM_FILESAVEAS, CmFileSaveAs),
    EV_COMMAND_ENABLE(CM_FILESAVE, CmSaveEnable),
    END_RESPONSE_TABLE;
  //
  TCoolEditFile::TCoolEditFile(TWindow* parent, int id, LPCTSTR title, int x,
    int y, int w, int h,  LPCTSTR fileName,
    TModule* module)
    :
  TCoolEdit(parent, id, title, x, y, w, h, module)
  {
    FileName = fileName ? strnewdup(fileName) : 0;
  }
  //
  TCoolEditFile::~TCoolEditFile()
  {
    delete[] FileName;
  }
  //
  // performs setup for a TEditFile
  //
  void
    TCoolEditFile::SetupWindow()
  {
    TCoolEdit::SetupWindow();
    FileData.Flags = OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT|OFN_EXPLORER|OFN_ENABLESIZING;
    FileData.SetFilter(GetModule()->LoadString(IDS_FILEFILTER).c_str());

    SetFileName(FileName);
    if (FileName && !TCoolEdit::FileOpen(FileName)){
      tstring msgTemplate(GetModule()->LoadString(IDS_UNABLEREAD));
      LPTSTR msg = new _TCHAR[_MAX_PATH + msgTemplate.length()];
      _stprintf(msg, msgTemplate.c_str(), FileName);
      MessageBox(msg, 0, MB_ICONEXCLAMATION | MB_OK);
      delete[] msg;
      SetFileName(0);
    }
  }
  //
  // sets the file name of the window and updates the caption
  // replacing an empty name with 'Untitled' in its caption
  //
  void
    TCoolEditFile::SetFileName(LPCTSTR fileName)
  {
    if (fileName != FileName) {
      delete[] FileName;
      FileName = fileName ? strnewdup(fileName) : 0;
    }
    tstring untitled(GetModule()->LoadString(IDS_UNTITLEDFILE));
    SetDocTitle(FileName ? (LPCTSTR)FileName : untitled.c_str(), 0);
  }
  //
  void
    TCoolEditFile::FileNew()
  {
    if(CanClear()){
      TCoolEdit::FileNew();
      SetFileName(0);
    }
  }
  //
  bool
    TCoolEditFile::FileOpen(LPCTSTR filename)
  {
    if (filename==0 && CanClear()){
      *FileData.FileName = 0;
      if (TFileOpenDialog(this, FileData).Execute() == IDOK){
        if(TCoolEdit::FileOpen(FileData.FileName))
          SetFileName(FileData.FileName);
        else{
          tstring msgTemplate(GetModule()->LoadString(IDS_UNABLEREAD));
          LPTSTR  msg = new TCHAR[_MAX_PATH + msgTemplate.length()];
          _stprintf(msg, msgTemplate.c_str(), FileData.FileName);
          MessageBox(msg, 0, MB_ICONEXCLAMATION | MB_OK);
          delete[] msg;
          return false;
        }
      }
    }
    else {
      if(TCoolEdit::FileOpen(filename)){
        SetFileName(filename);
        return true;
      }
      else{
        tstring msgTemplate(GetModule()->LoadString(IDS_UNABLEREAD));
        LPTSTR  msg = new TCHAR[_MAX_PATH + msgTemplate.length()];
        _stprintf(msg, msgTemplate.c_str(), filename);
        MessageBox(msg, 0, MB_ICONEXCLAMATION | MB_OK);
        delete[] msg;
        return false;
      }
    }
    return false;
  }
  //
  bool
    TCoolEditFile::FileSave()
  {
    if (GetBuffer()->IsDirty()) {
      if(!FileName)
        return FileSaveAs();
      if (!GetBuffer()->Save(FileName)) {
        tstring msgTemplate(GetModule()->LoadString(IDS_UNABLEWRITE));
        LPTSTR msg = new TCHAR[_MAX_PATH + msgTemplate.length()];
        _stprintf(msg, msgTemplate.c_str(), FileName);
        MessageBox(msg, 0, MB_ICONEXCLAMATION | MB_OK);
        delete[] msg;
        return false;
      }
    }
    return true;
  }
  //
  bool
    TCoolEditFile::FileSaveAs()
  {
    if (FileName)
      _tcscpy(FileData.FileName, FileName);
    else
      *FileData.FileName = 0;

    if (TFileSaveDialog(this, FileData).Execute() == IDOK) {
      if(GetBuffer()->Save(FileData.FileName)){
        SetFileName(FileData.FileName);
        return true;
      }
      tstring msgTemplate(GetModule()->LoadString(IDS_UNABLEWRITE));
      LPTSTR  msg = new TCHAR[_MAX_PATH + msgTemplate.length()];
      _stprintf(msg, msgTemplate.c_str(), FileName);
      MessageBox(msg, 0, MB_ICONEXCLAMATION | MB_OK);
      delete[] msg;
    }
    return false;
  }
  //
  void
    TCoolEditFile::CmSaveEnable(TCommandEnabler& tce)
  {
    tce.Enable(GetBuffer()->IsDirty());
  }
  //
  // returns a bool value indicating whether or not it is Ok to clear
  // the TEdit's text
  //
  // returns true if the text has not been changed, or if the user Oks the
  // clearing of the text
  //
  bool
    TCoolEditFile::CanClear()
  {
    if (GetBuffer()->IsDirty()) {
      tstring msgTemplate(GetModule()->LoadString(IDS_FILECHANGED));
      tstring untitled(GetModule()->LoadString(IDS_UNTITLEDFILE));
      LPTSTR  msg = new TCHAR[_MAX_PATH+msgTemplate.length()];

      _stprintf(msg, msgTemplate.c_str(),
        FileName ? (LPCTSTR)FileName : untitled.c_str());

      int result = MessageBox(msg, 0, MB_YESNOCANCEL|MB_ICONQUESTION);
      delete[] msg;
      return result==IDYES ? FileSave() : result != IDCANCEL;
    }
    return true;
  }
  //
  bool
    TCoolEditFile::CanClose()
  {
    return TCoolEdit::CanClose();
  }
  //----------------------------------------------------------------------------
  // TCoolSearchView Implementation
  //

  //
  // Build a response table for all messages/commands handled
  // by TCoolSearchView derived from TCoolSearchWnd.
  //
  DEFINE_RESPONSE_TABLE1(TCoolSearchView, TCoolSearchWnd)
    EV_VN_ISWINDOW,
    END_RESPONSE_TABLE;
  //
  TCoolSearchView::TCoolSearchView(TDocument& doc, TWindow* parent)
    :
  TCoolSearchWnd(parent, 0, _T(""), 0, 0, 0, 0, 0),
    TView(doc)
  {
  }
  //
  //----------------------------------------------------------------------------
  // TCoolEditView Implementation
  //
  //
  // Build a response table for all messages/commands handled
  // by TCoolEditView derived from TCoolEdit.
  //
  DEFINE_RESPONSE_TABLE1(TCoolEditView, TCoolEdit)
    EV_VN_ISWINDOW,
    END_RESPONSE_TABLE;
  //
  TCoolEditView::TCoolEditView(TDocument& doc, TWindow* parent)
    :
  TCoolEdit(parent, 0, _T(""), 0, 0, 0, 0, 0),
    TView(doc)
  {
  }
  //
  /////////////////////////////////////////////////////////////////////////////////////////
  //
