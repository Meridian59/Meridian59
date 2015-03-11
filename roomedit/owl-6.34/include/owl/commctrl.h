//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Definition of classes for CommonControl encapsulation
//----------------------------------------------------------------------------

#if !defined(OWL_COMMCTRL_H)
#define OWL_COMMCTRL_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

//JJH
#if !defined(RC_INVOKED)

# include <owl/control.h>
# include <owl/module.h>

#if !defined(__RICHEDIT_H)
# include <richedit.h>
#endif

#endif /*RC_INVOKED*/

//
// Get the necessary system headers
//
#if !defined(_INC_COMMCTRL)
# include <commctrl.h>
#endif

#include <owl/shelwapi.h>

namespace owl {

//
// Property sheets notify property pages of events via the standard WM_NOTIFY/
// NMHDR* mechanism. However, there is no Control ID involved in this
// particular flavour of notification. The WM_NOTIFY handler of ObjectWindows
// relies on the CtlID for subdispatching. Hence, we'll have 'PropPageID' as
// the default identifier so that we do not have to repeat dispatching logic
// in the 'EvNotify' of TPropertyPage.
//
const int PropPageID = 0xFFF0;

//
//
//
const uint ComCtlVersionWin95 = 0x40000; // 4.0
const uint ComCtlVersionIE3   = 0x40007; // 4.7
const uint ComCtlVersionIE4   = 0x40047; // 4.71
const uint ComCtlVersionIE41  = 0x40048; // 4.72

//----------------------------------------------------------------------------
// Forward declaration of classes used in dispatch macros
//
class _OWLCLASS THdrItem;

// Generic definitions/compiler options (eg. alignment) preceeding the
// definition of classes
#include <owl/preclass.h>


/// \class TDllVersionInfo
/// lite wrapper
//
class _OWLCLASS TDllVersionInfo: public DLLVERSIONINFO {
  public:
    TDllVersionInfo()
      {
        memset(this, 0, sizeof(TDllVersionInfo));
        cbSize = sizeof(TDllVersionInfo);
      }

};

class _OWLCLASS TCommCtrl;

/// \addtogroup commctrl
/// @{
/// \class TCommCtrlDll
// ~~~~~ ~~~~~~~~~~~~
/// Actual TModule derivative that wraps the common control DLL, implements delay loading and delay function banding
//
/// The TCommCtrlDll class encapsulates the Common Control DLL (COMCTL32.DLL). It
/// provides an easy method to dynamically test for the availability of the DLL and
/// bind to its exported functions at runtime. By using the TCommCtrlDll class
/// instead of direct calls to the Common Control DLL, ObjectWindows applications
/// can provide the appropriate behavior when running in an environment where the
/// DLL is not available.
/// 
/// Each data member of the TCommCtrlDll class corresponds to the API with a similar
/// name exposed by the Common Control DLL. For example,
/// TCommCtrlDll::InitCommonControls corresponds to the InitCommonControls API
/// exported by the Common Control DLL.
//
class _OWLCLASS TCommCtrlDll : public TModule {
  public:
    TCommCtrlDll();
    void    InitCommonControls();
    bool    GetCtrlVersion(TDllVersionInfo& info);
    uint32  GetCtrlVersion();

    static  bool    InitCommonControlsEx(uint32 type);
    static  void    CheckCommonControl(uint32 type);

    static  bool  InitializeFlatSB(HWND hWnd);                      // Version 4.71
    static  HRESULT  UninitializeFlatSB(HWND hWnd);                  // Version 4.71
    static  bool   FlatSB_EnableScrollBar(HWND,int,uint);            // Version 4.71
    static  bool   FlatSB_ShowScrollBar(HWND,int,bool);              // Version 4.71
    static  bool   FlatSB_GetScrollInfo(HWND,int,LPSCROLLINFO);       // Version 4.71
    static  int   FlatSB_SetScrollInfo(HWND,int,LPSCROLLINFO,bool); // Version 4.71
    static  int   FlatSB_GetScrollPos(HWND,int);                    // Version 4.71
    static  int    FlatSB_SetScrollPos(HWND,int,int,bool);            // Version 4.71
    static  bool  FlatSB_GetScrollProp(HWND,uint,int*);              // Version 4.71
    static  bool   FlatSB_SetScrollProp(HWND,uint,int,bool);          // Version 4.71
    static  bool   FlatSB_GetScrollRange(HWND,int,int*,int*);        // Version 4.71
    static  int    FlatSB_SetScrollRange(HWND,int,int,int,bool);      // Version 4.71

    // Image List
    static  HIMAGELIST ImageList_Read( LPSTREAM pstm);
    static  bool  ImageList_Write(HIMAGELIST himl, LPSTREAM pstm);
    static  bool  ImageList_Copy(HIMAGELIST himlDst, int iDst, HIMAGELIST himlSrc, int iSrc, uint uFlags); // Version 4.70
    static  HIMAGELIST ImageList_Duplicate(HIMAGELIST himl);                // Version 4.70
    static  bool  ImageList_DrawIndirect(IMAGELISTDRAWPARAMS* pimldp);      // Version 4.70
    static  bool  ImageList_SetImageCount(HIMAGELIST himl, uint uNewCount);  // Version 4.70
    static  int    ImageList_AddMasked(HIMAGELIST,HBITMAP,COLORREF);
    static  bool  ImageList_BeginDrag(HIMAGELIST miml,int track, int dxHot, int dyHot);
    static  void  ImageList_EndDrag();
    static  bool  ImageList_DragEnter(HWND hLock, int x, int y);
    static  bool  ImageList_DragLeave(HWND hLock);
    static  bool  ImageList_DragMove(int x, int y);
    static  bool  ImageList_SetDragCursorImage(HIMAGELIST himl, int drag, int dxHot, int dyHot);
    static  bool  ImageList_DragShowNolock(bool show);
    static  HIMAGELIST ImageList_GetDragImage(POINT* ppt, POINT* pptHotSpot);
    static  bool  ImageList_DrawEx(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, int dx, int dy, COLORREF rgbBk, COLORREF rgbFg, UINT fStyle);
    static  bool  ImageList_GetIconSize(HIMAGELIST himl, int  *cx, int  *cy);
    static  HICON ImageList_GetIcon(HIMAGELIST himl, int i, uint flags);
    static  bool  ImageList_GetImageInfo(HIMAGELIST himl, int i, IMAGEINFO * pImageInfo);
    static  bool  ImageList_Replace(HIMAGELIST himl, int i, HBITMAP hbmImage, HBITMAP hbmMask);
    static  bool  ImageList_Remove(HIMAGELIST himl, int i);
    static  HIMAGELIST  ImageList_LoadImage(HINSTANCE hi, LPCTSTR lpbmp, int cx, int cGrow, COLORREF crMask, uint uType, uint uFlags);
    static  HIMAGELIST ImageList_Merge(HIMAGELIST himl1, int i1, HIMAGELIST himl2, int i2, int dx, int dy);
    static  bool  ImageList_SetIconSize(HIMAGELIST himl, int cx, int cy);

    static  HBITMAP CreateMappedBitmap(HINSTANCE hInstance, int idBitmap,uint wFlags, LPCOLORMAP lpColorMap, int iNumMaps);
    static  HPROPSHEETPAGE CreatePropertySheetPage(LPCPROPSHEETPAGE lpPage);
    static  HWND CreateStatusWindow(long style, LPCTSTR lpszText, HWND hwndParent, uint wID);
    static  HWND CreateToolbarEx(HWND hwnd, DWORD ws, UINT wID, int nBitmaps,HINSTANCE hBMInst, UINT wBMID, LPCTBBUTTON lpButtons,
                                int iNumButtons, int dxButton, int dyButton,int dxBitmap, int dyBitmap, UINT uStructSize);
    static  HWND CreateUpDownControl(DWORD dwStyle, int x, int y, int cx, int cy,HWND hParent, int nID, HINSTANCE hInst,
                                HWND hBuddy,int nUpper, int nLower, int nPos);
    static  bool DestroyPropertySheetPage(HPROPSHEETPAGE);
    static  void DrawInsert(HWND handParent, HWND hLB, int nItem);
    static  void DrawStatusText(HDC hDC, LPRECT lprc, LPCTSTR pszText, uint uFlags);
    static  void GetEffectiveClientRect(HWND hWnd, LPRECT lprc, int* lpInfo);

    static  HIMAGELIST  ImageList_Create(int cx, int cy, uint flags, int cInitial, int cGrow);
    static  bool  ImageList_Destroy(HIMAGELIST himl);
    static  int    ImageList_GetImageCount(HIMAGELIST himl);
    static  int    ImageList_Add(HIMAGELIST himl, HBITMAP hbmImage, HBITMAP hbmMask);
    static  int    ImageList_ReplaceIcon(HIMAGELIST himl, int i, HICON hicon);
    static  COLORREF ImageList_SetBkColor(HIMAGELIST himl, COLORREF clrBk);
    static  COLORREF ImageList_GetBkColor(HIMAGELIST himl);
    static  bool  ImageList_SetOverlayImage(HIMAGELIST himl, int iImage, int iOverlay);
    static  bool  ImageList_Draw(HIMAGELIST himl, int i, HDC hdcDst, int x, int y, uint fStyle);

    static  int    LBItemFromPt(HWND hLB, POINT pt, bool bAutoScroll);
    static  bool  MakeDragList(HWND hLB);
    static  void  MenuHelp(uint uMsg, WPARAM wParam, LPARAM lParam, HMENU hMainMenu, HINSTANCE hInst, HWND hwndStatus, uint*lpwIDs);
    static  int    PropertySheet(LPCPROPSHEETHEADER hHeader);
    static  bool  ShowHideMenuCtl(HWND hWnd, uint uFlags, int* lpInfo);

