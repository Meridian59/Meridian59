//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TOleWindow. Window class that supports OLE 2
/// container & server using OCF TOcView & TOcRemView
//----------------------------------------------------------------------------
#include <ocf/pch.h>

#include <owl/window.h>
#include <owl/gdiobjec.h>
#include <owl/scroller.h>
#include <ocf/ocdoc.h>
#include <ocf/ocview.h>
#include <ocf/occtrl.h>
#include <ocf/oleframe.h>
#include <ocf/olewindo.h>
#include <owl/uimetric.h>
#include <owl/edit.rh>
#include <owl/docview.rh>
#include <ocf/oleview.rh>

namespace ocf {

using namespace owl;

using namespace std;

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OcfRemView);//????????????????????????????????????????????????????????

///----------------------------------------------------------------------------
/// Size of rectangle in two clks must occur to be a doubleClick
/// ---- moved in StartDrag ----
///  TSize DblClkDelta(TUIMetric::CxDoubleClk/2,
///                    TUIMetric::CyDoubleClk/2);

//----------------------------------------------------------------------------
// TOleClientDC
//

//
/// Constructs a TOleClientDC object . The parameter win references the window that
/// TOleClientDC uses to create a device context (DC). If the scale parameter is
/// true, TOleClientDC takes care of scaling. However, if your application handles
/// scaling, you can pass scale as false.
/// Scrolling is controlled by the presence of a scroller (TScroller). TOleClientDC
/// by default takes care of both scaling and scrolling.
//
TOleClientDC::TOleClientDC(TOleWindow& win, bool scale)
:
  TClientDC(win)
{
  win.SetupDC(*this, scale);
}

//----------------------------------------------------------------------------
// TOleWindow
//

DEFINE_RESPONSE_TABLE1(TOleWindow, TWindow)
  EV_WM_PAINT,
  EV_WM_LBUTTONDOWN,
  EV_WM_RBUTTONDOWN,
  EV_WM_LBUTTONDBLCLK,
  EV_WM_MOUSEMOVE,
  EV_WM_LBUTTONUP,
  EV_WM_SIZE,
  EV_WM_MDIACTIVATE,
  EV_WM_MOUSEACTIVATE,
  EV_WM_SETFOCUS,
  EV_WM_SETCURSOR,
  EV_WM_DROPFILES,
  EV_WM_VSCROLL,
  EV_WM_HSCROLL,
  EV_WM_MENUSELECT,

  EV_COMMAND_ENABLE(CM_FILECLOSE, CeFileClose),

  EV_COMMAND(CM_EDITDELETE, CmEditDelete),
  EV_COMMAND_ENABLE(CM_EDITDELETE, CeEditDelete),
  EV_COMMAND(CM_EDITCUT, CmEditCut),
  EV_COMMAND_ENABLE(CM_EDITCUT, CeEditCut),
  EV_COMMAND(CM_EDITCOPY, CmEditCopy),
  EV_COMMAND_ENABLE(CM_EDITCOPY, CeEditCopy),
  EV_COMMAND(CM_EDITPASTE, CmEditPaste),
  EV_COMMAND_ENABLE(CM_EDITPASTE, CeEditPaste),
  EV_COMMAND(CM_EDITPASTESPECIAL, CmEditPasteSpecial),
  EV_COMMAND_ENABLE(CM_EDITPASTESPECIAL, CeEditPasteSpecial),
  EV_COMMAND(CM_EDITPASTELINK, CmEditPasteLink),
  EV_COMMAND_ENABLE(CM_EDITPASTELINK, CeEditPasteLink),
  EV_COMMAND(CM_EDITINSERTOBJECT, CmEditInsertObject),

  EV_COMMAND(CM_EDITINSERTCONTROL, CmEditInsertControl),
  EV_COMMAND_ENABLE(CM_EDITINSERTOBJECT, CeEditInsertObject),
  EV_COMMAND_ENABLE(CM_EDITINSERTCONTROL, CeEditInsertControl),

  EV_COMMAND_ENABLE(CM_EDITLINKS, CeEditLinks),
  EV_COMMAND(CM_EDITLINKS, CmEditLinks),
  EV_COMMAND_ENABLE(CM_EDITOBJECT, CeEditObject),
  EV_COMMAND_ENABLE(CM_EDITCONVERT, CeEditConvert),
  EV_COMMAND(CM_EDITCONVERT, CmEditConvert),
  EV_COMMAND_ENABLE(CM_EDITSHOWOBJECTS, CeEditShowObjects),
  EV_COMMAND(CM_EDITSHOWOBJECTS, CmEditShowObjects),

  EV_MESSAGE(WM_OCEVENT, EvOcEvent),
///  EV_OC_VIEWDISCARDUNDO,  // !CQ not processing this yet...

  // Container specific messages
  //
  EV_OC_VIEWPARTINVALID,
  EV_OC_VIEWTITLE,
  EV_OC_VIEWSETTITLE,
  EV_OC_VIEWBORDERSPACEREQ,
  EV_OC_VIEWBORDERSPACESET,
  EV_OC_VIEWDROP,
  EV_OC_VIEWDRAG,
  EV_OC_VIEWSCROLL,
  EV_OC_VIEWGETSCALE,
  EV_OC_VIEWGETSITERECT,
  EV_OC_VIEWSETSITERECT,
  EV_OC_VIEWPARTACTIVATE,
  EV_OC_VIEWPASTEOBJECT,

  // Server specific messages
  //
  EV_OC_VIEWPAINT,
  EV_OC_VIEWSAVEPART,
  EV_OC_VIEWLOADPART,
  EV_OC_VIEWINSMENUS,
  EV_OC_VIEWSHOWTOOLS,
  EV_OC_VIEWGETPALETTE,
  EV_OC_VIEWCLIPDATA,
  EV_OC_VIEWSETDATA,
  EV_OC_VIEWCLOSE,
  EV_OC_VIEWPARTSIZE,
  EV_OC_VIEWOPENDOC,
  EV_OC_VIEWATTACHWINDOW,
  EV_OC_VIEWSETSCALE,
  EV_OC_VIEWGETITEMNAME,
  EV_OC_VIEWSETLINK,
  EV_OC_VIEWBREAKLINK,
  EV_OC_VIEWDOVERB,

  // Ambient properties
  //
  EV_OC_AMBIENT_GETBACKCOLOR,
  EV_OC_AMBIENT_GETFORECOLOR,
  EV_OC_AMBIENT_GETLOCALEID,
  EV_OC_AMBIENT_GETTEXTALIGN,
  EV_OC_AMBIENT_GETMESSAGEREFLECT,
  EV_OC_AMBIENT_GETUSERMODE,
  EV_OC_AMBIENT_GETUIDEAD,
  EV_OC_AMBIENT_GETSHOWGRABHANDLES,
  EV_OC_AMBIENT_GETSHOWHATCHING,
  EV_OC_AMBIENT_GETDISPLAYASDEFAULT,
  EV_OC_AMBIENT_GETSUPPORTSMNEMONICS,
  EV_OC_AMBIENT_GETDISPLAYNAME,
  EV_OC_AMBIENT_GETSCALEUNITS,
  EV_OC_AMBIENT_GETFONT,
  EV_OC_AMBIENT_SETBACKCOLOR,
  EV_OC_AMBIENT_SETFORECOLOR,
  EV_OC_AMBIENT_SETLOCALEID,
  EV_OC_AMBIENT_SETTEXTALIGN,
  EV_OC_AMBIENT_SETMESSAGEREFLECT,
  EV_OC_AMBIENT_SETUSERMODE,
  EV_OC_AMBIENT_SETUIDEAD,
  EV_OC_AMBIENT_SETSHOWGRABHANDLES,
  EV_OC_AMBIENT_SETSHOWHATCHING,
  EV_OC_AMBIENT_SETDISPLAYASDEFAULT,
  EV_OC_AMBIENT_SETSUPPORTSMNEMONICS,
  EV_OC_AMBIENT_SETDISPLAYNAME,
  EV_OC_AMBIENT_SETSCALEUNITS,
  EV_OC_AMBIENT_SETFONT,

  // Standard Events
  //
  EV_OC_CTRLEVENT_CLICK,
  EV_OC_CTRLEVENT_DBLCLICK,


  // Ctrl events
  //
  EV_OC_CTRLEVENT_CLICK,
  EV_OC_CTRLEVENT_DBLCLICK,
  EV_OC_CTRLEVENT_FOCUS,
  EV_OC_CTRLEVENT_MOUSEDOWN,
  EV_OC_CTRLEVENT_MOUSEMOVE,
  EV_OC_CTRLEVENT_MOUSEUP,
  EV_OC_CTRLEVENT_KEYDOWN,
  EV_OC_CTRLEVENT_KEYUP,
  EV_OC_CTRLEVENT_PROPERTYCHANGE,
  EV_OC_CTRLEVENT_PROPERTYREQUESTEDIT,
  EV_OC_CTRLEVENT_ERROREVENT,
  EV_OC_CTRLEVENT_CUSTOMEVENT,

END_RESPONSE_TABLE;

//
/// Constructs a TOleWindow object associated with the specified parent window and
/// module instance.
//
TOleWindow::TOleWindow(TWindow* parent, TModule* module)
:
  TWindow(parent, 0, module),
  Pos(0, 0, 0, 0),
  Remote(false),
  ShowObjects(false)
{
  // Initialize virtual base, in case the derived-most used default ctor
  //
  TWindow::Init(parent, 0, module);

  // Derived class will need to create a OcDocument object to hold the OLE
  // parts that we create and a OcView to provide OLE services
  //
  OcApp  = 0;
  OcDoc  = 0;
  OcView = 0;

  Init();
}

//
/// Initializes the TOleWindow object with the appropriate window style and
/// initializes the necessary data members (for example, sets the accelerator ID to
/// IDA_OLEVIEW).
//
void
TOleWindow::Init()
{
  // Clip children to not paint on in-place servers, & clip siblings to not
  // paint on floating tool palettes
  //
  Attr.Style |= WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

  Attr.AccelTable = IDA_OLEVIEW;
  DragHit   = TUIHandle::Outside;
  DragPart  = 0;
  DragDC    = 0;

  // Minimum part size
  //
  HDC dc = GetDC(0);
  MinWidth  = GetDeviceCaps(dc, LOGPIXELSX) / 4;
  MinHeight = GetDeviceCaps(dc, LOGPIXELSY) / 4;
  ReleaseDC(0, dc);

  // Snag the OcApp from the TOleFrame object for quick reference
  //
  TOleFrame* olefr = TYPESAFE_DOWNCAST(GetApplication()->GetMainWindow(), TOleFrame);
  CHECK(olefr);
  OcApp = olefr->GetOcApp();
  OcApp->AddRef();
}

