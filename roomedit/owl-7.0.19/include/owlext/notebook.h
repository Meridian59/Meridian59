//----------------------------------------------------------------------------
// OWL Extensions (OWLEXT) Class Library
//    notebook.h - defines TNotebook class
//----------------------------------------------------------------------------
//    Copyright (c) 1995 Borland International
//
//----------------------------------------------------------------------------
#if !defined(OWLEXT_NOTEBOOK_H)
#define OWLEXT_NOTEBOOK_H

#include <owlext/defs.h>

//------------------------------------------------------------------------------
#define NW_HSCROLL  WM_USER+0x2801  // custom message used to talk to parent
       // (params are same as for WM_HSCROLL msg)
#define NW_VSCROLL  WM_USER+0x2802  // custom message used to talk to parent
      // (params are same as for WM_VSCROLL msg)

//---- all 3 defines below use currently selected brush ----
#define FASTRECT(dc,l,t,w,h)  dc.PatBlt(l, t, w, h, PATCOPY)
#define FASTVLINE(dc,l,t,h)  dc.PatBlt(l, t, 1, h, PATCOPY)
#define FASTHLINE(dc,l,t,w)  dc.PatBlt(l, t, w, 1, PATCOPY)
//---- all 3 defines below use currently selected brush ----
#define FASTRECT(dc,l,t,w,h)  dc.PatBlt(l, t, w, h, PATCOPY)
#define FASTVLINE(dc,l,t,h)  dc.PatBlt(l, t, 1, h, PATCOPY)
#define FASTHLINE(dc,l,t,w)  dc.PatBlt(l, t, w, 1, PATCOPY)

#define CSB_NOTDOWN      1  // mouse not currently down
#define CSB_LEFTARROW    2  // mouse went down at tab scroll left button
#define CSB_RIGHTARROW  3  // mouse went down at tab scroll right button
#define CSB_LEFTTHUMB    4  // mouse went down at tab to left of thumb
#define CSB_RIGHTTHUMB  5  // mouse went down at tab to right of thumb
#define CSB_TOPARROW    6  // mouse went down at tab scroll top button
#define CSB_BOTTOMARROW  7  // mouse went down at tab scroll bottom button
#define CSB_TOPTHUMB    8  // mouse went down at tab to top of thumb
#define CSB_BOTTOMTHUMB  9  // mouse went down at tab to bottom of thumb

#define CSB_FORCEALL    10  // force entire scrollbar redraw
#define CSB_THUMB        11  // mouse went down at tab scroll thumb button
#define CSB_THUMBFOCUS  12  // draw solid at origval and focus rect at new

// repaint param values to SetValue()
#define SV_NOREPAINT     0  // don't update thumb display
#define SV_REPAINT       1  // repaint thumb position
#define SV_THUMBTRACK    2  // paint thumb position in thumb tracking style

namespace OwlExt {

// Generic definitions/compiler options (eg. alignment) preceding the
// definition of classes
//
#include <owl/preclass.h>

class OWLEXTCLASS TCustomScroll {
  public:
    TCustomScroll(owl::TWindow *Parent, int x, int y, int width, int height,
              int idnum = 0, bool frame3d = false, bool arrowline = true,
             bool thumbmove = false, int repeatdelay = 250);
    ~TCustomScroll();

    void ShowScrollBar(bool show);
    void SysColorChange();

    int   GetScrollPos();
    void   GetScrollRange(int& low, int& high);
    void   ForcePaint();

  protected:
    void    AutoScrollMouse();
    virtual void AutoScrollAction() {;}
    virtual void MyPaint(int newstate) {;}

