//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
//----------------------------------------------------------------------------

#if !defined(OCF_OLEVIEW_H)
#define OCF_OLEVIEW_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <ocf/olewindo.h>
#include <owl/docview.h>
#include <owl/docmanag.h>

namespace ocf {

//
// Additional notification messages
//
const owl::uint vnInvalidate  = 9;   // a rect in view needs repaint
const owl::uint vnLinkView    = 10;  // find view with matched TOcLinkView
const owl::uint vnLinkMoniker = 11;  // find view with matched moniker

#if defined(OWL5_COMPAT)

NOTIFY_SIG(vnInvalidate, owl::TRect&)
NOTIFY_SIG(vnLinkView, TOcLinkView&)
NOTIFY_SIG(vnLinkMoniker, owl::TString&)

#endif

template <owl::TMsgId>
struct TDocViewDispatch;

template <> struct TDocViewDispatch<WM_OWLNOTIFY>
{
  static const owl::TMsgId MessageId = WM_OWLNOTIFY;

  typedef owl::TResult TResult;
  typedef owl::TParam1 TParam1;
  typedef owl::TParam2 TParam2;

  template <owl::uint NotificationCode>
  struct TNotificationDispatch;

};

template <> struct TDocViewDispatch<WM_OWLNOTIFY>::TNotificationDispatch<vnInvalidate>
{
  template <class T, bool (T::*M)(owl::TRect&)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {
    PRECONDITION(p2);
    return (p2 != 0 && (static_cast<T*>(i)->*M)(*reinterpret_cast<owl::TRect*>(p2))) ? TRUE : FALSE;
  }
};

template <> struct TDocViewDispatch<WM_OWLNOTIFY>::TNotificationDispatch<vnLinkView>
{
  template <class T, bool (T::*M)(TOcLinkView&)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {
    PRECONDITION(p2);
    return (p2 != 0 && (static_cast<T*>(i)->*M)(*reinterpret_cast<TOcLinkView*>(p2))) ? TRUE : FALSE;
  }
};

template <> struct TDocViewDispatch<WM_OWLNOTIFY>::TNotificationDispatch<vnLinkMoniker>
{
  template <class T, bool (T::*M)(owl::TString&)>
  static TResult Decode(void* i, TParam1, TParam2 p2)
  {
    PRECONDITION(p2);
    return (p2 != 0 && (static_cast<T*>(i)->*M)(*reinterpret_cast<owl::TString*>(p2))) ? TRUE : FALSE;
  }
};

#define OCF_EV_VN_(notificationCode, method)\
  VN_DEFINE(notificationCode, method, ::ocf::TDocViewDispatch<WM_OWLNOTIFY>::TNotificationDispatch<notificationCode>::Decode)

#define EV_VN_INVALIDATE OCF_EV_VN_(vnInvalidate, VnInvalidate)
#define EV_VN_LINKVIEW OCF_EV_VN_(vnLinkView, VnLinkView)
#define EV_VN_LINKMONIKER OCF_EV_VN_(vnLinkMoniker, VnLinkMoniker)

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TOleView
// ~~~~~ ~~~~~~~~
/// The OLE2 window view class. Used as a view in doc/view model
//
/// Derived from TWindowView and TView, TOleView supports the View half of the
/// Doc/View pair and creates a window with a view that can display an associated
/// document. Documents use views to display themselves to a user. Regardless of
/// whether a view belongs to a server or a container, TOleView sets up a
/// corresponding TOcDocument object (an entire compound document).
///
/// In the case of an OLE-enabled container application, view refers to the window
/// where the container application draws the compound document, which may consist
/// of one or more linked and embedded objects. To display these objects in
/// different formats, a container can be associated with more than one view.
/// Similarly, to display the data properly, each embedded object can also have its
/// own view. Each container view creates a corresponding ObjectComponents TOcView
/// object.
/// If the view belongs to an OLE-enabled server application, TOleView creates a
/// remote view on the server's document (a TOcRemView object). TOleView takes care
/// of transmitting messages from the server to the container, specifically in the
/// case of merging menus and redrawing embedded objects, and supports merging the
/// server's and the container's pop-up menu items to form a composite menu. Because
/// it knows the dimensions of the server's view, TOleView is responsible for
/// telling the container how to redraw the embedded object.
///
/// Similarly to TView, TOleView supports the creation of views and provides several
/// event handling functions that allow the view to query, commit, and close views.
/// TOleView  also manages the writing to storage of documents that belong to a
/// container or a server.
class _OCFCLASS TOleView : public TOleWindow, public owl::TView {
  public:
    TOleView(owl::TDocument& doc, owl::TWindow* parent = 0);
   ~TOleView();

    static LPCTSTR StaticName();

    // Inherited virtuals from TView
    //
    auto GetViewName() -> LPCTSTR override;
    auto GetWindow() -> owl::TWindow* override;
    auto SetDocTitle(LPCTSTR docname, int index) -> bool override;
    auto OleShutDown() -> bool override;

