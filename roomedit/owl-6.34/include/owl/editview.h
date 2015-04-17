//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
// Definition of class TEditView
//----------------------------------------------------------------------------

#if !defined(OWL_EDITVIEW_H)
#define OWL_EDITVIEW_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/docmanag.h>
#include <owl/editsear.h>


namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup docview
/// @{
/// \class TEditView
// ~~~~~ ~~~~~~~~~
/// Derived from TView and TEditSearch, TEditView provides a view wrapper for the
/// ObjectWindows text edit class (TEdit). A streamable class, TEditView includes
/// several event-handling functions that handle messages between a document and its
/// views.
//
class _OWLCLASS TEditView : public TEditSearch, public TView {
  public:
    TEditView(TDocument& doc, TWindow* parent = 0);
   ~TEditView();
    static LPCTSTR StaticName();  /// put in resource

    // Overridden virtuals from TView
    //
    LPCTSTR    GetViewName();
    TWindow*   GetWindow();
    bool       SetDocTitle(LPCTSTR docname, int index);
    using TView::SetDocTitle; ///< String-aware overload

    // Overridden virtuals from TWindow
    //
    bool   Create();
    bool   CanClose();

  protected:
    bool   LoadData();

    long   GetOrigin() const;
    void   SetOrigin(long origin);

    // message response functions
    //
    void   EvNCDestroy();
    bool   VnCommit(bool force);
    bool   VnRevert(bool clear);
    bool   VnIsWindow(HWND hWnd);
    bool   VnIsDirty() ;
    bool   VnDocClosed(int omode);

  protected_data:
/// Holds the file position at the beginning of the display.
    long   Origin;

  DECLARE_RESPONSE_TABLE(TEditView);
  //DECLARE_STREAMABLE(_OWLCLASS, owl::TEditView,1);
  DECLARE_STREAMABLE_OWL(TEditView,1);
};
/// @}

// define streameable inlines (VC)
DECLARE_STREAMABLE_INLINES( owl::TEditView );

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementation
//

//
/// Returns "Edit View", the descriptive name of the class for the ViewSelect menu.
//
inline LPCTSTR TEditView::StaticName() {
  return _T("Edit View");
}  // put in resource

//
/// Overrides TView::GetViewName and returns the descriptive name of the class
/// ("StaticName").
//
inline LPCTSTR TEditView::GetViewName() {
  return StaticName();
}

//
/// Overrides TView::GetWindow and returns this as a TWindow.
//
inline TWindow* TEditView::GetWindow() {
  return (TWindow*)this;
}

//
/// Overrides TView::SetDocTitle and forwards the title to its base class,
/// TEditSearch. index is the number of the view displayed in the caption bar.
/// docname is the name of the document displayed in the view window.
//
inline bool TEditView::SetDocTitle(LPCTSTR docname, int index) {
  return TEditSearch::SetDocTitle(docname, index);
}

//
/// Returns a nonzero value if the view can be closed.
//
inline bool TEditView::CanClose(){
  return TEditSearch::CanClose() &&
         (Doc->NextView(this) ||
          Doc->NextView(0) != this ||
          Doc->CanClose());
}

//
/// Returns the position of the stream buffer at which the edit buffer is stored.
//
inline long TEditView::GetOrigin() const {
  return Origin;
}

//
/// Sets the position of the stream buffer that the edit buffer is stored.
//
inline void TEditView::SetOrigin(long origin) {
  Origin = origin;
}

//
/// Returns a nonzero value if changes made to the data in the view have not been
/// saved to the document; otherwise, it returns 0.
//
inline bool TEditView::VnIsDirty() {
  return TEditSearch::IsModified();
}


} // OWL namespace


#endif  // OWL_EDITVIEW_H