//
/// Checks to see if there are any open views, and, if no open views exist, destroys
/// the TOleWindow object.
//
TOleWindow::~TOleWindow()
{
  // Let the OC objects go. They will delete themselves when they can
  // If it's a remote view, then the last release is called by the container.
  //
  if (OcDoc)
    OcDoc->Close();           // close all the embedded parts first
  if (OcView && !IsRemote()) {
    OcView->ReleaseObject();  // use ReleaseObject here to tell it we are gone
    OcView = 0;
  }

  delete OcDoc;
  if (OcApp) {
    OcApp->Release();
    OcApp = 0;
  }
}

//
/// Establishes a connection between the TOcView object and the view's HWND so the
/// view can send notification messages to the window.
//
void
TOleWindow::SetupWindow()
{
  if (!OcView)
    CreateOcView(0, false, 0);  // don't have much context by this time
  CHECK(OcView);
  TWindow::SetupWindow();
  OcView->SetupWindow(*this, IsRemote());
}

//
/// Called to perform the actual setting up of the OcView member
//
/// Creates an ObjectComponents view associated with the embedded object. Associates
/// the view with the document template specified in tpl. If isEmbedded is true, a
/// remote view is created (that is, a TOcRemView instead of a TOcView). The outer
/// parameter refers to the IUnknown interface with which the view will aggregate
/// itself.
///
/// Derived class needs to construct the TOcView/TOcRemView here & return it
//
TOcView*
TOleWindow::CreateOcView(TRegLink* link, bool isRemote, IUnknown* outer)
{
  if (!OcDoc)
    OcDoc = new TOcDocument(*GetOcApp());

  if (!OcView) {
    TRegList* regList = link ? &link->GetRegList() : 0;

    // Create a remote view on the server document if it embeded, else make a
    // normal container view [capable of hosting OCXes or 'normal' servers]
    //
    if (isRemote)
      OcView = new TOcRemView(*GetOcDoc(), regList, outer);
    else

    // By default, our 32-bit container support can 'hold' embedded objects
    // and OCX controls. So we'll default to a 'TOcxView'. However, this
    // can be optimized if the container will never contain any OLE controls.
    //
#if !defined(OWL_NO_OCX_CONTAINER_SUPPORT)
      OcView = new TOcxView(*GetOcDoc(), regList, outer);
#endif

    Remote = isRemote;
  }
  return OcView;
}

//
/// Overrides the usual EvCommandEnable message in order to enable the OLE verbs
/// from CM_EDITFIRSTVERB to CM_EDITLASTVERB. These commands enable the OLE-specific
/// Edit menu selections, such as Edit, Open, and Play. Many of the other commands
/// are passed to TWindow::EvCommand for normal processing.
/// If a window is embedded, however, TOleWindow calls upon TWindow's
/// RouteCommandEnable to perform command enabling.
//
void
TOleWindow::EvCommandEnable(TCommandEnabler& commandEnabler)
{
  if (CM_EDITFIRSTVERB <= commandEnabler.GetId() && commandEnabler.GetId() <= CM_EDITLASTVERB) {
    CeEditVerbs(commandEnabler);
  }
  else if (IsRemote()) {
    // Get the focus, in case it is a child that should receive the cmds
    // fall back to this window in case the Ole menu bar stole focus!
    //
    THandle  hCmdTarget = ::GetFocus();
    if (hCmdTarget != GetHandle() && !IsChild(hCmdTarget))
      hCmdTarget = GetHandle();

    RouteCommandEnable(hCmdTarget, commandEnabler);
  }
  else {
    TWindow::EvCommandEnable(commandEnabler);
  }
}

//
/// Overrides the usual EvCommand message to handle the OLE verbs from
/// CM_EDITFIRSTVERB to CM_EDITLASTVERB. These commands, which are defined in
/// oleview.rh, correspond to the OLE-specific Edit menu selections such as Edit,
/// Open, and Play. All of the other commands are passed to TWindow::EvCommand for
/// normal processing.
//
TResult
TOleWindow::EvCommand(uint id, THandle hCtl, uint notifyCode)
{
  TRACEX(OcfRemView, 1, _T("TOleWindow::EvCommand - id(") << id << _T("), ctl(") <<\
    hex << static_cast<void*>(hCtl) << _T("), code(") << notifyCode << _T(")"));

  if (hCtl == 0) {
    if (CM_EDITFIRSTVERB <= id && id <= CM_EDITLASTVERB) {
      EvDoVerb(id - CM_EDITFIRSTVERB);
      return 0;
    }
  }
  return TWindow::EvCommand(id, hCtl, notifyCode);
}

//
/// Intercept CanClose() to interpose OpenEdit semantics. OLE2 servers don't
/// prompt the user on close--just save & close.
///
/// Returns true if the window can be closed. Checks all the server's child windows'
/// CanClose functions, which must return true before the window can be closed.
/// Terminates any open editing transactions before closing the window; otherwise,
/// passes control to TWindow::CanClose.
///
bool
TOleWindow::CanClose()
{
  // We don't want to close the view for DLL servers
  //
  if (IsOpenEditing() && !OcApp->IsOptionSet(amExeMode)) {
    TOleFrame* olefr = TYPESAFE_DOWNCAST(GetApplication()->GetMainWindow(), TOleFrame);
    CHECK(olefr);
    olefr->ShowWindow(SW_HIDE);
    OleShutDown();
    return false;
  }

  if (GetOcRemView() || TWindow::CanClose()) {
    if (OcDoc)
      OcDoc->Close();
    return true;
  }
  return false;
}

//
/// Invalidates the area where the embedded object exists. The server uses this
/// function to tell OLE that the part (the embedded object), has changed. OLE then
/// asks the server to redraw the part into a new metafile so that OLE can redraw
/// the object for the container application even when the server application is not
/// active.
//
void
TOleWindow::InvalidatePart(TOcInvalidate invalid)
{
  if (GetOcRemView())
    GetOcRemView()->Invalidate(invalid);
}

//
/// Checks whether the window is in Open-Edit mode.
//
bool
TOleWindow::IsOpenEditing() const
{
  TOcRemView* ocRemView = const_cast<TOleWindow*>(this)->GetOcRemView();
  return ocRemView && ocRemView->GetState() == TOcRemView::OpenEditing;
}

//
//
//
TOcPart*
TOleWindow::InsertObject(TOcInitInfo& initInfo, TRect* pos)
{
  TRect rect;

  // Pass size request on if caller does not specify
  //
  if (!pos) {
    pos = &rect;
    GetInsertPosition(*pos);
  }

  TOcPart* part;
  try {
    part = new TOcPart (*GetOcDoc());
    part->Init(&initInfo, *pos);
    SetSelection(part);
    OcView->Rename();
    InvalidatePart(invView);
  }
  catch (TXOle& /*xole*/) {
    return 0;
  }
  return part;
}

//
//
//
TOcPart*
TOleWindow::InsertObject(CLSID& objIID, TRect* pos)
{
  TOcInitInfo initInfo(OcView);

  // Initialize InitInfo structure
  //
  initInfo.CId   = (BCID) &objIID;    // Object's GUID
  initInfo.Where = iwNew;             // Flag it's a new embedding
  initInfo.How   = ihEmbed;           // Want embedded

  return InsertObject(initInfo, pos);
}

//
//
//
TOcPart*
TOleWindow::InsertObject(TString& objProgId, TRect* pos)
{
  // Get IID of OCX from specified ProgID
  //
  CLSID objIID = CLSID_NULL;
  HRESULT hr = ::CLSIDFromProgID(objProgId, &objIID);

  if (FAILED(hr)) {
    return 0;
  }
  return InsertObject(objIID, pos);
}


//
//
//
TOcControl*
TOleWindow::InsertControl(TOcInitInfo& initInfo, TRect* pos, int id)
{
  TRect rect;

  // Pass size request on if caller does not specify
  //
  if (!pos) {
    pos = &rect;
    GetInsertPosition(*pos);
  }

  // If user did not specify the size (i.e. right & bottom), use
  // some default values...
  //
  if (pos->left == pos->right)
    pos->right = pos->left + 100;
  if (pos->top == pos->bottom)
    pos->bottom = pos->top + 100;

  TOcControl* ctrl;
  try {
    ctrl = new TOcControl (*GetOcDoc());
    ctrl->Init(&initInfo, *pos);
    SetSelection(ctrl);
    OcView->Rename();
    InvalidatePart(invView);
    ctrl->SetId(id);
  }
  catch (TXOle& /*xole*/) {
    return 0;
  }
  return ctrl;
}

//
//
//
TOcControl*
TOleWindow::InsertControl(CLSID& ocxIID, TRect* pos, int id)
{
  TOcInitInfo initInfo(OcView);

  // Initialize InitInfo structure
  //
  initInfo.CId   = (BCID) &ocxIID;    // OCX's GUID
  initInfo.Where = iwNewOcx;          // Flag it's an OCX
  initInfo.How   = ihEmbed;           // Want embedded

  return InsertControl(initInfo, pos, id);
}

//
//
//
TOcControl*
TOleWindow::InsertControl(const TString& ocxProgID, TRect* pos, int id)
{
  // Get IID of OCX from specified ProgID
  //
  CLSID ocxIID = CLSID_NULL;
  HRESULT hr = ::CLSIDFromProgID(ocxProgID, &ocxIID);

  if (FAILED(hr)) {
    return 0;
  }
  return InsertControl(ocxIID, pos, id);
}


