//-------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
// Copyright(c) 1996 by Manic Software.
// All rights reserved.
//
// TOWLEXTDll (and friends) implementation
//-------------------------------------------------------------------
#include <owlext/pch.h>
#pragma hdrstop

#include <owlext/owlext.h>
#include <owlext/util.h>
#include <owlext/version.h>
#include <owl/bitset.h>

using namespace owl;


//
// Diagnostic group declarations/definitions
//
DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlExtModule, 1, 0);

#if defined (_BUILDOWLEXTDLL)
#if !defined(BI_COMP_BORLANDC)
int WINAPI
DllMain(HINSTANCE hInstance, uint32 reason, void* lpreserved);
#else
int WINAPI
DllEntryPoint(HINSTANCE hInstance, uint32 reason, void* lpreserved);
#endif
#endif //#if defined (_BUILDOWLEXTDLL)


#if defined(BI_COMP_BORLANDC)
//
// Fixing BUG in BC 5.x with templates
//
static void NeverCallThisDummyFunction()
{
  typedef TBitFlags<uint> ___type1;
  ___type1 dummy1;
}
#endif

namespace OwlExt {

#if defined (_BUILDOWLEXTDLL)

static LPCTSTR OwlExtName = _T("OWLEXT");

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                    TOWLEXTDll
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
TOWLEXTDll::TOWLEXTDll(bool shouldLoad, bool mustLoad)
:
TModule(OwlExtName, shouldLoad, mustLoad),
GetOWLEXTVersion(*this, "GetOWLEXTVersion")
{
  TRACEX(OwlExtModule, 1, "TOWLEXTDll external constructor invoked");
}
TOWLEXTDll::TOWLEXTDll(HINSTANCE instance)
:
TModule(OwlExtName, instance),
GetOWLEXTVersion(*this, "GetOWLEXTVersion")
{
  TRACEX(OwlExtModule, 1, "TOWLEXTDll internal constructor invoked");
}
TOWLEXTDll::~TOWLEXTDll()
{
  TRACEX(OwlExtModule, 1, "TOWLEXTDll destructor invoked");
}

TModuleVersionInfo& TOWLEXTDll::GetVersionInfo()
{
  TRACEX(OwlExtModule, 1, "TOWLEXTDll::GetVersionInfo invoked");
  static TModuleVersionInfo verInfo(*this);
  return verInfo;
}


// Localized method to obtain this DLL's HINSTANCE value
HINSTANCE
GetThisHINSTANCE()
{
  MEMORY_BASIC_INFORMATION mbi;
#if defined(BI_COMP_BORLANDC)
  if (VirtualQuery(DllEntryPoint, &mbi, sizeof(mbi)))
#else
  if (VirtualQuery(DllMain, &mbi, sizeof(mbi)))
#endif
    return (HINSTANCE)mbi.BaseAddress;
  else
    return 0;
}

TOWLEXTDll&
TOWLEXTDll::Instance()
{
  // This is amended slightly from the text, since I figured it would be a small
  // performance increase to check the global owl::Module TModule pointer,
  // for the (normal) case where DllEntryPoint() is called (and TOWLEXTDll is
  // safely initialized) before anybody calls TOWLEXTDll::Instance(). It also
  // helps to offer another way of doing this DLL-initialization thing.
  //

  if(owl::Module == NULL){
    TRACEX(OwlExtModule, 1, "TOWLEXTDll::Instance creating TOWLEXTDll object");
    static TOWLEXTDll theModule(GetThisHINSTANCE());
    owl::Module = &theModule;
  }

  return *((TOWLEXTDll*)owl::Module);
  // Need to return a type of 'TOWLEXTDll&', and '*Module' is a 'TModule&'; cast
  // explicitly to a TOWLEXTDll* so when we dereference, it's of the right type
}

//
int
TOWLEXTDll::ThreadAttach()
{
  // If this module depended on maintaining separate data for each thread, we'd
  // do the necessary initialization here

  TRACEX(OwlExtModule, 1, "TOWLEXTDll::ThreadAttach invoked");
  return 1;
}

//
int
TOWLEXTDll::ThreadDetach()
{
  // If this module depended on maintaining separate data for each thread, we'd
  // do the necessary uninitialization here

  TRACEX(OwlExtModule, 1, "TOWLEXTDll::ThreadDetach invoked");
  return 1;
}
#endif //#if defined (_BUILDOWLEXTDLL)

} // OwlExt namespace

#if defined (_BUILDOWLEXTDLL)

//
// Exported method to obtain OWLEXT's version number
//
extern "C" OWLEXTFUNC(long WINAPI) GetOWLEXTVersion()
{
  return OWLEXT_FILEVERSION;
}

