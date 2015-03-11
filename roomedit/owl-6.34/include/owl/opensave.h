//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of TOpenSave abstract, TFileOpen, TFileSave common Dialog classes
//----------------------------------------------------------------------------

#if !defined(OWL_OPENSAVE_H)
#define OWL_OPENSAVE_H

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
/// \class TOpenSaveDialog
// ~~~~~ ~~~~~~~~~~~~~~~
/// TOpenSaveDialog is the base class for modal dialogs that let you open and save a
/// file under a specified name. TOpenSaveDialog constructs a TData structure and
/// passes it the TOpenSaveDialog constructor. Then the dialog is executed (modal)
/// or created (modeless). Upon return, the necessary fields are updated, including
/// an error field that contains 0, or a common dialog extended error.
class _OWLCLASS TOpenSaveDialog : public TCommonDialog {
  public:

/// TOpenSaveDialog structure contains information about the user's file open or
/// save selection. Specifically, this structure stores a user-specified file name
/// filter, file extension, file name, the initial directory to use when displaying
/// file names, any error codes, and various file attributes that determine, for
/// example, if the file is a read-only file. The classes TFileOpenDialog and
/// TFileSaveDialog use the information stored in this structure when a file is
/// opened or saved.
    class _OWLCLASS TData {
      public:
      	
				/// Flag contains one or more of the following constants:
				/// - \c \b  OFN_HIDEREADONLY	Hides the read-only check box.
				/// - \c \b  OFN_FILEMUSTEXIST	Lets the user enter only names of existing files in the File
				/// Name entry field. If an invalid file name is entered, a warning message is
				/// displayed.
				/// - \c \b  OFN_PATHMUSTEXIST	Lets the user enter only valid path names. If an invalid path
				/// name is entered, a warning message is displayed.
				/// - \c \b  OFN_NOVALIDATE	Performs no check of the file name and requires the owner of a
				/// derived class to perform validation.
				/// - \c \b  OFN_NOCHANGEDIR	Sets the current directory back to what it was when the dialog
				/// was initiated.
				/// - \c \b  OFN_ALLOWMULTISELECT	Allows multiple selections in the File Name list box.
				/// - \c \b  OFN_CREATEPROMPT	Asks if the user wants to create a file that does not
				/// currently exist.
				/// - \c \b  OFN_EXTENSIONDIFFERENT	Idicates the user entered a file name different from the
				/// specified in DefExt. This message is returned to the caller.
				/// - \c \b  OFN_NOREADONLYRETURN	The returned file does not have the Read Only attribute
				/// set and is not in a write-protected directory. This message is returned to the
				/// caller.
				/// - \c \b  OFN_NOTESTFILECREATE	The file is created after the dialog box is closed. If the
				/// application sets this flag, there is no check against write protection, a full
				/// disk, an open drive door, or network protection. For certain network
				/// environments, this flag should be set.
				/// - \c \b  OFN_OVERWRITEPROMPT	The Save As dialog box displays a message asking the user if
				/// it's OK to overwrite an existing file.
				/// - \c \b  OFN_SHAREAWARE	If this flag is set and a call to open a file fails because of a
				/// sharing violation, the error is ignored and the dialog box returns the given
				/// file name. If this flag is not set, the virtual function ShareViolation is
				/// called, which returns OFN_SHAREWARN (by default) or one of the following values:
				/// --	OFN_SHAREFALLTHROUGH - File name is returned from the dialog box. 
				/// --	OFN_SHARENOWARN - No further action is taken.
				/// --	OFN_SHAREWARN - User receives the standard warning message for this type of error.
				/// - \c \b  OFN_SHOWHELP	Shows the Help button in the dialog box.
        uint32  Flags;

