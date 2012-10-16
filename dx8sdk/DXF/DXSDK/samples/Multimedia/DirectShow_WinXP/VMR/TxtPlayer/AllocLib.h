//------------------------------------------------------------------------------
// File: AllocLib.h
//
// Desc: DirectShow sample code - header file for VMR sample applications
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#ifndef __INC_ALLOCLIB_H__
#define __INC_ALLOCLIB_H__

#ifndef __ZEROSTRUCT_DEFINED
#define __ZEROSTRUCT_DEFINED
template <typename T>
__inline void ZeroStruct(T& t)
{
    ZeroMemory(&t, sizeof(t));
}
#endif

#ifndef __INITDDSTRUCT_DEFINED
#define __INITDDSTRUCT_DEFINED
template <typename T>
__inline void INITDDSTRUCT(T& dd)
{
    ZeroStruct(dd);
    dd.dwSize = sizeof(dd);
}
#endif


HRESULT
PaintDDrawSurfaceBlack(
    LPDIRECTDRAWSURFACE7 pDDrawSurface
    );

#endif