//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of Common Dialog abstract base class
//----------------------------------------------------------------------------

#if !defined(OWL_COMMDIAL_H)
#define OWL_COMMDIAL_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/dialog.h>

#if !defined(_INC_COMMDLG)
# include <commdlg.h>
#endif

#include <cderr.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>


/// \addtogroup commdlg
/// @{
/// \class TCommonDialog
// ~~~~~ ~~~~~~~~~~~~~
/// Derived from TDialog, TCommonDialog is the abstract base class for TCommonDialog
/// objects. It provides the basic functionality for creating dialog boxes using the
/// common dialog DLL. The ObjectWindows classes that inherit this common dialog
/// functionality include
/// - \c \b  TChooseColorDialog - a modal dialog box that lets a user select
/// colors for an application.
/// - \c \b  TChooseFontDialog - a modal dialog box that lets a user select
/// fonts for an application.
/// - \c \b  TReplaceDialog a modeless dialog box that lets a user specify a
/// text selection to replace.
/// - \c \b  TFindDialog a modeless dialog box that lets a user specify a text
/// selection to find.
/// - \c \b  TFileOpenDialog - a modal dialog box that lets a user specify a
/// file to open.
/// - \c \b  TFileSaveDialog - a modal dialog box that lets a user specify a
/// file to save.
/// - \c \b  TPrintDialog - a modal dialog box that lets a user specify printer
/// options.
/// 
/// Each common dialog class uses a nested class, TData, that stores the attributes
/// and user-specified data. For example, the TChooseColorDialog::TData class stores
/// the color attributes the user selects in response to a prompt in a common dialog
/// box. In fact, to create a common dialog box, you construct a TData object first,
/// then fill in the data members of the TData object before you even construct the
/// common dialog box object. Finally, you either execute a modal dialog box or
/// create a modeless dialog box.
///
class _OWLCLASS TCommonDialog : public TDialog {
  public:
    TCommonDialog(TWindow* parent, LPCTSTR title = 0, TModule* module = 0);
    TCommonDialog(TWindow* parent, const tstring& title, TModule* = 0);
   ~TCommonDialog();

    virtual int  DoExecute();

  protected:
    const tstring& GetCDTitle() const;

    // Override virtual functions defined by class TWindow
    //
    virtual TPerformCreateReturnType PerformCreate(int deprecated_argument = 0);
    virtual void SetupWindow();

    // Default behavior inline for message response functions
    //
    void CmOkCancel(); //EV_COMMAND(IDOK or IDCANCEL
    void EvClose();    //EV_CLOSE
    void CmHelp();     //EV_COMMAND(pshHelp,

  protected_data:
    tstring CDTitle; ///< Stores the optional caption displayed in the common dialog box.

  private:
    TCommonDialog(const TCommonDialog&);
    TCommonDialog& operator=(const TCommonDialog&);

  DECLARE_RESPONSE_TABLE(TCommonDialog);
  DECLARE_CASTABLE;
};

//
/// \class TCommDlg
// ~~~~~ ~~~~~~~~
/// delay loading COMDLG32.DLL/COMMDLG.DLL
class _OWLCLASS TCommDlg {
  public:
    static  BOOL  ChooseColor(LPCHOOSECOLOR);
    static  BOOL  ChooseFont(LPCHOOSEFONT);
    static  DWORD CommDlgExtendedError();
    static  HWND  FindText(LPFINDREPLACE);
    static  short GetFileTitle(LPCTSTR,LPTSTR,WORD);
    static  BOOL  GetOpenFileName(LPOPENFILENAME);
    static  BOOL  PageSetupDlg(LPPAGESETUPDLG);
#if WINVER >= 0x500
    static  BOOL  PrintDlgEx(LPPRINTDLGEX);
#endif
    static  BOOL  PrintDlg(LPPRINTDLG);
    static  HWND  ReplaceText(LPFINDREPLACE);
    static  BOOL  GetSaveFileName(LPOPENFILENAME);
    static  TModule&  GetModule();
};
/// @}

// Generic definitions/compiler options (eg. alignment) following the 
// definition of classes
#include <owl/posclass.h>



//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Return the title from the common dialog.
//
inline const tstring& TCommonDialog::GetCDTitle() const
{
  return CDTitle;
}

//
/// Responds to a click on the dialog box's OK or Cancel button by calling
/// DefaultProcessing to let the common dialog DLL process the command.
//
inline void TCommonDialog::CmOkCancel()
{
  DefaultProcessing();
}

//
/// Responds to a WM_CLOSE message by calling DefaultProcessing to let the common
/// dialog DLL process the command.
//
inline void TCommonDialog::EvClose()
{
  DefaultProcessing();
}

//
/// Default handler for the pshHelp push button (the Help button in the dialog box).
//
inline void TCommonDialog::CmHelp()
{
  DefaultProcessing();
}

} // OWL namespace


#endif  // OWL_COMMDIAL_H
