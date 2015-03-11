//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of TCommCtrlDll, a wrapper for the common control dll
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/defs.h>
#include <owl/commctrl.h>

namespace owl {

// Common Controls diagnostic group
OWL_DIAGINFO;

DIAG_DEFINE_GROUP_INIT(OWL_INI, OwlCommCtrl, 1, 0);


  static tchar CommCtrlDllName[] = _T("COMCTL32.DLL");

  //static const char CreateToolbarStr[]        = "CreateToolbar", ?????????
  static const char ImageList_ReadStr[]          = "ImageList_Read";
  static const char ImageList_WriteStr[]        = "ImageList_Write";
  static const char ImageList_AddMaskedStr[]    = "ImageList_AddMasked";
  static const char ImageList_BeginDragStr[]    = "ImageList_BeginDrag";
  static const char ImageList_CopyStr[]          = "ImageList_Copy";
  static const char ImageList_DragEnterStr[]    = "ImageList_DragEnter"; 
  static const char ImageList_DragLeaveStr[]    = "ImageList_DragLeave"; 
  static const char ImageList_DragMoveStr[]      = "ImageList_DragMove"; 
  static const char ImageList_DrawExStr[]        = "ImageList_DrawEx"; 
  static const char ImageList_DrawIndirectStr[]  = "ImageList_DrawIndirect";
  static const char ImageList_DuplicateStr[]    = "ImageList_Duplicate";
  static const char ImageList_GetDragImageStr[]  = "ImageList_GetDragImage"; 
  static const char ImageList_GetIconStr[]      = "ImageList_GetIcon"; 
  static const char ImageList_GetIconSizeStr[]  = "ImageList_GetIconSize"; 
  static const char ImageList_GetImageInfoStr[]  = "ImageList_GetImageInfo"; 
  static const char ImageList_MergeStr[]        = "ImageList_Merge"; 
  static const char ImageList_RemoveStr[]        = "ImageList_Remove"; 
  static const char ImageList_ReplaceStr[]      = "ImageList_Replace"; 
  static const char ImageList_SetDragCursorImageStr[] = "ImageList_SetDragCursorImage"; 
  static const char ImageList_SetImageCountStr[]= "ImageList_SetImageCount"; 
  static const char ImageList_SetIconSizeStr[]  = "ImageList_SetIconSize"; 
  static const char ImageList_DragShowNolockStr[]= "ImageList_DragShowNolock"; 
  static const char ImageList_EndDragStr[]      = "ImageList_EndDrag"; 
  
