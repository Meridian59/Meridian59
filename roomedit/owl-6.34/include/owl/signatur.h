//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
// Copyright (c) 1998 by Yura Bidus
//
/// \file
/// Message handler signature templates
//----------------------------------------------------------------------------

#if !defined(OWL_SIGNATUR_H)
#define OWL_SIGNATUR_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif



namespace owl {

class _OWLCLASS TCommandEnabler;
class _OWLCLASS TDocument;
class _OWLCLASS TView;
class _OWLCLASS TDockingSlip;
class _OWLCLASS TWindow;
class _OWLCLASS TPoint;
class _OWLCLASS TDropInfo;
class _OWLCLASS TSize;
class _OWLCLASS TRect;

struct _OWLCLASS THelpHitInfo;



////////////////////////////////////////////////////////////////////////////
//  These macros are used for declaring signature templates for OWL dispatch
//  tables.  The signatures are used to ensure that methods provided to the
//  macros are of the proper form (ie, signature).  Eg:
//
//      DECLARE_DISPSIGNATURE1 (void, v_EnterPage_Sig, TEnterPageNotify &)
//
//  This creates a signature verification template called v_EnterPage_Sig
//  that is used like this:
//
//      #define  EV_TABN_ENTERPAGE(id,method)
//                  { nmEnterPage, id, (::owl::TAnyDispatcher)::owl::v_LPARAM_Dispatch,
//                    (TMyPMF) v_EnterPage_Sig (&TMyClass::method) }
//
//  All this will simply guarantee that "method" is of the form:
//
//          void Method (TEnterPageNotify &);
//
//  The number on the end of DECLARE_DISPSIGNATUREx is either 0, 1, 2, 3 or
//  4, depending on the number of arguments the method takes.
//
#define DECLARE_SIGNATURE0(R,Name) \
    template <class T> \
    inline R  (T::*Name(R  (T::*pmf)()))() \
        { return pmf; }

#define DECLARE_SIGNATURE1(R,Name,P1) \
    template <class T> \
    inline R  (T::*Name(R  (T::*pmf)(P1)))(P1) \
        { return pmf; }

#define DECLARE_SIGNATURE2(R,Name,P1,P2) \
    template <class T> \
    inline R  (T::*Name(R  (T::*pmf)(P1,P2)))(P1,P2) \
        { return pmf; }

#define DECLARE_SIGNATURE3(R,Name,P1,P2,P3) \
    template <class T> \
    inline R  (T::*Name(R  (T::*pmf)(P1,P2,P3)))(P1,P2,P3) \
        { return pmf; }

#define DECLARE_SIGNATURE4(R,Name,P1,P2,P3,P4) \
    template <class T> \
    inline R  (T::*Name(R  (T::*pmf)(P1,P2,P3,P4)))(P1,P2,P3,P4) \
        { return pmf; }

#define DECLARE_SIGNATURE5(R,Name,P1,P2,P3,P4,P5) \
    template <class T> \
    inline R  (T::*Name(R  (T::*pmf)(P1,P2,P3,P4,P5)))\
                                                      (P1,P2,P3,P4,P5) \
        { return pmf; }

#define DECLARE_SIGNATURE6(R,Name,P1,P2,P3,P4,P5,P6) \
    template <class T> \
    inline R  (T::*Name(R  (T::*pmf)(P1,P2,P3,P4,P5,P6)))\
                                                      (P1,P2,P3,P4,P5,P6) \
        { return pmf; }

#define DECLARE_SIGNATURE7(R,Name,P1,P2,P3,P4,P5,P6,P8) \
    template <class T> \
    inline R  (T::*Name(R  (T::*pmf)(P1,P2,P3,P4,P5,P6,P8)))\
                                                      (P1,P2,P3,P4,P5,P6,P8) \
        { return pmf; }

#define DECLARE_SIGNATURE9(R,Name,P1,P2,P3,P4,P5,P6,P8,P9) \
    template <class T> \
    inline R  (T::*Name(R  (T::*pmf)(P1,P2,P3,P4,P5,P6,P8,P9)))\
                                                      (P1,P2,P3,P4,P5,P6,P8,P9) \
        { return pmf; }

#define DECLARE_SIGNATURE10(R,Name,P1,P2,P3,P4,P5,P6,P8,P9,P10) \
    template <class T> \
    inline R  (T::*Name(R  (T::*pmf)\
                  (P1,P2,P3,P4,P5,P6,P8,P9,P10)))(P1,P2,P3,P4,P5,P6,P8,P9,P10) \
        { return pmf; }

#define DECLARE_SIGNATURE11(R,Name,P1,P2,P3,P4,P5,P6,P8,P9,P10,P11) \
    template <class T> \
    inline R  (T::*Name(R  (T::*pmf)\
         (P1,P2,P3,P4,P5,P6,P8,P9,PP10,P11)))(P1,P2,P3,P4,P5,P6,P8,P9,P10,P11) \
        { return pmf; }

#define DECLARE_SIGNATURE12(R,Name,P1,P2,P3,P4,P5,P6,P8,P9,P10,P11,P12) \
    template <class T> \
    inline R  (T::*Name(R  (T::*pmf)\
        (P1,P2,P3,P4,P5,P6,P8,P9,P10,P11,P12)))(P1,P2,P3,P4,P5,P6,P8,P9,P10,P11,P12)\
        { return pmf; }

//----------------------------------------------------------------------------
//  LEGEND: in order to keep signature names from getting too long, the
//          following abbreviations are used for the data types passed to and
//          returned from the methods. The first code is the return type, the
//          rest are args in left to right order.
//
//  - v   (void return)
//  - i,I (int)
//  - I32 (int32)
//  - U   (uint)
//  - B   (bool)
//  - H   (HANDLE)
//  - W   (HWND)
//  - S   (char *)
//  - CS  (const char *)
//  - other types spelled out
//

//----------------------------------------------------------------------------

DECLARE_SIGNATURE0(bool,B_Sig)
DECLARE_SIGNATURE1(bool,B_B_Sig,bool)
DECLARE_SIGNATURE1(bool,B_I_Sig,int)
DECLARE_SIGNATURE1(bool,B_U_Sig,uint)
DECLARE_SIGNATURE1(bool,B_CREATE_Sig,CREATESTRUCT &)
DECLARE_SIGNATURE1(bool,B_CS_Sig,LPCTSTR)
DECLARE_SIGNATURE1(bool,B_MSG_Sig,MSG *)
DECLARE_SIGNATURE1(bool,B_RECT_Sig,TRect *)
DECLARE_SIGNATURE1(bool,B_HDC_Sig,HDC)
DECLARE_SIGNATURE2(bool,B_U_RECT_Sig,uint,TRect&)
DECLARE_SIGNATURE2(bool,B_U_U_Sig,uint,uint)
DECLARE_SIGNATURE2(bool,B_U_LPARAM_Sig,uint,LPARAM)
DECLARE_SIGNATURE3(bool,B_W_U_U_Sig,HWND,uint,uint)
DECLARE_SIGNATURE3(bool,B_U_U_POINT_Sig,uint,uint,const TPoint&)
DECLARE_SIGNATURE3(bool,B_U_i_POINT_Sig,uint,int,const TPoint&)

DECLARE_SIGNATURE2(bool,B_B_HKL_Sig,bool,HKL)
DECLARE_SIGNATURE2(bool,B_I_HKL_Sig,int,HKL)

DECLARE_SIGNATURE1(void,v_HRGN_Sig,HRGN)
DECLARE_SIGNATURE3(void,v_B_I_I_Sig,bool,int,int)
DECLARE_SIGNATURE1(bool,B_I32_Sig,int32)
DECLARE_SIGNATURE1(bool,B_PPALETTE_Sig,LOGPALETTE * *)

//----------------------------------------------------------------------------
DECLARE_SIGNATURE0(LPCTSTR,CS_Sig)
DECLARE_SIGNATURE0(HANDLE,H_Sig)
DECLARE_SIGNATURE0(HFONT,HFONT_Sig)
DECLARE_SIGNATURE1(HICON,HICON_B_Sig,bool)
DECLARE_SIGNATURE1(int,i_CREATE_Sig,CREATESTRUCT &)
DECLARE_SIGNATURE1(int,i_U_Sig,uint)
DECLARE_SIGNATURE2(HICON,HICON_B_HICON_Sig,bool,HICON)
DECLARE_SIGNATURE3(HBRUSH,HBRUSH_HDC_W_U_Sig,HDC,HWND,uint)
DECLARE_SIGNATURE3(int,i_U_W_U_Sig,uint,HWND,uint)
//----------------------------------------------------------------------------

DECLARE_SIGNATURE1(LRESULT,LRESULT_MDICREATESTRUCT_Sig,MDICREATESTRUCT &)
DECLARE_SIGNATURE2(int,int_U_COMPAREITEM_Sig,uint,const COMPAREITEMSTRUCT&)
DECLARE_SIGNATURE2(int32,I32_U_I32_Sig,uint,int32)
DECLARE_SIGNATURE2(LRESULT,LRESULT_WPARAM_LPARAM_Sig,WPARAM,LPARAM) //Used for EV_MESSAGE
DECLARE_SIGNATURE3(LRESULT,LRESULT_U_U_HMENU_Sig,uint,uint,HMENU)

//----------------------------------------------------------------------------

DECLARE_SIGNATURE0(uint32,U32_Sig)
DECLARE_SIGNATURE1(uint32,U32_VOIDFP_Sig,void *)

//----------------------------------------------------------------------------

DECLARE_SIGNATURE0(uint,U_Sig)
DECLARE_SIGNATURE2(uint,U_B_CALCSIZE_Sig,bool,NCCALCSIZE_PARAMS &)
DECLARE_SIGNATURE1(uint,U_MSG_Sig,const MSG *)
DECLARE_SIGNATURE1(uint,U_POINT_Sig,const TPoint&)
DECLARE_SIGNATURE3(uint,U_W_U_U_Sig,HWND,uint,uint)

//----------------------------------------------------------------------------

DECLARE_SIGNATURE0(void,v_Sig)
DECLARE_SIGNATURE1(void,v_B_Sig,bool)
DECLARE_SIGNATURE1(void,v_COMMANDENABLER_Sig,TCommandEnabler&)
DECLARE_SIGNATURE1(void,v_CS_Sig,LPCTSTR)
DECLARE_SIGNATURE1(void,v_DROP_Sig,TDropInfo)
DECLARE_SIGNATURE1(void,v_H_Sig,HANDLE)
DECLARE_SIGNATURE1(void,v_HDC_Sig,HDC)
DECLARE_SIGNATURE2(void,v_B_B_Sig,bool,bool)
DECLARE_SIGNATURE2(void,v_B_HTASK_Sig,bool,HTASK)
DECLARE_SIGNATURE2(void,v_B_U_Sig,bool,uint)
DECLARE_SIGNATURE2(void,v_H_B_Sig,HANDLE,bool)
DECLARE_SIGNATURE2(void,v_HDC_U_Sig,HDC,uint)
DECLARE_SIGNATURE3(void,v_B_W_W_Sig,bool,HWND,HWND)

DECLARE_SIGNATURE1(void,v_HELPINFO_Sig,const HELPINFO&)

DECLARE_SIGNATURE2(bool,b_HWND_COPYDATASTRUCT_Sig,HWND,const COPYDATASTRUCT&)

DECLARE_SIGNATURE2(void,v_HFONT_B_Sig,HFONT,bool)
DECLARE_SIGNATURE1(void,v_HMENU_Sig,HMENU)
DECLARE_SIGNATURE3(void,v_HMENU_U_B_Sig,HMENU,uint,bool)
DECLARE_SIGNATURE1(void,v_i_Sig,int)
DECLARE_SIGNATURE1(void,v_MINMAXINFO_Sig,MINMAXINFO &)
DECLARE_SIGNATURE1(void,v_OWLDOCUMENT_Sig,TDocument&)
DECLARE_SIGNATURE1(void,v_OWLVIEW_Sig,TView&)
DECLARE_SIGNATURE2(void,v_OWLWINDOWDOCKED_Sig,uint,const TDockingSlip&)
DECLARE_SIGNATURE1(void,v_OWLHELPHIT_Sig,const THelpHitInfo&)
DECLARE_SIGNATURE1(void,v_POINT_Sig,const TPoint&)
DECLARE_SIGNATURE1(void,v_S_Sig,LPTSTR)
DECLARE_SIGNATURE1(void,v_U_Sig,uint)
DECLARE_SIGNATURE3(void,v_U_B_W_Sig,uint,bool,HWND)
DECLARE_SIGNATURE2(void,v_U_CS_Sig,uint,LPCTSTR)
DECLARE_SIGNATURE2(void,v_U_DELETEITEM_Sig,uint,const DELETEITEMSTRUCT &)
DECLARE_SIGNATURE2(void,v_U_DRAWITEM_Sig,uint,const DRAWITEMSTRUCT &)
DECLARE_SIGNATURE2(void,v_U_MEASUREITEM_Sig,uint,MEASUREITEMSTRUCT &)
DECLARE_SIGNATURE2(void,v_U_POINT_Sig,uint,const TPoint&)
DECLARE_SIGNATURE2(void,v_U_S_Sig,uint,LPTSTR)
DECLARE_SIGNATURE2(void,v_U_SIZE_Sig,uint,const TSize&)
DECLARE_SIGNATURE2(void,v_U_STYLE_Sig,uint,STYLESTRUCT&)
DECLARE_SIGNATURE2(void,v_U_CSTYLE_Sig,uint,const STYLESTRUCT&)
DECLARE_SIGNATURE2(void,v_U_U_Sig,uint,uint)
DECLARE_SIGNATURE2(void,v_WPARAM_U_Sig,WPARAM,uint)
DECLARE_SIGNATURE3(void,v_U_U_HMENU_Sig,uint,uint,HMENU)
DECLARE_SIGNATURE3(void,v_U_U_U_Sig,uint,uint,uint)
DECLARE_SIGNATURE3(void,v_U_U_W_Sig,uint,uint,HWND)
DECLARE_SIGNATURE2(void,v_U_W_Sig,uint,HWND)
DECLARE_SIGNATURE1(void,v_W_Sig,HWND)
DECLARE_SIGNATURE2(void,v_W_H_Sig,HWND,HANDLE)
DECLARE_SIGNATURE3(void,v_W_I_I_Sig,HWND,int,int)
DECLARE_SIGNATURE3(void,v_W_U_U_Sig,HWND,uint,uint)
DECLARE_SIGNATURE2(void,v_W_W_Sig,HWND,HWND)
DECLARE_SIGNATURE1(void,v_WINDOWPOS_Sig,WINDOWPOS &)
DECLARE_SIGNATURE1(void,v_CWINDOWPOS_Sig,const WINDOWPOS&)
DECLARE_SIGNATURE3(void,v_ParentNotify_Sig,uint,WPARAM,LPARAM)

} // OWL namespace

#endif  // OWL_SIGNATUR_H
