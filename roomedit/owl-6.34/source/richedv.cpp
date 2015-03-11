//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
/// \file
/// Implementation of class TRichEditView
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/richedv.h>
#include <owl/choosefo.h>
#include <owl/chooseco.h>
#include <owl/docview.rh>
#include <owl/richedv.rh>

#include <stdio.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

using namespace std;

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlDocView);        // General Doc/View diagnostic group


// Makes it easier to read when specifying offsets.  Converts x to
// thousandths of inches.  Make sure x is decimal, not a fraction ( .25
// instead of 1/4).
//
#define TOHIENGLISH(x) (long) ((1000)*(x))


DEFINE_RESPONSE_TABLE1(TRichEditView, TRichEdit)
  EV_COMMAND(CM_FILESAVE,               CmFileSave),
  EV_COMMAND(CM_FILESAVEAS,             CmFileSaveAs),
//  EV_COMMAND(CM_FILEOPEN,               CmFileOpen),
//  EV_COMMAND(CM_FILENEW,                CmFileNew),
  EV_COMMAND(CM_FILECLOSE,              CmFileClose),
  EV_COMMAND(CM_FORMATFONT,              CmFormatFont),
  EV_COMMAND_ENABLE(CM_FORMATFONT,      CeFormatFont),
  EV_COMMAND(CM_FORMATCOLOR,             CmFormatColor),
  EV_COMMAND_ENABLE(CM_FORMATCOLOR,     CeFormatColor),
  EV_COMMAND(CM_PARAGRAPHLEFT,          CmParagraphLeft),
  EV_COMMAND_ENABLE(CM_PARAGRAPHLEFT,   CeParagraphLeft),
  EV_COMMAND(CM_PARAGRAPHCENTER,        CmParagraphCenter),
  EV_COMMAND_ENABLE(CM_PARAGRAPHCENTER, CeParagraphCenter),
  EV_COMMAND(CM_PARAGRAPHRIGHT,         CmParagraphRight),
  EV_COMMAND_ENABLE(CM_PARAGRAPHRIGHT,  CeParagraphRight),
  EV_COMMAND(CM_PARAGRAPHBULLET,        CmParagraphBullet),
  EV_COMMAND_ENABLE(CM_PARAGRAPHBULLET, CeParagraphBullet),
  EV_COMMAND(CM_FORMATBOLD,             CmFormatBold),
  EV_COMMAND_ENABLE(CM_FORMATBOLD,      CeFormatBold),
  EV_COMMAND(CM_FORMATITALIC,           CmFormatItalic),
  EV_COMMAND_ENABLE(CM_FORMATITALIC,    CeFormatItalic),
  EV_COMMAND(CM_FORMATUNDERLINE,        CmFormatUnderline),
  EV_COMMAND_ENABLE(CM_FORMATUNDERLINE, CeFormatUnderline),
  EV_COMMAND(CM_FORMATSTRIKEOUT,        CmFormatStrikeout),
  EV_COMMAND_ENABLE(CM_FORMATSTRIKEOUT, CeFormatStrikeout),
  EV_VN_DOCCLOSED,
  EV_VN_ISWINDOW,
  EV_VN_ISDIRTY,
  EV_VN_COMMIT,
  EV_VN_REVERT,
//  EV_WM_RBUTTONDOWN,
END_RESPONSE_TABLE;

//
// Create the rich edit control object
//
TRichEditView::TRichEditView(TDocument& doc, TWindow* parent)
:
  TRichEdit(parent, GetNextViewId(), (LPCTSTR)0,0,0,0,0),
  TView(doc)
{
  Attr.AccelTable = IDA_RICHEDITVIEW;
  if (::FindResource(*GetModule(), TResId(IDM_RICHEDITVIEW), RT_MENU))
    SetViewMenu( new TMenuDescr(IDM_RICHEDITVIEW,0,2,0,2,0,0, GetModule()));
}


TRichEditView::~TRichEditView()
{
}


//
// Does a given HWND belong to this view? Yes if it is us, or a child of us
//
bool
TRichEditView::VnIsWindow(HWND hWnd)
{
  return hWnd == TWindow::GetHandle() || IsChild(hWnd);
}


//
// Our document object was closed
//
bool
TRichEditView::VnDocClosed(int omode)
{
  if (VnIsDirty() || !(omode & ofWrite))  // make sure someone else's write
    return false;

  LoadData();
  return true;
}


//
//
//
bool
TRichEditView::LoadData()
{
  TPointer<tistream> inStream(Doc->InStream(ios::in | ios::binary));
  if (!inStream) {
    Doc->PostError(IDS_UNABLEOPEN, MB_OK);
    return false;
  }
  // Could check for a valid file (eg. FileSize != 0)
  // before proceeding with a call to Clear() here.
  //
  Clear();

  // Stream in data from file
  //
  bool status = ReadFromStream(*inStream, GetFormat());
  if (!status){
    Doc->PostError(IDS_READERROR, MB_OK);
  }
  else
   ClearModify();

  return status;
}


