//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TStatic, the class for static controls and base for
/// any control that manages simple text.
//----------------------------------------------------------------------------

#if !defined(OWL_STATIC_H)
#define OWL_STATIC_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/control.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TStatic
// ~~~~~ ~~~~~~~
/// An interface object that represents a static text interface element. Static
/// elements consist of text or graphics that the user does not change. An
/// application, however, can modify the static control. You must use a TStatic
/// object, for example, to create a static control that's used to display a text
/// label such as a copyright notice in a parent TWindow object. TStatic can also be
/// used to make it easier to modify the text of static controls in TDialog objects.
/// See the sample program in the EXAMPLES\\OWL\\OWLAPI\\STATIC directory for an
/// example of a static control.
class _OWLCLASS TStatic : public TControl {
  public:

    //
    // For use with GetImage and SetImage.
    //
    enum TImageType 
    { 
      Bitmap = IMAGE_BITMAP, 
      Icon = IMAGE_ICON, 
      Cursor = IMAGE_CURSOR, 
      EnhMetaFile = IMAGE_ENHMETAFILE 
    };

    TStatic(TWindow* parent, int id, LPCTSTR title, int x, int y, int w, int h, 
      uint textLen = 0, TModule* = 0);
    TStatic(TWindow* parent, int id, const tstring& title, int x, int y, int w, int h,
      uint textLen = 0, TModule* = 0);
    TStatic(TWindow* parent, int resourceId, uint textLen = 0, TModule* = 0);
    TStatic(TWindow* parent, int resourceId, const tstring& title, uint textLimit = 0, TModule* = 0);
    TStatic(THandle hWnd, TModule* = 0);

    int GetTextLen() const;

    int GetText(LPTSTR str, int maxChars) const;
    tstring GetText() const;

    void SetText(LPCTSTR str);
    void SetText(const tstring& str) {SetText(str.c_str());}
    void SetText(uint resourceStringId);

    virtual void Clear();

    //
    /// Returns the length limit of the control's text.
    //
    uint GetTextLimit() const;
    void  SetTextLimit(uint textlimit);

    // Set and Get icon for Statics with the SS_ICON style
    //
    HICON GetIcon() const;
    HICON SetIcon(HICON);

    //
    /// TWindow override
    /// Transfers TextLimit characters of text to or from a transfer buffer pointed to
    /// by buffer. If direction is tdGetData, the text is transferred to the buffer from
    /// the static control. If direction is tdSetData, the static control's text is set
    /// to the text contained in the transfer buffer. Transfer returns  TextLimit, the
    /// number of bytes stored in or retrieved from the buffer. If direction is
    /// tdSizeData, Transfer returns  TextLimit without transferring data.
    //
    uint Transfer(void* buffer, TTransferDirection direction);

    /// \name Accessors and mutators for setting the transfer buffer field type
    /// These are used by the Safe Buffer Transfer machinery, and should not be
    /// used in general. TODO: Consider making these private with friend access.
    /// @{

    enum TTransferBufferFieldType
    {
      tbftCharArray, tbftString
    };
    TTransferBufferFieldType GetTransferBufferFieldType() const;
    void SetTransferBufferFieldType(TTransferBufferFieldType);

    /// @}

    // Set\Get image associated with static control
    //
    HANDLE GetImage(TImageType imageType = Bitmap) const;
    HANDLE SetImage(HANDLE image, TImageType imageType = Bitmap );

  protected:
    TTransferBufferFieldType TransferBufferFieldType;

    // Override TWindow virtual member functions
    //
    virtual TGetClassNameReturnType GetClassName();

    // Make sure that control is re-painted when resized
    //
    void EvSize(uint sizeType, const TSize& size);

  public_data:

    /// Holds the size of the text buffer for static controls. Because of the null
    /// terminator on the string, the number of characters that can be stored in the
    /// static control is one less than  TextLimit.  TextLimit is also the number of
    /// bytes transferred by the Transfer member function.
    //
    uint  TextLimit;  

  private:

    // Transfer implementations for different field types
    //
    uint TransferCharArray(void* buffer, TTransferDirection direction);
    uint TransferString(void* buffer, TTransferDirection direction);

    // Hidden to prevent accidental copying or assignment
    //
    TStatic(const TStatic&);
    TStatic& operator =(const TStatic&);

  DECLARE_RESPONSE_TABLE(TStatic);
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TStatic, 1);
  DECLARE_STREAMABLE_OWL(TStatic, 1);
};
// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TStatic );

//
// Static control notifications (Win32 only). Methods are: void Method()
//
// EV_STN_CLICKED(id, method)
// EV_STN_DBLCLK(id, method)
// EV_STN_ENABLE(id, method)
// EV_STN_DISABLE(id, method)
//

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//OWL_INSTANTIATE_TMPL(TStatic)    // VC++ issue

//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Returns the handle of the icon used for this static control.
//
inline HICON TStatic::GetIcon() const {
  PRECONDITION(GetHandle());
  return (HICON)CONST_CAST(TStatic*,this)->SendMessage(STM_GETICON);
}

//
/// Sets the handle of the icon.
//
inline HICON TStatic::SetIcon(HICON icon) {
  PRECONDITION(GetHandle());
  return (HICON)SendMessage(STM_SETICON, TParam1(icon));
}

//
/// Return handle of image used for static control
//
inline HANDLE TStatic::GetImage(TImageType imageType) const
{
  PRECONDITION(GetHandle());
  return (HANDLE)CONST_CAST(TStatic*,this)->SendMessage(STM_GETIMAGE,(TParam2)imageType);
}

//
/// Set handle of image.
//
inline HANDLE TStatic::SetImage(HANDLE image, TImageType imageType)
{
  PRECONDITION(GetHandle());
  return (HANDLE)SendMessage(STM_SETIMAGE, (TParam1)imageType, (TParam2)image);
}

//
/// Return the current length of the text in the control, excluding the terminating null character.
//
inline int TStatic::GetTextLen() const {
  PRECONDITION(GetHandle());
  return ::GetWindowTextLength(GetHandle());
}

//
/// Retrieves the static control's text, stores it in the str argument of maxChars
/// size, and returns the number of characters copied, excluding the terminating null character.
//
inline int TStatic::GetText(LPTSTR str, int maxChars) const {
  PRECONDITION(GetHandle());
  return ::GetWindowText(GetHandle(), str, maxChars);
}

//
/// String-aware overload
//
inline tstring TStatic::GetText() const {
  return TWindow::GetWindowText();
}

//
/// Sets the static control's text to the string supplied in str.
//
inline void TStatic::SetText(LPCTSTR str) {
  PRECONDITION(GetHandle());
  ::SetWindowText(GetHandle(), str);
}

//
/// Sets the static control's text to the resource string identified by the given id.
//
inline void TStatic::SetText(uint resourceStringId) {
  SetText(LoadString(resourceStringId));
}

//
/// Returns the size of the static control's transfer buffer; including space for the terminating null character.
//
inline uint TStatic::GetTextLimit() const {
  return TextLimit;
}

//
/// Sets the size of the static control's transfer buffer; including space for the terminating null character.
//
inline void TStatic::SetTextLimit(uint textlimit) {
  TextLimit = textlimit;
}

} // OWL namespace


#endif  // OWL_STATIC_H
