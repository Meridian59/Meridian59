//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
///   Implementation of TOcRemView Class
//----------------------------------------------------------------------------
#include <ocf/pch.h>

#include <ocf/ocremvie.h>
#include <ocf/ocapp.h>
#include <ocf/ocstorag.h>
#include <ocf/ocbocole.h>


namespace ocf {

using namespace owl;

using namespace std;

DIAG_DEFINE_GROUP_INIT(OWL_INI, OcfRemView, 1, 0);
DIAG_DECLARE_GROUP(OcfRefCount);

const _TCHAR  RemViewStreamName[] = _T("OcRemoteView");

//
/// Constructor for compatibility only
//
TOcRemView::TOcRemView(TOcDocument& doc, TRegList* regList, IUnknown* outer)
:
  BSiteI(0), BContainerI(0), BLSiteI(0),
  State(Hidden),
  Kind(DontKnow),
  TOcView(doc, regList, outer)
{
  Init();
  TRACEX(OcfRefCount, 1, "TOcRemView() @" << (void*)this);
}

//
/// New constructor that uses host interfaces
//
TOcRemView::TOcRemView(TOcDocument& doc,
                       TOcContainerHost* ch, TOcServerHost* sh,
                       TRegList* regList, IUnknown* outer)
:
  BSiteI(0), BContainerI(0), BLSiteI(0),
  State(Hidden),
  Kind(DontKnow),
  TOcView(doc, ch, sh, regList, outer)
{
  Init();
  TRACEX(OcfRefCount, 1, "TOcRemView() @" << (void*)this);
}

//
/// Do real constructor work
//
void
TOcRemView::Init()
{
  // Create a site for this remote view
  //
  if (SUCCEEDED(OcApp.BOleComponentCreate(&BSite, (IUnknown*)(IBContainer*)this,
      OcApp.IsOptionSet(amExeModule)? cidBOleSite : cidBOleInProcSite))) {

    if (SUCCEEDED(BSite->QueryInterface(IID_IBSite, (LPVOID *)&BSiteI)))
      Release();

    // Connect the part and the site
    //
    if (BSiteI) {
      const _TCHAR* progid = RegList->Lookup(OcApp.IsOptionSet(amDebug) ?
                                           "debugprogid" : "progid");
      BSiteI->Init((IBDataProvider2*)this, this, OleStr(progid), true);
    }

    if (SUCCEEDED(BSite->QueryInterface(IID_IBContainer, (LPVOID *)&BContainerI)))
      Release();

    if (SUCCEEDED(BSite->QueryInterface(IID_IBLinkable,(LPVOID *)&BLSiteI)))
      BLSiteI->Release();   // avoid deadlock

    if (SUCCEEDED(BSite->QueryInterface(IID_IBApplication, (LPVOID *)&BAppI)))
      BAppI->Release();     // avoid deadlock
  }
}

//
//
//
TOcRemView::~TOcRemView()
{
  // Make sure running object is unregistered
  //
  if (Kind == LoadFromFile && State != Closing)
    EvClose();

  // Disconnect the server view host with this OcRemView
  //
  ServerHost->EvOcViewClose();

  // Release all helpers, but first call base class to release helpers
  // BEFORE BSite is released
  //
  Shutdown();

  if (BSite) {
    BSite->Release(); // Causes OcAppShutDown if last TOcRemView
    BSite = 0;
  }
}

//
/// Should only be called by the owner/creator of this object
//
void
TOcRemView::ReleaseObject()
{
  AddRef();   // cancel release, view holds a pointer but not a ref count
  TOcView::ReleaseObject();
}

//
//
//
HRESULT
TOcRemView::QueryObject(const IID & iid, void * * iface)
{
  PRECONDITION(iface);
  HRESULT hr;

  static_cast<void>
  (
  // interfaces
  //
     SUCCEEDED(hr = IBPart2_QueryInterface(this, iid, iface))
  || SUCCEEDED(hr = IBDataProvider2_QueryInterface(this, iid, iface))

  // helpers
  //
  || (BSite && SUCCEEDED(hr = BSite->QueryInterface(iid, iface)))

  // base classes
  //
  || SUCCEEDED(hr = TOcView::QueryObject(iid, iface))
  );

  return hr;
}


ulong _IFUNC
TOcRemView::AddRef()
{
  return GetOuter()->AddRef();
}

ulong _IFUNC
TOcRemView::Release()
{
  return GetOuter()->Release();
}

HRESULT _IFUNC
TOcRemView::QueryInterface(const GUID & iid, void ** iface)
{
  return GetOuter()->QueryInterface(iid, iface);
}

//----------------------------------------------------------------------------
// IBDropDest forwarding to base
//
void _IFUNC
TOcRemView::DragFeedback(TPoint * p, const TRect * r, TOcMouseAction a,
                         uint cf, HRESULT& hr)
{
  TOcView::DragFeedback(p, r, a, cf, hr);
}

//----------------------------------------------------------------------------
// IBContainer overrides of TOcView implementation
//

//
/// Show the container window
//
HRESULT _IFUNC
TOcRemView::BringToFront()
{
  TOcView::BringToFront();
  return HR_NOERROR;
}

//
/// Check option flag to see if in place activation is allowed
//
HRESULT _IFUNC
TOcRemView::AllowInPlace()
{
  if (IsOptionSet(voNoInPlace))
    return HR_FALSE;
  else if (IsOptionSet(voNoNestedInPlace) && State != OpenEditing)
    return HR_FALSE;
  else
    return HR_NOERROR;
}

//----------------------------------------------------------------------------
// IBSite pass-thrus

//
/// Invalidate the site within the active view
//
void
TOcRemView::Invalidate(TOcInvalidate invalid)
{
  if (BSiteI)
    BSiteI->Invalidate(invalid);
}

//
/// Disconnect from the client site
//
void
TOcRemView::Disconnect()
{
  TRACEX(OcfRemView, 1, "Disconnect() on " << (void*)this);

  if (BSiteI)
    BSiteI->Disconnect();
}

//
/// Set focus to the inplace server
//
bool
TOcRemView::EvSetFocus(bool set)
{
  BSiteI->OnSetFocus(set);
  return TOcView::EvSetFocus(set);
}


//----------------------------------------------------------------------------
// IBDocument pass-thrus

void
TOcRemView::EvClose()
{
  TRACEX(OcfRemView, 1, "EvClose() on " << (void*)this);

  if (BSite && BLSiteI)
    BLSiteI->OnRename(0, 0);

  OcDocument.Close();  // Disconnect from the parts, if any
  TOcView::EvClose();
}

//
/// Update monikers with new name
//
void
TOcRemView::Rename()
{
  PRECONDITION(BLSiteI);

  TOcView::Rename();

  // Update the part's moniker
  //
//  BLSiteI->OnRename(BLDocumentI, OleStr(DefaultStreamName));
}

//
/// Get the window text of the container
//
TString
TOcRemView::GetContainerTitle()
{
  if (BContainerI)
    return BContainerI->GetWindowTitle();
  return _T("");
}

//
///  Get the initial size and position from the app
//
void
TOcRemView::GetInitialRect(bool selection)
{
  TOcPartSize ps(selection);

  // See if the container has already provided a site size, & if so provide it
  // as a default size to the server object.
  //
  bool haveSite = false;
  if (BSiteI)
    haveSite = SUCCEEDED(BSiteI->GetSiteRect(&ps.PartRect, 0))
               && !ps.PartRect.IsEmpty();

  // Let the server app provide an initial server extent if it wants to.
  //
  if (!ServerHost->EvOcViewPartSize(ps)) {

    // The server has not requested a specific size. If the container has
    // already provided a site size, use it. Otherwise pick an arbitrary
    // non-zero size.
    //
    if (!haveSite) {
      HDC dc = ::GetDC(0);

      // a 3" x 2" extent for server
      //
      ps.PartRect = TRect(0, 0,
                          ::GetDeviceCaps(dc, LOGPIXELSX)*3,
                          ::GetDeviceCaps(dc, LOGPIXELSY)*2);
      ReleaseDC(0, dc);
    }
  }
  Extent = ps.PartRect.Size();
  Origin = ps.PartRect.TopLeft();
}

//
/// Save remote view specific information to compound file
//
bool
TOcRemView::Save(IStorage* storageI)
{
  TOcStorage Storage(storageI);

  // Create/open a stream in our storage to save part information
  //
  STATSTG  statstg;
  if (!SUCCEEDED(Storage.Stat(&statstg, STATFLAG_NONAME)))
    return false;

  TOcStream  stream(Storage, RemViewStreamName, true, statstg.grfMode);

  // Write TOcRemView data into stream
  //
  ulong count;
  if (!SUCCEEDED(stream.Write(&Origin, sizeof(TPoint), &count)))
    return false;

  if (!SUCCEEDED(stream.Write(&Extent, sizeof(TSize), &count)))
    return false;

  return true;
}

//
/// Load remote view specific information
//
bool
TOcRemView::Load(IStorage* storageI)
{
  TOcStorage Storage(storageI);

  // Open a stream with oc part information
  //
  STATSTG statstg;
  if (!HRSucceeded(Storage.Stat(&statstg, STATFLAG_NONAME)))
    return false;

  try {
    TOcStream  stream(Storage, RemViewStreamName, false, statstg.grfMode);

    // Read TOcPart data from stream. Server part info will be read in as
    // needed after Init()
    //
    ulong count;
    if (!HRSucceeded(stream.Read(&Origin, sizeof(TPoint), &count)))
      return false;

    if (!HRSucceeded(stream.Read(&Extent, sizeof(TSize), &count)))
      return false;
  }
  catch (TXObjComp&) {
    // There is no remote view related info saved.
    // This happens when saving the document while running as a stand alone
    // app. But subsequently, a embed from file is executed.
  }

  return true;
}

//----------------------------------------------------------------------------
// IBContains

//
/// This is called when we want to create a part from data saved
/// in the storage named "path"
//
HRESULT _IFUNC
TOcRemView::Init(LPCOLESTR path)
{
  PRECONDITION(path);

  GetInitialRect();

  // Assume for now that we are created to load our object from a file. Kind
  // will get changed later to Link if we are asked to Open()
  //
  Kind = LoadFromFile;

  // Read object's data from storage
  //
  if (!ContainerHost->EvOcViewOpenDoc(OleStr(path))) {
    return HR_FAIL;
  }

  // Pretend we are open editing to make sure that the doc server is attached
  // to a child window in case this is a link.
  //
  State = OpenEditing;
  ServerHost->EvOcViewAttachWindow(true);

  // If embed from file, we had to call BDocumentI->Init() to initialize
  // IBApplication, but we do not want to register the drop target, therefore,
  // we use a 0 for the window handle.
  //
  HWND oldWin = ContainerHost->EvOcGetWindow();
  ContainerHost->SetWindow(0);
  BDocumentI->Init(this);
  ContainerHost->SetWindow(oldWin);

  // Register this document in running object table
  //
  OcDocument.SetName(owl::tstring(OleStr(path)));
  return HR_NOERROR;
}

HRESULT _IFUNC
TOcRemView::GetPart(IBPart * * parti, LPCOLESTR moniker)
{
  if (Kind == LoadFromFile)
    Kind = Link;          // must be a link, not a transient load

  return TOcView::GetPart(parti, moniker);
}

//----------------------------------------------------------------------------
// IDataNegotiator implementation to eliminate ambiguity (IBPart & IBDropDest)

uint _IFUNC
TOcRemView::CountFormats()
{
  return TOcView::CountFormats();
}

HRESULT _IFUNC
TOcRemView::GetFormat(uint index, TOcFormatInfo * fmt)
{
  return TOcView::GetFormat(index, fmt);
}

//----------------------------------------------------------------------------
// IBDataProvider2 implementation

//
/// Request native data for pasting into client application.
/// This is only called at paste time (not at copy time).
//
HANDLE _IFUNC
TOcRemView::GetFormatData(TOcFormatInfo * fmt)
{
  PRECONDITION(fmt);

  TOcFormat* format = FormatList.Find(fmt->Id);
  if (format) {
    TOcFormatData formatData(*format);
    if (ServerHost->EvOcViewClipData(formatData))
      return formatData.Handle;
  }
  return 0;
}

//
/// Request native data for pasting into client application.
/// This is only called at paste time (not at copy time).
//
HRESULT _IFUNC
TOcRemView::SetFormatData(TOcFormatInfo * fmt, HANDLE data, BOOL /*release*/)
{
  PRECONDITION(fmt);
  PRECONDITION(data);

  TOcFormat* format = FormatList.Find(fmt->Id);
  if (format) {
    TOcFormatData formatData(*format, 0, data);
    if (ServerHost->EvOcViewSetData(formatData))
      return HR_NOERROR;
  }
  return HR_FAIL;
}

//
/// Render the view in the DC provided. May be a MetaFile
/// Packup all the args & forward message to real view to paint
//
HRESULT _IFUNC
TOcRemView::Draw(HDC dc, const RECTL *  pos, const RECTL * clip,
                 TOcAspect aspect, TOcDraw bd)
{
  PRECONDITION(dc);
  bool metafile = ::GetDeviceCaps(dc, TECHNOLOGY) == DT_METAFILE;

  // Rely on the BOle shading
  //
  if (bd == drShadingOnly)
    return HR_NOERROR;

  TRect p((int)pos->left, (int)pos->top, (int)pos->right, (int)pos->bottom);
  TRect c((int)clip->left, (int)clip->top, (int)clip->right, (int)clip->bottom);

  TPoint oldViewOrg;
  TPoint oldWinOrg;
  TSize  oldViewExt;
  TSize  oldWinExt;
  int    oldMapMode;
  HRGN   hRgn;

  if (metafile) {
    p.SetEmpty();
    ::SetMapMode(dc, MM_ANISOTROPIC);

    ::SetWindowExtEx(dc, Extent.cx, Extent.cy, 0);
    ::SetWindowOrgEx(dc, 0, 0, 0);
  }
  else if (!OcApp.IsOptionSet(amExeModule)) {
    if (p.Width() != Extent.cx || p.Height() != Extent.cy) {
      TOcScaleFactor scaleFactor(p, Extent);
      ServerHost->EvOcViewSetScale(scaleFactor);
    }
    hRgn = ::CreateRectRgn(0, 0, 0, 0);
    ::GetClipRgn(dc, hRgn);
    ::LPtoDP(dc, (TPoint*)&p, 2);
    ::LPtoDP(dc, (TPoint*)&c, 2);
    ::IntersectClipRect(dc, c.left, c.top, c.right, c.bottom);

    ::SetViewportOrgEx(dc, p.left, p.top, &oldViewOrg);
    ::GetWindowOrgEx(dc, &oldWinOrg);
    ::GetWindowExtEx(dc, &oldWinExt);
    ::GetViewportExtEx(dc, &oldViewExt);
    oldMapMode  = ::GetMapMode(dc);
  }

  p.Normalize();
  c.Normalize();

  TOcViewPaint vp = { dc, &p, &c, (TOcAspect)aspect, false, 0, 0 };
  bool result = ServerHost->EvOcViewPaint(vp);

  // Restore the dc attributes
  //
  if (!metafile && !OcApp.IsOptionSet(amExeModule)) {
    ::SetMapMode(dc, oldMapMode);
    ::SetViewportOrgEx(dc, oldViewOrg.x, oldViewOrg.y, 0);
    ::SetWindowOrgEx(dc, oldWinOrg.x, oldWinOrg.y, 0);
    ::SetViewportExtEx(dc, oldViewExt.cx, oldViewExt.cy, 0);
    ::SetWindowExtEx(dc, oldWinExt.cx, oldWinExt.cy, 0);
    ::SelectClipRgn(dc, hRgn);
    ::DeleteObject(hRgn);
  }

  return HRFailIfZero(result);
}

//
/// Return the 'size' of the document that this view in on
//
HRESULT _IFUNC
TOcRemView::GetPartSize(TSize * size)
{
  *size = Extent;
  return HR_NOERROR;
}

//
/// Save the document that we are a view on
//
HRESULT _IFUNC
TOcRemView::Save(IStorage* storage, BOOL sameAsLoad, BOOL remember)
{
  PRECONDITION(storage);

  TOcSaveLoad ocSave(storage, ToBool(sameAsLoad), ToBool(remember));
  return HRFailIfZero(ServerHost->EvOcViewSavePart(ocSave));
}

//----------------------------------------------------------------------------
// IBPart2 implementation

//
/// Load the associated document and activate the remote view
//
HRESULT _IFUNC
TOcRemView::Init(IBSite *, TOcInitInfo * initInfo)
{
  PRECONDITION(initInfo);

  // Now we know that we are a normal embedding
  //
  Kind = Embedding;

  // If existing document, notify the app to read it in.
  //
  if (initInfo->Where == iwStorage) {
    TOcSaveLoad ocLoad(initInfo->Storage, true, true);
    if (!ServerHost->EvOcViewLoadPart(ocLoad))
      return HR_FAIL;
  }
  else if (initInfo->Where == iwNew)
    GetInitialRect();

  // Show & activate this embedded object
  //
  Show(true);
  Activate(true);

  return HR_NOERROR;
}

//
/// Close the remote view window, & if canShutDown is true, try to close the
/// server app too
//
HRESULT _IFUNC
TOcRemView::Close()
{
  TRACEX(OcfRemView, 1, "Close() on " << (void*)this <<
         " Closing:" << (int)ToBool(State==Closing) << " Win:" << hex <<
         static_cast<void*>(ServerHost->EvOcGetWindow()));

  // Keep DLL server in memory until its ref count goes to 0
  //
  if (State != Closing && OcApp.IsOptionSet(amExeMode)) {
    EvClose();
  }

  // Remember that we are closing so that we don't do Close() again
  //
  State = Closing;

  return HR_NOERROR;
}

//
/// Query to determine if this server view can open in place
//
HRESULT _IFUNC
TOcRemView::CanOpenInPlace()
{
  if (IsOptionSet(voNoInPlaceServer))
    return HR_FALSE;    // Force server to open edit
  else
    return HR_NOERROR;  // allow in place activation
}

//
/// Set a new position for our document within its container
//
HRESULT _IFUNC
TOcRemView::SetPartPos(TRect * r)
{
  Origin = *(POINT*)&r->left;

  CHECK(BSiteI);
  TOcScaleInfo scale;
  BSiteI->GetZoom(&scale);
  TOcScaleFactor scaleFactor(scale);

  // No scaling
  //
  if (scale.xN == 1 && scale.yN == 1) {
    scaleFactor.SiteSize.cx = scaleFactor.PartSize.cx = r->right - r->left;
    scaleFactor.SiteSize.cy = scaleFactor.PartSize.cy = r->bottom - r->top;
  }

  ::MoveWindow(GetWindow(), r->left, r->top,
               scaleFactor.SiteSize.cx, scaleFactor.SiteSize.cy, true);

  ServerHost->EvOcViewSetScale(scaleFactor);

  return HR_NOERROR;
}

HRESULT _IFUNC
TOcRemView::SetPartSize(TSize * size)
{
  Extent = *size;
  return HR_NOERROR;
}

//
/// Activate this view
//
HRESULT _IFUNC
TOcRemView::Activate(BOOL activate)
{
  if (activate && GetWindow())
    ::SetFocus(GetWindow());
  return HR_NOERROR;
}

//
/// Show/Hide the server view window
//
HRESULT _IFUNC
TOcRemView::Show(BOOL show)
{
  if (GetWindow())
    ::ShowWindow(GetWindow(), show ? SW_SHOW : SW_HIDE);
  return HR_NOERROR;
}

//
/// Start or end open editing
/// Work with the window Z-order and parenting
//
HRESULT _IFUNC
TOcRemView::Open(BOOL open)
{
  if (open) {
    // Just show the window, if it's already active
    //
    if (State == InPlaceActive) {
      HWND contWin = BAppI->GetWindow();
      ::BringWindowToTop(contWin);
    }
    else {
      State = OpenEditing;    // we now transition into open edit state
      if (Kind == LoadFromFile)
        Kind = Link;          // must be a link, not a transient load

      // Register drag&drop target
      //
      BDocumentI->Init(this);

      // Forward event to app to let it attach the window to an appropriate
      // frame
      //
      ServerHost->EvOcViewAttachWindow(true);
      BringToFront();

      // Build up open edit caption in the form "<menuname> in <containername>"
      //
      if (RegList && Kind != Link) {
        owl::tstring newTitle = (*RegList)["menuname"];
        newTitle += GetGlobalModule().LoadString(IDS_IN);
        LPCOLESTR str = BContainerI->GetWindowTitle();
        if (str)
          newTitle += OleStr(str);
        ServerHost->EvOcViewSetTitle(newTitle.c_str());
      }
      else {
        ServerHost->EvOcViewSetTitle(nullptr); // if link, caption is filename
      }
    }
  }
  else {
    // Unregister drag&drop target
    //
    BDocumentI->OnClose();

    // Reparent back to the real parent
    //
    ServerHost->EvOcViewAttachWindow(false);
  }
  return HR_NOERROR;
}

//
/// Enumerate the verbs for our document
//
HRESULT _IFUNC
TOcRemView::EnumVerbs(TOcVerb *)
{
  return HR_FAIL;  // Not called on BOle parts
}

//
/// Perform a verb on our document
//
HRESULT _IFUNC
TOcRemView::DoVerb(uint verb)
{
  return HRFailIfZero(ServerHost->EvOcViewDoVerb(verb));
}

//
/// Open or close this view as an in-place edit session. If hWndParent is 0,
/// then in-place is closing
//
HWND _IFUNC
TOcRemView::OpenInPlace(HWND hWndParent)
{
  if (hWndParent) {
    State = InPlaceActive;  // transition into in-place-active state

    // Register drag&drop target
    //
    BDocumentI->Init(this);

    ::SetParent(GetWindow(), hWndParent);
  }
  else {
    State = Hidden;  // transition back into normal, hidden state
    BDocumentI->OnClose();

    // Deactivate in-place active object, if any
    //
    if (ActivePart)
      ActivePart->Activate(false);

    // Set back to real parent and don't show it
    //
    if (!ServerHost->EvOcViewAttachWindow(false)) {
      Show(false);
      ::SetParent(GetWindow(), OcApp.GetWindow());
    }
  }
  return GetWindow();
}

//
/// Insert the server's menus into the shared menubar
//
HRESULT _IFUNC
TOcRemView::InsertMenus(HMENU hMenu, TOcMenuWidths * omw)
{
  PRECONDITION(omw && hMenu);

  TOcMenuDescr md;
  md.HMenu = hMenu;

  int i;
  for (i = 0; i < 6; i++) {
    md.Width[i] = (int)omw->Width[i];
    i++;
    md.Width[i] = 0;  // make sure the server's are zeroed
    omw->Width[i] = 0;
  }

  if (!ServerHost->EvOcViewInsMenus(md))
    return HR_FAIL;

  for (i = 0; i < 6; i++)
    omw->Width[i] = md.Width[i];

  return HR_NOERROR;
}

//
/// Retrieve rectangle info about the toolbars
//
static void
getTBRects(TOcToolBarInfo & tbi,
            TRect& ttbr, TRect& ltbr, TRect& btbr,
            TRect& rtbr, TRect& border)
{
  if (tbi.HLeftTB)
    ::GetWindowRect(tbi.HLeftTB, &ltbr);
  else
    ltbr.Set(0,0,0,0);
  if (tbi.HTopTB)
    ::GetWindowRect(tbi.HTopTB, &ttbr);
  else
    ttbr.Set(0,0,0,0);
  if (tbi.HRightTB)
    ::GetWindowRect(tbi.HRightTB, &rtbr);
  else
    rtbr.Set(0,0,0,0);
  if (tbi.HBottomTB)
    ::GetWindowRect(tbi.HBottomTB, &btbr);
  else
    btbr.Set(0,0,0,0);
  border.Set(ltbr.Width() ? ltbr.Width()-1 : 0,
             ttbr.Height() ? ttbr.Height()-1 : 0,
             rtbr.Width() ? rtbr.Width()-1 : 0,
             btbr.Height() ? btbr.Height()-1 : 0);
}

//
/// Actually parent & position the toolbars in the container frame.
/// Assume all the toolbars have WS_BORDER style & overlaps accordingly
//
static void
adjustTBPos(TOcToolBarInfo & tbi, const TRect& contFrameR, bool setParent,
             const TRect& ttbr, const TRect& ltbr,
             const TRect& btbr, const TRect& rtbr,
             const TRect& border)
{
  if (tbi.HLeftTB) {
    ::MoveWindow(tbi.HLeftTB, -1, -1+border.top, ltbr.Width(),
                 contFrameR.Height()+2-border.top-border.bottom, true);
    if (setParent) {
      ::SetParent(tbi.HLeftTB, tbi.HFrame);
      ::ShowWindow(tbi.HLeftTB, SW_NORMAL);
    }
  }
  if (tbi.HTopTB) {
    ::MoveWindow(tbi.HTopTB, -1, -1, contFrameR.Width()+2, ttbr.Height(),
                 true);
    if (setParent) {
      ::SetParent(tbi.HTopTB, tbi.HFrame);
      ::ShowWindow(tbi.HTopTB, SW_NORMAL);
    }
  }
  if (tbi.HRightTB) {
    ::MoveWindow(tbi.HRightTB, contFrameR.right-rtbr.Width()+1, -1+border.top,
                 rtbr.Width(), contFrameR.Height()+2-border.top-border.bottom,
                 true);
    if (setParent) {
      ::SetParent(tbi.HRightTB, tbi.HFrame);
      ::ShowWindow(tbi.HRightTB, SW_NORMAL);
    }
  }
  if (tbi.HBottomTB) {
    ::MoveWindow(tbi.HBottomTB, -1, contFrameR.bottom-btbr.Height()+1,
                 contFrameR.Width()+2, btbr.Height(), true);
    if (setParent) {
      ::SetParent(tbi.HBottomTB, tbi.HFrame);
      ::ShowWindow(tbi.HBottomTB, SW_NORMAL);
    }
  }
}

//
/// Show or hide the tool windows used by our view
//
HRESULT _IFUNC
TOcRemView::ShowTools(BOOL show)
{
  TOcToolBarInfo & tbi = ToolBarInfo;
  tbi.Show = show;
  tbi.HTopTB = tbi.HLeftTB = tbi.HBottomTB = tbi.HRightTB = 0;
  if (show)
    tbi.HFrame = BAppI->GetWindow();  // Container's frame
  else
    tbi.HFrame = OcApp.GetWindow();   // This server's frame

  // Let the view create/destroy its toolbars & fill them in for us to work
  // with. View may also change the HFrame for special purposes
  //
  if (!ServerHost->EvOcViewShowTools(tbi))
    return HR_FAIL;

  // Assist view in showing toobars by negotiating, parenting to the container
  // frame & showing the provided toolbars
  //
  if (show) {
    TRect ltbr;
    TRect ttbr;
    TRect rtbr;
    TRect btbr;
    TRect border;
    getTBRects(tbi, ltbr, ttbr, rtbr, btbr, border);

    // Request the desired border space from the container. Scale back requests
    // as needed if container denies requests.
    //
    int i;
    for (i = 0; i < 3; i++) {
      if (!SUCCEEDED(BAppI->RequestBorderSpace(&border))) {
        switch (i) {
          case 0:
            border.bottom = 0;
            tbi.HBottomTB = 0;
            break;
          case 1:
            border.right = 0;
            tbi.HRightTB = 0;
            break;
          case 2:
            border.left = 0;
            tbi.HLeftTB = 0;
        }
      }
      else
        break;
    }
    if (i == 3 || !SUCCEEDED(BAppI->SetBorderSpace(&border)))
      return HR_FAIL;  // don't know why it failed. Give up

    TRect contFrameR;
    BAppI->GetWindowRect(&contFrameR);
    adjustTBPos(tbi, contFrameR, true, ltbr, ttbr, rtbr, btbr, border);
  }
  // Assist view in hiding toobars by reparenting them to frame (defaults to
  // this server) & hiding them
  //
  else {
    if (tbi.HLeftTB) {
      ::SetParent(tbi.HLeftTB, tbi.HFrame);
      ::ShowWindow(tbi.HLeftTB, SW_HIDE);
    }
    if (tbi.HTopTB) {
      ::SetParent(tbi.HTopTB, tbi.HFrame);
      ::ShowWindow(tbi.HTopTB, SW_HIDE);
    }
    if (tbi.HRightTB) {
      ::SetParent(tbi.HRightTB, tbi.HFrame);
      ::ShowWindow(tbi.HRightTB, SW_HIDE);
    }
    if (tbi.HBottomTB) {
      ::SetParent(tbi.HBottomTB, tbi.HFrame);
      ::ShowWindow(tbi.HBottomTB, SW_HIDE);
    }
  }
  return HR_NOERROR;
}

//
/// A container window has resized. Perform any necessary adjustment of our
/// tools.
//
void _IFUNC
TOcRemView::FrameResized(const TRect * contFrameR, BOOL isMainFrame)
{
  if (!isMainFrame)
    return;

  TRect ltbr;
  TRect ttbr;
  TRect rtbr;
  TRect btbr;
  TRect border;
  getTBRects(ToolBarInfo, ltbr, ttbr, rtbr, btbr, border);
  adjustTBPos(ToolBarInfo, *contFrameR, false, ltbr, ttbr, rtbr, btbr, border);
}

//
/// Let the server provide drag feedback
//
HRESULT _IFUNC
TOcRemView::DragFeedback(TPoint * where, BOOL /*nearScroll*/)
{
  PRECONDITION(where);

  TPoint awhere(*where);
  TOcDragDrop dd = { 0, &awhere, 0 };
  return HRFailIfZero(ServerHost->EvOcViewDrag(dd));
}

//
/// Optional palette query for
//
HRESULT _IFUNC
TOcRemView::GetPalette(LOGPALETTE * * palette)
{
  PRECONDITION(palette);

  return HRFailIfZero(ServerHost->EvOcViewGetPalette(palette));
}

HRESULT _IFUNC
TOcRemView::SetHost(IBContainer *)
{
  return HR_FAIL;  // Not called on BOle parts.
}

HRESULT _IFUNC
TOcRemView::DoQueryInterface(const IID & iid, void * * iface)
{
  PRECONDITION(iface);

  return TOcView::QueryInterface(iid, iface);  // Unused on server side
}

LPOLESTR _IFUNC
TOcRemView::GetName(TOcPartName)
{
  return 0;  // not called on BOle parts
}

} // OCF namespace

//==============================================================================