//
// Create the control with scroll bars for easy text editing
//
bool
TRichEditView::Create()
{
  //Attr.Style |= ( WS_HSCROLL | WS_VSCROLL /*| ES_AUTOHSCROLL*/);

  try {
    TRichEdit::Create();   // throws exception TWindow::TXWindow
  }
  catch (TXOwl&) {
    Doc->PostError(IDS_NOMEMORYFORVIEW, MB_OK);
    NotOK();
    return true;   // cannot return false - throws another exception
  }
  if (Doc->GetDocPath() == 0) {
    return true;           // new file, no data to display
  }
  if (!LoadData())
    NotOK();
  return true;
}

//
//
//
bool
TRichEditView::VnCommit(bool force)
{
  if (!force && !VnIsDirty())
    return true;

  TPointer<tostream> outStream(Doc->OutStream(ios::out | ios::binary));
  if(!outStream){
    Doc->PostError(IDS_UNABLEOPEN, MB_OK);
    return false;
  }

  // in case user change template ?????
  //
//  if(!_tcscmp(GetDocument().GetTemplate()->GetDefaultExt(),_T("rtf"))){
//    SetFormat(SF_RTF);
//    SetViewMenu( new TMenuDescr(IDM_RICHEDITVIEW,0,2,0,2,0,0) );
//  }
//  else{
//    SetFormat(SF_TEXT);
//    SetViewMenu( new TMenuDescr(IDM_EDITVIEW,0,2,0,0,0,0) );
//  }

   bool status = WriteToStream(*outStream, GetFormat());

  if (!status)
    Doc->PostError(IDS_WRITEERROR, MB_OK);
  else
    ClearModify();

  return status;
}


//
// Erase all changes since last save
//
bool
TRichEditView::VnRevert(bool clear)
{
  TRichEdit::Clear();
  ClearModify();   // reset edit control
  return clear ? true : LoadData();
}


//
// Set the font
//
void
TRichEditView::SetupWindow()
{
  // Select all the text in the new richedit.  Actually, there shouldn't
  // be any, but there seems to be some character format junk already
  // there.  If we don't select it too, it won't be changed by the
  // call to SetCharFormat() below, which means if the user cursors
  // to the last position in the richedit and types, he will be
  // typing in the default font (not our Courier New).
  //
  SetSelection(0,2);

  // Set the default format to an easy-reading font so everything lines
  // up
  //
  TCharFormat defaultFmt( *this, false);
  defaultFmt.SetFaceName(_T("Courier New"));
  defaultFmt.EnableBold(false);

  SetCharFormat(defaultFmt);
  SetSelection(0,0);

  // Oodles and oodles of text
  //
  SendMessage( EM_EXLIMITTEXT, 0, (DWORD)0xFFFFFFF );
  ClearModify();
}

//
// We need to make CanClose() a dummy function and let the DocManager
// determine if doc needs to be saved.  If we don't, the user will get
// two "you gonna save this before you close it, you moron?" message
// boxes.
//
bool
TRichEditView::CanClear()
{
  // Return true to let it fall through (pardon the inadvertent rhyme)
  //
  return true;
}

// menu response functions...

//
//
//
void
TRichEditView::CmFormatFont()
{
  //
  // Retrieve current character format
  //
  TCharFormat chFormat(*this);

  //
  // Fill logfont structure using info. from character format
  //
  LOGFONT lf;
  chFormat.SetFontInfo(lf);

  //
  // Initialize structure for Font common dialog
  //
  TChooseFontDialog::TData fontData;
  fontData.LogFont = lf;
  fontData.DC = TClientDC(*this);
  fontData.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS;
  fontData.Color = chFormat.GetTextColor();
  fontData.FontType = SCREEN_FONTTYPE;

  //
  // Display dialog
  //
  if (TChooseFontDialog(this, fontData).Execute() == IDOK) {
    //
    // Update character format from chosen font info
    //
    chFormat.GetFontInfo(fontData.LogFont);
    chFormat.SetTextColor(fontData.Color);
    SetCharFormat(chFormat);
  }
}

//
//
//
void
TRichEditView::CmFormatBold()
{
  ToggleCharAttribute(CFM_BOLD, CFE_BOLD);
}

//
//
//
void
TRichEditView::CmFormatItalic()
{
  ToggleCharAttribute(CFM_ITALIC, CFE_ITALIC);
}

//
//
//
void
TRichEditView::CmFormatUnderline()
{
  ToggleCharAttribute(CFM_UNDERLINE, CFE_UNDERLINE);
}

//
//
//
void
TRichEditView::CmFormatStrikeout()
{
  ToggleCharAttribute(CFM_STRIKEOUT, CFE_STRIKEOUT);
}


//
//
//
void
TRichEditView::CeFormatFont(TCommandEnabler& /*ce*/)
{
}

//
//
//
void
TRichEditView::CeFormatBold(TCommandEnabler& ce)
{
  uint result = HasCharAttribute(CFM_BOLD, CFE_BOLD);
  if( result == Yes )
    ce.SetCheck( TCommandEnabler::Checked );
  else if( result == No )
    ce.SetCheck( TCommandEnabler::Unchecked );
  else
    ce.SetCheck( TCommandEnabler::Indeterminate );
}

