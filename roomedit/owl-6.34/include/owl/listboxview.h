//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1993, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of class TListBoxView
//----------------------------------------------------------------------------

#if !defined(OWL_LISTBOXVIEW_H)
#define OWL_LISTBOXVIEW_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/docmanag.h>
#include <owl/listbox.h>
#include <owl/listboxview.rh>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the 
// definition of classes
#include <owl/preclass.h>

/// \addtogroup docview
/// @{
/// \class TListBoxView
// ~~~~~ ~~~~~~~~~
/// Provides views for list boxes.
class _OWLCLASS TListBoxView : public TListBox, public TView {
  public:
    TListBoxView(TDocument& doc, TWindow* parent = 0);

    // Overridden virtuals from TView
    //
    LPCTSTR GetViewName();
    TWindow* GetWindow();
    bool     SetDocTitle(LPCTSTR docname, int index);
    using TView::SetDocTitle; ///< String-aware overload

    // Overridden virtuals from TListBox
    //
    virtual int   AddString(LPCTSTR str);
    using TListBox::AddString; // Inject string-aware overload.

    // Overridden virtuals from TWindow
    //
    bool CanClose();
    bool Create();

    static LPCTSTR StaticName();  ///< put in resource

  protected:
    long GetOrigin() const;
    void SetOrigin(long origin);

    int  GetMaxWidth() const;
    void SetMaxWidth(int maxwidth);

    void SetExtent(const tstring& str);
    bool LoadData(int top, int sel);

    // Message response functions
    //
    void CmEditUndo();
    void CmEditCut();
    void CmEditCopy();
    void CmEditPaste();
    void CmEditDelete();
    void CmEditClear();
    void CmEditAdd();
    void CmEditItem();
    void CmSelChange();

    uint EvGetDlgCode(const MSG*);

    bool VnDocClosed(int omode);
    bool VnCommit(bool force);
    bool VnRevert(bool clear);
    bool VnIsWindow(HWND hWnd);
    bool VnIsDirty();

  public_data:
    int  DirtyFlag;   ///< flag to indicate if the view is dirty

  protected_data:
    long Origin;      ///< object's beginning position on the stream
    int  MaxWidth;    ///< maximum horizontal extent (the number of pixels by which the view can be scrolled horizontally)

  DECLARE_RESPONSE_TABLE(TListBoxView);
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TListBoxView,1);
  DECLARE_STREAMABLE_OWL(TListBoxView,1);
};
/// @}

// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TListBoxView );

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Returns a constant string for the view name, ListView. This information is
/// displayed in the user interface selection box.
//
inline LPCTSTR TListBoxView::StaticName() {
  return _T("List View");
}

//
/// Overrides TView's virtual GetViewName function and returns the descriptive name
/// of the class (StaticName).
//
inline LPCTSTR TListBoxView::GetViewName() {
  return StaticName();
}

//
/// Overrides TView's virtual GetWindow function and returns the list view object as
/// a TWindow.
//
inline TWindow* TListBoxView::GetWindow() {
  return (TWindow*)this;
}

//
/// Overrides TView's virtual SetDocTitle function and stores the document title.
/// This name is forwarded up the parent chain until a TFrameWindow object accepts
/// the data and displays it in its caption.
//
inline bool TListBoxView::SetDocTitle(LPCTSTR docname, int index) {
  return TListBox::SetDocTitle(docname, index);
}

//
/// Return true if both the listbox and the document can close.
///
/// Checks to see if all child views can be closed before closing the current view.
/// If any child returns 0, CanClose returns 0 and aborts the process. If all
/// children return nonzero, it calls TDocManager::FlushDoc.
//
inline bool TListBoxView::CanClose() {
  return TListBox::CanClose() &&
         (Doc->NextView(this) ||
          Doc->NextView(0) != this ||
          Doc->CanClose());
}

//
/// Returns the origin position on the persistent stream.
//
inline long TListBoxView::GetOrigin() const {
  return Origin;
}

//
/// Sets the position of the object
//
inline void TListBoxView::SetOrigin(long origin) {
  Origin = origin;
}

//
/// Returns the maximum width of the strings.
//
inline int TListBoxView::GetMaxWidth() const {
  return MaxWidth;
}

//
/// Sets the maximum width of the strings.
//
inline void TListBoxView::SetMaxWidth(int maxwidth) {
  MaxWidth = maxwidth;
}

//
/// Returns a nonzero value if the window's handle passed in hWnd is the same as
/// that of the view's display window.
//
inline bool TListBoxView::VnIsWindow(HWND hWnd) {
  return GetHandle() == hWnd;
}

//
/// Returns a nonzero value if changes made to the data in the view have not been
/// saved to the document; otherwise, returns 0.
//
inline bool TListBoxView::VnIsDirty() {
  return DirtyFlag != 0;
}


} // OWL namespace


#endif  // OWL_LISTBOXVIEW_H
