//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of helper classes for Pane Splitters
//----------------------------------------------------------------------------

#if !defined(OWL_SPLITTER_H)
#define OWL_SPLITTER_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/layoutwi.h>
#include <owl/shddel.h>
#include <owl/template.h> //added by DLN

namespace owl {

class _OWLCLASS TDC;

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// Enumeration describing whether to split a window in the X or Y plane.
//
enum TSplitDirection {
  psHorizontal,               ///< Horizontal split
  psVertical,                 ///< Vertical split
  psNone                      ///< Unspecified split
};

class _OWLCLASS TSplitterIndicator;
class _OWLCLASS TPaneSplitter;

//
/// \class TSplitter
// ~~~~~ ~~~~~~~~~
/// Abstract base class for TVSplitter and THSplitter.
//
class _OWLCLASS TSplitter : public TLayoutWindow {
  public:
    TSplitter(TWindow* parent, TPaneSplitter* ps, float percent);

    int                         operator ==(const TSplitter& splitter) const;
    float                       GetPercent();
    void                        SetPercent(float p);
    virtual TRect               GetRect() = 0;
    TRect                       GetScreenRect();
    virtual TSplitDirection     SplitDirection() = 0;
    virtual TSplitterIndicator* CreateSplitterIndicator() = 0;
    virtual TWindow*            Pane1() = 0;
    virtual TWindow*            Pane2() = 0;
    virtual void                Split(TWindow* targetPane,
                                      TWindow* newPane,
                                      TSplitDirection sd,
                                      float percent) = 0;
    virtual TLayoutMetrics      Setup(TWindow* targetPane,
                                      TWindow* newPane,
                                      float percent) = 0;
    virtual void                AdjForResize(const TSize& sz) = 0;
    TLayoutWindow*              RemovePane(TWindow* pane,
                                           TShouldDelete::DeleteType dt =
                                             TShouldDelete::DefDelete);
    TRect                       GetMoveArea();
    virtual void                Move(int dist) = 0;
    virtual void                AdjSplitterWidth(int w) = 0;

  protected:
    void                        EvLButtonDown(uint modKeys, const TPoint& point);
    void                        EvLButtonUp(uint modKeys, const TPoint& point);
    void                        EvMouseMove(uint modKeys, const TPoint& point);
    bool                        EvSetCursor(HWND /*hWndCursor*/,
                                            uint /*hitTest*/,
                                            uint /*mouseMsg*/);
    void                        EvSize(uint sizeType, const TSize& size);
    void                        Paint(TDC& dc, bool erase, TRect& rect);

    void                        SetupEpilog(TSplitter* s, TWindow* targetPane,
                                            TWindow* newPane,
                                            TLayoutMetrics& lmOfTargetPane,
                                            TLayoutMetrics& lmOfNewPane);

  protected_data:
    // Represents the percent 'pane1' is of splitter (.5, .25, etc.).
    //
    float          PercentOf;
    TPaneSplitter* PaneSplitter;

  DECLARE_RESPONSE_TABLE(TSplitter);
};

//
/// \class TVSplitter
// ~~~~~ ~~~~~~~~~~
/// Represents a vertical splitter.
//
class _OWLCLASS TVSplitter : public TSplitter {
  public:
    TVSplitter(TWindow* parent, TPaneSplitter* ps, float percent=50.0f);

    // Pane1 is left of splitter and pane2 is right.
    //
    TWindow*             Pane1();
    TWindow*             Pane2();
    void                 Split(TWindow* targetPane, TWindow* newPane,
                               TSplitDirection sd, float percent=50.0f);
    TLayoutMetrics       Setup(TWindow* targetPane, TWindow* newPane, float percent);
    TRect                GetRect();
    TSplitDirection      SplitDirection();
    TSplitterIndicator*  CreateSplitterIndicator();
    void                 AdjForResize(const TSize& sz);
    void                 Move(int dist);
    void                 AdjSplitterWidth(int w);
};

//
/// \class THSplitter
// ~~~~~ ~~~~~~~~~~
/// Represents a horizontal splitter.
//
class _OWLCLASS THSplitter : public TSplitter {
  public:
    THSplitter(TWindow* parent, TPaneSplitter* ps, float percent=50.0f);

    // Pane1 is above splitter and pane2 is below.
    //
    TWindow*             Pane1();
    TWindow*             Pane2();
    void                 Split(TWindow* targetPane, TWindow* newPane,
                               TSplitDirection sd, float percent=50.0f);
    TLayoutMetrics       Setup(TWindow* targetPane, TWindow* newPane, float percent);
    TRect                GetRect();
    TSplitDirection      SplitDirection();
    TSplitterIndicator*  CreateSplitterIndicator();
    void                 AdjForResize(const TSize& sz);
    void                 Move(int dist);
    void                 AdjSplitterWidth(int w);
};

//
/// \class TSplitterIndicator
// ~~~~~ ~~~~~~~~~~~~~~~~~~
/// Indicates to the user where the splitter will be moved to when dragging
/// completes. Abstract base class for TVSplitterIndicator and
/// THSplitterIndicator.
//
class _OWLCLASS TSplitterIndicator : public TRect {
  public:
    TSplitterIndicator(TSplitter* splitter, const TRect& rect, uint cushion=0);
    bool              operator == (const TSplitterIndicator& si) const;
    // make happy STL
    bool              operator < (const TSplitterIndicator& si) const;
    virtual void      ConnectToRect(const TRect& rect) = 0;
    virtual void      Move(int dist) = 0;
    uint              GetDistMoved();
    void              Draw();
    void              Clear();
    virtual int       CalcDistMoved(const TPoint& start, const TPoint& cur) = 0;
    virtual bool      CouldContain(const TPoint& point) = 0;
    TSplitter*        GetSplitter();
    virtual TRect     CalcAreaOfSplitterMove() = 0;
    void              SetCushion(const uint cushion);