//
//
//
TOcControl*
TOleWindow::GetOcControlOfOCX(CLSID ocxIID, uint id)
{
  // Iterate through all parts of this window
  //
  for (TOcPartCollectionIter i(GetOcDoc()->GetParts()); i; i++) {
    TOcPart* p = *i;

    // NOTE: Here we'll simply dynamic_cast the TOcPart pointer to a
    //       TOcControl. A more OLEish approach would be to querry for
    //       a control interface [IBControl for example] as a safety
    //       measure first.
    //
    TOcControl* pCtrl = TYPESAFE_DOWNCAST(p, TOcControl);
    if (pCtrl) {
      IOleObject* pOleObject = 0;
      if (SUCCEEDED(pCtrl->QueryServer(IID_IOleObject, (void**)&pOleObject))) {

        // !BB Could add a call to pOleObject->GetUserType to retrieve
        // !BB the name of the control for debugging purposes...
        CLSID userClsId;
        if (SUCCEEDED(pOleObject->GetUserClassID(&userClsId)) &&
                      userClsId == ocxIID) {
          if (id == 0 || pCtrl->GetId() == id) {
            pOleObject->Release();
            return pCtrl;
          }
        }
        pOleObject->Release();
      }
    }
  }

  // Control proxy was not found
  //
  return 0;
}


//
/// Enables the FileClose command, which lets the user exit from the window view.
//
void
TOleWindow::CeFileClose(TCommandEnabler& ce)
{
  // if open editing server, reflect the container's name in "close" option
  //
  if (IsOpenEditing() && GetOcRemView() && GetOcRemView()->GetKind() != TOcRemView::Link) {
    TCHAR strCloseMenu[128];
    owl::tstring optName = GetModule()->LoadString(IDS_CLOSESERVER);
    TString title = GetOcRemView()->GetContainerTitle();
    if (title.Length() > 0) {
      _tcscpy(strCloseMenu, optName.c_str());
      _tcscat(strCloseMenu, title);
      //LPCTSTR str = title;
      //optName += str;
    }
    //ce.SetText(optName.c_str());
    ce.SetText(&strCloseMenu[0]);
  }
}

//
/// Enables a command with an ID of CM_EDITINSERTOBJECT if the OcApp and OcView
/// objects exist. Disables the command otherwise.
//
void
TOleWindow::CeEditInsertObject(TCommandEnabler& ce)
{
  ce.Enable(OcApp && OcView);
}

//
/// Enables a command with an ID of CM_EDITINSERTCONTROL if the OcApp and OcView
/// objects exist. Disables the command otherwise.
//
void
TOleWindow::CeEditInsertControl(TCommandEnabler& ce)
{
  ce.Enable(OcApp && OcView);
}

//
/// Gets the position (rect) where the embedded object is inserted. You need to
/// override this function if you want to override any default position.
///
/// \note Currently we're using only the top left point for position, size is
/// redundant.
//
void
TOleWindow::GetInsertPosition(TRect& rect)
{
  TOleClientDC dc(*this);

  // Default insertion point is at 0.5" away from current viewport origin
  //
  rect.left  = dc.GetDeviceCaps(LOGPIXELSX) / 2;  // in pixels
  rect.top = dc.GetDeviceCaps(LOGPIXELSY) / 2;    // in pixels

  // Adjust the position of embedded object to be in pixels and reflect the
  // zoom factor.
  //
  dc.LPtoDP((TPoint*)&rect, 1);

  // No size yet.
  //
  rect.right = rect.left;
  rect.bottom = rect.top;
}

//
/// Responds to a command with an ID of CM_EDITINSERTOBJECT by creating,
/// initializing, painting, and selecting an OLE object (TOcPart object).
//
void
TOleWindow::CmEditInsertObject()
{
  PRECONDITION(OcView);
  TOcInitInfo initInfo(OcView);

  if (OcApp->Browse(initInfo)) {
    InsertObject(initInfo);
  }
}

//
/// Responds to a command with an ID of CM_EDITINSERTCONTROL by creating,
/// initializing, painting, and selecting an OCX control (TOcControl object).
//
void
TOleWindow::CmEditInsertControl()
{
  PRECONDITION(OcView);
  TOcInitInfo initInfo(OcView);

  if (OcApp->BrowseControls(initInfo)) {
    InsertControl(initInfo);
  }
}

//
/// Enables a command with an ID of CM_EDITDELETE, which lets the user delete the
/// selected object from the view.
//
void
TOleWindow::CeEditDelete(TCommandEnabler& ce)
{
  ce.Enable(DragPart != 0);
}

//
/// Responds to a command with an ID of CM_EDITDELETE by deleting the selected text.
//
void
TOleWindow::CmEditDelete()
{
  if (!DragPart)
    return;

  TOcPartChangeInfo changeInfo(DragPart, TOcInvalidate(invData | invView));
  EvOcViewPartInvalid(changeInfo);

  DragPart->Delete();
  DragPart = 0;
}

//
/// Enables a command with an ID of CM_EDITCUT, which lets a user copy and delete
/// the selected object from the view.
//
void
TOleWindow::CeEditCut(TCommandEnabler& ce)
{
  ce.Enable(DragPart != 0);
}

//
/// Responds to a command with an ID of CM_EDITCUT by copying the selected text to
/// the clipboard before cutting the text.
//
void
TOleWindow::CmEditCut()
{
  DragPart->Detach();
  OcApp->Copy(DragPart);
  SetSelection(0);
}

//
/// Enables a command with an ID of CM_EDITCOPY, which lets the user copy selected
/// object to the clipboard.
//
void
TOleWindow::CeEditCopy(TCommandEnabler& ce)
{
  ce.Enable(DragPart != 0);
}

//
/// Responds to a command with an ID of CM_EDITCOPY by copying the selected text to
/// the clipboard.
//
void
TOleWindow::CmEditCopy()
{
  if (DragPart)
    OcApp->Copy(DragPart);
}

//
/// Enables a PasteLink command with an ID of CM_EDITPASTELINK, which lets the user
/// link to the embedded object on the clipboard. See the ocrxxxx Clipboard
/// Constants for a description of the available clipboard formats.
//
void
TOleWindow::CeEditPasteLink(TCommandEnabler& ce)
{
  PRECONDITION(OcApp && OcView);
  ce.Enable(OcApp->EnableEditMenu(meEnablePasteLink, OcView));
}

//
/// Responds to a command with an ID of CM_EDITPASTELINK by creating a link between
/// the current document and the object on the clipboard.
//
void
TOleWindow::CmEditPasteLink()
{
   OcView->Paste(true);
}

//
/// Enables a command with an ID of CM_EDITPASTE, which lets the user paste the
/// embedded object from the clipboard.
//
void
TOleWindow::CeEditPaste(TCommandEnabler& ce)
{
  PRECONDITION(OcApp && OcView);
  ce.Enable(OcApp->EnableEditMenu(meEnablePaste, OcView));
}

//
/// Responds to a command with an ID of CM_EDITPASTE by pasting an object from the
/// clipboard into the document.
//
void
TOleWindow::CmEditPaste()
{
   OcView->Paste(false);
   InvalidatePart(invView);
}

//
/// Enables the PasteSpecial command, which lets the user select a clipboard format
/// to be pasted or paste linked.  See the ocrxxxx Clipboard Constants for a
/// description of the available clipboard formats.-
//
void
TOleWindow::CeEditPasteSpecial(TCommandEnabler& ce)
{
  PRECONDITION(OcApp && OcView);
  ce.Enable(OcApp->EnableEditMenu(meEnableBrowseClipboard, OcView));
}

//
/// Responds to a command with an ID of CM_EDITPASTESPECIAL by letting the user
/// select an object from a list of available formats for pasting from the clipboard
/// onto the document.
//
void
TOleWindow::CmEditPasteSpecial()
{
  TOcInitInfo initInfo(GetOcView());

  if (GetOcView()->BrowseClipboard(initInfo)) {
    if (!OcView->PasteNative(initInfo)) { // Not native data
      EvOcViewPasteObject(initInfo);
    }
    InvalidatePart(invView);
  }
}

//
/// Enables a command with an ID of CM_EDITOBJECT, which lets the user edit the
/// embedded object.
//
void
TOleWindow::CeEditObject(TCommandEnabler& ce)
{
  // Downcast to get at submenu item
  //
  TMenuItemEnabler* me = TYPESAFE_DOWNCAST(&ce, TMenuItemEnabler);
  if (!me)
    return;

  int verbPos = me->GetPosition(); // remember the verb menu position
  TMenu editMenu(me->GetMenu());

  owl::tstring optName = GetModule()->LoadString(IDS_EDITOBJECT);
  if (!DragPart) {
    // Remove the verb menu, if any
    //
    if (editMenu.GetSubMenu(verbPos)) {
      editMenu.DeleteMenu(verbPos, MF_BYPOSITION);
      editMenu.InsertMenu(verbPos, MF_GRAYED | MF_BYPOSITION | MF_STRING,
            CM_EDITOBJECT, optName.c_str());
    }
    ce.Enable(false);
    return;
  }

  // Add verb menu
  //
  TOcVerb ocVerb;
  TMenu* verbMenu = CreateVerbPopup(ocVerb);
  owl::tstring newMenuName(OleStr(ocVerb.TypeName));
  newMenuName += _T(" ");
  newMenuName += optName;
  editMenu.ModifyMenu(verbPos, MF_ENABLED | MF_BYPOSITION | MF_POPUP,
    reinterpret_cast<TMenuItem>(verbMenu->GetHandle()), newMenuName.c_str());
  delete verbMenu;
  ce.Enable(true);
}

//
/// Enables a command with an ID of CM_EDITCONVERT, which lets the user convert the
/// selected object from one format to another. This is an OLE-specific pop-up menu option.
//
void
TOleWindow::CeEditConvert(TCommandEnabler& ce)
{
  ce.Enable(DragPart != 0);
}

/// Responds to a command with an ID of CM_EDITCONVERT by converting an object from
/// one type to another.
void
TOleWindow::CmEditConvert()
{
  OcApp->Convert(DragPart, false);
}

//
/// Enables a command with an ID of CM_EDITLINKS, which lets the user manually
/// update the list of linked items in the current view.
//
void
TOleWindow::CeEditLinks(TCommandEnabler& ce)
{
  PRECONDITION(OcApp && OcView);
  ce.Enable(OcApp->EnableEditMenu(meEnableBrowseLinks, OcView));
}

//
/// Responds to a command with an ID of CM_EDITLINKS by updating the user-selected
/// list of linked items in the current view.
//
void
TOleWindow::CmEditLinks()
{
  PRECONDITION(OcView);
  OcView->BrowseLinks();
}

