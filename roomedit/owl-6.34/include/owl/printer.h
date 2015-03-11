//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//----------------------------------------------------------------------------

#if !defined(OWL_PRINTER_H)
#define OWL_PRINTER_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/dialog.h>
#include <owl/printdia.h>
#include <owl/printer.rh>

namespace owl {

class _OWLCLASS TPrintDC;

#include <owl/preclass.h>

/// \addtogroup print
/// @{

//
/// \class TPrinterAbortDlg
/// TPrinterAbortDlg is the object type of the default printer-abort dialog box.
/// This dialog box is initialized to display the title of the current printout, as
/// well as the device and port currently used for printing.
/// 
/// TPrinterAbortDlg expects to have three static text controls, with control IDs of
/// 101 for the title, 102 for the device, and 103 for the port. These controls must
/// have "%s" somewhere in the text strings so that they can be replaced by the
/// title, device, and port. The dialog-box controls can be in any position and tab
/// order.
//
class _OWLCLASS TPrinterAbortDlg : public TDialog 
{
  public:
    TPrinterAbortDlg(TWindow* parent, TResId, const tstring& title, const tstring& device, const tstring& port, HDC = HDC(-1));

  protected:
    void     SetupWindow();
    void     CmCancel();

    HDC      PrnDC;              ///< Device context to print on

  DECLARE_RESPONSE_TABLE(TPrinterAbortDlg);
};

//
/// TPrintout banding flags
//
enum TPrintoutFlags 
{
  pfGraphics = 0x01,     ///< Current band accepts graphics
  pfText     = 0x02,     ///< Current band accepts text
  pfBoth     = (pfGraphics|pfText) ///< Current band accepts both text and graphics
};

//
/// \class TPrintout
/// TPrintout represents the physical printed document that is to sent to a printer
/// to be printed. TPrintout does the rendering of the document onto the printer.
/// Because this object type is abstract, it cannot be used to print anything by
/// itself. For every document, or document type, a class derived from TPrintout
/// must be created and its PrintPage function must be overridden.
//
class _OWLCLASS TPrintout : public TStreamableBase 
{
  public:
    TPrintout(const tstring& title);
    virtual ~TPrintout();

    virtual void    SetPrintParams(TPrintDC* dc, TSize pageSize);
    virtual void    GetDialogInfo(int& minPage, int& maxPage, int& selFromPage, int& selToPage);
    virtual void    BeginPrinting();
    virtual void    BeginDocument(int startPage, int endPage, uint flags);
    virtual bool    HasPage(int pageNumber);
    virtual void    PrintPage(int page, TRect& rect, uint flags);
    virtual void    EndDocument();
    virtual void    EndPrinting();

    // Accessors to data members of printout object
    //
    TSize           GetPageSize() const;
    LPCTSTR         GetTitle() const;
    bool            WantBanding() const;
    void            SetBanding(bool banding=true);
    bool            WantForceAllBands() const;
    void            SetForceAllBands(bool force=true);
    TPrintDC*       GetPrintDC();

  protected:

    void SetTitle(const tstring& title);
    void SetPrintDC(TPrintDC* dc);
    void SetPageSize(const TSize& pagesize);

  protected_data:
    
    /// Title is the current title to use for the printout. By default, this title
    /// appears in the Abort dialog box and as the name of the job in the Print Manager.
    //
    tstring Title;         

    /// If Banding is true, the printout is banded and the PrintPage function is called
    /// once for every band. Otherwise, PrintPage is called only once for every page.
    /// Banding a printout is more memory- and time-efficient than not banding. By
    /// default, Banding is set to false.
    //
    bool Banding;       
    
    /// Many device drivers do not provide all printer bands if both text and graphics
    /// are not performed on the first band (which is typically a text-only band).
    /// Leaving ForceAllBands true forces the printer driver to provide all bands
    /// regardless of what calls are made in the PrintPage function. If PrintPage does
    /// nothing but display text, it is more efficient for ForceAllBands to be false. By
    /// default, it is true. ForceAllBands takes effect only if Banding is true.
    //
    bool ForceAllBands; 
    
    TPrintDC* DC;       ///< pointer to DC created amd owned by our TPrinter
    TSize     PageSize; ///< dimensions of the printout page

  private:

    // Hidden to prevent accidental copying or assignment
    //
    TPrintout(const TPrintout&);
    TPrintout& operator =(const TPrintout&);

