//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TClipboard which provides clipboard encapsulation
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/clipboar.h>

namespace owl {

OWL_DIAGINFO;

/// Points to a string that specifies the name of the protocol the client needs. The
/// default protocol is "StdFileEditing," which is the name of the object linking
/// and embedding protocol.
LPCTSTR TClipboard::DefaultProtocol = _T("StdFileEditing");

/// \class TClipboard
/// TClipboard encapsulates and manipulates Clipboard data. You can open, close,
/// empty, and paste data in a variety of data formats between the Clipboard and the
/// open window.  An object on the Clipboard can exist in a variety of Clipboard
/// formats, which range from bitmaps to text.
/// 
/// Usually, the window is in charge of manipulating Clipboard interactions between
/// the window and the Clipboard. It does this by responding to messages sent
/// between the Clipboard owner and the application. The following ObjectWindows
/// event-handling functions encapsulate these Clipboard messages:
/// EvRenderFormat - 	Responds to a WM_RENDERFORMAT message sent to the Clipboard
/// owner if a specific Clipboard format that an application has requested hasn't
/// been rendered. After the Clipboard owner renders the data in the requested
/// format, it callsSetClipboardData to place the data on the Clipboard.
/// EvRenderAllFormats - 	Responds to a message sent to the Clipboard owner if the
/// Clipboard owner has delayed rendering a Clipboard format.  After the Clipboard
/// owner renders data in all of possible formats, it calls SetClipboardData.
/// 
/// The following example tests to see If there is a palette on the Clipboard. If
/// one exists, TClipboard retrieves the palette, realizes it, and then closes the
/// Clipboard.
/// \code
///   if (clipboard.IsClipboardFormatAvailable(CF_PALETTE)) { 
///     newPal = new TPalette(TPalette(clipboard));  // make a copy
///     UpdatePalette(true);
///   }
///   // Try DIB format first
///   if (clipboard.IsClipboardFormatAvailable(CF_DIB)) { 
///     newDib = new TDib(TDib(clipboard));        // make a copy
///     newBitmap = new TBitmap(*newDib, newPal);  // newPal==0 is OK
///   // try metafile Second
///   //
///   } else if (clipboard.IsClipboardFormatAvailable(CF_METAFILEPICT)) { 
///     if (!newPal)
///       newPal = new TPalette((HPALETTE)GetStockObject(DEFAULT_PALETTE));
///     newBitmap = new TBitmap(TMetaFilePict(clipboard), *newPal,
///                             GetClientRect().Size());
/// ...
///   // Gets a bitmap , keeps it, and sets up data on the clipboard.
///   //
///   delete Bitmap;
///   Bitmap = newBitmap;
/// 
///   if (!newDib)
///     newDib = new TDib(*newBitmap, newPal);
/// #endif    // ????
///   delete Dib;
///   Dib = newDib;
/// 
///   delete Palette;
///   Palette = newPal ? newPal : new TPalette(*newDib);
///   Palette->GetObject(Colors);
/// 
///   PixelWidth  = Dib->Width();
///   PixelHeight = Dib->Height();
///   AdjustScroller();
///   SetCaption("(Clipboard)");
/// 
///   clipboard.CloseClipboard();
/// \endcode



//
/// Constructs a clipboard object to grab the clipboard given a window handle.
/// This is the preferred method of getting the clipboard;
//
/// Throws an exception on open failure if mustOpen is true (default)
/// mustOpen can be passed as false for compatability
//
TClipboard::TClipboard(HWND hWnd, bool mustOpen)
{
  OpenClipboard(hWnd);
  if (mustOpen && !IsOpen)
    throw TXClipboard(IDS_CLIPBOARDBUSY);
}

//
/// Destruct a clipboard object & close the clipboard if open
//
TClipboard::~TClipboard()
{
  if (IsOpen)
    ::CloseClipboard();
}

//
/// If the Clipboard is opened (IsOpen is true), closes the Clipboard. Closing the
/// Clipboard allows other applications to access the Clipboard.
//
void
TClipboard::CloseClipboard()
{
  if (IsOpen) {
    ::CloseClipboard();
    IsOpen = false;
  }
}

//
/// Opens the Clipboard and associates it with the window specified in Wnd. Other
/// applications cannot change the Clipboard data until the Clipboard is closed.
/// Returns true if successful; otherwise, returns false.
//
bool
TClipboard::OpenClipboard(HWND hWnd)
{
  return IsOpen = ::OpenClipboard(hWnd) != 0;
}

//----------------------------------------------------------------------------

//
/// Construct an available format iterator for a clipboard.
//
#if __DEBUG >= 1
TClipboardFormatIterator::TClipboardFormatIterator(const TClipboard& clip)
#else
TClipboardFormatIterator::TClipboardFormatIterator(const TClipboard& /*clip*/)
#endif
{
  PRECONDITION(bool(clip));
  Restart();
}

//
/// Restart the format iterator.
//
void
TClipboardFormatIterator::Restart()
{
  _Current = ::EnumClipboardFormats(0);
}

//
/// Get the next available format.
//
uint
TClipboardFormatIterator::operator ++()
{
  return _Current = ::EnumClipboardFormats(_Current);
}

//
/// Get the previous format.
//
uint
TClipboardFormatIterator::operator ++(int)
{
  uint current = _Current;
  _Current = ::EnumClipboardFormats(_Current);
  return current;
}

//----------------------------------------------------------------------------

//
/// Create the TXClipboard exception with a string resource.
//
TXClipboard::TXClipboard(uint resId)
:
  TXOwl(resId)
{
}

//
// Clone the exception for safe throwing in Windows.
//
TXClipboard*
TXClipboard::Clone() const
{
  return new TXClipboard(*this);
}


//
/// Throw the exception.
//
void
TXClipboard::Throw()
{
  throw *this;
}

//
/// Throw the exception.
//
void
TXClipboard::Raise(uint resourceId)
{
  TXClipboard(resourceId).Throw();
}

} // OWL namespace
/////////////////////////////////
