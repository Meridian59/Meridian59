//-----------------------------------------------------------------------------
// File: Moire.h
//
// Desc: 
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _MOIRE_H
#define _MOIRE_H

#define MAX_DEVICE_OBJECTS 10

struct DeviceObjects
{
    CD3DFont* m_pStatsFont;
    LPDIRECT3DVERTEXBUFFER8 m_pVBArray[3];
    LPDIRECT3DTEXTURE8 m_pTexArray[4];
};


class   CMoireScreensaver : public CD3DScreensaver
{
protected:
    DeviceObjects  m_DeviceObjectsArray[MAX_DEVICE_OBJECTS];
    DeviceObjects* m_pDeviceObjects;

    DWORD   m_dwNumVerticesArray[3];

    DWORD   m_dwMeshInterval;
    INT     m_iMesh;
    INT     m_iMeshPrev;
    FLOAT   m_fTimeNextMeshChange;
    FLOAT   m_fTimeStartMeshChange;

    DWORD   m_dwTextureInterval;
    INT     m_iTexture;
    INT     m_iTexturePrev;
    FLOAT   m_fTimeNextTextureChange;
    FLOAT   m_fTimeStartTextureChange;

    DWORD   m_dwColorInterval;
    INT     m_iColorScheme;
    INT     m_iColorSchemePrev;
    FLOAT   m_fTimeNextColorChange;
    FLOAT   m_fTimeStartColorChange;
    BOOL    m_bBrightColors;
    
    FLOAT   m_fScale1, m_fScale2;
    FLOAT   m_fRot1, m_fRot2;

    D3DXCOLOR m_col1;
    D3DXCOLOR m_col2;
    D3DXCOLOR m_col3;

    FLOAT   m_fSpeed;

protected:
    virtual VOID    DoConfig();
    virtual VOID    ReadSettings();

    virtual VOID    SetDevice( UINT iDevice );
    virtual HRESULT Render();
    virtual HRESULT FrameMove();
    virtual HRESULT RestoreDeviceObjects();
    virtual HRESULT InvalidateDeviceObjects();

    VOID    Randomize( INT* piNum, INT iMax );
    HRESULT BuildTextures();
    HRESULT BuildMeshes();
    HRESULT LoadDDSTextureFromResource( LPDIRECT3DDEVICE8 pd3dDevice, 
        TCHAR* strRes, LPDIRECT3DTEXTURE8* ppTex );
    VOID    GenerateColors( INT iColorScheme, BOOL bBright, 
        D3DXCOLOR* pcol1, D3DXCOLOR* pcol2, D3DXCOLOR* pcol3 );
    HRESULT ReadRegistry();
    HRESULT WriteRegistry();


public:
    CMoireScreensaver();

    // Override from CD3DScreensaver
    INT_PTR CALLBACK ConfigureDialogProc( HWND hwndDlg, UINT uMsg, 
                                          WPARAM wParam, LPARAM lParam );
};

#endif
