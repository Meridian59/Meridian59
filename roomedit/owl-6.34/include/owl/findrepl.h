//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of FindReplace- abstract, Find-, Replace- common Dialog classes
//----------------------------------------------------------------------------

#if !defined(OWL_FINDREPL_H)
#define OWL_FINDREPL_H

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

/// \addtogroup commdlg
/// @{

/// \class TFindReplaceDialog
// ~~~~~ ~~~~~~~~~~~~~~~~~~
/// TFindReplaceDialog is an abstract base class for a modeless dialog box that lets
/// you search for and replace text. This base class contains functionality common
/// to both derived classes, TFindDialog (lets you specify text to find) and
/// TReplaceDialog (lets you specify replacement text). TFindReplaceDialog
/// communicates with the owner window using a registered message.
//
class _OWLCLASS TFindReplaceDialog : public TCommonDialog {
  public:
 
		/// The TFindReplaceDialog::TData class encapsulates information necessary to
		/// initialize a TFindReplace dialog box. The TFindDialog and TReplaceDialog classes
		/// use the TFindReplaceDialog::TData  class to initialize the dialog box and to
		/// accept user-entered options such as the search and replacement text strings.
    class _OWLCLASS TData {
      public:
				/// Flags, which indicates the state of the control buttons and the action that
				/// occurred in the dialog box, can be a combination of the following constants that
				/// indicate which command the user wants to select:
				/// - \c \b  FR_DOWN	The Down button in the Direction group of the Find dialog box is
				/// selected.
				/// - \c \b  FR_HIDEMATCHCASE	The Match Case check box is hidden.
				/// - \c \b  FR_HIDEWHOLEWORD	The Whole Word check box is hidden.
				/// - \c \b  FR_HIDEUPDOWN.	The Up and Down buttons are hidden.
				/// - \c \b  FR_MATCHCASE	The Match Case check box is checked.
				/// - \c \b  FR_NOMATCHCASE	The Match Case check box is disabled. This occurs when the
				/// dialog box is first initialized.
				/// - \c \b  FR_NOUPDOWN	The Up and Down buttons are disabled. This occurs when the dialog
				/// box is first initialized.
				/// - \c \b  FR_NOWHOLEWORD	The Whole Word check box is disabled. This occurs when the
				/// dialog box is first initialized.
				/// - \c \b  FR_REPLACE	The Replace button was pressed in the Replace dialog box.
				/// - \c \b  FR_REPLACEALL	The Replace All button was pressed in the Replace dialog box.
				/// - \c \b  FR_WHOLEWORD	The Whole Word check box is checked.
        uint32   Flags;
        
				/// BuffSize contains the size of the text buffer.
        int      BuffSize;

				/// If the dialog box is successfully created, Error  is 0. Otherwise, it contains
				/// one or more of the following error codes:
				/// - \c \b  CDERR_LOCKRESOURCEFAILURE	Failed to lock a specified resource.
				/// - \c \b  CDERR_LOADRESFAILURE	Failed to load a specified resource.
				/// - \c \b  CDERR_LOADSTRFAILURE	Failed to load a specified string.
				/// - \c \b  CDERR_REGISTERMSGFAIL	The window message (a value used to communicate between
				/// applications) cannot be registered. This message value is used when sending or
				/// posting window messages.
        uint32   Error;
        
				/// Contains the search string.
        LPTSTR   FindWhat;
        
				/// ReplaceWith contains the replacement string.
        LPTSTR   ReplaceWith;

        TData(uint32 flags = 0, int buffSize = 81);
        TData(const TData& src);
       ~TData();

        TData& operator =(const TData& src);

        void     Write(opstream& os);
        void     Read(ipstream& is);
    };

    TFindReplaceDialog(TWindow*        parent,
                       TData&          data,
                       TResId          templateId = 0,
                       LPCTSTR title = 0,
                       TModule*        module = 0);

    TFindReplaceDialog(
      TWindow* parent,
      TData& data,
      TResId templateId,
      const tstring& title,
      TModule* = 0);

    void         UpdateData(TParam2 param = 0);

  protected:

    virtual void Init(TResId templateId);
    virtual TPerformCreateReturnType PerformCreate(int deprecated_argument = 0);
    virtual TDialogProcReturnType DialogFunction(TMsgId, TParam1, TParam2);

    TData&   GetData();
    void     SetData(TData& data);