				/// Error contains one or more of the following error codes:
				/// - \c \b  CDERR_DIALOGFAILURE	Failed to create a dialog box.
				/// - \c \b  CDERR_LOCKRESOURCEFAILURE	Failed to lock a specified resource.
				/// - \c \b  CDERR_LOADRESFAILURE	Failed to load a specified resource.
				/// - \c \b  CDERR_LOADSTRFAILURE	Failed to load a specified string.
        uint32  Error;

				/// Holds the name of the file to be saved or opened.
        LPTSTR  FileName;

				/// Filter holds the filter to use initially when displaying file names.
        LPTSTR  Filter;
        
				/// CustomFilter stores the user-specified file filter; for example, *.CPP.        
        LPTSTR  CustomFilter;

				/// FilterIndex indicates which filter to use initially when displaying file names.
        int     FilterIndex;

				/// InitialDir holds the directory to use initially when displaying file names.
        LPTSTR  InitialDir;
        
				///< DefExt stores the default extension.
        LPTSTR  DefExt;
        int     MaxPath;
        uint32  FlagsEx;

        TData(uint32 flags=0, LPCTSTR filter=0, LPTSTR customFilter=0,
              LPTSTR initialDir=0, LPTSTR defExt=0,int maxPath=0,
              int filterIndex = 0, uint32 flagsEx=0);

        TData(
          uint32 flags, 
          const tstring& filter, 
          LPTSTR customFilter = 0,
          LPTSTR initialDir = 0, 
          LPTSTR defExt = 0,
          int maxPath = 0,
          int filterIndex = 0, 
          uint32 flagsEx = 0);

        TData(const TData& src);
       ~TData();

        TData& operator =(const TData& src);

        void     SetFilter(LPCTSTR filter = 0);
        void SetFilter(const tstring& filter) {SetFilter(filter.c_str());}

        void     Write(opstream& os);
        void     Read(ipstream& is);
    };

    TOpenSaveDialog(TWindow*        parent,
                    TData&          data,
                    TResId          templateId = 0,
                    LPCTSTR         title = 0,
                    TModule*        module = 0);

    TOpenSaveDialog(
      TWindow* parent,
      TData& data,
      TResId templateId,
      const tstring& title,
      TModule* module = 0);

    static int GetFileTitleLen(LPCTSTR fileName);
    static int GetFileTitle(LPCTSTR fileName, LPTSTR fileTitle,
                            int bufLen);

  protected:
    TOpenSaveDialog(TWindow* parent, TData& data, TModule*   module = 0);
    void    Init(TResId templateId);

    virtual TDialogProcReturnType DialogFunction(TMsgId, TParam1, TParam2);

    // Override TWindow & TDialog virtuals
    //
    int     DoExecute() = 0;

    // Virtual function called when a share violation occurs in dlg
    //
    virtual int ShareViolation();

    // Messages registered by the common dialog DLL
    //
    static uint GetShareViMsgId();

    // Default behavior inline for message response functions
    //
    void    CmOk();           //EV_COMMAND(IDOK,
    void    CmLbSelChanged(); //EV_COMMAND(lst1 or lst2

    OPENFILENAME& GetOFN();
    void          SetOFN(const OPENFILENAME& _ofn);

    TData&  GetData();
    void    SetData(TData& data);

  protected_data:
/// Contains the attributes of the file name such as length, extension, and
/// directory. ofn is initialized using the fields in the TOpenSaveDialog::TData
/// class. 
    OPENFILENAME ofn;

/// Stores the file name, its length, extension, filter, initial directory, default
/// file name, extension, and any error messages.
    TData&       Data;

    tstring Title; ///< Stores the title of the dialog, if given.

/// Contains the message ID of the registered ShareViolation message. 
    static const uint ShareViMsgId;

  private:
    TOpenSaveDialog(const TOpenSaveDialog&);
    TOpenSaveDialog& operator =(const TOpenSaveDialog&);

  DECLARE_RESPONSE_TABLE(TOpenSaveDialog);
};

