//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
/// \file
/// TOleWindow - Class encapsulating a window which can be an OLE container
///              or server window.
//----------------------------------------------------------------------------

#if !defined(OCF_OLEWINDO_H)
#define OCF_OLEWINDO_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/window.h>
#include <owl/dc.h>
#include <owl/uihelper.h>

#include <ocf/ocfevent.h>
#include <ocf/ocremvie.h>
#include <ocf/ocpart.h>
#include <ocf/oclink.h>

namespace owl {class _OWLCLASS TPopupMenu;};

namespace ocf {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
#include <owl/preclass.h>


class _ICLASS   TOcControl;

//
/// \class TOleWindow
// ~~~~~ ~~~~~~~~~~
/// The generic OLE2 window. Use as a client of a frame window.
//
/// Derived from TWindow, TOleWindow provides support for embedding objects in a
/// compound document and serves as the client of a frame window. A compound
/// document, such as the one TOleWindow supports, can contain many different types
/// of embedded objects, from spreadsheets to bitmaps. In addition to providing
/// support for a variety of basic window operations, TOleWindow also implements
/// several OLE-related operations, among them
/// - Responding to drag and drop events
/// - In-place editing (the process whereby an embedded object can be
/// edited without having to switch to its associated server application)
/// - Activating an embedded object's server application
/// - Creating views for the container application
/// - Transmitting a document's scaling information between a
/// container's and a server's view windows
///
/// TOleWindow has the ability to determine whether it's acting as a server or a
/// container. If it is a container, TOleWindow has a pointer to a TOcView or if it
/// is a server, TOleWindow establishes a pointer to a TOcRemView.  From the
/// server's point of view, every remote view has a corresponding TOleWindow.
///
/// Through its many event-handling member functions, TOleWindow communicates with
/// ObjectComponents to implement container and server support for embedded objects,
/// update views, and respond to a variety of menu commands associated with the
/// typical command identifiers (for example, CM_FILEMENU). It also supports
/// OLE-specific verbs such as those activated from the Edit menu (for example, Edit
/// and Open). These commands and verbs can originate from various sources such as a
/// menu selection, a radio button, or even an internal program message.
///
/// Conversely, ObjectComponents talks to ObjectWindows by means of the various
/// EV_OC_Xxxx messages. Some of these messages, such as EV_OC_VIEWPARTINVALID,
/// implement container support while others, such as EV_OC_VIEWCLOSE, implement
/// server support.
///
/// For any user-defined classes derived from TOleWindow, you need to choose which
/// functions are appropriate. If you want to provide additional server support, you
/// need to define only those functions that implement server messages; if you want
/// to provide container support, you need to define only those functions that
/// provide additional container support.
///
/// For example, the data embedded in the container application (a compound document
/// having one or more embedded objects) and the data embedded in the server
/// application (a single OLE object with or without other embedded objects) can be
/// written to storage and loaded  from storage. If you're using TOleWindow without
/// TOleView, you have to manipulate the storage by talking directly to the
/// ObjectComponents class, TOcDocument.
///
/// In addition to communicating with ObjectComponents classes, TOleWindow supports
/// many transactions as a result of its interaction with other ObjectWindows
/// classes. By virtue of its derivation from TWindow, naturally it inherits much of
/// TWindow's functionality.
class _OCFCLASS TOleWindow : virtual public owl::TWindow {
  public:
    TOleWindow(owl::TWindow* parent = 0, owl::TModule* module = 0);
   ~TOleWindow();

    // Accessors
    //
    TOcDocument*      GetOcDoc();
    TOcView*          GetOcView();
    TOcRemView*       GetOcRemView();
    TOcApp*           GetOcApp();
    bool              HasActivePart();
    bool              SelectEmbedded();

    /// \name Query about current state
    /// @{
    bool              IsOpenEditing() const;
    bool              IsRemote() const;
    /// @}

    /// \name Virtuals that derived class can override
    /// @{
    virtual bool      OleShutDown();
    virtual void      SetupDC(owl::TDC& dc, bool scale = true);
    virtual TOcView*  CreateOcView(owl::TRegLink* link, bool isRemote, IUnknown* outer);
    /// @}

    /// \name Helper routines to create new objects or controls [Container specific]
    /// @{
    TOcPart*          InsertObject(TOcInitInfo& initInfo, owl::TRect* pos = 0);
    TOcPart*          InsertObject(CLSID& objIID, owl::TRect* pos = 0);
    TOcPart*          InsertObject(owl::TString& objProgId, owl::TRect* pos = 0);

