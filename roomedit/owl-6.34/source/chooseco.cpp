//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TChooseColorDialog, a Choose Color Common Dialog class
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/chooseco.h>

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlCommDialog);  // diagnostic group for common dialogs

uint TChooseColorDialog::SetRGBMsgId = 0;

DEFINE_RESPONSE_TABLE1(TChooseColorDialog, TCommonDialog)
END_RESPONSE_TABLE;

//
/// Initialize the choose color common dialog data members.
//
/// Constructs a dialog box with specified parent window, data, resource identifier,
/// window caption, and module ID. Sets the attributes of the dialog box based on
/// info in the TChooseColor::TData  structure.
//
TChooseColorDialog::TChooseColorDialog(TWindow*        parent,
                                       TData&          data,
                                       TResId          templateId,
                                       LPCTSTR         title,
                                       TModule*        module)
:
  TCommonDialog(parent, title, module),
  Data(data)
{
  Init(templateId);
}

//
/// String-aware overload
//
TChooseColorDialog::TChooseColorDialog(TWindow* parent, TData& data, TResId templateId, const tstring& title, TModule* module)
: TCommonDialog(parent, title, module),
  Data(data)
{
  Init(templateId);
}

void TChooseColorDialog::Init(TResId templateId)
{
  if (!SetRGBMsgId)
    SetRGBMsgId = ::RegisterWindowMessage(SETRGBSTRING);

  memset(&Cc, 0, sizeof Cc);
  Cc.lStructSize = sizeof Cc;
  Cc.hwndOwner = GetParentO() ? GetParentO()->GetHandle() : 0;
  Cc.hInstance = HWND(GetModule()->GetHandle());  // hInstance is badly typed
  Cc.Flags = CC_ENABLEHOOK | Data.Flags;
  if (templateId) {
    Cc.lpTemplateName = templateId;
    Cc.Flags |= CC_ENABLETEMPLATE;
  }
  else
    Cc.Flags &= ~CC_ENABLETEMPLATE;
  Cc.lpfnHook = 0;

  Cc.rgbResult = Data.Color;
  Cc.lpCustColors = (COLORREF *)Data.CustColors;

  TRACEX(OwlCommDialog, OWL_CDLEVEL, "TChooseColorDialog constructed @" << (void*)this);
}

//
/// Destructor does nothing in non diagnostic versions of the library.
/// In the diagnostic version, it displays a trace message.
//
TChooseColorDialog::~TChooseColorDialog()
{
  TRACEX(OwlCommDialog, OWL_CDLEVEL, "TChooseColorDialog destructed @" << (void*)this);
}

//
/// Override the virtual DialogFunction.
/// Does no additional processing.
///
/// Returns true if a message is handled.
//
TDialog::TDialogProcReturnType
TChooseColorDialog::DialogFunction(TMsgId msg, TParam1 param1, TParam2 param2)
{
  return TCommonDialog::DialogFunction(msg, param1, param2);
}

//
/// Execute the dialog to retrieve user's choice of color.
//
/// If no error occurs, DoExecute copies flags and colors into Data and returns
/// zero. If an error occurs, DoExecute returns the IDCANCEL with Data.Error set to
/// the value returned from CommDlgExtendedError.
//
int
TChooseColorDialog::DoExecute()
{
  Cc.lpfnHook = LPCCHOOKPROC(StdDlgProc);
  int ret = TCommDlg::ChooseColor(&Cc);
  if (ret) {
    Data.Flags = Cc.Flags;
    Data.Error = 0;
    Data.Color = Cc.rgbResult;
  }
  else {
    Data.Error = TCommDlg::CommDlgExtendedError();
  }
  return ret ? IDOK : IDCANCEL;
}

TChooseColorDialog::TData::TData(const TData& other) 
{
  operator =(other);
}

TChooseColorDialog::TData& TChooseColorDialog::TData::operator =(const TData& other)
{
  Flags = other.Flags;
  Error = other.Error;
  Color = other.Color;

  // Copy the custom colors owned by 'other', if any.
  //
  CustColorsArray = other.CustColorsArray; 

  // If 'other' is using its custom colors, 'this' will use its custom colors too.
  // Otherwise, both will use the same external custom colors.
  //
  bool custColorsArrayInUse = (!other.CustColorsArray.empty() && other.CustColors == &other.CustColorsArray[0]);
  CustColors = custColorsArrayInUse ? &CustColorsArray[0] : other.CustColors;

  return *this;
} 

} // OWL namespace
///////////////////////////////////////////