  protected:
    static TCommCtrlDll& GetModule();

    uint32  Version;
    friend class TCommCtrl; 
};



// !BB #if 0   // !BB Need method to turn off COMMCTRL to fix P1 bug and this is driving me nuts !!!
// !BB  //     // !BB Templates are great until you need to tweak the code generated...
// !BB  // 
// !BB  // class TCommCtrl
// !BB  // ~~~~~ ~~~~~~~~~
// !BB  // !BB #define TCommCtrl TDllLoader<TCommCtrlDll>
// !BB  typedef TDllLoader<TCommCtrlDll> TCommCtrl;
// !BB
// !BB  #if defined(BI_COMP_BORLANDC)
// !BB  # if defined(_OWLDLL) || defined(BI_APP_DLL)
// !BB  //
// !BB  // Export template of TDllLoader<TComCtrlDll> when building ObjectWindows DLL
// !BB  // and provide import declaration of DLL instance for users of the class.
// !BB  //
// !BB  template class _OWLCLASS TDllLoader<TCommCtrlDll>;
// !BB  # endif //  _OWLDLL || BI_APP_DLL
// !BB  #endif  //  BI_COMP_BORLANDC
// !BB  
// !BB  #else

//
/// Simple wrapper class which creates a single instance of the TCommCtrlDll
/// class defined above.
//
class _OWLCLASS TCommCtrl {
  public:
    static TCommCtrlDll* Dll();
    static bool          IsAvailable();
};

// !BB  #endif

//
/// \class TXCommCtrl
// ~~~~~ ~~~~~~~~~~
/// Exception object thrown by the Common Control wrappers of ObjectWindows.
//
class _OWLCLASS TXCommCtrl : public TXOwl {
  public:
    TXCommCtrl();
    virtual TXCommCtrl* Clone() const; // override
    void        Throw();
    static void Raise();
};

//
/// \class TNotify
// ~~~~~ ~~~~~~~
/// TNotify is a thin wrapper around the NMHDR structure. It's a
/// place-holder for future enhancements for handling notifications.
//
class _OWLCLASS TNotify : public NMHDR {
  public:
    TNotify();
    TNotify(HWND ctl, uint id, uint code);
};


//----------------------------------------------------------------------------
// Structures used for UpDown (Spin Ctrl) notifications
//----------------------------------------------------------------------------
//
/// \class TNmUpDown
// ~~~~~ ~~~~~~~~~
/// TNmUpDown is a wrapper of the NM_UPDOWN structure sent with
/// notifications from an 'UpDown' control.
//
class _OWLCLASS TNmUpDown : public NM_UPDOWN {
  public:
    TNmUpDown(HWND hwnd, uint id, uint code, int pos, int delta);

    // Allows the notification structure to be transparently treated an
    // an NMHDR structure thereby eliminating the need to explicitly refer
    // to the NMHDR data member [always the first member of notification
    // structures]
    //
    operator  NMHDR&() { return hdr; }
};



//----------------------------------------------------------------------------
// Structures used for Tab Control notifications
//----------------------------------------------------------------------------
//
/// \class TTabKeyDown
// ~~~~~ ~~~~~~~~~~~
/// Wrapper of the TC_KEYDOWN structure sent to the parent of a tabcontrol
/// to notify that a key has been pressed
//
class _OWLCLASS TTabKeyDown : public TC_KEYDOWN {
  public:
    TTabKeyDown();
    TTabKeyDown(HWND ctl, uint id, uint code, uint16 vk, uint flg);

/// Allows the notification structure to be transparently treated as an NMHDR
/// structure, thereby eliminating the need to explicitly refer to the NMHDR data
/// member (which is always the first member of notification structures).
    operator  NMHDR&() { return hdr; }
};



//----------------------------------------------------------------------------
// Structures used for tooltip notifications & API
//----------------------------------------------------------------------------
//
/// \class TTooltipText
// ~~~~~ ~~~~~~~~~~~~
/// TTooltipText identifies a tool for which text is to be displayed. It
/// is sent to the parent of the tooltip via a WM_NOTIFY/TTN_NEEDTEXT
/// notification. It receives the text to be displayed.
//
class _OWLCLASS TTooltipText : public TOOLTIPTEXT {
  public:
    operator  NMHDR&() { return hdr; }

    // Set the text to be displayed for the tool.
    //
    void        SetText(LPCTSTR buff);
    void        SetText(int resId, HINSTANCE hInstance);
    void        CopyText(LPCTSTR buff);
};

//
//
//
class _OWLCLASS TTtCustomDraw : public NMTTCUSTOMDRAW {
  public:
    operator  NMCUSTOMDRAW&() { return nmcd; }
};

//
//
//
class _OWLCLASS TTooltipDispInfo : public NMTTDISPINFO {
  public:
    operator  NMHDR&() { return hdr; }
};

//
// Documented but not defined ????????????????
//
#if 0
class _OWLCLASS TTooltipCreated : public NMTOOLTIPSCREATED {
  public:
    operator  NMHDR&() { return hdr; }
};
#endif

//----------------------------------------------------------------------------
// Structures used for Column Header notifications
//----------------------------------------------------------------------------
//
/// \class THdrNotify
// ~~~~~ ~~~~~~~~~~
/// THdrNotify encapsulates the 'HD_NOTIFY' structure which is sent
/// with the WM_NOTIFY messages sent by the HeaderControl to its parent.
//
class _OWLCLASS THdrNotify : public HD_NOTIFY {
  public:
/// Allows the notification structure to be transparently treated as an NMHDR
/// structure, thereby eliminating the need to explicitly refer to the NMHDR data
/// member (which is always the first member of notification structures).
    operator  NMHDR&() { return hdr; }
};

//
// Version 4.0
//
class _OWLCLASS THdrDispInfo : public NMHDDISPINFO {
  public:
    operator  NMHDR&() { return hdr; }
};

//----------------------------------------------------------------------------
// TDateTimePicker structures
//
// Version 4.70 
//
class _OWLCLASS TDateTimeChange : public NMDATETIMECHANGE {
  public:
    operator  NMHDR&() { return nmhdr; }
};

//
// Version 4.70 
//
class _OWLCLASS TDateTimeFormat : public NMDATETIMEFORMAT {
  public:
    operator  NMHDR&() { return nmhdr; }
};

//
// Version 4.70 
//
class _OWLCLASS TDateTimeFormatQuery : public NMDATETIMEFORMATQUERY {
  public:
    operator  NMHDR&() { return nmhdr; }
};

//
// Version 4.70 
//
class _OWLCLASS TDateTimeString : public NMDATETIMESTRING {
  public:
    operator  NMHDR&() { return nmhdr; }
};

//
// Version 4.70 
//
class _OWLCLASS TDateTimeKeyDown : public NMDATETIMEWMKEYDOWN {
  public:
    operator  NMHDR&() { return nmhdr; }
};

//----------------------------------------------------------------------------
// Structures used for RichEdit notifications and API
//----------------------------------------------------------------------------
//
/// \class TEnDropFiles
// ~~~~~ ~~~~~~~~~~~~
/// Structure sent with EN_DROPFILES notification.
//
class _OWLCLASS TEnDropFiles : public ENDROPFILES {
  public:
/// Allows the notification structure to be transparently treated as an NMHDR
/// structure, thereby eliminating the need to explicitly refer to the NMHDR data
/// member (which is always the first member of notification structures).
    operator  NMHDR&() { return nmhdr; }
};

//
/// \class TMsgFilter
// ~~~~~ ~~~~~~~~~~
/// Structure sent with EN_MSGFILTER notification
//
class _OWLCLASS TMsgFilter : public MSGFILTER {
  public:
/// Allows the notification structure to be transparently treated as an NMHDR
/// structure, thereby eliminating the need to explicitly refer to the NMHDR data
/// member (which is always the first member of notification structures).
    operator  NMHDR&() { return nmhdr; }
};

//
/// \class TEnProtected
// ~~~~~ ~~~~~~~~~~~~
/// Structure sent with EN_PROTECTED notification
//
class _OWLCLASS TEnProtected : public ENPROTECTED {
  public:
/// Allows the notification structure to be transparently treated as an NMHDR
/// structure, thereby eliminating the need to explicitly refer to the NMHDR data
/// member (which is always the first member of notification structures).
    operator  NMHDR&() { return nmhdr; }
};

//
/// \class TReqResize
// ~~~~~ ~~~~~~~~~~
/// Structure sent with EN_REQUESTRESIZE notification
//
class _OWLCLASS TReqResize : public REQRESIZE {
  public:
/// Allows the notification structure to be transparently treated as an NMHDR
/// structure, thereby eliminating the need to explicitly refer to the NMHDR data
/// member (which is always the first member of notification structures).
    operator  NMHDR&() { return nmhdr; }
};

//
/// \class TSelChange
// ~~~~~ ~~~~~~~~~~
/// Structure sent with EN_SELCHANGE notification
//
class _OWLCLASS TSelChange : public SELCHANGE {
  public:
/// Allows the notification structure to be transparently treated as an NMHDR
/// structure, thereby eliminating the need to explicitly refer to the NMHDR data
/// member (which is always the first member of notification structures).
    operator  NMHDR&() { return nmhdr; }
};


//
// Forward declarations of notifications
//

//----------------------------------------------------------------------------
// Structures used for ListView control (TListViewCtrl) notifications
//----------------------------------------------------------------------------
//
/// \class TLvNotify
// ~~~~~ ~~~~~~~~~
/// Basic TListViewCtrl notification
//
class _OWLCLASS TLvNotify : public NMLISTVIEW {
  public:
    operator  NMHDR&() { return hdr; }
};
typedef TLvNotify TLwNotify; // Old alternative name - deprecated.

//
/// \class TLvDispInfoNotify
// ~~~~~ ~~~~~~~~~~~~~~~~~
/// A TListViewCtrl notification to repaint an item.
//
class _OWLCLASS TLvDispInfoNotify : public NMLVDISPINFO {
  public:
    operator  NMHDR&() { return hdr; }
};
typedef TLvDispInfoNotify TLwDispInfoNotify; // Old alternative name - deprecated.

//
/// \class TLvKeyDownNotify
// ~~~~~ ~~~~~~~~~~~~~~~~
/// A TListViewCtrl notification that a key has been pressed.
//
class _OWLCLASS TLvKeyDownNotify : public NMLVKEYDOWN {
  public:
    operator  NMHDR&() { return hdr; }
};
typedef TLvKeyDownNotify TLwKeyDownNotify; // Old alternative name - deprecated.

//
/// \class TLvBkImage
// ~~~~~ ~~~~~~~~~~
// Version 4.70
class _OWLCLASS TLvBkImage : public LVBKIMAGE{
  public:
    TLvBkImage(){}
};
typedef TLvBkImage TLwBkImage; // Old alternative name - deprecated.

//
/// \class TLvCacheHint
// ~~~~~ ~~~~~~~~~~
// Version 4.70
class _OWLCLASS TLvCacheHint : public NMLVCACHEHINT{
  public:
    TLvCacheHint(){}
    operator  NMHDR&() { return hdr; }
};
typedef TLvCacheHint TLwCacheHint; // Old alternative name - deprecated.

//
/// \class TLvCustomDraw
// ~~~~~ ~~~~~~~~~~~~~
// Version 4.70/4.71
class _OWLCLASS TLvCustomDraw : public NMLVCUSTOMDRAW{
  public:
    TLvCustomDraw(){}

