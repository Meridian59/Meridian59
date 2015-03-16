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
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/panespli.h>
#include <owl/docview.h>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
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
    bool     CanClose();

    // Override virtuals from TView
    //
    LPCTSTR  GetViewName();
    TWindow* GetWindow();
    bool     SetDocTitle(LPCTSTR docname, int index);
    using TView::SetDocTitle; ///< String-aware overload

  private:
    // Event handlers
    //
    bool     VnIsWindow(HWND hWnd);

  DECLARE_RESPONSE_TABLE(TPaneSplitterView);
//  DECLARE_STREAMABLE_OWL(TPaneSplitterView, 1);

}; //{{TPaneSplitterView}}
/// @}

// define streameable inlines (VC)
//DECLARE_STREAMABLE_INLINES(TPaneSplitterView);

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
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
          Doc->NextView(0) != this ||
          Doc->CanClose());
}
//
inline LPCTSTR TPaneSplitterView::GetViewName(){
  return StaticName();
}
//
inline TWindow* TPaneSplitterView::GetWindow(){
  return (TWindow*)this;
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
