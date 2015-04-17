//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1991, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation classes handling Windows resources
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/resource.h>
#include <owl/gdiobjec.h>
#include <owl/gadgetwi.h>
#include <owl/buttonga.h>
#include <owl/celarray.h>


namespace owl {

OWL_DIAGINFO;


//#if defined(BI_COMP_MSC)
//  const char* szRT_TOOLBAR    = RT_TOOLBAR;
//  const char* szRT_NEWDIALOG  = RT_NEWDIALOG;
//  const char* szRT_DIALOG      = RT_DIALOG;
//#endif

//
//
//
TDialogRes::TDialogRes(HINSTANCE hInst, TResId id) 
           : OldResource(0)
           , NewResource(0)
{
  // In 32-bit, favour the new dialog resource over the old one
  //
  NewResource = new TResource<DLGTEMPLATEEX, (int)RtNewDialog>(hInst, id);
  if (!NewResource->IsOK()) 
    // When the new resource cannot be found in 32-bit, try
    // the old DLGTEMPLATE resource
    //
    OldResource = new TResource<DLGTEMPLATE, (int)RtDialog>(hInst, id);
}

//
//
//
TDialogRes::TDialogRes(HINSTANCE hInst, TResId id, LANGID langid)
           :OldResource(0)
          , NewResource(0)
{
  NewResource = new TResource<DLGTEMPLATEEX, (int)RtNewDialog>(hInst, id, langid);
  if (!NewResource->IsOK())
    OldResource = new TResource<DLGTEMPLATE, (int)RtDialog>(hInst, id, langid);
}

//
//
//
TDialogRes::~TDialogRes()
{
  delete OldResource;
  delete NewResource;
}


//
//
//
bool
TDialogRes::IsOK() const
{
  return (NewResource && NewResource->IsOK()) || 
         (OldResource && OldResource->IsOK());
}


//
//
//
bool      
TDialogRes::IsDialogEx() const 
{
  PRECONDITION(IsOK());
  return (!OldResource || !OldResource->IsOK()) && 
         ( NewResource &&  NewResource->IsOK());
}

DLGTEMPLATEEX*
TDialogRes::GetTemplateEx() const
{
  PRECONDITION(IsDialogEx());
  return NewResource->operator DLGTEMPLATEEX*();
}

DLGTEMPLATE*
TDialogRes::GetTemplate() const
{
  PRECONDITION(!IsDialogEx());
  return  OldResource->operator DLGTEMPLATE*();
}

void      
TDialogRes::GetRect(TRect& rect) const
{
  if (!IsDialogEx()) {
    DLGTEMPLATE* dlgTemplate = GetTemplate();
    rect.left  = dlgTemplate->x;
    rect.right = dlgTemplate->x + dlgTemplate->cx;
    rect.top   = dlgTemplate->y;
    rect.bottom= dlgTemplate->y + dlgTemplate->cy;
  } else {
    DLGTEMPLATEEX* dlgTemplate = GetTemplateEx();
    rect.left  = dlgTemplate->x;
    rect.right = dlgTemplate->x + dlgTemplate->cx;
    rect.top   = dlgTemplate->y;
    rect.bottom= dlgTemplate->y + dlgTemplate->cy;
  }
}

int
TDialogRes::GetText(LPTSTR buffer, int size, TDlgResText which) const
{
  LPCWSTR p, pMenu, pClass, pCaption;

  if (!IsDialogEx()) {
    DLGTEMPLATE* dlgTemplate = GetTemplate();
    p = (LPCWSTR)(dlgTemplate+1);
    pMenu = p;
    pClass = pMenu + ((*pMenu == 0xffff) ? 2 : lstrlenW(pMenu)+1);
    pCaption = pClass + ((*pClass == 0xffff) ? 2 : lstrlenW(pClass)+1);
  }
  //else{  Y.B. Finish ??????????????????????????????????????????????????????
  //    DLGTEMPLATEEX* dlgTemplateEx = GetTemplateEx();
    // !BB Get info about DLGEX!
  //}


  switch (which) {
    case drtMenuName:   p = pMenu;    break;
    case drtClassName:  p = pClass;   break;
    case drtCaption:    p = pCaption; break;
    default:
      return 0;
  };
#  if defined(UNICODE)
  int req_size = (*p == 0xffff) ? 2 : lstrlen(p) + 1;
  if (!buffer || size < req_size) return req_size;
  if((*p == 0xffff)){
    memcpy(buffer, p, 2);
    return 2;
  }
  lstrcpy(buffer, p);
  return lstrlen(buffer);
#  else
  return WideCharToMultiByte(CP_ACP, 0, p, (*p == 0xffff) ? 2 : lstrlenW(p),
                             buffer, size, 0, 0);
#  endif
}

//
//
//
TToolbarRes::TToolbarRes(HINSTANCE hInst, TResId id, TAutoDelete autoDelete)
:
  TResource<TOOLBARTEMPLATE, (int)RtToolbar>(hInst, id), 
  ToolbarBitmap(0)
{
  ShouldDelete  = (autoDelete == AutoDelete);

  // Following will throw exception in case of failure
  //
  ToolbarBitmap = new TBitmap(hInst, id);
}

//
//
//
TToolbarRes::~TToolbarRes()
{
  if (ShouldDelete)
    delete ToolbarBitmap;
}

//
//
//
TOOLBARTEMPLATE&
TToolbarRes::TBInfo() const {
  PRECONDITION(IsOK());
  return *(CONST_CAST(TToolbarRes*, this)->operator TOOLBARTEMPLATE*());
}

//
//
//
TBitmap&
TToolbarRes::GetBitmap() {
  PRECONDITION(IsOK());
  PRECONDITION(ToolbarBitmap && ToolbarBitmap->GetHandle());
  return *ToolbarBitmap;
}

//
//
//
int
TToolbarRes::GetCount() const {
  PRECONDITION(IsOK());
  return TBInfo().count;    
}

//
//
//
ushort*
TToolbarRes::GetIds() const {
  PRECONDITION(IsOK());
  return TBInfo().ids;
}

//
//
//
int
TToolbarRes::GetWidth() const {
  PRECONDITION(IsOK());
  return TBInfo().width;
}

//
//
//
int
TToolbarRes::GetHeight() const {
  PRECONDITION(IsOK());
  return TBInfo().height;
}


//
//
TToolbarBldr::TToolbarBldr(TGadgetWindow& win, TResId id, HINSTANCE hinst)
:
  Win(win), 
  TbarRes(0), 
  Id(id), 
  Hinst(hinst)
{ 
}

//
//
//
TToolbarBldr::~TToolbarBldr()
{
  delete TbarRes;
}

//
// Populate the gadgetwindow with button gadgets and separators using a
// bitmap and toolbar resource pair. Returns true if the resource was
// successfully loaded and the gadgets created and false otherwise.
//
void
TToolbarBldr::Build()
{
  // Default to window's module if none was specified
  //
  if (!Hinst)
    Hinst = *(Win.GetModule());

  // NOTE: Don't let the TToolbarRes instance own the bitmap, we'll hand
  //       it to the celarray instead.
  //
  TbarRes = new TToolbarRes(Hinst, Id, NoAutoDelete);

  CHECK(TbarRes->IsOK());
  TCelArray* celArray = new TCelArray(&TbarRes->GetBitmap(), 
                                      TbarRes->GetCount(),
                                      TSize(TbarRes->GetWidth(), 
                                            TbarRes->GetHeight()));
  // Hand celarray to gadgetwindow
  // NOTE: Nice enhancement would be to add/merge toolbar resources using
  //       TCelArray's Add(TCelArray& src, index) method if the window
  //       already has a celarray. Unfortunately, the tbar resource does 
  //       provide a method to specify insertion indices to make this
  //       approach usable. So we'll wipe out the previous celarray.
  //
  Win.SetCelArray(celArray);

  // Create gadgets based on resource information
  //
  for (int i=0, j=0; i<TbarRes->GetCount(); i++) {
    if (TbarRes->GetIds()[i] == 0)
      Win.Insert(*new TSeparatorGadget);
    else {
      Win.Insert(*new TButtonGadget(j++, TbarRes->GetIds()[i], 
                                    TButtonGadget::Command, false,
                                    TButtonGadget::Up, true));
    }
  }
}

} // OWL namespace
/* ========================================================================== */

