//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1998 by Yura Bidus, All Rights Reserved
//
/// \file
/// Definition of class TFlatScroller.
//----------------------------------------------------------------------------

#if !defined(OWL_FSCROLL_H)
#define OWL_FSCROLL_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

#include <owl/scroller.h>
#include <owl/commctrl.h>

namespace owl {

#include <owl/preclass.h>

/// \addtogroup newctrl
/// @{
/// \class TFlatScroller
// ~~~~~ ~~~~~~~~~~~~~
/// Class TFlatScroller implements the actual scroller object. All functions
/// are inline or virtual to avoid pulling in code when no scrollers have
/// been constructed.
//
class _OWLCLASS TFlatScroller : public TScroller {
  public:
    TFlatScroller(TWindow* window,
              int      xUnit,  int  yUnit,
              long     xRange, long yRange);
    virtual ~TFlatScroller();

    virtual void SetWindow(TWindow* win);
    virtual bool EnableScrollBar(uint flags=SB_BOTH, uint arrow=ESB_ENABLE_BOTH);
    virtual void ShowScrollBar(int code, bool show=true);

    void  EnableFlatStyle(bool enable=true);
    bool   GetScrollProp(uint index, int& value);
    bool   SetScrollProp(uint index, int value, bool redraw = true);

  protected:
    virtual void   SetScrollPage(int bar, int page, bool redraw = true);
    virtual int   GetScrollPage(int bar) const;
    virtual void  GetScrollRange(int bar, int& minPos, int& maxPos) const;
    virtual void  SetScrollRange(int  bar, int  minPos, int  maxPos, bool redraw = true);
    virtual int   GetScrollPos(int bar) const;
    virtual int   SetScrollPos(int bar, int pos, bool redraw = true);
    virtual int GetScrollTrackPos(int bar) const;

  DECLARE_STREAMABLE_OWL(TFlatScroller, 1);
};
/// @}

DECLARE_STREAMABLE_INLINES(owl::TFlatScroller);

#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
inline void TFlatScroller::SetWindow(TWindow* win)
{
  TScroller::SetWindow(win);
   EnableFlatStyle(true);
}

//
inline void TFlatScroller::EnableFlatStyle(bool enable)
{
  if(Window->GetHandle()){
    if(enable)
      InitializeFlatSB(*Window);
    else
      UninitializeFlatSB(*Window);
  }
}

//
inline bool TFlatScroller::GetScrollProp(uint index, int& value)
{
  PRECONDITION(Window->GetHandle());
  return FlatSB_GetScrollProp(*Window, index, &value);
}

//
inline bool TFlatScroller::SetScrollProp(uint index, int value, bool redraw)
{
  PRECONDITION(Window->GetHandle());
  return FlatSB_SetScrollProp(*Window, index, value, redraw);
}

//
inline bool TFlatScroller::EnableScrollBar(uint index, uint arrow)
{
  PRECONDITION(Window->GetHandle());
  return FlatSB_EnableScrollBar(*Window, index, arrow);
}

//
inline void TFlatScroller::ShowScrollBar(int code, bool show)
{
  PRECONDITION(Window->GetHandle());
  FlatSB_ShowScrollBar(*Window, code, show);
}

////////////////////////////////////////////////////////////////////////////////

} // OWL namespace


#endif  // OWL_SCROLLER_H
