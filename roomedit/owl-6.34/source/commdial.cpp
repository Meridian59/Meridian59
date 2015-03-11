//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of Common Dialog abstract base class
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/commdial.h>
#include <owl/applicat.h>

#if defined(__BORLANDC__)
# pragma option -w-ccc // Disable "Condition is always true/false"
#endif

namespace owl {

// Diagnostic group for Common Dialog
OWL_DIAGINFO;
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlCommDialog, 1, 0);

//
/// \class TCommDlg
// ~~~~~ ~~~~~~~
/// delay loading COMDLG32.DLL/COMMDLG.DLL
/////////////////////////////////////////////////////////////
  static const tchar comdlgStr[]            = _T("COMDLG32.DLL");
  static const char CommDlgExtendedErrorStr[]  = "CommDlgExtendedError";
# if defined(UNICODE)
    static const char ChooseColorStr[]    = "ChooseColorW";
    static const char ChooseFontStr[]      = "ChooseFontW";
    static const char FindTextStr[]        = "FindTextW";
    static const char GetFileTitleStr[]    = "GetFileTitleW";
    static const char GetOpenFileNameStr[]= "GetOpenFileNameW";
    static const char PageSetupDlgStr[]    = "PageSetupDlgW";
    static const char PrintDlgStr[]        = "PrintDlgW";
    static const char PrintDlgExStr[]        = "PrintDlgExW";
    static const char ReplaceTextStr[]    = "ReplaceTextW";
    static const char GetSaveFileNameStr[]= "GetSaveFileNameW";
# else
    static const char ChooseColorStr[]    = "ChooseColorA";
    static const char ChooseFontStr[]      = "ChooseFontA";
    static const char FindTextStr[]        = "FindTextA";
    static const char GetFileTitleStr[]    = "GetFileTitleA";
    static const char GetOpenFileNameStr[]= "GetOpenFileNameA";
    static const char PageSetupDlgStr[]    = "PageSetupDlgA";
    static const char PrintDlgStr[]        = "PrintDlgA";
    static const char PrintDlgExStr[]      = "PrintDlgExA";
    static const char ReplaceTextStr[]    = "ReplaceTextA";
    static const char GetSaveFileNameStr[]= "GetSaveFileNameA";
# endif

//
/// Returns TModule object wrapping the handle of the WINMM.DLL module
//
TModule&
TCommDlg::GetModule()
{
  static TModule comdlgModule(comdlgStr, true, true, false);
  return comdlgModule;
}

//
/// Invokes 'ChooseColor' indirectly
//
BOOL  
TCommDlg::ChooseColor(LPCHOOSECOLOR p1)
{
  static TModuleProc1<BOOL,LPCHOOSECOLOR>
         chooseColor(GetModule(), ChooseColorStr);
  return chooseColor(p1);
}
//
/// Invokes 'ChooseFont' indirectly
//
BOOL  
TCommDlg::ChooseFont(LPCHOOSEFONT p1)
{
  static TModuleProc1<BOOL,LPCHOOSEFONT>
         chooseFont(GetModule(), ChooseFontStr);
  return chooseFont(p1);
}
//
/// Invokes 'CommDlgExtendedError' indirectly
//
DWORD 
TCommDlg::CommDlgExtendedError()
{
  static TModuleProc0<DWORD>
         commDlgExtendedError(GetModule(), CommDlgExtendedErrorStr);
  return commDlgExtendedError();
}
//
/// Invokes 'FindText' indirectly
//
HWND  
TCommDlg::FindText(LPFINDREPLACE p1)
{
  static TModuleProc1<HWND,LPFINDREPLACE>
         findText(GetModule(), FindTextStr);
  return findText(p1);
}
//
/// Invokes 'GetFileTitle' indirectly
//
short 
TCommDlg::GetFileTitle(LPCTSTR p1, LPTSTR p2, WORD p3)
{
  static TModuleProc3<short,LPCTSTR,LPTSTR,WORD>
         getFileTitle(GetModule(), GetFileTitleStr);
  return getFileTitle(p1,p2,p3);
}
//
/// Invokes 'GetOpenFileName' indirectly
//
BOOL  
TCommDlg::GetOpenFileName(LPOPENFILENAME p1)
{
#if defined(UNIX)
  return ::GetOpenFileName(p1);
#else
  static TModuleProc1<BOOL,LPOPENFILENAME>
         getOpenFileName(GetModule(), GetOpenFileNameStr);
  return getOpenFileName(p1);
#endif
}
//
/// Invokes 'PageSetupDlg' indirectly
//
BOOL  
TCommDlg::PageSetupDlg(LPPAGESETUPDLG p1)
{
  static TModuleProc1<BOOL,LPPAGESETUPDLG>
         pageSetupDlg(GetModule(), PageSetupDlgStr);
  return pageSetupDlg(p1);
}
#if(WINVER >= 0x0500)
//
/// Invokes 'PrintDlgEx' indirectly
//
BOOL
TCommDlg::PrintDlgEx(LPPRINTDLGEX p1)
{
  static TModuleProc1<BOOL,LPPRINTDLGEX>
         printDlg(GetModule(), PrintDlgExStr);
  return printDlg(p1);
}
#endif // #if(WINVER >= 0x0500)
//
/// Invokes 'PrintDlg' indirectly
//
BOOL  
TCommDlg::PrintDlg(LPPRINTDLG p1)
{
  static TModuleProc1<BOOL,LPPRINTDLG>
         printDlg(GetModule(), PrintDlgStr);
  return printDlg(p1);
}
//
/// Invokes 'ReplaceText' indirectly
//
HWND  
TCommDlg::ReplaceText(LPFINDREPLACE p1)
{
  static TModuleProc1<HWND,LPFINDREPLACE>
         replaceText(GetModule(), ReplaceTextStr);
  return replaceText(p1);
}
//
/// Invokes 'GetSaveFileName' indirectly
//
BOOL
TCommDlg::GetSaveFileName(LPOPENFILENAME p1)
{
#if defined(UNIX)
  return ::GetSaveFileName(p1);
#else
  static TModuleProc1<BOOL,LPOPENFILENAME>
         getSaveFileName(GetModule(), GetSaveFileNameStr);
  return getSaveFileName(p1);
#endif
}
/////////////////////////////////////////////////////////////////////////////////


