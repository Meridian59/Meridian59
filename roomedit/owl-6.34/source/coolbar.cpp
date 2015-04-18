//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Bidus Yura, All Rights Reserved
//
/// \file
/// Implementation of the TCoolBar class
//----------------------------------------------------------------------------
#include <owl/pch.h>

#include <owl/coolbar.h>
#include <owl/imagelst.h>
#include <owl/tooltip.h>
#include <owl/functor.h>
#include <stdio.h>

namespace owl {

OWL_DIAGINFO;
DIAG_DECLARE_GROUP(OwlCommCtrl);        // CommonCtrl Diagnostic group

// Maintain backward compatibility with IE3
static uint32
SizeOfReBarBandInfo(REBARBANDINFO& info)
{
  info.cbSize = REBARBANDINFO_V3_SIZE;
  if(TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionWin95)
    info.cbSize = sizeof(REBARBANDINFO);
  return info.cbSize;
}

//
// Constructor of TCoolBar control
//
TCoolBar::TCoolBar(TWindow* parent, int id, int x, int y, int w, int h,
                   TModule* module)
:
  TControl(parent, id, _T(""), x, y, w, h, module),
  BgColor(GETPUTOBJMFUNCTOR(TColor,TCoolBar,GetBgColor,SetBgColor)),
  TextColor(GETPUTOBJMFUNCTOR(TColor,TCoolBar,GetTextColor,SetTextColor)),
  ColorScheme(GETPUTOBJMFUNCTOR(TColorScheme,TCoolBar,GetColorScheme,SetColorScheme)),
  Palette(FPalette,PUTMFUNCTOR(TPalette*,TCoolBar,SetPalette)),
  Buddy(FBuddy,PUTMFUNCTOR(TWindow*,TCoolBar,SetBuddy)),
  Tooltip(FTooltip,PUTMFUNCTOR(TTooltip*,TCoolBar,SetTooltip)),
  Unicode(GETPUTMFUNCTOR(bool,TCoolBar,GetUnicode,SetUnicode)),
  Bands(GETOBJMFUNCTOR(TCoolBandArray,TCoolBar,GetBands))
{
  // Modify the style to some reasonable defaults...
  // ... this comes from the Microsoft example by Nancy Winnick Cluts
  Attr.Style = WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|
               CCS_NODIVIDER|CCS_NOPARENTALIGN|
               RBS_VARHEIGHT     | // This style is trashed by TDecoratedFrame::Insert
               RBS_BANDBORDERS;    // This style is trashed by TDecoratedFrame::Insert
               //WS_BORDER;        // This style must be added after TDecoratedFrame::Insert
  Init();
}


TCoolBar::TCoolBar(TWindow* parent, int resourceId, TModule* module)
:
  TControl(parent, resourceId, module),
  BgColor(GETPUTOBJMFUNCTOR(TColor,TCoolBar,GetBgColor,SetBgColor)),
  TextColor(GETPUTOBJMFUNCTOR(TColor,TCoolBar,GetTextColor,SetTextColor)),
  ColorScheme(GETPUTOBJMFUNCTOR(TColorScheme,TCoolBar,GetColorScheme,SetColorScheme)),
  Palette(FPalette,PUTMFUNCTOR(TPalette*,TCoolBar,SetPalette)),
  Buddy(FBuddy,PUTMFUNCTOR(TWindow*,TCoolBar,SetBuddy)),
  Tooltip(FTooltip,PUTMFUNCTOR(TTooltip*,TCoolBar,SetTooltip)),
  Unicode(GETPUTMFUNCTOR(bool,TCoolBar,GetUnicode,SetUnicode)),
  Bands(GETOBJMFUNCTOR(TCoolBandArray,TCoolBar,GetBands))
{
  Init();
}

//
//
// Destructor
TCoolBar::~TCoolBar()
{
  if(ShouldDelete)
    delete FBandImages;
  delete FColorScheme;
  delete FPalette;
  delete FBands;
  //delete FTooltip; //???????????????????
}

//
//
//
void
TCoolBar::Init()
{
  // Initialise the image list pointer
  FBands        = new TCoolBandArray;
  FBandImages   = 0;
  FBuddy        = 0;
  FPalette       = 0;
  FColorScheme  = 0;
  FBgColor        = TColor::None;
  FTextColor     = TColor::None;
  FTooltip      = 0;

  // Does this apply to extended common controls?
  if (!TCommCtrl::IsAvailable() &&
       TCommCtrl::Dll()->GetCtrlVersion() > ComCtlVersionWin95)
    TXCommCtrl::Raise();

  TCommCtrl::Dll()->CheckCommonControl(ICC_COOL_CLASSES);

  // Update flag based on availability of Common Control Library
  //
}

// Return the proper class name.
// Windows class: REBARCLASSNAME is defined in commctrl.h
TWindow::TGetClassNameReturnType
TCoolBar::GetClassName()
{
  return REBARCLASSNAME;
}

//
//
//
void
TCoolBar::SetupWindow()
{
  ModifyStyle(0,CCS_NODIVIDER|CCS_NOPARENTALIGN|RBS_VARHEIGHT|RBS_BANDBORDERS|WS_BORDER);

  TControl::SetupWindow();

  // Initialise the rebar.
  // This would typically be used to associate an image list with
  // the rebar. It is not clear yet whether the rebar cleans up this
  // image list itself. If I use a TImageList for this who deletes it?
  // If me is the handle still valid? My treatment seems to work.

  Info.fMask &= ~RBIM_IMAGELIST;
  Info.himl = 0;
  if(FBandImages)
    Info.SetImageList(*FBandImages);
  SendMessage(RB_SETBARINFO, 0, TParam2(&Info));
  if(FColorScheme)
    SendMessage(RB_SETCOLORSCHEME,0,TParam2(FColorScheme));
  if(FPalette)
    SendMessage(RB_SETPALETTE,0,TParam2(FPalette->GetHandle()));
  if(FBgColor != TColor::None)
    SendMessage(RB_SETBKCOLOR, 0, TParam2((COLORREF)FBgColor));
  if(FTextColor != TColor::None)
    SendMessage(RB_SETTEXTCOLOR, 0, TParam2((COLORREF)FTextColor));
  if(FBuddy && FBuddy->GetHandle())
    SendMessage(RB_SETPARENT, TParam1(FBuddy->GetHandle()));
  if(FTooltip){
    FTooltip->SetParent(this);
    if(!FTooltip->GetHandle())
      FTooltip->Create();
    SendMessage(RB_SETTOOLTIPS, TParam1(FTooltip->GetHandle()));
  }
  for(int i = 0; i < (int)FBands->Size(); i++){
    REBARBANDINFO info;
    (*FBands)[i]->SetupData(this, info);
    SendMessage(RB_INSERTBAND, i, TParam2(&info));
  }
}

//#define RB_INSERTBANDA  (WM_USER +  1)
//#define RB_INSERTBANDW  (WM_USER +  10)
//Message      RB_INSERTBAND
//Description  Inserts a band into a rebar.
//wParam       UINT - the zero-based index of the place to insert the band; -1 signifies
//             that the band will be added to the end
//lParam       LPREBARINFO [???] - pointer to a structure containing information about the band
//Returns      0 if successful; non-zero otherwise
bool
TCoolBar::Insert(TCoolBand* band, uint index)
{
  uint idx = index;
  if(index == static_cast<uint>(-1)) //JJH added static_cast
    idx = FBands->Add(band);
  else
    FBands->AddAt(band, index);
  if (GetHandle()){
    REBARBANDINFO info;
    (*FBands)[idx]->SetupData(this, info);
    return SendMessage(RB_INSERTBAND, index, TParam2(&info)) == 0;
  }
  return true;
}


//#define RB_DELETEBAND   (WM_USER +  2)
//Message      RB_DELETEBAND
//Description  Removes a band from the rebar.
//wParam       UINT - the zero-based index of the band to delete
//lParam       0 (not used)
//Returns      0 if successful; non-zero otherwise
bool
TCoolBar::Delete(uint index)
{
  FBands->Destroy(index);
  if (GetHandle())
    return SendMessage(RB_DELETEBAND, index)==0;
  return true;
}

//#define RB_GETBARINFO   (WM_USER +  3)
//Message      RB_GETBARINFO
//Description  Retrieves information about the rebar control and the image list it may use.
//wParam       0 (not used)
//lParam       LPREBARINFO - pointer to a structure containing rebar information
//Returns      0 if successful; non-zero otherwise
bool
TCoolBar::GetInfo(TRebarInfo& info)
{
  if (GetHandle())
    return SendMessage(RB_GETBARINFO, 0, TParam2(&info))==0;
  info = Info;
  return true;
}

//#define RB_SETBARINFO   (WM_USER +  4)
//Message      RB_SETBARINFO
//Description  Sets information about the rebar.
//wParam       0
//lParam       LPREBARINFO - pointer to a structure containing new rebar information
//Returns      0 if successful; non-zero otherwise
bool
TCoolBar::SetInfo(TRebarInfo& info)
{
  Info = info;
  Info.fMask &= ~RBIM_IMAGELIST;
  Info.himl = 0;
  if(FBandImages)
    Info.SetImageList(*FBandImages); // Sets NULL aswell to use no images.
  if (GetHandle())
    return SendMessage(RB_SETBARINFO, 0, TParam2(&Info))==0;
  return true;
}

//#define RB_SETPARENT    (WM_USER +  7)
//Message      RB_SETPARENT
//Description  Sets the window that will receive rebar notification messages. Note that this
//             message does not actually set the parent of the Rebar control. When you call
//             GetParent, you will still get the handle to the original parent window that was
//             specified when the Rebar control was created.
//wParam       HWND - handle to the new parent window
//lParam       0
//Returns      HWND - handle to the previous parent window; NULL if there is no previous parent
void
TCoolBar::SetBuddy(TWindow* buddy)
{
  FBuddy = buddy;
  if(FBuddy && FBuddy->GetHandle() && GetHandle())
    SendMessage(RB_SETPARENT, TParam1(FBuddy->GetHandle()));
}

void
TCoolBar::SetTooltip(TTooltip* tip)
{
  if(GetHandle())
    SendMessage(RB_SETTOOLTIPS, TParam1(tip->GetHandle()));
  delete FTooltip;
  FTooltip = tip;
}

//#define RB_GETBANDCOUNT (WM_USER +  12)
//Message      RB_GETBANDCOUNT
//Description  Returns the number of bands in the rebar.
//wParam       0 (not used)
//lParam       0 (not used)
//Returns      UINT - Number of bands in the rebar
uint
TCoolBar::GetBandCount()
{
  if(GetHandle())
    return (uint)SendMessage(RB_GETBANDCOUNT);
  return FBands->Size();
}

//
//
//
void
TCoolBar::SetImageList(TImageList* images, TAutoDelete del)
{
  // This is intended to change the imagelist after creation...
  // ... as yet untested.
  // Delete the old image list and assign the new one.
  if(ShouldDelete)
    delete FBandImages;
  Info.fMask &= ~RBIM_IMAGELIST;
  Info.himl = 0;
  FBandImages = images;
  ShouldDelete = del == AutoDelete;
  if(FBandImages)
    Info.SetImageList(*FBandImages); // Sets NULL aswell to use no images.

  if (GetHandle())
    SendMessage(RB_SETBARINFO, 0, TParam2(&Info));
}

//
//
//
bool
TCoolBar::ToggleBandBorders()
{
  // Check the existing style
  bool result = GetStyle() & (uint32)RBS_BANDBORDERS;
  if(result)
    ModifyStyle(RBS_BANDBORDERS, 0, 0);
  else
    ModifyStyle(0, RBS_BANDBORDERS, 0);

  // Set the window position to force a redraw - the "+ 1" seems to do the trick
  if(GetHandle()){
    TRect rect = GetWindowRect();
    SetWindowPos(0,0,0,rect.Width(),rect.Height()+1,SWP_NOZORDER|SWP_NOCOPYBITS);
  }

  // Return the new setting
  return !result;
}


//
//
//
bool
TCoolBar::ToggleVarHeight()
{
  // Check the existing style
  bool result = GetStyle() & (uint32)RBS_VARHEIGHT;
  if(result)
    ModifyStyle(RBS_VARHEIGHT, 0, 0);
  else
    ModifyStyle(0, RBS_VARHEIGHT, 0);

  // Set the window position to force a full redraw. The "+ 1" seems to do the trick
  if(GetHandle()){
    TRect rec = GetWindowRect();
    SetWindowPos(0,0,0,rec.Width(),rec.Height()+1,SWP_NOZORDER|SWP_NOCOPYBITS);
  }
  // Return the new setting
  return !result;
}


//
//
//
bool
TCoolBar::ToggleBorder()
{
  // Check the existing style
  bool result = GetStyle() & (uint32)WS_BORDER;
  if (result)
    ModifyStyle(WS_BORDER, 0, 0);
  else
    ModifyStyle(0, WS_BORDER, 0);

  // Set the window position to force a full redraw. The "+ 1" seems to do the trick
  if(GetHandle()){
    TRect rec = GetWindowRect();
    SetWindowPos(0,0,0,rec.Width(),rec.Height()+1,SWP_NOZORDER|SWP_NOCOPYBITS);
    // Tell the parent window to layout again - I don't know why it does not
    // do this itself re the border
    if(Parent){
      TNotify notify(GetHandle(), GetId(), RBN_HEIGHTCHANGE);
      Parent->HandleMessage(WM_NOTIFY, GetId(), (long)&notify);
    }
  }
  // Return the new setting
  return !result;
}

void
TCoolBar::SetColorScheme(const TColorScheme& clr)
{
  if(!FColorScheme)
    FColorScheme = new TColorScheme;
  *FColorScheme  = clr;
  if(GetHandle())
    SendMessage(RB_SETCOLORSCHEME,0,TParam2(FColorScheme));
}
//
//
//
TColorScheme&
TCoolBar::GetColorScheme()
{
  if(!FColorScheme)
    FColorScheme = new TColorScheme;
  if(GetHandle())
    SendMessage(RB_GETCOLORSCHEME,0,TParam2(FColorScheme));
  return *FColorScheme;
}
//
//
//
void
TCoolBar::SetPalette(TPalette* palette)
{
  delete FPalette;
  FPalette = palette;
  if(GetHandle())
    SendMessage(RB_SETPALETTE,0,TParam2(FPalette->GetHandle()));
}
//
// TCoolBand
// ~~~~~~~~~
//

TCoolBand::TCoolBand()
{
  Init();
}

TCoolBand::TCoolBand(uint id, const tstring& str, TWindow* child, uint style)
{
  Init();
  Id = id;
  Text(str);
  Child = child;
  Style = style;
}

//
//
//
TCoolBand::~TCoolBand()
{
  //delete Bitmap;
}

//
//
//
void
TCoolBand::Init()
{
  Child  = 0;
  //Bitmap = 0;
  FgColor  = TColor::None;
  BgColor  = TColor::None;
  Style    = 0;
  Width    = 0;
  MinSize  = TSize(-1,-1);
  Id       = -1;
  ImageIndex = -1;
  Mask     = 0;
}
//
//
//
void
TCoolBand::SetupData(TCoolBar* parent, REBARBANDINFO& info)
{
  memset(&info, 0, sizeof(REBARBANDINFO));
  SizeOfReBarBandInfo(info);

  if(Mask)
    info.fMask = Mask;

  if(Child){
    //Child()->SetBkgndColor(TColor::Transparent);
    //Child()->ModifyExStyle(0,WS_EX_TRANSPARENT);
    if(!Child()->GetHandle())
      Child()->Create();
    info.fMask |= RBBIM_CHILD;
     info.hwndChild = Child()->GetHandle();
  }
  if(Bitmap){
    info.fMask |= RBBIM_BACKGROUND;
    info.hbmBack = Bitmap->GetHandle();
   }
  if(TColor::None != FgColor){
    info.fMask |= RBBIM_COLORS;
    info.clrFore = FgColor();
  }
  if(TColor::None != BgColor){
    info.fMask |= RBBIM_COLORS;
    info.clrBack = BgColor();
  }
  if(Style){
     info.fMask |= RBBIM_STYLE;
    info.fStyle = Style;
  }
  if(Width != uint(-1)){
    info.fMask |= RBBIM_SIZE;
    info.cx = Width;
  }
  if(MinSize() != TSize(-1,-1)){
    info.fMask |= RBBIM_CHILDSIZE;
    info.cxMinChild = MinSize().cx;
    info.cyMinChild = MinSize().cy;
  }
  if(Id != -1){
    info.fMask |= RBBIM_ID;
    info.wID = Id;
  }
  if(ImageIndex != -1){
    info.fMask |= RBBIM_IMAGE;
    info.iImage = ImageIndex;
  }
  if(!Text().empty()){
    info.fMask  |= RBBIM_TEXT;
    info.lpText = (tchar*)Text().c_str();
    info.cch    = static_cast<UINT>(Text().length());
  }
  Parent = parent;
}
} // OWL namespace
/* ========================================================================== */
