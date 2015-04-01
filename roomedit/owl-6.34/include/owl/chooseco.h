//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of Choose Color Common Dialog class
//----------------------------------------------------------------------------

#if !defined(OWL_CHOOSECO_H)
#define OWL_CHOOSECO_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/commdial.h>
#include <owl/color.h>
#include <vector>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup commdlg
/// @{
/// \class TChooseColorDialog
// ~~~~~ ~~~~~~~~~~~~~~~~~~
/// Wrapper for the Choose-Color common dialog.
//
/// TChooseColorDialog objects represent modal dialog box interface elements that
/// allow color selection and custom color adjustment. TChooseColorDialog can be
/// made to appear modeless to the user by creating the dialog's parent as an
/// invisible pop-up window and making the pop-up window a child of the main
/// application window. TChooseColorDialog uses the TChooseColor::TData struct to
/// initialize the dialog box with the user's color selection.
//
class _OWLCLASS TChooseColorDialog : public TCommonDialog {
  public:
  	
/// Defines information necessary to initialize the dialog box with the user's color
/// selection.
    class _OWLCLASS TData {
      public:
        TData(uint32 flags = 0, bool allocCustColor = false);
        TData(uint32 flags, const TColor& initColor, 
              bool allocCustColors = false);

        TData(const TData& other);
        TData& operator =(const TData& other);

/// Flags can be a combination of the following values that control the appearance
/// and functionality of the dialog box:
/// - \c \b  CC_FULLOPEN	Causes the entire dialog box to appear when the dialog box is
/// created.
/// - \c \b  CC_PREVENTFULLOPEN	Disables the "Define Custom Colors" push button.
/// - \c \b  CC_RGBINIT	Causes the dialog box to use the color specified in rgbResult as the
/// initial color selection.
/// - \c \b  CC_SHOWHELP	Causes the dialog box to show the Help push button.
        uint32              Flags;
        
///  If the dialog box is successfully executed,  Error  is 0. Otherwise, it
/// contains one of the following codes:
/// - \c \b  CDERR_DIALOGFAILURE	Failed to create a dialog box.
/// - \c \b  CDERR_FINDRESFAILURE	Failed to find a specified resource.
/// - \c \b  CDERR_LOADRESFAILURE	Failed to load a specified resource.
/// - \c \b  CDERR_LOCKRESOURCEFAILURE	Failed to lock a specified resource.
/// - \c \b  CDERR_LOADSTRFAILURE	Failed to load a specified string.
        uint32              Error;
        
/// Specifies the color that is initially selected when the dialog box is created.
/// Contains the user's color selection when the dialog box is closed.
        TColor              Color;
        
/// Points to an array of 16 colors.
        TColor*             CustColors;

      protected:
        std::vector<TColor> CustColorsArray;  
    };

    TChooseColorDialog(TWindow* parent,
                       TData&   data,
                       TResId   templateId = 0,
                       LPCTSTR   title = 0,
                       TModule* module = 0);

    TChooseColorDialog(TWindow* parent, TData& data, TResId templateId, const tstring& title, TModule* = 0);

   ~TChooseColorDialog();

    // Set the current RGB color in this dialog
    //
    void    SetRGBColor(const TColor& color);

  protected:
    void Init(TResId templateId);
    TData&  GetData();
    void    SetData(const TData& data);

    CHOOSECOLOR&  GetCC();
    void   SetCC(const CHOOSECOLOR& cc);

    int     DoExecute();
    virtual TDialogProcReturnType DialogFunction(TMsgId, TParam1, TParam2);

    TResult  EvSetRGBColor(TParam1, TParam2);      // EV_REGISTERED(SETRGBSTRING,

    /// Registered messages this class sends (to itself)
    //
    static uint SetRGBMsgId;

  protected_data:
/// Stores the length of the TChooseColorDialog structure, the window that owns the
/// dialog box, and the data block that contains the dialog template. It also points
/// to an array of 16 RGB values for the custom color boxes in the dialog box, and
/// specifies the dialog-box initialization flags.
    union {
      CHOOSECOLOR  Cc;  ///< New name
      CHOOSECOLOR  cc;  ///< Old name
    };
    
/// Data is a reference to the TData object passed in the constructor.
    TData&       Data;

  private:
    TChooseColorDialog(const TChooseColorDialog&);
    TChooseColorDialog& operator=(const TChooseColorDialog&);

  DECLARE_RESPONSE_TABLE(TChooseColorDialog);
  DECLARE_CASTABLE;
};
/// @}


// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Sets the current RGB color for the open dialog box by sending a SetRGBMsgId. You
/// can use SetRGBColor() to send a message to change the current color selection.
//
inline void TChooseColorDialog::SetRGBColor(const TColor& color) {
  SendMessage(SetRGBMsgId, 0, color);
}

//
/// Return the data object for this common dialog.
//
inline TChooseColorDialog::TData& TChooseColorDialog::GetData() {
  return Data;
}

//
/// Sets the data for this common dialog.
//
inline void TChooseColorDialog::SetData(const TData& data) {
  Data = data;
}

//
/// Return the CHOOSECOLOR data structure for this dialog.
//
inline CHOOSECOLOR& TChooseColorDialog::GetCC() {
  return Cc;
}

//
/// Set the CHOOSECOLOR data structure for this dialog.
/// Use this function with caution!
//
inline void TChooseColorDialog::SetCC(const CHOOSECOLOR& cc) {
  Cc = cc;
}

//
/// The user has clicked on a color.
//
/// Responds to the message sent by SetRGBColor by forwarding the to the original
/// class. This event handler is not in the response table.
//
inline TResult TChooseColorDialog::EvSetRGBColor(TParam1, TParam2) {
  return DefaultProcessing();
}

//
//
//
inline TChooseColorDialog::TData::TData(uint32 flags, bool allocCustColor) {
  Flags = flags;
  if (allocCustColor) 
  {
    CustColorsArray.resize(16);
    CustColors = &CustColorsArray[0];
  }
  else
  {
    CustColors = 0;
  }
}

//
//
//
inline TChooseColorDialog::TData::TData(uint32 flags, 
                                        const TColor& initColor, 
                                        bool allocCustColor) {
  Flags = flags;
  Color = initColor;  
  if (allocCustColor) 
  {
    CustColorsArray.resize(16);
    CustColors = &CustColorsArray[0];
  }
  else
  {
    CustColors = 0;
  }
}


} // OWL namespace

#endif  // OWL_CHOOSECO_H