//
/// \class TFileOpenDialog
// ~~~~~ ~~~~~~~~~~~~~~~
/// TFileOpenDialog is a modal dialog box that lets you specify the name of a file
/// to open. Use this dialog box to respond to a CM_FILEOPEN command that's
/// generated when a user selects File|Open from a menu. TFileOpenDialog  uses the
/// TOpenSave::TDatastructure to initialize the file open dialog box.
class _OWLCLASS TFileOpenDialog : public TOpenSaveDialog {
  public:
    TFileOpenDialog(TWindow*        parent,
                    TData&          data,
                    TResId          templateId = 0,
                    LPCTSTR title = 0,
                    TModule*        module = 0);

    // Override TDialog virtual functions
    //
    int  DoExecute();

  private:
    TFileOpenDialog(const TOpenSaveDialog&);
    TFileOpenDialog& operator=(const TOpenSaveDialog&);
};

//
/// \class TFileSaveDialog
// ~~~~~ ~~~~~~~~~~~~~~~
/// TFileSaveDialog is a modal dialog box that lets you enter the name of a file to
/// save. Use TFileSaveDialog to respond to a CM_FILESAVEAS command generated when a
/// user selects File|Save from a menu. TFileSaveDialog uses the TOpenSave::TData
/// structure to initialize the file save dialog box.
class _OWLCLASS TFileSaveDialog : public TOpenSaveDialog {
  public:
    TFileSaveDialog(TWindow*        parent,
                    TData&          data,
                    TResId          templateId = 0,
                    LPCTSTR title = 0,
                    TModule*        module = 0);

    // Override TDialog virtual functions
    //
    int  DoExecute();

  private:
    TFileSaveDialog(const TFileSaveDialog&);
    TFileSaveDialog& operator=(const TFileSaveDialog&);
};
/// @}


// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Returns the sharing violation message ID.
//
inline uint TOpenSaveDialog::GetShareViMsgId()
{
  return ShareViMsgId;
}

//
/// Returns the title length of the dialog.
//
inline int TOpenSaveDialog::GetFileTitleLen(LPCTSTR fileName)
{
  return TCommDlg::GetFileTitle((LPTSTR)fileName, 0, 0);   //Win32 cast
}

//
/// Returns the title of the dialog.
//
inline int TOpenSaveDialog::GetFileTitle(LPCTSTR fileName,
                                         LPTSTR fileTitle,
                                         int fileTitleLen)
{
  return TCommDlg::GetFileTitle((LPTSTR)fileName, fileTitle, (uint16)fileTitleLen);
                        //Win32 casts
}

//
/// Responds to a click on the dialog box's OK button (with the identifier IDOK).
/// Calls CloseWindow (passing IDOK).
inline void TOpenSaveDialog::CmOk()
{
  DefaultProcessing();
}

//
/// Indicates that the selection state of the file name list box in the
/// GetOpenFileName or GetSaveFileName dialog boxes has changed. CmLbSelChanged is a
/// default handler for command messages sent by lst1 or lst2 (the file and
/// directory list boxes, respectively).
inline void TOpenSaveDialog::CmLbSelChanged()
{
  DefaultProcessing();
}

//
/// Returns the OPENFILENAME structure.
//
inline OPENFILENAME& TOpenSaveDialog::GetOFN()
{
  return ofn;
}

//
/// Sets the OPENFILENAME structure for the dialog.
//
inline void TOpenSaveDialog::SetOFN(const OPENFILENAME& _ofn)
{
  ofn = _ofn;
}

//
/// Retrieves the transfer buffer for the dialog.
//
inline TOpenSaveDialog::TData& TOpenSaveDialog::GetData()
{
  return Data;
}

//
/// Sets the transfer buffer for the dialog.
//
inline void TOpenSaveDialog::SetData(TOpenSaveDialog::TData& data)
{
  Data = data;
}

} // OWL namespace


#endif  // OWL_OPENSAVE_H
