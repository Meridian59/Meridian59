//----------------------------------------------------------------------------
// File: d3dgraphics.h
//
// Desc: see main.cpp 
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _MAZE_GRAPHICS_H
#define _MAZE_GRAPHICS_H



class CMazeApp;

struct VERTEX_TEXURED
{
    D3DXVECTOR3 vPos;
    D3DXVECTOR3 vNormal;
    FLOAT       fU, fV;
};
#define FVF_TEXTURED (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)


struct VERTEX_DIFFUSE
{
    D3DXVECTOR3 vPos;
    DWORD       dwDiffuse;
};
#define FVF_DIFFUSE (D3DFVF_XYZ|D3DFVF_DIFFUSE)


struct VERTEX_TL_DIFFUSE
{
    D3DXVECTOR3 vPos;
    float       fRHW;
    DWORD       dwDiffuse;
};
#define FVF_TL_DIFFUSE (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)


struct VERTEX_TL_TEXTURED
{
    D3DXVECTOR3 vPos;
    float       fRHW;
    DWORD       dwDiffuse;
    DWORD       dwSpecular;    
    FLOAT       fU, fV;
};
#define FVF_TL_TEXTURED  (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_SPECULAR|D3DFVF_TEX1)


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
class CD3DGraphics : public IMazeGraphics, public CD3DScreensaver
{
public:
    CD3DGraphics();
    ~CD3DGraphics();

    // From IMazeGraphics
    virtual VOID    Init( CMazeApp* pMazeApp, CDPlay8Client* pDP8Client, CMazeClient* pMazeClient ); 
    virtual HRESULT Create( HINSTANCE hInstance );
    virtual INT     Run();
    virtual void    HandleOutputMsg( EnumLineType enumLineType, TCHAR* strLine );
    virtual BOOL    IsPreview() { return m_bIsPreview; };
    virtual void    Shutdown();

    virtual BOOL    GetFloatWinInfo()                          { return FALSE; }
    virtual VOID    SetDevice( UINT iDevice )                  { }
    virtual HRESULT ConfirmDevice(D3DCAPS8*,DWORD,D3DFORMAT);
    virtual HRESULT OneTimeSceneInit();
    virtual HRESULT InitDeviceObjects();
    virtual HRESULT RestoreDeviceObjects();
    virtual HRESULT FrameMove();
    virtual HRESULT Render();
    virtual HRESULT InvalidateDeviceObjects();
    virtual HRESULT DeleteDeviceObjects();
    virtual HRESULT FinalCleanup();
    virtual LRESULT SaverProc( HWND hWnd , UINT message , WPARAM wParam , LPARAM lParam );

    CMazeApp*           m_pMazeApp;
    CDPlay8Client*      m_pDP8Client;
    CMazeClient*        m_pMazeClient;

protected:
    static INT_PTR CALLBACK StaticSaverConfigDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    INT_PTR CALLBACK        SaverConfigDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

    HRESULT PostCreate();
    int     ScreenSaverDoConfig( BOOL bIsScreenSaverSettings );
    void    ExtractSaverConfigDlgSettings( HWND hDlg );
    void    PopulateSaverConfigDlg( HWND hDlg );
    HRESULT CreateTextureFromResource( INT nResource, LPDIRECT3DTEXTURE8* ppTexture );
    void    ComputeCameraMatrix();

    void    DrawFloor();
    void    DrawWalls();
    void    DrawCeiling();
    void    DrawMiniMap();
    void    DrawPlayers();
    void    DrawIndicators();
    void    LoadQuad( VERTEX_TEXURED* pVerts, WORD* pwIndex, WORD wOffset,
                      const D3DXVECTOR3& vOrigin, const D3DXVECTOR3& vBasis1,
                      const D3DXVECTOR3& vBasis2, const D3DXVECTOR3& vNormal );

    HINSTANCE               m_hInstance;

    MazeConfig*             m_pConfig;

    BOOL                    m_bIsPreview;
    BOOL                    m_bIsScreenSaverSettings;

    DWORD                   m_dwStartMode;
    DWORD                   m_dwAppFlags;
    HWND                    m_hRefWindow;

    CD3DFont*               m_pFont;
    CD3DFont*               m_pStatsFont;

    SmartVB<VERTEX_TEXURED,FVF_TEXTURED,1000> m_SmartVB;
    LPDIRECT3DVERTEXBUFFER8 m_pMiniMapVB;
    LPDIRECT3DVERTEXBUFFER8 m_pMiniMapBackgroundVB;
    LPDIRECT3DVERTEXBUFFER8 m_pIndicatorVB;

    D3DXMATRIX              m_Projection;
    D3DXMATRIX              m_Camera;

    D3DXVECTOR3             m_vCameraPos;
    FLOAT                   m_fCameraYaw;

    D3DLIGHT8               m_Light;

    LPDIRECT3DTEXTURE8      m_pWallTexture;
    LPDIRECT3DTEXTURE8      m_pFloorTexture;
    LPDIRECT3DTEXTURE8      m_pCeilingTexture;
    LPDIRECT3DTEXTURE8      m_pNetIconTexture;
    LPDIRECT3DTEXTURE8      m_pLocalIconTexture;

    ID3DXMesh*              m_pSphere;
    CD3DFile*               m_pPlayerMesh;
    DWORD                   m_dwTesselation;

    enum { MAX_VISLIST = 300 };
    MazeCellRef                             m_mcrVisList[MAX_VISLIST];
    DWORD                                   m_dwNumVisibleCells;
};


#endif