  protected:
    //---- non measurement items --------
    owl::TWindow* Parent;    // parent window of this sb
    int trange;      // cnt-1 of # of thumb positions in mouse track
    int vrange;      // cnt-1 of # of values in low...high range
    int mousedown;    // part of sb that mouse went down on
    int newloc;      // new value for mousedown
    int displaystate;    // state of scrollbar as currently displayed
    int lowval;      // lowest value for scrollbar
    int highval;    // highest value for scrollbar
    int curval;      // current value (position) of scrollbar
    int origval;    // value before mouse capture
    int scrollmsg;    // WM_HSCROLL or NW_HSCROLL
    int repeatdelay;    // # of milliseconds to delay mouse repeat
    long mylongid;    // my idnum as HIWORD() of a long value
    bool frame3d;    // TRUE if 3d frame around sb requested
    bool arrowline;    // TRUE if line of arrow needed on scroll button
    bool thumbmove;    // TRUE if tracking thumb moved (vs. focus rect)
    bool showit;    // TRUE if should show & be enabled
    //----- various measurements (window-relative coordinates) ------
    owl::TRect  framerect;    // rect for 3d frame
    owl::TRect sbrect;    // rect for actual scrollbar
    int   sbheight;    // height of actual scrollbar
    int   sbwidth;    // width of actual scrollbar
    int   sqsize;      // size of arrow & thumb squares
    int   barsize;    // thickness of arrow inner bars (1-2 pixels)

    //------------------------------------------
    owl::TBrush*  framebrush;
    owl::TBrush*  trackbrush;
    owl::TBrush*  hilitebrush;
    owl::TBrush*  shadowbrush;
    owl::TBrush*  facebrush;

  protected:
    void   BuildBrushes();
    void   DeleteBrushes();

};
//
//
//
class OWLEXTCLASS THScroll: public TCustomScroll {
  public:
    THScroll(owl::TWindow *Parent, int x, int y, int width, int height,
              int idnum = 0, int Lowval = 0, int Highval = 100,
             int initval = 0, bool frame3d = false, bool Arrowline = true,
             bool Thumbmove = false, int Repeatdelay = 250);
    ~THScroll();

    int   SetScrollPos(int newval, int repaint);
    void   SetScrollRange(int lowval, int highval, bool redraw);

    void ChangeRect(const owl::TRect& newrect, bool repaint = true);
    //----- use below 3 funcs to pass along mouse events --------
    bool MyLButtonDown(owl::uint hitTest, owl::TPoint& winpt);
    bool MyMouseMove(owl::uint hitTest, owl::TPoint& winpt);
    bool MyLButtonUp(owl::uint hitTest, owl::TPoint& winpt);

  protected:
    int   trackwidth;    // height of thumb track (overlap 2 black lines)
    int   trackxoff;    // yoffset where thumb track starts
    int   thumbxoff;    // where thumb is currently sitting
    int   thumbgrabxoff;    // offset from start of thumb to mouse pt
    int   lastfocusxoff;    // last yoff of thumb focus rect
  protected:
    //------------------------------------------
    int   GetMouseLoc(owl::TPoint& winpt);
    void   AutoScrollAction();
    void   MyPaint(int newstate);
    void  DrawFrames(owl::TDC& mydc);
    void   DrawLeftArrow(owl::TDC& mydc, bool isdown);
    void   DrawRightArrow(owl::TDC& mydc, bool isdown);
    void   DrawLeftTrack(owl::TDC& mydc, bool isdown);
    void   DrawRightTrack(owl::TDC& mydc, bool isdown);
    void   DrawThumb(owl::TDC& mydc, bool drawfocus, bool erasefocus);
    void   DrawArrowButton(owl::TDC& mydc, owl::TRect& brect, bool isdown, bool left);
    void   DrawShadeFrame(owl::TDC& mydc, owl::TRect& myrect, bool forthumb);
    void   DrawDownFrame(owl::TDC& mydc, owl::TRect& myrect);
};
//----------------------------------------------------------------------------
class OWLEXTCLASS TVScroll : public TCustomScroll{
  public:
    TVScroll(owl::TWindow *Parent, int x, int y, int width, int height,
               int idnum = 0, int Lowval = 0, int Highval = 100, int initval = 0,
              bool frame3d = false, bool Arrowline = true, bool Thumbmove = false,
              int Repeatdelay = 250);
    ~TVScroll();

