//----------------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
//  Copyright (c) 1998 by Yura Bidus. All Rights Reserved.
//
//
//  OVERVIEW
//  ~~~~~~~~
//  Source file for implementation of TTipDialog.
//
//----------------------------------------------------------------------------
#include <owlext\pch.h>
#pragma hdrstop

#  if !defined(OWL_REGISTRY_H)
#    include <owl/registry.h>
#  endif

#include <owlext/tipdialg.h>
#include <owlext/tipdialg.rh>

using namespace owl;

namespace OwlExt {

using namespace std;

static const _TCHAR szSection[]    = _T("Tip");
static const _TCHAR szIntFilePos[] = _T("FilePos");
static const _TCHAR szIntStartup[] = _T("StartUp");
static const _TCHAR szTipFile[]    = _T("\\tipfile.tip");

// private function
TModule* FindResourceModule(TWindow* parent, TModule* module, TResId resId, LPCTSTR type);

//****************************************************************************//
// Build a response table for all messages/commands handled
// by the application.
DEFINE_RESPONSE_TABLE1(TTipDialog, TDialog)
  EV_COMMAND(IDC_TIP_NEXT, CmNextTip),
  EV_WM_CTLCOLORSTATIC(EvCtlColor),
  EV_WM_PAINT,
END_RESPONSE_TABLE;


//@mfunc Constructor
//@parm TWindow* | parent | Parent window
//@parm char* | regKey | Registry key
TTipDialog::TTipDialog(TWindow* parent, LPCTSTR regkey, TModule* module)
:
TDialog(parent, IDD_TIPDLG, FindResourceModule(parent,module,IDD_TIPDLG,RT_DIALOG)),
ShowDlg(true),
TipError(false),
RegKey(regkey),
BulbBitmap(0)
{
}
//------------------------------------------------------------------------------
TTipDialog::~TTipDialog()
{
  delete BulbBitmap;
}
//------------------------------------------------------------------------------
bool TTipDialog::ShowOnStartup()
{
  if(CurrentTip.empty()){
    long pos = ReadSettings();
    OpenTipFile(pos);
  }
  return ShowDlg;
}
//------------------------------------------------------------------------------
//@mfunc GetTipName()
owl::tstring TTipDialog::GetTipName()
{
  // if only filename assume that file placed in programm directory
  TAPointer<_TCHAR> buffer(new _TCHAR[_MAX_PATH]);
  GetApplication()->GetModuleFileName(buffer, _MAX_PATH);
  _TCHAR* lp = _tcsrchr(buffer, _T('\\'));
  *lp = 0;
  _tcscat(buffer, szTipFile);  // concatenate "\\AMasker.tip"
  return owl::tstring((_TCHAR*)buffer);
}
//------------------------------------------------------------------------------
//@mfunc CmNextTip()
void TTipDialog::CmNextTip()
{
  GetNextTipString();
  SetDlgItemText(IDC_TIP_STRING, CurrentTip.c_str());
}
//------------------------------------------------------------------------------
//@mfunc CloseTipFile()
long TTipDialog::CloseTipFile()
{
  // But make sure the tips file existed in the first place....
  std::streamoff pos = 0;
  if(TipFile.rdbuf()->is_open()){
    pos = TipFile.tellg();
    TipFile.rdbuf()->close();
  }
  return static_cast<long>(pos);
}
//------------------------------------------------------------------------------
//@mfunc OpenTipFile(long filePos)
void TTipDialog::OpenTipFile(long filePos)
{
  FileName = GetTipName();

  TipFile.open(FileName.Canonical().c_str(), ios::in);

  if(!TipFile.rdbuf()->is_open())
  {
    TipError = true;
    CurrentTip = GetModule()->LoadString(IDS_TIPDLG_ERRFILEABSENT);
  }
  else
  {
    TipFile.seekg(filePos);
    if(TipFile.fail())
    {
      TipError = true;
      MessageBox(IDS_TIPDLG_ERRFILE_CORRUPT, MB_OK);
    }
    else
      GetNextTipString();
  }
}
//------------------------------------------------------------------------------
//@mfunc SetupWindow()
void TTipDialog::SetupWindow()
{
  if(CurrentTip.empty()){
    long pos = ReadSettings();
    OpenTipFile(pos);
  }

  TDialog::SetupWindow();

  SetDlgItemText(IDC_TIP_STRING, CurrentTip.c_str());

  SendDlgItemMessage(IDC_TIP_STARTUP, BM_SETCHECK, ShowDlg ? 1 : 0, 0);

  // If Tips file does not exist then disable NextTip
  if(TipError)
    ::EnableWindow(GetDlgItem(IDC_TIP_NEXT), FALSE);
}
//------------------------------------------------------------------------------
//@mfunc CleanupWindow()
void TTipDialog::CleanupWindow()
{
  ShowDlg = SendDlgItemMessage(IDC_TIP_STARTUP, BM_GETCHECK, 0, 0) == BST_CHECKED;
  long pos = CloseTipFile();
  WriteSettings(pos);

  TDialog::CleanupWindow();
}
//------------------------------------------------------------------------------
//@mfunc ReadSettings()
long TTipDialog::ReadSettings()
{
  TRegKey pTipReg(TRegKey::GetCurrentUser(), RegKey + _T('\\') + szSection);
  long filePos;
  uint32 size = sizeof(long);
  if(pTipReg.QueryValue(szIntFilePos, 0, (uint8*)&filePos, &size) != ERROR_SUCCESS)
    filePos = 0;
  long data = 1;
  size = sizeof(long);
  if(pTipReg.QueryValue(szIntStartup, 0, (uint8*)&data, &size) != ERROR_SUCCESS)
    data = 1;
  ShowDlg = data==1;
  return filePos;
}
//------------------------------------------------------------------------------
//@mfunc WriteSettings(
void TTipDialog::WriteSettings(long filePos)
{
  TRegKey pTipReg(TRegKey::GetCurrentUser(), RegKey + _T('\\') + szSection);
  pTipReg.SetValue(szIntFilePos, (uint32)filePos);
  pTipReg.SetValue(szIntStartup, (uint32)ShowDlg);
}
//------------------------------------------------------------------------------
//@mfunc This routine identifies the next string that needs to be
// read from the tips file
void TTipDialog::GetNextTipString()
{
  _TCHAR buffer[MAX_PATH];
  TipFile.getline(buffer, MAX_PATH);
  while(!TipFile.eof() && buffer[0] == _T(';'))
    TipFile.getline(buffer, MAX_PATH);
  CurrentTip = buffer;

  // If We have either reached EOF or enocuntered some problem
  // In both cases reset the pointer to the beginning of the file
  // This behavior is same as VC++ Tips file
  if(TipFile.eof()){
    // why I can't just  reset pointer to beginning?????
    TipFile.rdbuf()->close();
    TipFile.open(FileName.Canonical().c_str(), ios::in); // ifstream

    if(!TipFile.rdbuf()->is_open()){
      CurrentTip = GetModule()->LoadString(IDS_TIPDLG_ERRFILEABSENT);
      TipError = true;
      return;
    }

    TipFile.getline(buffer, MAX_PATH);
    while(!TipFile.eof() && buffer[0] == _T(';'))
      TipFile.getline(buffer, MAX_PATH);
    CurrentTip = buffer;

  }
  else if(TipFile.fail())
  {
    TipError = true;
    MessageBox(IDS_TIPDLG_ERRFILE_CORRUPT);
  }
}
//------------------------------------------------------------------------------
//@mfunc Handler for EvCtlColor message
//@parm HDC | hDC | Handle to DC
//@parm HWND | hWndChild | Handle Window of control
//@parm uint | ctlType | control ID
//@rdesc HBRUSH
HBRUSH TTipDialog::EvCtlColor (HDC hDC, HWND hWndChild, uint ctlType)
{
  PRECONDITION(ctlType == CTLCOLOR_STATIC);
  if (::GetDlgCtrlID(hWndChild) == IDC_TIP_STRING)
    return (HBRUSH)GetStockObject(WHITE_BRUSH);
  return TDialog::EvCtlColor(hDC, hWndChild, ctlType);
}
//------------------------------------------------------------------------------
TBitmap*
TTipDialog::GetBitmap()
{
  return new TBitmap(*GetModule(), IDB_TIP_BULB);
}
//------------------------------------------------------------------------------
//@mfunc EvPaint
void TTipDialog::EvPaint ()
{
  TPaintDC dc(*this);

  TRect rect;

  // Paint the background white.
  HWND hFrame = GetDlgItem(IDC_TIP_FRAME);
  ::GetClientRect(hFrame, &rect);
  ::MapWindowPoints(hFrame, *this, (TPoint*)&rect, 2); // map to dialog coordinates
  dc.TextRect(rect);

  // Get paint area for the big static control
  ::GetWindowRect(GetDlgItem(IDC_TIP_BULB), &rect);
  ::MapWindowPoints(HWND_DESKTOP, *this, (TPoint*)&rect, 2); // map to dialog coordinates

  // Load bitmap and get dimensions of the bitmap
  if(!BulbBitmap)
    BulbBitmap = GetBitmap();

  // Draw bitmap in top corner and validate only top portion of window
  TMemoryDC bmpDC(*BulbBitmap);
  dc.BitBlt(TRect(rect.TopLeft(),BulbBitmap->Size()), bmpDC, TPoint(0,0));

  // Draw out "Did you know..." message next to the bitmap
  dc.SetTextColor(TColor::LtBlue);
  owl::tstring msg = GetModule()->LoadString(IDS_TIP_DIDYOUKNOW);
  rect.left += BulbBitmap->Width();
  TPoint pt1 = rect.TopLeft() + TPoint(0, BulbBitmap->Height() / 2);
  dc.ExtTextOut(pt1, ETO_OPAQUE, 0, msg, -1);

}
//------------------------------------------------------------------------------
IMPLEMENT_STREAMABLE1(TTipDialog, TDialog);
#if OWL_PERSISTENT_STREAMS

//
// Reads data of the uninitialized TTipDialog from the passed ipstream
//
void*
TTipDialog::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  TTipDialog* o = GetObject();
  ReadVirtualBase((TDialog*)o, is);

  is >> o->ShowDlg;
  is >> o->TipError;
  is >> o->RegKey;

  return o;
}

//
// Writes data of the TFrameWindow to the passed opstream
//
void
TTipDialog::Streamer::Write(opstream& os) const
{
  TTipDialog* o = GetObject();
  WriteVirtualBase((TDialog*)o, os);

  os << o->ShowDlg;
  os << o->TipError;
  os << o->RegKey;
}
#endif

} // OwlExt namespace

/* ========================================================================== */