    operator  NMCUSTOMDRAW&() { return nmcd; }

    TColor   GetTextColor()   const { return TColor(clrText);   }
    TColor   GetBkColor()     const { return TColor(clrTextBk); }
    int     GetSubItem()    const { return iSubItem;          } // Version 4.71
};
typedef TLvCustomDraw TLwCustomDraw; // Old alternative name - deprecated.

//
/// \class TLvFindItem
// ~~~~~ ~~~~~~~~~~~
// Version 4.70
class _OWLCLASS TLvFindItem : public NMLVFINDITEM{
  public:
    TLvFindItem(){}

    operator  NMHDR&() { return hdr; }
};
typedef TLvFindItem TLwFindItem; // Old alternative name - deprecated.

//
/// \class TLvOdStateChanged
// ~~~~~ ~~~~~~~~~~~~~~~~~
// Version 4.70
class _OWLCLASS TLvOdStateChanged : public NMLVODSTATECHANGE{
  public:
    TLvOdStateChanged(){}

    operator  NMHDR&() { return hdr; }
};
typedef TLvOdStateChanged TLwOdStateChanged; // Old alternative name - deprecated.

//
/// \class TLvGetInfoTip
// ~~~~~ ~~~~~~~~~~~~~
// Version 4.71
class _OWLCLASS TLvGetInfoTip : public NMLVGETINFOTIP{
  public:
    TLvGetInfoTip(){}

    operator  NMHDR&() { return hdr; }
};
typedef TLvGetInfoTip TLwGetInfoTip; // Old alternative name - deprecated.

//
/// \class TLvItemActivate
// ~~~~~ ~~~~~~~~~~~~~~~
// Version 4.71
class _OWLCLASS TLvItemActivate : public NMITEMACTIVATE{
  public:
    TLvItemActivate(){}

    operator  NMHDR&() { return hdr; }
};
typedef TLvItemActivate TLwItemActivate; // Old alternative name - deprecated.



//----------------------------------------------------------------------------
// Structures used for TreeView control (TTreeViewCtrl) notifications
//----------------------------------------------------------------------------
//
/// \class TTvNotify
// ~~~~~ ~~~~~~~~~
/// Basic TTreeViewCtrl notification
//
class _OWLCLASS TTvNotify : public NMTREEVIEW {
  public:
    operator  NMHDR&() { return hdr; }
};
typedef TTvNotify TTwNotify; // Old alternative name - deprecated.

//
/// \class TTvDispInfoNotify
// ~~~~~ ~~~~~~~~~~~~~~~~~
/// A TTreeViewCtrl notification to repaint an item.
//
class _OWLCLASS TTvDispInfoNotify : public NMTVDISPINFO {
  public:
    operator  NMHDR&() { return hdr; }
};
typedef TTvDispInfoNotify TTwDispInfoNotify; // Old alternative name - deprecated.

//
/// \class TTvKeyDownNotify
// ~~~~~ ~~~~~~~~~~~~~~~~
/// A TTreeViewCtrl notification that a key has been pressed.
//
class _OWLCLASS TTvKeyDownNotify : public NMTVKEYDOWN {
  public:
    operator  NMHDR&() { return hdr; }
};
typedef TTvKeyDownNotify TTwKeyDownNotify; // Old alternative name - deprecated.

//
/// \class TTvHitTestInfo
// ~~~~~ ~~~~~~~~~~~~~~
/// A TTreeViewCtrl notification for hit-testing.
//
class _OWLCLASS TTvHitTestInfo : public TVHITTESTINFO {
  public:
};
typedef TTvHitTestInfo TTwHitTestInfo; // Old alternative name - deprecated.

//
/// \class TTvCustomDraw
// ~~~~~ ~~~~~~~~~~~~~
// Version 4.70/4.71
class _OWLCLASS TTvCustomDraw : public NMTVCUSTOMDRAW{
  public:
    operator NMHDR&() {return nmcd.hdr;}
    operator  NMCUSTOMDRAW&() { return   nmcd; }

    TColor   GetTextColor()   const { return TColor(clrText);   }
    TColor   GetBkColor()     const { return TColor(clrTextBk); }
    int     GetLevel()      const { return iLevel;            } // Version 4.71
};
typedef TTvCustomDraw TTwCustomDraw; // Old alternative name - deprecated.

//
// class TTvGetInfoTip
// ~~~~~ ~~~~~~~~~~~~~
// Version 4.71
class _OWLCLASS TTvGetInfoTip : public NMTVGETINFOTIP {
  public:
    operator  NMHDR&() { return hdr; }
};
typedef TTvGetInfoTip TTwGetInfoTip; // Old alternative name - deprecated.

////////////////////////////////////////////////////////////////////////////////
//
/// \class TNmMouse
// ~~~~~ ~~~~~~~~
//
class _OWLCLASS TNmMouse : public NMMOUSE {
  public:
    operator  NMHDR&() { return hdr; }
};

//
/// \class TNmObjectNotify
// ~~~~~ ~~~~~~~~~~~~~~~
//
class _OWLCLASS TNmObjectNotify : public NMOBJECTNOTIFY {
  public:
    operator  NMHDR&() { return hdr; }
};

//
/// \class TNmKey
// ~~~~~ ~~~~~~
//
class _OWLCLASS TNmKey : public NMKEY {
  public:
    operator  NMHDR&() { return hdr; }
};


//
/// \class TNmChar
// ~~~~~ ~~~~~~~
//
class _OWLCLASS TNmChar : public NMCHAR {
  public:
    operator  NMHDR&() { return hdr; }
};

//
/// \class TNmCustomDraw
// ~~~~~ ~~~~~~~~~~~~~
//
class _OWLCLASS TNmCustomDrawInfo : public NMCUSTOMDRAW {
  public:
    TNmCustomDrawInfo(){}
};

//
/// \class TNmCustomDraw
// ~~~~~ ~~~~~~~~~~~~~
//
class _OWLCLASS TNmCustomDraw : public NMTTCUSTOMDRAW {
  public:
    TNmCustomDraw(){}
};

//
/// \class TRebarInfo
// ~~~~~ ~~~~~~~~~~
//
class _OWLCLASS TRebarInfo : public REBARINFO {
  public:
    TRebarInfo() { cbSize = sizeof(REBARINFO); }

    // Would prefer to use TImageLists...
    void SetImageList(HIMAGELIST Images);
    HIMAGELIST GetImageList();
};

//
/// \class TColorScheme
// ~~~~~ ~~~~~~~~~~~~
//
class _OWLCLASS TColorScheme : public COLORSCHEME {
  public:
    TColorScheme()
      {
        dwSize=sizeof(COLORSCHEME);
        clrBtnHighlight=clrBtnShadow=TColor::None;
      }
    TColorScheme(const TColor& btn, const TColor& shad)
      {
        dwSize = sizeof(COLORSCHEME);
        clrBtnHighlight=btn;
        clrBtnShadow=shad;
      }