//
/// Enables the Edit|Verbs command, which lets the user select one of the
/// OLE-specific verbs from the Edit menu: for example, Edit, Open, or Play.
//
void
TOleWindow::CeEditVerbs(TCommandEnabler& ce)
{
  ce.Enable(DragPart != 0);
}

//
/// Responds to a command with an ID of CM_EDITSHOWOBJECTS by toggling the value of
/// the ShowObjects data member.
//
void
TOleWindow::CmEditShowObjects()
{
  ShowObjects = !ShowObjects;
  Invalidate();
}

//
/// Checks or unchecks the Edit|Show Objects menu command according to the value of
/// the ShowObjects data member.
//
void
TOleWindow::CeEditShowObjects(TCommandEnabler& ce)
{
  ce.SetCheck(ShowObjects ? TCommandEnabler::Checked : TCommandEnabler::Unchecked);
}

//
/// Returns true if the container's view holds an in-place active embedded object.
//
bool
TOleWindow::HasActivePart()
{
  return OcView->GetActivePart() != 0;
}

//
/// Performs hit testing to tell where the cursor is located within the window and
/// what object the cursor is moving over. If the cursor is within an embedded
/// object,  EvSetCursor changes the shape of the cursor.
/// When the cursor is over an inactive part and not on a handle, EvSetCursor uses
/// an arrow cursor. If the cursor is on one of the handles of the embedded part,
/// EvSetCursor changes the cursor to a resizing cursor.
//
bool
TOleWindow::EvSetCursor(THandle hWnd, uint hitTest, uint mouseMsg)
{
  TPoint pt;
  GetCursorPos(pt);
  ScreenToClient(pt);
  TOcPart* p = OcView->GetActivePart();

  if (hitTest == HTCLIENT) {
    if (p) {   // there is an activated part
      TUIHandle handle(p->GetRect(), TUIHandle::Framed);
      if (handle.HitTest(pt) == TUIHandle::MidCenter) {
        return false; // let the inplace server set its cursor shape
      }
      else {   // use arrow cursor
        ::SetCursor(::LoadCursor(0, IDC_ARROW));
        return true;
      }
    }
  }

  // Set cursor for resize when cursor over inactive part
  //
  if (!p && DragPart) {
    TRect rect(DragPart->GetRect());
    TOleClientDC dc(*this);
    dc.LPtoDP((TPoint*)&rect, 2);

    TUIHandle handle(rect, TUIHandle::HandlesIn | TUIHandle::Grapples |
                     TUIHandle::Framed);

    if (handle.HitTest(pt) != TUIHandle::Outside)
      ::SetCursor(::LoadCursor(0, TResId(handle.GetCursorId(handle.HitTest(pt)))));
    else
      ::SetCursor(::LoadCursor(0, IDC_ARROW));

    return true;
  }

  return ShowCursor(hWnd, hitTest, mouseMsg);
}

//
/// Handles the WM_NCHITTEST message, setting the cursor according to its position
/// on the screen. By default, the cursor is always an arrow. But by overriding the
/// ShowCursor function, you can specify the conditions under which the cursor
/// should change and to which bitmap it should change.
/// HWND is the window containing the cursor, hitTest is a constant that represents
/// the current position of the mouse (for a list of possible values, see the
/// WM_NCHITTEST topic), and mouseMsg is a constant that represents the current
/// mouse action, such as WM_MOUSEACTIVATE.
/// For an example of an implementation of the ShowCursor function, see Step 17 of
/// the OWL tutorial.
//
bool
TOleWindow::ShowCursor(THandle /*hWnd*/, uint /*hitTest*/, uint /*mouseMsg*/)
{
  ::SetCursor(::LoadCursor(0, IDC_ARROW));
  return true;
}

//
// Find out if drag and drop needs to be started
//
bool
TOleWindow::StartDrag(uint modKeys, TPoint& point)
{
    static TSize DblClkDelta(TUIMetric::CxDoubleClk/2,
                      TUIMetric::CyDoubleClk/2);

  // no part no drag
  if (!DragPart)
    return false;

  // maybe it was a double click
  if (abs(point.X()-DragStart.X()) <= DblClkDelta.cx ||
            abs(point.Y()-DragStart.Y()) <= DblClkDelta.cy)
    return false;

  // start drag and drop anyway if outside client area
  if (!InClient(*DragDC, point))
    return true;

  // start if ctrl or ctrl-shift is pressed but not if alt since move inside client area
  // doesn't really make sense (a move is a move don't bother ole for that)
  if ((modKeys & MK_CONTROL))
    return true;

  return false;
}

//
/// This is a response method for an incoming EV_WM_DROPFILES message.
/// Accept dropped file from file manager
//
void
TOleWindow::EvDropFiles(TDropInfo dropInfo)
{
  int fileCount = dropInfo.DragQueryFileCount();
  for (int index = 0; index < fileCount; index++) {
    int fileLength = dropInfo.DragQueryFileNameLen(index)+1;
    TAPointer<OLECHAR> filePath(new OLECHAR[fileLength]);
    dropInfo.DragQueryFile(index, OleStr((OLECHAR*)filePath), fileLength);

    TOcInitInfo initInfo(ihEmbed, iwFile, OcView);
    initInfo.Path = filePath;

    TRect rect;
    GetInsertPosition(rect);
    TOcPart* part = new TOcPart (*GetOcDoc());
    part->Init(&initInfo, rect);
    SetSelection(part);

    OcView->Rename();
    InvalidatePart(invView);
  }
  dropInfo.DragFinish();
}

//
/// Responds to a mouse button double click message. EvLButtonDblClk performs hit
/// testing to see which embedded object, if any, is being clicked on, then in-place
/// activates the embedded object.
//
void
TOleWindow::EvLButtonDblClk(uint modKeys, const TPoint& point_)
{
  PRECONDITION(GetOcDoc() && GetOcView());
  TOleClientDC dc(*this);
  TPoint point(point_);
  dc.DPtoLP(&point);

  TOcPart* p = GetOcDoc()->GetParts().Locate(point);

  if (modKeys & MK_CONTROL) {
    if (p)
      p->Open(true);  // Ctrl key forces open editing
  }
  else {
    SetSelection(p);

    if (p && p == GetOcView()->GetActivePart()) { // resync the active flag
      p->Activate(false);
    }

    GetOcView()->ActivatePart(p); // In-place activation
  }
}

//
/// Selects the embedded object indicated in the part parameter. When the embedded
/// object is selected, a selection box is drawn around the area. After an embedded
/// object is selected, the user can perform operations on the embedded object: for
/// example, moving, sizing, or copying the embedded object to the clipboard.
//
void
TOleWindow::SetSelection(TOcPart* part)
{
  if (part == DragPart)
    return;

  // Invalidate old part
  //
  TOcPartChangeInfo changeInfo(DragPart, invView);
  if (DragPart) {
    DragPart->Select(false);
    DragPart->Activate(false);
    EvOcViewPartInvalid(changeInfo);
  }

  DragPart = part;
  changeInfo.SetPart(DragPart);
  if (DragPart) {
    part->Select(true); // select this one
    EvOcViewPartInvalid(changeInfo);
  }
}

//
/// Responds to a right button down message. Performs additional hit testing to see
/// which embedded object, if any, is being clicked on and displays a local menu
/// with appropriate options for the embedded object.
/// point refers to the place where the mouse is located. modKeys holds the values
/// for a combined key and transaction, such as a Shift+Double-click of the mouse
/// button.
//
void
TOleWindow::EvRButtonDown(uint, const TPoint& point_)
{
  PRECONDITION(GetOcDoc());

  // Perform hit test on parts...
  //
  TPoint oldPoint = point_;
  TOleClientDC dc(*this);
  TPoint point(point_);
  dc.DPtoLP(&point);

  TOcPart* p = GetOcDoc()->GetParts().Locate(point);
  SetSelection(p);

  if (DragPart) {
    // Create popup menu
    //
    TMenu menu(GetModule()->LoadMenu(IDM_OLEPOPUP), AutoDelete);
    TPopupMenu popMenu(menu.GetSubMenu(0));

    if (popMenu.GetHandle()) {
      TOcVerb ocVerb;
      TMenu* verbMenu = CreateVerbPopup(ocVerb);

      owl::tstring optName = GetModule()->LoadString(IDS_EDITOBJECT);
      owl::tstring newMenuName(OleStr(ocVerb.TypeName));
      newMenuName += _T(" ");
      newMenuName += optName;
      popMenu.ModifyMenu(CM_EDITOBJECT, MF_ENABLED | MF_BYCOMMAND | MF_POPUP,
        reinterpret_cast<TMenuItem>(verbMenu->GetHandle()), newMenuName.c_str());
      delete verbMenu;

      // Add the verb menu
      //
      ClientToScreen(oldPoint);

      // Route commands through main window
      //
      popMenu.TrackPopupMenu(TPM_RIGHTBUTTON, oldPoint, 0, *this);
    }
  }
}

//
/// If an embedded object is no longer the active embedded object, either because
/// the user has ended an in-place editing session or because the user has clicked
/// outside the embedded object, call Deactivate to unselect the object. Returns
/// true if successful.
//
bool
TOleWindow::Deactivate()
{
  // Deactivate active part, if any
  //
  if (DragPart && DragPart->IsActive()) {
    SetSelection(0);
    return true;
  }
  else
    return false;
}

//
/// Selects the embedded object at the specified point (measured in logical units).
/// Returns true if the object is captured by the mouse drag; otherwise, returns
/// false.
//
bool
TOleWindow::Select(uint, TPoint& point)
{
  PRECONDITION(GetOcDoc());

  // If the point is not on the current selection, perform hit test on parts
  // to find & select one
  //
  if (!DragPart || !DragPart->IsVisible(TRect(point, TSize(1,1))))
    SetSelection(GetOcDoc()->GetParts().Locate(point));

  // If a part is now selected, manipulate it.
  //
  if (DragPart) {
    DragRect = DragPart->GetRect();
    DragRect.right++;
    DragRect.bottom++;
    if (DragPart->IsSelected()) {
      TUIHandle handle(DragRect, TUIHandle::HandlesIn | TUIHandle::Grapples |
                       TUIHandle::Framed);
      DragHit = handle.HitTest(point);
    }
    else {
      DragHit = TUIHandle::MidCenter;
    }

    if (!DragDC)
      DragDC = new TOleClientDC(*this);
    DragDC->DrawFocusRect(DragRect);

    DragStart = DragPt = point;
    SetCapture();

    return true;
  }

  return false;
}

