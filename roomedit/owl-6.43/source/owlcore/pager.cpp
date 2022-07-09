//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Bidus Yura, All Rights Reserved
//
/// \file
/// Implementation of the TPageScroller class
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/pager.h>


namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlCommCtrl);        // CommonCtrl Diagnostic group

//
//
//
TPageScroller::TPageScroller(TWindow* parent, int id, TWindow* child, int x, int y,
                       int w, int h, TModule* module)
:
  TControl(parent,id,_T(""),x,y,w,h,module),
  ChildWindow(child)
{
  // Does this apply to extended common controls?
  if (!TCommCtrl::IsAvailable() &&
       TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionIE4)
    TXCommCtrl::Raise();

  TCommCtrl::Dll()->CheckCommonControl(ICC_PAGESCROLLER_CLASS);
}


//
//
//
TPageScroller::TPageScroller(TWindow* parent, int resourceId, TWindow* child,
                       TModule* module)
:
  TControl(parent, resourceId, module),
  ChildWindow(child)
{
  // Does this apply to extended common controls?
  if (!TCommCtrl::IsAvailable() &&
       TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionIE4)
    TXCommCtrl::Raise();

  TCommCtrl::Dll()->CheckCommonControl(ICC_PAGESCROLLER_CLASS);

  DisableTransfer();
}

//
//
//
TPageScroller::~TPageScroller()
{
}


// Return the proper class name.
// Windows class: WC_PAGESCROLLER is defined in commctrl.h
TWindow::TGetClassNameReturnType
TPageScroller::GetClassName()
{
  return WC_PAGESCROLLER;
}

//
//
//
void
TPageScroller::SetChild(TWindow* child)
{
  ChildWindow = child;
  if(child && GetHandle()){
    if(!child->GetHandle())
      child->Create();
     SendMessage(PGM_SETCHILD,0,TParam2(child->GetHandle()));
  }
}

void
TPageScroller::SetupWindow()
{
  TControl::SetupWindow();
  if(ChildWindow)
    SetChild(ChildWindow);
}
} // OWL namespace
/* ========================================================================== */
