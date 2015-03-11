//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of classes supporting Printing/PrintPreview for TRichEdit
//----------------------------------------------------------------------------

#if !defined(OWL_RICHEDPR_H)
#define OWL_RICHEDPR_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/printer.h>
#include <owl/preview.h>
#include <owl/prevwin.h>
#include <owl/layoutwi.h>
#include <owl/richedit.h>
#include <owl/contain.h>
#include <owl/template.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup print
/// @{
/// \class TRichEditPreviewFrame
// ~~~~~ ~~~~~~~~~~~~~~~~~~~~~
/// TRichEditPreviewFrame is a window object which contains preview pages displaying
/// data from a rich edit control. It contains a simple preview toolbar and one or
/// two preview pages.
//
class _OWLCLASS TRichEditPreviewFrame : public TPreviewWin {
  public:
    TRichEditPreviewFrame(TWindow*         parentWindow, /* Parent object  */
                          TPrinter&       printer,       /* Printer object */
                          TPrintout&       printout,      /* Printout object*/
                          TRichEdit&       richEdit,      /* RichEdit ctrl. */
                          LPCTSTR         title,        /* Title of window*/
                          TLayoutWindow*   client);       /* Client window  */

    TRichEditPreviewFrame(
      TWindow* parentWindow,
      TPrinter& printer,
      TPrintout& printout,
      TRichEdit& richEdit,
      const tstring& title,
      TLayoutWindow* client);

  protected:
    TPreviewPage* GetNewPreviewPage(TWindow* parent,
                                    TPrintout& printout,
                                    TPrintDC&  prndc,
                                    TSize&     printExtent,
                                    int        pagenum = 1);
};

//
/// \class TRichEditPagePreview
// ~~~~~ ~~~~~~~~~~~~~~~~~~~~
/// TRichEditPagePreview is a window which displays rich edit data formatted for a
/// particular printer DC.
//
class _OWLCLASS TRichEditPagePreview : public TPreviewPage {
  public:
    TRichEditPagePreview(TWindow* parent, TPrintout& printout,
                         TPrintDC&  prndc, TSize& printExtent,
                         int pagenum = 1);
  protected:
    void  Paint(TDC& dc, bool, TRect& clip);
};

//
/// \class TRichEditPrintout
// ~~~~~ ~~~~~~~~~~~~~~~~~
/// TRichEditPrintout encapsulates the information to print/preview data from a rich
/// edit control. For example, it holds the offset of pages, the range of data to be
/// printer/previewed, etc.
//
class _OWLCLASS TRichEditPrintout : public TPrintout {
  public:
    TRichEditPrintout(TPrinter& printer,
                      TRichEdit& richEdit,
                      LPCTSTR title);

    TRichEditPrintout(TPrinter& printer, TRichEdit& richEdit, const tstring& title);
   
   ~TRichEditPrintout();

    // Overriden virtual methods of TPrintout
    //
    void          GetDialogInfo (int& minPage, int& maxPage,
                                 int& selFromPage, int& selToPage);
    void          SetPrintParams(TPrintDC* dc, TSize pageSize);
    bool          HasPage(int pageNumber);
    void          BeginPrinting();
    void          PrintPage(int page, TRect& rect, uint flags);
    void          EndPrinting();
    
    
/// This is an overriden virtual method of TPrintout.
/// \todo There is no such method in TPrintout and there is no implementation of this method anywhere, so it should be removed
    void          CleanupPrintParams();

    // Helper methods
    //
    int           PageOfOffset(int offset);

  protected:

// !BB  TDC*           PreviewDC;
// !BB  int            MapMode;
// !BB  bool           Scale;
// !BB  int            PrevMode;
// !BB  NS_CLASSLIB::TSize          OldVExt, OldWExt;
// !BB  NS_CLASSLIB::TRect          OrgR;
// !BB  NS_CLASSLIB::TSize          PrinterPageSizeInch;

    // !BB May be able to rely in TPrinter::GetPageSize() ??
    TPrinter&      Printer;               ///< Ref. to associated printer
    TRichEdit&     RichEdit;              ///< Ref. to associated control
    TSize          SizePhysPage;          ///< Physical size of printer (pels)
    TSize          SizePhysInch;          ///< Size of printer in inches
    TSize          Margins;               ///< Size of margins
    TFormatRange   FmtRange;              ///< Range of text to format
    bool           FlushCache;            ///< Have cached formatted data
    int            TextLen;               ///< Length of text formatted
    int            PageCount;             ///< Number of pages formatted
    TIntArray      PageIndices;           ///< Index of page offsets
};

/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


} // OWL namespace


#endif  // OWL_REDITPRI_H