    int   SetScrollPos(int newval, int repaint);
    void  SetScrollRange(int lowval, int highval, bool redraw);

    void   ChangeRect(const owl::TRect& rect, bool repaint = true);

    //----- use below 3 funcs to pass along mouse events --------
    bool MyLButtonDown(owl::uint hitTest, owl::TPoint& winpt);
    bool MyMouseMove(owl::uint hitTest, owl::TPoint& winpt);
    bool MyLButtonUp(owl::uint hitTest, owl::TPoint& winpt);

  protected:
    int   trackheight;    // height of thumb track (overlap 2 black lines)
    int   trackyoff;    // yoffset where thumb track starts
    int   thumbyoff;    // where thumb is currently sitting
    int   thumbgrabyoff;    // offset from start of thumb to mouse pt
    int   lastfocusyoff;    // last yoff of thumb focus rect
  protected:
    //------------------------------------------
    void BuildBrushes();
    void DeleteBrushes();
    int  GetMouseLoc(owl::TPoint& winpt);
    virtual void AutoScrollAction();
    void MyPaint(int newstate);
    void DrawFrames(owl::TDC& dc);
    void DrawTopArrow(owl::TDC& dc, bool isdown);
    void DrawBottomArrow(owl::TDC& dc, bool isdown);
    void DrawTopTrack(owl::TDC& dc, bool isdown);
    void DrawBottomTrack(owl::TDC& dc, bool isdown);
    void DrawThumb(owl::TDC& dc, bool drawfocus, bool erasefocus);
    void DrawArrowButton(owl::TDC& dc, owl::TRect& rect, bool isdown, bool top);
    void DrawShadeFrame(owl::TDC& dc, owl::TRect& rect, bool forthumb);
    void DrawDownFrame(owl::TDC& dc, owl::TRect& rect);
};
////////////////////////////////////////////////////////////////////////////////
#define MAXTABNAMELEN   30
#define MAXTABSSHOWN    99

#define NB_TABCHANGED   20099

#define ID_TABSB        3001
#define ID_WINHSB       0
#define ID_WINVSB       0

#define NBC_PAINTALL    0
#define NBC_PAINTTABS   1

#define TABSAFETY       8               // pixels after last tab

// values for tabloc
#define LOC_TOP         0
#define LOC_RIGHT       1
#define LOC_BOTTOM      2
#define LOC_LEFT        3

//----------------------------------------------------------------------------
class OWLEXTCLASS TNotebook : public virtual owl::TWindow {
  public:
    //--- redefined functions ---
    TNotebook(int tabloc = 0);
    ~TNotebook();
    void SetTabCnt(int tabcnt, int firsttab = 0, int activetab = 0);

    int   GetScrollPos(int bar);
    void   GetScrollRange(int bar, int& low, int& high);
    int   SetScrollPos(int bar, int pos, bool redraw = true);
    void   SetScrollRange(int  bar, int  minPos, int  maxPos, bool redraw = true);

