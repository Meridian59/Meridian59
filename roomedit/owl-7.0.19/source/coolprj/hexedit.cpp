//----------------------------------------------------------------------------
// ObjectWindow - OWL NExt
// Copyright 1999. Yura Bidus. All Rights reserved.
//
// For more information, including license details, see
// http://owlnext.sourceforge.net
//
//  OVERVIEW
//  ~~~~~~~~
//  Syntax coloring Hex editor.
//----------------------------------------------------------------------------

#include <coolprj/pch.h>
#pragma hdrstop

#include <owl/file.h>
#include <owl/filename.h>
#include <owl/configfl.h>
#include <owl/scroller.h>
#include <owl/uimetric.h>

#include <sstream>
#include <algorithm>

using namespace owl;
using namespace std;

#include <coolprj/hexedit.h>

//#define USE_AUTOORG

#define HEX_TIMER_DEBOUNCE      1002
#ifdef WIN32
const int MSB=0x80000000;
#else
const int MSB=0x8000;
#endif

static const _TCHAR*  configSection     = _T("HexSyntaxFormat");
static const _TCHAR*  configBaseFont    = _T("BaseFont");
static const _TCHAR*  configFormatIdx   = _T("FormatIdx%d");
static const _TCHAR*  configOWLNExt     = _T("OWL NExt");

_TCHAR hextable[16] = {_T ('0'), _T ('1'), _T ('2'), _T ('3'), _T ('4'), _T ('5'), _T ('6'), _T ('7'),
                     _T ('8'), _T ('9'), _T ('A'), _T ('B'), _T ('C'), _T ('D'), _T ('E'), _T ('F')};
#define TOHEX(a, b) {*b++ = hextable[a >> 4];*b++ = hextable[a&0x0f];}

static const char*  streamSelectionId = "HEX000";
static const char*  lineSelectionId   = "HEX001";


void __Trace(LPCTSTR lpszFormat, ...);
//------------------------------------------------------------------------------
inline void __NoTrace(LPCTSTR /*lpszFormat*/, ...){}
//------------------------------------------------------------------------------

#if 0
#define _TRACE __Trace
#else
#define _TRACE __NoTrace
#endif

