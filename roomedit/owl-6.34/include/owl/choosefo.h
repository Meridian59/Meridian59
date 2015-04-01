//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of Choose Font Common Dialog class
//----------------------------------------------------------------------------

#if !defined(OWL_CHOOSEFO_H)
#define OWL_CHOOSEFO_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/commdial.h>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>


class _OWLCLASS TColor;

/// \addtogroup commdlg
/// @{
/// \class TChooseFontDialog
// ~~~~~ ~~~~~~~~~~~~~~~~~
/// Encapsulates the Choose-Font common dialog.
//
/// A TChooseFontDialog represents modal dialog-box interface elements that create a
/// system-defined dialog box from which the user can select a font, a font style
/// (such as bold or italic), a point size, an effect (such as strikeout or
/// underline), and a color. TChooseFontDialog can be made to appear modeless by
/// creating the dialog's parent as an invisible pop-up window and making the pop-up
/// window a child of the main application window. TChooseFontDialog  uses the
/// TChooseFontDialog::TData  structure to initialize the dialog box with the
/// user-selected font styles.
//
class _OWLCLASS TChooseFontDialog : public TCommonDialog {
  public:
  	
/// Defines information necessary to initialize the dialog box with the user's font
/// selection.
    class _OWLCLASS TData 
    {
      public:
      	
/// Flags can be a combination of the following constants that control the
/// appearance and functionality of the dialog box:
/// - \c \b  CF_APPLY	Enables the display and use of the Apply button.
/// - \c \b  CF_ANSIONLY	Specifies that the ChooseFontDialog structure allows only the
/// selection of fonts that use the ANSI character set.
/// - \c \b  CF_BOTH	Causes the dialog box to list both the available printer and screen
/// fonts.
/// - \c \b  CF_EFFECTS	Enables strikeout, underline, and color effects.
/// - \c \b  CF_FIXEDPITCHONLY	Enables fixed-pitch fonts only.
/// - \c \b  CF_FORCEFONTEXIST	Indicates an error if the user selects a nonexistent font or
/// style.
/// - \c \b  CF_INITTOLOGFONTSTRUCT	Uses the LOGFONT structure at which LogFont points to
/// initialize the dialog controls.
/// - \c \b  CF_LIMITSIZE	Limits font selection to those between SizeMin and SizeMax.
/// - \c \b  CF_NOSIMULATIONS	Does not allow GDI font simulations.
/// - \c \b  CF_PRINTERFONTS	Causes the dialog box to list only the fonts supported by the
/// printer that is associated with the device context.
/// - \c \b  CF_SCALABLEONLY	Allows only the selection of scalable fonts.
/// - \c \b  CF_SCREENFONTS	Causes the dialog box to list only the system-supported screen
/// fonts.
/// - \c \b  CF_SHOWHELP	Causes the dialog box to show the Help button.
/// - \c \b  CF_TTONLY	Enumerates and allows the selection of TrueType fonts only.
/// - \c \b  CF_USESTYLE	Specifies that Style points to a buffer containing the style
/// attributes used to initialize the selection of font styles.
/// - \c \b  CF_WYSIWYG	Allows only the selection of fonts available on both the printer and
/// the screen.
        uint32  Flags;
        
/// If the dialog box is successfully executed, Error  returns 0. Otherwise, it
/// contains one of the following codes:
/// - \c \b  CDERR_DIALOGFAILURE	Failed to create a dialog box.
/// - \c \b  CDERR_FINDRESFAILURE	Failed to find a specified resource.
/// - \c \b  CDERR_LOCKRESOURCEFAILURE	Failed to lock a specified resource.
/// - \c \b  CDERR_LOADRESFAILURE	Failed to load a specified resource.
/// - \c \b  CDERR_LOADSTRFAILURE	Failed to load a specified string.
/// - \c \b  CFERR_MAXLESSTHANMIN	The size specified in SizeMax is less than the size in
/// SizeMin.
/// - \c \b  CFERR_NOFONTS	No fonts exist.
        uint32  Error;
        
/// Handle to the device context from which fonts are obtained.
        HDC     DC;
        
/// Attributes of the font.
        LOGFONT LogFont;
        
/// Point size of the font.
        int     PointSize;
        
/// Indicates the font color that is initially selected when the dialog box is
/// created; contains the user's font color selection when the dialog box is closed.
        TColor  Color;
        
/// Style of the font such as bold, italic, underline, or strikeout.
        LPTSTR   Style;
        
/// Font type or name.
        uint16  FontType;
        
/// Minimum size of the font.
        int     SizeMin;
        
/// Maximum size of the font.
        int     SizeMax;
        
    };

    TChooseFontDialog(TWindow*  parent,
                      TData&    data,
                      TResId    templateId = 0,
                      LPCTSTR     title = 0,
                      TModule*  module = 0);
   ~TChooseFontDialog();

  protected:
    TData&  GetData();
    void    SetData(TData& data);

    CHOOSEFONT&   GetCF();
    void    SetCF(const CHOOSEFONT& cf);

    int     DoExecute();
    virtual TDialogProcReturnType DialogFunction(TMsgId, TParam1, TParam2);

    // Default behavior inline for message response functions
    //
    void    CmFontApply();   // EV_COMMAND(psh3...

  protected_data:
/// Contains font attributes. cf is initialized using fields in the
/// TChooseFontDialog::TData structure. It stores the length of the structure, the
/// window that owns the dialog box and the data block that contains the dialog
/// template. It also specifies the dialog-box initialization flags.
    union {
      CHOOSEFONT Cf;  ///< New name
      CHOOSEFONT cf;  ///< old name
    };

/// Data is a reference to the TData object passed in the constructor.
    TData&       Data;

  private:
    TChooseFontDialog(const TChooseFontDialog&);
    TChooseFontDialog& operator=(const TChooseFontDialog&);

  DECLARE_RESPONSE_TABLE(TChooseFontDialog);
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
/// Return the data associated with this dialog.
//
inline TChooseFontDialog::TData& TChooseFontDialog::GetData() {
  return Data;
}

//
/// Set the data associated with this dialog.
/// Use this function with caution!
//
inline void TChooseFontDialog::SetData(TData& data) {
  Data = data;
}

//
/// Return the CHOOSEFONT data structure for this dialog.
//
inline CHOOSEFONT& TChooseFontDialog::GetCF() {
  return Cf;
}

//
/// Set the CHOOSEFONT data structure for this dialog.
//
inline void TChooseFontDialog::SetCF(const CHOOSEFONT& cf) {
  Cf = cf;
}

//
/// User clicked on Apply for the font.
//
/// Default handler for the third push button (the Apply button) in the dialog box.
//
inline void TChooseFontDialog::CmFontApply() {
  DefaultProcessing();
}



} // OWL namespace


#endif  // OWL_CHOOSEFO_H
