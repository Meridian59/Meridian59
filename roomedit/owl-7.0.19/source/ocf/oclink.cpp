//----------------------------------------------------------------------------
// ObjectComponents
// Copyright (c) 1994, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TOcLinkView Class
//----------------------------------------------------------------------------
#include <ocf/pch.h>

#include <ocf/oclink.h>
#include <ocf/ocapp.h>
#include <ocf/ocremvie.h>

namespace ocf {

using namespace owl;

//
//
//
TOcLinkView::TOcLinkView(TOcView* ocView, TRegList* regList, IUnknown* outer)
:
  BSiteI(0), OcView(ocView),
  Origin(0,0),
  Extent(0,0)
{
  PRECONDITION(OcView);
  SetOuter(outer);
  AddRef();    // TUnknown defaults to 0, we need 1

  // Create a site for this remote view
  //
  if (SUCCEEDED(OcView->OcApp.BOleComponentCreate(&BSite, (IUnknown*)(IBPart*)this,
      OcView->OcApp.IsOptionSet(amExeModule)? cidBOleSite : cidBOleInProcSite))) {

    if (SUCCEEDED(BSite->QueryInterface(IID_IBSite, (LPVOID *)&BSiteI)))
      Release();

    // Connect the part and the site
    //
    if (BSiteI) {
      const _TCHAR* progid = regList->Lookup(OcView->OcApp.IsOptionSet(amDebug) ?
                                           "debugprogid" : "progid");
      BSiteI->Init(this, this, OleStr(progid), true);
    }

    if (SUCCEEDED(BSite->QueryInterface(IID_IBApplication, (LPVOID *)&BAppI)))
      BAppI->Release();     // avoid deadlock
  }
}

//
//
//
TOcLinkView::~TOcLinkView()
{
  // Detach the link view
  //
  //Detach();
  if (BSite) {
    BSite->Release();
  }
}

//
/// Remove this link view from the document
//
int
TOcLinkView::Detach()
{
  return OcView->OcDocument.GetViews().Detach(this);
}

//
//
//
HRESULT
TOcLinkView::QueryObject(const IID & iid, void * * iface)
{
  PRECONDITION(iface);
  HRESULT hr;

  static_cast<void>
  (
  // interfaces
  //
     SUCCEEDED(hr = IBPart_QueryInterface(this, iid, iface))
  || SUCCEEDED(hr = IBDataProvider_QueryInterface(this, iid, iface))

  // helpers
  //
  || (BSite && SUCCEEDED(hr = BSite->QueryInterface(iid, iface)))
  );

  return hr;
}

//----------------------------------------------------------------------------
// IBSite pass-thrus

//
/// Invalidate the site corresponding to this view
//
void
TOcLinkView::Invalidate(TOcInvalidate invalid)
{
  if (BSiteI)
    BSiteI->Invalidate(invalid);
}

//
/// Disconnect from the client site
//
void
TOcLinkView::Disconnect()
{
  if (BSiteI)
    BSiteI->Disconnect();
}

//
/// Remember the name of the moniker
//
void
TOcLinkView::SetMoniker(LPCTSTR name)
{
  Moniker  = const_cast<LPTSTR>(name);  // force TString to copy
}


//----------------------------------------------------------------------------
// IDataNegotiator implementation -- delegate to our owning OcView

//
//
//
uint _IFUNC
TOcLinkView::CountFormats()
{
  return OcView->CountFormats();
}

//
//
//
HRESULT _IFUNC
TOcLinkView::GetFormat(uint index, TOcFormatInfo * fmt)
{
  PRECONDITION(fmt);

  return OcView->GetFormat(index, fmt);
}

//----------------------------------------------------------------------------
// IBDataNegotiator implementation

//
/// Request native data for pasting into client application.
/// This is only called at paste time (not at copy time).
//
HANDLE _IFUNC
TOcLinkView::GetFormatData(TOcFormatInfo * fmt)
{
  PRECONDITION(fmt);

  TOcFormat* format = OcView->FormatList.Find(fmt->Id);
  if (format) {
    TOcFormatData formatData(*format);
    if (OcView->ServerHost->EvOcViewClipData(formatData))
      return formatData.Handle;
  }

  return 0;
}

//
/// Get the initial size and position from the host app into our members
//
void
TOcLinkView::GetLinkRect()
{
  TOcPartSize ps(true, &Moniker);

  // Ask the app for initial server extent
  //
  if (!OcView->ServerHost->EvOcViewPartSize(ps)) {
    // An empty rect as default means that the container
    // decides the size for this server
    //
    ps.PartRect.SetNull();
  }

  Extent = ps.PartRect.Size();
  Origin = ps.PartRect.TopLeft();
}

//
/// Render the view in the DC provided. Should be a MetaFile
/// Packup all the args & forward message to real view to paint
//
HRESULT _IFUNC
TOcLinkView::Draw(HDC dc, const RECTL *  pos, const RECTL * clip,
                  TOcAspect aspect, TOcDraw bd)
{
  PRECONDITION(dc);

  // Rely on the bolero shading
  //
  if (bd == drShadingOnly)
    return HR_NOERROR;

  TRect p((int)pos->left, (int)pos->top, (int)pos->right, (int)pos->bottom);
  TRect c((int)clip->left, (int)clip->top, (int)clip->right, (int)clip->bottom);

  p.SetEmpty();
  ::SetMapMode(dc, MM_ANISOTROPIC);

  ::SetWindowExtEx(dc, Extent.cx, Extent.cy, 0);
  ::SetWindowOrgEx(dc, 0, 0, 0);

  p.Normalize();
  c.Normalize();

  // Find out where the TOleLinkView is
  //
  GetLinkRect();
  *(TPoint*)&p = Origin;

  TOcViewPaint vp = { dc, &p, &c, (TOcAspect)aspect, false, &Moniker, 0 };

  return HRFailIfZero(OcView->ServerHost->EvOcViewPaint(vp));
}

//
/// Return the 'size' of the document that this view in on
//
HRESULT _IFUNC
TOcLinkView::GetPartSize(TSize * size)
{
  *size = Extent;
  return HR_NOERROR;
}

//
/// Save the document that we are a view on
//
HRESULT _IFUNC
TOcLinkView::Save(IStorage* storage, BOOL sameAsLoad, BOOL remember)
{
  PRECONDITION(storage);

  TOcSaveLoad ocSave(storage, ToBool(sameAsLoad), ToBool(remember));

  return HRFailIfZero(OcView->ServerHost->EvOcViewSavePart(ocSave));
}

#if 0
//
//
//
HRESULT _IFUNC
TOcLinkView::SetFormatData(TOcFormatInfo * /*fmt*/, HANDLE /*data*/, BOOL /*release*/)
{
  return HR_NOTIMPL;
}
#endif

//----------------------------------------------------------------------------
// IBPart implementation

//
/// Load the associated document and activate the remote view
//
HRESULT _IFUNC
TOcLinkView::Init(IBSite *, TOcInitInfo * /*initInfo*/)
{
  return HR_NOERROR;
}

//
/// Close the remote view window, & if canShutDown is true, try to close the server
/// app too
//
HRESULT _IFUNC
TOcLinkView::Close()
{
  OcView->ServerHost->EvOcViewBreakLink(*this); // !CQ check return?
  return HR_NOERROR;
}

//
/// Query to determine if this view can open in place
//
HRESULT _IFUNC
TOcLinkView::CanOpenInPlace()
{
  return HR_FAIL;  // Links never open in place
}

//
/// Set a new position for our document within its container
//
HRESULT _IFUNC
TOcLinkView::SetPartPos(TRect * r)
{
  Origin = *(POINT*)&r->left;
  return HR_NOERROR;
}

//
//
//
HRESULT _IFUNC
TOcLinkView::SetPartSize(TSize * size)
{
  Extent = *size;
  return HR_NOERROR;
}

//
/// Activate this view
//
HRESULT _IFUNC
TOcLinkView::Activate(BOOL /*activate*/)
{
  return HR_NOERROR;
}

//
/// Show/Hide the server view window
//
HRESULT _IFUNC
TOcLinkView::Show(BOOL /*show*/)
{
  return HR_NOERROR;
}

//
/// Start or end open editing
/// Work with the window Z-order and parenting
//
HRESULT _IFUNC
TOcLinkView::Open(BOOL open)
{
  if (open) {
    TOcRemView* ocRemView = TYPESAFE_DOWNCAST(OcView, TOcRemView);
    if (ocRemView)
      ocRemView->SetOpenEditing();

    OcView->ServerHost->EvOcViewAttachWindow(true);
    OcView->BringToFront();
  }

  return HR_NOERROR;
}

//
/// Enumerate the verbs for our document
//
HRESULT _IFUNC
TOcLinkView::EnumVerbs(TOcVerb *)
{
  return HR_FAIL;  // Not called on BOle parts
}

//
/// Perform a verb on our document
//
HRESULT _IFUNC
TOcLinkView::DoVerb(uint)
{
  return HR_FAIL;  // Assume that links don't need to do verbs
}

//
/// Open or close this view as an in-place edit session. If hWndParent is 0, then
/// in-place is closing
//
HWND _IFUNC
TOcLinkView::OpenInPlace(HWND /*hWndParent*/)
{
  return 0;
}

//
/// Insert the server's menus into the shared menubar
//
HRESULT _IFUNC
TOcLinkView::InsertMenus(HMENU /*hMenu*/, TOcMenuWidths * /*omw*/)
{
  return HR_NOERROR;
}

//
/// Show or hide the tool windows used by our view
//
HRESULT _IFUNC
TOcLinkView::ShowTools(BOOL /*show*/)
{
  return HR_NOERROR;
}

//
/// A container window has resized. Perform any necessary adjustment of our
/// tools.
//
void _IFUNC
TOcLinkView::FrameResized(const TRect * /*contFrameR*/, BOOL /*isMainFrame*/)
{
}

//
/// Let the server provide drag feedback
//
HRESULT _IFUNC
TOcLinkView::DragFeedback(TPoint * where, BOOL /*nearScroll*/)
{
  TPoint awhere(*where);
  TOcDragDrop dd = { 0, &awhere, 0 };
  return HRFailIfZero(OcView->ServerHost->EvOcViewDrag(dd));
}

//
/// Optional palette query for
//
HRESULT _IFUNC
TOcLinkView::GetPalette(LOGPALETTE * * palette)
{
  PRECONDITION(palette);

  return HRFailIfZero(OcView->ServerHost->EvOcViewGetPalette(palette));
}

//
//
//
HRESULT _IFUNC
TOcLinkView::SetHost(IBContainer * /*objContainer*/)
{
  return HR_FAIL;  // Not called on BOle parts.
}

//
//
//
HRESULT _IFUNC
TOcLinkView::DoQueryInterface(const IID & iid, void * * iface)
{
  PRECONDITION(iface);

  return OcView->QueryInterface(iid, iface);  // Unused on server side
}

//
//
//
LPOLESTR _IFUNC
TOcLinkView::GetName(TOcPartName /*name*/)
{
  return 0;  // Not called on BOle parts.
}


//----------------------------------------------------------------------------
// TOcLinkCollection
//

//
//
//
TOcLinkCollection::TOcLinkCollection()
{
}

//
//
//
TOcLinkCollection::~TOcLinkCollection()
{
  Clear();
}

//
/// Release Views in the collection
//
void
TOcLinkCollection::Clear()
{
  for (int i = Count() - 1; i >= 0; i--) {
    TOcLinkView* view = (TOcLinkView*)(*this)[i];
    view->Release();
    Base::Detach(i); ///DR why do we want to keep data after a clear? Remove it.
  }
}

//
//
//
int
TOcLinkCollection::Detach(TOcLinkView* const& view, int del)
{
  int ret = Base::Detach(Find(view));
  if (ret && del)
    const_cast<TOcLinkView*>(view)->Release();
  return ret;
}

//
//
//
TOcLinkView*
TOcLinkCollection::Find(TString const moniker) const
{
  for (TOcViewCollectionIter j((TOcViewCollection&)*this); j; j++) {
    TOcLinkView* view = (TOcLinkView*)j.Current();
    if (view && strcmp(view->GetMoniker(), moniker) == 0) {
      return view;
    }
  }

  return 0;
}

} // OCF namespace

//==============================================================================

