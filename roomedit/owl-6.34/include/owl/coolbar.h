//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
/// \file
/// Definition of class TCoolBar.
//----------------------------------------------------------------------------

#if !defined(OWL_COOLBAR_H)
#define OWL_COOLBAR_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/control.h>
#include <owl/commctrl.h>
#include <owl/property.h>
#include <owl/shellitm.h>
#include <owl/template.h>

namespace owl {


class _OWLCLASS TImageList;
class _OWLCLASS TCoolBar;

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup newctrl
/// @{
/// \class TCoolBand
// ~~~~~ ~~~~~~~~~
//
class _OWLCLASS TCoolBand {
  friend class TCoolBar;
  public:
    TCoolBand();
    TCoolBand(uint id, const tstring& str = tstring(), TWindow* child=0, uint style=0);
    ~TCoolBand();

    TObjProperty<tstring>   Text;       ///< Band text label
    TObjProperty<TBitmap>      Bitmap;     ///< The band's background bitmap
    TObjProperty<TColor>      FgColor;    ///< Band colors -
    TObjProperty<TColor>      BgColor;    //
    TProperty<TWindow*>        Child;      ///< Handle of the band's child window
    TProperty<uint>             Style;      ///< Band style flags
    TProperty<uint>             Width;      ///< Width of the band: don't use -1
    TObjProperty<TSize>       MinSize;    ///< Minimum size of the band's child window don't use TSize(-1,-1)
    TProperty<int>             Id;         ///< The band's ID number: don't use -1
    TProperty<int>            ImageIndex; ///< Band image index (into rebar image list): don't use -1
    TProperty<uint>           Mask;       ///< Mask indicating which other members are valid

    bool operator < (const TCoolBand& band) { return this < &band; }
    bool operator ==(const TCoolBand& band) { return this == &band;}

    // for this operations must be valid CollBar Handle
    void  BeginDrag(const TPoint& point);
    void  DragMove(const TPoint& point);
    void  EndDrag();
    void  Maximize(bool ideal);
    void  Minimize();
    //bool   Move(); //?????????????????????????
    bool  ShowBand(bool show);

    void GetBorders(TRect& rect);
    void GetRect(TRect& rect);

  protected:
    // Initialise all data members to zero
    void Init();
    void SetupData(TCoolBar* parent, REBARBANDINFO& info);

    TCoolBar* Parent;
};

typedef TIPtrArray<TCoolBand*>     TCoolBandArray;
typedef TCoolBandArray::Iterator   TCoolBandArrayIter;

/// \class TCoolBar
/// This simple implementation does no more than encapsulte the messages which
/// can be sent to the rebar control. Possible extensions could manipulate bands
/// directly (by constructing a TCoolBand internally).
class _OWLCLASS TCoolBar : public TControl {
  friend class _OWLCLASS TCoolBand; //JJH class keyword can be before or after _OWLCLASS?
  public:
    // Constructors and destructors
    TCoolBar(TWindow* parent, int id, int x, int y, int w, int h, TModule* module = 0);
    TCoolBar(TWindow* parent, int resourceId, TModule* module = 0);
    ~TCoolBar();

    // Properties
    TFuncObjProperty<TColor>            BgColor;
    TFuncObjProperty<TColor>            TextColor;
    TFuncObjProperty<TColorScheme>      ColorScheme;
    TFuncPropertyVarPut<TPalette*>      Palette;
    TFuncPropertyVarPut<TWindow*>       Buddy; ///< "Parent" for notification messages
    TFuncPropertyVarPut<TTooltip*>       Tooltip;
    TFuncProperty<bool>      Unicode; // needs valid HWND
    TFuncObjPropertyGet<TCoolBandArray>  Bands;

    // insert/delete
    bool  Insert(TCoolBand* band, uint index = -1);
    bool  Delete(uint index = 0);

    // info
    //bool  GetInfo(TCoolBand& band, uint index);    //????????????????????
    //bool  SetInfo(TCoolBand& band, uint index);    //????????????????????
    bool  GetInfo(TRebarInfo& info);
    bool  SetInfo(TRebarInfo& info);

    void SetImageList(TImageList* list, TAutoDelete = AutoDelete);
    TImageList* GetImageList();

    // Row information
    uint GetRowCount();
    uint GetRowHeight(int index);