    TColor  BtnHighlight() { return TColor(clrBtnHighlight);    }
    void      BtnHighlight(const TColor& clr){clrBtnHighlight=clr;}
    TColor   BtnShadow()    { return TColor(clrBtnShadow);        }
    void      BtnShadow(const TColor& clr){ clrBtnShadow  = clr;  }
};

//
/// \class TRbHitTestInfo
// ~~~~~ ~~~~~~~~~~
//
class _OWLCLASS TRbHitTestInfo : public RBHITTESTINFO {
  public:
    TRbHitTestInfo(const TPoint& point){pt.x = point.x; pt.y=point.y;}
};

//
// ComboBoxEx structures
// ~~~~~~~~~~ ~~~~~~~~~~
//
class _OWLCLASS TCBExItemInfo : public COMBOBOXEXITEM {
  public:
};

class _OWLCLASS TCBExEditInfo : public NMCBEENDEDIT {
  public:
};

class _OWLCLASS TCBExDragInfo : public NMCBEDRAGBEGIN {
  public:
};

class _OWLCLASS TCBExNotifyInfo : public NMCOMBOBOXEX {
  public:
};

//
/// MonthCalendar Control notifications
//

class _OWLCLASS TMCHitTestInfo:  public MCHITTESTINFO{
  public:
    TMCHitTestInfo() { cbSize = sizeof(MCHITTESTINFO);}
};

//
/// \class TNmDayState
// ~~~~~ ~~~~~~~~~~~
//
class _OWLCLASS TNmDayState : public NMDAYSTATE {
  public:
    operator  NMHDR&() { return nmhdr; }
};

//
/// \class TNmSelChange
// ~~~~~ ~~~~~~~~~~~~
//
class _OWLCLASS TNmSelChange : public NMSELCHANGE {
  public:
    operator  NMHDR&() { return nmhdr; }
};


//
/// Pager Control notifications
//

class _OWLCLASS TNmPGCalcSize:  public NMPGCALCSIZE{
  public:
    operator  NMHDR&() { return hdr; }
};

class _OWLCLASS TNmPGScroll:  public NMPGSCROLL{
  public:
    operator  NMHDR&() { return hdr; }
};

//
/// IP Address control notifications
//

class _OWLCLASS TNmIPAddress:  public NMIPADDRESS{
  public:
    operator  NMHDR&() { return hdr; }
};


// Generic definitions/compiler options (eg. alignment) following the
// definition of classes
#include <owl/posclass.h>

//----------------------------------------------------------------------------
// DISPATCH.H
//

_OWLFUNC(LRESULT)
v_NMHDRCode_Dispatch(TGeneric&, void (TGeneric::*)(uint), WPARAM, LPARAM);

//----------------------------------------------------------------------------
// SIGNATURE.H
//
//} // OWL namespace
DECLARE_SIGNATURE1(void,v_NMHDR_Sig,NMHDR &)
DECLARE_SIGNATURE1(bool,b_NMHDR_Sig,NMHDR &)
DECLARE_SIGNATURE1(void,v_NOTIFY_Sig,TNotify &)
DECLARE_SIGNATURE1(bool,b_NOTIFY_Sig,TNotify &)
DECLARE_SIGNATURE1(int,i_NOTIFY_Sig,TNotify &)
DECLARE_SIGNATURE1(int,i_CUSTOMDRAW_Sig,TNmCustomDraw &)
DECLARE_SIGNATURE1(int,i_NMMOUSE_Sig,TNmMouse &)
DECLARE_SIGNATURE1(void,v_NMCHAR_Sig,TNmChar &)
DECLARE_SIGNATURE1(bool,b_NMOBJECTNOTIFY_Sig,TNmObjectNotify &)
DECLARE_SIGNATURE1(bool,b_NMKEY_Sig,TNmKey &)
DECLARE_SIGNATURE2(void,v_i_NOTIFY_Sig,int,TNotify &)
DECLARE_SIGNATURE2(bool,b_i_NOTIFY_Sig,int,TNotify &)
// documentd but not defined
#if 0
DECLARE_SIGNATURE1(void,v_TOOLTIPSCREATED_Sig,TTooltipCreated &)
#endif
//namespace owl {


//----------------------------------------------------------------------------
// WINDOWEV.H
//

//
/// Notification handled by parent with handler returning void
///      i.e.  void method()
//
#define EV_COMMCTL_NOTIFY(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::v_Dispatch,\
  (TMyPMF)::owl::v_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting notification
/// code.        i.e. void method(uint notifyCode)
//
#define EV_COMMCTL_NOTIFY_AND_CODE(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::v_NMHDRCode_Dispatch,\
  (TMyPMF)::owl::v_U_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting NMHDR&
/// as parameter.        i.e. void method(NMHDR& nmHdr)
//
#define EV_COMMCTL_NOTIFY_AND_NMHDR(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_NMHDR_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting NMHDR& and
/// returning a bool.
///                      i.e. bool method(NMHDR& nmHdr)
//
#define EV_COMMCTL_NOTIFY_BOOL_AND_NMHDR(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_NMHDR_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting TNotify&
/// as parameter.
///                      i.e. void method(TNotify& nmHdr)
//
#define EV_COMMCTL_NOTIFY_AND_NOTIFY(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting TNotify& and
/// returning a bool.
///                     i.e. bool method(TNotify& nmHdr)
//
#define EV_COMMCTL_NOTIFY_BOOL_AND_NOTIFY(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting TNotify& and
/// returning an int.
///                      i.e. int method(TNotify& not)
//
#define EV_COMMCTL_NOTIFY_INT_AND_NOTIFY(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::i_LPARAM_Dispatch,\
  (TMyPMF)::owl::i_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

/// Notification handled by parent with handler expecting TNmMouse&
/// as parameter.
///                      i.e. int method(TNmMouse& nmHdr)
//
#define EV_COMMCTL_NOTIFY_AND_NMMOUSE(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::i_LPARAM_Dispatch,\
  (TMyPMF)::owl::i_NMMOUSE_Sig<TMyClass>(&TMyClass::method)}


/// Notification handled by parent with handler expecting TNmCustomDraw &
/// as parameter.
///                      i.e. int method(TNmCustomDraw & )
//
#define EV_COMMCTL_NOTIFY_AND_CUSTOMDRAW(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::i_LPARAM_Dispatch,\
  (TMyPMF)::owl::i_CUSTOMDRAW_Sig<TMyClass>(&TMyClass::method)}


/// Notification handled by parent with handler expecting TNmChar &
/// as parameter.
///                      i.e. void method(TNmChar & )
//
#define EV_COMMCTL_NOTIFY_AND_NMCHAR(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_NMCHAR_Sig<TMyClass>(&TMyClass::method)}

/// Notification handled by parent with handler expecting TNmKey &
/// as parameter.
///                      i.e. bool method(TNmKey & nmKey)
//
#define EV_COMMCTL_NOTIFY_AND_NMKEY(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_NMKEY_Sig<TMyClass>(&TMyClass::method)}

/// Notification handled by parent with handler expecting int and TNotify &
/// as parameter.
///                      i.e. void method(int id, TNotify & not)
//
#define EV_COMMCTL_NOTIFY_AND_ID_NOTIFY(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::v_WPARAM_LPARAM_Dispatch,\
   (TMyPMF)::owl::v_i_NOTIFY_Sig<TMyClass>(&TMyClass::method)}
   	
#define EV_COMMCTL_NOTIFY_AND_ID_NOTIFY_RETURN(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::U_WPARAM_LPARAM_Dispatch,\
  	(TMyPMF)::owl::b_i_NOTIFY_Sig<TMyClass>(&TMyClass::method)} 

/// \name Common Control Notification  Messages
/// @{
/// These common control macros handle NM_xxxx notification codes. To determine the
/// name of the notification code that corresponds to the EV_XXXX macro, remove the
/// EV_ prefix.
#define EV_NM_CLICK(id, method)       EV_COMMCTL_NOTIFY(id, NM_CLICK, method)
#define EV_NM_DBLCLK(id, method)      EV_COMMCTL_NOTIFY(id, NM_DBLCLK, method)
#define EV_NM_KILLFOCUS(id, method)   EV_COMMCTL_NOTIFY(id, NM_KILLFOCUS, method)
#define EV_NM_OUTOFMEMORY(id, method) EV_COMMCTL_NOTIFY(id, NM_OUTOFMEMORY, method)
#define EV_NM_RCLICK(id, method)      EV_COMMCTL_NOTIFY(id, NM_RCLICK, method)
#define EV_NM_RDBLCLK(id, method)     EV_COMMCTL_NOTIFY(id, NM_RDBLCLK, method)
#define EV_NM_RETURN(id, method)      EV_COMMCTL_NOTIFY(id, NM_RETURN, method)
#define EV_NM_SETFOCUS(id, method)    EV_COMMCTL_NOTIFY(id, NM_SETFOCUS, method)
#define EV_NM_CUSTOMDRAW(id, method)  EV_COMMCTL_NOTIFY_AND_CUSTOMDRAW(id, NM_CUSTOMDRAW,method)
#define EV_NM_HOVER(id, method)        EV_COMMCTL_NOTIFY_BOOL_AND_NOTIFY(id, NM_HOVER, method)
#define EV_NM_NCHITTEST(id, method)    EV_COMMCTL_NOTIFY_AND_NMMOUSE(id, NM_NCHITTEST, method)
#define EV_NM_KEYDOWN(id, method)      EV_COMMCTL_NOTIFY_AND_NMKEY(id, NM_KEYDOWN, method)
#define EV_NM_RELEASEDCAPTURE(id, method) EV_COMMCTL_NOTIFY(id, NM_RELEASEDCAPTURE, method)
#define EV_NM_SETCURSOR(id, method)   EV_COMMCTL_NOTIFY_AND_NMMOUSE(id, NM_SETCURSOR, method)
#define EV_NM_CHAR(id, method)         EV_COMMCTL_NOTIFY_AND_NMCHAR(id, NM_CHAR, method)
/// @}

