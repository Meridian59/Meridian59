/*
 * ole2ui.h
 *
 * Published definitions, structures, types, and function prototypes for the
 * OLE 2.0 User Interface support library.
 *
 */
/*
 *      C/C++ Run Time Library - Version 6.5
 *
 *      Copyright (c) 1994 by Borland International
 *      All Rights Reserved.
 *
 */

#ifndef _OLE2UI_H_
#define _OLE2UI_H_

#ifndef RC_INVOKED
#include <windows.h>
#include <shellapi.h>
#include <ole2.h>
#include <string.h>

#ifdef _TCHAR_DEFINED
#define __TCHAR_DEFINED
#endif
#include <oledlg.h>

#ifndef _BOLESTD_H_
#include "bolestd.h"
#endif
#endif // !RC_INVOKED

#if !defined( EXPORT )
#define EXPORT
#endif  // !EXPORT


extern HINSTANCE  ghInst;

/*
 * Initialization / Uninitialization routines.  OleUIInitialize
 * must be called prior to using any functions in OLE2UI, and OleUIUnInitialize
 * must be called before you app shuts down and when you are done using the
 * library.
 *
 * NOTE:  If you are using the DLL version of this library, these functions
 * are automatically called in the DLL's LibMain and WEP, so you should
 * not call them directly from your application.
 */

#ifndef RC_INVOKED

// Backward compatibility with older library
#define OleUIUninitialize OleUIUnInitialize

STDAPI_(BOOL) OleUIInitialize(HINSTANCE hInstance, HINSTANCE hPrevInst);
STDAPI_(BOOL) OleUIUninitialize(void);

#endif // !RC_INVOKED

extern UINT       cfObjectDescriptor;
extern UINT       cfLinkSrcDescriptor;
extern UINT       cfEmbedSource;
extern UINT       cfEmbeddedObject;
extern UINT       cfLinkSource;
extern UINT       cfOwnerLink;
extern UINT       cfFileName;

#define ID_PU_LINKS       900   // IDC_PU_LINKS

/*************************************************************************
** OLE OBJECT FEEDBACK EFFECTS
*************************************************************************/

#ifndef RC_INVOKED

#define OLEUI_HANDLES_USEINVERSE    0x00000001L
#define OLEUI_HANDLES_NOBORDER      0x00000002L
#define OLEUI_HANDLES_INSIDE        0x00000004L
#define OLEUI_HANDLES_OUTSIDE       0x00000008L


#define OLEUI_SHADE_FULLRECT        1
#define OLEUI_SHADE_BORDERIN        2
#define OLEUI_SHADE_BORDEROUT       3

/* objfdbk.c function prototypes */
STDAPI_(void) OleUIDrawHandles(LPRECT lpRect, HDC hdc, DWORD dwFlags, UINT cSize, BOOL fDraw);
STDAPI_(void) OleUIDrawShading(LPRECT lpRect, HDC hdc, DWORD dwFlags, UINT cWidth);
STDAPI_(void) OleUIShowObject(LPCRECT lprc, HDC hdc, BOOL fIsLink);

//Metafile utility functions
STDAPI_(void)    OleUIMetafilePictIconFree(HGLOBAL);

#endif // !RC_INVOKED

/*************************************************************************
** Hatch window definitions and prototypes                              **
*************************************************************************/

#ifndef RC_INVOKED

#define DEFAULT_HATCHBORDER_WIDTH   4

STDAPI_(BOOL) RegisterHatchWindowClass(HINSTANCE hInst);
STDAPI_(HWND) CreateHatchWindow(HWND hWndParent, HINSTANCE hInst);
STDAPI_(UINT) GetHatchWidth(HWND hWndHatch);
STDAPI_(void) GetHatchRect(HWND hWndHatch, LPRECT lpHatchRect);
STDAPI_(void) SetHatchRect(HWND hWndHatch, LPRECT lprcHatchRect);
STDAPI_(void) SetHatchWindowSize(
      HWND        hWndHatch,
      LPRECT      lprcIPObjRect,
      LPRECT      lprcClipRect,
      LPPOINT     lpptOffset
);


//Hook type used in all structures.
typedef UINT (CALLBACK *LPFNOLEUIHOOK)(HWND, UINT, WPARAM, LPARAM);

//API prototype
STDAPI_(UINT) OleUIInsertControl(LPOLEUIINSERTOBJECT);

#endif // !RC_INVOKED

//Standard error definitions
#define BOLEUI_FALSE                     0
#define BOLEUI_SUCCESS                   1     //No error, same as BOLEUI_OK
#define BOLEUI_OK                        1     //OK button pressed
#define BOLEUI_CANCEL                    2     //Cancel button pressed

#define BOLEUI_ERR_STANDARDMIN           100
#define BOLEUI_ERR_STRUCTURENULL         101   //Standard field validation
#define BOLEUI_ERR_STRUCTUREINVALID      102
#define BOLEUI_ERR_CBSTRUCTINCORRECT     103
#define BOLEUI_ERR_HWNDOWNERINVALID      104
#define BOLEUI_ERR_LPSZCAPTIONINVALID    105
#define BOLEUI_ERR_LPFNHOOKINVALID       106
#define BOLEUI_ERR_HINSTANCEINVALID      107
#define BOLEUI_ERR_LPSZTEMPLATEINVALID   108
#define BOLEUI_ERR_HRESOURCEINVALID      109

#define BOLEUI_ERR_FINDTEMPLATEFAILURE   110   //Initialization errors
#define BOLEUI_ERR_LOADTEMPLATEFAILURE   111
#define BOLEUI_ERR_DIALOGFAILURE         112
#define BOLEUI_ERR_LOCALMEMALLOC         113
#define BOLEUI_ERR_GLOBALMEMALLOC        114
#define BOLEUI_ERR_LOADSTRING            115

#define BOLEUI_ERR_STANDARDMAX           116   //Start here for specific errors.

//Dialog Identifiers as passed in Help messages to identify the source.
#define IDD_INSERTCONTROL       32265

//Help Button Identifier
#define ID_OLEUIHELP                    99
#define IDOK    1
#define IDCANCEL 2


//Insert Object Dialog identifiers
#define ID_IO_OBJECTTYPELIST            2103
#define ID_IO_RESULTTEXT                2109
#define ID_IO_FILETYPE                  2113

//Length of the CLSID string
#define OLEUI_CCHCLSIDSTRING            39

#endif  //_BOLEUI_H_
