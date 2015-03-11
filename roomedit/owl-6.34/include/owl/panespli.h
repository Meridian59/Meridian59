//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
// Definition of Pane Splitter classes
//----------------------------------------------------------------------------

#if !defined(OWL_PANESPLI_H)
#define OWL_PANESPLI_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/splitter.h>

/*  we are not using owl's resource files
#if !defined(OWL_PANESPLI_RH)
# include <owl/panespli.rh>
#endif
*/


namespace owl {

/// \cond

//
// Define macro [missing from 16-bit headers]used by PANESPLI.CPP
//
#if !defined(IDC_SIZEALL)
# define IDC_SIZEALL         MAKEINTRESOURCE(32646)
#endif

/// \endcond

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>

//
/// \class TPaneSplitter
// ~~~~~ ~~~~~~~~~~~~~
/// TPaneSplitter is a class that acts as a container for child windows (called
/// panes) and splitters (pane mover, separator). It supports operations for
/// manipulating panes (add, remove, move, resize, etc) and splitters (drawing,
/// width, etc). Splitters can be moved (thereby resizing the panes) by function
/// call or mouse input. Using the mouse, multiple splitters can be moved
/// simultaneously.
//
class _OWLCLASS TPaneSplitter : public TLayoutWindow
{
  public:
    typedef TShouldDelete::TDelete TDelete;

    TPaneSplitter(TWindow*        parent,
                  LPCTSTR title = 0,
                  int             splitterWidth = 0,
                  TModule*        module = 0);

    TPaneSplitter(
      TWindow* parent,
      const tstring& title,
      int splitterWidth = 0,
      TModule* module = 0);

    ~TPaneSplitter();

    // Pane functions
    //
    virtual bool  SplitPane(TWindow* target, TWindow* newPane,
                            TSplitDirection splitDir, float percent = 0.50f);
    bool          RemovePane(TWindow* pane, TDelete dt = TShouldDelete::DefDelete);
    bool          ReplacePane(TWindow* target, TWindow* newPane,
                              TDelete dt = TShouldDelete::DefDelete);
    bool          SwapPanes(TWindow* pane1, TWindow* pane2);
    int           PaneCount();
    void          RemoveAllPanes(TDelete dt = TShouldDelete::DefDelete);

    typedef int (*TForEachPaneCallback)(TWindow&, void*);
    void          ForEachPane(TForEachPaneCallback callback, void* p);

    // Splitter functions
    //
    int           GetSplitterWidth();
    int           SetSplitterWidth(int newWidth);
    void          SetSplitterCushion(uint cushion);
    bool          MoveSplitter(TWindow* pane, int dist);

    bool          DelObj(TDelete dt);
    void          DeleteOnClose(TDelete dt=TShouldDelete::Delete);

  protected:
    void          SetupWindow();
    void          CleanupWindow();
    void          EvSize(uint sizeType, const TSize& size);
    virtual void  DrawSplitter(TDC& dc, const TRect& splitter);

// ****************  (JAM) 03-16-01
//          The following class members were moved from private to protected
    TSplitterIndicatorList* SplitterIndicatorList;
    bool                     Dragging;
    TSplitterIndicatorMgr    SplitterIndicatorMgr;
    
    void          GetDefLM(TLayoutMetrics& lm);
    bool          HasPane(TWindow* p);
    virtual void  StartSplitterMove(TSplitter* splitter, const TPoint& point);
    virtual int   RemovePanes();
    void          MoveSplitters();
    

  private:
    enum    TTraversalOrder {psPreOrder, psInOrder, psPostOrder};
    typedef int (TPaneSplitter::*TForEachObjectCallback)(TWindow*, void*, void*);

    int                      SplitterWidth;
    uint                    SplitterCushion;
    HCURSOR                  ResizeCursorH;
    HCURSOR                  ResizeCursorV;
    HCURSOR                  ResizeCursorHV;
    bool                     PaneSplitterResizing;
    TDelete                  ShouldDelete;

    void          MouseMoved(const TPoint& point);

    virtual void  EndSplitterMove(const TPoint& point);
    void          SetSplitterMoveCursor(TSplitter* splitter, const TPoint& point);

    void          ForEachObject(TWindow* o,
                                TForEachObjectCallback callback,
                                void* p1, void* p2 = 0,
                                TTraversalOrder order = psPostOrder);
    void          ScreenToClientRect(TRect& rect);
    void          ClientToScreenRect(TRect& rect);
    
    TLayoutWindow* DoRemovePane(TWindow* pane, TDelete dt = TShouldDelete::DefDelete);
    void          FindIntersectingSplitters(const TPoint& point);
    
        void          DestroyPane(TWindow* pane, TDelete dt = TShouldDelete::DefDelete);

    // ForEachObject() callbacks..
    //
    int           DoForEachPane(TWindow* splitter, void* p1, void* p2);
    int           DoFindIntersectingSplitters(TWindow* splitter, void* p1, void* p2);
    int           GetListOfPanesToRemove(TWindow* splitter, void* p1, void* p2);
    int           AdjSplitterWidth(TWindow* splitter, void* p1, void* p2);
    int           DoPaneCount(TWindow* splitter, void* p1, void* p2);
    int           GetPanes(TWindow* splitter, void* p1, void* p2);
    int           GetSplitters(TWindow* splitter, void* p1, void* p2);

  friend class _OWLCLASS TSplitter;
  friend class _OWLCLASS THSplitter;
  friend class _OWLCLASS TVSplitter;

  DECLARE_RESPONSE_TABLE(TPaneSplitter);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>


//----------------------------------------------------------------------------
// Inline implementations
//

//
/// Returns the width of the splitter widget.
//
inline int
TPaneSplitter::GetSplitterWidth()
{
  return SplitterWidth;
}

//
/// Set the cushion for the splitter widget.
//
inline void
TPaneSplitter::SetSplitterCushion(uint cushion)
{
  SplitterCushion = cushion;
}
//
/// Returns true if the object should be deleted.
//
inline bool
TPaneSplitter::DelObj(TDelete dt)
{
  return dt == TShouldDelete::Delete ||
         (dt==TShouldDelete::DefDelete && ShouldDelete==TShouldDelete::Delete);
}

//
//
//
inline void          
TPaneSplitter::DeleteOnClose(TDelete dt)
{
  ShouldDelete = dt;
}

} // OWL namespace



#endif // OWL_PANESPLI_H
