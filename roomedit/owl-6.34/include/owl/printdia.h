//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of Print and PrintSetup common Dialogs classes
//----------------------------------------------------------------------------

#if !defined(OWL_PRINTDIA_H)
#define OWL_PRINTDIA_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/commdial.h>
#include <owl/dc.h>

namespace owl {

#include <owl/preclass.h>

/// \addtogroup print
/// @{

//
/// \class TPrintDialog
/// TPrintDialog displays a modal print or a page setup
/// dialog. The print dialog box lets you print a document. The print setup dialog
/// box lets you configure the printer and specify additional print job
/// characteristics. The page setup dialog lets you specify page settings, such as
/// margins, and paper orientation. You can also use TPrinter and TPrintout to
/// provide support for printer dialog boxes.  TPrintDialog uses the
/// TPrintDialog::TData structure to initialize the dialog box with the user's
/// printer options, such as the number of pages to print, the output device, and so on.
//
class _OWLCLASS TPrintDialog : public TCommonDialog 
{
  public:
    
    //
    /// TPrintDialog::TData contains information required to initialize the printer
    /// dialog box with the user's print selections. This information consists of the
    /// number of copies to be printed, the first and last pages to print, the maximum
    /// and minimum number of pages that can be printed and various flag values that
    /// indicate whether the Pages radio button is displayed, the Print to File check
    /// box is enabled, and so on.TPrintDialog uses this struct to initialize the print
    /// dialog box. Whenever the user changes the print requirements, this struct is
    /// updated.
    ///
    /// If an error occurs, TPrintDialog::TData returns one of the common dialog
    /// extended error codes. TPrintDialog::TData also takes care of locking and
    /// unlocking memory associated with the DEVMODE and DEVNAMES structures, which
    /// contain information about the printer driver, the printer, and the output
    /// printer port.
    //
    class _OWLCLASS TData {
      public:
      
        /// Flags , which are used to initialize the printer dialog box, can be one or more
        /// of the following values that control the appearance and functionality of the
        /// dialog box:
        ///
        /// - \c \b  PD_ALLPAGES		Indicates that the All radio button was selected when the user
        ///   closed the dialog box.
        /// - \c \b  PD_COLLATE 		Causes the Collate checkbox to be checked when the dialog box is
        ///   created.
        /// - \c \b  PD_DISABLEPRINTTOFILE	Disables the Print to File check box.
        /// - \c \b  PD_HIDEPRINTTOFILE	Hides and disables the Print to File check box.
        /// - \c \b  PD_NOPAGENUMS 		Disables the Pages radio button and the associated edit
        ///   control.
        /// - \c \b  PD_NOSELECTION		Disables the Selection radio button.
        /// - \c \b  PD_NOWARNING 		Prevents the warning message from being displayed when there is
        ///   no default printer.
        /// - \c \b  PD_PAGENUMS 		Selects the Pages radio button when the dialog box is created.
        /// - \c \b  PD_PRINTSETUP 		Displays the Print Setup dialog box rather than the Print
        ///   dialog box.
        /// - \c \b  PD_PRINTTOFILE		Checks the Print to File check box when the dialog box is
        ///   created.
        /// - \c \b  PD_RETURNDC 		Returns a device context matching the selections that the user
        ///   made in the dialog box.
        /// - \c \b  PD_RETURNDEFAULT	Returns DevNames structures that are initialized for the
        ///   default printer without displaying a dialog box.
        /// - \c \b  PD_RETURNIC 		Returns an information context matching the selections that the
        ///   user made in the dialog box.
        /// - \c \b  PD_SELECTION 		Selects the Selection radio button when the dialog box is
        ///   created.
        /// - \c \b  PD_SHOWHELP 		Shows the Help button in the dialog box.
        /// - \c \b  PD_USEDEVMODECOPIES	If a printer driver supports multiple copies, setting this
        ///   flag causes the requested number of copies to be stored in the dmCopies member
        ///   of the DevMode structure and 1 in Copies. If a printer driver does not support
        ///   multiple copies, setting this flag disables the Copies edit control. If this
        ///   flag is not set, the number 1 is stored in DevMode and the requested number of
        ///   copies in Copies.
        //
        uint32          Flags;