    // Inherited virtuals from TWindow
    //
    auto CanClose() -> bool override;

  protected:
    auto CreateOcView(owl::TRegLink*, bool isEmbedded, IUnknown* outer) -> TOcView* override;
    void CleanupWindow() override;
    bool OtherViewExists();

    /// \name View notifications
    /// @{
    bool VnInvalidate(owl::TRect& rect);
    bool VnDocOpened(int omode);
    bool VnDocClosed(int omode);
    /// @}

    auto GetViewMenu() -> owl::TMenuDescr* override;
    auto GetFileName() -> LPCTSTR override;
    auto EvOcViewPartInvalid(TOcPartChangeInfo& changeInfo) -> bool override;
//    owl::uint32 EvOcViewDiscardUndo(void * undo);

    // Container specific messages
    //
//    bool   EvOcAppInsMenus(TOcMenuDescr &);

    /// \name Server specific messages
    /// @{
    bool   EvOcViewClose();
    bool   EvOcViewSavePart(TOcSaveLoad & ocSave);
    bool   EvOcViewLoadPart(TOcSaveLoad & ocLoad);
    bool   EvOcViewOpenDoc(LPCTSTR path);
    bool   EvOcViewInsMenus(TOcMenuDescr & sharedMenu);
    bool   EvOcViewAttachWindow(bool attach);
    bool   EvOcViewSetLink(TOcLinkView& view);
    bool   EvOcViewBreakLink(TOcLinkView& view);
    bool   EvOcViewGetItemName(TOcItemName& item);
    /// @}

  private:
    bool   Destroying;

    // event handlers
    //
    bool     VnIsWindow(HWND hWnd);

  DECLARE_RESPONSE_TABLE(TOleView);
  DECLARE_STREAMABLE_OCF(TOleView,1);
};

DECLARE_STREAMABLE_INLINES( ocf::TOleView );

//
/// \class TOleLinkView
// ~~~~~ ~~~~~~~~~~~~
/// Derived from TView, TOleLinkView provides embedding and linking support for a
/// portion of a document instead of an entire document. With the added
/// functionality of TOleLinkView, a container gains the ability to embed or link to
/// a selection within the server document.
///
/// The main purpose of a class derived from TOleLinkView is to attach a view to a
/// portion of a document whenever a link is created to a selection within a server
/// document. After this link is established, any changes made to the linked
/// selection in the server document are sent to the container via the following
/// sequence of steps:
/// - 1.  When a user changes the server document, TOleLinkView receives a
/// notification message
/// - 2.  TOleLinkView checks to see if the selection it represents has changed. If
/// the selection has changed, TOleLinkView notifies TOcLinkView about the change.
/// - 3.  When TOcLinkView receives the change message, it notifies the container that
/// the selection has changed.
///
/// Non-Doc/View servers need to maintain a list of the TOleLinkViews attached to
/// the document so that change notifications can be sent to each one of the views.
class _OCFCLASS TOleLinkView : public owl::TView {
  public:
    TOleLinkView(owl::TDocument& doc, TOcLinkView& view);
   ~TOleLinkView();

    virtual bool VnLinkView(TOcLinkView& view);
    virtual bool VnLinkMoniker(owl::TString& moniker);
    virtual bool UpdateLinks();
        owl::TString& GetMoniker();

    static LPCTSTR StaticName();
    auto GetViewName() -> LPCTSTR override;

  protected:
    TOcLinkView&  OcLinkView;     // TOcLinkView partner for this view

  DECLARE_RESPONSE_TABLE(TOleLinkView);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

// --------------------------------------------------------------------------
// Inline implementation
//

//
/// Returns the constant string "Ole View" that is displayed in the user interface
/// selection box.
inline LPCTSTR TOleView::StaticName() {
  return _T("Ole View");
}

//
/// Overrides TView's virtual GetViewName function and returns the name of the class
/// (TOleView).
inline LPCTSTR TOleView::GetViewName() {
  return StaticName();
}

//
/// Overrides TView's virtual GetWindow function and returns the TWindow instance
/// associated with this view.
inline owl::TWindow* TOleView::GetWindow() {
  return (owl::TWindow*)this;
}

//
/// Overrides TView's and TWindow's virtual SetDocTitle function and stores the
/// title of the document associated with this view.
inline bool TOleView::SetDocTitle(LPCTSTR docname, int index) {
#  if defined(BI_COMP_BORLANDC)
  return owl::TWindow::SetDocTitle(docname, index);
#else
  return TWindow::SetDocTitle(docname, index);
#endif
}

//
inline LPCTSTR TOleView::GetFileName() {
  return GetDocument().GetTitle();
}

//
/// Returns the constant string "Link View" that is displayed in the user interface
/// selection box.
inline LPCTSTR TOleLinkView::StaticName() {
  return _T("Link View");
}

//
/// Overrides TView's virtual GetViewName function and returns the static name,
/// "Link View".
inline LPCTSTR TOleLinkView::GetViewName() {
  return StaticName();
}

} // OCF namespace

#endif  // OWL_OLEVIEW_H
