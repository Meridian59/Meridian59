#if !defined(OCFEVX_H)
#define OCFEVX_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif

//
// LRESULT MainWnd_OnOcEvent(HWND hwnd, WPARAM wParam, LPARAM lParam)
//
#define HANDLE_WM_OCEVENT(hwnd, wParam, lParam, fn)\
  (LRESULT)(fn)((hwnd), (wParam), (lParam))

//
// macros for WM_OCEVENT subdispatching
//
#define HANDLE_OCF(hwnd, message, fn) \
  case (message): return (LRESULT)HANDLE_##message((hwnd), (lParam), (fn))


//
// bool Cls_OnOcAppInsMenus(HWND hwnd, TOcMenuDescr * menuDescr)
//
#define HANDLE_OC_APPINSMENUS(hwnd, lParam, fn) \
    (LRESULT)(UINT)(bool)(fn)((hwnd), (TOcMenuDescr *)(lParam))


//
// bool Cls_OnOcAppMenus(HWND hwnd, TOcMenuDescr * menuDescr)
//
#define HANDLE_OC_APPMENUS(hwnd, lParam, fn) \
    (LRESULT)(UINT)(bool)(fn)((hwnd), (TOcMenuDescr *)(lParam))


//
// bool Cls_OnOcAppProcessMsg(HWND hwnd, MSG * msg)
//
#define HANDLE_OC_APPPROCESSMSG(hwnd, lParam, fn) \
    (LRESULT)(UINT)(fn)((hwnd), (MSG *)(lParam))


//
// bool Cls_OnOcAppFrameRect(HWND hwnd, TRect * rect)
//
#define HANDLE_OC_APPFRAMERECT(hwnd, lParam, fn) \
    (LRESULT)(UINT)(fn)((hwnd), (TRect *)(lParam))


//
// bool Cls_OnOcAppBorderSpaceReq(HWND hwnd, TRect * rect)
//
#define HANDLE_OC_APPBORDERSPACEREQ(hwnd, lParam, fn) \
    (LRESULT)(UINT)(fn)((hwnd), (TRect *)(lParam))


//
// bool Cls_OnOcAppBorderSpaceSet(HWND hwnd, TRect * rect)
//
#define HANDLE_OC_APPBORDERSPACESET(hwnd, lParam, fn) \
    (LRESULT)(UINT)(fn)((hwnd), (TRect *)(lParam))


//
// void Cls_OnOcAppStatusText(HWND hwnd, const char * text)
//
#define HANDLE_OC_APPSTATUSTEXT(hwnd, lParam, fn) \
    ((fn)((hwnd), (const char *)(lParam)), 0L)


//
// void Cls_OnOcAppRestoreUI(HWND hwnd)
//
#define HANDLE_OC_APPRESTOREUI(hwnd, lParam, fn) \
    ((fn)((hwnd)), 0L)


//
// void Cls_OnOcAppDialogHelp(HWND hwnd, TOcDialogHelp * dialogHelp)
//
#define HANDLE_OC_APPDIALOGHELP(hwnd, lParam, fn) \
    ((fn)((hwnd), (TOcDialogHelp *)(lParam)), 0L)


//
// TOcView * Cls_OnOcAppCreateComp(HWND hwnd, TDocTemplate * docTemplate)
//
#define HANDLE_OC_APPCREATECOMP(hwnd, lParam, fn) \
    (LRESULT)(fn)((hwnd), (TDocTemplate *)(lParam))


//
// bool Cls_OnOcAppShutDown(HWND hwnd)
//
#define HANDLE_OC_APPSHUTDOWN(hwnd, lParam, fn) \
    (LRESULT)(UINT)(fn)((hwnd))


//
// View events
//

//
// const char * Cls_OnOcViewTitle(HWND hwnd)
//
#define HANDLE_OC_VIEWTITLE(hwnd, lParam, fn) \
    (LRESULT)(fn)((hwnd))


//
// bool Cls_OnOcViewBorderSpaceReq(HWND hwnd, TRect * rect)
//
#define HANDLE_OC_VIEWBORDERSPACEREQ(hwnd, lParam, fn) \
    (LRESULT)(UINT)(fn)((hwnd), (TRECT *)(lParam))


//
// bool Cls_OnOcViewBorderSpaceSet(HWND hwnd, TRect * rect)
//
#define HANDLE_OC_VIEWBORDERSPACESET(hwnd, lParam, fn) \
    (LRESULT)(UINT)(fn)((hwnd), (TRECT *)(lParam))


//
// bool Cls_OnOcViewDrop(HWND hwnd, TOcDragDrop * dragDropInfo)
//
#define HANDLE_OC_VIEWDROP(hwnd, lParam, fn) \
    (LRESULT)(fn)((hwnd), (TOcDragDrop *)(lParam))


//
// bool Cls_OnOcViewDrag(HWND hwnd, TOcDragDrop * dragDropInfo)
//
#define HANDLE_OC_VIEWDRAG(hwnd, lParam, fn) \
  (LRESULT)(UINT)(fn)((hwnd), (TOcDragDrop *)(lParam))