  protected:
    owl::TRect nbcrect;              // current rect for notebook control area
    owl::TRect vsbrect;              // rect for vertical scroll bar
    owl::TRect hsbrect;              // rect for horizontal scroll bar
    owl::TRect tsbrect;              // rect for tabs scroll bar
    owl::TRect tabsrect;             // rect for set of tabs
    owl::TRect clientrect;           // size of client area after our controls
    owl::TRect clientprev;           // to detect window size changes
    owl::TSize Frame;                // actual window frame sizes
    POINT tabpoints[MAXTABSSHOWN][5];
    int tabcnt;                 // # of tabs to move thru
    int firsttab;               // # of first tab displayed (0-rel)
    int activetab;              // # of active tab (0-rel)
    int nbcheight;              // height of notebook control area
    int textextra;
    int fillmode;               // polygon fill mode used
    int rgncnt;                 // number of tabs showing & regions allocated
    int toffs[MAXTABSSHOWN];    // toffset (x or y) of each tab drawing
    int defer_ncpaint;          // if nonzero, don't paint nonclient area
    int tabloc;                 // position of tabs on client rectangle
    bool tabhorz;               // TRUE if tabloc is LOC_TOP or LOC_BOTTOM
    bool wanthscroll;           // TRUE if WS_HSCROLL specified in style bits
    bool wantvscroll;           // TRUE if WS_VSCROLL specified in style bits
    bool hscrollon;             // TRUE if horz scrollbar being shown now
    bool vscrollon;             // TRUE if vert scrollbar being shown now
    bool erasetabs;             // TRUE if tabs should be erased as drawn
    owl::TPen* framepen;             // for drawing gray stuff over and over
    owl::TPen* shadowpen;            // for drawing gray stuff over and over
    owl::TFont* tabfont;             // font for tabs
    owl::TBrush* bgbrush;            // white/ltgray dither pattern brush
    owl::TBrush* framebrush;
    owl::TBrush* hilitebrush;
    owl::TBrush* shadowbrush;
    owl::TBrush* facebrush;
    owl::TBrush* trackbrush;
    owl::TBrush* tabbrush;           // bg color of current tab
    owl::TBrush* default_tabbrush;   // bg color of most tabs
    THScroll* tabhscroll;       // horizontal scrollbar for tabs
    TVScroll* tabvscroll;       // vertical scrollbar for tabs
    THScroll* winhscroll;       // horizontal scrollbar for window
    TVScroll* winvscroll;       // vertical scrollbar for window
    //------------------------------------------
    bool EvNCCreate(CREATESTRUCT & createStruct);
    void EvSize(owl::uint sizeType, const owl::TSize& newsize);
    void EvGetMinMaxInfo(MINMAXINFO & minmaxinfo);
    owl::uint EvNCCalcSize(bool calcValidRects, NCCALCSIZE_PARAMS & calcSize);
    UINT EvNCHitTest(const owl::TPoint& screenPt);
    void EvNCPaint(HRGN rgn);
    void EvNCLButtonDown(owl::uint hitTest, const owl::TPoint& screenPt);
    void EvNCLButtonDblClk(owl::uint hitTest, const owl::TPoint& screenPt);
    void EvMouseMove(owl::uint hitTest, const owl::TPoint& clientpt);
    void EvLButtonUp(owl::uint hitTest, const owl::TPoint& clientpt);
    void EvSysColorChange();
    //------------------------------------------
    void virtual GetTabName(int tabnum, LPTSTR tabname);
    void virtual ActivateTab(int tabnum);
    //------------------------------------------
    void BuildPaintObjects();
    void DeletePaintObjects();
    void ComputeSizes();
    void CalcTabRects();
    void ForceResize(bool repaint);
    void CreateScrollBars();
    bool MyLButtonDown(owl::uint hitTest, owl::TPoint& screenPt);
    LRESULT NWHscroll(WPARAM wparam, LPARAM lparam);
    void ScrollTabs(int delta);
    void PaintNBC(int paintcontrol);
    void DrawShadeFrame(owl::TDC& dc, owl::TRect& rect);
    void DrawDownFrame(owl::TDC& dc, owl::TRect& rect);
    void DrawTabs(owl::TDC& dc);
    int DrawHorzTab(owl::TDC& dc, int tabnum, int rgnnum, int xoff);
    int DrawVertTab(owl::TDC& dc, int tabnum, int rgnnum, int xoff);
    void EraseLastHorz(owl::TDC& dc, int toff);
    void EraseLastVert(owl::TDC& dc, int toff);
    void DrawBackground(owl::TDC& dc, owl::TRect& rect, int paintcontrol);
    void DrawTabLines(owl::TDC& dc, owl::TRect& rect, bool shadowline);
    void BuildBgBrush();
    //------------------------------------------
    DECLARE_RESPONSE_TABLE(TNotebook);
};

// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

} // OwlExt namespace

#endif // OWLEXT_NOTEBOOK_H
