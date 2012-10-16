//----------------------------------------------------------------------------
// File: main.cpp
//
// Desc: This is a DirectPlay 8 client/server sample. The client comes in two flavors.  
//       A console based version, and a D3D client.  The D3D client can optionally 
//       be run as screen saver by simply copying mazeclient.exe to your 
//       \winnt\system32\ and renaming it to mazeclient.scr.  This will make 
//       it a screen saver that will be detected by the display control panel.  
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#define D3D_OVERLOADS
#include <windows.h>
#include <D3DX8.h>
#include <dplay8.h>
#include "DXUtil.h"
#include "SyncObjects.h"
#include "IMazeGraphics.h"
#include "DummyConnector.h"
#include "MazeApp.h"
#include "MazeServer.h"
#include "ConsoleGraphics.h"
#include "MazeApp.h"


CMazeApp         g_MazeApp;
CConsoleGraphics g_ConsoleGraphics;


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
int __cdecl main( int argc, TCHAR* argv[] )
{
    if( FAILED( g_MazeApp.Create( &g_ConsoleGraphics ) ) )
        return 0;

    return g_MazeApp.Run( 0 );
}