    TOcControl*       InsertControl(TOcInitInfo& initInfo, owl::TRect* pos = 0, int id = 0);
    TOcControl*       InsertControl(CLSID& ocxIID, owl::TRect* pos = 0, int id = 0);
    TOcControl*       InsertControl(const owl::TString& ocxProgID, owl::TRect* pos = 0, int id = 0);
    /// @}

    /// \name Helper routines to retrieve information about parts (embeddings/ocxes)
    /// @{
    TOcControl*       GetOcControlOfOCX(CLSID ocxIID, owl::uint id = 0);
    /// @}

  protected:
    /// \name Overridables
    /// @{
    virtual void      GetInsertPosition(owl::TRect& rect);
    virtual void      InvalidatePart(TOcInvalidate invalid); // server only
    virtual bool      Deactivate();
    virtual bool      Select(owl::uint modKeys, owl::TPoint& point);
    virtual bool      PaintSelection(owl::TDC& dc, bool erase, owl::TRect& rect,
                                     void* userData = 0);
    virtual bool      PaintParts(owl::TDC& dc, bool erase, owl::TRect& rect, bool metafile);
    virtual bool      PaintLink(owl::TDC& dc, bool erase, owl::TRect& rect,
                                owl::TString& moniker);
    virtual void      GetLogPerUnit(owl::TSize& logPerUnit);
    virtual void      SetScale(owl::uint16 percent);
    virtual void      SetSelection(TOcPart* part);
    virtual bool      ShowCursor(HWND wnd, owl::uint hitTest, owl::uint mouseMsg);
    virtual bool      StartDrag(owl::uint modKeys, owl::TPoint& point);
    virtual LPCTSTR    GetFileName();
    /// @}

  protected:
    // Override virtual functions defined by owl::TWindow
    //
    void SetupWindow() override;
    void CleanupWindow() override;
    auto CanClose() -> bool override;

    bool              InClient(owl::TDC& dc, owl::TPoint& point);

    auto EvCommand(owl::uint id, HWND hWndCtl, owl::uint notifyCode) -> owl::TResult override;
    void EvCommandEnable(owl::TCommandEnabler&) override;
    void              EvPaint();
    void              EvSize(owl::uint sizeType, const owl::TSize& size);
    void              EvMDIActivate(HWND hWndActivated, HWND hWndDeactivated);
    owl::uint              EvMouseActivate(HWND topParent, owl::uint hitCode, owl::uint msg);
    void              EvSetFocus(HWND hWndLostFocus);

    virtual bool      EvOcViewPartInvalid(TOcPartChangeInfo& changeInfo);

    void              CeFileClose(owl::TCommandEnabler& ce);
    void              CmEditDelete();
    void              CmEditCut();
    void              CmEditCopy();
    void              CmEditPaste();
    void              CmEditPasteSpecial();
    void              CmEditPasteLink();
    void              CmEditLinks();
    void              CmEditConvert();
    void              CmEditInsertObject();
    void              CmEditInsertControl();
    void              CmEditShowObjects();

    void              CeEditDelete(owl::TCommandEnabler& ce);
    void              CeEditCut(owl::TCommandEnabler& ce);
    void              CeEditCopy(owl::TCommandEnabler& ce);
    void              CeEditPaste(owl::TCommandEnabler& ce);
    void              CeEditPasteSpecial(owl::TCommandEnabler& ce);
    void              CeEditPasteLink(owl::TCommandEnabler& ce);
    void              CeEditLinks(owl::TCommandEnabler& ce);
    void              CeEditObject(owl::TCommandEnabler& ce);
    void              CeEditConvert(owl::TCommandEnabler& ce);
    void              CeEditVerbs(owl::TCommandEnabler& ce);
    void              CeEditInsertObject(owl::TCommandEnabler& ce);
    void              CeEditInsertControl(owl::TCommandEnabler& ce);
    void              CeEditShowObjects(owl::TCommandEnabler& ce);