//
/// Responds to a left button down message by beginning a mouse drag transaction at
/// the given point. Performs additional hit testing to see which embedded object,
/// if any, is being clicked on. The modKeys parameter holds the values for a key
/// combination such as a shift and double click of the mouse button.
/// See also EvRButtonDown
//
void
TOleWindow::EvLButtonDown(uint modKeys, const TPoint& point_)
{
  // Deactivating in-place active object, if any
  //
  if (Deactivate())
    return;

  // Convert the point to logical unit
  //
  if (!DragDC)
    DragDC = new TOleClientDC(*this);

  TPoint point(point_);
  DragDC->DPtoLP(&point);
  Select(modKeys, point);
}

//
/// Returns true if point is inside the client area of the window. Returns false otherwise.
//
bool
TOleWindow::InClient(TDC& dc, TPoint& point)
{
  TRect logicalRect = GetClientRect();

  dc.DPtoLP((TPoint*)&logicalRect, 2);
  return logicalRect.Contains(point);
}

//
/// Responds to a mouse move message with the appropriate transaction. If the mouse
/// is being dragged, the embedded object is moved. If a resizing operation occurs,
/// then the embedded object is resized. This message is handled only when a mouse
/// dragging or resizing action involving the embedded object occurs.
//
void
TOleWindow::EvMouseMove(uint modKeys, const TPoint& point_)
{
  if (!DragDC)
    return;

  // Convert the point to logical unit
  //
  TPoint point(point_);
  DragDC->DPtoLP(&point);

  // A MidCenter hit is a move
  //
  if (DragHit == TUIHandle::MidCenter) {
    DragDC->DrawFocusRect(DragRect);   // erase old rect

    // check if initiate drag and drop
    if (StartDrag(modKeys, point)) {
      TOcDropAction outAction;
      OcApp->Drag(DragPart, TOcDropAction(daDropCopy | daDropMove | daDropLink),
                  outAction);

      TOcPartChangeInfo changeInfo(DragPart, TOcInvalidate(invView | invData));
      EvOcViewPartInvalid(changeInfo);
      DragHit = TUIHandle::Outside;
      ReleaseCapture();

      // Delete the dragged part since it was dragged out
      //
      if (outAction == daDropMove) {
        DragPart->Delete();
        DragPart = 0;
      }
    }
    else {
      TPoint delta = point - DragPt;
      DragRect.Offset(delta.x, delta.y);
      DragDC->DrawFocusRect(DragRect);   // draw new rect
    }
  }
  // All other non-outside hits are resizes
  //
  else if (DragHit != TUIHandle::Outside) { // handle
    DragDC->DrawFocusRect(DragRect);   // erase old rect
    int dl = (DragHit%3) == 0 ? point.x - DragPt.x : 0;
    int dr = (DragHit%3) == 2 ? point.x - DragPt.x : 0;
    int dt = (DragHit/3) == 0 ? point.y - DragPt.y : 0;
    int db = (DragHit/3) == 2 ? point.y - DragPt.y : 0;

    // maintain minimum part size
    //
    if ((DragRect.Width() + dr - dl) >= MinWidth) {
      DragRect.left += dl;
      DragRect.right += dr;
    }
    if ((DragRect.Height() + db - dt) >= MinHeight) {
      DragRect.top  += dt;
      DragRect.bottom += db;
    }

    DragDC->DrawFocusRect(DragRect);   // draw new rect
  }

  DragPt = point;
}

//
/// Responds to a left button up message by ending a mouse drag action. point refers
/// to the place where the mouse is located. modKeys holds the values for a combined
/// key and mouse transaction.
//
void
TOleWindow::EvLButtonUp(uint /*modKeys*/, const TPoint& /*point*/)
{
  if (DragPart) {
    TOcPartChangeInfo changeInfo(DragPart, TOcInvalidate(invView | invData));

    // All non-outside hits are moves or resizes
    //
    if (DragHit != TUIHandle::Outside) {
      EvOcViewPartInvalid(changeInfo);
      DragPart->SetPos(DragRect.TopLeft());
      if (DragHit != TUIHandle::MidCenter)
        DragPart->SetSize(DragRect.Size());  // A MidCenter hit is a move only
      EvOcViewPartInvalid(changeInfo);
    }
    InvalidatePart(invView);

    DragHit = TUIHandle::Outside;
    ReleaseCapture();
  }

  if (DragDC) {
    delete DragDC;
    DragDC = 0;
    DragRect.SetNull();
  }
}

//
/// Passes the event to TWindow::EvSize for normal processing and forwards the event
/// to TOcView::EvResize to let a possible in-place server adjust  its size.
//
void
TOleWindow::EvSize(uint sizeType, const TSize& size)
{
  TWindow::EvSize(sizeType, size);
  OcView->EvResize();
}

//
/// Responds to a message forwarded from the MDI child window (if one exists) and
/// lets the TOcView class know that the view window child window frame has been
/// activated or deactivated.
/// The hWndActivated parameter contains a handle to the MDI child window being
/// activated. Both the child window being activated being activated and the child
/// window (hWndDeactivated) being deactivated receive this message.
//
void
TOleWindow::EvMDIActivate(THandle hActivated, THandle /*hDeactivated*/)
{
  if (OcView)
    OcView->EvActivate(hActivated == *Parent);
}

/// Forwards the WM_MOUSEACTIVATE message to the top parent of the TOleWindow object.
uint
TOleWindow::EvMouseActivate(THandle topParent, uint /*hitCode*/, uint /*msg*/)
{
  if (topParent)
    ForwardMessage(topParent);

  return MA_ACTIVATE;
}

//
/// Responds to a change in focus of the window.  hWndLostFocus contains a handle to
/// the window losing the focus. EvSetFocus checks to see if an in-place server
/// exists and, if so, passes the focus to the in-place server.
//
void
TOleWindow::EvSetFocus(THandle hLostFocus)
{
  TWindow::EvSetFocus(hLostFocus);
  if (OcView)
    OcView->EvSetFocus(true); // Maybe active part (if any) wants focus
}

//
/// In response to a WM_HSCROLL message, EvHScroll calls TWindow::EvHScroll and
/// invalidates the window.
//
void
TOleWindow::EvHScroll(uint scrollCode, uint thumbPos, THandle hCtl)
{
  TWindow::EvHScroll(scrollCode, thumbPos, hCtl);

  InvalidatePart(invView);
}

//
/// In response to a WM_VSCROLL message, EvVScroll calls TWindow::EvVScroll and
/// invalidates the window.
//
void
TOleWindow::EvVScroll(uint scrollCode, uint thumbPos, THandle hCtl)
{
  TWindow::EvVScroll(scrollCode, thumbPos, hCtl);

  InvalidatePart(invView);
}

//
/// Handles WM_MENUSELECT to provide hint text in the container's status bar, based
/// on the menu item id. It treats popup items separately and asks them for their
/// ids. This implementation is similar to the code in TDecoratedFrame.
//
void
TOleWindow::EvMenuSelect(uint menuItemId, uint flags, HMENU hMenu)
{
  if (GetOcRemView()) {
    if (flags == 0xFFFF && hMenu == 0) {  // menu closing
      GetOcRemView()->SetContainerStatusText(_T(""));
      return;
    }
    else if (flags & MF_POPUP) 
    {
      TMenu popupMenu(hMenu);
      const int posItem = menuItemId;
      menuItemId = popupMenu.GetMenuItemID(posItem);
    }
    else if (flags & (MF_SEPARATOR | MF_MENUBREAK | MF_MENUBARBREAK))
    {
      menuItemId = 0;  // display an empty help message
    }
    owl::tstring text = GetModule()->LoadString(menuItemId);
    GetOcRemView()->SetContainerStatusText(text.c_str());
  }
  else
    TWindow::EvMenuSelect(menuItemId, flags, hMenu);
}

//
/// Responds to a WM_OCEVENT message and subdispatches the message based on wParam.
/// ObjectComponents sends WM_OCEVENT messages when it needs to communicate with an
/// OLE-generated event; for example, if a server wants to display toolbars.
//
TResult
TOleWindow::EvOcEvent(TParam1 param1, TParam2 param2)
{
  TEventHandler::TEventInfo eventInfo(WM_OCEVENT, static_cast<uint>(param1));
  if (Find(eventInfo))
    return Dispatch(eventInfo, param1, param2);
  return 0;
}

//
/// Asks the container for the caption in its frame window. Returns the frame
/// window's caption.
//
LPCTSTR
TOleWindow::EvOcViewTitle()
{
  TCHAR title[128];
  Parent->GetWindowText(title, 128);
  ContainerName = title;

  return ContainerName.c_str();
}

//
/// Sets the window's caption to title. The new caption is the name of the in-place
/// active server merged with the caption of the container's window. In the case of
/// an MDI child window, the new caption is the in-place server's name merged with
/// the caption of the MDI child window. When the child window is maximized, the
/// merged caption is appended to the end of the main frame window's caption.
//
void
TOleWindow::EvOcViewSetTitle(LPCTSTR title)
{
  if (title && *title)
    Parent->SetWindowText(title);
  else if (GetOcRemView() && GetOcRemView()->GetKind() == TOcRemView::Link) {
    LPCTSTR caption = GetFileName();
    if (caption)
      Parent->SetWindowText(caption);
  }
}

//
/// Requests that the server create space for a tool bar in the view of an embedded
/// object.
/// If the TOleWindow object is unable to handle the message,
/// EvOcViewBorderSpaceReq returns false, the default value.
//
bool
TOleWindow::EvOcViewBorderSpaceReq(TRect * /*space*/)
{
  return false;
}

//
/// Requests that the server's tool bar be placed in the container's view of an
/// embedded object.
/// If the TOleWindow object is unable to handle the message, EvOcViewBorderSpaceSet
/// returns false, the default value.
//
bool
TOleWindow::EvOcViewBorderSpaceSet(TRect * /*space*/)
{
  return false;
}

//
/// Requests a given object be dropped at a specified place on the container's
/// window.
/// If the TOleWindow object is unable to handle the message, EvOcViewDrop returns
/// false.
//
bool
TOleWindow::EvOcViewDrop(TOcDragDrop & /*ddInfo*/)
{
  return true;    // ok to drop anything, we can take it...
}