// documentd but not defined
//~~~~~~~~~~~~~~~~~~~~~~~~~~
#if 0
//
/// Version 4.72 
/// Notification handled by parent with handler expecting TTooltipCreated &
/// as parameter.
///                      i.e. void method(TTooltipCreated & tip)
//
#define EV_NM_TOOLTIPSCREATED(id, method)\
  {{NM_TOOLTIPSCREATED}, 0, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
   (TMyPMF)::owl::v_TOOLTIPSCREATED_Sig<TMyClass>(&TMyClass::method)}
#endif

//----------------------------------------------------------------------------
//
// Animation control notifications
//
// void method()
//
#define EV_ACN_START(id, method)      EV_CHILD_NOTIFY(id, ACN_START, method)
#define EV_ACN_STOP(id, method)       EV_COMMCTL_NOTIFY(id, ACN_STOP, method)


//----------------------------------------------------------------------------
// Header control notifications
//
DECLARE_SIGNATURE1(void,v_HEADERNOTIFY_Sig,THdrNotify &)
DECLARE_SIGNATURE1(bool,b_HEADERNOTIFY_Sig,THdrNotify &)
DECLARE_SIGNATURE1(void,v_HDN_DISPINFO_NOTIFY_Sig,THdrDispInfo &)
DECLARE_SIGNATURE1(void,b_HDN_DISPINFO_NOTIFY_Sig,THdrDispInfo &)

//
/// Notification handled by parent with handler expecting THdrNotify&
/// as parameter.        i.e. void method(THdrNotify& nmHdr)
//
#define EV_HEADERNOTIFY(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_HEADERNOTIFY_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting THdrNotify& and
/// returning a bool.
///                      i.e. bool method(THdrNotify& nmHdr)
//
#define EV_HEADERNOTIFY_BOOL(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_HEADERNOTIFY_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting THdrDispInfo& and
/// returning a void.    i.e. bool method(THdrDispInfo& nmHdr)
//
#define EV_HDN_DISPINFO_NOTIFY_BOOL(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_HDN_DISPINFO_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting THdrDispInfo&
/// i.e. void method(THdrDispInfo& nmHdr)
//
#define EV_HDN_DISPINFO_NOTIFY(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_HDN_DISPINFO_NOTIFY_Sig<TMyClass>(&TMyClass::method)}


/// \name Header Control Notification  Messages
/// @{
/// These header control macros handle EN_xxxx notification codes. To determine the
/// name of the notification code that corresponds to the EV_XXXX macro, remove the
/// EV_ prefix.
#define EV_HDN_BEGINDRAG(id, method)        EV_HEADERNOTIFY_BOOL(id, HDN_BEGINDRAG, method)
#define EV_HDN_BEGINTRACK(id, method)       EV_HEADERNOTIFY_BOOL(id, HDN_BEGINTRACK, method)
#define EV_HDN_DIVIDERDBLCLICK(id, method)  EV_HEADERNOTIFY(id, HDN_DIVIDERDBLCLICK, method)
#define EV_HDN_ENDDRAG(id, method)          EV_HEADERNOTIFY_BOOL(id, HDN_ENDDRAG, method)
#define EV_HDN_ENDTRACK(id, method)         EV_HEADERNOTIFY(id, HDN_ENDTRACK, method)
#define EV_HDN_ITEMCHANGED(id,method)       EV_HEADERNOTIFY(id, HDN_ITEMCHANGED, method)
#define EV_HDN_ITEMCHANGING(id,method)      EV_HEADERNOTIFY_BOOL(id, HDN_ITEMCHANGING, method)
#define EV_HDN_ITEMCLICK(id, method)        EV_HEADERNOTIFY(id, HDN_ITEMCLICK, method)
#define EV_HDN_ITEMDBLCLICK(id, method)     EV_HEADERNOTIFY(id, HDN_ITEMDBLCLICK, method)
#define EV_HDN_TRACK(id, method)            EV_HEADERNOTIFY_BOOL(id, HDN_TRACK, method)
#define EV_HDN_GETDISPINFO(id, method)      EV_HDN_DISPINFO_NOTIFY(id, HDN_GETDISPINFO, method)  // new
/// @}

//----------------------------------------------------------------------------
// TDateTimepicker control notifications
//
DECLARE_SIGNATURE1(int,i_DATETIMECHANGE_Sig,TDateTimeChange &)
DECLARE_SIGNATURE1(int,i_NMDATETIMEFORMAT_Sig,TDateTimeFormat &)
DECLARE_SIGNATURE1(int,i_NMDATETIMEFORMATQUERY_Sig,TDateTimeFormatQuery &)
DECLARE_SIGNATURE1(int,i_NMDATETIMESTRING_Sig,TDateTimeString &)
DECLARE_SIGNATURE1(int,i_NMDATETIMEWMKEYDOWN_Sig,TDateTimeKeyDown &)

//
// void method()
#define EV_DTN_CLOSEUP(id, method)          EV_COMMCTL_NOTIFY(id, DTN_CLOSEUP, method)

//
/// i.e. int method(TDateTimeChange & dtNot)
/// owner of control must return zerro
#define EV_DTN_DATETIMECHANGE(id, method)                       \
  {{DTN_DATETIMECHANGE}, id, (::owl::TAnyDispatcher) ::owl::i_LPARAM_Dispatch, \
  (TMyPMF)::owl::i_DATETIMECHANGE_Sig<TMyClass>(&TMyClass::method)}

//
/// void method()
#define EV_DTN_DROPDOWN(id, method)         EV_COMMCTL_NOTIFY(id, DTN_DROPDOWN, method)

//
/// i.e. int method(TDateTimeFormat & dtNot)
/// owner of control must return zerro
#define EV_DTN_FORMAT(id, method)                       \
  {{DTN_FORMAT}, id, (::owl::TAnyDispatcher) ::owl::i_LPARAM_Dispatch, \
  (TMyPMF)::owl::i_NMDATETIMEFORMAT_Sig<TMyClass>(&TMyClass::method)}

//
/// i.e. int method(TDateTimeFormat & dtNot)
/// owner of control must return zerro
#define EV_DTN_FORMATQUERY(id, method)                      \
  {{DTN_FORMATQUERY}, id, (::owl::TAnyDispatcher) ::owl::i_LPARAM_Dispatch,\
  (TMyPMF)::owl::i_NMDATETIMEFORMATQUERY_Sig<TMyClass>(&TMyClass::method)}

//
/// i.e. int method(TDateTimeString & dtNot)
/// owner of control must return zerro
#define EV_DTN_USERSTRING(id, method)                      \
  {{DTN_FORMATQUERY}, id, (::owl::TAnyDispatcher) ::owl::i_LPARAM_Dispatch,\
  (TMyPMF)::owl::i_NMDATETIMESTRING_Sig<TMyClass>(&TMyClass::method)}

//
//
/// i.e. int method(TDateTimeKeyDown & dtNot)
/// owner of control must return zerro
#define EV_DTN_WMKEYDOWN(id, method)                      \
  {{DTN_FORMATQUERY}, id, (::owl::TAnyDispatcher) ::owl::i_LPARAM_Dispatch,\
  (TMyPMF)::owl::i_NMDATETIMEWMKEYDOWN_Sig<TMyClass>(&TMyClass::method)}

/// \name Tab Control Notificaiton  Messages
/// @{
/// The following tab control macros handle TCN_xxxx notification codes. To
/// determine the name of the notification code that corresponds to the EV_XXXX
/// macro, remove the EV_ prefix.

//} // OWL namespace
DECLARE_SIGNATURE1(void,v_KEYDOWNNOTIFY_Sig,TTabKeyDown &)
//namespace owl {

//
/// Notification handled by parent with handler expecting THdrNotify&
/// as parameter.        i.e. void method(TTabKeyDown& nmHdr)
//
#define EV_TCN_KEYDOWN(id, method)\
  {{TCN_KEYDOWN}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_KEYDOWNNOTIFY_Sig<TMyClass>(&TMyClass::method)}


#define EV_TCN_SELCHANGE(id, method)    EV_COMMCTL_NOTIFY_AND_NOTIFY(id, TCN_SELCHANGE, method)
#define EV_TCN_SELCHANGING(id, method)  EV_COMMCTL_NOTIFY_BOOL_AND_NOTIFY(id, TCN_SELCHANGING, method)

// Version 4.71 : bool method(TNmObjectNotify& nmHdr)
#define EV_TCN_GETOBJECT(id, method) \
  {{TCN_GETOBJECT}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_NMOBJECTNOTIFY_Sig<TMyClass>(&TMyClass::method)}

/// @}

/// \name Property Sheet Notification  Messages
/// @{
/// The following property sheet macros handle PSN_xxxx notification codes. To
/// determine the name of the notification code that corresponds to the EV_XXXX
/// macro, remove the EV_ prefix.
///
/// \note These macros hard-code the CtlID to PropPageID. Since Property
//       Pages do not have the concept of IDs [the way Ctls have an
//       ID by which their parent can identify them] this mechanism
//       facilitates NOTIFICATION dispatching.
//

#define EV_PSN_APPLY(method) \
        EV_COMMCTL_NOTIFY_INT_AND_NOTIFY(PropPageID, (uint)PSN_APPLY, method)

// Version 4.71
#define EV_PSN_GETOBJECT(method) \
  {{PSN_GETOBJECT}, PropPageID, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_NMOBJECTNOTIFY_Sig<TMyClass>(&TMyClass::method)}

#define EV_PSN_HELP(method) \
        EV_COMMCTL_NOTIFY_AND_NOTIFY(PropPageID, (uint)PSN_HELP, method)

