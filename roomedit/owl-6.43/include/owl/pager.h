//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
/// \file
/// Definition of class TPageScroller.
//----------------------------------------------------------------------------

#if !defined(OWL_PAGER_H)
#define OWL_PAGER_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/control.h>
#include <owl/commctrl.h>
#include <owl/shellitm.h>

namespace owl {

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

/// \addtogroup newctrl
/// @{
/// \class TPageScroller
// ~~~~~ ~~~~~~~~~~~~~
//
class _OWLCLASS TPageScroller : public TControl {
  public:
    enum TPagerButton{
      pbTopOrLeft     = PGB_TOPORLEFT,
      pbBottomOrRight = PGB_BOTTOMORRIGHT,
    };

    TPageScroller (TWindow* parent,
                  int       id,
                  TWindow*  child,
                  int x, int y, int w, int h,
                  TModule*   module = 0);

    TPageScroller (TWindow* parent,
                   int       resourceId,
                  TWindow*  child,
                  TModule*   module = 0);
    ~TPageScroller();

    void     ForwardMouse(bool forward = true);

    void      SetBkColor(const TColor& clr);
    TColor  GetBkColor() const;

    void    SetBorder(int border);
    int      GetBorder() const;

    void    SetButtonSize(int btnsize);
    int      GetButtonSize() const;

    int      GetButtonState(TPagerButton button) const;
    TDropTarget GetDropTarget();
    void     SetPosition(int position);
    int     GetPosition() const;

    void    RecalcSize();

    void    SetChild(TWindow* child);

    // Override TWindow virtual member functions
    //
    void    SetupWindow();

  protected:
    // Override TWindow virtual member functions
    //
    virtual TGetClassNameReturnType GetClassName();

    TWindow*  ChildWindow;

  private:
    // Hidden to prevent accidental copying or assignment
    //
    TPageScroller(const TPageScroller&);
    TPageScroller& operator =(const TPageScroller&);

///YB Need to be consistent - Common Controls are not streamable for the
///YB the most part. This guy only invokes TControl's version anyway.
///YB
///YB DECLARE_STREAMABLE_FROM_BASE(_OWLCLASS, owl::TPageScroller, 1);
};
/// @}

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//
// inlines
//

inline void TPageScroller::ForwardMouse(bool forward){
  PRECONDITION(GetHandle());
  SendMessage(PGM_FORWARDMOUSE, TParam1((BOOL)forward), 0);
}

//
inline void TPageScroller::SetBkColor(const TColor& clr){
  PRECONDITION(GetHandle());
  SendMessage(PGM_SETBKCOLOR, 0, TParam2((COLORREF)clr));
}

//
inline TColor TPageScroller::GetBkColor() const{
  PRECONDITION(GetHandle());
   return static_cast<COLORREF>(CONST_CAST(TPageScroller*,this)->SendMessage(PGM_GETBKCOLOR,0,0));
}

//
inline void TPageScroller::SetBorder(int border){
  PRECONDITION(GetHandle());
   SendMessage(PGM_SETBORDER,0,TParam2(border));
}

//
inline int TPageScroller::GetBorder() const{
  PRECONDITION(GetHandle());
   return (int)CONST_CAST(TPageScroller*,this)->SendMessage(PGM_GETBORDER,0,0);
}

//
inline void TPageScroller::SetButtonSize(int btnsize){
  PRECONDITION(GetHandle());
   SendMessage(PGM_SETBUTTONSIZE,0,TParam2(btnsize));
}

//
inline int TPageScroller::GetButtonSize() const{
  PRECONDITION(GetHandle());
   return (int)CONST_CAST(TPageScroller*,this)->SendMessage(PGM_GETBUTTONSIZE,0,0);
}

inline int TPageScroller::GetButtonState(TPagerButton button) const{
  PRECONDITION(GetHandle());
   return (int)CONST_CAST(TPageScroller*,this)->SendMessage(PGM_GETBUTTONSTATE,0,TParam2(button));
}

inline TDropTarget TPageScroller::GetDropTarget(){
  PRECONDITION(GetHandle());
  IDropTarget* iface;
  SendMessage(PGM_GETDROPTARGET,0,TParam2(&iface));
  return TDropTarget(iface);
}

//
inline void TPageScroller::SetPosition(int pos){
  PRECONDITION(GetHandle());
   SendMessage(PGM_SETPOS,0,TParam2(pos));
}

//
inline int TPageScroller::GetPosition() const{
  PRECONDITION(GetHandle());
   return (int)CONST_CAST(TPageScroller*,this)->SendMessage(PGM_GETPOS,0,0);
}

//
inline void TPageScroller::RecalcSize(){
  PRECONDITION(GetHandle());
   SendMessage(PGM_RECALCSIZE,0,0);
}

} // OWL namespace

#endif  // OWL_MONTHCAL_H
