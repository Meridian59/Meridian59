//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of classes for clipboard Encapsulation
//----------------------------------------------------------------------------

#if !defined(OWL_CLIPBOAR_H)
#define OWL_CLIPBOAR_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>
#include <owl/except.h>         // Owl exception classes


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup Clipboard
/// @{
/// \class TClipboard
// ~~~~~ ~~~~~~~~~~
/// The clipboard class encapsulates the methods for the clipboard object
/// of Windows.
//
class _OWLCLASS TClipboard {
  public:
    // Constructors / destructor
    //
    TClipboard(HWND hWnd, bool mustOpen=true);  // aquire & open the clipboard
   ~TClipboard();

    // Close & reopen the clipboard
    //
    void        CloseClipboard();
    bool        OpenClipboard(HWND hWnd);

    operator    bool() const;

    HANDLE      GetClipboardData(uint format) const;
    HWND        GetOpenClipboardWindow() const;
    HWND        GetClipboardOwner() const;
    HWND        GetClipboardViewer() const;
    int         GetClipboardFormatName(uint format, LPTSTR formatName,
                                       int maxCount) const;
    int         GetPriorityClipboardFormat(uint * priorityList, int count) const;
    int         CountClipboardFormats() const;
    bool        IsClipboardFormatAvailable(uint format) const;
    bool        EmptyClipboard();
    uint        RegisterClipboardFormat(const tstring& formatName) const;
    HANDLE      SetClipboardData(uint format, HANDLE handle);
    HWND        SetClipboardViewer(HWND hWnd) const;

#if defined(__OLE_H) || defined(_INC_OLE)
    bool        QueryCreate(LPCTSTR protocol = DefaultProtocol,
                            OLEOPT_RENDER   renderopt= olerender_draw,
                            OLECLIPFORMAT   format   = 0);

    bool QueryCreate(const tstring& protocol, OLEOPT_RENDER renderopt = olerender_draw, OLECLIPFORMAT format = 0)
    {return QueryCreate(protocol.c_str(), renderopt, format);}

    bool        QueryLink(LPCTSTR protocol = DefaultProtocol,
                          OLEOPT_RENDER   renderopt= olerender_draw,
                          OLECLIPFORMAT   format   = 0);

    bool QueryLink(const tstring& protocol, OLEOPT_RENDER renderopt = olerender_draw, OLECLIPFORMAT format = 0)
    {return QueryLink(protocol.c_str(), renderopt, format);}

#endif

    static LPCTSTR     DefaultProtocol;

  protected_data:
    bool              IsOpen;

    TClipboard();      // used by the obsolete global object
};
/// @}

/// \addtogroup except
/// @{
/// \class TXClipboard
// ~~~~~ ~~~~~~~~~~~
/// TXClipboard creates the TXClipboard exception with a string resource.
//
class _OWLCLASS TXClipboard : public TXOwl {
  public:
    TXClipboard(uint resourceId = IDS_CLIPBOARDBUSY);
    virtual TXClipboard* Clone() const; // override
    void Throw();

    static void Raise(uint resourceId = IDS_CLIPBOARDBUSY);
};
/// @}

/// \addtogroup Clipboard
/// @{
// class TClipboardFormatIterator
// ~~~~~ ~~~~~~~~~~~~~~~~~~~~~~~~
class _OWLCLASS TClipboardFormatIterator {
  public:
    TClipboardFormatIterator(const TClipboard&);

    uint          Current();
                  operator void* ();
    uint          operator ++();
    uint          operator ++(int);
    void          Restart();

  private:
    uint          _Current;
};
/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Create an empty clipboard object.
/// Set the state of the object to not opened.
//
inline TClipboard::TClipboard() {
  IsOpen = false;
}

//
/// Return true if the clipboard is currently owned by this application.
//
inline TClipboard::operator bool() const {
  return IsOpen;
}

//
/// Retrieves data from the Clipboard in the format specified by format.The
/// following formats are supported:
/// - \c \b  CF_BITMAP	Data is in a bitmap format
/// - \c \b  CF_DIB	Data is memory
/// - \c \b  CF_DIF	Data is in a Data Interchange Format (DIF).
/// - \c \b  CF_DSPMETAFILEPICT	Data is in a metafile format.
/// - \c \b  CF_DSPTEXT	Data is in a text format.
/// - \c \b  CF_METAFILEPICT	Data is in a metafile structure.
/// - \c \b  CF_OEMTEXT	Data is an array of text characters in OEM character set.
/// - \c \b  CF_OWNERDISPLAT	Data is in a special format that the application must display.
/// - \c \b  CF_PALETTE	Data is in a color palette format.
/// - \c \b  CF_PENDATA	Data is used for pen format.
/// - \c \b  CF_RIFF	Data is in Resource Interchange File Format (RIFF).
/// - \c \b  CF_SYLK	Data is in symbolic Link format (SYLK).
/// - \c \b  CF_TEXT	Data is stored as an array of text characters.
/// - \c \b  CF_TIFF	Data is in Tag Image File Format (TIFF).
/// - \c \b  CF_WAVE	Data is in a sound wave format.
//
inline HANDLE TClipboard::GetClipboardData(uint Format) const {
  return ::GetClipboardData(Format);
}

//
/// Retrieves the handle of the window that currently has the Clipboard open. If the
/// Clipboard is not open, the return value is NULL. Once the Clipboard is opened,
/// applications cannot modify the data.
//
inline HWND TClipboard::GetOpenClipboardWindow() const {
  return ::GetOpenClipboardWindow();
}

//
/// Retrieves the handle of the window that currently owns the Clipboard, otherwise
/// returns NULL.
//
inline HWND TClipboard::GetClipboardOwner() const {
  return ::GetClipboardOwner();
}