#define EV_PSN_KILLACTIVE(method)\
        EV_COMMCTL_NOTIFY_BOOL_AND_NOTIFY(PropPageID, (uint)PSN_KILLACTIVE, method)

#define EV_PSN_QUERYCANCEL(method)\
        EV_COMMCTL_NOTIFY_BOOL_AND_NOTIFY(PropPageID, (uint)PSN_QUERYCANCEL, method)

#define EV_PSN_RESET(method)\
        EV_COMMCTL_NOTIFY_AND_NOTIFY(PropPageID, (uint)PSN_RESET, method)

#define EV_PSN_SETACTIVE(method)\
        EV_COMMCTL_NOTIFY_INT_AND_NOTIFY(PropPageID, (uint)PSN_SETACTIVE, method)

#define EV_PSN_WIZBACK(method)\
        EV_COMMCTL_NOTIFY_INT_AND_NOTIFY(PropPageID, (uint)PSN_WIZBACK, method)

#define EV_PSN_WIZFINISH(method)\
        EV_COMMCTL_NOTIFY_BOOL_AND_NOTIFY(PropPageID, (uint)PSN_WIZFINISH, method)

#define EV_PSN_WIZNEXT(method)\
        EV_COMMCTL_NOTIFY_INT_AND_NOTIFY(PropPageID, (uint)PSN_WIZNEXT, method)

/// @}

/// \name Rich Edit Notification  Messages
/// @{
/// The following rich edit macros handle EN_xxxx notification codes. To determine
/// the name of the notification code that corresponds to the EV_XXXX macro, remove
/// the EV_ prefix.

DECLARE_SIGNATURE1(bool,b_ENDROPFILES_Sig,TEnDropFiles &)
DECLARE_SIGNATURE1(bool,b_MSGFILTER_Sig,TMsgFilter &)
DECLARE_SIGNATURE1(bool,b_PROTECTED_Sig,TEnProtected &)
DECLARE_SIGNATURE1(void,v_REQRESIZE_Sig,TReqResize &)
DECLARE_SIGNATURE1(bool,b_SELCHANGE_Sig,TSelChange &)

