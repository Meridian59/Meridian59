//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998, by Yura Bidus, All Rights Reserved
//
/// \file
///  Class definition for TPaneSplitterView (TPaneSplitter, TView).
//
//----------------------------------------------------------------------------

#if !defined(OWL_PSVIEW_H) // Sentry, use file only if it's not already included.
#define OWL_PSVIEW_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/panespli.h>
#include <owl/docview.h>

namespace owl {

#include <owl/preclass.h>

/// \addtogroup docview
/// @{
/// \class TPaneSplitterView
// ~~~~~ ~~~~~~~~~~~~~~~~~
//
class _OWLCLASS TPaneSplitterView: public TPaneSplitter, public TView {
  public:
DECLARE_CASTABLE;
    TPaneSplitterView(TDocument& doc, TWindow* parent=0);
    virtual ~TPaneSplitterView();

    static LPCTSTR StaticName();

    // Override virtuals from TWindow
    //
    auto CanClose() -> bool override;

    // Override virtuals from TView
    //
    auto GetViewName() -> LPCTSTR override;
    auto GetWindow() -> TWindow* override;
    auto SetDocTitle(LPCTSTR docname, int index) -> bool override;
    using TView::SetDocTitle; ///< String-aware overload

  private:
    // Event handlers
    //
    bool     VnIsWindow(HWND hWnd);

  DECLARE_RESPONSE_TABLE(TPaneSplitterView);

}; 
/// @}

#include <owl/posclass.h>

//-------------------------------------------------------------------------
//
// Inlines
//
//
//
inline LPCTSTR TPaneSplitterView::StaticName(){
  return _T("PaneSplitter View");
}
//
inline bool TPaneSplitterView::CanClose(){
  return TPaneSplitter::CanClose() &&
         (Doc->NextView(this) ||
          Doc->NextView(nullptr) != this ||
          Doc->CanClose());
}
//
inline LPCTSTR TPaneSplitterView::GetViewName(){
  return StaticName();
}
//
inline TWindow* TPaneSplitterView::GetWindow(){
  return static_cast<TWindow*>(this);
}
//
/// Does a given HWND belong to this view? Yes if it is us, or a child of us
//
inline bool TPaneSplitterView::VnIsWindow(HWND hWnd){
  return hWnd == GetHandle() || IsChild(hWnd);
}
//
inline bool TPaneSplitterView::SetDocTitle(LPCTSTR docname, int index){
  return TPaneSplitter::SetDocTitle(docname, index);
}

//

} // OWL namespace

#endif  // PSVIEW_H sentry.