    void              EvMouseMove(owl::uint modKeys, const owl::TPoint& point);
    void              EvRButtonDown(owl::uint modKeys, const owl::TPoint& point);
    void              EvLButtonUp(owl::uint modKeys, const owl::TPoint& point);
    void              EvLButtonDown(owl::uint modKeys, const owl::TPoint& point);
    void              EvLButtonDblClk(owl::uint modKeys, const owl::TPoint& point);
    bool              EvSetCursor(HWND hWndCursor, owl::uint hitTest, owl::uint mouseMsg);
    void              EvDropFiles(owl::TDropInfo dropInfo);
    void              EvHScroll(owl::uint scrollCode, owl::uint thumbPos, HWND hWndCtl);
    void              EvVScroll(owl::uint scrollCode, owl::uint thumbPos, HWND hWndCtl);
    void              EvMenuSelect(owl::uint menuItemId, owl::uint flags, HMENU hMenu);

    owl::TResult           EvOcEvent(owl::TParam1 param1, owl::TParam2 param2);
/// owl::uint32  EvOcViewDiscardUndo(void * undo);

    /// \name Container specific messages
    /// @{
    LPCTSTR            EvOcViewTitle();
    void              EvOcViewSetTitle(LPCTSTR title);

    bool              EvOcViewBorderSpaceReq(owl::TRect * rect);
    bool              EvOcViewBorderSpaceSet(owl::TRect * rect);
    bool              EvOcViewDrop(TOcDragDrop & ddInfo);
    bool              EvOcViewDrag(TOcDragDrop & ddInfo);
    bool              EvOcViewScroll(TOcScrollDir scrollDir);
    bool              EvOcViewGetScale(TOcScaleFactor& scaleFactor);
    bool              EvOcViewGetSiteRect(owl::TRect * rect);
    bool              EvOcViewSetSiteRect(owl::TRect * rect);
    bool              EvOcViewPartActivate(TOcPart& ocPart);
    bool              EvOcViewPasteObject(TOcInitInfo& init);
    /// @}

    /// \name Server specific messages
    /// @{
    bool              EvOcPartInvalid(TOcPart & /*part*/);
    bool              EvOcViewSavePart(TOcSaveLoad & ocSave);
    bool              EvOcViewLoadPart(TOcSaveLoad & ocLoad);
    bool              EvOcViewPaint(TOcViewPaint & vp);
    bool              EvOcViewInsMenus(TOcMenuDescr & sharedMenu);
    bool              EvOcViewShowTools(TOcToolBarInfo & tbi);
    bool              EvOcViewGetPalette(LOGPALETTE * * palette);
    bool              EvOcViewClipData(TOcFormatData & format);
    bool              EvOcViewSetData(TOcFormatData & format);
    bool              EvOcViewClose();
    bool              EvOcViewPartSize(TOcPartSize & size);
    bool              EvOcViewOpenDoc(LPCTSTR path);
    bool              EvOcViewAttachWindow(bool attach);
    bool              EvOcViewSetScale(TOcScaleFactor& scaleFactor);
    bool              EvOcViewGetItemName(TOcItemName& item);
    bool              EvOcViewSetLink(TOcLinkView& view);
    bool              EvOcViewBreakLink(TOcLinkView& view);
    bool              EvOcViewDoVerb(owl::uint verb);

    virtual bool      EvOcViewTransformCoords(owl::uint verb);
    /// @}

    /// \name Ambient property specific messages
    /// @{
    virtual bool      EvOcAmbientGetBackColor(long* rgb);
    virtual bool      EvOcAmbientGetForeColor(long* rgb);
    virtual bool      EvOcAmbientGetLocaleID(long* locale);
    virtual bool      EvOcAmbientGetTextAlign(short* align);
    virtual bool      EvOcAmbientGetMessageReflect(bool* msgReflect);
    virtual bool      EvOcAmbientGetUserMode(bool* mode);
    virtual bool      EvOcAmbientGetUIDead(bool* dead);
    virtual bool      EvOcAmbientGetShowGrabHandles(bool* show);
    virtual bool      EvOcAmbientGetShowHatching(bool* show);
    virtual bool      EvOcAmbientGetDisplayAsDefault(bool* disp);
    virtual bool      EvOcAmbientGetSupportsMnemonics(bool* support);
    virtual bool      EvOcAmbientGetDisplayName(owl::TString** name);
    virtual bool      EvOcAmbientGetScaleUnits(owl::TString** units);
    virtual bool      EvOcAmbientGetFont(IDispatch** font);