namespace OwlExt {

// OWLEXTEntryPoint is our TOWLEXTDll-friendly function to take care of all DLL-
// notification responses
//
int OWLEXTEntryPoint(HINSTANCE hInstance, uint32 reason, LPVOID)
{
  TRACEX(OwlExtModule, 1, "OWLEXTEntryPoint called");

  switch (reason)
  {
    // DLL_PROCESS_ATTACH--a process has LoadLibrary()'ed us
    //
  case DLL_PROCESS_ATTACH:
    {
      // First see if we need to create the TOWLEXTDll instance
      if (!owl::Module){
        // Yep, so create it
        static TOWLEXTDll theModule(hInstance);
        owl::Module = &theModule;
      }
      return ::OWLInitUserDLL(hInstance, 0) ? 1 : 0;
    }

    // DLL_THREAD_ATTACH--a process which loaded this DLL has spun off a thread
    //
  case DLL_THREAD_ATTACH:
    return TOWLEXTDll::Instance().ThreadAttach();

    // DLL_THREAD_DETACH--a process which loaded this DLL has a thread ending
    //
  case DLL_THREAD_DETACH:
    return TOWLEXTDll::Instance().ThreadDetach();

    // DLL_PROCESS_DETACH--a process which loaded this DLL is unloading us
    //
  case DLL_PROCESS_DETACH:
    {
      return 1;
    }
  }
  return 0;
}
} // OwlExt namespace

// DllEntryPoint is called whenever 'something' happens to the DLL. Defer all actions
// to the global function OWLEXTEntryPoint
//
#if !defined(BI_COMP_BORLANDC)
int WINAPI
DllMain(HINSTANCE hInstance, uint32 reason, void* lpreserved)
#else
int WINAPI
DllEntryPoint(HINSTANCE hInstance, uint32 reason, void* lpreserved)
#endif
{
  return OwlExt::OWLEXTEntryPoint(hInstance, reason, lpreserved);
}



// Singleton accessor method; does the same as TOWLEXTDll::Instance(), but
// some may find the global function syntax to be cleaner
//
#if defined(BI_COMP_BORLANDC)
extern "C" OWLEXTFUNC(TModule*  WINAPI)
#else
STDAPI_(TModule*)
#endif
GetOWLEXTModule()
{
  return &(OwlExt::TOWLEXTDll::Instance());
}
#endif //#if defined (_BUILDOWLEXTDLL)

