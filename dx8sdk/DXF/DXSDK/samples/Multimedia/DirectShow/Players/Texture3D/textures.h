//-----------------------------------------------------------------------------
// File: Textures.h
//
// Desc: DirectShow sample code - header file for DirectShow/Direct3D8 video 
//       texturing
//       
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------


#include <d3dx8.h>
#include <windows.h>
#include <mmsystem.h>
#include <atlbase.h>
#include <stdio.h>

#include <d3d8types.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
HRESULT InitDShowTextureRenderer(LPDIRECT3DTEXTURE8 pTexture);
void CheckMovieStatus(void);
void CleanupDShow(void);
HRESULT AddToROT(IUnknown *pUnkGraph); 
void RemoveFromROT(void);
void Msg(TCHAR *szFormat, ...);

//-----------------------------------------------------------------------------
// Direct3D global variables
//-----------------------------------------------------------------------------
extern LPDIRECT3D8             g_pD3D;		 // Used to create the D3DDevice
extern LPDIRECT3DDEVICE8       g_pd3dDevice; // Our rendering device
extern LPDIRECT3DVERTEXBUFFER8 g_pVB;		 // Buffer to hold vertices
extern LPDIRECT3DTEXTURE8      g_pTexture;   // Our texture