        ///  If the dialog box is successfully executed, Error returns 0. Otherwise, it
        /// contains one of the following error codes.
        ///
        /// - \c \b  CDERR_DIALOGFAILURE	Failed to create a dialog box.
        /// - \c \b  CDERR_FINDRESFAILURE	Failed to find a specified resource.
        /// - \c \b  CDERR_INITIALIZATION	Failed to initialize the common dialog box function.
        ///   A lack of sufficient memory can generate this error.
        /// - \c \b  CDERR_LOCKRESOURCEFAILURE	Failed to lock a specified resource.
        /// - \c \b  CDERR_LOADRESFAILURE	Failed to load a specified resource.
        /// - \c \b  CDERR_LOADSTRFAILURE	Failed to load a specified string.
        /// - \c \b  CDERR_MEMALLOCFAILURE	Unable to allocate memory for internal data structures.
        /// - \c \b  CDERR_MEMLOCKFAILURE	Unable to lock the memory associated with a handle.
        /// - \c \b  CDERR_REGISTERMSGFAIL	A message, designed for the purpose of communicating
        ///   between two applications, could not be registered.
        /// - \c \b  PDERR_CREATEICFAILURE	TPrintDialog failed to create an information context.
        /// - \c \b  PDERR_DEFAULTDIFFERENT	The printer described by structure members doesn't match
        ///   the default printer. This error message can occur if the user changes the
        ///   printer specified in the control panel.
        /// - \c \b  PDERR_DNDMMISMATCH	The printer specified in DevMode and in DevNames is
        ///   different.
        /// - \c \b  PDERR_GETDEVMODEFAIL	The printer device-driver failed to initialize the DevMode
        ///   structure.
        /// - \c \b  PDERR_INITFAILURE 	The TPrintDialog structure could not be initialized.
        /// - \c \b  PDERR_LOADDRVFAILURE	The specified printer's device driver could not be loaded.
        /// - \c \b  PDERR_NODEFAULTPRN	A default printer could not be identified.
        /// - \c \b  PDERR_NODEVICES	No printer drivers exist.
        /// - \c \b  PDERR_PARSEFAILURE	The string in the [devices] section of the WIN.INI file
        ///   could not be parsed.
        /// - \c \b  PDERR_PRINTERNOTFOUND	The [devices] section of the WIN.INI file doesn't contain
        ///   the specified printer.
        /// - \c \b  PDERR_RETDEFFAILURE	Either DevMode or DevNames contain zero.
        /// - \c \b  PDERR_SETUPFAILURE	TPrintDialog failed to load the required resources.
        //
        uint32 Error;
      
        int FromPage; ///< FromPage indicates the beginning page to print.
        int ToPage;   ///< ToPage indicates the ending page to print.
        int MinPage;  ///< MinPage indicates the minimum number of pages that can be printed.
        int MaxPage;  ///< MaxPage indicates the maximum number of pages that can be printed.
        int Copies;   ///< Copies indicates the actual number of pages to be printed.

        uint32 PageSetupFlags; ///< Additional page setup dialog flags
        TPoint PaperSize;      ///< Size of the paper user has chosen
        TRect  MinMargin;      ///< Minimum allowable margins of the paper
        TRect  Margin;         ///< Initial margins for the paper
        bool   DoPageSetup;    ///< Flag to do page setup?
        bool	 UseOldDialog;   ///< Flag to force use of the old print dialog under Win2K/XP/

        TData();
        ~TData();

        void Lock();
        void Unlock();

        /// Accessors and mutators for the internal Win32 DEVMODE structure.
        /// @{
        void ClearDevMode();
        const DEVMODE* GetDevMode() const;
        DEVMODE* GetDevMode();
        void SetDevMode(const DEVMODE* devMode);
        /// @}

        /// Accessors and mutators for the internal Win32 DEVNAMES structure.
        /// @{
        void ClearDevNames();
        const DEVNAMES* GetDevNames() const;
        LPCTSTR GetDriverName() const;
        LPCTSTR GetDeviceName() const;
        LPCTSTR GetOutputName() const;
        void SetDevNames(const tstring& driver, const tstring& device, const tstring& output);
        /// @}

        /// Transfers the printers device context.
        //
        TPrintDC* TransferDC();

        void* Read(ipstream& is, uint32 version);
        void Write(opstream& os);

      private:

        HGLOBAL   HDevMode;
        HGLOBAL   HDevNames;
        HDC       HDc;
        DEVMODE*  DevMode;
        DEVNAMES* DevNames;

        // Hide to prevent copying & assignment.
        //
        TData(const TData&);
        TData& operator =(const TData&);

      friend class TPrintDialog;
    };

    TPrintDialog(TWindow* parent, TData&, 
      LPCTSTR printTemplateName = 0, 
      LPCTSTR setupTemplateName = 0, 
      LPCTSTR title = 0, 
      TModule* = 0);

    TPrintDialog(TWindow* parent, TData&, 
      const tstring& printTemplateName, 
      const tstring& setupTemplateName = tstring(), 
      const tstring& title = tstring(), 
      TModule* = 0);

    bool GetDefaultPrinter();
    int  DoExecute();

  protected:

    void Init();
    virtual TDialogProcReturnType DialogFunction(TMsgId, TParam1, TParam2);

    // Default behavior inline for message response functions
    //
    void CmSetup(); //EV_COMMAND(psh1,

    PRINTDLG&  GetPD();
    void SetPD(const PRINTDLG& _pd);

    TData& GetData();

  protected_data:

    /// Specifies the dialog box print job characteristics such as page range, number of
    /// copies, device context, and so on necessary to initialize the print or print
    /// setup dialog box.
    //
    union 
    {
      PRINTDLG     pd;  // old name
      PRINTDLG     Pd;  // new name
    };

  #if(WINVER >= 0x0500)
    PRINTDLGEX     Pde;  // new structure
    PRINTPAGERANGE PdeRange;
  #endif

    PAGESETUPDLG   Psd;

    /// Data is a reference to the TData object passed in the constructor. The TData
    /// object contains print specifications such as the number of copies to be printed,
    /// the number of pages, the output device name, and so on.
    //
    TData&         Data;

  private:

    tstring PrintTemplateName;
    tstring SetupTemplateName;

    // Hide to prevent copying & assignment.
    //
    TPrintDialog(const TPrintDialog&);
    TPrintDialog& operator=(const TPrintDialog&);

  DECLARE_RESPONSE_TABLE(TPrintDialog);
  DECLARE_CASTABLE;
};

/// @}

#include <owl/posclass.h>

} // OWL namespace

#endif  // OWL_PRINTDIA_H