//
/// Notification handled by parent with handler expecting TEnDropFiles& and
/// returning a bool.
///                      i.e. bool method(TEnDropFiles& nmHdr)
//
#define EV_EN_DROPFILES(id, method)\
  {{EN_DROPFILES}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_ENDROPFILES_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting TMsgFilter& and
/// returning a bool.
///                      i.e. bool method(TMsgFilter& nmHdr)
//
#define EV_EN_MSGFILTER(id, method)\
  {{EN_MSGFILTER}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_MSGFILTER_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting TEnProtected& and
/// returning a bool.
///                      i.e. bool method(TEnProtected& nmHdr)
//
#define EV_EN_PROTECTED(id, method)\
  {{EN_PROTECTED}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_PROTECTED_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting TReqResize&.
//
///                      i.e. void method(TReqResize& nmHdr)
//
#define EV_EN_REQRESIZE(id, method)\
  {{EN_REQUESTRESIZE}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_REQRESIZE_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting TSelChange& and
/// returning a bool.
///                      i.e. bool method(TSelChange& nmHdr)
//
#define EV_RICHED_EN_SELCHANGE(id, method)\
  {{EN_SELCHANGE}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_SELCHANGE_Sig<TMyClass>(&TMyClass::method)}

/// @}


/// \name UpDown Notification  Messages
/// @{
/// The following Up/Down control macro handles UDN_xxxx notification codes. To
/// determine the name of the notification code that corresponds to the EV_XXXX
/// macro, remove the EV_ prefix.

//
//
//
inline
TNmUpDown::TNmUpDown(HWND hwnd, uint id, uint code, int pos, int delta)
{
  iPos = pos;
  iDelta = delta;
  hdr.hwndFrom = hwnd;
  hdr.idFrom = id;
  hdr.code = code;
}

DECLARE_SIGNATURE1(bool,b_NMUPDOWN_NOTIFY_Sig,TNmUpDown &)

//
/// Notification handled by parent expecting a TNmUpDown notification
//
///      i.e. bool HandleUpDown(TNmUpDown& updownNot);
//
#define EV_UDN_DELTAPOS(id, method) \
  {{UDN_DELTAPOS}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_NMUPDOWN_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

/// @}


/// \name Tool Tip Notification  Messages
/// @{
/// The following tooltip macros handle TTN_xxxx notification codes. To determine
/// the name of the notification code that corresponds to the EV_XXXX macro, remove
/// the EV_ prefix.

DECLARE_SIGNATURE1(void,v_NEEDTEXT_Sig,TTooltipText &)
DECLARE_SIGNATURE1(int,i_NMTTCUSTOMDRAW_Sig,TTtCustomDraw &)
DECLARE_SIGNATURE1(void,v_NMTTDISPINFO_Sig,TTooltipDispInfo &)

/// Notification handled by parent with handler expecting TTooltipText &
/// as parameter.
///                      i.e. void method(TTooltipText & )
//
#define EV_TTN_NEEDTEXT(id, method)  \
  {{TTN_NEEDTEXT}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_NEEDTEXT_Sig<TMyClass>(&TMyClass::method)}

/// Notification handled by parent with handler expecting TNmCustomDraw &
/// as parameter.
///                     i.e. int method(TTtCustomDraw & )
//
#define EV_TTN_CUSTOMDRAW(id, method)  \
  {{NM_CUSTOMDRAW}, id, (::owl::TAnyDispatcher) ::owl::i_LPARAM_Dispatch,\
  (TMyPMF)::owl::i_NMTTCUSTOMDRAW_Sig<TMyClass>(&TMyClass::method)}

//
/// supersedes the TTN_NEEDTEXT notification !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/// Notification handled by parent with handler expecting TTooltipDispInfo &
/// as parameter.
///                      i.e. void method(TTooltipDispInfo & )
//
#define EV_TTN_GETDISPINFO(id, method)  \
  {{TTN_GETDISPINFO}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_NMTTDISPINFO_Sig<TMyClass>(&TMyClass::method)}

#define EV_TTN_POP(id, method)  EV_COMMCTL_NOTIFY_AND_ID_NOTIFY(id, TTN_POP, method)
#define EV_TTN_SHOW(id, method) EV_COMMCTL_NOTIFY_AND_ID_NOTIFY_RETURN(id, TTN_SHOW, method)

/// @}

/// \name Tree View Notification  Messages
/// @{
/// The following tree view macros handle TVN_xxxx notification codes. To determine
/// the name of the notification code that corresponds to the EV_XXXX macro, remove
/// the EV_ prefix.
DECLARE_SIGNATURE1(void,v_TREEWIND_NOTIFY_Sig,TTvNotify &)
DECLARE_SIGNATURE1(bool,b_TREEWIND_NOTIFY_Sig,TTvNotify &)
DECLARE_SIGNATURE1(void,v_TV_KEYDOWN_NOTIFY_Sig,TTvKeyDownNotify &)
DECLARE_SIGNATURE1(bool,b_TV_DISPINFO_NOTIFY_Sig,TTvDispInfoNotify &)
DECLARE_SIGNATURE1(void,v_TV_DISPINFO_NOTIFY_Sig,TTvDispInfoNotify &)
DECLARE_SIGNATURE1(int,i_TV_CUSTOMDRAW_Sig,TTvCustomDraw &) // Version 4.70/4.71
DECLARE_SIGNATURE1(void,v_TV_GETINFOTIP_Sig,TTvGetInfoTip &) // Version 4.70/4.71

#if _WIN32_WINNT >= 0x0600 // _WIN32_WINNT_VISTA

DECLARE_SIGNATURE1(bool, b_NMTVITEMCHANGE_Sig, NMTVITEMCHANGE&)
DECLARE_SIGNATURE1(VOID, v_NMTVSTATEIMAGECHANGING_Sig, NMTVSTATEIMAGECHANGING&)

#endif

//
/// Notification handled by parent with handler expecting TTvNotify&
/// as parameter.        i.e. void method(TTvNotify& nmHdr)
//
#define EV_TREEWIND_NOTIFY(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_TREEWIND_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting TTvNotify& and
/// returning a bool.    i.e. bool method(TTvNotify& nmHdr)
//
#define EV_TREEWIND_NOTIFY_BOOL(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_TREEWIND_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting TTvDispInfoNotify&
/// as parameter.        i.e. void method(TTvDispInfoNotify& nmHdr)
//
#define EV_TV_DISPINFO_NOTIFY(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_TV_DISPINFO_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting TTvDispInfoNotify& and
/// returning a bool.    i.e. bool method(TTvDispInfoNotify& nmHdr)
//
#define EV_TV_DISPINFO_NOTIFY_BOOL(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_TV_DISPINFO_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

#if _WIN32_WINNT >= 0x0600 // _WIN32_WINNT_VISTA

//
/// Notification handled by parent with handler expecting NMTVITEMCHANGE& and
/// returning a bool.    i.e. bool method(NMTVITEMCHANGE& nmHdr)
//
#define EV_TREEWIND_NMTVITEMCHANGE(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_NMTVITEMCHANGE_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting NMTVSTATEIMAGECHANGING&
/// as a parameter.      i.e. void method(NMTVSTATEIMAGECHANGING& nmHdr)
//
#define EV_NM_TVSTATEIMAGECHANGING(id, method)\
  {{NM_TVSTATEIMAGECHANGING}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_NMTVSTATEIMAGECHANGING_Sig<TMyClass>(&TMyClass::method)}

#endif

#define EV_TVN_BEGINDRAG(id, method)      EV_TREEWIND_NOTIFY        (id, TVN_BEGINDRAG, method)
#define EV_TVN_BEGINLABELEDIT(id, method) EV_TV_DISPINFO_NOTIFY_BOOL(id, TVN_BEGINLABELEDIT, method)
#define EV_TVN_BEGINRDRAG(id, method)     EV_TREEWIND_NOTIFY        (id, TVN_BEGINRDRAG, method)
#define EV_TVN_DELETEITEM(id, method)     EV_TREEWIND_NOTIFY        (id, TVN_DELETEITEM, method)
#define EV_TVN_ENDLABELEDIT(id, method)   EV_TV_DISPINFO_NOTIFY     (id, TVN_ENDLABELEDIT, method)
#define EV_TVN_GETDISPINFO(id, method)    EV_TV_DISPINFO_NOTIFY     (id, TVN_GETDISPINFO, method)
#define EV_TVN_ITEMEXPANDED(id, method)   EV_TREEWIND_NOTIFY        (id, TVN_ITEMEXPANDED, method)
#define EV_TVN_ITEMEXPANDING(id, method)  EV_TREEWIND_NOTIFY_BOOL   (id, TVN_ITEMEXPANDING, method)
#define EV_TVN_SELCHANGED(id, method)     EV_TREEWIND_NOTIFY        (id, TVN_SELCHANGED, method)
#define EV_TVN_SELCHANGING(id, method)    EV_TREEWIND_NOTIFY_BOOL   (id, TVN_SELCHANGING, method)
#define EV_TVN_SETDISPINFO(id, method)    EV_TV_DISPINFO_NOTIFY     (id, TVN_SETDISPINFO, method)
#define EV_TVN_KEYDOWN(id, method)\
  {{TVN_KEYDOWN}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_TV_KEYDOWN_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

#if _WIN32_WINNT >= 0x0600 // _WIN32_WINNT_VISTA

#define EV_TVN_ITEMCHANGED(id, method)    EV_TREEWIND_NMTVITEMCHANGE(id, TVN_ITEMCHANGED, method)
#define EV_TVN_ITEMCHANGING(id, method)   EV_TREEWIND_NMTVITEMCHANGE(id, TVN_ITEMCHANGING, method)

#endif

/// Notification handled by parent with handler expecting TTvCustomDraw &
/// as parameter.
///                      i.e. int method(TTvCustomDraw & )
//
#define EV_TVN_CUSTOMDRAW(id, method)  \
  {{NM_CUSTOMDRAW}, id, (::owl::TAnyDispatcher) ::owl::i_LPARAM_Dispatch,\
  (TMyPMF)::owl::i_TV_CUSTOMDRAW_Sig<TMyClass>(&TMyClass::method)}

/// Notification handled by parent with handler expecting TTvGetInfoTip &
/// as parameter.
///                      i.e. void method(TTvGetInfoTip & )
//
#define EV_TVN_GETINFOTIP(id, method)  \
  {{TVN_GETINFOTIP}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_TV_GETINFOTIP_Sig<TMyClass>(&TMyClass::method)}

#define EV_TVN_SINGLEEXPAND(id, method)  EV_TREEWIND_NOTIFY(id, TVN_SINGLEEXPAND, method)
/// @}

//----------------------------------------------------------------------------
// ListWindow control notifications
//
//

DECLARE_SIGNATURE1(void,v_LISTWIND_NOTIFY_Sig,TLvNotify &)
DECLARE_SIGNATURE1(bool,b_LISTWIND_NOTIFY_Sig,TLvNotify &)
DECLARE_SIGNATURE1(void,v_LV_KEYDOWN_NOTIFY_Sig,TLvKeyDownNotify &)
DECLARE_SIGNATURE1(bool,b_LV_DISPINFO_NOTIFY_Sig,TLvDispInfoNotify &)
DECLARE_SIGNATURE1(void,v_LV_DISPINFO_NOTIFY_Sig,TLvDispInfoNotify &)
DECLARE_SIGNATURE1(void,v_LV_GETINFOTIP_Sig,TLvGetInfoTip &) // Version 4.71
DECLARE_SIGNATURE1(bool,b_NMLVCACHEHINT_Sig,TLvCacheHint &)  // Version 4.70
DECLARE_SIGNATURE1(int,i_NMLVFINDITEM_Sig,TLvFindItem &)     // Version 4.70
DECLARE_SIGNATURE1(bool,b_NMLVODSTATECHANGE_Sig,TLvOdStateChanged &) // Version 4.70
DECLARE_SIGNATURE1(int,i_NMLVCUSTOMDRAW_Sig,TLvCustomDraw &) // Version 4.70/4.71



//
/// Notification handled by parent with handler expecting TLvNotify&
/// as parameter.        i.e. void method(TLwNotify& nmHdr)
//
#define EV_LISTWIND_NOTIFY(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_LISTWIND_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting TLvNotify& and
/// returning a bool.
///                      i.e. bool method(TLvNotify& nmHdr)
//
#define EV_LISTWIND_NOTIFY_BOOL(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_LISTWIND_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting TLvDispInfoNotify&
/// as parameter.        i.e. void method(TLvDispInfoNotify& nmHdr)
//
#define EV_LV_DISPINFO_NOTIFY(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_LV_DISPINFO_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

//
/// Notification handled by parent with handler expecting TLvDispInfoNotify& and
/// returning a bool.
//                      i.e. bool method(TLvDispInfoNotify& nmHdr)
//
#define EV_LV_DISPINFO_NOTIFY_BOOL(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_LV_DISPINFO_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

/// \name List View Notification  Messages
/// @{
/// These list view macros handle LVN_xxxx notification codes. To determine the name
/// of the notification code that corresponds to the EV_XXXX macro, remove the EV_
/// prefix.

/// void method(TLvKeyDownNotify & nmHdr)
#define EV_LVN_KEYDOWN(id, method)\
  {{LVN_KEYDOWN}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_LV_KEYDOWN_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

/// void method(TLvGetInfoTip& info)        Version 4.71
#define EV_LVN_GETINFOTIP(id, method)\
  {{LVN_GETINFOTIP}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_LV_GETINFOTIP_Sig<TMyClass>(&TMyClass::method)}

/// bool method(TLvCacheHint& info)        Version 4.70
#define EV_LVN_ODCACHEHINT(id, method)\
  {{LVN_ODCACHEHINT}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_NMLVCACHEHINT_Sig<TMyClass>(&TMyClass::method)}

/// int method(TLvFindItem& info)        Version 4.70
#define EV_LVN_ODFINDITEM(id, method)\
  {{LVN_ODFINDITEM}, id, (::owl::TAnyDispatcher) ::owl::i_LPARAM_Dispatch,\
  (TMyPMF)::owl::i_NMLVFINDITEM_Sig<TMyClass>(&TMyClass::method)}

/// bool method(TLvOdStateChanged& info)        Version 4.70
#define EV_LVN_ODSTATECHANGED(id, method)\
  {{LVN_ODSTATECHANGED}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_NMLVODSTATECHANGE_Sig<TMyClass>(&TMyClass::method)}

/// int method(TLvCustomDraw& info)        Version 4.70/4.71
#define EV_LVN_CUSTOMDRAW(id, method)\
  {{NM_CUSTOMDRAW}, id, (::owl::TAnyDispatcher) ::owl::i_LPARAM_Dispatch,\
  (TMyPMF)::owl::i_NMLVCUSTOMDRAW_Sig<TMyClass>(&TMyClass::method)}


#define EV_LVN_BEGINDRAG(id, method)        EV_LISTWIND_NOTIFY(id, LVN_BEGINDRAG, method)
#define EV_LVN_BEGINLABELEDIT(id, method)   EV_LV_DISPINFO_NOTIFY_BOOL(id, LVN_BEGINLABELEDIT, method)
#define EV_LVN_BEGINRDRAG(id, method)       EV_LISTWIND_NOTIFY(id, LVN_BEGINRDRAG, method)
#define EV_LVN_COLUMNCLICK(id, method)      EV_LISTWIND_NOTIFY(id, LVN_COLUMNCLICK, method)
#define EV_LVN_DELETEALLITEMS(id, method)   EV_LISTWIND_NOTIFY(id, LVN_DELETEALLITEMS, method)
#define EV_LVN_DELETEITEM(id, method)       EV_LISTWIND_NOTIFY(id, LVN_DELETEITEM, method)
#define EV_LVN_SETDISPINFO(id, method)      EV_LV_DISPINFO_NOTIFY(id, LVN_SETDISPINFO, method)

#if 0   // LVN_ENDDRAG is documented but not defined (currently)
#define EV_LVN_ENDDRAG(id, method)          EV_LISTWIND_NOTIFY(id, LVN_ENDDRAG, method)
#endif

#if 0   // LVN_ENDRDRAG is documented but not defined (currently)
#define EV_LVN_ENDRDRAG(id, method)         EV_LISTWIND_NOTIFY(id, LVN_ENDRDRAG, method)
#endif

#define EV_LVN_ENDLABELEDIT(id, method)     EV_LV_DISPINFO_NOTIFY_BOOL(id, LVN_ENDLABELEDIT, method)
#define EV_LVN_GETDISPINFO(id, method)      EV_LV_DISPINFO_NOTIFY(id, LVN_GETDISPINFO, method)
#define EV_LVN_INSERTITEM(id, method)       EV_LISTWIND_NOTIFY(id, LVN_INSERTITEM, method)
#define EV_LVN_ITEMCHANGED(id, method)      EV_LISTWIND_NOTIFY(id, LVN_ITEMCHANGED, method)
#define EV_LVN_ITEMCHANGING(id, method)     EV_LISTWIND_NOTIFY_BOOL(id, LVN_ITEMCHANGING, method)
#define EV_LVN_HOTTRACK(id, method)         EV_LISTWIND_NOTIFY_BOOL(id, LVN_HOTTRACK, method)     // Version 4.71
#define EV_LVN_MARQUEEBEGIN(id, method)     EV_LISTWIND_NOTIFY_BOOL(id, LVN_MARQUEEBEGIN, method) // Version 4.70


// Version 4.70
// if ctrl version > 4.7 -> can be casted to TLvItemActivate
//  bool method(TNotify& nmHdr)
// bool method(TNotify& nmHdr)
// {
//  TLvItemActivate& item = (TLvItemActivate&)nmHdr;
// }
#define EV_LVN_ITEMACTIVATE(id, method)     EV_COMMCTL_NOTIFY_BOOL_AND_NOTIFY(id, LVN_ITEMACTIVATE, method)

/// @}


//
// Rebar notifications
//
//#define RBN_AUTOSIZE           void method(TNotify& not)
#define EV_RBN_AUTOSIZE(id, method)\
  EV_COMMCTL_NOTIFY_AND_NOTIFY(id, RBN_AUTOSIZE, method)
//#define RBN_BEGINDRAG         bool method(TNotify& not)
#define EV_RBN_BEGINDRAG(id, method)\
  EV_COMMCTL_NOTIFY_BOOL_AND_NOTIFY(id, RBN_BEGINDRAG, method)
//#define RBN_CHILDSIZE          void method(TNotify& not)
#define EV_RBN_CHILDSIZE(id, method)\
  EV_COMMCTL_NOTIFY_AND_NOTIFY(id, RBN_CHILDSIZE, method)
//#define RBN_DELETEDBAND        void method(TNotify& not)
#define EV_RBN_DELETEDBAND(id, method)\
  EV_COMMCTL_NOTIFY_AND_NOTIFY(id, RBN_DELETEDBAND, method)
//#define RBN_DELETINGBAND      void method(TNotify& not)
#define EV_RBN_DELETINGBAND(id, method)\
  EV_COMMCTL_NOTIFY_AND_NOTIFY(id, RBN_DELETINGBAND, method)
//#define RBN_ENDDRAG            void method(TNotify& not)
#define EV_RBN_ENDDRAG(id, method)\
  EV_COMMCTL_NOTIFY_AND_NOTIFY(id, RBN_ENDDRAG, method)
//#define RBN_GETOBJECT         bool method(TNotify& not) -> must return true
#define EV_RBN_GETOBJECT(id, method)\
  EV_COMMCTL_NOTIFY_BOOL_AND_NOTIFY(id, RBN_GETOBJECT, method)
//#define RBN_HEIGHTCHANGE      void method(TNotify& not)
#define EV_RBN_HEIGHTCHANGE(id, method)\
  EV_COMMCTL_NOTIFY_AND_NOTIFY(id, RBN_HEIGHTCHANGE, method)
//#define RBN_LAYOUTCHANGED      void method(TNotify& not)
#define EV_RBN_LAYOUTCHANGED(id, method)\
  EV_COMMCTL_NOTIFY_AND_NOTIFY(id, RBN_LAYOUTCHANGED, method)


//
// ComboBoxEx notifications
//
//
DECLARE_SIGNATURE1(bool,b_CBEN_ITEMINFO_NOTIFY_Sig,TCBExItemInfo &)
DECLARE_SIGNATURE1(void,v_CBEN_DRAGBEGIN_NOTIFY_Sig,TCBExDragInfo &)
DECLARE_SIGNATURE1(bool,b_CBEN_ENDEDIT_NOTIFY_Sig,TCBExEditInfo &)
//namespace owl {

//
// bool method(TCBExItemInfo &)
#define EV_CBEN_ITEMINFO_NOTIFY_BOOL(id, notifyCode, method)\
  {{notifyCode}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_LISTWIND_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

// void method(TNotify &)
#define EV_CBEN_BEGINEDIT(id, method)\
  EV_COMMCTL_NOTIFY_AND_NOTIFY(id, CBEN_BEGINEDIT, method)
// bool method(TCBExItemInfo &)
#define EV_CBEN_DELETEITEM(id, method)\
  EV_CBEN_ITEMINFO_NOTIFY_BOOL(id, CBEN_DELETEITEM, method)

// void method(TCBExDragInfo &)
#define EV_CBEN_DRAGBEGIN(id, method)\
  {{CBEN_DRAGBEGIN}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_CBEN_DRAGBEGIN_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

// bool method(TCBExEditInfo &)
#define EV_CBEN_ENDEDIT(id, method)\
  {{CBEN_ENDEDIT}, id, (::owl::TAnyDispatcher) ::owl::B_LPARAM_Dispatch,\
  (TMyPMF)::owl::b_CBEN_ENDEDIT_NOTIFY_Sig<TMyClass>(&TMyClass::method)}

// bool method(TCBExItemInfo &)
#define EV_CBEN_GETDISPINFO(id, method)\
   EV_CBEN_ITEMINFO_NOTIFY_BOOL(id, CBEN_GETDISPINFO, method)
// bool method(TCBExItemInfo &)
#define EV_CBEN_INSERTITEM(id, method)\
   EV_CBEN_ITEMINFO_NOTIFY_BOOL(id, CBEN_INSERTITEM, method)


//
// Month Calendar Control
//
//
//
DECLARE_SIGNATURE1(void,v_NMDAYSTATE_Sig,TNmDayState &)
DECLARE_SIGNATURE1(void,v_NMSELCHANGE_Sig,TNmSelChange &)

// void method(TNmDayState &)
#define EV_MCN_GETDAYSTATE(id, method)\
  {{MCN_GETDAYSTATE}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_NMDAYSTATE_Sig<TMyClass>(&TMyClass::method)}

// void method(TNmSelChange &)
#define EV_MCN_SELCHANGE(id, method)\
  {{MCN_SELCHANGE}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_NMSELCHANGE_Sig<TMyClass>(&TMyClass::method)}

// void method(TNmSelChange &)
#define EV_MCN_SELECT(id, method)\
  {{MCN_SELECT}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_NMSELCHANGE_Sig<TMyClass>(&TMyClass::method)}

//
// Pager Control notifications
//


DECLARE_SIGNATURE1(void,v_NMPGCALCSIZE_Sig,TNmPGCalcSize &)
DECLARE_SIGNATURE1(void,v_NMPGSCROLL_Sig,TNmPGScroll &)


// void method(TNmPGCalcSize &)
#define EV_PGN_CALCSIZE(id, method)\
  {{PGN_CALCSIZE}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_NMPGCALCSIZE_Sig<TMyClass>(&TMyClass::method)}

// void method(TNmPGScroll &)
#define EV_PGN_SCROLL(id, method)\
  {{PGN_SCROLL}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_NMPGSCROLL_Sig<TMyClass>(&TMyClass::method)}


//
// IP Address control notifications
//


DECLARE_SIGNATURE1(void,v_NMIPADDRESS_Sig,TNmIPAddress &)

// void method(TNmIPAddress &)
#define EV_IPN_FIELDCHANGED(id, method)\
  {{IPN_FIELDCHANGED}, id, (::owl::TAnyDispatcher) ::owl::v_LPARAM_Dispatch,\
  (TMyPMF)::owl::v_NMIPADDRESS_Sig<TMyClass>(&TMyClass::method)}


//
// Track Bar notifications
//
//

/// @}

////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Inline implemenations
//

inline uint32 TCommCtrlDll::GetCtrlVersion() 
{
  return Version;
}

//
/// Constructor to create a TNotify object (NMHDR wrapper) from the specified
/// window handle, control id and notification code.
//
inline 
TNotify::TNotify(HWND ctl, uint id, uint ctlCode) {
  hwndFrom = ctl;
  idFrom = id;
  code = ctlCode;
}

//
/// Constructor to create a TNotify object whose members are initialized to
/// zero.
//
inline
TNotify::TNotify() {
  hwndFrom = 0;
  idFrom = 0;
  code = 0;
}

//
/// TTabKeyDown contains information about a key press in a tab control.
/// This constructor initializes the structure NUL value for the
/// virtual wVKey and flags members.
//
inline
TTabKeyDown::TTabKeyDown() {
  wVKey = 0;
  flags = 0;
}

//
/// TTabKeyDown contains information about a key press in a tab control.
/// This constructor initializes the members to the specified parameters.
//
inline
TTabKeyDown::TTabKeyDown(HWND ctl, uint id, uint code, uint16 vk, uint flg)
{
  hdr.hwndFrom = ctl;
  hdr.idFrom = id;
  hdr.code = code;
  wVKey = vk;
  flags = flg;
}


inline void
TRebarInfo::SetImageList(HIMAGELIST Images)
{
  fMask |= RBIM_IMAGELIST;
  himl = Images;
}

//
//
//
inline HIMAGELIST
TRebarInfo::GetImageList()
{
  return himl;
}


// !BB   ostream& _OWLFUNC operator << (ostream&, const TNotify&);
// !BB   istream& _OWLFUNC operator >> (istream&, TNotify&);
// !BB   ostream& _OWLFUNC operator << (ostream&, const THdrNotify&);
// !BB   istream& _OWLFUNC operator >> (istream&, THdrNotify&);


} // OWL namespace


#endif  // OWL_COMMCTRL_H

