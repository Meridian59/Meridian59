//-----------------------------------------------------------------------------
// File: DIUtil.h
//
// Desc: Input routines
//
// Copyright (C) 1995-2001 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef DIUTIL_H
#define DIUTIL_H

#define DIRECTINPUT_VERSION 0x700
#include <dinput.h>


HRESULT DIUtil_InitInput( HWND hWnd );
VOID    DIUtil_ReadKeys( DWORD* pdwKey );
VOID    DIUtil_CleanupInput();
HRESULT DIUtil_ReacquireInputDevices();


#endif