DEFINE_RESPONSE_TABLE1(TCommonDialog, TDialog)
  EV_WM_CLOSE,
  EV_COMMAND(IDOK, CmOkCancel),
  EV_COMMAND(IDCANCEL, CmOkCancel),
END_RESPONSE_TABLE;

//
/// Invokes a TWindow constructor, passing the parent window parent, and constructs
/// a common dialog box that you can modify to suit your specifications. You can
/// indicate the module ID (module) and window caption (title), which otherwise
/// default to 0 and blank, respectively.
//
TCommonDialog::TCommonDialog(TWindow*  parent,
                             LPCTSTR   title,
                             TModule*  module)
:
  TDialog(parent, 0, module),
  CDTitle(title ? title : _T(""))
{
  TRACEX(OwlCommDialog, OWL_CDLEVEL, "TCommonDialog constructed @" << (void*)this);
}

//
/// String-aware overload
//
TCommonDialog::TCommonDialog(TWindow*  parent, const tstring& title, TModule*  module)
  : TDialog(parent, title.c_str(), module),
  CDTitle(title)
{
  TRACEX(OwlCommDialog, OWL_CDLEVEL, "TCommonDialog constructed @" << (void*)this);
}

//
/// Generate message in diagnostic libraries.
//
TCommonDialog::~TCommonDialog()
{
  TRACEX(OwlCommDialog, OWL_CDLEVEL, "TCommonDialog destructed @" << (void*)this);
}

//
/// Assigns the caption of the dialog box to CDTitle if CDTitle is non-blank.
///
/// Override virtual to set the caption.
//
void
TCommonDialog::SetupWindow()
{
  TRACEX(OwlCommDialog, 1, "TCommonDialog::SetupWindow() @" << (void*)this);

  TDialog::SetupWindow();
  if (!CDTitle.empty())
    SetCaption(CDTitle);
}

//
/// Default handler for a modeless common dialog.
/// Wrong usage if here.
//
TWindow::TPerformCreateReturnType 
TCommonDialog::PerformCreate(int)
{
  PRECONDITIONX(false, _T("Wrong usage for a modal common dialog. Use Execute() instead."));
  OWL_SET_OR_RETURN_HANDLE(GetHandle());
}

//
/// Default handler for a modal common dialog.
/// Wrong usage if here.
//
/// Called by Execute, DoExecute executes a modal dialog box. If the dialog box
/// execution is canceled or unsuccessful, DoExecute returns IDCANCEL.
//
int TCommonDialog::DoExecute()
{
  TRACEX(OwlCommDialog, 0, "Wrong usage for a modeless common dialog. Use Create() instead.");
  return IDCANCEL;
}

} // OWL namespace
/* ========================================================================== */