    FINDREPLACE&  GetFR();
    void   SetFR(const FINDREPLACE& _fr);

    // Default behavior inline for message response functions
    //
    void     CmFindNext();   ///< EV_COMMAND(IDOK,
    void     CmReplace();    ///< EV_COMMAND(psh1,
    void     CmReplaceAll(); ///< EV_COMMAND(psh2,
    void     CmCancel();     ///< EV_COMMAND(IDCANCEL,

    void     EvNCDestroy();

  protected_data:
/// A struct that contains find-and-replace attributes, such as the size of the find
/// buffer and pointers to search and replace strings, used for find-and-replace
/// operations.
    FINDREPLACE  fr;
    
/// Data is a reference to the TData object passed in the constructor.
    TData&       Data;

  DECLARE_RESPONSE_TABLE(TFindReplaceDialog);
  DECLARE_CASTABLE;
};

//
/// \class TFindDialog
// ~~~~~ ~~~~~~~~~~~
/// TFindDialog objects represents modeless dialog box interface elements that let
/// you specify text to find. TFindDialog communicates with the owner window using a
/// registered message. Derived from  TFindReplaceDialog , TFindDialog uses the 
/// TFindReplaceDialog::TData  structure to initialize the dialog box with
/// user-entered values (such as the text string to find).
//
class _OWLCLASS TFindDialog : public TFindReplaceDialog {
  public:
    TFindDialog(TWindow*        parent,
                TData&          data,
                TResId          templateId = 0,
                LPCTSTR         title = 0,
                TModule*        module = 0);

    TFindDialog(TWindow* parent, TData& data, TResId templateId, const tstring& title, TModule* = 0);

  protected:
    virtual TPerformCreateReturnType PerformCreate(int deprecated_argument = 0);

  private:
    TFindDialog();
    TFindDialog(const TFindDialog&);

  DECLARE_CASTABLE;
};

//
/// \class TReplaceDialog
// ~~~~~ ~~~~~~~~~~~~~~
/// TReplaceDialog creates a modeless dialog box that lets the user enter a
/// selection of text to replace. Because these are model dialog boxes, you can
/// search for text, edit the text in the window, and return to the dialog box to
/// enter another selection. TReplaceDialog uses the TFindReplaceDialog::TData class
/// to set the user-defined values for the dialog box, such as the text strings to
/// search for and replace.
//
class _OWLCLASS TReplaceDialog : public TFindReplaceDialog {
  public:
    TReplaceDialog(TWindow*        parent,
                   TData&          data,
                   TResId          templateId = 0,
                   LPCTSTR         title = 0,
                   TModule*        module = 0);

    TReplaceDialog(TWindow* parent, TData& data, TResId templateId, const tstring& title, TModule* = 0);

  protected:
    virtual TPerformCreateReturnType PerformCreate(int deprecated_argument = 0);

  private:
    TReplaceDialog(const TReplaceDialog&);
    TReplaceDialog& operator=(const TReplaceDialog&);

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
/// Returns the transfer data for the find and replace dialog.
//
inline TFindReplaceDialog::TData& TFindReplaceDialog::GetData() {
  return Data;
}

//
/// Sets the transfer data for the dialog.
//
inline void  TFindReplaceDialog::SetData(TData& data) {
  Data = data;
}

//
/// Returns the underlying system structure for the find and replace dialog.
//
inline FINDREPLACE& TFindReplaceDialog::GetFR() {
  return fr;
}

//
/// Sets the underlying system structure for the dialog.
//
inline void TFindReplaceDialog::SetFR(const FINDREPLACE& _fr) {
  fr = _fr;
}

//
/// Responds to a click of the Find Next button.
///
/// Default behavior inline for message response functions
//
inline void TFindReplaceDialog::CmFindNext() {
  DefaultProcessing();
}

//
/// Responds to a click of the Replace button.
///
/// Default behavior inline for message response functions
//
inline void TFindReplaceDialog::CmReplace() {
  DefaultProcessing();
}

//
/// Default behavior inline for message response functions
///
/// Responds to a click of the Replace All button.
//
inline void TFindReplaceDialog::CmReplaceAll() {
  DefaultProcessing();
}

//
/// Responds to a click of the Cancel button.
///
/// Default behavior inline for message response functions
//
inline void TFindReplaceDialog::CmCancel() {
  DefaultProcessing();
}

} // OWL namespace

#endif  // OWL_FINDREPL_H