THexBuffer::THexBuffer()
:
  SyntaxArray(0)
  ,Flags(bfCreateBackup)
{
  memset(&BaseFont, 0, sizeof(BaseFont));
  _tcscpy(BaseFont.lfFaceName, _T("Courier New"));
  BaseFont.lfHeight         = -13;
  BaseFont.lfWeight         = FW_NORMAL;
  BaseFont.lfItalic         = FALSE;
  BaseFont.lfCharSet        = DEFAULT_CHARSET;
  BaseFont.lfOutPrecision   = OUT_DEFAULT_PRECIS;
  BaseFont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
  BaseFont.lfQuality        = DEFAULT_QUALITY;
  BaseFont.lfPitchAndFamily = FIXED_PITCH;
}
//-----------------------------------------------------------------------------
THexBuffer::~THexBuffer()
{
  delete SyntaxArray;
}
//-----------------------------------------------------------------------------
static void
SetSyntaxNode(TCoolTextBuffer::TSyntaxDescr& node, TCoolTextBuffer::TSyntaxDescr* descr)
{
  // white Space
  if(descr){
    node.TxColor    = descr->TxColor;
    node.BkColor    = descr->BkColor;
    node.FontIndex  = descr->FontIndex;
  }
}
//-----------------------------------------------------------------------------
void THexBuffer::SetSyntaxDescr(int index, const TSyntaxDescr& node)
{
  if(!SyntaxArray)
    SyntaxArray = new TSyntaxDescr[COLORINDEX_LAST];
  SyntaxArray[index] = node;
}
//-----------------------------------------------------------------------------
THexBuffer::TSyntaxDescr* THexBuffer::GetDefSyntaxDescr(int index)
{
  static TSyntaxDescr array[] = {
    TSyntaxDescr(TColor::SysWindowText,       TColor::SysWindow,         0), //HEXINDEX_WHITESPACE
    // Address
    TSyntaxDescr(TColor::SysWindowText,       TColor::SysWindow,         0), //HEXINDEX_ADDRESS
    // Digits part
    TSyntaxDescr(TColor::SysWindowText,       TColor::SysWindow,         0), //HEXINDEX_DIGITS
    // ASCII part
    TSyntaxDescr(TColor::SysWindowText,       TColor::SysWindow,         0), //HEXINDEX_ASCII
    // Selected Text
    TSyntaxDescr(TColor::White,               TColor::Black,             0), //HEXINDEX_SELECTION
  };
  return index < static_cast<int>(COUNTOF(array)) ? &array[index] : 0;
}
//-----------------------------------------------------------------------------
void THexBuffer::BuildDefSyntaxDescr()
{
  if(!SyntaxArray)
    SyntaxArray = new TSyntaxDescr[HEXINDEX_LAST];

  for(int i = 0; i < HEXINDEX_LAST; i++)
    SetSyntaxNode(SyntaxArray[i], GetDefSyntaxDescr(i));
}
//-----------------------------------------------------------------------------
void THexBuffer::SetSyntaxArray(TSyntaxDescr* array)
{
  PRECONDITION(array);
  delete SyntaxArray;
  SyntaxArray = array;
}
//-----------------------------------------------------------------------------
void THexBuffer::SaveSyntaxDescr(TConfigFile& file)
{
  PRECONDITION(SyntaxDescrExist());

  TConfigFileSection section(file, configSection);
  // 1. Save Font Info
  section.WriteFont(configBaseFont, BaseFont);

  // 2. Write color data + special flag, so WriteColor not applicable
  // maybe better WriteInteger???
  _TCHAR buffer[40];
  for(int i = 0; i < HEXINDEX_LAST; i++){
    wsprintf(buffer, configFormatIdx, i);
    section.WriteData(buffer, (void*)&(SyntaxArray[i]), sizeof(SyntaxArray[i]));
  }
}
//-----------------------------------------------------------------------------
void THexBuffer::RestoreSyntaxDescr(TConfigFile& file)
{
  if(!SyntaxArray)
    SyntaxArray = new TSyntaxDescr[HEXINDEX_LAST];
  if(!file.SectionExists(configSection))
    BuildDefSyntaxDescr();
  else{

    TConfigFileSection section(file, configSection);
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
void THexBuffer::GetFont(LOGFONT& lf) const
{
  lf = BaseFont;
}
//-----------------------------------------------------------------------------
void THexBuffer::SetFont(const LOGFONT& lf)
{
  BaseFont = lf;
}
//-----------------------------------------------------------------------------
uint32 THexBuffer::Search(const TEditPos& /*searchRange_*/, uint8* /*text*/,
                          uint /*textLen*/, bool /*up*/)
{
/*
  if (!text || !text[0] || !textLen)
    return uint32(-1);

  TEditPos searchRange(searchRange_);
  if(!searchRange.Valid()){
    // if not valid -> all file
    searchRange.col = 0;
    searchRange.row = GetDataSize();
  }

  // Lock the text buffer to get the pointer, and search thru it for the text
  //
  uint8* pos;
  uint8* buffer = up ? GetData() + searchRange.row-1 : GetData()+searchRange.col;
  uint bufLen    = searchRange.row-searchRange.col;

  if(up)
    pos = binrstrcd(buffer, bufLen, text, textLen);
  else
    pos = binstrcd(buffer, bufLen, text, textLen);

  // If we've got a match, select that text, cleanup & return.
  //
  if(pos)
    return pos - GetData();
*/
  return uint32(-1);
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
THexFileBuffer::THexFileBuffer()
:
  THexBuffer()
  ,BinaryData(0)
  ,DataSize(0)
{
}
//-----------------------------------------------------------------------------
THexFileBuffer::~THexFileBuffer()
{
  delete BinaryData;
}
//-----------------------------------------------------------------------------
bool THexFileBuffer::Load(LPCTSTR filename)
{
  TFile file(filename);
  if(file.IsOpen())
    return Load(file);
  return false;
}
//-----------------------------------------------------------------------------
bool THexFileBuffer::Save(LPCTSTR filename, bool clearDirty)
{
  // save to tmp file and create backup
  TFileName savename(filename);
  TFileName tmpname(TFileName::TempFile);

  TFile file;
  if(file.Open(tmpname.Canonical().c_str(), TFile::WriteOnly|
               TFile::PermExclusive|TFile::CreateAlways)){
    if(Save(file, clearDirty)){
      if((Flags&bfCreateBackup) && savename.Exists()){
        TFileName bakname(savename);
        owl::tstring backupExt = bakname.GetParts(TFileName::Ext);
        backupExt[1] = _T('~');
        bakname.SetPart(TFileName::Ext, backupExt);
        savename.Move(bakname, TFileName::ReplaceExisting|
                      TFileName::CopyAllowed);
      }
      tmpname.Move(savename, TFileName::ReplaceExisting|TFileName::CopyAllowed);
      file.Close();
      return true;
    }
    file.Close();
  }
  return false;
}
//-----------------------------------------------------------------------------
bool THexFileBuffer::Load(TFile& file)
{
  TFileStatus status;
  if(!file.GetStatus(status))
    return false;
  if(status.size==0)
    return true;
  TAPointer<uint8> buffer(new uint8[status.size]);
  uint32 size = file.Read((uint8*) buffer, status.size);
  if(size == TFILE_ERROR)
    return false;

  delete BinaryData;
  BinaryData = buffer.Relinquish();
  DataSize = size;
  EnableReadOnly((status.attribute&TFile::RdOnly) !=0);
  SetDirty(false);

  return true;
}
//-----------------------------------------------------------------------------
bool THexFileBuffer::Save(TFile& file, bool clearDirty)
{
  if(!file.Write(BinaryData, DataSize))
    return false;
  if(clearDirty)
    SetDirty(false);
  return true;
}
//-----------------------------------------------------------------------------
void THexFileBuffer::Clear()
{
  delete BinaryData;
  BinaryData = 0;
  DataSize   = 0;
  SetDirty(false);
}
//-----------------------------------------------------------------------------
uint THexFileBuffer::GetBuffer(uint index, uint8* buffer, uint size)
{
  if(index >= DataSize)
    return 0;
  uint count = std::min(size, DataSize-index);
  memcpy(buffer,BinaryData+index,count);
  return count;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//{{THexViewWnd Implementation}}

UINT THexViewWnd::SelClipFormat = 0;

//
// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(THexViewWnd, TControl)
  EV_WM_SIZE,
  EV_WM_SETCURSOR,
  EV_COMMAND(CM_EDITCOPY, CmEditCopy),
  EV_COMMAND_ENABLE(CM_EDITCOPY, CeSelectEnable),
  EV_WM_KILLFOCUS,
  EV_WM_SYSCOLORCHANGE,
  EV_WM_SETFOCUS,
  EV_WM_HSCROLL,
  EV_WM_VSCROLL,
  EV_WM_MOUSEWHEEL,
  EV_WM_LBUTTONDOWN,
  EV_WM_MOUSEMOVE,
  EV_WM_LBUTTONUP,
#if !defined(COOL_NODRAGDROP)
  EV_WM_TIMER,
#endif
  EV_WM_KEYDOWN,
END_RESPONSE_TABLE;

//-----------------------------------------------------------------------------
bool
THexViewWnd::TTextSelection::TextToClipboard(TClipboard& cb, uint8* text, int len)
{
  // Allocate a global memory object for the text.
  HGLOBAL hHandle = ::GlobalAlloc(GMEM_DDESHARE, len);
  if (hHandle){
    // Lock the handle and copy the text to the buffer.
    void* pBlock = ::GlobalLock(hHandle);
    memcpy(pBlock, text, len);
    ::GlobalUnlock(hHandle);
    // Place the handle on the clipboard, as internal format.
    cb.SetClipboardData(GetFormat(), hHandle);
    return true;
  }
  return false;
}
//-----------------------------------------------------------------------------
bool
THexViewWnd::TTextSelection::Copy()
{
  bool retval = false;
  TClipboard clipboard(*GetParent());
  if (clipboard.EmptyClipboard()){
    auto ostrm = ostringstream{};
    if(Copy(ostrm)){
      const auto s = ostrm.str();
      retval = TextToClipboard(clipboard, (uint8*)s.c_str(), static_cast<int>(s.size()));
    }
  }
  return retval;
}
//-----------------------------------------------------------------------------
THexViewWnd::TLineSelection::TLineSelection(THexViewWnd* parent,
                                            int start, int end)
:
  TTextSelection(parent), Start(start), End(end)
{
}
//-----------------------------------------------------------------------------
bool
THexViewWnd::TLineSelection::GetSelection(int line, int& startChar,
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
THexViewWnd::TLineSelection::Copy(ostream& os)
{
  int start = Start, end   = End;
  if(start > end)
    std::swap(start,end);

  os << lineSelectionId;


  uint charInLine = GetParent()->GetCharsInLine();
  TTmpBuffer<uint8> buff(charInLine);

  for(int i = start; i <= end; i++){
    uint length = GetParent()->GetLineData(i, buff, charInLine);
    os.write((const char*)(uint8*)buff,length);
  }
  os << ends;
  return true;
}
//-----------------------------------------------------------------------------
bool
THexViewWnd::TStreamSelection::GetSelection(int line, int& startChar,
                                            int& endChar)
{
  if(HasSelection(line)){
    TEditRange range(Range);
    range.Normalize();
    startChar = line > range.srow ? 0 : range.scol;
    endChar   = line < range.erow ? GetParent()->GetLineLength(line)+1 : range.ecol;
  }
  return true;
}
//-----------------------------------------------------------------------------
bool THexViewWnd::TStreamSelection::Copy(ostream& os)
{
  TEditRange range(Range);
  range.Normalize();

  os << streamSelectionId;

  uint charInLine = GetParent()->GetCharsInLine();
  TTmpBuffer<uint8> buff(charInLine);

  for(int i = range.srow; i <= range.erow; i++){
    int length = GetParent()->GetLineData(i, buff, charInLine);
    uint8* text = buff;
    if(i == range.srow){
      if(range.srow == range.erow && range.ecol < length)
        length = range.ecol;
      text += range.scol;
    }
    else if(i == range.erow && range.ecol < length)
      length = range.ecol;
    length -= range.scol;
    os.write((const char*)text,length);
  }
  os << ends;
  return true;
}
//-----------------------------------------------------------------------------
// class THexViewWnd
// ~~~~~ ~~~~~~~~~~~~
//
THexViewWnd::THexViewWnd(TWindow* parent,int id,LPCTSTR title,int x, int y,
                         int w, int h, TModule* module)
:
  Inherited(parent,id,title,x,y,w,h,module)
  ,MemoryBitmap(0)
  ,CursorPos(0,0)
  ,Flags(cfAddress|cfDigit|cfAscii/*|cfNoUseCaret*/)
  ,LineHeight(-1)
  ,CharWidth(-1)
  ,ScreenLines(-1)
  ,ScreenChars(-1)
  ,SelType(stStreamSelection)
  ,Selection(0)
  ,PosType(psDigit)
#if !defined(COOL_NODRAGDROP)
  ,DragDropProxy(new THexDragDropProxy)
  ,DragDropSupport(DragDropProxy)
  ,DragSelTimer(0)
#endif
{
  ModifyStyle(0, WS_VSCROLL|WS_HSCROLL);
  SetBkgndColor(TColor::Transparent);
  memset(TextFonts,0,sizeof(TextFonts));
  if(!SelClipFormat)
    SelClipFormat = ::RegisterClipboardFormat(_T("HexEdit Binary Data"));
}
//-----------------------------------------------------------------------------
THexViewWnd::~THexViewWnd()
{
#if !defined(COOL_NODRAGDROP)
  delete DragDropProxy;
#endif
  delete MemoryBitmap;
  for(int i = 0; i < 8; i++)
    delete TextFonts[i];
  delete Selection;
}
//-----------------------------------------------------------------------------
void THexViewWnd::SetupWindow()
{
  if(!Scroller)
    Scroller = new TScroller(this, 7, 16, 80, 60);
#if !defined(USE_AUTOORG)
  Scroller->AutoOrg = false;
#endif

  // Call base class function.
  Inherited::SetupWindow();

#if !defined(COOL_NODRAGDROP)
  DragDropSupport.Register(*this);
#endif

  SetCursor(0, IDC_IBEAM);

  THexBuffer* buffer = GetBuffer();
  if(!buffer->SyntaxDescrExist())
    buffer->BuildDefSyntaxDescr();

}
//-----------------------------------------------------------------------------
void THexViewWnd::CleanupWindow()
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
//
TConfigFile*
THexViewWnd::CreateConfigFile()
{
  return new TRegConfigFile(configOWLNExt);
}
//-----------------------------------------------------------------------------
void
THexViewWnd::GetWindowClass(WNDCLASS& wndClass)
{
  // Call base class function.
  Inherited::GetWindowClass(wndClass);
  wndClass.style |= CS_DBLCLKS;
}
//-----------------------------------------------------------------------------
auto THexViewWnd::GetWindowClassName() -> TWindowClassName
{
  return TWindowClassName{_T("HEXEDIT")};
}
//
void
THexViewWnd::FileNew()
{
  PRECONDITION(GetBuffer());
  GetBuffer()->Clear();
  ResetView();
  AdjustScroller();
  Invalidate();
}
//
bool
THexViewWnd::FileOpen(LPCTSTR filename)
{
  PRECONDITION(GetBuffer());
  if(!GetBuffer()->Load(filename))
    return false;
  ResetView();
  if(GetHandle()){
    AdjustScroller();
    Invalidate();
  }
  return true;
}
//-----------------------------------------------------------------------------
void THexViewWnd::ResetView()
{
  //ClearFlag(cfBookmarkExist);
  SelType           = stStreamSelection;
  delete Selection;
  Selection         = 0;
  CursorPos.row      = 0;
  CursorPos.col      = 0;
  //LastHit.x          = -1;
  //LastHit.y          = -1;
  if(Scroller){
    Scroller->XPos = 0;
    Scroller->YPos = 0;
  }

  if(GetHandle())
    UpdateCaret();
}
//-----------------------------------------------------------------------------
void THexViewWnd::AdjustScroller()
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
  int charsInLine = 0;
  if(IsShowAddress())
    charsInLine += IsShowWideAddress() ? 10 : 6;
  if(IsShowDigit())
    charsInLine += GetCharsInLine()*3+1;
  if(IsShowAscii())
    charsInLine += GetCharsInLine();

  TPoint range(charsInLine, GetNumLines()-1);
  Scroller->SetUnits(GetCharWidth(), GetLineHeight());

  Scroller->SetRange(range.x, range.y);

  //Scroller->ScrollTo(0, 0);
  if (!GetUpdateRect(clientRect, false))
    InvalidateLines(0, -1, false);
  in_work = false;
}
//-----------------------------------------------------------------------------
void THexViewWnd::EvSize(uint sizeType, const TSize& size)
{
  Inherited::EvSize(sizeType, size);
  if (sizeType != SIZEICONIC) {
    delete MemoryBitmap;
    MemoryBitmap = 0;
    ScreenLines = -1;
    ScreenChars = -1;
    AdjustScroller();
    //Invalidate(false);
  }
}
//-----------------------------------------------------------------------------
TFont*
THexViewWnd::CreateFont(int index)
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
//-----------------------------------------------------------------------------
TFont*
THexViewWnd::GetFont(bool italic, bool bold, bool underline)
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
THexViewWnd::SetFont(const LOGFONT &lf)
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
  if(IsWindow()){
    AdjustScroller();
    UpdateCaret();
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::Paint(TDC& dc, bool /*erase*/, TRect&)
{
  TRect clientRect;
  GetClientRect(clientRect);

  THexBuffer* buffer = GetBuffer();
  if(!buffer){
    TColor oldClr = dc.SetBkColor(GetBkColor(HEXINDEX_WHITESPACE));
    dc.TextRect(clientRect);
    dc.SetBkColor(oldClr);
    return;
  }
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
  TRect cacheLine(0, 0, lineRect.Width(), lineHeight);

  int currentLine = GetTopLine();
  while (lineRect.Top() < clientRect.Bottom()){
    if(currentLine < lineCount){
      DrawMargin(memDC, cacheMargin, currentLine);
      DrawLine(memDC, cacheLine, currentLine);
    }
    else{
      DrawMargin(memDC, cacheMargin, -1);
      DrawLine(memDC, cacheLine, -1);
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
//-----------------------------------------------------------------------------
void
THexViewWnd::CalcLineCharDim()
{
  TClientDC  clientDC(*this);
  TSize extent = GetFont()->GetTextExtent(clientDC, _T("X"));
  LineHeight = extent.cy;
  if (LineHeight < 1)
    LineHeight = 1;
  CharWidth = extent.cx;
}
//-----------------------------------------------------------------------------
int THexViewWnd::GetNumLines() const
{
  THexBuffer* buffer = ((THexViewWnd*)this)->GetBuffer();
  if(!buffer)
    return 0;
  uint num = buffer->GetDataSize()/GetCharsInLine();
  if(buffer->GetDataSize()%GetCharsInLine())
    num++;
  return num;
}
//-----------------------------------------------------------------------------
int
THexViewWnd::GetCharsInLine() const
{
  return 16;//?????????????????????????????????????????????????????????????????
}
//-----------------------------------------------------------------------------
int
THexViewWnd::GetCharWidth() const
{
  if (CharWidth == -1)
    CONST_CAST(THexViewWnd*,this)->CalcLineCharDim();
  return CharWidth;
}
//-----------------------------------------------------------------------------
int
THexViewWnd::GetLineHeight() const
{
  if (LineHeight == -1)
    CONST_CAST(THexViewWnd*,this)->CalcLineCharDim();
  return LineHeight;
}
//-----------------------------------------------------------------------------
int
THexViewWnd::GetTopLine()
{
  return Scroller->YPos;
}
//-----------------------------------------------------------------------------
int
THexViewWnd::GetOffsetChar()
{
  return Scroller->XPos;
}
//-----------------------------------------------------------------------------
void
THexViewWnd::SetSyntaxFormat(TDC& dc, int index)
{
  THexBuffer::TSyntaxDescr& node = GetSyntaxDescr(index);
  dc.SetTextColor(node.TxColor);
  dc.SetBkColor(node.BkColor);
  dc.SelectObject(*GetFont(node.FontIndex));
}
//-----------------------------------------------------------------------------
void
THexViewWnd::InvalidateLines(int index1, int index2, bool invalidateMargin)
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
//-----------------------------------------------------------------------------
int
THexViewWnd::GetScreenLines()
{
  if (ScreenLines == -1){
    TRect rect;
    GetClientRect(rect);
    ScreenLines = rect.Height() / GetLineHeight();
  }
  return ScreenLines;
}
//-----------------------------------------------------------------------------
bool
THexViewWnd::IsInSelection(const TEditPos& textPos) const
{
  if(Selection && Selection->HasSelection(textPos.row)){
    int startSel, endSel;
    if(Selection->GetSelection(textPos.row, startSel, endSel))
      return textPos.col >= startSel && textPos.col < endSel;
  }
  return false;
}
//-----------------------------------------------------------------------------
void
THexViewWnd::EnableShowAddress(bool enable)
{
  if(IsShowAddress() != enable){
    enable ? SetFlag(cfAddress) : ClearFlag(cfAddress);
    if (IsWindow())
      InvalidateLines(0, -1);
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::EnableShowDigit(bool enable)
{
  if(IsShowDigit() != enable){
    enable ? SetFlag(cfDigit) : ClearFlag(cfDigit);
    if (IsWindow())
      InvalidateLines(0, -1);
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::EnableShowAscii(bool enable)
{
  if(IsShowAscii() != enable){
    enable ? SetFlag(cfAscii) : ClearFlag(cfAscii);
    if (IsWindow())
      InvalidateLines(0, -1);
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::EnableShowWideAddress(bool enable)
{
  if(IsShowWideAddress() != enable){
    enable ? SetFlag(cfShowWideAddr) : ClearFlag(cfShowWideAddr);
    if (IsWindow())
      InvalidateLines(0, -1);
  }
}
//-----------------------------------------------------------------------------
int
THexViewWnd::GetScreenChars()
{
  if (ScreenChars == -1){
    TRect rect;
    GetClientRect(rect);
    ScreenChars = (rect.Width() - GetMarginWidth()) / GetCharWidth();
  }
  return ScreenChars;
}
//-----------------------------------------------------------------------------
int
THexViewWnd::GetLineLength(int index)
{
  THexBuffer* buffer = ((THexViewWnd*)this)->GetBuffer();
  CHECK(buffer);
  index = index*GetCharsInLine();
  int retsize = buffer->GetDataSize() - index;
  return std::min(retsize, GetCharsInLine());
}
//-----------------------------------------------------------------------------
uint
THexViewWnd::GetLineData(int index, uint8* buff, uint size) const
{
  THexBuffer* buffer = ((THexViewWnd*)this)->GetBuffer();
  CHECK(buffer);
  index = index*GetCharsInLine();
  int retsize = buffer->GetDataSize() - index;
  retsize = std::min(std::min(retsize, GetCharsInLine()), static_cast<int>(size));
  return buffer->GetBuffer(index, buff, retsize);
}
//-----------------------------------------------------------------------------
void
THexViewWnd::DrawMargin(TDC&, const TRect&, int /*lineIndex*/)
{
  // nothing for now
}
//-----------------------------------------------------------------------------
void
THexViewWnd::DrawLine(TDC& dc, const TRect& rect, int lineIndex)
{
  PRECONDITION(lineIndex >= -1 && lineIndex < GetNumLines());
  PRECONDITION(GetBuffer());

  if(lineIndex == -1){
    //  Draw line beyond the text
    dc.SetBkColor(GetBkColor(HEXINDEX_WHITESPACE));
    dc.TextRect(rect);
    return;
  }

  //  Acquire the background color for the current line
  bool drawWhitespace = false;
  int colorIdx = HEXINDEX_ADDRESS;


  TTmpBuffer<uint8> buff(GetCharsInLine());
  int length = GetLineData(lineIndex, buff, GetCharsInLine());
  uint8* pszChars = buff;

  if (!length){
    //  Draw the empty line
    TRect r(rect);
    TEditPos editPos(0, lineIndex);
    if(IsFlagSet(cfFocused|cfShowInactiveSel) && IsInSelection(editPos)){
      dc.SetBkColor(GetBkColor(HEXINDEX_SELECTION));
      TRect rc(r.left, r.top, r.left + GetCharWidth(), r.bottom);
      dc.TextRect(rc);
      r.left += GetCharWidth();
    }
    dc.SetBkColor(GetBkColor(drawWhitespace ? colorIdx : HEXINDEX_WHITESPACE));
    dc.TextRect(r);
    return;
  }

  //  Draw the line text
#if !defined(USE_AUTOORG)
  TPoint origin(rect.left - GetOffsetChar() * GetCharWidth(), rect.top);
#else
  TPoint origin(rect.left, rect.top);
#endif

  TTmpBuffer<_TCHAR> buffer(256);
  uint charWidth = GetCharWidth();

  // paint address
  if(IsFlagSet(cfAddress)){
    _TCHAR fmt[8] = {_T ('%'), _T ('0'), _T ('4'), _T ('l'), _T ('X'), _T(' '),_T(' ')};
    int len  = IsShowWideAddress() ? 10 : 6;
    if(IsShowWideAddress())
      fmt[2] = _T ('8');
    wsprintf(buffer, fmt, lineIndex*GetCharsInLine());
    SetSyntaxFormat(dc, HEXINDEX_ADDRESS);
    VERIFY(dc.ExtTextOut(origin, ETO_CLIPPED, &rect, &buffer[0], len));

    // constant
    origin.x += charWidth * len;
  }

  int len = std::min(length, GetCharsInLine());
  if(IsFlagSet(cfDigit)){
    LPTSTR p = (_TCHAR*)buffer;
    int w = 0;
    int i = 0;
    for(; i < len; i++){
      TOHEX(pszChars[i], p);
      *p++ = _T (' ');
      w += 3;
    }
    *p = _T (' ');
    w++;
    SetSyntaxFormat(dc, HEXINDEX_DIGITS);
    VERIFY(dc.ExtTextOut(origin, ETO_CLIPPED, &rect, &buffer[0], w));

    // constant
    origin.x += charWidth * w;
    int width = 3*GetCharsInLine()+1;
    if(width > w){
      TRect frect(rect);
      frect.left = origin.x;
      dc.TextRect(frect);
      origin.x += charWidth * (width - w);
    }
  }

  if(IsFlagSet(cfAscii)){
    LPTSTR p = (_TCHAR*)buffer;
    int w = 0;
    for(int i = 0; i < len; i++){
      *p++ = _istprint(pszChars[i]) ? pszChars[i] : _T('.');
      w++;
    }

    SetSyntaxFormat(dc, HEXINDEX_ASCII);
    VERIFY(dc.ExtTextOut(origin, ETO_CLIPPED, &rect, &buffer[0], w));

    origin.x += charWidth * w;

    int width = GetCharsInLine();
    if(width > w){
      TRect frect(rect);
      frect.left = origin.x;
      dc.TextRect(frect);
      origin.x += charWidth * (width - w);
    }
  }

  //  Draw whitespaces to the left of the text
  TRect frect(rect);
  if (origin.x > frect.left)
    frect.left = origin.x;
  if (frect.right > frect.left){
    if((IsFlagSet(cfFocused|cfShowInactiveSel)) && IsInSelection(TEditPos(length, lineIndex))){
      dc.SetBkColor(GetBkColor(HEXINDEX_SELECTION));
      TRect rc(frect.left, frect.top, frect.left+GetCharWidth(), frect.bottom);
      dc.TextRect(rc);
      frect.left += GetCharWidth();
    }
    if(frect.right > frect.left){
      dc.SetBkColor(GetBkColor(drawWhitespace ? colorIdx : HEXINDEX_WHITESPACE));
      dc.TextRect(frect);
    }
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::EnableCaret(bool enable)
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
//-----------------------------------------------------------------------------
void
THexViewWnd::EnableShowInactiveSel(bool enable)
{
  if(enable != IsShowInactiveSel()){
    enable ? SetFlag(cfShowInactiveSel) : ClearFlag(cfShowInactiveSel);
    if (IsWindow())
      InvalidateLines(0, -1);
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::EnableDragDrop(bool enable)
{
  if(IsFlagSet(cfNoDragDrop) != enable){
    enable ? SetFlag(cfNoDragDrop) : ClearFlag(cfNoDragDrop);
  }
}
//-----------------------------------------------------------------------------
bool
THexViewWnd::EvSetCursor(HWND hWndCursor, uint hitTest, uint mouseMsg)
{
  if (hitTest == HTCLIENT){
    TPoint point;
    Inherited::GetCursorPos(point);
    ScreenToClient(point);
    TPosType pos = Client2PosType(point);
    if(pos==psDigit || pos==psAscii){
      if(Selection && IsInSelection(Client2Text(point))){
#if !defined(COOL_NODRAGDROP)
        if(!IsFlagSet(cfNoDragDrop))
#endif
          ::SetCursor(::LoadCursor(0, TResId(IDC_ARROW))); // Set To Arrow Cursor
        return true;
      }
      else
        return Inherited::EvSetCursor(hWndCursor, hitTest, mouseMsg);
    }
    else{
      ::SetCursor(::LoadCursor(0, TResId(IDC_ARROW))); // Set To Arrow Cursor
      return true;
    }
  }
  return Inherited::EvSetCursor(hWndCursor, hitTest, mouseMsg);
}
//-----------------------------------------------------------------------------
THexViewWnd::TPosType
THexViewWnd::Client2PosType(const TPoint& point)
{
  int margin = GetMarginWidth();
  int leftMargin = GetCharWidth()*GetOffsetChar();
  if(IsShowAddress()){
    int addWidth = GetCharWidth()*(IsShowWideAddress() ? 8 : 4)+margin;
    if((point.x > margin-leftMargin) && (point.x < addWidth-leftMargin))
      return ptAddress;
    margin += addWidth + GetCharWidth()*2-margin;
  }
  if(IsShowDigit()){
    int digWidth = GetCharWidth()*(GetCharsInLine()*3-1)+margin;
    if((point.x > margin-leftMargin) && (point.x < digWidth-leftMargin))
      return psDigit;
    margin += digWidth + GetCharWidth()*2-margin;
  }
  if(IsShowAscii()){
    int ascWidth = GetCharWidth()*GetCharsInLine()+margin;
    if((point.x > margin-leftMargin) && (point.x < ascWidth-leftMargin))
      return psAscii;
  }
  return ptNone;
}
//-----------------------------------------------------------------------------
TEditPos
THexViewWnd::Client2Text(const TPoint& point, bool* sPart)
{
  int lineCount     = GetNumLines();
  TPosType posType = Client2PosType(point);
  if(posType==ptNone||posType==ptAddress)
    return TEditPos();

  TEditPos p(0, GetTopLine() + point.y / GetLineHeight());
  if(p.row >= lineCount)
    p.row = lineCount - 1;
  if(p.row < 0)
    p.row = 0;

  int  length = 0;
  if(p.row >= 0 && p.row < lineCount)
    length = GetLineLength(p.row);

  int index = -1;
  int charWidth = GetCharWidth();
  uint spos = GetOffsetChar()*charWidth;
  if(IsShowAddress())
    spos += charWidth*(IsShowWideAddress() ? 10 : 6);

  if(posType==psDigit){
    index = (point.x-spos)/(charWidth*3);
    if(index < GetCharsInLine()){
      p.col = index;
      if(sPart){
        *sPart = false;
        int rem = (point.x-spos)/charWidth%3;
        if(rem==1)
          *sPart = true;
        else if(rem==2)
          p.col++;
      }
      return p;
    }
  }
  else if(posType==psAscii){
    if(IsShowDigit())
      spos += charWidth*(GetCharsInLine()*3+1);
    index = (point.x-spos)/charWidth;
    if(index < GetCharsInLine()){
      p.col = index;
      return p;
    }
  }

  CHECK(index >= 0 && index <= length); InUse(length);
  p.col = index;
  return p;
}
//-----------------------------------------------------------------------------
TPoint
THexViewWnd::Text2Client(const TEditPos& pos)
{
  PRECONDITION(GetBuffer());
//  VERIFY_TEXTPOS(pos);

  TPoint p(0,(pos.row - GetTopLine())*GetLineHeight());
  if(PosType==ptAddress){
    p.x = 0;
  }
  else if(PosType==psDigit){
    if(IsShowAddress())
      p.x += IsShowWideAddress() ? 10 : 6;
    p.x += pos.col*3;
    if(IsFlagSet(cfSecondDigit))
      p.x += 1;
  }
  else if(PosType==psAscii){
    if(IsShowAddress())
      p.x += IsShowWideAddress() ? 10 : 6;
    if(IsShowDigit())
      p.x += 16*3+1;
    p.x += pos.col;
  }
  p.x = (p.x - GetOffsetChar()) * GetCharWidth() + GetMarginWidth();
  return p;
}
//-----------------------------------------------------------------------------
void
THexViewWnd::ShowCursor()
{
  if(!IsFlagSet(cfNoUseCaret)){
    ClearFlag(cfCursorHidden);
    UpdateCaret();
    ShowCaret();
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::HideCursor()
{
  if(!IsFlagSet(cfNoUseCaret)){
    SetFlag(cfCursorHidden);
    UpdateCaret();
    HideCaret();
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::SetCursorPos(const TEditPos& newPos)
{
  //VERIFY_TEXTPOS(newPos);
  CursorPos = newPos;
  UpdateCaret();
}
//-----------------------------------------------------------------------------
bool
THexViewWnd::CreateCaret()
{
  Inherited::CreateCaret(false, GetCharWidth(), GetLineHeight());
  return true;
}
//-----------------------------------------------------------------------------
void
THexViewWnd::UpdateCaret()
{
  if(IsFlagSet(cfNoUseCaret))
    return;
  if (IsFlagSet(cfFocused) && !IsFlagSet(cfCursorHidden) &&
      CalculateActualOffset(CursorPos.row, CursorPos.col) >= GetOffsetChar()){
    if(!IsFlagSet(cfHasCaret)){
      if(CreateCaret())
        SetFlag(cfHasCaret);
    }
    if(IsFlagSet(cfHasCaret)){
      SetCaretPos(Text2Client(CursorPos));
    }
  }
}
//-----------------------------------------------------------------------------
int
THexViewWnd::CalculateActualOffset(int lineIndex, int charIndex)
{
  int length = GetLineLength(lineIndex);
  return charIndex < length ? charIndex : length-1;
}
//-----------------------------------------------------------------------------
void THexViewWnd::EvKillFocus(HWND hWndGetFocus)
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
//-----------------------------------------------------------------------------
void THexViewWnd::EvSysColorChange()
{
  Inherited::EvSysColorChange();
  InvalidateLines(0, -1);
}
//-----------------------------------------------------------------------------
#if !defined(COOL_NODRAGDROP)
void THexViewWnd::EvTimer(uint timerId)
{
  if(timerId == HEX_TIMER_DEBOUNCE){
    CHECK(DragSelTimer);
    if(IsFlagSet(cfDragPending)){
      //_TRACE("TCoolTextWnd::EvTimer()\n");
      ExecuteDragDrop();
    }
  }
}
#endif
//-----------------------------------------------------------------------------
void THexViewWnd::EvSetFocus(HWND hWndLostFocus)
{
  Inherited::EvSetFocus(hWndLostFocus);

  SetFlag(cfFocused);
  if (Selection)
    InvalidateLines(Selection->GetStart().row, Selection->GetEnd().row);
  ShowCursor();
}
//-----------------------------------------------------------------------------
void THexViewWnd::GoToAddress(int address, bool relative)
{
  THexBuffer* buffer = GetBuffer();
  CHECK(buffer);

  int ypos = address/GetCharsInLine();
  int xpos = address%GetCharsInLine();
  TEditPos cursorPos(CursorPos);
  if(relative){
    cursorPos.row += ypos;
    cursorPos.col += xpos;
    if(cursorPos.col > GetCharsInLine()){
      cursorPos.row++;
      cursorPos.col = cursorPos.col-GetCharsInLine();
    }
  }
  else{
    cursorPos.row = ypos;
    cursorPos.col = xpos;
  }
  int maxpos = buffer->GetDataSize();
  if((cursorPos.row*GetCharsInLine()+cursorPos.col) > maxpos){
    cursorPos.row = maxpos/GetCharsInLine();
    cursorPos.col = maxpos%GetCharsInLine();
  }
  //VERIFY_TEXTPOS(cursorPos);
  SetCursorPos(cursorPos);
  SetSelection(TEditRange(cursorPos, cursorPos));
  ScrollToCaret(cursorPos);
}
//-----------------------------------------------------------------------------
void
THexViewWnd::ClearSelection()
{
  if(Selection){
    InvalidateLines(Selection->GetStart().row, Selection->GetEnd().row);
    delete Selection;
    Selection = 0;
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::SetSelection(const TEditRange& range)
{
  //VERIFY_TEXTPOS(range.Start());
  //VERIFY_TEXTPOS(range.End());
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
//-----------------------------------------------------------------------------
THexViewWnd::TTextSelection*
THexViewWnd::CreateSelection(const TEditRange& range)
{
  switch(SelType){
    case stStreamSelection:
      return new TStreamSelection(this, range);
    case stLineSelection:
      return new TLineSelection(this, range.srow, range.srow);
  }
  return 0;
}
//-----------------------------------------------------------------------------
void THexViewWnd::EvHScroll(uint scrollCode, uint thumbPos, HWND hWndCtl)
{
  Inherited::EvHScroll(scrollCode,thumbPos,hWndCtl);
  if(IsFlagSet(cfHasCaret))
    SetCaretPos(Text2Client(CursorPos));
}
//-----------------------------------------------------------------------------
void THexViewWnd::EvVScroll(uint scrollCode, uint thumbPos, HWND hWndCtl)
{
  Inherited::EvVScroll(scrollCode,thumbPos,hWndCtl);
  if(IsFlagSet(cfHasCaret))
    SetCaretPos(Text2Client(CursorPos));
}
//-----------------------------------------------------------------------------
void
THexViewWnd::EvMouseWheel(uint modKeys, int zDelta, const TPoint& point)
{
  Inherited::EvMouseWheel(modKeys, zDelta, point);
  if(IsFlagSet(cfHasCaret))
    SetCaretPos(Text2Client(CursorPos));
}
//-----------------------------------------------------------------------------
void
THexViewWnd::SelectAll()
{
  int lineCount = GetNumLines();
  CursorPos.col = GetLineLength(lineCount - 1);
  CursorPos.row = lineCount - 1;
  SetSelection(TEditRange(0, 0, CursorPos.col, CursorPos.row));
  UpdateCaret();
}
//
#if !defined(COOL_NODRAGDROP)
void
THexViewWnd::ExecuteDragDrop()
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
THexViewWnd::SetDragDropProxy(TDragDropProxy* proxy)
{
  TDragDropProxy* tmp = DragDropProxy;
  DragDropProxy = proxy;
  DragDropSupport.SetProxy(DragDropProxy);
  return tmp;
}
#endif
//-----------------------------------------------------------------------------
bool
THexViewWnd::Search(const TEditPos& /*startPos_*/, LPCTSTR text, uint /*len*/,
                     TFindFlags /*flags*/)
{
  if (!text || !text[0])  //???????????????????????????????????????????????????????
    return false;
/*
  TCoolTextBuffer* buffer = GetBuffer();
  TEditPos startPos(startPos_);
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
    TEditRange resRange(resPos.col,resPos.row,resPos.col+_tcslen(text), resPos.row);
    SetSelection(resRange);
    CursorPos = resRange.End();
    ScrollToCaret(CursorPos);
    return true;
  }
*/
  return false;
}
//
#if !defined(COOL_NODRAGDROP)
DROPEFFECT
THexViewWnd::GetDropEffect()
{
  //_TRACE("THexViewWnd::GetDropEffect\n");
  return DROPEFFECT_COPY;
}
//
void
THexViewWnd::DropSource(DROPEFFECT de)
{
  CHECK(de == DROPEFFECT_COPY); InUse(de);
  //_TRACE("THexViewWnd::DropSource\n");
}
//
void
THexViewWnd::DropText(IDataObject*, const TPoint&, DROPEFFECT)
{
  //_TRACE("THexViewWnd::DropText\n");
  // nothing
}
//
bool
THexViewWnd::SetDragData()
{
  if(Selection){
    auto ostrm = ostringstream{};
    Selection->Copy(ostrm);
    const auto s = ostrm.str();
    const char* text = s.c_str();
    std::streamsize len = s.size();

    // Allocate a global memory object for the text.
    HGLOBAL hHandle = ::GlobalAlloc(GMEM_DDESHARE, static_cast<SIZE_T>(len));
    if (hHandle){
      // Lock the handle and copy the text to the buffer.
      void* pBlock = ::GlobalLock(hHandle);
      memcpy(pBlock, text, static_cast<size_t>(len));
      ::GlobalUnlock(hHandle);
      // Place the handle on the clipboard, as internal format.
      DragDropProxy->SetDragData(SelClipFormat, hHandle);
    }
    else{
      return false;
    }
    return true;
  }
  return false;
}
#endif
//-----------------------------------------------------------------------------
void THexViewWnd::EvLButtonDown(uint modKeys, const TPoint& point)
{
  //_TRACE("TCoolTextWnd::EvLButtonDown()\n");
  bool needRedraw = false;
  bool bShift   = modKeys & MK_SHIFT;
  bool bControl = modKeys & MK_CONTROL;

  //We don't need to capture the mouse - TScroller does it.
  SetFlag(cfMouseDown);
  if(point.x < GetMarginWidth()){
    //AdjustTextPoint(point);
    if(bControl){
      SelectAll();
      ClearFlag(cfMouseDown);
      return;
    }
    else{
      CursorPos   = Client2Text(point);
      CursorPos.col = 0; // set new position on start of line
      TEditPos endPos(CursorPos);
      endPos.col  = GetLineLength(CursorPos.row); //  Force full line
      SetSelection(TEditRange(CursorPos, endPos));
      UpdateCaret();
      Inherited::EvLButtonDown(modKeys, point);
      return;
    }
  }

  PosType   = Client2PosType(point);
  bool selPos;
  CursorPos = Client2Text(point, &selPos);
  selPos ? SetFlag(cfSecondDigit) : ClearFlag(cfSecondDigit);

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
        DragSelTimer = SetTimer(HEX_TIMER_DEBOUNCE, DragDropSupport.GetDelay());
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
    SelType   = stStreamSelection;
    Selection = CreateSelection(TEditRange(CursorPos, CursorPos));
    SelType   = selType;
  }
  UpdateCaret();
  if(needRedraw){
    InvalidateLines(0, -1, false);
  }
  Inherited::EvLButtonDown(modKeys, point);
}
//-----------------------------------------------------------------------------
void THexViewWnd::EvMouseMove(uint modKeys, const TPoint& point)
{
  if(IsFlagSet(cfMouseDown)){
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
//-----------------------------------------------------------------------------
void THexViewWnd::EvLButtonUp(uint modKeys, const TPoint& point)
{
  //_TRACE("THexViewWnd::EvLButtonUp() START\n");
#if !defined(COOL_NODRAGDROP)
  // Workaround in conflict Scroller->AutoMode  and Drag-and-Drop
  if(!Scroller->AutoMode){
    Scroller->AutoMode = true;
    return;
  }
#endif
  if(IsFlagSet(cfMouseDown)){
    ClearFlag(cfMouseDown);

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
  //_TRACE("THexViewWnd::EvLButtonUp() Scroller->AutoMode = true\n");
  Inherited::EvLButtonUp(modKeys, point);
}
//-----------------------------------------------------------------------------
void
THexViewWnd::PreCheckSelection()
{
  if(!Selection)
    Selection = CreateSelection(TEditRange(CursorPos, CursorPos));
}
//-----------------------------------------------------------------------------
void
THexViewWnd::PostCheckSelection()
{
  if(Selection && Selection->IsEmpty()){
    delete Selection;
    Selection = 0;
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::ScrollToCaret(const TEditPos& cursorPos)
{
  int dx=0,dy=0;

  //VERIFY_TEXTPOS(cursorPos);
  TPoint caretPos = Text2Client(cursorPos);
  TRect  clientRect;
  GetClientRect(clientRect);

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

    //VERIFY_TEXTPOS(cursorPos);
    caretPos = Text2Client(cursorPos);

  }
  SetCursorPos(Client2Text(caretPos));
}
//-----------------------------------------------------------------------------
void
THexViewWnd::EvKeyDown(uint key, uint repeatCount, uint /*flags*/)
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
      if(is_shift){
         ExtendRight(repeatCount);
      }
      else{
        ClearSelection();
        MoveRight(repeatCount);
      }
      break;
    case VK_LEFT:
      if(is_shift){
        ExtendLeft(repeatCount);
      }
      else{
        ClearSelection();
        MoveLeft(repeatCount);
      }
      break;
    case VK_DOWN:
      if(is_ctrl){
        if(!is_shift)
          ScrollDown(repeatCount);
      }
      else{
        if(is_shift)
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
        if(is_shift)
          ExtendUp(repeatCount);
        else{
          ClearSelection();
          MoveUp(repeatCount);
        }
      }
      break;

    case VK_NEXT: //Page Down
      if(!is_ctrl){
        if(is_shift)
          ExtendPgDown(repeatCount);
        else{
          ClearSelection();
          MovePgDown(repeatCount);
        }
      }
      break;

    case VK_PRIOR:  //Page up
      if(!is_ctrl){
        if(is_shift)
          ExtendPgUp(repeatCount);
        else{
          ClearSelection();
          MovePgUp(repeatCount);
        }
      }
      break;

    case VK_HOME:
      if(is_shift){
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
      if(is_shift){
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
      //Anything else is a character key - let windows have it
      //back and we will catch it in EvChar.
    default:
      DefaultProcessing();
      return; //NOTE - return not break!
  }
  //Inherited::EvKeyDown(key, repeatCount, flags); //????
}
//-----------------------------------------------------------------------------
void
THexViewWnd::KeyEscape(int /*repeatCount*/)
{
  if(!IsStreamSelMode())
    SelType = stStreamSelection;
  else if(Selection)
    ClearSelection();
  else
    DefaultProcessing();
}
//-----------------------------------------------------------------------------
void
THexViewWnd::ExtendRight(int repeatCount)
{
  int lastLine = GetNumLines()-1;
  if(CursorPos.row < lastLine ||
      (CursorPos.row == lastLine && CursorPos.col < GetLineLength(lastLine))){
    PreCheckSelection();
    int startPos = CursorPos.row;
    while(repeatCount--){
      if(PosType==psDigit){
        if(IsFlagSet(cfSecondDigit))
          CursorPos.col++,ClearFlag(cfSecondDigit);
        else
          SetFlag(cfSecondDigit);
      }
      else
        CursorPos.col++;
      if(CursorPos.col >= GetLineLength(CursorPos.row)){
        if(CursorPos.row < GetNumLines()-1){
          CursorPos.row++;
          CursorPos.col = 0;
        }
      }
    }
    Selection->Extend(CursorPos);
    PostCheckSelection();
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::MoveRight(int repeatCount)
{
  int lastLine = GetNumLines()-1;
  if(CursorPos.row < lastLine ||
      (CursorPos.row == lastLine && CursorPos.col < GetLineLength(lastLine))){
    int startPos = CursorPos.row;
    while(repeatCount--){
      if(PosType==psDigit){
        if(IsFlagSet(cfSecondDigit))
          CursorPos.col++,ClearFlag(cfSecondDigit);
        else
          SetFlag(cfSecondDigit);
      }
      else
        CursorPos.col++;
      if(CursorPos.col >= GetLineLength(CursorPos.row)){
        if(CursorPos.row < GetNumLines()-1){
          CursorPos.row++;
          CursorPos.col = 0;
        }
      }
    }
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::ExtendLeft(int repeatCount)
{
  if(CursorPos.row > 0 || (CursorPos.row == 0 && CursorPos.col > 0)){
    int startPos = CursorPos.row;
    PreCheckSelection();
    while(repeatCount--){
      if(PosType==psDigit){
        if(IsFlagSet(cfSecondDigit))
          ClearFlag(cfSecondDigit);
        else
          CursorPos.col--,SetFlag(cfSecondDigit);
      }
      else
        CursorPos.col--;
      if(CursorPos.col < 0){
        if(CursorPos.row > 0){
          CursorPos.row--;
          CursorPos.col = GetLineLength(CursorPos.row);
        }
        else
          CursorPos.col = 0;
      }
    }
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);

    Selection->Extend(CursorPos);
    PostCheckSelection();
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::MoveLeft(int repeatCount)
{
  if(CursorPos.row>0 || (CursorPos.row==0 && CursorPos.col>0)){
    int startPos = CursorPos.row;
    while(repeatCount--){
      if(PosType==psDigit){
        if(IsFlagSet(cfSecondDigit))
          ClearFlag(cfSecondDigit);
        else
          CursorPos.col--,SetFlag(cfSecondDigit);
      }
      else
        CursorPos.col--;

      if(CursorPos.col < 0){
        if(CursorPos.row > 0){
          CursorPos.row--;
          CursorPos.col = GetLineLength(CursorPos.row);
        }
        else
          CursorPos.col = 0;
      }
    }
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::ExtendDown(int repeatCount)
{
  if(CursorPos.row < GetNumLines()-1){
    int startPos = CursorPos.row;
    PreCheckSelection();
    CursorPos.row += repeatCount;
    if(CursorPos.row > GetNumLines()-1)
      CursorPos.row = GetNumLines()-1;
    Selection->Extend(CursorPos);
    PostCheckSelection();
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::MoveDown(int repeatCount)
{
  if(CursorPos.row < GetNumLines()-1){
    int startPos = CursorPos.row;
    CursorPos.row += repeatCount;
    if(CursorPos.row > GetNumLines()-1)
      CursorPos.row = GetNumLines()-1;
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::ExtendUp(int repeatCount)
{
  if(CursorPos.row > 0){
    int startPos = CursorPos.row;
    PreCheckSelection();
    CursorPos.row -= repeatCount;
    if(CursorPos.row < 0)
      CursorPos.row = 0;
    Selection->Extend(CursorPos);
    PostCheckSelection();
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::MoveUp(int repeatCount)
{
  if(CursorPos.row > 0){
    int startPos = CursorPos.row;
    CursorPos.row -= repeatCount;
    if(CursorPos.row < 0)
      CursorPos.row = 0;
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::ExtendPgDown(int repeatCount)
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
      Scroller->ScrollBy(0, newTopLine-topLine);
      Selection->Extend(CursorPos);
      PostCheckSelection();
      ScrollToCaret(CursorPos);
      InvalidateLines(startPos, CursorPos.row);
    }
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::MovePgDown(int repeatCount)
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
      Scroller->ScrollBy(0, newTopLine-topLine);
      ScrollToCaret(CursorPos);
      InvalidateLines(startPos, CursorPos.row);
    }
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::ExtendPgUp(int repeatCount)
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
      Scroller->ScrollBy(0, newTopLine-topLine);
      Selection->Extend(CursorPos);
      PostCheckSelection();
      ScrollToCaret(CursorPos);
      InvalidateLines(startPos, CursorPos.row);
    }
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::MovePgUp(int repeatCount)
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
      Scroller->ScrollBy(0, newTopLine-topLine);
      ScrollToCaret(CursorPos);
      InvalidateLines(startPos, CursorPos.row);
    }
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::ExtendCtrlHome(int /*repeatCount*/)
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
//-----------------------------------------------------------------------------
void
THexViewWnd::MoveCtrlHome(int /*repeatCount*/)
{
  if(CursorPos.col || CursorPos.row){
    int startPos = CursorPos.row;
    CursorPos.col = 0;
    CursorPos.row = 0;
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::ExtendHome(int /*repeatCount*/)
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
//-----------------------------------------------------------------------------
void
THexViewWnd::MoveHome(int /*repeatCount*/)
{
  if(CursorPos.col){
    int startPos = CursorPos.row;
    CursorPos.col = 0;
    ScrollToCaret(CursorPos);
    InvalidateLines(startPos, CursorPos.row);
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::ExtendCtrlEnd(int /*repeatCount*/)
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
//-----------------------------------------------------------------------------
void
THexViewWnd::MoveCtrlEnd(int /*repeatCount*/)
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
//-----------------------------------------------------------------------------
void
THexViewWnd::ExtendEnd(int /*repeatCount*/)
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
//-----------------------------------------------------------------------------
void
THexViewWnd::MoveEnd(int /*repeatCount*/)
{
  int endPos = GetLineLength(CursorPos.row);
  if(CursorPos.col != endPos){
    CursorPos.col = endPos;
    ScrollToCaret(CursorPos);
    InvalidateLines(CursorPos.row, CursorPos.row);
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::ScrollDown(int repeatCount)
{
  if(Scroller->YPos < Scroller->YRange){
    if(repeatCount > (Scroller->YRange-Scroller->YPos))
      repeatCount = Scroller->YRange - Scroller->YPos;
    Scroller->ScrollBy(0,repeatCount);
    CursorPos.row += repeatCount;
    SetCursorPos(CursorPos);
  }
}
//-----------------------------------------------------------------------------
void
THexViewWnd::ScrollUp(int repeatCount)
{
  if(Scroller->YPos > 0){
    if(repeatCount > Scroller->YPos)
      repeatCount = Scroller->YPos;
    Scroller->ScrollBy(0,-(int)repeatCount);
    CursorPos.row -= repeatCount;
    SetCursorPos(CursorPos);
  }
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// THexEditView Implementation
//
//
// Build a response table for all messages/commands handled
// by THexEditView derived from THexViewWnd.
//
DEFINE_RESPONSE_TABLE1(THexEditView, THexViewWnd)
  EV_VN_ISWINDOW,
END_RESPONSE_TABLE;
//
THexEditView::THexEditView(TDocument& doc, TWindow* parent)
:
  THexViewWnd(parent, 0, _T(""), 0, 0, 0, 0, 0),
  TView(doc)
{
}
//
THexEditView::~THexEditView()
{
  //
}

/*
void CHexEditView::OnLButtonDown(UINT nFlags, CPoint point)
{
  SetFocus();
  if(!m_pData || !*m_pData)
    return;

  if(nFlags & MK_SHIFT)
  {
    m_selStart = m_currentAddress;
  }
  CPoint pt = CalcPos(point.x, point.y);
  if(pt.x > -1)
  {
    m_editPos = pt;
    pt.x *= m_nullWidth;
    pt.y *= m_lineHeight;

    if(pt.x == 0 && (m_dwFlags & HVW_SHOW_ADDRESS))
      CreateAddressCaret();
    else
      CreateEditCaret();

    SetCaretPos(pt);
    if(nFlags & MK_SHIFT)
    {
      m_selEnd = m_currentAddress;
      if(m_currentMode == EDIT_HIGH || m_currentMode == EDIT_LOW)
        m_selEnd++;
      RedrawWindow();
    }
  }
  if(!(nFlags & MK_SHIFT))
  {
    if(DragDetect(this->m_hWnd, point))
    {
      m_selStart = m_currentAddress;
      m_selEnd   = m_selStart;
      SetCapture();
    }
    else
    {
      BOOL bsel = m_selStart != 0xffffffff;

      m_selStart = 0xffffffff;
      m_selEnd   = 0xffffffff;
      if(bsel)
        RedrawWindow();
    }
  }
  if(!IsSelected())
  {
    ShowCaret();
  }
}

void CHexEditView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
  nFlags; point;
}

void CHexEditView::OnMouseMove(UINT nFlags, CPoint point)
{
  if(!m_pData || !*m_pData)
    return;

  if(nFlags & MK_LBUTTON && m_selStart != 0xffffffff)
  {
    CRect rc;
    GetClientRect(&rc);
    if(!rc.PtInRect(point))
    {
      if(point.y < 0)
      {
        OnVScroll(SB_LINEUP, 0, NULL);
        point.y = 0;
      }
      else if(point.y > rc.Height())
      {
        OnVScroll(SB_LINEDOWN, 0, NULL);
        point.y = rc.Height() -1;
      }
    }

    //
    // we are selecting
    //
    int  seo = m_selEnd;
    CPoint pt = CalcPos(point.x, point.y);
    if(pt.x > -1)
    {
      m_selEnd = m_currentAddress;
      if(m_currentMode == EDIT_HIGH || m_currentMode == EDIT_LOW)
        m_selEnd++;
    }
    if(IsSelected())
      DestroyCaret();

    if(seo != m_selEnd)
      RedrawWindow();
  }
}

void CHexEditView::OnLButtonUp(UINT nFlags, CPoint point)
{
  if(IsSelected())
    ReleaseCapture();

  CWnd::OnLButtonUp(nFlags, point);
}

void CHexEditView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  nFlags;nRepCnt;
  if(!m_pData || !*m_pData)
    return;
  if(nChar == _T ('\t'))
    return;
  if(GetKeyState(VK_CONTROL) & 0x80000000)
  {
    switch(nChar)
    {
      case 0x03:
        if(IsSelected())
          OnEditCopy();
        return;
      case 0x16:
        OnEditPaste();
        return;
      case 0x18:
        if(IsSelected())
          OnEditCut();
        return;
      case 0x1a:
        OnEditUndo();
        return;
    }
  }

  if(nChar == 0x08)
  {
    if(m_currentAddress > 0)
    {
      m_currentAddress--;
      SelDelete(m_currentAddress, m_currentAddress+1);
      RepositionCaret(m_currentAddress);
      RedrawWindow();
    }
    return;
  }

  SetSel(-1, -1);
  if (!(m_dwFlags & HVW_READ_ONLY))
    switch(m_currentMode)
    {
      case EDIT_NONE:
        return;
      case EDIT_HIGH:
      case EDIT_LOW:
        if((nChar >= _T ('0') && nChar <= _T ('9')) || (nChar >= _T ('a') && nChar <= _T ('f')))
        {
          UINT b = nChar - _T ('0');
          if(b > 9)
            b = 10 + nChar - _T ('a');

          if(m_currentMode == EDIT_HIGH)
          {
            (*m_pData)[m_currentAddress] = (unsigned char)(((*m_pData)[m_currentAddress] & 0x0f) | (b << 4));
          }
          else
          {
            (*m_pData)[m_currentAddress] = (unsigned char)(((*m_pData)[m_currentAddress] & 0xf0) | b);
          }
          CDocument *pDoc = GetDocument ();
          pDoc->SetModifiedFlag ();
          Move(1,0);
        }
        break;
      case EDIT_ASCII:
        {
          (*m_pData)[m_currentAddress] = (unsigned char)nChar;
          CDocument *pDoc = GetDocument ();
          pDoc->SetModifiedFlag ();
          Move(1,0);
        }
        break;
    }
  RedrawWindow();
}

void CHexEditView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  nFlags; nRepCnt;

  BOOL bShift = GetKeyState(VK_SHIFT) & 0x80000000;
  BOOL bac = (m_dwFlags & HVW_NO_ADDRESS_CHANGE) != 0;
  m_dwFlags |= HVW_NO_ADDRESS_CHANGE;
  switch(nChar)
  {
    case VK_DOWN:
      if(bShift)
      {
        if(!IsSelected())
        {
          m_selStart = m_currentAddress;
        }
        Move(0,1);
        m_selEnd   = m_currentAddress;
        if(m_currentMode == EDIT_HIGH || m_currentMode == EDIT_LOW)
          m_selEnd++;
        RedrawWindow();
        break;
      }
      else
        SetSel(-1, -1);
      Move(0,1);
      break;
    case VK_UP:
      if(bShift)
      {
        if(!IsSelected())
        {
          m_selStart = m_currentAddress;
        }
        Move(0,-1);
        m_selEnd   = m_currentAddress;
        RedrawWindow();
        break;
      }
      else
        SetSel(-1, -1);
      Move(0,-1);
      break;
    case VK_LEFT:
      if(bShift)
      {
        if(!IsSelected())
        {
          m_selStart = m_currentAddress;
        }
        Move(-1,0);
        m_selEnd   = m_currentAddress;
        RedrawWindow();
        break;
      }
      else
        SetSel(-1, -1);
      Move(-1,0);
      break;
    case VK_RIGHT:
      if(bShift)
      {
        if(!IsSelected())
        {
          m_selStart = m_currentAddress;
        }
        Move(1,0);
        m_selEnd   = m_currentAddress;
        if(m_currentMode == EDIT_HIGH || m_currentMode == EDIT_LOW)
          m_selEnd++;
        RedrawWindow();
        break;
      }
      else
        SetSel(-1, -1);
      Move(1,0);
      break;
    case VK_PRIOR:
      if(bShift)
      {
        if(!IsSelected())
        {
          m_selStart = m_currentAddress;
        }
        OnVScroll(SB_PAGEUP, 0, NULL);
        Move(0,0);
        m_selEnd   = m_currentAddress;
        RedrawWindow();
        break;
      }
      else
        SetSel(-1, -1);
      OnVScroll(SB_PAGEUP, 0, NULL);
      Move(0,0);
      break;
    case VK_NEXT:
      if(bShift)
      {
        if(!IsSelected())
        {
          m_selStart = m_currentAddress;
        }
        OnVScroll(SB_PAGEDOWN, 0, NULL);
        Move(0,0);
        m_selEnd   = m_currentAddress;
        RedrawWindow();
        break;
      }
      else
        SetSel(-1, -1);
      OnVScroll(SB_PAGEDOWN, 0, NULL);
      Move(0,0);
      break;
    case VK_HOME:
      if(bShift)
      {
        if(!IsSelected())
        {
          m_selStart = m_currentAddress;
        }
        if(GetKeyState(VK_CONTROL) & 0x80000000)
        {
          OnVScroll(SB_THUMBTRACK, 0, NULL);
          Move(0,0);
        }
        else
        {
          m_currentAddress /= m_bpr;
          m_currentAddress *= m_bpr;
          Move(0,0);
        }
        m_selEnd   = m_currentAddress;
        RedrawWindow();
        break;
      }
      else
        SetSel(-1, -1);
      if(GetKeyState(VK_CONTROL) & 0x80000000)
      {
        OnVScroll(SB_THUMBTRACK, 0, NULL);
        m_currentAddress = 0;
        Move(0,0);
      }
      else
      {
        m_currentAddress /= m_bpr;
        m_currentAddress *= m_bpr;
        Move(0,0);
      }
      break;
    case VK_END:
      if(bShift)
      {
        if(!IsSelected())
        {
          m_selStart = m_currentAddress;
        }
        if(GetKeyState(VK_CONTROL) & 0x80000000)
        {
          m_currentAddress = *m_length-1;
          OnVScroll(SB_THUMBTRACK, ((*m_length+(m_bpr/2)) / m_bpr) - m_lpp, NULL);
          Move(0,0);
        }
        else
        {
          m_currentAddress /= m_bpr;
          m_currentAddress *= m_bpr;
          m_currentAddress += m_bpr - 1;
          if(m_currentAddress > *m_length)
            m_currentAddress = *m_length-1;
          Move(0,0);
        }
        m_selEnd   = m_currentAddress;
        RedrawWindow();
        break;
      }
      else
        SetSel(-1, -1);
      if(GetKeyState(VK_CONTROL) & 0x80000000)
      {
        m_currentAddress = *m_length-1;
        if(m_dwFlags & HVW_HALF_PAGE)
          OnVScroll(SB_THUMBTRACK, 0, NULL);
        else
          OnVScroll(SB_THUMBTRACK, ((*m_length+(m_bpr/2)) / m_bpr) - m_lpp, NULL);
        Move(0,0);
      }
      else
      {
        m_currentAddress /= m_bpr;
        m_currentAddress *= m_bpr;
        m_currentAddress += m_bpr - 1;
        if(m_currentAddress > *m_length)
          m_currentAddress = *m_length-1;
        Move(0,0);
      }
      break;
    case VK_INSERT:
      SelInsert(m_currentAddress, std::max(1, m_selEnd-m_selStart));
      RedrawWindow();
      break;
    case VK_DELETE:
      if(IsSelected())
      {
        OnEditClear();
      }
      else
      {
        SelDelete(m_currentAddress, m_currentAddress+1);
        RedrawWindow();
      }
      break;
    case _T ('\t'):
      switch(m_currentMode)
      {
        case EDIT_NONE:
          m_currentMode = EDIT_HIGH;
          break;
        case EDIT_HIGH:
        case EDIT_LOW:
          m_currentMode = EDIT_ASCII;
          break;
        case EDIT_ASCII:
          m_currentMode = EDIT_HIGH;
          break;
      }
      Move(0,0);
      break;

  }
  if (bac)
    m_dwFlags |= HVW_NO_ADDRESS_CHANGE;
  else
    m_dwFlags &= ~HVW_NO_ADDRESS_CHANGE;
}

void CHexEditView::Move(int x, int y)
{
  switch(m_currentMode)
  {
    case EDIT_NONE:
      return;
    case EDIT_HIGH:
      if(x != 0)
        m_currentMode = EDIT_LOW;
      if(x == -1)
        m_currentAddress --;
      m_currentAddress += y* m_bpr;
      break;
    case EDIT_LOW:
      if(x != 0)
        m_currentMode = EDIT_HIGH;
      if(x == 1)
        m_currentAddress++;
      m_currentAddress += y* m_bpr;
      break;
    case EDIT_ASCII:
      {
        m_currentAddress += x;
        m_currentAddress += y*m_bpr;
      }
      break;
  }
  if(m_currentAddress < 0)
    m_currentAddress = 0;

  if(m_currentAddress >= *m_length)
  {
    m_currentAddress -= x;
    m_currentAddress -= y*m_bpr;
  }
  m_dwFlags |= HVW_NO_ADDRESS_CHANGE;
  if(m_currentAddress < m_topindex)
  {
    OnVScroll(SB_LINEUP, 0, NULL);
  }
  if(m_currentAddress >= m_topindex + m_lpp*m_bpr)
  {
    OnVScroll(SB_LINEDOWN, 0, NULL);
  }
  m_dwFlags &= ~HVW_NO_ADDRESS_CHANGE;
  //ScrollIntoView(m_currentAddress);
  RepositionCaret(m_currentAddress);
}

void CHexEditView::RepositionCaret(int   p)
{
  int x, y;

  y = (p - m_topindex) / m_bpr;
  x = (p - m_topindex) % m_bpr;

  switch(m_currentMode)
  {
    case EDIT_NONE:
      CreateAddressCaret();
      x = 0;
      break;
    case EDIT_HIGH:
      CreateEditCaret();
      x *= m_nullWidth*3;
      x += m_offHex;
      break;
    case EDIT_LOW:
      CreateEditCaret();
      x *= m_nullWidth*3;
      x += m_nullWidth;
      x += m_offHex;
      break;
    case EDIT_ASCII:
      CreateEditCaret();
      x *= m_nullWidth;
      x += m_offAscii;
      break;
  }
  m_editPos.x = x;
  m_editPos.y = y*m_lineHeight;
  CRect rc;
  GetClientRect(&rc);
  if(rc.PtInRect(m_editPos))
  {
    SetCaretPos(m_editPos);
    ShowCaret();
  }
}

void CHexEditView::OnEditClear()
{
  m_currentAddress = m_selStart;
  SelDelete(m_selStart, m_selEnd);
  RepositionCaret(m_currentAddress);
  RedrawWindow();
}

void CHexEditView::OnEditCopy()
{
  COleDataSource*   pSource = new COleDataSource();
  EmptyClipboard();
  if(m_currentMode != EDIT_ASCII)
  {
    int     dwLen = (m_selEnd-m_selStart);
    HGLOBAL   hMemb = ::GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE|GMEM_ZEROINIT, m_selEnd-m_selStart);
    HGLOBAL   hMema = ::GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE|GMEM_ZEROINIT, (dwLen) * 3);

    if (!hMema)
      return;

    // copy binary
    LPBYTE  p = (BYTE*)::GlobalLock(hMemb);
    memcpy(p, (*m_pData)+m_selStart, dwLen);
    ::GlobalUnlock(hMemb);

    // copy ascii
    p = (BYTE*)::GlobalLock(hMema);
    for(int  i = 0; i < dwLen;)
    {
      TOHEX((*m_pData)[m_selStart+i], p);
      *p++ = _T (' ');
      i++;
    }

    ::GlobalUnlock(hMema);

    pSource->CacheGlobalData(RegisterClipboardFormat(_T ("BinaryData")), hMemb);
    pSource->CacheGlobalData(CF_TEXT, hMema);
  }
  else
  {
    HGLOBAL       hMemb = ::GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE|GMEM_ZEROINIT, m_selEnd-m_selStart);
    HGLOBAL       hMema = ::GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE|GMEM_ZEROINIT, m_selEnd-m_selStart);
    if (!hMemb || !hMema)
      return;

    // copy binary
    LPBYTE  p = (BYTE*)::GlobalLock(hMemb);
    int   dwLen = m_selEnd-m_selStart;

    memcpy(p, (*m_pData)+m_selStart, dwLen);
    ::GlobalUnlock(hMemb);

    // copy ascii
    p = (BYTE*)::GlobalLock(hMema);
    memcpy(p, (*m_pData)+m_selStart, dwLen);
    for(int i = 0; i < dwLen; p++, i++)
      if(!isprint(*p))
        *p = _T ('.');
    ::GlobalUnlock(hMema);

    pSource->CacheGlobalData(RegisterClipboardFormat(_T ("BinaryData")), hMemb);
    pSource->CacheGlobalData(CF_TEXT, hMema);
  }
  pSource->SetClipboard();
}

void CHexEditView::OnEditPaste()
{
  COleDataObject  obj;
  if (obj.AttachClipboard())
  {
    HGLOBAL hmem = NULL;
    if (obj.IsDataAvailable(RegisterClipboardFormat(_T ("BinaryData"))))
    {
      hmem = obj.GetGlobalData(RegisterClipboardFormat(_T ("BinaryData")));
    }
    else if (obj.IsDataAvailable(CF_TEXT))
    {
      hmem = obj.GetGlobalData(CF_TEXT);
    }
    if(hmem)
    {
      LPBYTE  p = (BYTE*)::GlobalLock(hmem);
      DWORD dwLen = ::GlobalSize(hmem);
      int   insert;
      int   oa = m_currentAddress;

      NormalizeSel();
      if(m_selStart == 0xffffffff)
      {
        if(m_currentMode == EDIT_LOW)
          m_currentAddress++;
        insert = m_currentAddress;
        SelInsert(m_currentAddress, dwLen);
      }
      else
      {
        insert = m_selStart;
        SelDelete(m_selStart, m_selEnd);
        SelInsert(insert, dwLen);
        SetSel(-1, -1);
      }

      memcpy((*m_pData)+insert, p, dwLen);

      m_currentAddress = oa;
      RedrawWindow();
      ::GlobalUnlock(hmem);
    }
  }
}

int CHexEditView::GetOrigData(LPBYTE &p)
{
  p = *m_pData;
  return *m_length;
}

void CHexEditView::OnBpr (UINT nCmd)
{
  SetBPR(nCmd - ID_FORMAT_BPR_FIRST + 1);
  RedrawWindow ();
}

void CHexEditView::OnUpdateBpr (CCmdUI * pCmdUI)
{
  pCmdUI->SetCheck (pCmdUI->m_nID - ID_FORMAT_BPR_FIRST + 1 == (UINT) m_bpr);
}
*/

