//------------------------------------------------------------------------------
// File: Bdasampl.h
//
// Desc: Sample code header file for BDA graph building.
//
// Copyright (c) 2000-2001, Microsoft Corporation. All rights reserved.
//------------------------------------------------------------------------------

#ifndef BDASAMPL_H_INCLUDED_
#define BDASAMPL_H_INCLUDED_

#include <streams.h>
#include <mmreg.h>
#include <msacm.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>

// Temporarily disable /W4 C4201: nonstandard extension used : nameless struct/union
#pragma warning(disable:4201)

#include <ks.h>

// Enable default handling of C4201 warning
#pragma warning(default:4201)

#include <ksmedia.h>
#include <bdatypes.h>
#include <bdamedia.h>
#include <bdaiface.h>
#include <uuids.h>
#include <tuner.h>
#include <commctrl.h>
#include <atlbase.h>

#include "resource.h"

//-----------------------------------------------------------------------------
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK       DlgProc(HWND, UINT, WPARAM, LPARAM);

BOOL CALLBACK       AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK       SelectChannelDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
VOID                ErrorMessageBox(LPTSTR sz,...);

//-----------------------------------------------------------------------------
// index of tuning spaces 
enum NETWORK_TYPE 
{
    ATSC            = 0x0003
};

#endif // BDASAMPL_H_INCLUDED_

