//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Dispatch functions (crackers) to crack a Windows message and pass control
/// to a member function via a pointer (pmf).
//----------------------------------------------------------------------------

#if !defined(OWL_DISPATCH_H)
#define OWL_DISPATCH_H

#include <owl/private/defs.h>
#if defined(BI_HAS_PRAGMA_ONCE)
# pragma once
#endif
#if defined(BI_COMP_WATCOM)
# pragma read_only_file
#endif

#include <owl/defs.h>

namespace owl {

class _OWLCLASS TPoint;
class _OWLCLASS TRect;
class _OWLCLASS TSize;

//
/// \class TGeneric
// ~~~~~ ~~~~~~~
/// Generic class for casting pointer to objects and pointer to member functions
/// Class is not actually defined or implemented anywhere
//

/// \cond

class TGeneric;

#if defined(BI_COMP_MSC)
# pragma pointers_to_members(full_generality, virtual_inheritance)
#endif

/// \endcond

//
/// \typedef TAnyPMF
// ~~~~~~~ ~~~~~~~
/// generic pointer to member function
//
typedef void (TGeneric::*TAnyPMF)();

//
/// \typedef TAnyDispatcher
// ~~~~~~~ ~~~~~~~~~~~~~~
/// all message dispatcher functions take four parameters:
/// - reference to an object
/// - pointer to member function (signature varies according to the cracking
///   that the function performs)
/// - \c \b  wParam
/// - \c \b  lParam
//
typedef _OWLFUNC(LRESULT) (*TAnyDispatcher)(TGeneric&, TAnyPMF, WPARAM, LPARAM);



//
// LEGEND: in order to keep dispatcher names from getting too long, the
//         following abbreviations are used. The names are based on the data
//         sizes passed & returned, & which param they come from.
//
// - v   (void return)
// - i   (int)
// - U   (uint)
// - H   (HANDLE) (requires special cracking, uint size)
// - W   (HWND)
// - I32 (int32)
// - U32 (uint32)
// - SIZE (const TSize&) (TSize object constructed)
// - POINT (const TPoint&) (TPoint object constructed)
// - POINTER (void*) (model ambient size)
//
// Possible future cracker encoding
//  Which param:
//  - 1  wParam
//  - 2  lParam
//    How cracked (default to no processing, size of param):
//    - U Ambient size uint (squashed lParam in 16bit land)
//    - L Low 16bit word
//    - H high 16bit word
//
// Custom message crackers are named based on the message they crack
//

//----------------------------------------------------------------------------

//
/// passes lParam as an LPARAM and returns an int result
//
_OWLFUNC(LRESULT)
i_LPARAM_Dispatch(TGeneric&, int (TGeneric::*)(LPARAM), WPARAM, LPARAM);

//
/// passes wParam as a WPARAM and returns an int result
//
_OWLFUNC(LRESULT)
i_WPARAM_Dispatch(TGeneric&, int (TGeneric::*)(WPARAM), WPARAM, LPARAM);

//
/// passes wParam as a WPARAM and returns a bool result
//
_OWLFUNC(LRESULT)
B_WPARAM_Dispatch(TGeneric&, bool (TGeneric::*)(WPARAM), WPARAM, LPARAM);

//----------------------------------------------------------------------------

//
/// passes nothing and returns an LRESULT result
//
_OWLFUNC(LRESULT)
LRESULT_Dispatch(TGeneric&, LRESULT (TGeneric::*)(), WPARAM, LPARAM);

//
/// passes lParam as an LPARAM and returns an LRESULT result
//
_OWLFUNC(LRESULT)
LRESULT_LPARAM_Dispatch(TGeneric&, LRESULT (TGeneric::*)(LPARAM), WPARAM, LPARAM);

//
/// passes lParam as a uint and returns an LRESULT result
//
_OWLFUNC(LRESULT)
LRESULT_U_Dispatch(TGeneric&, LRESULT (TGeneric::*)(uint), WPARAM, LPARAM);

//
/// passes wParam as a WPARAM and lParam as an LPARAM and returns an LRESULT result
//
_OWLFUNC(LRESULT)
LRESULT_WPARAM_LPARAM_Dispatch(TGeneric&, LRESULT (TGeneric::*)(WPARAM, LPARAM), WPARAM, LPARAM);

//----------------------------------------------------------------------------

//
/// passes no arguments and returns a uint result
//
_OWLFUNC(LRESULT)
U_Dispatch(TGeneric&, uint (TGeneric::*)(), WPARAM, LPARAM);
//
/// passes lParam as an LPARAM and returns a bool result
//
_OWLFUNC(LRESULT)
B_LPARAM_Dispatch(TGeneric&, bool (TGeneric::*)(LPARAM), WPARAM, LPARAM);

//
/// passes no arguments and returns a bool result
//
_OWLFUNC(LRESULT)
B_Dispatch(TGeneric&, bool (TGeneric::*)(), WPARAM, LPARAM);

//
/// passes lParam as an LPARAM and returns a uint result
//
_OWLFUNC(LRESULT)
U_LPARAM_Dispatch(TGeneric&, uint (TGeneric::*)(LPARAM), WPARAM, LPARAM);

//
/// passes nothing and returns a uint32 result
//
_OWLFUNC(LRESULT)
U32_Dispatch(TGeneric&, uint32 (TGeneric::*)(), WPARAM, LPARAM);

//
/// passes lParam as a const TPoint& and returns a uint result
//
_OWLFUNC(LRESULT)
U_POINT_Dispatch(TGeneric&, uint (TGeneric::*)(const owl::TPoint&), WPARAM, LPARAM);

//
/// passes lParam as a void* and returns a uint result
//
_OWLFUNC(LRESULT)
U_POINTER_Dispatch(TGeneric&, uint (TGeneric::*)(void*), WPARAM, LPARAM);

//
/// passes lParam as a void* and returns a bool result
//
_OWLFUNC(LRESULT)
B_POINTER_Dispatch(TGeneric&, bool (TGeneric::*)(void*), WPARAM, LPARAM);

//
/// passes wParam as a uint and returns a uint result
//
_OWLFUNC(LRESULT)
U_U_Dispatch(TGeneric&, uint (TGeneric::*)(uint), WPARAM, LPARAM);

//
/// passes wParam as a bool and returns a bool result
//
_OWLFUNC(LRESULT)
B_B_Dispatch(TGeneric&, bool (TGeneric::*)(bool), WPARAM, LPARAM);

//
/// passes param2 as an int and returns bool
//
_OWLFUNC(LRESULT)
B_I2_Dispatch(TGeneric&, bool (TGeneric::*)(int), WPARAM, LPARAM);

//
/// passes wParam as a bool and returns a uint result
//
_OWLFUNC(LRESULT)
U_B_Dispatch(TGeneric&, uint (TGeneric::*)(bool), WPARAM, LPARAM);

//
/// passes wParam as bool, lParam as a uint and returns a bool result
//
_OWLFUNC(LRESULT)
B_B_U_Dispatch(TGeneric&, bool (TGeneric::*)(bool, uint), WPARAM, LPARAM);

//
/// passes wParam as a uint, lParam as a uint and returns a uint
//
_OWLFUNC(LRESULT)
U_U_U_Dispatch(TGeneric&, uint (TGeneric::*)(uint, uint), WPARAM, LPARAM);

//
/// passes wParam as a bool, lParam as a uint and returns a uint
//
_OWLFUNC(LRESULT)
U_B_U_Dispatch(TGeneric&, uint (TGeneric::*)(bool, uint), WPARAM, LPARAM);

//
/// passes wParam as a uint, lParam as a uint and returns a bool
//
_OWLFUNC(LRESULT)
B_U_U_Dispatch(TGeneric&, bool (TGeneric::*)(uint, uint), WPARAM, LPARAM);

//
/// passes wParam as a uint, lParam as a reference to a rect, copying to a
/// temp rect & back.  returns uint
//
_OWLFUNC(LRESULT)
U_U_RECT_Dispatch(TGeneric&, uint (TGeneric::*)(uint, const owl::TRect&), WPARAM, LPARAM);

//
/// passes wParam as a uint, lParam as a reference to a rect, copying to a
/// temp rect & back.  returns bool
//
_OWLFUNC(LRESULT)
B_U_RECT_Dispatch(TGeneric&, bool (TGeneric::*)(uint, const owl::TRect&), WPARAM, LPARAM);

//
/// passes wParam.lo as a uint and wParam.hi as uint, lParam as a reference to a point, 
/// copying to a temp point & back.  returns bool
//
_OWLFUNC(LRESULT)
B_U_U_POINT_Dispatch(TGeneric&, bool (TGeneric::*)(uint, uint, const owl::TPoint&), WPARAM, LPARAM);

//
/// passes wParam.lo as a uint and wParam.hi as int, lParam as a reference to a point, 
/// copying to a temp point & back.  returns bool
//
_OWLFUNC(LRESULT)
B_U_i_POINT_Dispatch(TGeneric&, bool (TGeneric::*)(uint, int, const owl::TPoint&), WPARAM, LPARAM);

//
/// passes wParam as a uint, lParam.lo as a uint, and lParam.hi as a uint, and returns uint.
//
_OWLFUNC(LRESULT)
U_U_U_U_Dispatch(TGeneric&, uint (TGeneric::*)(uint, uint, uint), WPARAM, LPARAM);

//
/// passes wParam as an HWND, lParam.lo as a uint, and lParam.hi as a uint, and returns uint.
//
_OWLFUNC(LRESULT)
U_W_U_U_Dispatch(TGeneric&, uint (TGeneric::*)(HWND, uint, uint), WPARAM, LPARAM);

//
/// passes wParam as a uint, lParam.lo as a uint, and lParam.hi as a uint and returns a bool.
// 
_OWLFUNC(LRESULT)
B_U_U_U_Dispatch(TGeneric&, bool (TGeneric::*)(uint, uint, uint), WPARAM, LPARAM);

//
/// passes wParam as an HWND, lParam.lo as a uint, and lParam.hi as a uint and returns a bool.
// 
_OWLFUNC(LRESULT)
B_W_U_U_Dispatch(TGeneric&, bool (TGeneric::*)(HWND, uint, uint), WPARAM, LPARAM);

//
/// passes wParam as a WPARAM and lParam as an LPARAM and returns a uint result
//
_OWLFUNC(LRESULT)
U_WPARAM_LPARAM_Dispatch(TGeneric&, uint (TGeneric::*)(WPARAM, LPARAM), WPARAM, LPARAM);

//
/// passes wParam as a WPARAM and lParam as an LPARAM and returns a bool result
//
_OWLFUNC(LRESULT)
B_WPARAM_LPARAM_Dispatch(TGeneric&, bool (TGeneric::*)(WPARAM, LPARAM), WPARAM, LPARAM);

//
/// passes wParam as a bool and lParam as an LPARAM and returns a uint result
//
_OWLFUNC(LRESULT)
U_B_LPARAM_Dispatch(TGeneric&, uint (TGeneric::*)(bool, LPARAM), WPARAM, LPARAM);

//----------------------------------------------------------------------------

//
/// passes nothing and always returns 0
//
_OWLFUNC(LRESULT)
v_Dispatch(TGeneric&, void (TGeneric::*)(), WPARAM, LPARAM);

//
/// passes lParam as an LPARAM and always returns 0
//
_OWLFUNC(LRESULT)
v_LPARAM_Dispatch(TGeneric&, void (TGeneric::*)(LPARAM), WPARAM, LPARAM);

//
/// passes lParam as a const TPoint& and always returns 0
//
_OWLFUNC(LRESULT)
v_POINT_Dispatch(TGeneric&, void (TGeneric::*)(const owl::TPoint&), WPARAM, LPARAM);

//
/// passes lParam as a void* and always returns 0
//
_OWLFUNC(LRESULT)
v_POINTER_Dispatch(TGeneric&, void (TGeneric::*)(void*), WPARAM, LPARAM);

//
/// passes lParam as a uint and always returns 0
//
_OWLFUNC(LRESULT)
v_U_Dispatch(TGeneric&, void (TGeneric::*)(uint), WPARAM, LPARAM);

//
/// passes wParam as a uint and lParam as a const TSize& and always returns 0
//
_OWLFUNC(LRESULT)
v_U_SIZE_Dispatch(TGeneric&, void (TGeneric::*)(uint, const owl::TSize&), WPARAM, LPARAM);

//
/// passes wParam as a uint and lParam as a const TPoint& and always returns 0
//
_OWLFUNC(LRESULT)
v_U_POINT_Dispatch(TGeneric&, void (TGeneric::*)(uint, const owl::TPoint&), WPARAM, LPARAM);

//
/// passes wParam as a uint and lParam as a uint and always returns 0
//
_OWLFUNC(LRESULT)
v_U_U_Dispatch(TGeneric&, void (TGeneric::*)(uint, uint), WPARAM, LPARAM);

//
/// passes wParam as a uint and lParam as an LPARAM and always returns 0
//
_OWLFUNC(LRESULT)
v_U_LPARAM_Dispatch(TGeneric&, void (TGeneric::*)(uint, LPARAM), WPARAM, LPARAM);

//
/// passes wParam as a WPARAM and lParam as a uint and always returns 0
//
_OWLFUNC(LRESULT)
v_WPARAM_U_Dispatch(TGeneric&, void (TGeneric::*)(WPARAM, uint), WPARAM, LPARAM);

//
/// passes wParam as a uint and lParam as a bool and always returns 0
//
_OWLFUNC(LRESULT)
v_U_B_Dispatch(TGeneric&, void (TGeneric::*)(uint, bool), WPARAM, LPARAM);

//
/// passes wParam as a WPARAM and lParam as a bool and always returns 0
//
_OWLFUNC(LRESULT)
v_WPARAM_B_Dispatch(TGeneric&, void (TGeneric::*)(WPARAM, bool), WPARAM, LPARAM);

//
/// passes wParam as a bool and lParam as a uint and always returns 0
//
_OWLFUNC(LRESULT)
v_B_U_Dispatch(TGeneric&, void (TGeneric::*)(bool, uint), WPARAM, LPARAM);

//
/// passes wParam as a bool and lParam as a LPARAM and always returns 0
//
_OWLFUNC(LRESULT)
v_B_LPARAM_Dispatch(TGeneric&, void (TGeneric::*)(bool, LPARAM), WPARAM, LPARAM);

//
/// passes wParam as a uint, lParam.lo as a uint, and lParam.hi as a uint and always returns 0
//
_OWLFUNC(LRESULT)
v_U_U_U_Dispatch(TGeneric&, void (TGeneric::*)(uint, uint, uint), WPARAM, LPARAM);

//
/// passes wParam as an HWND, lParam.lo as a uint, and lParam.hi as a uint and always returns 0
//
_OWLFUNC(LRESULT)
v_W_U_U_Dispatch(TGeneric&, void (TGeneric::*)(HWND, uint, uint), WPARAM, LPARAM);

//
/// passes wParam as a uint, lParam.lo as a int, and lParam.hi as a int and always returns 0
//
_OWLFUNC(LRESULT)
v_U_i_i_Dispatch(TGeneric&, void (TGeneric::*)(uint, int, int), WPARAM, LPARAM);

//
/// passes wParam as an HWND, lParam.lo as a int, and lParam.hi as a int and always returns 0
//
_OWLFUNC(LRESULT)
v_W_i_i_Dispatch(TGeneric&, void (TGeneric::*)(HWND, int, int), WPARAM, LPARAM);

//
/// passes wParam as a bool, lParam.lo as a uint, and lParam.hi as a uint and always returns 0
//
_OWLFUNC(LRESULT)
v_B_U_U_Dispatch(TGeneric&, void (TGeneric::*)(bool, uint, uint), WPARAM, LPARAM);

//
/// passes wParam as a uint, lParam.lo as a uint, and lParam.hi as a bool and always returns 0
//
_OWLFUNC(LRESULT)
v_U_U_B_Dispatch(TGeneric&, void (TGeneric::*)(uint, uint, bool), WPARAM, LPARAM);

//
/// passes wParam as an HMENU, lParam.lo as a uint, and lParam.hi as a bool and always returns 0
//
_OWLFUNC(LRESULT)
v_HMENU_U_B_Dispatch(TGeneric&, void (TGeneric::*)(HMENU, uint, bool), WPARAM, LPARAM);

//
/// passes wParam as a WPARAM and always returns 0
//
_OWLFUNC(LRESULT)
v_WPARAM_Dispatch(TGeneric&, void (TGeneric::*)(WPARAM), WPARAM, LPARAM);//

//
/// passes wParam as an HWND and lParam as a const reference and returns bool result
/// for WM_COPYDATA
//
_OWLFUNC(LRESULT)
b_HWND_COPYDATASTRUCT_Dispatch(TGeneric&, bool (TGeneric::*)(HWND, const COPYDATASTRUCT&), WPARAM, LPARAM);

//
/// passes wParam as a bool and always returns 0
//
_OWLFUNC(LRESULT)
v_B_Dispatch(TGeneric&, void (TGeneric::*)(bool), WPARAM, LPARAM);

//
/// passes wParam as bool and lParam as bool and always returns 0
//
_OWLFUNC(LRESULT)
v_B_B_Dispatch(TGeneric&, void (TGeneric::*)(bool, bool), WPARAM, LPARAM);

//
/// passes wParam as a WPARAM and lParam as an LPARAM and always returns 0
//
_OWLFUNC(LRESULT)
v_WPARAM_LPARAM_Dispatch(TGeneric&, void (TGeneric::*)(WPARAM, LPARAM), WPARAM, LPARAM);


//----------------------------------------------------------------------------
// Semi-custom crackers

//
/// passes wParam.lo as a uint, lParam as an HWND, and wParam.hi as a uint, and returns an int result
//
_OWLFUNC(LRESULT)
i_U_W_U_Dispatch(TGeneric&, int (TGeneric::*)(uint, HWND, uint), WPARAM, LPARAM);

//
/// passes wParam.lo as a uint, wParam.hi as a uint, and lParam as an HWND, and always returns 0
//
_OWLFUNC(LRESULT)
v_U_U_W_Dispatch(TGeneric&, void (TGeneric::*)(uint, uint, HWND), WPARAM, LPARAM);

//----------------------------------------------------------------------------
// message-specific crackers

//
/// cracker for WM_ACTIVATE
/// passes a uint, a bool, and an HWND and always returns 0
//
_OWLFUNC(LRESULT)
v_Activate_Dispatch(TGeneric&, void (TGeneric::*)(uint, bool, HWND), WPARAM, LPARAM);

//
/// cracker for WM_MDIACTIVATE
/// passes two HWNDs and always returns 0
//
_OWLFUNC(LRESULT)
v_MdiActivate_Dispatch(TGeneric&, void (TGeneric::*)(HWND, HWND), WPARAM, LPARAM);

//
/// passes two uints and an HMENU and returns an LRESULT result
//
_OWLFUNC(LRESULT)
LRESULT_U_U_HMENU_Dispatch(TGeneric&, LRESULT (TGeneric::*)(uint, uint, HMENU), WPARAM, LPARAM);

//
/// cracker for WM_PARENTNOTIFY
/// passes wparam.lo as uint (event), wParam as WPARAM and lParam as LPARAM, and always returns 0
//
_OWLFUNC(LRESULT)
v_ParentNotify_Dispatch(TGeneric&, void (TGeneric::*)(uint, WPARAM, LPARAM), WPARAM, LPARAM);


} // OWL namespace


#endif  // OWL_DISPATCH_H