//
// bool Cls_OnOcViewScroll(HWND hwnd, TOcScrollDir * scrollDir)
//
#define HANDLE_OC_VIEWSCROLL(hwnd, lParam, fn) \
  (LRESULT)(UINT)(fn)((hwnd), (TOcScrollDir *)(lParam))


//
// bool Cls_OnOcViewPartInvalid(HWND hwnd, TOcPartChangeInfo * part)
//
#define HANDLE_OC_VIEWPARTINVALID(hwnd, lParam, fn) \
  (LRESULT)(UINT)(fn)((hwnd), (TOcPartChangeInfo *)(lParam))


//
// bool Cls_OnOcViewPaint(HWND hwnd, TOcViewPaint * paintInfo)
//
#define HANDLE_OC_VIEWPAINT(hwnd, lParam, fn) \
  (LRESULT)(UINT)(fn)((hwnd), (TOcViewPaint *)(lParam))


//
// bool Cls_OnOcViewLoadPart(HWND hwnd, TOcSaveLoad * loadInfo)
//
#define HANDLE_OC_VIEWLOADPART(hwnd, lParam, fn) \
  (LRESULT)(UINT)(fn)((hwnd), (TOcSaveLoad *)(lParam))


//
// bool Cls_OnOcViewSavePart(HWND hwnd, TOcSaveLoad * saveInfo)
//
#define HANDLE_OC_VIEWSAVEPART(hwnd, lParam, fn) \
  (LRESULT)(UINT)(fn)((hwnd), (TOcSaveLoad *)(lParam))


//
// bool Cls_OnOcViewClose(HWND hwnd)
//
#define HANDLE_OC_VIEWCLOSE(hwnd, lParam, fn) \
  (LRESULT)(UINT)(fn)((hwnd))

//
// bool Cls_OnOcViewInsMenus(HWND hwnd, TOcMenuDescr * menuDescr)
//
#define HANDLE_OC_VIEWINSMENUS(hwnd, lParam, fn) \
  (LRESULT)(UINT)(fn)((hwnd), (TOcMenuDescr *)(lParam))


//
// bool Cls_OnOcViewShowTools(HWND hwnd, TOcToolBar * toolBarInfo)
//
#define HANDLE_OC_VIEWWSHOWTOOLS(hwnd, lParam, fn) \
  (LRESULT)(UINT)(fn)((hwnd), (TOcToolBar *)(lParam))


//
// bool Cls_OnOcViewGetPalette(HWND hwnd, LOGPALETTE * * paletteInfo)
//
#define HANDLE_OC_VIEWGETPALETTE(hwnd, lParam, fn) \
  (LRESULT)(UINT)(fn)((hwnd), (LOGPALETTE * *)(lParam))


//
// HANDLE Cls_OnOcViewClipData(HWND hwnd, TOcFormat * formatInfo)
//
#define HANDLE_OC_VIEWCLIPDATA(hwnd, lParam, fn) \
  (LRESULT)(fn)((hwnd), (TOcFormat *)(lParam))


//
// bool Cls_OnOcViewPartSize(HWND hwnd, TRect * rect)
//
#define HANDLE_OC_VIEWPARTSIZE(hwnd, lParam, fn) \
  (LRESULT)(UINT)(fn)((hwnd), (TRECT *)(lParam))


//
// bool Cls_OnOcViewOpenDoc(HWND hwnd, const char * text)
//
#define HANDLE_OC_VIEWOPENDOC(hwnd, lParam, fn) \
  (LRESULT)(UINT)(fn)((hwnd), (const char *)(lParam))


//
// bool Cls_OnOcViewAttachWindow(HWND hwnd)
//
#define HANDLE_OC_VIEWATTACHWINDOW(hwnd, lParam, fn) \
  (LRESULT)(UINT)(fn)((hwnd))


//
// bool Cls_OnOcViewSetScale(HWND hwnd, TOcScaleFactor* scaleInfo)
//
#define HANDLE_OC_VIEWSETSCALE(hwnd, lParam, fn) \
  (LRESULT)(UINT)(fn)((hwnd), (TOcScaleFactor*)(lParam))

//
// bool Cls_OnOcViewGetScale(HWND hwnd, TOcScaleFactor* scaleInfo)
//
#define HANDLE_OC_VIEWGETSCALE(hwnd, lParam, fn) \
  (LRESULT)(UINT)(fn)((hwnd), (TOcScaleFactor*)(lParam))


//
// bool Cls_OnOcViewSetSiteRect(HWND hwnd, TRect * rect)
//
#define HANDLE_OC_VIEWSETSITERECT(hwnd, lParam, fn) \
  (LRESULT)(UINT)(fn)((hwnd), (TRect *)(lParam))


//
// bool Cls_OnOcViewGetSiteRect(HWND hwnd, TRect * rect)
//
#define HANDLE_OC_VIEWGETSITERECT(hwnd, lParam, fn) \
  (LRESULT)(UINT)(fn)((hwnd), (TRect *)(lParam))


//
// void Cls_OnOcViewSetTitle(HWND hwnd, const char * title)
//
#define HANDLE_OC_VIEWSETTITLE(hwnd, lParam, fn) \
  ((fn)((hwnd), (const char *)(lParam)), 0)


#endif  // OCFEVENT