    virtual bool      EvOcAmbientSetBackColor(long rgb);
    virtual bool      EvOcAmbientSetForeColor(long rgb);
    virtual bool      EvOcAmbientSetLocaleID(long locale);
    virtual bool      EvOcAmbientSetTextAlign(short align);
    virtual bool      EvOcAmbientSetMessageReflect(bool msgReflect);
    virtual bool      EvOcAmbientSetUserMode(bool mode);
    virtual bool      EvOcAmbientSetUIDead(bool dead);
    virtual bool      EvOcAmbientSetShowGrabHandles(bool show);
    virtual bool      EvOcAmbientSetShowHatching(bool show);
    virtual bool      EvOcAmbientSetDisplayAsDefault(bool disp);
    virtual bool      EvOcAmbientSetSupportsMnemonics(bool support);
    virtual bool      EvOcAmbientSetDisplayName(owl::TString* name);
    virtual bool      EvOcAmbientSetScaleUnits(owl::TString* units);
    virtual bool      EvOcAmbientSetFont(IDispatch* font);
    /// @}

    /// \name Ctrl event specific messages
    /// @{
    virtual bool      EvOcCtrlClick(TCtrlEvent* pev);
    virtual bool      EvOcCtrlDblClick(TCtrlEvent* pev);
    virtual bool      EvOcCtrlMouseDown(TCtrlMouseEvent* pev);
    virtual bool      EvOcCtrlMouseMove(TCtrlMouseEvent* pev);
    virtual bool      EvOcCtrlMouseUp(TCtrlMouseEvent* pev);
    virtual bool      EvOcCtrlKeyDown(TCtrlKeyEvent* pev);
    virtual bool      EvOcCtrlKeyUp(TCtrlKeyEvent* pev);
    virtual bool      EvOcCtrlErrorEvent(TCtrlErrorEvent* pev);
    virtual bool      EvOcCtrlFocus(TCtrlFocusEvent* pev);
    virtual bool      EvOcCtrlPropertyChange(TCtrlPropertyEvent* pev);
    virtual bool      EvOcCtrlPropertyRequestEdit(TCtrlPropertyEvent* pev);
    virtual bool      EvOcCtrlCustomEvent(TCtrlCustomEvent* pev);
    /// @}

    owl::TPopupMenu*        CreateVerbPopup(const TOcVerb& ocVerb);
    void              EvDoVerb(owl::uint whichVerb);
    void              Init();

  protected:
/// Points to the embedded object (the part) being dragged.
    TOcPart*          DragPart;

/// Indicates the position in the embedded object where the user points and clicks
/// the mouse. This can be any one of the TUIHandle::TWhere enumerated values; for
/// example, TopLeft, TopCenter, TopRight, MidLeft, MidCenter, MidRight, BottomLeft,
/// BottomCenter, BottomRight, or Outside when no dragging is taking place.
    owl::TUIHandle::TWhere DragHit;

/// Points to the device context used while an object is being dragged.
    owl::TDC*              DragDC;

/// Indicates the point (in logical units) where the mouse is over the dragged object.
    owl::TPoint            DragPt;

/// Holds the rectangle being dragged.
    owl::TRect              DragRect;

/// Holds the point where the dragging of the embedded object began.
    owl::TPoint            DragStart;

/// Holds the current scaling factor. The server uses this information to determine how to scale the document.
    TOcScaleFactor    Scale;

/// Holds the current area in the window where the object is embedded. Pos reflects
/// the area where the object is moved if you move the object.
    owl::TRect              Pos;

/// Holds the ObjectComponents document associated with this TOleWindow.
    TOcDocument*      OcDoc;

/// Holds the ObjectComponents view or remote view (the server's) associated with the TOleWindow view.
    TOcView*          OcView;

/// Holds the ObjectComponents application associated with this TOleWindow.
    TOcApp*           OcApp;

/// Holds the name of the container. The server displays the container's name when an
/// embedded object is being edited in the server's window (referred to as out-of-place editing).
    owl::tstring        ContainerName;

/// Returns true if the window represents an embedded server. Returns false otherwise.
    bool              Remote;

/// Is true if the embedded object's frame (the gray or shaded brushes around the object)
/// is displayed. The frame can be turned on or off depending on how you want the object to appear.
    bool              ShowObjects;

/// The minimum width of the part.
    int               MinWidth;

/// The minimum height of the part.
    int               MinHeight;

