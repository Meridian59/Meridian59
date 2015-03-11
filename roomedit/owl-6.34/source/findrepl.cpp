//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of abstract class TFindReplaceDialog, and leaf classes
/// TFindDialog and TReplaceDialog that encapsulate Common Dialogs
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/findrepl.h>

#if !defined(ctlFirst)
# include <dlgs.h>
#endif

namespace owl {

OWL_DIAGINFO;

//
/// Constructs a TData object with the specified flag value that initializes the
/// status of the dialog box control buttons and the buffer size for the find and
/// replace search strings.
//
TFindReplaceDialog::TData::TData(uint32 flags, int buffSize)
:
  Flags(flags), BuffSize(buffSize), Error(0)
{
  FindWhat = new tchar[BuffSize];
  ReplaceWith = new tchar[BuffSize];
  *FindWhat = *ReplaceWith = 0;
}

//
//
//
TFindReplaceDialog::TData::TData(const TData& src)
:
  Flags(src.Flags),
  BuffSize(src.BuffSize),
  Error(0)
{
  FindWhat = strnewdup(src.FindWhat, BuffSize);
  ReplaceWith = strnewdup(src.ReplaceWith, BuffSize);
}


//
/// Destroys a ~TData object.
//
TFindReplaceDialog::TData::~TData()
{
  delete[] FindWhat;
  delete[] ReplaceWith;
}

//
//
//
TFindReplaceDialog::TData&
TFindReplaceDialog::TData::operator =(const TData& src)
{
  Flags = src.Flags;
  BuffSize = src.BuffSize;
  Error = 0;

  delete[] FindWhat;
  FindWhat = strnewdup(src.FindWhat, BuffSize);

  delete[] ReplaceWith;
  ReplaceWith = strnewdup(src.ReplaceWith, BuffSize);

  return *this;
}


//----------------------------------------------------------------------------

DEFINE_RESPONSE_TABLE1(TFindReplaceDialog, TCommonDialog)
  EV_WM_NCDESTROY,
END_RESPONSE_TABLE;

//
/// Used by constructors in derived classes, Init initializes a TFindReplaceDialog
/// object with the current resource ID and other members.
//
void
TFindReplaceDialog::Init(TResId templateId)
{
  memset(&fr, 0, sizeof(FINDREPLACE));
  fr.lStructSize = sizeof(FINDREPLACE);
  fr.hwndOwner = GetParentO() ? GetParentO()->GetHandle() : 0;
  fr.hInstance = *GetModule();
  Data.Flags &= ~(FR_FINDNEXT|FR_REPLACE|FR_REPLACEALL|FR_DIALOGTERM);
  fr.Flags = FR_ENABLEHOOK | Data.Flags;
  if (templateId) {
    fr.lpTemplateName = templateId;
    fr.Flags |= FR_ENABLETEMPLATE;
  }
  else
    fr.Flags &= ~FR_ENABLETEMPLATE;

  fr.lpstrFindWhat = Data.FindWhat;
  fr.wFindWhatLen = (uint16)Data.BuffSize;
  fr.lpstrReplaceWith = Data.ReplaceWith;
  fr.wReplaceWithLen = (uint16)Data.BuffSize;
}

//
/// Constructs a TFindReplaceDialog object with a parent window, resource ID, and
/// caption. Sets the attributes of the dialog box with the specified data from the 
/// TFindReplaceDialog::TData  structure.
//
TFindReplaceDialog::TFindReplaceDialog(TWindow* parent, TData& data, TResId templateId,
                                       LPCTSTR title, TModule* module)
:
  TCommonDialog(parent, title, module),
  Data(data)
{
  Init(templateId);
}

//
/// String-aware overload
//
TFindReplaceDialog::TFindReplaceDialog(TWindow* parent, TData& data, TResId templateId, const tstring& title, TModule* module)
:
  TCommonDialog(parent, title, module),
  Data(data)
{
  Init(templateId);
}

//
//
//
TWindow::TPerformCreateReturnType 
TFindReplaceDialog::PerformCreate(int arg)
{
  PRECONDITIONX(!(OWL_STRICT && arg), _T("The deprecated argument to PerformCreate is disallowed.")); InUse(arg);
  OWL_SET_OR_RETURN_HANDLE(GetHandle());
}

//
/// Returns true if a message is handled.
//
TDialog::TDialogProcReturnType
TFindReplaceDialog::DialogFunction(TMsgId msg, TParam1 param1, TParam2 param2)
{
  return TCommonDialog::DialogFunction(msg, param1, param2);
}

//
/// Calls TWindow::EvNCDestroy, which responds to an incoming EV_WM_NCDESTROY
/// message which tells the owner window that is nonclient area is being destroyed.
///
/// Make sure flags get copied over before we go
//
void
TFindReplaceDialog::EvNCDestroy()
{
  Data.Flags = fr.Flags;
  TWindow::EvNCDestroy();
}

//
/// Updates the flags from the passed-in parameter. Assumes the parameters is a
/// pointer to a FINDREPLACE structure.
//
void
TFindReplaceDialog::UpdateData(TParam2 param2)
{
  if (param2)
    Data.Flags = ((LPFINDREPLACE)param2)->Flags;
  else
    Data.Flags = fr.Flags;
}

//----------------------------------------------------------------------------

//
/// Constructs a TFindDialog object with the given parent window, resource ID, and
/// caption. Sets the attributes of the dialog box based onTFindReplaceDialog::TData
/// structure, which contains information about the text string to search for.
//
TFindDialog::TFindDialog(TWindow* parent, TData& data, TResId templateId,
                         LPCTSTR  title, TModule* module)
:
  TFindReplaceDialog(parent, data, templateId, title, module)
{
}

//
/// String-aware overload
//
TFindDialog::TFindDialog(TWindow* parent, TData& data, TResId templateId, const tstring& title, TModule* module)
:
  TFindReplaceDialog(parent, data, templateId, title, module)
{}

//
//
//
TWindow::TPerformCreateReturnType 
TFindDialog::PerformCreate(int arg)
{
  PRECONDITIONX(!(OWL_STRICT && arg), _T("The deprecated argument to PerformCreate is disallowed.")); InUse(arg);
  fr.lpfnHook = LPFRHOOKPROC(StdDlgProc);
  THandle h = TCommDlg::FindText(&fr);
  OWL_SET_OR_RETURN_HANDLE(h);
}

//----------------------------------------------------------------------------

//
/// Constructs a TReplaceDialog object with a parent window, resource ID, template
/// name, caption, and module instance. The data  parameter is a reference to the
/// TFindReplaceDialog::TData class that contains information about the appearance
/// and functionality of the dialog box, such as the user-entered text strings to
/// search for and replace.
//
TReplaceDialog::TReplaceDialog(TWindow* parent, TData& data, TResId templateId,
                               LPCTSTR title, TModule* module)
:
  TFindReplaceDialog(parent, data, templateId, title, module)
{
}

//
/// String-aware overload
//
TReplaceDialog::TReplaceDialog(TWindow* parent, TData& data, TResId templateId, const tstring& title, TModule* module)
:
  TFindReplaceDialog(parent, data, templateId, title, module)
{}

//
//
//
TWindow::TPerformCreateReturnType 
TReplaceDialog::PerformCreate(int arg)
{
  PRECONDITIONX(!(OWL_STRICT && arg), _T("The deprecated argument to PerformCreate is disallowed.")); InUse(arg);
  fr.lpfnHook = LPFRHOOKPROC(StdDlgProc);
  THandle h = TCommDlg::ReplaceText(&fr);
  OWL_SET_OR_RETURN_HANDLE(h);
}

//Keep streaming out if not used

//
//
//
void
TFindReplaceDialog::TData::Read(ipstream& is)
{
  is >> Flags;
  is >> BuffSize;
  delete[] FindWhat;
  delete[] ReplaceWith;
  FindWhat = new tchar[BuffSize];
  ReplaceWith = new tchar[BuffSize];
  is.readBytes(FindWhat, BuffSize);
  is.readBytes(ReplaceWith, BuffSize);
}

//
//
//
void
TFindReplaceDialog::TData::Write(opstream& os)
{
  os << Flags;
  os << BuffSize;
  os.writeBytes(FindWhat, BuffSize);
  os.writeBytes(ReplaceWith, BuffSize);
}



} // OWL namespace