    // Style modifiers
    bool ToggleBandBorders();
    bool ToggleVarHeight();
    bool ToggleBorder();

    // Have to be valid HWND
    uint  GetBandCount();
    uint  GetHeight();
    TDropTarget GetDropTarget();
    HWND  GetToolTips();
    uint  HitTest(TRbHitTestInfo& tst);
    int    FindBand(uint id); // return index
    bool  SetRect(const TRect& rec);

  protected:
     virtual TGetClassNameReturnType GetClassName();
    void SetupWindow();
    void Init();

    void SetPalette(TPalette*  palette);
    void SetBuddy(TWindow* buddy);
    void SetTooltip(TTooltip* tip);

    void SetBgColor(const TColor& clr);
    TColor& GetBgColor();
    void SetTextColor(const TColor& clr);
    TColor& GetTextColor();
    void SetColorScheme(const TColorScheme& clr);
    TColorScheme& GetColorScheme();
    void SetUnicode(bool unicode);
    bool GetUnicode();
    TCoolBandArray& GetBands();

    // Rebar imagelist
    TCoolBandArray*  FBands;
    TRebarInfo       Info;
    TWindow*         FBuddy;
    TColor          FBgColor;
    TColor          FTextColor;
    TImageList*     FBandImages;
    bool             ShouldDelete;
    TColorScheme*    FColorScheme;
    TPalette*        FPalette;
    TTooltip*        FTooltip;
};
/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// Inline implementations

//
//
//
inline void
TCoolBand::BeginDrag(const TPoint& point)
{
  PRECONDITION(Parent && Parent->GetHandle());
  size_t index = Parent->FBands->Find(this); //JJH int -> size_t
  if(index != NPOS)
    Parent->SendMessage(RB_BEGINDRAG, (TParam1)(uint)index, MkUint32((uint16)point.x,(uint16)point.y));
}

//
//
//
inline void
TCoolBand::DragMove(const TPoint& point)
{
  PRECONDITION(Parent && Parent->GetHandle());
  Parent->SendMessage(RB_DRAGMOVE, 0, MkUint32((uint16)point.x,(uint16)point.y));
}

//
//
//
inline void
TCoolBand::EndDrag()
{
  PRECONDITION(Parent && Parent->GetHandle());
  Parent->SendMessage(RB_DRAGMOVE);
}

// RB_GETBANDBORDERS
// wParam = (WPARAM)(UINT) uBand;
// lParam = (LPARAM)(LPRECT) lprc;
inline void
TCoolBand::GetBorders(TRect& rect)
{
  PRECONDITION(Parent && Parent->GetHandle());
  size_t index = Parent->FBands->Find(this); //JJH int -> size_t/
  if(index != NPOS)
    Parent->SendMessage(RB_GETBANDBORDERS, (TParam1)index, (TParam2)(LPRECT)&rect);
}

//RB_GETRECT
//    wParam = (WPARAM)(INT) iBand;
//    lParam = (LPARAM)(LPRECT) lprc;
inline void
TCoolBand::GetRect(TRect& rect)
{
  PRECONDITION(Parent && Parent->GetHandle());
  size_t index = Parent->FBands->Find(this);//JJH int -> size_t
  if(index != NPOS)
    Parent->SendMessage(RB_GETRECT, (TParam1)index, (TParam2)(LPRECT)&rect);
}
//
//
//
inline void
TCoolBand::Maximize(bool ideal)
{
  PRECONDITION(Parent && Parent->GetHandle());
  size_t index = Parent->FBands->Find(this);//JJH int -> size_t
  if(index != NPOS)
    Parent->SendMessage(RB_MAXIMIZEBAND, (TParam1)index, (TParam2)(BOOL)ideal);
}
//
//
//
inline void
TCoolBand::Minimize()
{
  PRECONDITION(Parent && Parent->GetHandle());
  size_t index = Parent->FBands->Find(this);//JJH int -> size_t
  if(index != NPOS)
    Parent->SendMessage(RB_MINIMIZEBAND,(TParam1)index);
}
//
//
//
inline bool
TCoolBand::ShowBand(bool show)
{
  PRECONDITION(Parent && Parent->GetHandle());
  size_t index = Parent->FBands->Find(this);//JJH int -> size_t
  if(index != NPOS)
    return Parent->SendMessage(RB_SHOWBAND,TParam1(index),TParam2(show)) != 0;
  return false;
}


////////////////////////////////////////////////////////////////////////////////
//
// TCoolBar
// ~~~~~~~~
//
inline void
TCoolBar::SetBgColor(const TColor& clr)
{
  FBgColor = clr;
  if (GetHandle())
    SendMessage(RB_SETBKCOLOR, 0, TParam2((COLORREF)clr));
}
//
inline TColor&
TCoolBar::GetBgColor()
{
  if (GetHandle())
    FBgColor = (COLORREF)SendMessage(RB_GETBKCOLOR,0,0);
  return FBgColor;
}
//
inline void
TCoolBar::SetTextColor(const TColor& clr)
{
  FTextColor = clr;
  if (GetHandle())
    SendMessage(RB_SETTEXTCOLOR, 0, TParam2((COLORREF)clr));
}
//
inline TColor&
TCoolBar::GetTextColor()
{
  if (GetHandle())
    FTextColor = (COLORREF)SendMessage(RB_GETTEXTCOLOR,0,0);
  return FTextColor;
}
//#define RB_GETROWCOUNT  (WM_USER +  13)
//Message      RB_GETROWCOUNT
//Description  Retrieves the number of rows in the rebar.
//wParam       0 (not used)
//lParam       0 (not used)
//Returns      UINT -the number of rows
inline uint
TCoolBar::GetRowCount()
{
  PRECONDITION(GetHandle());
  return (uint)SendMessage(RB_GETROWCOUNT);
}

//#define RB_GETROWHEIGHT (WM_USER +  14)
//Message      RB_GETROWHEIGHT
//Description  Returns the height of the specified row in the rebar.
//wParam       UINT - the zero-based row to check
//lParam       0 (not used)
//Returns      UINT - the height, in pixels, of the row
inline uint
TCoolBar::GetRowHeight(int index)
{
  return (uint)SendMessage(RB_GETROWHEIGHT, index,0);
}

//
//
//
inline uint
TCoolBar::GetHeight()
{
  PRECONDITION(GetHandle());
  return (uint)SendMessage(RB_GETBARHEIGHT);
}

//RB_GETDROPTARGET
//    wParam = 0;
//    lParam = (IDropTarget**)ppDropTarget;
//Retrieves a rebar control's IDropTarget interface pointer.
inline TDropTarget
TCoolBar::GetDropTarget()
{
  IDropTarget* iface;
  SendMessage(RB_GETDROPTARGET,0,(TParam2)&iface);
  return TDropTarget(iface);
}

//RB_GETDROPTARGET
//    wParam = 0;
//    lParam = (IDropTarget**)ppDropTarget;
//Retrieves a rebar control's IDropTarget interface pointer.
inline HWND
TCoolBar::GetToolTips()
{
  return (HWND)SendMessage(RB_GETTOOLTIPS,0,0);
}

inline TImageList*
TCoolBar::GetImageList()
{
  return FBandImages;
}

//
//
//
inline void
TCoolBar::SetUnicode(bool unicode)
{
  PRECONDITION(GetHandle());
  SendMessage(RB_SETUNICODEFORMAT,unicode?1:0);
}

//
//
//
inline bool
TCoolBar::GetUnicode()
{
  PRECONDITION(GetHandle());
  return SendMessage(RB_GETUNICODEFORMAT,0,0) != 0;
}
//
//
//
inline uint
TCoolBar::HitTest(TRbHitTestInfo& info)
{
  PRECONDITION(GetHandle());
  return (uint)SendMessage(RB_HITTEST,0,TParam2(&info));
}
//
//
//
inline int
TCoolBar::FindBand(uint id)
{
  PRECONDITION(GetHandle());
  return (int)SendMessage(RB_IDTOINDEX,id);
}
//
//
//
inline bool
TCoolBar::SetRect(const TRect& rec)
{
  PRECONDITION(GetHandle());
  return SendMessage(RB_SIZETORECT,0,TParam2(&rec)) != 0;
}
//
//
//
inline TCoolBandArray&
TCoolBar::GetBands()
{
  PRECONDITION(FBands);
  return *FBands;
}

} // OWL namespace


#endif  // OWL_IECTRL_H