  protected:
    TSplitter*  Splitter;
    bool        Showing;
    uint        DistMoved;
    //
    // Minimum distance btwn separators.
    // 
    uint        Cushion;
};

//
/// \class TVSplitterIndicator
// ~~~~~ ~~~~~~~~~~~~~~~~~~~
/// Vertical indicator.
//
class _OWLCLASS TVSplitterIndicator : public TSplitterIndicator {
  public:
    TVSplitterIndicator(TSplitter* splitter, const TRect& rect, uint cushion=0);

    // Override virtuals
    //
    void    ConnectToRect(const TRect& rect);
    void    Move(int dist);
    int     CalcDistMoved(const TPoint& start, const TPoint& cur);
    bool    CouldContain(const TPoint& point);
    TRect   CalcAreaOfSplitterMove();
};

//
/// \class THSplitterIndicator
// ~~~~~ ~~~~~~~~~~~~~~~~~~~
/// Horizontal indicator.
//
class _OWLCLASS THSplitterIndicator : public TSplitterIndicator {
  public:
    THSplitterIndicator(TSplitter* splitter, const TRect& rect, uint cushion=0);

    // Override virtuals
    //
    void    ConnectToRect(const TRect& rect);
    void    Move(int dist);
    int     CalcDistMoved(const TPoint& start, const TPoint& cur);
    bool    CouldContain(const TPoint& point);
    TRect   CalcAreaOfSplitterMove();
};

//
//DLN replaced by class decl below class TSplitterIndicatorList;

//------------------------------------------------------------------------------
/// \class TSplitterIndicatorList
// ~~~~~ ~~~~~~~~~~~~~~~~~~~~~~
class TSplitterIndicatorList: public TBaseList<TSplitterIndicator*>{
  public:
    TSplitterIndicatorList(){;}
    uint  NSplitterIndicators() { return Size(); }
    TSplitterIndicator* FindIndicatorWithSplitter(TSplitter* splitter);
    void Flush(bool del);
};
typedef TSplitterIndicatorList::Iterator TSplitterIndicatorListIterator;

//
/// \class TSplitterIndicatorMgr
// ~~~~~ ~~~~~~~~~~~~~~~~~~~~~
/// Handles the moving of the indicators.
//
class _OWLCLASS TSplitterIndicatorMgr {
  public:
    TSplitterIndicatorMgr();

    void            StartMove(TSplitterIndicatorList& sil, const TPoint& point);
    void            EndMove();
    void            MoveIndicators(const TPoint& point);

  private:
    TSplitterIndicatorList*   SplitterIndicatorList;
    TPoint                    StartDragPoint;

    void            DrawIndicators();
    void            ClearIndicators();
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations

//
inline int TSplitter::operator ==(const TSplitter& splitter) const
{
  return this == &splitter;
}

//
inline float TSplitter::GetPercent()
{
  return PercentOf;
}

//
inline void TSplitter::SetPercent(float p)
{
  PercentOf = p;
}

//
/// Return the area the splitter can move in (bounding rectangle).
/// The rect is in client coordinates.
//
inline TRect TSplitter::GetMoveArea()
{
  return GetWindowRect();
}

//
/// Similar to above except rect is in screen coordinates.
//
inline TRect TSplitter::GetScreenRect()
{
  TRect  rect = GetRect();
  MapWindowPoints(0, (TPoint*)&rect, 2);  // map to screen
  return rect;
}

//
inline TSplitDirection TVSplitter::SplitDirection()
{
  return psVertical;
}

//
inline TSplitDirection
THSplitter::SplitDirection()
{
  return psHorizontal;
}

//
inline TSplitterIndicator::TSplitterIndicator(TSplitter* splitter, const TRect& rect, uint cushion)
:
  TRect(rect), Splitter(splitter), Showing(false), DistMoved(0), Cushion(cushion)
{
}

//
inline bool TSplitterIndicator::operator ==(const TSplitterIndicator& si) const
{
  return this == &si;
}

//
inline uint
TSplitterIndicator::GetDistMoved()
{
  return DistMoved;
}

//
/// Clear indicator from screen.
//
inline void
TSplitterIndicator::Clear()
{
  if (Showing) {
    Showing = false;
    Draw();
    Showing = false;
  }
}

//
/// Return splitter from which this indicator was created.
//
inline TSplitter*
TSplitterIndicator::GetSplitter()
{
  return Splitter;
}

//
/// Set cushion
//
inline void
TSplitterIndicator::SetCushion(const uint cushion)
{
   Cushion = cushion;
}

//
inline TVSplitterIndicator::TVSplitterIndicator(TSplitter* splitter, const TRect& rect, uint cushion)
:
  TSplitterIndicator(splitter, rect, cushion)
{
}

//
inline THSplitterIndicator::THSplitterIndicator(TSplitter* splitter, const TRect& rect, uint cushion)
:
  TSplitterIndicator(splitter, rect, cushion)
{
}

/*
inline uint TSplitterIndicatorList::NSplitterIndicators()
{
  return size();
}
inline void TSplitterIndicatorList::Flush(bool del){
  if(del){
    TSplitterIndicatorList::iterator i = begin();
    while(i != end())
      delete *i;
  }
  this->erase(begin(),end());
}
*/
//
inline TSplitterIndicatorMgr::TSplitterIndicatorMgr()
:
  SplitterIndicatorList(0)
{
}

} // OWL namespace


#endif // OWL_SPLITTER_H