//
//
//
void
TRichEditView::CeFormatItalic(TCommandEnabler& ce)
{
  uint fmt = HasCharAttribute(CFM_ITALIC, CFE_ITALIC);
  if( fmt == Yes )
    ce.SetCheck( TCommandEnabler::Checked );
  else
  if( fmt == No )
    ce.SetCheck( TCommandEnabler::Unchecked );
  else
    ce.SetCheck( TCommandEnabler::Indeterminate );
}

//
//
//
void
TRichEditView::CeFormatUnderline(TCommandEnabler& ce)
{
  uint result = HasCharAttribute(CFM_UNDERLINE, CFE_UNDERLINE);
  if( result == Yes )
    ce.SetCheck( TCommandEnabler::Checked );
  else
  if( result == No )
    ce.SetCheck( TCommandEnabler::Unchecked );
  else
    ce.SetCheck( TCommandEnabler::Indeterminate );
}

//
//
//
void
TRichEditView::CeFormatStrikeout(TCommandEnabler& ce)
{
  uint result = HasCharAttribute(CFM_STRIKEOUT, CFE_STRIKEOUT);
  if( result == Yes )
    ce.SetCheck( TCommandEnabler::Checked );
  else
  if( result == No )
    ce.SetCheck( TCommandEnabler::Unchecked );
  else
    ce.SetCheck( TCommandEnabler::Indeterminate );
}


//
//
//
void
TRichEditView::CmParagraphLeft()
{
  TParaFormat pFormat;

  pFormat.SetAlignment(PFA_LEFT);
  SetParaFormat(pFormat);
}

//
//
//
void
TRichEditView::CmParagraphCenter()
{
  TParaFormat pFormat;

  pFormat.SetAlignment(PFA_CENTER);
  SetParaFormat(pFormat);
}

//
//
//
void
TRichEditView::CmParagraphRight()
{
  TParaFormat pFormat;

  pFormat.SetAlignment(PFA_RIGHT);
  SetParaFormat(pFormat);
}

//
//
//
void
TRichEditView::CmParagraphBullet()
{
  TParaFormat pFormat;

  pFormat.SetNumbering((pFormat.wNumbering == 0) ? (WORD)PFN_BULLET : (WORD)0);
  pFormat.SetOffset(TOHIENGLISH( .25 ));  // 1/4 inch offset.  The
                                          // richedit control operates
                                          // in MM_HIENGLISH
  SetParaFormat(pFormat);
}

//
//
//
void
TRichEditView::CeParagraphLeft(TCommandEnabler& tce)
{
  TParaFormat pFormat(*this);

  tce.SetCheck( (pFormat.wAlignment == PFA_LEFT) ?
                 TCommandEnabler::Checked : TCommandEnabler::Unchecked);
}

//
//
//
void
TRichEditView::CeParagraphCenter(TCommandEnabler& tce)
{
  TParaFormat pFormat(*this);

  tce.SetCheck( (pFormat.wAlignment == PFA_CENTER) ?
                TCommandEnabler::Checked : TCommandEnabler::Unchecked);
}

//
//
//
void
TRichEditView::CeParagraphRight(TCommandEnabler& tce)
{
  TParaFormat pFormat(*this);

  tce.SetCheck((pFormat.wAlignment == PFA_RIGHT) ?
                TCommandEnabler::Checked : TCommandEnabler::Unchecked);
}

//
//
//
void
TRichEditView::CeParagraphBullet(TCommandEnabler& tce)
{
  TParaFormat pFormat(*this);

  tce.SetCheck( (pFormat.wNumbering == PFN_BULLET) ?
                TCommandEnabler::Checked : TCommandEnabler::Unchecked);
}


//
//
//
void
TRichEditView::CmFormatColor()
{
  TChooseColorDialog::TData data;
  TCharFormat cf(*this);
  static TColor custColors[16];

  data.Flags      = CC_RGBINIT;
  data.Color      = cf.GetTextColor();
  data.CustColors = custColors;

  if( TChooseColorDialog(this, data).Execute() == IDOK ) {
    cf.SetTextColor( data.Color );
    SetCharFormat( cf );
  }
}

//
//
//
void
TRichEditView::CeFormatColor(TCommandEnabler& tce)
{
  tce.Enable();
}

IMPLEMENT_STREAMABLE2(TRichEditView, TRichEdit, TView);

#if !defined(BI_NO_OBJ_STREAMING)
//
//
void*
TRichEditView::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  ReadBaseObject((TRichEdit*)GetObject(), is);
  ReadBaseObject((TView*)GetObject(), is);
  return GetObject();
}

//
//
//
void
TRichEditView::Streamer::Write(opstream& os) const
{
  WriteBaseObject((TRichEdit*)GetObject(), os);
  WriteBaseObject((TView*)GetObject(), os);
}

#endif //BI_NO_OBJ_STREAMING

} // OWL namespace

//==============================================================================
