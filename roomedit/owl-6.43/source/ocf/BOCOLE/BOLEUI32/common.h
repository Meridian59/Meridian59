/*
 * COMMON.H
 *
 * Structures and definitions applicable to all OLE 2.0 UI dialogs.
 */
/*
 *      C/C++ Run Time Library - Version 6.5
 *
 *      Copyright (c) 1994 by Borland International
// Portions Copyright (c) 2009 OWLNext community
 *      All Rights Reserved.
 *
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include "ansiapi.h"

//Macros to handle control message packing

#ifndef COMMANDPARAMS
#define COMMANDPARAMS(wID, wCode, hWndMsg)                          \
   WORD        wID     = LOWORD(wParam);                           \
   WORD        wCode   = HIWORD(wParam);                           \
   HWND        hWndMsg = (HWND)(UINT)lParam;
#endif  //COMMANDPARAMS

#ifndef SendCommand
#define SendCommand(hWnd, wID, wCode, hControl)                     \
     SendMessage(hWnd, WM_COMMAND, MAKELONG(wID, wCode)      \
          , (LPARAM)hControl)
#endif  //SendCommand

#ifndef SendSetSel
#define SendSetSel(hWnd,nStart,nEnd)                               \
      SendMessage(hWnd, EM_SETSEL, (WPARAM) nStart, (LPARAM) nEnd)
#endif  //SendSetSel





//Property labels used to store dialog structures and fonts
#define STRUCTUREPROP       "Structure"
#define FONTPROP            "Font"


/*
 * Standard structure for all dialogs.  This commonality lets us make
 * a single piece of code that will validate this entire structure and
 * perform any necessary initialization.
 */
typedef struct tagOLEUISTANDARD
   {
   //These IN fields are standard across all OLEUI dialog functions.
   DWORD           cbStruct;       //Structure Size
   DWORD           dwFlags;        //IN-OUT:  Flags
   HWND            hWndOwner;      //Owning window
   LPCOLESTR       lpszCaption;    //Dialog caption bar contents
   LPFNOLEUIHOOK   lpfnHook;       //Hook callback
   LPARAM          lCustData;      //Custom data to pass to hook
   HINSTANCE       hInstance;      //Instance for customized template name
   LPCOLESTR       lpszTemplate;   //Customized template name
   HRSRC           hResource;      //Customized template handle

   DWORD           dwIBApplication;  //Bolero app ptr for help
   HHOOK           hHook;            //Bolero F1 hook
   HTASK           hTask;            //Bolero task for F1 hook
  } OLEUISTANDARD, *POLEUISTANDARD, FAR *LPOLEUISTANDARD;


//Function prototypes
//COMMON.C
UINT  WINAPI  UStandardValidation(const LPOLEUISTANDARD, const UINT, const HGLOBAL FAR *);
UINT  WINAPI  UStandardInvocation(DLGPROC, LPOLEUISTANDARD, HGLOBAL, LPCSTR);
LPVOID WINAPI LpvStandardInit(HWND, UINT, BOOL, HFONT FAR *);
LPVOID WINAPI LpvStandardEntry(HWND, UINT, WPARAM, LPARAM, UINT FAR *);
UINT WINAPI   UStandardHook(LPVOID, HWND, UINT, WPARAM, LPARAM);
void WINAPI   StandardCleanup(LPVOID, HWND);
void WINAPI   StandardShowDlgItem(HWND hDlg, int idControl, int nCmdShow);

//DRAWICON.C

//Structure for label and source extraction from a metafile
typedef struct tagLABELEXTRACT
   {
   LPOLESTR    lpsz;
   UINT        Index;      // index in lpsz (so we can retrieve 2+ lines)
   DWORD       PrevIndex;  // index of last line (so we can mimic word wrap)

   union
      {
      UINT    cch;        //Length of label for label extraction
      UINT    iIcon;      //Index of icon in source extraction.
      } u;

   //For internal use in enum procs
   BOOL        fFoundIconOnly;
   BOOL        fFoundSource;
   BOOL        fFoundIndex;
   } LABELEXTRACT, FAR * LPLABELEXTRACT;


//Structure for extracting icons from a metafile (CreateIcon parameters)
typedef struct tagICONEXTRACT
   {
   HICON       hIcon;          //Icon created in the enumeration proc.

   /*
    * Since we want to handle multitasking well we have the caller
    * of the enumeration proc instantiate these variables instead of
    * using statics in the enum proc (which would be bad).
    */
   BOOL        fAND;
   HGLOBAL     hMemAND;        //Enumeration proc allocates and copies
   } ICONEXTRACT, FAR * LPICONEXTRACT;


//Structure to use to pass info to EnumMetafileDraw
typedef struct tagDRAWINFO
   {
   RECT     Rect;
   BOOL     fIconOnly;
   } DRAWINFO, FAR * LPDRAWINFO;


int CALLBACK EXPORT EnumMetafileIconDraw(HDC, HANDLETABLE FAR *, METARECORD FAR *, int, LPARAM);
int CALLBACK EXPORT EnumMetafileExtractLabel(HDC, HANDLETABLE FAR *, METARECORD FAR *, int, LPLABELEXTRACT);
int CALLBACK EXPORT EnumMetafileExtractIcon(HDC, HANDLETABLE FAR *, METARECORD FAR *, int, LPICONEXTRACT);
int CALLBACK EXPORT EnumMetafileExtractIconSource(HDC, HANDLETABLE FAR *, METARECORD FAR *, int, LPLABELEXTRACT);


LPSTR FAR PASCAL PointerToNthField(LPSTR, int, char);

//Shared globals:  our instance, registered messages used from all dialogs and clipboard
// formats used by the PasteSpecial dialog
extern HINSTANCE  ghInst;

extern UINT       xMsgEndDialog;

// extern UINT       uMsgHelp;
// extern UINT       uMsgBrowse;
// extern UINT       uMsgChangeIcon;
// extern UINT       uMsgFileOKString;
// extern UINT       uMsgCloseBusyDlg;

// extern UINT       cfObjectDescriptor;
// extern UINT       cfLinkSrcDescriptor;
// extern UINT       cfEmbedSource;
// extern UINT       cfEmbeddedObject;
// extern UINT       cfLinkSource;
// extern UINT       cfOwnerLink;
// extern UINT       cfFileName;

//Standard control identifiers
#define ID_NULL                         98

#endif //_COMMON_H_