namespace OwlExt {


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                                                                     Functions
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

///////////////////////////////////////////////////////////////////////
//  Activates (brings to top and sets focus) the first window found
//  (by searching up the parent chain from the specified window) that
//  either has a caption or is a top-level window.
//
void ActivateWindow (TWindow* tWindow)
{
  HWND parent;
  LONG style;
  HWND window = tWindow->GetHandle();


  parent = window;

  do{
    window = parent;
    parent = ::GetParent (window);
    style  = ::GetWindowLong (window, GWL_STYLE);
  }while (parent && !(style & WS_CAPTION));

  ::BringWindowToTop (window);
}

////////////////////////////////////////////////////////////////////////////
//   Function:  CenterScreenRectOnRect
//
//     Params:  rc        - RECT* to the rectangle of window being
//                          centered
//
//              rcOver    - RECT* to the rectangle of the window being
//                          centered over.
//
//              NOTE:   Both RECT's should be GetWindowRect() values,
//                      i.e., screen coordinates.
//
//    Returns:  Nothing
//
//   Comments:  This function will set the members of 'rc' such that it
//              is centered over 'rcOver'.  The members of 'rc' will be
//              adjusted for the screen size so that the window will be
//              visible.
//
////////
void CenterScreenRectOnRect (TRect* rc, TRect* rcOver)
{
  int cxPopup  = rc->Width();
  int cyPopup  = rc->Height();
  int cxParent = rcOver->Width();
  int cyParent = rcOver->Height();
  int cxScreen = GetSystemMetrics (SM_CXSCREEN);
  int cyScreen = GetSystemMetrics (SM_CYSCREEN);

  // Center it:
  int xPopup = rcOver->left + ((cxParent - cxPopup) / 2);
  int yPopup = rcOver->top  + ((cyParent - cyPopup) / 2);

  // Adjust popup location for screen size:
  if (xPopup+cxPopup > cxScreen)
    xPopup = cxScreen - cxPopup;
  if (yPopup+cyPopup > cyScreen)
    yPopup = cyScreen - cyPopup;

  rc->left   = xPopup * (xPopup > 0);
  rc->top    = yPopup * (yPopup > 0);
  rc->right  = rc->left + cxPopup;
  rc->bottom = rc->top + cyPopup;
}

////////////////////////////////////////////////////////////////////////////
//  This function will center hWnd over hwndOver.  If hwndOver is NULL,
//  then hWnd will be centered on the screen.
//
void CenterWindowOver (HWND hWnd, HWND hwndOver)
{
  TRect rcOver, rcWnd;

  if (!hwndOver)
    hwndOver = ::GetDesktopWindow();

  ::GetWindowRect (hwndOver, &rcOver);
  ::GetWindowRect (hWnd, &rcWnd);

  CenterScreenRectOnRect (&rcWnd, &rcOver);

  ::SetWindowPos (hWnd, NULL, rcWnd.left, rcWnd.top, 0, 0,
    SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
}

////////////////////////////////////////////////////////////////////////////
//  This function updates the mouse cursor after forcing it to something
//  else via SetCursor
//
void UpdateCursor ()
{
  TPoint  pt;

  ::GetCursorPos (&pt);
  ::SetCursorPos (pt.x, pt.y);
}


// MFC style function, that draw the frame of a 3d rect with given colors
void Draw3dRect(TDC& dc, const TRect& rect, TColor clrTopLeft,
        TColor clrBottomRight)
{
  Draw3dRect(dc, rect.left, rect.top, rect.Width(),
    rect.Height(), clrTopLeft, clrBottomRight);
}

void Draw3dRect(TDC& dc, int x, int y, int cx, int cy, TColor clrTopLeft,
        TColor clrBottomRight)
{
  FillSolidRect(dc, x, y, cx - 1, 1, clrTopLeft);
  FillSolidRect(dc, x, y, 1, cy - 1, clrTopLeft);
  FillSolidRect(dc, x + cx, y, -1, cy, clrBottomRight);
  FillSolidRect(dc, x, y + cy, cx, -1, clrBottomRight);
}

// MFC style function, that draws a filled rect. Is this faster???
void
FillSolidRect(TDC& dc, int x, int y, int cx, int cy, TColor clr)
{
  TColor oldColor = dc.SetBkColor(clr);
  TRect rect(x, y, x + cx, y + cy);
  dc.TextRect(rect);
  dc.SetBkColor(oldColor);
}

void
FillMaskRect(TDC& dc, TRect rect)
{
  THatch8x8Brush br(THatch8x8Brush::Hatch11F1, TColor::Sys3dHilight,
    TColor::Sys3dFace);
  dc.FillRect(rect, br);
}

void DrawDisabledButton(TDC& dc, const TRect& rc)
{
  // create a monochrome memory DC
  //
  TMemoryDC ddc;
  TBitmap bmp(ddc, rc.Width(), rc.Height());
  ddc.SelectObject(bmp);

  // build a mask
  //
  ddc.PatBlt(0, 0, rc.Width(), rc.Height(), WHITENESS);
  dc.SetBkColor(TColor::Sys3dFace);
  ddc.BitBlt(0, 0, rc.Width(), rc.Height(), dc, rc.left, rc.top, SRCCOPY);
  dc.SetBkColor(TColor::Sys3dHilight);
  ddc.BitBlt(0, 0, rc.Width(), rc.Height(), dc, rc.left, rc.top, SRCPAINT);

  // Copy the image from the toolbar into the memory DC
  // and draw it (grayed) back into the toolbar.
  //
  dc.FillRect(rc, TBrush(TColor::Sys3dFace));
  dc.SetBkColor(RGB(0, 0, 0));
  dc.SetTextColor(RGB(255, 255, 255));
  TBrush brShadow(TColor::Sys3dShadow);
  TBrush brHilight(TColor::Sys3dHilight);
  dc.SelectObject(brHilight);
  dc.BitBlt(rc.left+1, rc.top+1, rc.Width(), rc.Height(), ddc, 0, 0, 0x00E20746L);
  dc.SelectObject(brShadow);
  dc.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), ddc, 0, 0, 0x00E20746L);

  // reset DCs
  //
  dc.RestoreBrush();
  dc.RestoreBrush();
  ddc.RestoreBitmap();
}

owl::tstring GetHintText(TWindow* win, uint id, THintText hintType)
// Search the parent frame window of win and return the result of GetHintText
{
  TWindow* parent= win->GetParentO();
  TDecoratedFrame* frame= parent ?
    TYPESAFE_DOWNCAST(parent, TDecoratedFrame) : 0;
  while (parent && !frame) {
    parent = parent->GetParentO();
    if (parent)
      frame = TYPESAFE_DOWNCAST(parent, TDecoratedFrame);
  }
  return frame ? frame->GetHintText(id, hintType) : owl::tstring();
}
//
// Find resource module
//
TModule*
FindResourceModule(TWindow* parent, TModule* module, TResId resId, LPCTSTR type)
{
  if(module && module->FindResource(resId, type))
    return module;

  if(parent && parent->GetModule() && parent->GetModule()->FindResource(resId, type))
    return parent->GetModule();
  return TModule::FindResModule(resId, type);
}

} // OwlExt namespace

/*============================================================================*/

