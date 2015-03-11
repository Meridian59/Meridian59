//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TChooseFontDialog, a Choose Font Common Dialog class
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/choosefo.h>

#if !defined(ctlFirst)
# include <dlgs.h>
#endif

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlCommDialog);  // diagnostic group for common dialogs

DEFINE_RESPONSE_TABLE1(TChooseFontDialog, TCommonDialog)
END_RESPONSE_TABLE;

//
/// Initialize the ChooseFont data members.
//
/// Constructs a dialog box with specified data, parent window, resource identifier,
/// window caption, and module ID. Sets the attributes of the dialog box based on
/// the font information in the TChooseFontDialog::TData  structure.
//
TChooseFontDialog::TChooseFontDialog(TWindow*        parent,
                                     TData&          data,
                                     TResId          templateId,
                                     LPCTSTR         title,
                                     TModule*        module)
:
  TCommonDialog(parent, title, module),
  Data(data)
{
  memset(&Cf, 0, sizeof(CHOOSEFONT));
  Cf.lStructSize = sizeof(CHOOSEFONT);
  Cf.hwndOwner = GetParentO() ? GetParentO()->GetHandle() : 0;
  Cf.hInstance = *GetModule();
  Cf.Flags = CF_ENABLEHOOK | Data.Flags;
  if (templateId) {
    Cf.lpTemplateName = templateId;
    Cf.Flags |= CF_ENABLETEMPLATE;
  }
  else
    Cf.Flags &= ~CF_ENABLETEMPLATE;
  Cf.lpfnHook = 0;

  Cf.hDC = Data.DC;
  Cf.lpLogFont = &Data.LogFont;
  Cf.iPointSize = Data.PointSize;
  Cf.rgbColors = Data.Color;
  Cf.lpszStyle = Data.Style;
  Cf.nFontType = Data.FontType;
  Cf.nSizeMin = Data.SizeMin;
  Cf.nSizeMax = Data.SizeMax;

  TRACEX(OwlCommDialog, OWL_CDLEVEL, "TChooseFontDialog constructed @" << (void*)this);
}


//
/// Destructor for this object does nothing in the non-diagnostic versions
/// of the library.
/// In the diagnostic version, it produces a trace message.
//
TChooseFontDialog::~TChooseFontDialog()
{
  TRACEX(OwlCommDialog, OWL_CDLEVEL, "TChooseFontDialog destructed @" << (void*)this);
}


//
/// Override the virtual DialogFunction.
/// It does no additional processing.
//
/// Returns true if a dialog box message is handled.
//
TDialog::TDialogProcReturnType
TChooseFontDialog::DialogFunction(TMsgId msg, TParam1 param1, TParam2 param2)
{
  return TCommonDialog::DialogFunction(msg, param1, param2);
}

//
/// Execute the dialog to retrieve the font selected by the user.
//
/// If no error occurs, DoExecute copies the flag values and font information into
/// Ddata, and returns IDOK or IDCANCEL. If an error occurs, DoExecute returns an
/// error code from TChooseFontDialog::TData structure's  Error  data member.
//
int
TChooseFontDialog::DoExecute()
{
  Cf.lpfnHook = LPCFHOOKPROC(StdDlgProc);
  int ret = TCommDlg::ChooseFont(&Cf);
  if (ret) {
    Data.Flags = Cf.Flags;
    Data.Error = 0;
    Data.PointSize = Cf.iPointSize;
    Data.Color = Cf.rgbColors;
    Data.Style = Cf.lpszStyle;
    Data.FontType = Cf.nFontType;
  }
  else 
    Data.Error = TCommDlg::CommDlgExtendedError();
  
  return ret ? IDOK : IDCANCEL;
}

} // OWL namespace
//////////////////////////////////////////////