//
/// Handles an OC_VIEWDRAG message asking the container to provide visual feedback
/// while the user is dragging the embedded object.
/// If the TOleWindow object is unable to handle the message, EvOcViewDrag returns
/// false.
//
bool
TOleWindow::EvOcViewDrag(TOcDragDrop & ddInfo)
{
  TClientDC dc(*this);
  TPen pen(TColor(128, 128, 128), 4, PS_DOT);
  dc.SelectObject(pen);
  dc.SelectStockObject(HOLLOW_BRUSH);
  dc.SetROP2(R2_NOTXORPEN);

  dc.Rectangle(*ddInfo.Pos);
  return true;
}

//
/// Asks the container to scroll the view window and updates any internal state as
/// needed. EvOcViewScroll is called when the server is resizing or a drop
/// interaction occurs near the edge of the window.
/// If the TOleWindow object is unable to handle the message, EvOcViewScroll returns
/// false.
//
bool
TOleWindow::EvOcViewScroll(TOcScrollDir /*scrollDir*/)
{
  return false;
}

//
/// Gets the size of the rectangle (the site) where the embedded object is to be
/// placed. rect refers to the size of the bounding rectangle that encloses the
/// embedded object.
//
bool
TOleWindow::EvOcViewGetSiteRect(TRect * rect)
{
  TOleClientDC dc(*this);

  return dc.LPtoDP((TPoint*)rect, 2);
}

//
/// Converts the rect to logical units. This area, referred to as the site, is
/// measured in logical units that take into account any scaling factor. rect refers
/// to the size of the bounding rectangle that encloses the embedded object.
//
bool
TOleWindow::EvOcViewSetSiteRect(TRect * rect)
{
  TOleClientDC dc(*this);

  return dc.DPtoLP((TPoint*)rect, 2);
}

//
/// Responds to an OC_VIEWGETSCALE message and gets the scaling for the server
/// object, causing the embedded object to be displayed using the correct scaling
/// value (for example, 120%). scaleFactor indicates the scaling factor, the ratio
/// between the size of the embedded object and the size of the site where the
/// object is to be displayed.
/// If the TOleWindow object is unable to handle the message, EvOcViewGetScale
/// returns false.
//
bool
TOleWindow::EvOcViewGetScale(TOcScaleFactor& scaleFactor)
{
  scaleFactor = Scale;
  return true;
}

//
/// Notifies ObjectWindows container applications that an embedded object is active.
/// ocPart is the embedded object that has been activated. EvOcViewPartActivate
/// returns true after the embedded object has been activated.
//
bool
TOleWindow::EvOcViewPartActivate(TOcPart& ocPart)
{
  SetSelection(&ocPart);
  return true;
}

//
/// Pastes an OLE object into the document pointed to by the TOleWindow::OcDoc data
/// member.
//
bool
TOleWindow::EvOcViewPasteObject(TOcInitInfo& init)
{
  TRect rect;
  GetInsertPosition(rect);
  TOcPart* part = new TOcPart (*GetOcDoc());
  part->Init(&init, rect);
  init.ReleaseDataObject();
  return true;
}

//
/// Asks the server to paint an object at a given position on a specified device
/// context.
/// If the TOleWindow object is unable to handle the message, EvOcViewPaint returns
/// false.
//
bool
TOleWindow::EvOcViewPaint(TOcViewPaint & vp)
{
  // Paint according to the view paint structure
  //
  TDC dc(vp.DC);
  Pos = *vp.Pos;

  // Paint embedded objects
  //

  bool metafile = dc.GetDeviceCaps(TECHNOLOGY) == DT_METAFILE;
  SetupDC(dc, !metafile);

  if (vp.Moniker) {
    PaintLink(dc, true, Pos, *vp.Moniker);
  }
  else if (vp.PaintSelection) {
    PaintSelection(dc, true, Pos, vp.UserData);
  }
  else {
    Paint(dc, true, Pos);
    PaintParts(dc, true, Pos, metafile);
  }

  Pos.SetNull();

  return true;
}

//
/// Asks the server to close a currently open document and its associated view.
/// If the TOleWindow object is unable to handle the message, EvOcViewClose returns
/// false.
///
/// TOcRemView is going away, disconnect TOleWindow with it so we don't use it
/// later. If this is a remote view, then close the doc too.
//
bool
TOleWindow::EvOcViewClose()
{
  if (IsRemote() && OcDoc)
    OcDoc->Close();

  OcView = 0;
  return true;
}

//
/// Asks the server to write an embedded object's data (the part as represented by
/// the ocSave parameter) into storage.
/// If the TOleWindow object is unable to handle the message, EvOcViewSavePart
/// returns false.
//
bool
TOleWindow::EvOcViewSavePart(TOcSaveLoad & /*ocSave*/)
{
  return true;
}

//
/// Requests that an embedded object load itself.
/// If the TOleWindow object is unable to handle the message, EvOcViewLoadPart
/// returns false.
//
bool
TOleWindow::EvOcViewLoadPart(TOcSaveLoad & /*ocLoad*/)
{
  return true;
}

//
/// The server asks itself  the size of its current rectangle and lets the container
/// know about the size of the server's view in pixels.
/// If the TOleWindow object is unable to handle the message, EvOcViewPartSize
/// returns false.
//
bool
TOleWindow::EvOcViewPartSize(TOcPartSize & /*size*/)
{
  return false;
}

//
/// Asks the container to open an existing document, which will be used for linking
/// from the embedding site.
/// If the TOleWindow object is unable to handle the message,  EvOcViewOpenDoc
/// returns false.
//
bool
TOleWindow::EvOcViewOpenDoc(LPCTSTR /*path*/)
{
  return true;
}

//
/// Requests that the menus in a composite menu (a menu composed of both the
/// server's and the container's menus).
/// If the TOleWindow object is unable to handle the message, EvOcViewInsMenus
/// returns false.
//
bool
TOleWindow::EvOcViewInsMenus(TOcMenuDescr & /*sharedMenu*/)
{
  return false;
}

//
/// Asks the server to provide its tool bars for display in the container's window.
/// Returns true if tool bars are supplied.
/// If the TOleWindow object is unable to handle the message,  EvOcViewShowTools
/// returns false.
//
bool
TOleWindow::EvOcViewShowTools(TOcToolBarInfo & tbi)
{
  TWindow* mainWindow = GetApplication()->GetMainWindow();
  CHECK(mainWindow);
  TWindow* toolBar = mainWindow->ChildWithId(IDW_TOOLBAR);
  if (!toolBar)
    return false;
  tbi.HTopTB = THandle(*toolBar);
  return true;
}

//
/// Requests the color palette to draw the object.
/// If the TOleWindow object is unable to handle the message, EvOcViewGetPalette
/// returns false.
//
bool
TOleWindow::EvOcViewGetPalette(LOGPALETTE * * /*palette*/)
{
  return false;
}

//
/// Requests clipboard data in the specified format.
/// If the TOleWindow object is unable to handle the message, EvOcViewClipData
/// returns false.
//
bool
TOleWindow::EvOcViewClipData(TOcFormatData & /*format*/)
{
  return false;
}

//
// Set format data into server
//
bool
TOleWindow::EvOcViewSetData(TOcFormatData & /*format*/)
{
  return false;
}