  DECLARE_RESPONSE_TABLE(TOleWindow);
  DECLARE_STREAMABLE_OCF(TOleWindow,1);
};

DECLARE_STREAMABLE_INLINES( ocf::TOleWindow );

//
/// \class TOleClientDC
// ~~~~~ ~~~~~~~~~~~~
/// Derived from TClientDC, TOleClientDC is a helper class that translates between
/// two different coordinate systems. For example, the window's logical points may
/// be measured in HIMETRIC or twips, whereas the coordinates of the actual output
/// device (the viewport) may be measured in pixels. Without the help of this class,
/// you would need to create a client DC and then setup the window's logical
/// coordinates (its origin) and its width and height (its extent) as well as the
/// viewport's origin (measured in device coordinates) and extent. Instead,
/// TOleClientDC performs these calculations for you by mapping logical points to
/// device points and vice versa.
/// TOleClientDC works with a TOleWindow object. By default, TOleClientDC takes care
/// of both scaling (adjusting the extents of the window and the viewport) and
/// scrolling (adjusting the origins of the window and the viewport).
//
class _OCFCLASS TOleClientDC : public owl::TClientDC {
  public:
    TOleClientDC(TOleWindow& win, bool scale = true);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

// --------------------------------------------------------------------------
// Inline implementation
//

//
/// Returns the ObjectComponents document associated with this window. This document
/// can be either a container's or a server's document.  If this is a TOcDocument
/// created by the container, the document is an entire compound document, which may
/// consist of one or more embedded objects. If this is a TOcDocument created by the
/// server, the document is a single OLE object's data.
//
inline TOcDocument* TOleWindow::GetOcDoc() {
  return OcDoc;
}

//
/// Points to the ObjectComponents container view associated with this window. The
/// container view holds the compound document (that is a document containing one or
/// more embedded objects).
//
inline TOcView* TOleWindow::GetOcView() {
  return OcView;
}

//
/// Returns the server's view associated with this window. In order to draw the OLE
/// object in the container's window, the server creates a remote view.
///
/// \note Returns 0 in case of failure. For example, a TOleWindow of a
///       container application does not have an associated 'TOcRemView'
///       and will return 0.
//
inline TOcRemView* TOleWindow::GetOcRemView() {
  return TYPESAFE_DOWNCAST(OcView, TOcRemView);
}

//
/// Returns the ObjectComponents application associated with this window. Every
/// ObjectComponents application that supports linking and embedding has an
/// associated TOcApp object.
inline TOcApp* TOleWindow::GetOcApp() {
  return OcApp;
}

//
/// Selects the embedded object and returns true to indicate that the object has
/// been selected.
inline bool TOleWindow::SelectEmbedded() {
  return DragPart != 0;
}

//
/// Returns true if the window represents an embedded server. Returns false
/// otherwise.
inline bool TOleWindow::IsRemote() const {
  return Remote;
}

//
/// Repaints the selected portion of a document on the given device context. The
/// parameter userData, which TocDataProvider passes to the application, contains
/// information the application uses to repaint the selection. Similar to a moniker,
/// this information tells the application where to find the selection and how to
/// repaint it.
inline bool TOleWindow::PaintSelection(owl::TDC& /*dc*/, bool /*erase*/,
                                       owl::TRect& /*rect*/, void* /*userData*/) {
  return false;
}

//
/// Repaints part of an object on the given device context. The erase parameter is
/// true if the background of the part is to be repainted. rect indicates the area
/// that needs repainting. metafile indicates whether or not the part is a metafile.
/// By default, PaintLink returns false. If your application supports working with
/// parts of an object, you should override the PaintLink function, providing your
/// own implementation for painting an object part.
/// For more information, see Step 17 of the OWL tutorial.
inline bool TOleWindow::PaintLink(owl::TDC& /*dc*/, bool /*erase*/,
                                  owl::TRect& /*rect*/, owl::TString& /*moniker*/) {
  return false;
}

//
/// Handles a WM_OCEVENT message concerning the embedded or linked object in the
/// document and invalidates the part.
/// If the TOleWindow object is unable to handle the message, EvOcPartInvalid
/// returns false.
inline bool TOleWindow::EvOcPartInvalid(TOcPart & /*part*/) {
  return false;
}

/// On linking must be implemented and return the file name
inline LPCTSTR TOleWindow::GetFileName() {
  return nullptr;
}

} // OCF namespace

#endif
