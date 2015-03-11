//
/// \file
/// Legacy support for Borland Windows Custom Controls  (BWCC)
//
// Part of OWLNext - the next generation Object Windows Library 
// Copyright (c) 1995, 1996 by Borland International, All Rights Reserved
//
// For more information, including license details, see 
// http://owlnext.sourceforge.net
//

// Borland Windows Custom Controls 
// (C) Copyright 1991-1998 by Borland International



#if !defined(__BWCC_H)
#define __BWCC_H

#if defined(OWL_SUPPORT_BWCC)

#ifndef RC_INVOKED
#pragma warn -nak   /* Ignore non-ansi keywords */
#endif

#if !defined(WORKSHOP_INVOKED)

#if !defined(__WINDOWS_H)
#include <windows.h>

#endif

#endif

#define BWCCVERSION 0x0200   // version 2.00

// from version 1.02 onward BWCCGetversion returns a DWORD
// The low-order word contains the version number
// and the high-order word contains the locale

#define BWCC_LOCALE_US     1
#define BWCC_LOCALE_JAPAN  2


#define BORDLGPROP  "FB"      // Borland dialog window uses
                              // this property for instance data
                              // users should not use a property
                              // with this name!

#if  !defined( IDHELP)

#define IDHELP    998         // Id of help button

#endif

#define BORDLGCLASS   "BorDlg"      // Our Custom Dialog class
#define BORGRAYCLASS  "BorDlg_Gray" // Our Custom Dialog class - gray background
#define BUTTON_CLASS  "BorBtn"    // Our Bitmap Buttons
#define RADIO_CLASS   "BorRadio"  // Our Radio Buttons
#define CHECK_CLASS   "BorCheck"  // Our Check Boxes
#define STATIC_CLASS  "BorStatic" // Our statics
#define SHADE_CLASS   "BorShade"  // Our Group Boxes and Dividers

#if !defined(EXPORT)
#define EXPORT __declspec(dllexport)
#endif

#define BWCCAPI EXPORT APIENTRY

// button style definitions:

// the Borland buttons use Windows button styles for button
// type: i.e. BS_PUSHBUTTON/BS_DEFPUSHBUTTON

// styles

#define BBS_BITMAP       0x8000L  // this is a bitmap static
#define BBS_PARENTNOTIFY 0x2000L  // Notify parent of TAB keys and focus
#define BBS_OWNERDRAW    0x1000L  // let parent paint via WM_DRAWITEM

// messages

#define BBM_SETBITS       ( BM_SETSTYLE + 10)

// notifications

#define BBN_SETFOCUS      ( BN_DOUBLECLICKED + 10)
#define BBN_SETFOCUSMOUSE ( BN_DOUBLECLICKED + 11)
#define BBN_GOTATAB       ( BN_DOUBLECLICKED + 12)
#define BBN_GOTABTAB      ( BN_DOUBLECLICKED + 13)
#define BBN_MOUSEENTER    ( BN_DOUBLECLICKED + 14)
#define BBN_MOUSELEAVE    ( BN_DOUBLECLICKED + 14)


// The following is the name of the window message passed to
// RegisterWindowMessage for CtlColor processing for group box shades:
#define BWCC_CtlColor_Shade "BWCC_CtlColor_Shade"

#define BSS_GROUP     1L  // recessed group box
#define BSS_HDIP      2L  // horizontal border
#define BSS_VDIP      3L  // vertical border
#define BSS_HBUMP     4L  // horizontal speed bump
#define BSS_VBUMP     5L  // vertical speed bump
#define BSS_RGROUP    6L  // raised group box

#define BSS_CAPTION   0x8000L // Set off the caption
#define BSS_CTLCOLOR  0x4000L // Send WM_CTLCOLOR messages to parent of control
#define BSS_NOPREFIX  0x2000L // & in caption does not underline following letter
#define BSS_LEFT      0x0000L // Caption is left-justified
#define BSS_CENTER    0x0100L // Caption is centered
#define BSS_RIGHT     0x0200L // Caption is right-justified
#define BSS_ALIGNMASK 0x0300L


#if defined( __cplusplus )
extern "C" {
#endif  /* __cplusplus */

extern HGLOBAL BWCCAPI SpecialLoadDialog
(

  HINSTANCE   hResMod,
  LPCSTR      lpResName,
  DLGPROC     fpDlgProc
);

extern HGLOBAL BWCCAPI MangleDialog
(
  HGLOBAL     hDlg,
  HINSTANCE   hResources,
  DLGPROC     fpDialogProc
);

extern LRESULT BWCCAPI BWCCDefDlgProc
(
  HWND        hWnd,
  UINT        message,
  WPARAM      wParam,
  LPARAM      lParam
);
extern LRESULT BWCCAPI BWCCDefGrayDlgProc
(
  HWND        hWnd,
  UINT        message,
  WPARAM      wParam,
  LPARAM      lParam
);


extern LRESULT BWCCAPI BWCCDefWindowProc
(
  HWND        hWnd,
  UINT        message,
  WPARAM      wParam,
  LPARAM      lParam
);

extern LRESULT BWCCAPI BWCCDefMDIChildProc
(
  HWND        hWnd,
  UINT        message,
  WPARAM      wParam,
  LPARAM      lParam
);

extern int BWCCAPI BWCCMessageBox
(
  HWND        hWndParent,
  LPCSTR      lpText,
  LPCSTR      lpCaption,
  UINT        wType
);


extern HBRUSH BWCCAPI BWCCGetPattern( void );

extern DWORD BWCCAPI BWCCGetVersion( void);

extern BOOL BWCCAPI BWCCIntlInit (UINT language);

extern BOOL BWCCAPI BWCCIntlTerm ( VOID );

extern BOOL BWCCAPI BWCCRegister(HINSTANCE hInsta);

#if defined( __cplusplus )
}
#endif  /* __cplusplus */

#ifndef RC_INVOKED
#pragma warn .nak   /* Ignore non-ansi keywords */
#endif

#else //#if defined(OWL_SUPPORT_BWCC)

#error bwcc.h should be include only when OWL_SUPPORT_BWCC is defined

#endif //#if defined(OWL_SUPPORT_BWCC)

#endif  /* __BWCC_H */