//
/// Handles the EV_OC_AMBIENT_GETBACKCOLOR message, which is sent by an OCX control.
/// Returns false by default. You should override the default implementation so the
/// function stores the window background color in rgb and returns true.
//
bool
TOleWindow::EvOcAmbientGetBackColor(long* /*backColor*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_GETFORECOLOR message, which is sent by an OCX control.
/// Returns false by default. You should override the default implementation so the
/// function stores the window forecolor in rgb and returns true.
bool
TOleWindow::EvOcAmbientGetForeColor(long* /*foreColor*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_GETLOCALEID message, which is sent by an OCX control.
/// Returns false by default. You should override the default implementation so the
/// function stores the window locale ID in locale and returns true.
bool
TOleWindow::EvOcAmbientGetLocaleID(long* /*localeId*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_GETTEXTALIGN message, which is sent by an OCX control.
/// Returns false by default. You should override the default implementation so the
/// function stores the window text alignment in show and returns true.
bool
TOleWindow::EvOcAmbientGetTextAlign(short* /*align*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_GETMESSAGEREFLECT message, which is sent by an OCX
/// control. Returns false by default. You should override the default
/// implementation so the function stores whether or not the window reflects
/// messages back to the control in msgReflect and returns true.
bool
TOleWindow::EvOcAmbientGetMessageReflect(bool* /*reflectMessage*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_GETUSERMODE message, which is sent by an OCX control.
/// Returns false by default. You should override the default implementation so the
/// function stores whether or not the window is in user mode in mode and returns
/// true.
bool
TOleWindow::EvOcAmbientGetUserMode(bool* /*userMode*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_GETUIDEAD message, which is sent by an OCX control.
/// Returns false by default. You should override the default implementation so the
/// function stores whether or not the window's user interface is disabled in dead
/// and returns true.
bool
TOleWindow::EvOcAmbientGetUIDead(bool* /*deadUI*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_GETSHOWGRABHANDLES message, which is sent by an OCX
/// control. Returns false by default. You should override the default
/// implementation so the function stores whether or not the window shows grab
/// handles in show and returns true.
bool
TOleWindow::EvOcAmbientGetShowGrabHandles(bool* /*showGrabHandles*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_GETSHOWHATCHING message, which is sent by an OCX
/// control. Returns false by default. You should override the default
/// implementation so the function stores whether or not the window shows hatching
/// in show and returns true.
bool
TOleWindow::EvOcAmbientGetShowHatching(bool* /*showHatching*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_GETDISPLAYASDEFAULT message, which is sent by an OCX
/// control. Returns false by default. You should override the default
/// implementation so the function stores whether or not the control is a default
/// control (buttons only) in disp and returns true.
bool
TOleWindow::EvOcAmbientGetDisplayAsDefault(bool* /*displayAsDefault*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_GETSUPPORTSMNEMONICS message, which is sent by an OCX
/// control. Returns false by default. You should override the default
/// implementation so the function stores whether or not the window supports
/// mnemonics in support and returns true.
bool
TOleWindow::EvOcAmbientGetSupportsMnemonics(bool* /*supportMnemonics*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_GETDISPLAYNAME message, which is sent by an OCX
/// control. Returns false by default. You should override the default
/// implementation so the function stores a control name (used in error messages) in
/// name and returns true.
bool
TOleWindow::EvOcAmbientGetDisplayName(TString** /*name*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_GETSCALEUNITS message, which is sent by an OCX
/// control. Returns false by default. You should override the default
/// implementation so the function stores the window scale units in units and
/// returns true.
bool
TOleWindow::EvOcAmbientGetScaleUnits(TString** /*units*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_GETFONT message, which is sent by an OCX control.
/// Returns false by default. You should override the default implementation so the
/// function stores the window font information in font and returns true.
bool
TOleWindow::EvOcAmbientGetFont(IDispatch** /*font*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_SETBACKCOLOR message, which is sent by an OCX control.
/// Returns false by default. You can override the default implementation so the
/// window changes its background color to rgb and returns true.
bool
TOleWindow::EvOcAmbientSetBackColor(long /*backColor*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_SETFORECOLOR message, which is sent by an OCX control.
/// Returns false by default. You can override the default implementation so the
/// window changes its background color to rgb and returns true.
bool
TOleWindow::EvOcAmbientSetForeColor(long /*foreColor*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_SETLOCALEID message, which is sent by an OCX control.
/// Returns false by default. You can override the default implementation so the
/// window changes its locale ID to locale and returns true.
bool
TOleWindow::EvOcAmbientSetLocaleID(long /*localeId*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_SETTEXTALIGN message, which is sent by an OCX control.
/// Returns false by default. You can override the default implementation so the
/// window aligns text according to the value in align and returns true.
bool
TOleWindow::EvOcAmbientSetTextAlign(short /*align*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_SETMESSAGEREFLECT message, which is sent by an OCX
/// control. Returns false by default. You can override the default implementation
/// so the window does or doesn't reflect messages according to the value in
/// msgReflect.
bool
TOleWindow::EvOcAmbientSetMessageReflect(bool /*reflect*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_SETUSERMODE message, which is sent by an OCX control.
/// Returns false by default. You can override the default implementation so the
/// window sets user mode equal to the value in mode.
bool
TOleWindow::EvOcAmbientSetUserMode(bool /*userMode*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_SETUIDEAD message, which is sent by an OCX control.
/// Returns false by default. You can override the default implementation so the
/// window enables or disables itself according to the value in dead.
bool
TOleWindow::EvOcAmbientSetUIDead(bool /*dead*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_SETSHOWGRABHANDLES message, which is sent by an OCX
/// control. Returns false by default. You can override the default implementation
/// so the window shows or hides grab handles according to the value in show.
bool
TOleWindow::EvOcAmbientSetShowGrabHandles(bool /*showHandles*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_SETSETSHOWHATCHING message, which is sent by an OCX
/// control. Returns false by default. You can override the default implementation
/// so the window shows or hides hatching according to the value in hatching.
bool
TOleWindow::EvOcAmbientSetShowHatching(bool /*showHatching*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_SETDISPLAYASDEFAULT message, which is sent by an OCX
/// control. Returns false by default. You can override the default implementation
/// so the window displays itself as a default control (if the window represents a
/// button control) if disp is true.
bool
TOleWindow::EvOcAmbientSetDisplayAsDefault(bool /*displayAsDefault*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_SETSUPPORTSMNEMONICS message, which is sent by an OCX
/// control. Returns false by default. You can override the default implementation
/// so the window turns its support for mnemonics on or off according to the value
/// in support.
bool
TOleWindow::EvOcAmbientSetSupportsMnemonics(bool /*supportMnemonics*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_SETDISPLAYNAME message, which is sent by an OCX
/// control. Returns false by default. You can override the default implementation
/// so the window changes its display name (for error messages) to name and returns true.
bool
TOleWindow::EvOcAmbientSetDisplayName(TString* /*name*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_SETSCALEUNITS message, which is sent by an OCX
/// control. Returns false by default. You can override the default implementation
/// so the window sets its scale units equal to units and returns true.
bool
TOleWindow::EvOcAmbientSetScaleUnits(TString* /*units*/)
{
  return false;
}

/// Handles the EV_OC_AMBIENT_SETFONT message, which is sent by an OCX control.
/// Returns false by default. You can override the default implementation so the
/// window changes its font to font and returns true.
bool
TOleWindow::EvOcAmbientSetFont(IDispatch* /*font*/)
{
  return false;
}

//
// Control Event Dispatches

//
/// Handles the EV_OC_CTRLCLICK message, which is sent by an OCX control. Returns
/// false by default. You can override the default implementation so the window
/// responds to the message and returns true.
//
bool
TOleWindow::EvOcCtrlClick(TCtrlEvent* /*pev*/)
{
  return false;
}

/// Handles the EV_OC_CTRLDBLCLICK message, which is sent by an OCX control. Returns
/// false by default. You can override the default implementation so the window
/// responds to the message and returns true.
bool
TOleWindow::EvOcCtrlDblClick(TCtrlEvent* /*pev*/)
{
  return false;
}

/// Handles the EV_OC_CTRLMOUSEDOWN message, which is sent by an OCX control.
/// Returns false by default. You can override the default implementation so the
/// window responds to the message and returns true.
bool
TOleWindow::EvOcCtrlMouseDown(TCtrlMouseEvent* /*pev*/)
{
  return false;
}

/// Handles the EV_OC_CTRLMOUSEUP message, which is sent by an OCX control. Returns
/// false by default. You can override the default implementation so the window
/// responds to the message and returns true.
bool
TOleWindow::EvOcCtrlMouseUp(TCtrlMouseEvent* /*pev*/)
{
  return false;
}

/// Handles the EV_OC_CTRLMOUSEMOVE message, which is sent by an OCX control.
/// Returns false by default. You can override the default implementation so the
/// window responds to the message and returns true.
bool
TOleWindow::EvOcCtrlMouseMove(TCtrlMouseEvent* /*pev*/)
{
  return false;
}

/// Handles the EV_OC_CTRLKEYDOWN message, which is sent by an OCX control. Returns
/// false by default. You can override the default implementation so the window
/// responds to the message and returns true.
bool
TOleWindow::EvOcCtrlKeyDown(TCtrlKeyEvent* /*pev*/)
{
  return false;
}

/// Handles the EV_OC_CTRLKEYUP message, which is sent by an OCX control. Returns
/// false by default. You can override the default implementation so the window
/// responds to the message and returns true.
bool
TOleWindow::EvOcCtrlKeyUp(TCtrlKeyEvent* /*pev*/)
{
  return false;
}

/// Handles the EV_OC_CTRLERROREVENT message, which is sent by an OCX control.
/// Returns false by default. You can override the default implementation so the
/// window responds to the message and returns true.
bool
TOleWindow::EvOcCtrlErrorEvent(TCtrlErrorEvent* /*pev*/)
{
  return false;
}

/// Handles the EV_OC_CTRLFOCUS message, which is sent by an OCX control. Returns
/// false by default. You can override the default implementation so the window
/// responds to the message and returns true.
bool
TOleWindow::EvOcCtrlFocus(TCtrlFocusEvent* /*pev*/)
{
  return false;
}

/// Handles the EV_OC_CTRLPROPERTYCHANGE message, which is sent by an OCX control.
/// Returns false by default. You can override the default implementation so the
/// window responds to the message and returns true.
bool
TOleWindow::EvOcCtrlPropertyChange(TCtrlPropertyEvent* /*pev*/)
{
  return false;
}

/// Handles the EV_OC_PROPERTYREQUESTEDIT message, which is sent by an OCX control.
/// Returns false by default. You can override the default implementation so the
/// window responds to the message and returns true.
bool
TOleWindow::EvOcCtrlPropertyRequestEdit(TCtrlPropertyEvent* /*pev*/)
{
  return false;
}

/// Handles the EV_OC_CTRLCUSTOMEVENT message, which is sent by an OCX control.
/// Returns false by default. You can override the default implementation so the
/// window responds to the message and returns true.
bool
TOleWindow::EvOcCtrlCustomEvent(TCtrlCustomEvent* /*pev*/)
{
  return false;
}

//
/// Repaints the embedded objects on the given device context. The erase parameter
/// is true if the background of the embedded object is to be repainted. rect
/// indicates the area that needs repainting. metafile indicates whether or not the
/// object is a metafile.
//
bool
TOleWindow::PaintParts(TDC& dc, bool, TRect&, bool metafile)
{
  if (!GetOcDoc())
    return false;

  TRect clientRect;
  TRect logicalRect = GetClientRect();

  if (IsRemote()) {
    clientRect = GetWindowRect();
    clientRect.Offset(-clientRect.left, -clientRect.top);
  }
  else {
    clientRect = logicalRect;
  }

  TPoint scrollPos(0, 0);

  if (!metafile) {
    dc.DPtoLP((TPoint*)&logicalRect, 2);
  }
  else {
    if (Scroller) {
      scrollPos.x = (int)Scroller->XPos;
      scrollPos.y = (int)Scroller->YPos;
    }
  }

  for (TOcPartCollectionIter i(GetOcDoc()->GetParts()); i; i++){
    TOcPart& p = *(i.Current());
    if (p.IsVisible(logicalRect) || metafile) {
      TRect r = p.GetRect();
      r.Offset(-scrollPos.x, -scrollPos.y);
      p.Draw(dc, r, clientRect, asDefault);

      if (metafile)
        continue;

      // Paint selection
      //
      if ((p.IsSelected() || ShowObjects) && r.Width() > 0 && r.Height() > 0) {
        uint handleStyle = p.IsLink() ? TUIHandle::DashFramed : TUIHandle::Framed;
        if (p.IsSelected())
          handleStyle |= TUIHandle::Grapples;
        TUIHandle(r, handleStyle).Paint(dc);
      }
    }
  }
  return true;
}

//
/// Sets the ratio of the embedded object's size to the size of the site.
//
void
TOleWindow::SetScale(uint16 percent)
{
  Scale.SetScale(percent);
  Invalidate();
}

//
/// Gets the logical units (typically pixels) per inch for a document so that the
/// document's embedded objects can be painted correctly on the screen device
/// context.
//
void
TOleWindow::GetLogPerUnit(TSize& logPerUnit)
{
  TScreenDC dc;

  logPerUnit.cx = dc.GetDeviceCaps(LOGPIXELSX);
  logPerUnit.cy = dc.GetDeviceCaps(LOGPIXELSY);
}

