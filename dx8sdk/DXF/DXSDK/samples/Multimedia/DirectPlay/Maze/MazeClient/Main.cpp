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
#include "D3DFont.h"
#include "D3DUtil.h"
#include "D3DFile.h"
#include "SmartVB.h"
#include "MazeApp.h"
#include "MazeServer.h"
#include "d3dsaver.h"
#include "D3DGraphics.h"


CMazeApp     g_MazeApp;
CD3DGraphics g_D3DGraphics;


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int )
{
    if( FAILED( g_MazeApp.Create( &g_D3DGraphics ) ) )
        return 0;

    return g_MazeApp.Run( hInstance );
}


