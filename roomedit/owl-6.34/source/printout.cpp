//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------
#include <owl/pch.h> 
#include <owl/printer.h>

namespace owl {

OWL_DIAGINFO;


//
/// Constructs an instance of TPrintOut with the given title.
//
TPrintout::TPrintout(const tstring& title)
:
  Title(title),
  Banding(false),
  ForceAllBands(true),
  DC(0),
  PageSize(0)
{}

//
/// Place-holder
//
TPrintout::~TPrintout()
{}

//
/// Returns the title of the current printout.
//
LPCTSTR TPrintout::GetTitle() const 
{
  return Title.c_str();
}

//
/// Returns true if banding is desired.
//
bool TPrintout::WantBanding() const 
{
  return Banding;
}

//
/// Returns true if the force all bands.
//
bool TPrintout::WantForceAllBands() const 
{
  return ForceAllBands;
}

//
/// Returns the DC associated with the printout.
//
TPrintDC* TPrintout::GetPrintDC() 
{
  return DC;
}

//
/// Sets the banding flag for the printout.
//
void TPrintout::SetBanding(bool banding) 
{
  Banding = banding;
}

//
/// Sets the force banding flag.
//
void TPrintout::SetForceAllBands(bool force) 
{
  ForceAllBands = force;
}

//
/// Sets the associated DC.
//
void TPrintout::SetPrintDC(TPrintDC* dc) 
{
  DC = dc;
}

//
/// Retrieves the size of the page.
//
TSize TPrintout::GetPageSize() const 
{
  return PageSize;
}

//
/// Sets the size of the page.
//
void TPrintout::SetPageSize(const TSize& pagesize) 
{
  PageSize = pagesize;
}

//
/// SetPrintParams sets DC to dc and PageSize to pageSize. The printer object's
/// Print function calls SetPrintParams to obtain the information it needs to
/// determine pagination and page count. Derived objects that override
/// SetPrintParams must call the inherited function.
//
void
TPrintout::SetPrintParams(TPrintDC* dc, TSize pageSize)
{
  SetPrintDC(dc);
  SetPageSize(pageSize);
}

//
/// Retrieves information needed to allow the printing of selected pages of the
/// document and returns true if page selection is possible. Use of page ranges is
/// optional, but if the page count is easy to determine, GetDialogInfo sets the
/// number of pages in the document. Otherwise, it sets the number of pages to 0 and
/// printing will continue until HasPage returns false.
//
void
TPrintout::GetDialogInfo(int& minPage, int& maxPage, int& selFromPage, int& selToPage)
{
  minPage = 1;
  maxPage = INT_MAX;
  selFromPage = selToPage = 0;
}

//
/// The printer object's Print function calls BeginPrinting once at the beginning of
/// a print job, regardless of how many copies of the document are to be printed.
/// Derived objects can override BeginPrinting to perform any initialization needed
/// before printing.
//
void
TPrintout::BeginPrinting()
{
}

//
/// The printer object's Print function calls BeginDocument once before printing
/// each copy of a document. The flags field indicates if the current print band
/// accepts graphics, text, or both.
/// The default BeginDocument does nothing. Derived objects can override
/// BeginDocument to perform any initialization needed at the beginning of each copy
/// of the document.
//
void
TPrintout::BeginDocument(int, int, uint)
{
}

//
/// HasPage is called after every page is printed. By default, it returns false,
/// indicating that only one page is to be printed. If the document contains more
/// than one page, this function must be overridden to return true while there are
/// more pages to print.
//
bool
TPrintout::HasPage(int page)
{
  return page == 1;
}

//
/// PrintPage is called for every page (or band, if Banding is true) and must be
/// overridden to print the contents of the given page. The rect and flags
/// parameters are used during banding to indicate the extent and type of band
/// currently requested from the driver (and should be ignored if Banding is false).
/// page is the number of the current page.
void
TPrintout::PrintPage(int, TRect&, uint)
{
}

//
/// The printer object's Print function calls EndDocument after each copy of the
/// document finishes printing. Derived objects can override EndDocument to perform
/// any needed actions at the end of each document.
//
void
TPrintout::EndDocument()
{
}

//
/// The printer object's Print function calls EndPrinting after all copies of the
/// document finish printing. Derived objects can override EndPrinting to perform
/// any needed actions at the end of each document.
//
void
TPrintout::EndPrinting()
{
}

//
/// Sets a new title for this printout. The passed title string is copied.
//
void
TPrintout::SetTitle(const tstring& title)
{
  Title = title;
}


IMPLEMENT_STREAMABLE(TPrintout);

#if !defined(BI_NO_OBJ_STREAMING)

//
// Deserialization from stream
//
void*
TPrintout::Streamer::Read(ipstream& is, uint32) const
{
#if defined(UNICODE)
  _USES_CONVERSION;
  char* title = is.freadString();
  GetObject()->Title = _A2W(title);
  delete[] title;
#else
  GetObject()->Title = is.freadString();
#endif
  is >> GetObject()->Banding;
  is >> GetObject()->ForceAllBands;
  return GetObject();
}

//
// Serialization to stream
//
void
TPrintout::Streamer::Write(opstream& os) const
{
  _USES_CONVERSION;
  os.fwriteString(_W2A(GetObject()->Title.c_str()));
  os << GetObject()->Banding;
  os << GetObject()->ForceAllBands;
}

#endif  // if !defined(BI_NO_OBJ_STREAMING)

} // OWL namespace
/* ========================================================================== */