//
/// Determines the viewport's origin and extent (the logical coordinates and the
/// size of the device context). Sets up the device context (DC) before painting the
/// embedded object. dc refers to the DC and scale indicates that the scaling factor
/// to use when painting the embedded object is a ratio between the site and the
/// embedded object.
//
void
TOleWindow::SetupDC(TDC& dc, bool scale)
{
  dc.SetMapMode(MM_ANISOTROPIC);

  // Setup window and viewport origin according to scroll amount
  //
  TPoint scrollPos(0, 0);
  if (Scroller) {
    scrollPos.x = (int)Scroller->XPos;
    scrollPos.y = (int)Scroller->YPos;
  }

  if (!scale) {
    dc.SetWindowOrg(scrollPos);
    return;
  }

  // Don't scale the scrolling amount
  //
  if (Scale.SiteSize.cx)
    scrollPos.x = (int)(((uint32)scrollPos.x * Scale.PartSize.cx +
                        Scale.SiteSize.cx/2) / Scale.SiteSize.cx);
  if (Scale.SiteSize.cy)
    scrollPos.y = (int)(((uint32)scrollPos.y * Scale.PartSize.cy +
                        Scale.SiteSize.cy/2) / Scale.SiteSize.cy);
  dc.SetWindowOrg(scrollPos);

  dc.SetViewportOrg(Pos.TopLeft());

  // set the window and viewport extaccording to zoom factor
  //
  TSize ext;
  GetLogPerUnit(ext);
  dc.SetWindowExt(ext);

  ext.cx = dc.GetDeviceCaps(LOGPIXELSX);
  ext.cy = dc.GetDeviceCaps(LOGPIXELSY);

  if (Scale.PartSize.cx)
    ext.cx = (int)(((uint32)ext.cx * Scale.SiteSize.cx + Scale.PartSize.cx/2) /
                   Scale.PartSize.cx);
  if (Scale.PartSize.cy)
    ext.cy = (int)(((uint32)ext.cy * Scale.SiteSize.cy + Scale.PartSize.cy/2) /
                   Scale.PartSize.cy);
  dc.SetViewportExt(ext);
}

//
/// Sets up the dc for proper scaling and scrolling and then calls the derived
/// class's Paint method to paint the contents of the dc.
//
void
TOleWindow::EvPaint()
{
  if (IsFlagSet(wfAlias))
    DefaultProcessing();  // use application-defined wndproc

  else {
    TPaintDC dc(*this);
    TRect&   rect = *(TRect*)&dc.Ps.rcPaint;

    if (Scroller)
      Scroller->BeginView(dc, rect);

    bool metafile = dc.GetDeviceCaps(TECHNOLOGY) == DT_METAFILE;
    SetupDC(dc, !metafile);
    Paint(dc, dc.Ps.fErase, rect);
    PaintParts(dc, dc.Ps.fErase, rect, metafile);

    if (Scroller)
      Scroller->EndView();
  }
}

//
/// Informs an active container that one of its embedded objects needs to be
/// redrawn. Changes in the container's part should be reflected in any other,
/// non-active views. Returns true after all views have been notified of the
/// necessary changes.
/// If the TOleWindow object is unable to handle the message, EvOcViewPartInvalid
/// returns false.
//
bool
TOleWindow::EvOcViewPartInvalid(TOcPartChangeInfo& changeInfo)
{
  // Our document is now dirty...

  // Reflect the change in part in other (non-active) views
  //
  TRect rect(changeInfo.GetPart()->GetRect());
  rect.right++;
  rect.bottom++;
  TOleClientDC dc(*this);
  dc.LPtoDP((TPoint*)&rect, 2);

  InvalidateRect(rect);  // Multiview support to be done in derived classes

  // Notify container if this is an intermediate container
  //
  InvalidatePart((TOcInvalidate)changeInfo.GetType());

  return true; // stop further processing by OCF
}

//
/// Creates and enables a pop-up menu option (ocVerb) on the Edit menu. The verb
/// describes an action (for example, Edit, Open, Play) that is appropriate for the
/// embedded object.
//
TPopupMenu*
TOleWindow::CreateVerbPopup(const TOcVerb& ocVerb)
{
  TPopupMenu* verbMenu = new TPopupMenu(NoAutoDelete);
  while (DragPart->EnumVerbs(ocVerb)) {
    verbMenu->AppendMenu(MF_STRING|MF_ENABLED,
                         (uint)CM_EDITFIRSTVERB + ocVerb.VerbIndex,
                         (LPCTSTR)OleStr(ocVerb.VerbName));
  }

  verbMenu->AppendMenu(MF_SEPARATOR, 0, 0);
  owl::tstring optName = GetModule()->LoadString(IDS_EDITCONVERT);
  verbMenu->AppendMenu(MF_STRING, CM_EDITCONVERT, optName.c_str());

  return verbMenu;
}

//
/// Executes an OLE-related menu option from the Edit menu (for example, Edit, Copy,
/// or Play) that is associated with the selected object.
//
void
TOleWindow::EvDoVerb(uint whichVerb)
{
  DragPart->DoVerb(whichVerb);
}

//
/// Attaches the view to its ObjectWindows parent window so that the user can
/// perform open editing on the embedded object, or if the embedded object has been
/// de-activated while in-place editing was occurring.
/// If the TOleWindow object is unable to handle the message, EvOcViewAttachWindow
/// returns false.
//
bool
TOleWindow::EvOcViewAttachWindow(bool attach)
{
  TOleFrame* mainWindow = TYPESAFE_DOWNCAST(GetApplication()->GetMainWindow(),
                                            TOleFrame);
  if (!mainWindow)
    return false;  // server app is shutting down

  if (attach) {
    if (IsOpenEditing()) {
      // Derived class needs to managed setting up frame differently, like
      // for MDI etc.
      //
      mainWindow->SetClientWindow(this);
    }
  }
  else {
    if (IsOpenEditing() && Parent != mainWindow)
      Parent->PostMessage(WM_CLOSE);
    SetParent(mainWindow->GetRemViewBucket());  // simple reparent
  }
  return true;
}

//
/// Handles the EV_OC_VIEWDOVERB message. Returns false. Implement your own event
/// handler to control how the window responds to verbs selected by the user.
//
bool
TOleWindow::EvOcViewDoVerb(uint /*verb*/)
{
  return false;
}

//
// Perform the action indentified by verb
//
bool
TOleWindow::EvOcViewTransformCoords(uint /*verb*/)
{
  return false;
}

//
/// Shuts down the associated ObjectComponents partners, if possible.
//
bool
TOleWindow::OleShutDown()
{
  if (IsRemote()) {
    TOcRemView* ocRemView = GetOcRemView();
    if (IsOpenEditing())
      ocRemView->Disconnect();
  }
  else {
    if (OcView)
      OcView->EvClose();
  }
  return true;
}

//
/// Performs normal window cleanup and informs the TOcView object that the window is closed.
//
void
TOleWindow::CleanupWindow()
{
  TOleFrame* mainWindow = TYPESAFE_DOWNCAST(GetApplication()->GetMainWindow(),
                                            TOleFrame);
  if (mainWindow)
    mainWindow->OleViewClosing(true);
  OleShutDown();
  if (mainWindow)
    mainWindow->OleViewClosing(false);
  TWindow::CleanupWindow();
}

//
/// Responds to an OC_VIEWSETSCALE message and handles the scaling for server
/// application, ensuring that the embedded object is displayed using the correct
/// scaling values (for example, 120%). The server uses this value in its paint
/// procedure when the embedded object needs to be redrawn. scaleFactor indicates
/// the scaling factor, the ratio between the size of the embedded object and the
/// size of the site where the object is to be displayed.
/// If the TOleWindow object is unable to handle the message, EvOcViewSetScale
/// returns false.
//
bool
TOleWindow::EvOcViewSetScale(TOcScaleFactor& scaleFactor)
{
  Scale = scaleFactor;
  return true;
}

//
/// Handles the EV_OC_VIEWGETITEMNAME. Returns false.
//
bool
TOleWindow::EvOcViewGetItemName(TOcItemName& /*item*/)
{
  return false;
}

//----------------------------------------------------------------------------
// Linking Spport
//

//
/// Responds to an OC_VIEWSETLINK message TOcLinkView sends when the server document
/// provides a link to a container document. EvOcViewSetLink establishes the link
/// between a TOleLinkView and a TOcLinkView. view references the view with which
/// the document or selection is associated. Returns false if unsuccessful.
/// Doc/View applications use TOleView 's implementation of this function.
//
bool
TOleWindow::EvOcViewSetLink(TOcLinkView& /*view*/)
{
  return false;
}

//
/// EvOcViewBreakLink responds to an OC_VIEWBREAKLINK message that TOcLinkView sends
/// when the server document that provides the link shuts down. EvOcViewBreakLink
/// breaks the link with a server document or a selection by deleting the
/// TOleLinkView associated with the TOcLinkView (view). Returns false if
/// unsuccessful. Doc/View applications use TOleView's EvOcViewBreakLink, which
/// overrides TOleWindow's version.
//
bool
TOleWindow::EvOcViewBreakLink(TOcLinkView& /*view*/)
{
  return false;
}

IMPLEMENT_STREAMABLE1(TOleWindow, TWindow);

#if OWL_PERSISTENT_STREAMS

//
//
//
void*
TOleWindow::Streamer::Read(ipstream& is, uint32 /*version*/) const
{
  owl::ReadBaseObject((TWindow*)GetObject(), is);

  GetObject()->Remote = false; // intialized in CreateOcView()
  GetObject()->OcApp  = 0;  // initialized in Init()
  GetObject()->OcDoc  = 0;
  GetObject()->OcView = 0;
  GetObject()->Init();

  is >> GetObject()->Scale;          // current scaling factor
  is >> GetObject()->Pos;            // current position
  is >> GetObject()->ShowObjects;   // Show embedded part frames?

  return GetObject();
}

//
//
//
void
TOleWindow::Streamer::Write(opstream& os) const
{
  owl::WriteBaseObject((TWindow*)GetObject(), os);
  os << GetObject()->Scale;          // current scaling factor
  os << GetObject()->Pos;            // current position
  os << GetObject()->ShowObjects;   // Show embedded part frames?
}

#endif



} // OCF namespace

//==============================================================================

