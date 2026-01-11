//----------------------------------------------------------------------------
//  ObjectWindows
//  OWL NExt
//  Copyright (c) 1999 by Yura Bidus . All Rights Reserved.
//
/// \file
///  Class definition for TTreeViewCtrlView (TTreeViewCtrl).
///  Class definition for TListViewCtrlView (TListViewCtrl).
//
//------------------------------------------------------------------------------

#if !defined(OWL_COMMVIEW_H) // Sentry, use file only if it's not already included.
#define OWL_COMMVIEW_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/docview.h>
#include <owl/treeviewctrl.h>
#include <owl/listviewctrl.h>


namespace owl {

#include <owl/preclass.h>

/// \addtogroup docview
/// @{
/// \class TTreeViewCtrlView
// ~~~~~ ~~~~~~~~~~~~~~~
//
//
class _OWLCLASS TTreeViewCtrlView : public TTreeViewCtrl,  public TView {
  public:
    TTreeViewCtrlView(TDocument& doc, TWindow* parent = 0, int id = -1);
    virtual ~TTreeViewCtrlView() override;

    static LPCTSTR StaticName();  // put in resource

    // Override virtuals from TWindow
    //
    auto CanClose() -> bool override;

    // Override virtuals from TView
    //
    auto GetViewName() -> LPCTSTR override;
    auto GetWindow() -> TWindow* override;
    auto SetDocTitle(LPCTSTR docname, int index) -> bool override;

  private:
    // Event handlers
    //
    bool     VnIsWindow(HWND hWnd);

  DECLARE_RESPONSE_TABLE(TTreeViewCtrlView);
};

typedef TTreeViewCtrlView TTreeWindowView;

//
/// \class TListViewCtrlView
// ~~~~~ ~~~~~~~~~~~~~~~
//
//

class _OWLCLASS TListViewCtrlView : public TListViewCtrl,  public TView {
  public:
    TListViewCtrlView(TDocument& doc, TWindow* parent = 0, int id = -1);
    virtual ~TListViewCtrlView();

    static LPCTSTR StaticName();  // put in resource

    // Override virtuals from TWindow
    //
    auto CanClose() -> bool override;

    // Override virtuals from TView
    //
    auto GetViewName() -> LPCTSTR override;
    auto GetWindow() -> TWindow* override;
    auto SetDocTitle(LPCTSTR docname, int index) -> bool override;

  private:
    // Event handlers
    //
    bool     VnIsWindow(HWND hWnd);

    DECLARE_RESPONSE_TABLE(TListViewCtrlView);
};
/// @}

typedef TListViewCtrlView TListWindowView;


#include <owl/posclass.h>

//
// Inlines
//

//
/// \class TTreeViewCtrlView
// ~~~~~ ~~~~~~~~~~~~~~~
inline TTreeViewCtrlView::~TTreeViewCtrlView()
{
}
//
inline LPCTSTR TTreeViewCtrlView::StaticName()
{
  return _T("TreeWindow View");
}
//
// Only query document if this is the last view open to it.
//
inline bool TTreeViewCtrlView::CanClose()
{
  return TTreeViewCtrl::CanClose() &&
         (Doc->NextView(this) ||
          Doc->NextView(nullptr) != this ||
          Doc->CanClose());
}
//
inline LPCTSTR TTreeViewCtrlView::GetViewName()
{
  return StaticName();
}
//
inline TWindow* TTreeViewCtrlView::GetWindow()
{
  return static_cast<TWindow*>(this);
}
//
/// Does a given HWND belong to this view? Yes if it is us, or a child of us
//
inline bool TTreeViewCtrlView::VnIsWindow(HWND hWnd)
{
  return hWnd == GetHandle() || IsChild(hWnd);
}
//
inline bool TTreeViewCtrlView::SetDocTitle(LPCTSTR docname, int index)
{
  return TTreeViewCtrl::SetDocTitle(docname, index);
}

//
//
/// \class TListViewCtrlView
// ~~~~~ ~~~~~~~~~~~~~~~
inline TListViewCtrlView::~TListViewCtrlView()
{
}
//
inline LPCTSTR TListViewCtrlView::StaticName()
{
  return _T("ListWindow View");
}
//
/// Only query document if this is the last view open to it.
//
inline bool TListViewCtrlView::CanClose()
{
  return TListViewCtrl::CanClose() &&
         (Doc->NextView(this) ||
          Doc->NextView(nullptr) != this ||
          Doc->CanClose());
}
//
inline LPCTSTR TListViewCtrlView::GetViewName()
{
  return StaticName();
}
//
inline TWindow* TListViewCtrlView::GetWindow()
{
  return static_cast<TWindow*>(this);
}
//
/// Does a given HWND belong to this view? Yes if it is us, or a child of us
//
inline bool TListViewCtrlView::VnIsWindow(HWND hWnd)
{
  return hWnd == GetHandle() || IsChild(hWnd);
}
//
inline bool TListViewCtrlView::SetDocTitle(LPCTSTR docname, int index)
{
  return TListViewCtrl::SetDocTitle(docname, index);
}
//================================================================

} // OWL namespace

#endif  // OWL_COMMVIEW_H sentry.