//
/// Retrieves the handle of the first window in the Clipboard-view chain. Returns
/// NULL if there is no viewer.
//
inline HWND TClipboard::GetClipboardViewer() const {
  return ::GetClipboardViewer();
}

//
/// Retrieves the name of the registered format specified by format and copies the
/// format to the buffer pointed to by formatName. maxCount specifies the maximum
/// length of the name of the format. If the name is longer than maxCount, it is
/// truncated.
//
inline int TClipboard::GetClipboardFormatName(uint Format, LPTSTR FormatName, int MaxCount) const {
  return ::GetClipboardFormatName(Format, FormatName, MaxCount);
}

//
/// Returns the first Clipboard format in a list. priorityList points to an array
/// that contains a list of the Clipboard formats arranged in order of priority. See
/// GetClipboardData for the Clipboard formats.
//
inline int TClipboard::GetPriorityClipboardFormat(uint * priorityList, int count) const {
  return ::GetPriorityClipboardFormat(priorityList, count);
}

//
/// Returns a count of the number of types of data formats the Clipboard can use.
//
inline int TClipboard::CountClipboardFormats() const {
  return ::CountClipboardFormats();
}

//
/// Indicates if the format specified in format exists for use in the Clipboard. See
/// GetClipBoardData for a description of Clipboard data formats.
/// The following code tests if the Clipboard can support the specified formats:
/// \code
/// void
/// TBmpViewWindow::CePaste(TCommandEnabler& ce)
/// { 
///   TClipboard& clipboard = OpenClipboard();
///   ce.Enable(
///     clipboard && (
///       clipboard.IsClipboardFormatAvailable(CF_METAFILEPICT) ||
///       clipboard.IsClipboardFormatAvailable(CF_DIB) ||
///       clipboard.IsClipboardFormatAvailable(CF_BITMAP)
///     )
///   );
///   clipboard.CloseClipboard();
/// \endcode
//
inline bool TClipboard::IsClipboardFormatAvailable(uint format) const {
  return ::IsClipboardFormatAvailable(format);
}

//
/// Clears the Clipboard and frees any handles to the Clipboard's data. Returns true
/// if the Clipboard is empty, or false if an error occurs.
//
inline bool TClipboard::EmptyClipboard() {
  return ::EmptyClipboard();
}

//
/// Registers a new Clipboard format. formatName points to a character string that
/// identifies the new format. If the format can be registered, the return value
/// indicates the registered format. If the format can't be registered, the return
/// value is 0. Once the format is registered, it can be used as a valid format in
/// which to render the data.
//
inline uint TClipboard::RegisterClipboardFormat(const tstring& formatName) const {
  return ::RegisterClipboardFormat(formatName.c_str());
}

//
/// Copy the data onto the clipboard in the format.
//
/// Sets a handle to the block of data at the location indicated by handle. format
/// specifies the format of the data block. The Clipboard must have been opened
/// before the data handle is set. format can be any one of the valid Clipboard
/// formats (for example, CF_BITMAP or CF_DIB). See GetClipboardData for a list of
/// these formats. handle is a handle to the memory location where the data data is
/// stored. If successful, the return value is a handle to the data; if an error
/// occurs, the return value is 0. Before the window is updated with the Clipboard
/// data, the Clipboard must be closed.
//
inline HANDLE TClipboard::SetClipboardData(uint Format, HANDLE Handle) {
  return ::SetClipboardData(Format,Handle);
}

//
/// Adds the window specified by Wnd to the chain of windows that WM_DRAWCLIPBOARD
/// notifies whenever the contents of the Clipboard change.
//
inline HWND TClipboard::SetClipboardViewer(HWND Wnd) const {
  return ::SetClipboardViewer(Wnd);
}

#if defined(__OLE_H) || defined(_INC_OLE)
//
/// Return true if the object on the clipboard can be rendered in the
/// requested format.
//
/// QueryCreate determines if the object on the Clipboard supports the specified
/// protocol and rendering options. DefaultProtocol points to a string specifying
/// the name of the protocol the client application needs to use. renderopt
/// specifies the client application's display and printing preference for the
/// Clipboard object. renderopt is set to olerender_draw, which tells the client
/// library to obtain and manage the data presentation. format specifies the
/// Clipboard format the client application requests. The macros _OLE_H or _INC_OLE
/// must be defined before this function can be used.
//
inline bool TClipboard::QueryCreate(
                LPCTSTR protocol,
                OLEOPT_RENDER   renderopt,
                OLECLIPFORMAT   format
              )
{
  return ::OleQueryCreateFromClip(protocol, renderopt, format) == OLE_OK;
}

//
/// Return true if the object on the clipboard can be linked to in the
/// requested format.
//
/// QueryLink determines if a client application can use the Clipboard data to
/// produce a linked object that uses the specified protocol and rendering options.
/// See TClipboard::QueryCreate for a description of the parameters. The macros
/// _OLE_H or _INC_OLE must be defined before this function can be used.
//
inline bool TClipboard::QueryLink(
                LPCTSTR protocol,
                OLEOPT_RENDER   renderopt,
                OLECLIPFORMAT   format
              )
{
  return ::OleQueryLinkFromClip(protocol, renderopt, format) == OLE_OK;
}
#endif


//
/// Return the current clipboard format.
//
inline uint TClipboardFormatIterator::Current() {
  return _Current;
}

//
/// Return true if the iterator is not at an end of the clipboard format
/// chain.
//
inline TClipboardFormatIterator::operator void* () {
  return _Current ? this : 0;
}


} // OWL namespace

#endif  // OWL_CLIPBOAR_H