  static const char InitializeFlatSBStr[]        = "InitializeFlatSB";
  static const char UninitializeFlatSBStr[]      = "UninitializeFlatSB";
  static const char FlatSB_EnableScrollBarStr[]  = "FlatSB_EnableScrollBar";
  static const char FlatSB_ShowScrollBarStr[]    = "FlatSB_ShowScrollBar";
  static const char FlatSB_GetScrollInfoStr[]    = "FlatSB_GetScrollInfo";
  static const char FlatSB_SetScrollInfoStr[]    = "FlatSB_SetScrollInfo";
  static const char FlatSB_GetScrollPosStr[]    = "FlatSB_GetScrollPos";
  static const char FlatSB_SetScrollPosStr[]    = "FlatSB_SetScrollPos";
  static const char FlatSB_GetScrollPropStr[]    = "FlatSB_GetScrollProp";
  static const char FlatSB_SetScrollPropStr[]    = "FlatSB_SetScrollProp";
  static const char FlatSB_GetScrollRangeStr[]  = "FlatSB_GetScrollRange";
  static const char FlatSB_SetScrollRangeStr[]  = "FlatSB_SetScrollRange";
#if defined(UNICODE)
  static const char CreatePropertySheetPageStr[]= "CreatePropertySheetPageW"; 
  static const char CreateStatusWindowStr[]      = "CreateStatusWindowW"; 
  static const char DrawStatusTextStr[]          = "DrawStatusTextW";
  static const char ImageList_LoadImageStr[]    = "ImageList_LoadImageW"; 
  static const char PropertySheetStr[]          = "PropertySheetW";
#else
  static const char CreatePropertySheetPageStr[]= "CreatePropertySheetPageA"; 
  static const char CreateStatusWindowStr[]      = "CreateStatusWindowA"; 
  static const char DrawStatusTextStr[]          = "DrawStatusTextA";
  static const char ImageList_LoadImageStr[]    = "ImageList_LoadImageA"; 
  static const char PropertySheetStr[]          = "PropertySheetA";
#endif
static const char DllGetVersionStr[]          = "DllGetVersion";
static const char InitCommonControlsStr[]      = "InitCommonControls";
static const char InitCommonControlsExStr[]    = "InitCommonControlsEx";
static const char CreateMappedBitmapStr[]      = "CreateMappedBitmap";
static const char CreateToolbarExStr[]        = "CreateToolbarEx";
static const char CreateUpDownControlStr[]    = "CreateUpDownControl";
static const char DestroyPropertySheetPageStr[]= "DestroyPropertySheetPage";
static const char DrawInsertStr[]              = "DrawInsert";
static const char GetEffectiveClientRectStr[]  = "GetEffectiveClientRect";
static const char ImageList_AddStr[]          = "ImageList_Add";
static const char ImageList_CreateStr[]        = "ImageList_Create";
static const char ImageList_DestroyStr[]      = "ImageList_Destroy";
static const char ImageList_DrawStr[]          = "ImageList_Draw";
static const char ImageList_GetBkColorStr[]    = "ImageList_GetBkColor";
static const char ImageList_GetImageCountStr[]  = "ImageList_GetImageCount";
static const char ImageList_ReplaceIconStr[]  = "ImageList_ReplaceIcon";
static const char ImageList_SetBkColorStr[]    = "ImageList_SetBkColor";
static const char ImageList_SetOverlayImageStr[]="ImageList_SetOverlayImage";
static const char LBItemFromPtStr[]            = "LBItemFromPt";
static const char MakeDragListStr[]            = "MakeDragList";
static const char MenuHelpStr[]                = "MenuHelp";
static const char ShowHideMenuCtlStr[]        = "ShowHideMenuCtl";

////////////////////////////////////////////////////////////////////////////////////////
//
//
//

TCommCtrlDll::TCommCtrlDll()
:
  TModule(CommCtrlDllName,true,true, false), // shouldLoad, mustLoad and !addToList
  Version(0)
{
  // Do the initialization now that the Dll is loaded
  //
  InitCommonControls();

  TDllVersionInfo info;
  GetCtrlVersion(info);
  Version = MkUint32((uint16)info.dwMinorVersion, (uint16)info.dwMajorVersion);
}

//
void 
TCommCtrlDll::InitCommonControls()
{
  static TModuleProcV0 initCommonControls(*this, InitCommonControlsStr);
  initCommonControls();
}
//
bool
TCommCtrlDll::InitCommonControlsEx(uint32 type)
{
  typedef int ( WINAPI* TProc)(LPINITCOMMONCONTROLSEX);
  static TProc Proc = (TProc)GetModule().GetProcAddress(InitCommonControlsExStr);
  if(!Proc)
    return false;

  INITCOMMONCONTROLSEX icc;
  icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
  icc.dwICC  = type;

  return Proc(&icc) != 0;
}
//
void
TCommCtrlDll::CheckCommonControl(uint32 type)
{
  if(!InitCommonControlsEx(type))
    TXCommCtrl::Raise();
}
//
bool
TCommCtrlDll::GetCtrlVersion(TDllVersionInfo& info)
{
  static DLLGETVERSIONPROC Proc = (DLLGETVERSIONPROC)GetProcAddress(DllGetVersionStr);
  if(!Proc)
    return false;

  bool bOk;
  if(!Proc){
    info.dwMajorVersion = 4;
    info.dwMinorVersion  = 0;
    bOk = true;
  }
  else
    bOk = Proc(&info) == S_OK;
  return bOk;
}
// Version 4.71
bool
TCommCtrlDll::InitializeFlatSB(HWND hWnd)
{
  static TModuleProc1<BOOL,HWND> 
    initializeFlatSB(GetModule(), InitializeFlatSBStr);
  return initializeFlatSB(hWnd);
}
// Version 4.71
HRESULT
TCommCtrlDll::UninitializeFlatSB(HWND hWnd)
{
  static TModuleProc1<HRESULT,HWND> 
    uninitializeFlatSB(GetModule(), UninitializeFlatSBStr);
  return uninitializeFlatSB(hWnd);
}
// Version 4.71
bool
TCommCtrlDll::FlatSB_EnableScrollBar(HWND hWnd, int index, uint arrow)
{
  static TModuleProc3<BOOL,HWND,int,uint> 
    flatSB_EnableScrollBar(GetModule(), FlatSB_EnableScrollBarStr);
  return flatSB_EnableScrollBar(hWnd, index, arrow);
}
// Version 4.71
bool
TCommCtrlDll::FlatSB_ShowScrollBar(HWND hWnd, int code, bool show)
{
  static TModuleProc3<BOOL,HWND,int,BOOL> 
    flatSB_ShowScrollBar(GetModule(), FlatSB_ShowScrollBarStr);
  return flatSB_ShowScrollBar(hWnd, code, (BOOL)show);
}
// Version 4.71
bool
TCommCtrlDll::FlatSB_GetScrollInfo(HWND hWnd, int code, LPSCROLLINFO fsi)
{
  static TModuleProc3<BOOL,HWND,int,LPSCROLLINFO> 
    flatSB_GetScrollInfo(GetModule(), FlatSB_GetScrollInfoStr);
  return flatSB_GetScrollInfo(hWnd, code, fsi);
}
// Version 4.71
int
TCommCtrlDll::FlatSB_SetScrollInfo(HWND hWnd, int code, LPSCROLLINFO fsi, bool redraw)
{
  static TModuleProc4<int,HWND,int,LPSCROLLINFO,BOOL> 
    flatSB_SetScrollInfo(GetModule(), FlatSB_SetScrollInfoStr);
  return flatSB_SetScrollInfo(hWnd, code, fsi, (BOOL)redraw);
}
// Version 4.71
int
TCommCtrlDll::FlatSB_GetScrollPos(HWND hWnd, int bar)
{
  static TModuleProc2<int,HWND,int> 
    flatSB_GetScrollPos(GetModule(), FlatSB_GetScrollPosStr);
  return flatSB_GetScrollPos(hWnd, bar);
}
// Version 4.71
int
TCommCtrlDll::FlatSB_SetScrollPos(HWND hWnd, int bar, int pos, bool redraw)
{
  static TModuleProc4<int,HWND,int,int,BOOL> 
    flatSB_SetScrollPos(GetModule(), FlatSB_SetScrollPosStr);
  return flatSB_SetScrollPos(hWnd, bar, pos, (BOOL)redraw);
}
// Version 4.71
bool
TCommCtrlDll::FlatSB_GetScrollProp(HWND hWnd, uint index, int* value)
{
  static TModuleProc3<BOOL,HWND,uint,int*> 
    flatSB_GetScrollProp(GetModule(), FlatSB_GetScrollPropStr);
  return flatSB_GetScrollProp(hWnd, index, value);
}
// Version 4.71
bool
TCommCtrlDll::FlatSB_SetScrollProp(HWND hWnd, uint index, int value, bool redraw)
{
  static TModuleProc4<BOOL,HWND,uint,int,BOOL> 
    flatSB_SetScrollProp(GetModule(), FlatSB_SetScrollPropStr);
  return flatSB_SetScrollProp(hWnd, index, value,(BOOL)redraw);
}
// Version 4.71
bool
TCommCtrlDll::FlatSB_GetScrollRange(HWND hWnd, int bar, int* minpos, int* maxpos)
{
  static TModuleProc4<BOOL,HWND,int,int*,int*> 
    flatSB_GetScrollRange(GetModule(), FlatSB_GetScrollRangeStr);
  return flatSB_GetScrollRange(hWnd,bar,minpos,maxpos);
}
// Version 4.71
int
TCommCtrlDll::FlatSB_SetScrollRange(HWND hWnd, int bar, int minpos, int maxpos, bool redraw)
{
  static TModuleProc5<int,HWND,int,int,int,BOOL> 
    flatSB_SetScrollRange(GetModule(), FlatSB_SetScrollRangeStr);
  return flatSB_SetScrollRange(hWnd,bar,minpos,maxpos,(BOOL)redraw);
}
// Version 4.70
bool
TCommCtrlDll::ImageList_Copy(HIMAGELIST himlDst,int iDst, HIMAGELIST himlSrc,
                            int iSrc, uint uFlags)
{
  static TModuleProc5<BOOL,HIMAGELIST,int,HIMAGELIST,int,uint> 
    imageList_Copy(GetModule(), ImageList_CopyStr);
  return imageList_Copy(himlDst,iDst, himlSrc, iSrc, uFlags);
}
// Version 4.70
bool
TCommCtrlDll::ImageList_DrawIndirect(IMAGELISTDRAWPARAMS* pimldp)
{
  static TModuleProc1<BOOL,IMAGELISTDRAWPARAMS*> 
    imageList_DrawIndirect(GetModule(), ImageList_DrawIndirectStr);
  return imageList_DrawIndirect(pimldp);
}
// Version 4.71
HIMAGELIST
TCommCtrlDll::ImageList_Duplicate(HIMAGELIST himl)
{
  static TModuleProc1<HIMAGELIST,HIMAGELIST> 
    imageList_Duplicate(GetModule(), ImageList_DuplicateStr);
  return imageList_Duplicate(himl);
}
// Version 4.70
bool
TCommCtrlDll::ImageList_SetImageCount(HIMAGELIST himl, uint uNewCount)
{
  static TModuleProc2<BOOL,HIMAGELIST,uint> 
    imageList_SetImageCount(GetModule(), ImageList_SetImageCountStr);
  return imageList_SetImageCount(himl, uNewCount);
}
//
HIMAGELIST 
TCommCtrlDll::ImageList_Read( LPSTREAM pstm)
{
  static TModuleProc1<HIMAGELIST,LPSTREAM> 
    imageList_Read(GetModule(), ImageList_ReadStr);
  return imageList_Read(pstm);
}
//
bool  
TCommCtrlDll::ImageList_Write(HIMAGELIST himl, LPSTREAM pstm)
{
  static TModuleProc2<BOOL,HIMAGELIST,LPSTREAM> 
    imageList_Write(GetModule(), ImageList_WriteStr);
  return imageList_Write(himl, pstm);
}
//
int 
TCommCtrlDll::ImageList_AddMasked(HIMAGELIST himl,HBITMAP hbmp,COLORREF clr)
{
  static TModuleProc3<int,HIMAGELIST,HBITMAP,COLORREF> 
    imageList_AddMasked(GetModule(), ImageList_AddMaskedStr);
  return imageList_AddMasked(himl, hbmp, clr);
}
//
bool 
TCommCtrlDll::ImageList_BeginDrag(HIMAGELIST himl,int track,int dxHot, int dyHot)
{
  static TModuleProc4<BOOL,HIMAGELIST,int,int,int> 
    imageList_BeginDrag(GetModule(), ImageList_BeginDragStr);
  return imageList_BeginDrag(himl,track,dxHot, dyHot);
}
//
void 
TCommCtrlDll::ImageList_EndDrag()
{
  static TModuleProcV0 imageList_EndDrag(GetModule(), ImageList_EndDragStr);
  imageList_EndDrag();
}
//
bool 
TCommCtrlDll::ImageList_DragEnter(HWND hLock, int x, int y)
{
  static TModuleProc3<BOOL,HWND,int,int> 
    imageList_DragEnter(GetModule(), ImageList_DragEnterStr);
  return imageList_DragEnter(hLock,x, y);
}
//
bool 
TCommCtrlDll::ImageList_DragLeave(HWND hLock)
{
  static TModuleProc1<BOOL,HWND> 
    imageList_DragLeave(GetModule(), ImageList_DragLeaveStr);
  return imageList_DragLeave(hLock);
}
//
bool 
TCommCtrlDll::ImageList_DragMove(int x, int y)
{
  static TModuleProc2<BOOL,int,int> 
    imageList_DragMove(GetModule(), ImageList_DragMoveStr);
  return imageList_DragMove(x,y);
}
//
bool 
TCommCtrlDll::ImageList_SetDragCursorImage(HIMAGELIST himl, int drag, int dxHot, int dyHot)
{
  static TModuleProc4<BOOL,HIMAGELIST,int,int,int> 
    imageList_SetDragCursorImage(GetModule(), ImageList_SetDragCursorImageStr);
  return imageList_SetDragCursorImage(himl, drag, dxHot, dyHot);
}
//
bool 
TCommCtrlDll::ImageList_DragShowNolock(bool show)
{
  static TModuleProc1<BOOL,BOOL> 
    imageList_DragShowNolock(GetModule(), ImageList_DragShowNolockStr);
  return imageList_DragShowNolock(show);
}
//
HIMAGELIST 
TCommCtrlDll::ImageList_GetDragImage(POINT* ppt, POINT* pptHotSpot)
{
  static TModuleProc2<HIMAGELIST,POINT *,POINT *> 
    imageList_GetDragImage(GetModule(), ImageList_GetDragImageStr);
  return imageList_GetDragImage(ppt, pptHotSpot);
}
//
bool  
TCommCtrlDll::ImageList_DrawEx(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, 
                               int dx, int dy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle)
{
  static TModuleProc10<BOOL,HIMAGELIST,int,HDC,int,int,int,int,COLORREF,COLORREF,UINT> 
    imageList_DrawEx(GetModule(), ImageList_DrawExStr);
  return imageList_DrawEx(himl, i, hdcDst, x, y, dx, dy, rgbBk, rgbFg, fStyle);
}
//
bool  
TCommCtrlDll::ImageList_GetIconSize(HIMAGELIST himl, int  *cx, int  *cy)
{
  static TModuleProc3<BOOL,HIMAGELIST,int *,int *> 
    imageList_GetIconSize(GetModule(), ImageList_GetIconSizeStr);
  return imageList_GetIconSize(himl, cx, cy);
}
//
HICON 
TCommCtrlDll::ImageList_GetIcon(HIMAGELIST himl, int i, uint flags)
{
  static TModuleProc3<HICON,HIMAGELIST,int,UINT> 
    imageList_GetIcon(GetModule(), ImageList_GetIconStr);
  return imageList_GetIcon(himl, i, flags);
}
//
bool  
TCommCtrlDll::ImageList_GetImageInfo(HIMAGELIST himl, int i, IMAGEINFO * pImageInfo)
{
  static TModuleProc3<BOOL,HIMAGELIST,int,IMAGEINFO *> 
    imageList_GetImageInfo(GetModule(), ImageList_GetImageInfoStr);
  return imageList_GetImageInfo(himl, i, pImageInfo);
}
//
bool
TCommCtrlDll::ImageList_Replace(HIMAGELIST himl, int i, HBITMAP hbmImage, HBITMAP hbmMask)
{
  static TModuleProc4<BOOL,HIMAGELIST,int,HBITMAP,HBITMAP> 
    imageList_Replace(GetModule(), ImageList_ReplaceStr);
  return imageList_Replace(himl, i, hbmImage, hbmMask);
}
//
bool  
TCommCtrlDll::ImageList_Remove(HIMAGELIST himl, int i)
{
  static TModuleProc2<BOOL,HIMAGELIST,int> 
    imageList_Remove(GetModule(), ImageList_RemoveStr);
  return imageList_Remove(himl, i);
}
//
HIMAGELIST 
TCommCtrlDll::ImageList_LoadImage(HINSTANCE hi, LPCTSTR lpbmp, int cx,
                                  int cGrow, COLORREF crMask, uint uType, uint uFlags)
{
    static TModuleProc7<HIMAGELIST,HINSTANCE,LPCTSTR,int,int,COLORREF,UINT,UINT>
        imageList_LoadImage(GetModule(), ImageList_LoadImageStr);
  return imageList_LoadImage(hi, lpbmp,cx, cGrow,crMask, uType, uFlags);
}//
HIMAGELIST 
TCommCtrlDll::ImageList_Merge(HIMAGELIST himl1, int i1, HIMAGELIST himl2, int i2, int dx, int dy)
{
  static TModuleProc6<HIMAGELIST,HIMAGELIST,int,HIMAGELIST,int,int,int> 
    imageList_Merge(GetModule(), ImageList_MergeStr);
  return imageList_Merge(himl1, i1, himl2, i2, dx, dy);
}
//
bool  
TCommCtrlDll::ImageList_SetIconSize(HIMAGELIST himl, int cx, int cy)
{
  static TModuleProc3<BOOL,HIMAGELIST,int,int> 
    imageList_SetIconSize(GetModule(), ImageList_SetIconSizeStr);
  return imageList_SetIconSize(himl, cx, cy);
}

HBITMAP 
TCommCtrlDll::CreateMappedBitmap(HINSTANCE hInstance, int idBitmap,uint wFlags, LPCOLORMAP lpColorMap, int iNumMaps)
{
  static TModuleProc5<HBITMAP,HINSTANCE,int,UINT,LPCOLORMAP,int> 
    createMappedBitmap(GetModule(), CreateMappedBitmapStr);
  return createMappedBitmap(hInstance,idBitmap,wFlags,lpColorMap,iNumMaps);
}
//
HPROPSHEETPAGE
TCommCtrlDll::CreatePropertySheetPage(LPCPROPSHEETPAGE lpPage)
{
    static TModuleProc1<HPROPSHEETPAGE,LPCPROPSHEETPAGE>
        createPropertySheetPage(GetModule(), CreatePropertySheetPageStr);
  return createPropertySheetPage(lpPage);
}
//
bool
TCommCtrlDll::DestroyPropertySheetPage(HPROPSHEETPAGE hPage)
{
  static TModuleProc1<BOOL,HPROPSHEETPAGE> 
    destroyPropertySheetPage(GetModule(), DestroyPropertySheetPageStr);
  return destroyPropertySheetPage(hPage);
}
//
HWND
TCommCtrlDll::CreateStatusWindow(long style, LPCTSTR lpszText, HWND hwndParent, uint wID)
{
    static TModuleProc4<HWND,LONG,LPCTSTR,HWND,UINT>
        createStatusWindow(GetModule(), CreateStatusWindowStr);
  return createStatusWindow(style,lpszText,hwndParent,wID);
}//
HWND 
TCommCtrlDll::CreateToolbarEx(HWND hwnd, DWORD ws, UINT wID, int nBitmaps,HINSTANCE hBMInst, 
                              UINT wBMID, LPCTBBUTTON lpButtons, int iNumButtons, int dxButton, 
                              int dyButton,int dxBitmap, int dyBitmap, UINT uStructSize)
{
  static TModuleProc13<HWND,HWND,DWORD,UINT,int,HINSTANCE,UINT,LPCTBBUTTON,int,int, int,int,int,UINT> 
    createToolbarEx(GetModule(), CreateToolbarExStr);
  return createToolbarEx(hwnd,ws,wID,nBitmaps,hBMInst,wBMID,lpButtons,
                         iNumButtons,dxButton,dyButton,dxBitmap,dyBitmap,uStructSize);
}
//
HWND 
TCommCtrlDll::CreateUpDownControl(DWORD dwStyle, int x, int y, int cx, int cy,HWND hParent, int nID, 
                                  HINSTANCE hInst, HWND hBuddy,int nUpper, int nLower, int nPos)
{
  static TModuleProc12<HWND,DWORD,int,int,int,int,HWND,int,HINSTANCE,HWND,int,int,int> 
    createUpDownControl(GetModule(), CreateUpDownControlStr);
  return createUpDownControl(dwStyle,x,y,cx,cy,hParent,nID,hInst,hBuddy,nUpper,nLower,nPos);
}
//
void 
TCommCtrlDll::DrawInsert(HWND handParent, HWND hLB, int nItem)
{
  static TModuleProcV3<HWND,HWND,int> 
    drawInsert(GetModule(), DrawInsertStr);
  drawInsert(handParent, hLB, nItem);
}
//
void
TCommCtrlDll::DrawStatusText(HDC hDC, LPRECT lprc, LPCTSTR pszText, uint uFlags)
{
    static TModuleProcV4<HDC,LPRECT,LPCTSTR,UINT>
        drawStatusText(GetModule(), DrawStatusTextStr);
  drawStatusText(hDC, lprc, pszText, uFlags);
}
//
void 
TCommCtrlDll::GetEffectiveClientRect(HWND hWnd, LPRECT lprc, int* lpInfo)
{
  static TModuleProcV3<HWND,LPRECT,LPINT> 
    getEffectiveClientRect(GetModule(), GetEffectiveClientRectStr);
  getEffectiveClientRect(hWnd, lprc, lpInfo);
}
//
int
TCommCtrlDll::PropertySheet(LPCPROPSHEETHEADER hHeader)
{
    static TModuleProc1<int,LPCPROPSHEETHEADER>
        propertySheet(GetModule(), PropertySheetStr);
  return propertySheet(hHeader);
}
//
HIMAGELIST  
TCommCtrlDll::ImageList_Create(int cx, int cy, uint flags, int cInitial, int cGrow)
{
  static TModuleProc5<HIMAGELIST,int,int,UINT,int,int> 
    imageList_Create(GetModule(), ImageList_CreateStr);
  return imageList_Create(cx,cy,flags,cInitial,cGrow);
}
//
bool  
TCommCtrlDll::ImageList_Destroy(HIMAGELIST himl)
{
  static TModuleProc1<BOOL,HIMAGELIST> 
    imageList_Destroy(GetModule(), ImageList_DestroyStr);
  return imageList_Destroy(himl);
}
//
int  
TCommCtrlDll::ImageList_GetImageCount(HIMAGELIST himl)
{
  static TModuleProc1<int,HIMAGELIST> 
    imageList_GetImageCount(GetModule(), ImageList_GetImageCountStr);
  return imageList_GetImageCount(himl);
}
//
int 
TCommCtrlDll::ImageList_Add(HIMAGELIST himl, HBITMAP hbmImage, HBITMAP hbmMask)
{
  TModuleProc3<int,HIMAGELIST,HBITMAP,HBITMAP> 
    imageList_Add(GetModule(), ImageList_AddStr);
  return imageList_Add(himl, hbmImage, hbmMask);
}
//
COLORREF 
TCommCtrlDll::ImageList_SetBkColor(HIMAGELIST himl, COLORREF clrBk)
{
  static TModuleProc2<COLORREF,HIMAGELIST,COLORREF> 
    imageList_SetBkColor(GetModule(), ImageList_SetBkColorStr);
  return imageList_SetBkColor(himl, clrBk);
}
//
COLORREF 
TCommCtrlDll::ImageList_GetBkColor(HIMAGELIST himl)
{
  static TModuleProc1<COLORREF,HIMAGELIST> 
    imageList_GetBkColor(GetModule(), ImageList_GetBkColorStr);
  return imageList_GetBkColor(himl);
}
//
bool 
TCommCtrlDll::ImageList_SetOverlayImage(HIMAGELIST himl, int iImage, int iOverlay)
{
  static TModuleProc3<BOOL,HIMAGELIST,int,int> 
    imageList_SetOverlayImage(GetModule(), ImageList_SetOverlayImageStr);
  return imageList_SetOverlayImage(himl,iImage, iOverlay);
}
//
bool 
TCommCtrlDll::ImageList_Draw(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, uint fStyle)
{
  static TModuleProc6<BOOL,HIMAGELIST,int,HDC,int,int,UINT> 
    imageList_Draw(GetModule(), ImageList_DrawStr);
  return imageList_Draw(himl, i, hdcDst, x, y, fStyle);
}
//
int   
TCommCtrlDll::ImageList_ReplaceIcon(HIMAGELIST himl, int i, HICON hicon)
{
  static TModuleProc3<BOOL,HIMAGELIST,int,HICON> 
    imageList_ReplaceIcon(GetModule(), ImageList_ReplaceIconStr);
  return imageList_ReplaceIcon(himl, i, hicon);
}
//
int 
TCommCtrlDll::LBItemFromPt(HWND hLB, POINT pt, bool bAutoScroll)
{
  static TModuleProc3<int,HWND,POINT,BOOL> lBItemFromPt(GetModule(), LBItemFromPtStr);
  return lBItemFromPt(hLB, pt, bAutoScroll);
}
//
bool 
TCommCtrlDll::MakeDragList(HWND hLB)
{
  static TModuleProc1<BOOL,HWND> makeDragList(GetModule(), MakeDragListStr);
  return  makeDragList(hLB);

}
//
void 
TCommCtrlDll::MenuHelp(uint uMsg, WPARAM wParam, LPARAM lParam, HMENU hMainMenu, HINSTANCE hInst, HWND hwndStatus, uint* lpwIDs)
{
  static TModuleProcV7<UINT,TParam1,TParam2,HMENU,HINSTANCE,HWND,UINT *> 
      menuHelp(GetModule(), MenuHelpStr);
  menuHelp(uMsg,wParam,lParam,hMainMenu,hInst,hwndStatus,lpwIDs);
}
//
bool 
TCommCtrlDll::ShowHideMenuCtl(HWND hWnd, uint uFlags, int* lpInfo)
{
  static TModuleProc3<BOOL,HWND,UINT,LPINT> 
    showHideMenuCtl(GetModule(), ShowHideMenuCtlStr);
  return showHideMenuCtl(hWnd, uFlags, lpInfo);
}
//
TCommCtrlDll& 
TCommCtrlDll::GetModule()
{
  static TCommCtrlDll commctrlDLL;
  return commctrlDLL;
}

//
//
//
TCommCtrlDll*
TCommCtrl::Dll()
{
  return &TCommCtrlDll::GetModule();
}

//
//
//
bool
TCommCtrl::IsAvailable()
{
  return Dll() != 0;
}

//----------------------------------------------------------------------------

//
/// Constructor of the Object thrown by the CommonControl wrappers of ObjectWindows.
//
TXCommCtrl::TXCommCtrl()
:
  TXOwl(IDS_COMMCTRL)
{
}

//
/// Route to copy an exception object. The 'Clone' method is invoked when
/// an exception must be suspended. The cloned copy can latter be used to
/// resume the exception.
//
TXCommCtrl*
TXCommCtrl::Clone() const
{
  return new TXCommCtrl(*this);
}


//
/// Method to localize the actual call to 'throw' an TXCommCtrl object.
/// \note Localizing the call to 'throw' is a size optimization: the
///       overhead of a 'throw' statement is generated only once.
//
void
TXCommCtrl::Throw()
{
  throw *this;
}

//
/// Constructs a TXCommCtrl exception from scratch, and throws it.
//
void
TXCommCtrl::Raise()
{
  TXCommCtrl().Throw();
}

//----------------------------------------------------------------------------
// DISPATCH.CPP
//
_OWLFUNC(LRESULT)
v_NMHDRCode_Dispatch(TGeneric& i, void (TGeneric::*f)(uint), WPARAM, LPARAM p2)
{
  (i.*f)((LPNMHDR(p2))->code);
  return 0;
}

} // OWL namespace
/* ========================================================================== */

