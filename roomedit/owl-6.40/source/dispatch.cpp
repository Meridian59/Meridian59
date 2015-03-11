//----------------------------------------------------------------------------
// ObjectWindows
// Copyright (c) 1992, 1996 by Borland International, All Rights Reserved
//
/// \file
/// Implementation of OWL message dispatcher functions
//----------------------------------------------------------------------------
#include <owl/pch.h>
#include <owl/dispatch.h>
#include <owl/wsyscls.h>

namespace owl {

OWL_DIAGINFO;

//----------------------------------------------------------------------------

_OWLFUNC(LRESULT)
i_LPARAM_Dispatch(TGeneric& i, int (TGeneric::*f)(LPARAM), WPARAM, LPARAM p2)
{
  return (i.*f)(p2);
}

_OWLFUNC(LRESULT)
i_WPARAM_Dispatch(TGeneric& i, int (TGeneric::*f)(WPARAM), WPARAM p1, LPARAM)
{
  return (i.*f)(p1);
}

_OWLFUNC(LRESULT)
B_WPARAM_Dispatch(TGeneric& i, bool (TGeneric::*f)(WPARAM), WPARAM p1, LPARAM)
{
  return (i.*f)(p1);
}

//----------------------------------------------------------------------------

_OWLFUNC(LRESULT)
LRESULT_Dispatch(TGeneric& i, LRESULT (TGeneric::*f)(), WPARAM, LPARAM)
{
  return (i.*f)();
}

_OWLFUNC(LRESULT)
LRESULT_LPARAM_Dispatch(TGeneric& i, LRESULT (TGeneric::*f)(LPARAM), WPARAM, LPARAM p2)
{
  return (i.*f)(p2);
}

_OWLFUNC(LRESULT)
LRESULT_U_Dispatch(TGeneric& i, LRESULT (TGeneric::*f)(uint), WPARAM, LPARAM p2)
{
  return (i.*f)((uint)p2);
}

_OWLFUNC(LRESULT)
LRESULT_WPARAM_LPARAM_Dispatch(TGeneric& i, LRESULT (TGeneric::*f)(WPARAM, LPARAM), WPARAM p1, LPARAM p2)
{
  return (i.*f)(p1, p2);
}

//----------------------------------------------------------------------------

_OWLFUNC(LRESULT)
U_Dispatch(TGeneric& i, uint (TGeneric::*f)(), WPARAM, LPARAM)
{
  return (i.*f)();
}

_OWLFUNC(LRESULT)
B_Dispatch(TGeneric& i, bool (TGeneric::*f)(), WPARAM, LPARAM)
{
  return (i.*f)();
}

_OWLFUNC(LRESULT)
B_LPARAM_Dispatch(TGeneric& i, bool (TGeneric::*f)(LPARAM), WPARAM, LPARAM p2)
{
  return (i.*f)(p2) != false;
}

_OWLFUNC(LRESULT)
U_LPARAM_Dispatch(TGeneric& i, uint (TGeneric::*f)(LPARAM), WPARAM, LPARAM p2)
{
  return (i.*f)(p2);
}

_OWLFUNC(LRESULT)
U32_Dispatch(TGeneric& i, uint32 (TGeneric::*f)(), WPARAM, LPARAM)
{
  return (i.*f)();
}

_OWLFUNC(LRESULT)
U_POINT_Dispatch(TGeneric& i, uint (TGeneric::*f)(const TPoint&), WPARAM, LPARAM p2)
{
  TPoint p(p2);
  return (i.*f)(p);
}

_OWLFUNC(LRESULT)
U_POINTER_Dispatch(TGeneric& i, uint (TGeneric::*f)(void*), WPARAM, LPARAM p2)
{
  return (i.*f)((void*)p2);
}

_OWLFUNC(LRESULT)
B_POINTER_Dispatch(TGeneric& i, bool (TGeneric::*f)(void*), WPARAM, LPARAM p2)
{
  return (i.*f)((void*)p2);
}

_OWLFUNC(LRESULT)
U_U_Dispatch(TGeneric& i, uint (TGeneric::*f)(uint), WPARAM p1, LPARAM)
{
  return (i.*f)((uint)p1);
}

_OWLFUNC(LRESULT)
B_B_Dispatch(TGeneric& i, bool (TGeneric::*f)(bool), WPARAM p1, LPARAM)
{
  return (i.*f)((bool)p1);
}

_OWLFUNC(LRESULT)
B_I2_Dispatch(TGeneric& i, bool (TGeneric::*f)(int), WPARAM, LPARAM p2)
{
  return (i.*f)((int)p2);
}

_OWLFUNC(LRESULT)
U_B_Dispatch(TGeneric& i, uint (TGeneric::*f)(bool), WPARAM p1, LPARAM)
{
  return (i.*f)((bool)p1);
}

_OWLFUNC(LRESULT)
B_B_U_Dispatch(TGeneric& i, bool (TGeneric::*f)(bool, uint), WPARAM p1, LPARAM p2)
{
  return (i.*f)((bool)p1, (uint)p2);
}

_OWLFUNC(LRESULT)
U_U_RECT_Dispatch(TGeneric& i, uint (TGeneric::*f)(uint, const TRect&), WPARAM p1, LPARAM p2)
{
  TRect r(*reinterpret_cast<LPRECT>(p2));
  return (i.*f)(p1, r);
}

_OWLFUNC(LRESULT)
B_U_RECT_Dispatch(TGeneric& i, bool (TGeneric::*f)(uint, const TRect&), WPARAM p1, LPARAM p2)
{
  TRect r(*reinterpret_cast<LPRECT>(p2));
  return (i.*f)(p1, r);
}

_OWLFUNC(LRESULT)
B_U_U_POINT_Dispatch(TGeneric& i, bool (TGeneric::*f)(uint, uint, const TPoint&), WPARAM p1, LPARAM p2)
{
  TPoint p(p2);
  return (i.*f)(LoUint16(p1), HiUint16(p1), p);
}

_OWLFUNC(LRESULT)
B_U_i_POINT_Dispatch(TGeneric& i, bool (TGeneric::*f)(uint, int, const owl::TPoint&), WPARAM p1, LPARAM p2)
{
  TPoint p(LoInt16(p2), HiInt16(p2));  
  return (i.*f)(LoUint16(p1), HiInt16(p1), p);
  //return 0;
}

_OWLFUNC(LRESULT)
U_U_U_U_Dispatch(TGeneric& i, uint (TGeneric::*f)(uint, uint, uint), WPARAM p1, LPARAM p2)
{
  return (i.*f)(p1, LoUint16(p2), HiUint16(p2));
}

_OWLFUNC(LRESULT)
U_W_U_U_Dispatch(TGeneric& i, uint (TGeneric::*f)(HWND, uint, uint), WPARAM p1, LPARAM p2)
{
  return (i.*f)(reinterpret_cast<HWND>(p1), LoUint16(p2), HiUint16(p2));
}

_OWLFUNC(LRESULT)
U_U_U_Dispatch(TGeneric& i, uint (TGeneric::*f)(uint, uint), WPARAM p1, LPARAM p2)
{
  return (i.*f)(p1, uint(p2));
}

_OWLFUNC(LRESULT)
U_B_U_Dispatch(TGeneric& i, uint (TGeneric::*f)(bool, uint), WPARAM p1, LPARAM p2)
{
  return (i.*f)((bool)p1, uint(p2));
}

_OWLFUNC(LRESULT)
B_U_U_Dispatch(TGeneric& i, bool (TGeneric::*f)(uint, uint), WPARAM p1, LPARAM p2)
{
  return (i.*f)(p1, (uint)p2);
}

_OWLFUNC(LRESULT)
B_U_U_U_Dispatch(TGeneric& i, bool (TGeneric::*f)(uint, uint, uint), WPARAM p1, LPARAM p2)
{
  return (i.*f)(p1, LoUint16(p2), HiUint16(p2));
}

_OWLFUNC(LRESULT)
B_W_U_U_Dispatch(TGeneric& i, bool (TGeneric::*f)(HWND, uint, uint), WPARAM p1, LPARAM p2)
{
  return (i.*f)(reinterpret_cast<HWND>(p1), LoUint16(p2), HiUint16(p2));
}

_OWLFUNC(LRESULT)
U_WPARAM_LPARAM_Dispatch(TGeneric& i, uint (TGeneric::*f)(WPARAM, LPARAM), WPARAM p1, LPARAM p2)
{
  return (i.*f)(p1, p2);
}

_OWLFUNC(LRESULT)
B_WPARAM_LPARAM_Dispatch(TGeneric& i, bool (TGeneric::*f)(WPARAM, LPARAM), WPARAM p1, LPARAM p2)
{
  return (i.*f)(p1, p2);
}

_OWLFUNC(LRESULT)
U_B_LPARAM_Dispatch(TGeneric& i, uint (TGeneric::*f)(bool, LPARAM), WPARAM p1, LPARAM p2)
{
  return (i.*f)((bool)p1, p2);
}

//----------------------------------------------------------------------------

_OWLFUNC(LRESULT)
v_Dispatch(TGeneric& i, void (TGeneric::*f)(), WPARAM, LPARAM)
{
  (i.*f)();
  return 0;
}

_OWLFUNC(LRESULT)
v_LPARAM_Dispatch(TGeneric& i, void (TGeneric::*f)(LPARAM), WPARAM, LPARAM p2)
{
  (i.*f)(p2);
  return 0;
}

_OWLFUNC(LRESULT)
v_POINT_Dispatch(TGeneric& i, void (TGeneric::*f)(const TPoint&), WPARAM, LPARAM p2)
{
  TPoint p(p2);
  (i.*f)(p);

  return 0;
}

_OWLFUNC(LRESULT)
v_POINTER_Dispatch(TGeneric& i, void (TGeneric::*f)(void*), WPARAM, LPARAM p2)
{
  (i.*f)((void*)p2);
  return 0;
}

_OWLFUNC(LRESULT)
v_U_Dispatch(TGeneric& i, void (TGeneric::*f)(uint), WPARAM, LPARAM p2)
{
  (i.*f)((uint)p2);
  return 0;
}

_OWLFUNC(LRESULT)
v_U_SIZE_Dispatch(TGeneric& i, void (TGeneric::*f)(uint, const TSize&), WPARAM p1, LPARAM p2)
{
  TSize s(static_cast<DWORD>(p2));
  (i.*f)(p1, s);
  return 0;
}

_OWLFUNC(LRESULT)
v_U_POINT_Dispatch(TGeneric& i, void (TGeneric::*f)(uint, const TPoint&), WPARAM p1, LPARAM p2)
{
  TPoint p(p2);
  (i.*f)(p1, p);
  return 0;
}

_OWLFUNC(LRESULT)
v_U_U_Dispatch(TGeneric& i, void (TGeneric::*f)(uint, uint), WPARAM p1, LPARAM p2)
{
  (i.*f)(p1, (uint)p2);
  return 0;
}

_OWLFUNC(LRESULT)
v_U_LPARAM_Dispatch(TGeneric& i, void (TGeneric::*f)(uint, LPARAM), WPARAM p1, LPARAM p2)
{
  (i.*f)(static_cast<uint>(p1), p2);
  return 0;
}

_OWLFUNC(LRESULT)
v_WPARAM_U_Dispatch(TGeneric& i, void (TGeneric::*f)(WPARAM, uint), WPARAM p1, LPARAM p2)
{
  (i.*f)(p1, (uint)p2);
  return 0;
}

_OWLFUNC(LRESULT)
v_U_B_Dispatch(TGeneric& i, void (TGeneric::*f)(uint, bool), WPARAM p1, LPARAM p2)
{
  (i.*f)(p1, (bool)p2);
  return 0;
}

_OWLFUNC(LRESULT)
v_WPARAM_B_Dispatch(TGeneric& i, void (TGeneric::*f)(WPARAM, bool), WPARAM p1, LPARAM p2)
{
  (i.*f)(p1, (bool)p2);
  return 0;
}

_OWLFUNC(LRESULT)
v_B_U_Dispatch(TGeneric& i, void (TGeneric::*f)(bool, uint), WPARAM p1, LPARAM p2)
{
  (i.*f)((bool)p1, (uint)p2);
  return 0;
}

_OWLFUNC(LRESULT)
v_B_LPARAM_Dispatch(TGeneric& i, void (TGeneric::*f)(bool, LPARAM), WPARAM p1, LPARAM p2)
{
  (i.*f)((bool)p1, p2);
  return 0;
}

_OWLFUNC(LRESULT)
v_U_U_U_Dispatch(TGeneric& i, void (TGeneric::*f)(uint, uint, uint), WPARAM p1, LPARAM p2)
{
  (i.*f)(p1, LoUint16(p2), HiUint16(p2));
  return 0;
}

_OWLFUNC(LRESULT)
v_W_U_U_Dispatch(TGeneric& i, void (TGeneric::*f)(HWND, uint, uint), WPARAM p1, LPARAM p2)
{
  (i.*f)(reinterpret_cast<HWND>(p1), LoUint16(p2), HiUint16(p2));
  return 0;
}

_OWLFUNC(LRESULT)
v_U_i_i_Dispatch(TGeneric& i, void (TGeneric::*f)(uint, int, int), WPARAM p1, LPARAM p2)
{
  (i.*f)(p1, LoInt16(p2), HiInt16(p2));
  return 0;
}

_OWLFUNC(LRESULT)
v_W_i_i_Dispatch(TGeneric& i, void (TGeneric::*f)(HWND, int, int), WPARAM p1, LPARAM p2)
{
  (i.*f)(reinterpret_cast<HWND>(p1), LoInt16(p2), HiInt16(p2));
  return 0;
}

_OWLFUNC(LRESULT)
v_B_U_U_Dispatch(TGeneric& i, void (TGeneric::*f)(bool, uint, uint), WPARAM p1, LPARAM p2)
{
  (i.*f)((bool)p1, LoUint16(p2), HiUint16(p2));
  return 0;
}

_OWLFUNC(LRESULT)
v_U_U_B_Dispatch(TGeneric& i, void (TGeneric::*f)(uint, uint, bool), WPARAM p1, LPARAM p2)
{
  (i.*f)(p1, LoUint16(p2), (bool)HiUint16(p2));
  return 0;
}

_OWLFUNC(LRESULT)
v_HMENU_U_B_Dispatch(TGeneric& i, void (TGeneric::*f)(HMENU, uint, bool), WPARAM p1, LPARAM p2)
{
  (i.*f)(reinterpret_cast<HMENU>(p1), LoUint16(p2), (bool)HiUint16(p2));
  return 0;
}

_OWLFUNC(LRESULT)
v_WPARAM_Dispatch(TGeneric& i, void (TGeneric::*f)(WPARAM), WPARAM p1, LPARAM)
{
  (i.*f)(p1);
  return 0;
}

_OWLFUNC(LRESULT)
v_B_Dispatch(TGeneric& i, void (TGeneric::*f)(bool), WPARAM p1, LPARAM)
{
  (i.*f)((bool)p1);
  return 0;
}

_OWLFUNC(LRESULT)
v_B_B_Dispatch(TGeneric& i, void (TGeneric::*f)(bool, bool), WPARAM p1, LPARAM p2)
{
  (i.*f)((bool)p1, (bool)p2);
  return 0;
}

_OWLFUNC(LRESULT)
b_HWND_COPYDATASTRUCT_Dispatch(TGeneric& i, bool (TGeneric::*f)(HWND, const COPYDATASTRUCT&), WPARAM p1, LPARAM p2)
{
  return p2 ? (i.*f)((HWND)p1, *(COPYDATASTRUCT*)p2) : false;
}

_OWLFUNC(LRESULT)
v_WPARAM_LPARAM_Dispatch(TGeneric& i, void (TGeneric::*f)(WPARAM, LPARAM), WPARAM p1, LPARAM p2)
{
  (i.*f)(p1, p2);
  return 0;
}

//----------------------------------------------------------------------------
//
// Semi-custom message crackers
//
_OWLFUNC(LRESULT)
i_U_W_U_Dispatch(TGeneric& i, int (TGeneric::*f)(uint, HWND, uint), WPARAM p1, LPARAM p2)
{
  return (i.*f)(LoUint16(p1), reinterpret_cast<HWND>(p2), HiUint16(p1));
}

_OWLFUNC(LRESULT)
v_U_U_W_Dispatch(TGeneric& i, void (TGeneric::*f)(uint, uint, HWND), WPARAM p1, LPARAM p2)
{
  (i.*f)(LoUint16(p1), HiUint16(p1), reinterpret_cast<HWND>(p2));
  return 0;
}

//----------------------------------------------------------------------------
//
// Custom message crackers
//

_OWLFUNC(LRESULT)
v_Activate_Dispatch(TGeneric& i, void (TGeneric::*f)(uint, bool, HWND), WPARAM p1, LPARAM p2)
{
  (i.*f)(LoUint16(p1), (bool)HiUint16(p1), reinterpret_cast<HWND>(p2));
  return 0;
}

_OWLFUNC(LRESULT)
v_MdiActivate_Dispatch(TGeneric& i, void (TGeneric::*f)(HWND, HWND), WPARAM p1, LPARAM p2)
{
  (i.*f)(reinterpret_cast<HWND>(p2), reinterpret_cast<HWND>(p1));
  return 0;
}

_OWLFUNC(LRESULT)
LRESULT_U_U_HMENU_Dispatch(TGeneric& i, LRESULT (TGeneric::*f)(uint, uint, HMENU), WPARAM p1, LPARAM p2)
{
  return (i.*f)(LoUint16(p1), HiUint16(p1), reinterpret_cast<HMENU>(p2));
}

_OWLFUNC(LRESULT)
v_ParentNotify_Dispatch(TGeneric& i, void (TGeneric::*f)(uint, WPARAM, LPARAM), WPARAM p1, LPARAM p2)
{
  (i.*f)(LoUint16(p1), p1, p2);
  return 0;
}

} // OWL namespace

//==============================================================================