  DECLARE_STREAMABLE_OWL(TPrintout, 1);
};

//
// Define streameable inlines (VC).
//
DECLARE_STREAMABLE_INLINES( owl::TPrintout );

//
/// \class TPrinter
/// TPrinter is an encapsulation around the Windows printer device interface,
/// and represents the physical printer device.  
///
/// To print or configure a printer, initialize an instance of TPrinter.
/// To print a TPrintout, send the TPrintout to the TPrinter's Print function.
///
/// Examples:
///
/// Creating a default device printing object:
/// \code
///   DefaultPrinter = new TPrinter();
/// \endcode
///
/// Creating a device for a specific printer:
/// \code
///   PostScriptPrinter = new TPrinter();
///   PostScriptPrinter->SetDevice("PostScript Printer", "PSCRIPT", "LPT2:");
/// \endcode
///
/// Allowing the user to setup the printer:
/// \code
///   DefaultPrinter->Setup(MyWindow);
/// \endcode
//

//
class _OWLCLASS TPrinter: public TStreamableBase 
{
  public:
    TPrinter(TApplication* app = 0);
    virtual ~TPrinter();

    virtual void     ClearDevice();
    virtual void     Setup(TWindow* parent);
    virtual bool     Print(TWindow* parent, TPrintout& printout, bool prompt);
    virtual void     ReportError(TWindow* parent, TPrintout& printout);

    TPrintDialog::TData& GetSetup();

    static void      SetUserAbort(HDC abortDC = HDC(-1));
    static HDC       GetUserAbort();

    // Accessors to protected state data
    //
    int              GetError();

    TPrintDialog::TData* GetData();
    void             SetData(TPrintDialog::TData* data);

    TApplication*    GetApplication();
    void             SetApplication(TApplication* app);

    TSize            GetPageSize() const;
    void             SetPageSize(const TSize& pagesize);
    TSize            GetPageSizeInch() const;
    void             SetPageSizeInch(const TSize& pageSizeInch);

    /// \name Retrieves/assigns the output file 
    /// The output file can be set to redirect the print job.
    /// Set to blank to disable redirection (default).
    //
    /// @{
    LPCTSTR          GetOutputFile() const;
    void             SetOutputFile(const tstring& outputFile);
    /// @}

    virtual void     SetPageSizes(const TPrintDC& dc);

  protected:
    virtual void     GetDefaultPrinter();
    virtual void     SetPrinter(const tstring& driver, const tstring& device, const tstring& output);

    virtual bool     ExecPrintDialog(TWindow* parent);
    virtual bool     ExecPageSetupDialog(TWindow* parent);
    virtual TWindow* CreateAbortWindow(TWindow* parent, TPrintout& printout);

  protected_data:
    int                  Error;       ///< negative if error occurred during print
    TPrintDialog::TData* Data;        ///< printer setup information
    TApplication*        Application; ///< the app owning this printer object

    TSize PageSize;        ///< size of the page
    TSize PageSizeInch;    ///< size of an inch on the page
    tstring OutputFile; ///< filename for output redirection (see request #3044058)

  private:
    static HDC UserAbortDC; ///< Set by print dialog to prntDC if user cancels. Set to -1 cancels all print jobs.

    // Hidden to prevent accidental copying or assignment
    //
    TPrinter(const TPrinter&);
    TPrinter& operator =(const TPrinter&);

  DECLARE_STREAMABLE_OWL(TPrinter, 2);
};

//
// define streameable inlines (VC)
//
DECLARE_STREAMABLE_INLINES( owl::TPrinter );

//
/// \class TXPrinter
/// TXPrinter describes an exception that results from an invalid printer object. 
/// This type of error can occur when printing to the physical printer.
//
class _OWLCLASS TXPrinter : public TXOwl 
{
  public:
    TXPrinter(uint resId = IDS_PRINTERERROR);

    virtual TXPrinter* Clone() const; // override
    void Throw();
    static void Raise(uint resId = IDS_PRINTERERROR);
};

//
/// \class TXPrinting
/// TXPrinting describes an exception that indicates that printing failed. 
/// See GetErrorMessage for error codes.
//
class _OWLCLASS TXPrinting : public TXOwl 
{
  public:
    TXPrinting(int error = SP_ERROR);

    virtual TXPrinting* Clone() const; // override
    void Throw();
    static void Raise(int error = SP_ERROR);

    int Error;
    tstring GetErrorMessage(TModule* = 0) const;
};

/// @}

#include <owl/posclass.h>

} // OWL namespace

#endif  // OWL_PRINTER_H
