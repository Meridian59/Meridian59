//-----------------------------------------------------------------------------
// File: Moire.cpp
//
// Desc: Fun screen saver
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <windows.h>
#include <d3d8.h>
#include <d3dx8.h>
#include <stdio.h>
#include <commdlg.h>
#include <commctrl.h>
#include "d3dsaver.h"
#include "d3dfont.h"
#include "Moire.h"
#include "Resource.h"
#include "dxutil.h"


//-----------------------------------------------------------------------------
// Name: struct MYVERTEX
// Desc: D3D vertex type for this app
//-----------------------------------------------------------------------------
struct MYVERTEX
{
    D3DXVECTOR3 p;     // Position
    FLOAT       tu;    // Vertex texture coordinates
    FLOAT       tv;
    
    MYVERTEX(D3DXVECTOR3 pInit, FLOAT tuInit, FLOAT tvInit)
        { p = pInit; tu = tuInit; tv = tvInit; }
};

#define D3DFVF_MYVERTEX (D3DFVF_XYZ | D3DFVF_TEX1)



CMoireScreensaver* g_pMyMoireScreensaver = NULL;




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything, and goes into a
//       message-processing loop. Idle time is used to render the scene.
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    HRESULT hr;
    CMoireScreensaver moireSS;

    if( FAILED( hr = moireSS.Create( hInst ) ) )
    {
        moireSS.DisplayErrorMsg( hr );
        return 0;
    }

    return moireSS.Run();
}




//-----------------------------------------------------------------------------
// Name: CMoireScreensaver()
// Desc: Constructor
//-----------------------------------------------------------------------------
CMoireScreensaver::CMoireScreensaver( )
{
    g_pMyMoireScreensaver = this;

    InitCommonControls();

    ZeroMemory( m_DeviceObjectsArray, sizeof(m_DeviceObjectsArray) );

    srand(0);

    lstrcpy( m_strRegPath, TEXT("Software\\Microsoft\\Screensavers\\Moire") );
    LoadString( NULL, IDS_DESCRIPTION, m_strWindowTitle, 200 );
    m_dwMeshInterval = 30;
    m_iMesh = -1;
    Randomize( &m_iMesh, 3 );
    m_iMeshPrev = -1;
    m_fTimeNextMeshChange = 0.0f;
    m_fTimeStartMeshChange = 0.0f;

    m_dwTextureInterval = 90;
    m_iTexture = -1;
    Randomize( &m_iTexture, 4 );
    m_iTexturePrev = -1;
    m_fTimeNextTextureChange = 0.0f;
    m_fTimeStartTextureChange = 0.0f;

    m_dwColorInterval = 60;
    m_iColorScheme = -1;
    Randomize( &m_iColorScheme, 4 );
    m_iColorSchemePrev = -1;
    m_fTimeNextColorChange = 0.0f;
    m_fTimeStartColorChange = 0.0f;
    m_bBrightColors = TRUE;

    m_fSpeed = 1.0f;
}




//-----------------------------------------------------------------------------
// Name: Randomize()
// Desc: Find a random number between 0 and iMax that is different from the
//       initial value of *piNum, and store it in *piNum.
//-----------------------------------------------------------------------------
VOID CMoireScreensaver::Randomize( INT* piNum, INT iMax )
{
    INT iInit = *piNum;
    while ( *piNum == iInit )
    {
        *piNum = rand() % iMax;
    }
}




//-----------------------------------------------------------------------------
// Name: SetDevice()
// Desc: 
//-----------------------------------------------------------------------------
VOID CMoireScreensaver::SetDevice( UINT iDevice )
{
    m_pDeviceObjects = &m_DeviceObjectsArray[iDevice];
}




//-----------------------------------------------------------------------------
// Name: BuildMeshes()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMoireScreensaver::BuildMeshes()
{
    SAFE_RELEASE( m_pDeviceObjects->m_pVBArray[0] );
    SAFE_RELEASE( m_pDeviceObjects->m_pVBArray[1] );
    SAFE_RELEASE( m_pDeviceObjects->m_pVBArray[2] );

    // Sandwich
    {
        m_dwNumVerticesArray[0] = 12;

        if( FAILED( m_pd3dDevice->CreateVertexBuffer( m_dwNumVerticesArray[0] * sizeof(MYVERTEX),
            D3DUSAGE_WRITEONLY, D3DFVF_MYVERTEX, D3DPOOL_MANAGED, &m_pDeviceObjects->m_pVBArray[0] ) ) )
        {
            return E_FAIL;
        }

        MYVERTEX* v;
        m_pDeviceObjects->m_pVBArray[0]->Lock( 0, 0, (BYTE**)&v, 0 );

        v[0] = MYVERTEX( D3DXVECTOR3(  10, -0.1f, -10 ),  0.0f,  0.0f );
        v[1] = MYVERTEX( D3DXVECTOR3( -10, -0.1f, -10 ), 10.0f,  0.0f );
        v[2] = MYVERTEX( D3DXVECTOR3(  10,  0.0f,  10 ),  0.0f, 10.0f );

        v[3] = MYVERTEX( D3DXVECTOR3( -10, -0.1f, -10 ), 10.0f,  0.0f );
        v[4] = MYVERTEX( D3DXVECTOR3( -10,  0.0f,  10 ), 10.0f, 10.0f );
        v[5] = MYVERTEX( D3DXVECTOR3(  10,  0.0f,  10 ),  0.0f, 10.0f );

        v[6] = MYVERTEX( D3DXVECTOR3( -10,  0.0f,  10 ),  0.0f, 10.0f );
        v[7] = MYVERTEX( D3DXVECTOR3( -10,  0.1f, -10 ),  0.0f,  0.0f );
        v[8] = MYVERTEX( D3DXVECTOR3(  10,  0.1f, -10 ), 10.0f,  0.0f );

        v[9] = MYVERTEX( D3DXVECTOR3(  10,  0.1f, -10 ), 10.0f,  0.0f );
        v[10] = MYVERTEX( D3DXVECTOR3( 10,  0.0f,  10 ), 10.0f, 10.0f );
        v[11] = MYVERTEX( D3DXVECTOR3(-10,  0.0f,  10 ),  0.0f, 10.0f );

        m_pDeviceObjects->m_pVBArray[0]->Unlock();
    }
    
    // Wall
    {
        m_dwNumVerticesArray[1] = 6;
        if( FAILED( m_pd3dDevice->CreateVertexBuffer( m_dwNumVerticesArray[1] * sizeof(MYVERTEX),
            D3DUSAGE_WRITEONLY, D3DFVF_MYVERTEX, D3DPOOL_MANAGED, &m_pDeviceObjects->m_pVBArray[1] ) ) )
        {
            return E_FAIL;
        }

        MYVERTEX* v;
        m_pDeviceObjects->m_pVBArray[1]->Lock( 0, 0, (BYTE**)&v, 0 );

        v[0] = MYVERTEX( D3DXVECTOR3(  20, -20, 0.0f ), 10.0f, 10.0f );
        v[1] = MYVERTEX( D3DXVECTOR3( -20, -20, 0.0f ),  0.0f, 10.0f );
        v[2] = MYVERTEX( D3DXVECTOR3(  20,  20, 0.0f ), 10.0f,  0.0f );

        v[3] = MYVERTEX( D3DXVECTOR3( -20, -20, 0.0f ),  0.0f, 10.0f );
        v[4] = MYVERTEX( D3DXVECTOR3( -20,  20, 0.0f ),  0.0f,  0.0f );
        v[5] = MYVERTEX( D3DXVECTOR3(  20,  20, 0.0f ), 10.0f,  0.0f );

        m_pDeviceObjects->m_pVBArray[1]->Unlock();
    }

    // Well
    {
        const INT NUM_SEGS = 48;
        m_dwNumVerticesArray[2] = NUM_SEGS * 3;
        if( FAILED( m_pd3dDevice->CreateVertexBuffer( m_dwNumVerticesArray[2] * sizeof(MYVERTEX),
            D3DUSAGE_WRITEONLY, D3DFVF_MYVERTEX, D3DPOOL_MANAGED, &m_pDeviceObjects->m_pVBArray[2] ) ) )
        {
            return E_FAIL;
        }

        MYVERTEX* v;
        m_pDeviceObjects->m_pVBArray[2]->Lock( 0, 0, (BYTE**)&v, 0 );

        INT iVertex = 0;
        FLOAT fTheta;
        FLOAT fTheta2;
        for( INT i = 0; i < NUM_SEGS; i++ )
        {
            fTheta  = (i+0) * (2 * D3DX_PI / NUM_SEGS);
            fTheta2 = (i+1) * (2 * D3DX_PI / NUM_SEGS);
            v[iVertex + 0] = MYVERTEX( D3DXVECTOR3(  10*cosf(fTheta),  10*sinf(fTheta),  -10.0f ),  5*cosf(fTheta),  5*sinf(fTheta) );
            v[iVertex + 1] = MYVERTEX( D3DXVECTOR3(                 0,                0,  100.0f ),  0.0f,  0.0f );
            v[iVertex + 2] = MYVERTEX( D3DXVECTOR3(  10*cosf(fTheta2), 10*sinf(fTheta2), -10.0f ),  5*cosf(fTheta2), 5*sinf(fTheta2) );

            iVertex += 3;
        }

        m_pDeviceObjects->m_pVBArray[2]->Unlock();
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: BuildTextures()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMoireScreensaver::BuildTextures()
{
    HRESULT hr;

    SAFE_RELEASE( m_pDeviceObjects->m_pTexArray[0] );
    SAFE_RELEASE( m_pDeviceObjects->m_pTexArray[1] );
    SAFE_RELEASE( m_pDeviceObjects->m_pTexArray[2] );
    SAFE_RELEASE( m_pDeviceObjects->m_pTexArray[3] );

    if( FAILED( hr = LoadDDSTextureFromResource( m_pd3dDevice, 
        MAKEINTRESOURCE(IDR_STRIPES_DDS), &m_pDeviceObjects->m_pTexArray[0] ) ) )
    {
        return hr;
    }

    if( FAILED( hr = LoadDDSTextureFromResource( m_pd3dDevice, 
        MAKEINTRESOURCE(IDR_BALL_DDS), &m_pDeviceObjects->m_pTexArray[1] ) ) )
    {
        return hr;
    }

    if( FAILED( hr = LoadDDSTextureFromResource( m_pd3dDevice, 
        MAKEINTRESOURCE(IDR_NOISE_DDS), &m_pDeviceObjects->m_pTexArray[2] ) ) )
    {
        return hr;
    }

    if( FAILED( hr = LoadDDSTextureFromResource( m_pd3dDevice, 
        MAKEINTRESOURCE(IDR_SPIRAL_DDS), &m_pDeviceObjects->m_pTexArray[3] ) ) )
    {
        return hr;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: LoadDDSTextureFromResource()
// Desc: Note that this function asks D3DX to not create a full mip chain
//       for the texture.  Modify the NumMips parameter to the
//       D3DXCreateTextureFromFileInMemoryEx call if you want to reuse this
//       function elsewhere and you do want a full mip chain for the texture.
//-----------------------------------------------------------------------------
HRESULT CMoireScreensaver::LoadDDSTextureFromResource( LPDIRECT3DDEVICE8 pd3dDevice, 
    TCHAR* strRes, LPDIRECT3DTEXTURE8* ppTex )
{
    HRESULT hr;
    HMODULE hModule = NULL;
    HRSRC rsrc;
    HGLOBAL hgData;
    LPVOID pvData;
    DWORD cbData;

    rsrc = FindResource( hModule, strRes, "DDS" );
    if( rsrc != NULL )
    {
        cbData = SizeofResource( hModule, rsrc );
        if( cbData > 0 )
        {
            hgData = LoadResource( hModule, rsrc );
            if( hgData != NULL )
            {
                pvData = LockResource( hgData );
                if( pvData != NULL )
                {
                    if( FAILED( hr = D3DXCreateTextureFromFileInMemoryEx( 
                        m_pd3dDevice, pvData, cbData, D3DX_DEFAULT, D3DX_DEFAULT, 
                        1, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, 
                        D3DX_FILTER_NONE, 0, NULL, NULL, ppTex ) ) )
                    {
                        return hr;
                    }
                }
            }
        }
    }
    
    if( *ppTex == NULL)
        return E_FAIL;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CMoireScreensaver::FrameMove()
{
    D3DXCOLOR colBlack( 0, 0, 0, 0 );

    m_fScale1 = 2.0f * (1 + sinf(m_fTime * m_fSpeed / 20.0f));
    m_fRot1 = 0.0f;

    m_fScale2 = 2.0f * (1 + sinf(m_fTime * m_fSpeed / 8.0f));
    m_fRot2 = m_fTime * m_fSpeed * 0.025f;

    if( m_fTimeNextColorChange == 0.0f )
    {
        // Schedule next color scheme change
        m_fTimeNextColorChange = m_fTime + m_dwColorInterval;
    }
    else if( m_fTimeNextColorChange < m_fTime )
    {
        // Start color scheme change
        m_iColorSchemePrev = m_iColorScheme;
        Randomize( &m_iColorScheme, 4 );
        m_fTimeStartColorChange = m_fTime;
        m_fTimeNextColorChange = 0.0f;
    }
    if( m_fTimeStartColorChange != 0.0f )
    {
        // Continue color scheme change
        if( m_fTime > (m_fTimeStartColorChange + 1.0f) )
        {
            // Conclude color scheme change
            m_fTimeStartColorChange = 0.0f;
            m_iColorSchemePrev = -1;
        }
        else
        {
            // For first second after color change, do linear 
            // transition from old to new color scheme
            FLOAT fBeta = m_fTime - m_fTimeStartColorChange; // varies 0.0 to 1.0
            D3DXCOLOR colOld1, colOld2, colOld3; // colors using old scheme
            D3DXCOLOR colNew1, colNew2, colNew3; // colors using new scheme
            GenerateColors( m_iColorSchemePrev, m_bBrightColors, &colOld1, &colOld2, &colOld3 );
            GenerateColors( m_iColorScheme, m_bBrightColors, &colNew1, &colNew2, &colNew3 );
            D3DXColorLerp( &m_col1, &colOld1, &colNew1, fBeta );
            D3DXColorLerp( &m_col2, &colOld2, &colNew2, fBeta );
            D3DXColorLerp( &m_col3, &colOld3, &colNew3, fBeta );
        }
    }
    else
    {
        // No color scheme change is active, so compute colors as usual
        GenerateColors( m_iColorScheme, m_bBrightColors, &m_col1, &m_col2, &m_col3 );
    }

    if( m_fTimeNextTextureChange == 0.0f )
    {
        // Schedule next texture change
        m_fTimeNextTextureChange = m_fTime + m_dwTextureInterval;
    }
    else if( m_fTimeNextTextureChange < m_fTime )
    {
        // Start texture change
        m_iTexturePrev = m_iTexture;
        m_fTimeStartTextureChange = m_fTime;
        m_fTimeNextTextureChange = 0.0f;
    }
    if( m_fTimeStartTextureChange != 0.0f )
    {
        // Continue texture change
        FLOAT fDelta = m_fTime - m_fTimeStartTextureChange; // varies 0.0 to 1.0

        // After 1 second, change textures if you haven't already
        if( fDelta > 1.0f && m_iTexture == m_iTexturePrev)
        {
            Randomize( &m_iTexture, 4 );
        }

        if( fDelta < 1.0f )
        {
            // For first second after change starts, fade colors from normal to black
            D3DXColorLerp( &m_col1, &m_col1, &colBlack, fDelta );
            D3DXColorLerp( &m_col2, &m_col2, &colBlack, fDelta );
            D3DXColorLerp( &m_col3, &m_col3, &colBlack, fDelta );
        }
        else if( fDelta < 2.0f )
        {
            // For second second after change starts, just show black
            m_col1 = colBlack;
            m_col2 = colBlack;
            m_col3 = colBlack;
        }
        else if( fDelta < 3.0f )
        {
            // For third second after change starts, fade colors from black to normal
            D3DXColorLerp( &m_col1, &colBlack, &m_col1, fDelta - 2.0f );
            D3DXColorLerp( &m_col2, &colBlack, &m_col2, fDelta - 2.0f );
            D3DXColorLerp( &m_col3, &colBlack, &m_col3, fDelta - 2.0f );
        }
        else
        {
            // transition done
            m_fTimeStartTextureChange = 0.0f;
            m_iTexturePrev = -1;
        }
    }

    if( m_fTimeNextMeshChange == 0.0f )
    {
        // Schedule next mesh change
        m_fTimeNextMeshChange = m_fTime + m_dwMeshInterval;
    }
    else if( m_fTimeNextMeshChange < m_fTime )
    {
        // Start mesh change
        m_iMeshPrev = m_iMesh;
        m_fTimeStartMeshChange = m_fTime;
        m_fTimeNextMeshChange = 0.0f;
    }
    if( m_fTimeStartMeshChange != 0.0f )
    {
        // Continue mesh change
        FLOAT fDelta = m_fTime - m_fTimeStartMeshChange;

        // After 1 second, change meshes if you haven't already
        if( fDelta > 1.0f && m_iMesh == m_iMeshPrev)
        {
            Randomize( &m_iMesh, 3 );
        }

        if( fDelta < 1.0f )
        {
            // For first second after change starts, fade colors from normal to black
            D3DXColorLerp( &m_col1, &m_col1, &colBlack, fDelta );
            D3DXColorLerp( &m_col2, &m_col2, &colBlack, fDelta );
            D3DXColorLerp( &m_col3, &m_col3, &colBlack, fDelta );
        }
        else if( fDelta < 2.0f )
        {
            // For second second after change starts, just show black
            m_col1 = colBlack;
            m_col2 = colBlack;
            m_col3 = colBlack;
        }
        else if( fDelta < 3.0f )
        {
            // For third second after change starts, fade colors from black to normal
            D3DXColorLerp( &m_col1, &colBlack, &m_col1, fDelta - 2.0f );
            D3DXColorLerp( &m_col2, &colBlack, &m_col2, fDelta - 2.0f );
            D3DXColorLerp( &m_col3, &colBlack, &m_col3, fDelta - 2.0f );
        }
        else
        {
            // transition done
            m_fTimeStartMeshChange = 0.0f;
            m_iMeshPrev = -1;
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GenerateColors()
// Desc: 
//-----------------------------------------------------------------------------
VOID CMoireScreensaver::GenerateColors( INT iColorScheme, BOOL bBright, 
    D3DXCOLOR* pcol1, D3DXCOLOR* pcol2, D3DXCOLOR* pcol3 )
{
    FLOAT fWave1;
    FLOAT fWave2;
    FLOAT fConstant;

    if( bBright )
    {
        fWave1 = 1.0f * (1.0f + sinf(m_fTime))/2;
        fWave2 = 0.5f * (1.0f + cosf(m_fTime))/2;
        fConstant = 0.5f;
    }
    else
    {
        fWave1 = 0.50f + 0.50f * (1.0f + sinf(m_fTime))/2;
        fWave2 = 0.25f + 0.25f * (1.0f + cosf(m_fTime))/2;
        fConstant = 0.375f;
    }

    switch( iColorScheme )
    {
    case 0:
        *pcol1 = D3DXCOLOR( fWave1, fWave2, 0.0f, 1.0f );
        *pcol2 = D3DXCOLOR( fWave2, fWave1, 0.0f, 1.0f );
        *pcol3 = D3DXCOLOR( 0.0f, 0.0f, fConstant, 1.0f );
        break;

    case 1:
        *pcol1 = D3DXCOLOR( 0.0f, fWave1, fWave2, 1.0f );
        *pcol2 = D3DXCOLOR( 0.0f, fWave2, fWave1, 1.0f );
        *pcol3 = D3DXCOLOR( fConstant, 0.0f, 0.0f, 1.0f );
        break;

    case 2:
        *pcol1 = D3DXCOLOR( fWave1, 0.0f, fWave2, 1.0f );
        *pcol2 = D3DXCOLOR( fWave2, 0.0f, fWave1, 1.0f );
        *pcol3 = D3DXCOLOR( 0.0f, fConstant, 0.0f, 1.0f );
        break;

    case 3:
        *pcol1 = D3DXCOLOR( fWave1, fWave1, fWave1, 1.0f );
        *pcol2 = D3DXCOLOR( fWave2, fWave2, fWave2, 1.0f );
        *pcol3 = D3DXCOLOR( 0.0f, 0.0f, 0.0f, 1.0f );
        break;
    }
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT CMoireScreensaver::Render()
{
    D3DXMATRIX mat1;
    D3DXMATRIX mat2;
    D3DXMATRIX mat3;

    SetProjectionMatrix( 0.05f, 120.0f );

    m_pd3dDevice->SetTexture(0, m_pDeviceObjects->m_pTexArray[m_iTexture] );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
//    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU,  D3DTADDRESS_WRAP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV,  D3DTADDRESS_WRAP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );

    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING,            TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,    TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,            D3DBLEND_SRCALPHA );
    m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,           D3DBLEND_ONE );

    HRESULT hr;
    DWORD dw;
    hr = m_pd3dDevice->ValidateDevice(&dw);

    m_pd3dDevice->SetVertexShader( D3DFVF_MYVERTEX );
    m_pd3dDevice->SetStreamSource( 0, m_pDeviceObjects->m_pVBArray[m_iMesh], sizeof(MYVERTEX) );

    // Clear the viewport
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET, m_col3, 1.0f, 0L );

    // Begin the scene 
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {

        D3DXMatrixScaling( &mat1, m_fScale1, m_fScale1, 1.0 );
        D3DXMatrixRotationZ( &mat2, m_fRot1 );
        D3DXMatrixMultiply( &mat3, &mat1, &mat2 );
        m_pd3dDevice->SetTransform( D3DTS_TEXTURE0, &mat3 );
//        m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, m_col1 );
        D3DMATERIAL8 mtrl;
        mtrl.Diffuse = m_col1;
        mtrl.Ambient = m_col1;
        mtrl.Specular = m_col1;
        mtrl.Emissive = m_col1;
        mtrl.Power = 0;
        m_pd3dDevice->SetMaterial(&mtrl);

        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, m_dwNumVerticesArray[m_iMesh] / 3 );

        D3DXMatrixScaling( &mat1, m_fScale2, m_fScale2, 1.0 );
        D3DXMatrixRotationZ( &mat2, m_fRot2 );
        D3DXMatrixMultiply( &mat3, &mat1, &mat2 );
        m_pd3dDevice->SetTransform( D3DTS_TEXTURE0, &mat3 );
//        m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, m_col2 );
        mtrl.Diffuse = m_col2;
        mtrl.Ambient = m_col2;
        mtrl.Specular = m_col2;
        mtrl.Emissive = m_col2;
        mtrl.Power = 0;
        m_pd3dDevice->SetMaterial(&mtrl);

        m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, m_dwNumVerticesArray[m_iMesh] / 3 );

        // Show frame rate
        m_pDeviceObjects->m_pStatsFont->DrawText( 3,  1, D3DCOLOR_ARGB(255,0,0,0), m_strFrameStats );
        m_pDeviceObjects->m_pStatsFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );

        m_pDeviceObjects->m_pStatsFont->DrawText( 3, 21, D3DCOLOR_ARGB(255,0,0,0), m_strDeviceStats );
        m_pDeviceObjects->m_pStatsFont->DrawText( 2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );

        // End the scene.
        m_pd3dDevice->EndScene();
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMoireScreensaver::RestoreDeviceObjects()
{
    if( m_pd3dDevice == NULL )
        return S_OK;
    
    // Set up sensible projection and view matrices
    D3DXMATRIX  view;
    D3DXMatrixLookAtLH( &view , &D3DXVECTOR3(0,0,-10) , &D3DXVECTOR3(0,0,0) , &D3DXVECTOR3(0,1,0) );
    m_pd3dDevice->SetTransform( D3DTS_VIEW , &view );

    // Set some basic renderstates
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE , TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE , FALSE );

    if( FAILED( BuildMeshes() ) )
        return E_FAIL;

    if( FAILED( BuildTextures() ) )
        return E_FAIL;

    m_pDeviceObjects->m_pStatsFont = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pDeviceObjects->m_pStatsFont->InitDeviceObjects( m_pd3dDevice );
    m_pDeviceObjects->m_pStatsFont->RestoreDeviceObjects();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMoireScreensaver::InvalidateDeviceObjects()
{
    m_pDeviceObjects->m_pStatsFont->InvalidateDeviceObjects();
    m_pDeviceObjects->m_pStatsFont->DeleteDeviceObjects();
    SAFE_DELETE( m_pDeviceObjects->m_pStatsFont );

    SAFE_RELEASE( m_pDeviceObjects->m_pVBArray[0] );
    SAFE_RELEASE( m_pDeviceObjects->m_pVBArray[1] );
    SAFE_RELEASE( m_pDeviceObjects->m_pVBArray[2] );

    SAFE_RELEASE( m_pDeviceObjects->m_pTexArray[0] );
    SAFE_RELEASE( m_pDeviceObjects->m_pTexArray[1] );
    SAFE_RELEASE( m_pDeviceObjects->m_pTexArray[2] );
    SAFE_RELEASE( m_pDeviceObjects->m_pTexArray[3] );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ReadSettings()
// Desc: 
//-----------------------------------------------------------------------------
VOID CMoireScreensaver::ReadSettings()
{
    ReadRegistry();
}




//-----------------------------------------------------------------------------
// Name: ConfigureDialogProcHelper()
// Desc: 
//-----------------------------------------------------------------------------
INT_PTR CALLBACK ConfigureDialogProcHelper( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    return g_pMyMoireScreensaver->ConfigureDialogProc( hwndDlg, uMsg, wParam, lParam );
}




//-----------------------------------------------------------------------------
// Name: DoConfig()
// Desc: 
//-----------------------------------------------------------------------------
VOID CMoireScreensaver::DoConfig()
{
    DialogBox( NULL, MAKEINTRESOURCE(IDD_CONFIGURE), m_hWndParent, ConfigureDialogProcHelper );
}




//-----------------------------------------------------------------------------
// Name: ConfigureDialogProc()
// Desc: 
//-----------------------------------------------------------------------------
INT_PTR CALLBACK CMoireScreensaver::ConfigureDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    HWND hwndSpeed = GetDlgItem( hwndDlg, IDC_SPEED );
    HWND hwndIntense = GetDlgItem( hwndDlg, IDC_INTENSE );
    HWND hwndPastel = GetDlgItem( hwndDlg, IDC_PASTEL );
    HWND hwndColorInterval = GetDlgItem( hwndDlg, IDC_COLORINTERVAL );
    HWND hwndTextureInterval = GetDlgItem( hwndDlg, IDC_TEXTUREINTERVAL );
    HWND hwndMeshInterval = GetDlgItem( hwndDlg, IDC_MESHINTERVAL );
    TCHAR sz[10];
    DWORD iSpeed;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        WriteRegistry();
        SendMessage(hwndSpeed, TBM_SETRANGE, FALSE, MAKELONG(0, 5));
        if( m_fSpeed == 0.25f )
            SendMessage(hwndSpeed, TBM_SETPOS, TRUE, 0);
        else if( m_fSpeed == 0.5f )
            SendMessage(hwndSpeed, TBM_SETPOS, TRUE, 1);
        else if( m_fSpeed == 0.75f )
            SendMessage(hwndSpeed, TBM_SETPOS, TRUE, 2);
        else if( m_fSpeed == 1.0f )
            SendMessage(hwndSpeed, TBM_SETPOS, TRUE, 3);
        else if( m_fSpeed == 1.5f )
            SendMessage(hwndSpeed, TBM_SETPOS, TRUE, 4);
        else if( m_fSpeed == 2.0f )
            SendMessage(hwndSpeed, TBM_SETPOS, TRUE, 5);

        if( m_bBrightColors )
            CheckRadioButton( hwndDlg, IDC_INTENSE, IDC_PASTEL, IDC_INTENSE );
        else
            CheckRadioButton( hwndDlg, IDC_INTENSE, IDC_PASTEL, IDC_PASTEL );

        wsprintf(sz, TEXT("%d"), m_dwColorInterval);
        SetWindowText(hwndColorInterval, sz);

        wsprintf(sz, TEXT("%d"), m_dwTextureInterval);
        SetWindowText(hwndTextureInterval, sz);

        wsprintf(sz, TEXT("%d"), m_dwMeshInterval);
        SetWindowText(hwndMeshInterval, sz);
        return TRUE;

    case WM_COMMAND:
        switch( LOWORD( wParam ) )
        {
        case IDC_COLORINTERVAL:
            if( HIWORD( wParam ) == EN_CHANGE )
            {
                m_fTimeNextColorChange = 0.0f;
                GetWindowText(hwndColorInterval, sz, 10);
                sscanf(sz, TEXT("%d"), &m_dwColorInterval);
            }
            break;

        case IDC_MESHINTERVAL:
            if( HIWORD( wParam ) == EN_CHANGE )
            {
                m_fTimeNextMeshChange = 0.0f;
                GetWindowText(hwndMeshInterval, sz, 10);
                sscanf(sz, TEXT("%d"), &m_dwMeshInterval);
            }
            break;

        case IDC_TEXTUREINTERVAL:
            if( HIWORD( wParam ) == EN_CHANGE )
            {
                m_fTimeNextTextureChange = 0.0f;
                GetWindowText(hwndTextureInterval, sz, 10);
                sscanf(sz, TEXT("%d"), &m_dwTextureInterval);
            }
            break;

        case IDC_SCREENSETTINGS:
            DoScreenSettingsDialog( hwndDlg );
            break;

        case IDOK:
            m_bBrightColors = ( IsDlgButtonChecked( hwndDlg, IDC_INTENSE ) == BST_CHECKED );
            iSpeed = (DWORD)SendMessage( hwndSpeed, TBM_GETPOS, 0, 0 );
            if( iSpeed == 0 )
                m_fSpeed = 0.25f;
            else if( iSpeed == 1 )
                m_fSpeed = 0.5f;
            else if( iSpeed == 2 )
                m_fSpeed = 0.75f;
            else if( iSpeed == 3 )
                m_fSpeed = 1.0f;
            else if( iSpeed == 4 )
                m_fSpeed = 1.5f;
            else if( iSpeed == 5 )
                m_fSpeed = 2.0f;
            WriteRegistry(); // save new settings
            EndDialog(hwndDlg, IDOK);
            break;

        case IDCANCEL:
            ReadRegistry(); // restore previous settings
            EndDialog(hwndDlg, IDCANCEL);
            break;
        }
        return TRUE;
    
    default:
        return FALSE;
    }
}




//-----------------------------------------------------------------------------
// Name: ReadRegistry()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMoireScreensaver::ReadRegistry()
{
    HKEY hkey;
    DWORD dwType = REG_DWORD;
    DWORD dwLength = sizeof(DWORD);

    if( ERROR_SUCCESS == RegCreateKeyEx( HKEY_CURRENT_USER, m_strRegPath, 
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL ) )
    {
        RegQueryValueEx( hkey, TEXT("Bright Colors"), NULL, &dwType, (BYTE*)&m_bBrightColors, &dwLength );

        RegQueryValueEx( hkey, TEXT("Mesh"), NULL, &dwType, (BYTE*)&m_iMesh, &dwLength);
        if( m_iMesh < 0 )
            m_iMesh = 0;
        else if( m_iMesh > 2 )
            m_iMesh = 2;

        RegQueryValueEx( hkey, TEXT("Texture"), NULL, &dwType, (BYTE*)&m_iTexture, &dwLength);
        if( m_iTexture < 0 )
            m_iTexture = 0;
        else if( m_iTexture > 3 )
            m_iTexture = 3;

        RegQueryValueEx( hkey, TEXT("Color Scheme"), NULL, &dwType, (BYTE*)&m_iColorScheme, &dwLength);
        if( m_iColorScheme < 0 )
            m_iColorScheme = 0;
        else if( m_iColorScheme > 3 )
            m_iColorScheme = 3;

        DWORD iSpeed;
        RegQueryValueEx( hkey, TEXT("Speed"), NULL, &dwType, (BYTE*)&iSpeed, &dwLength);
        if( iSpeed > 5 )
            iSpeed = 5;

        if( iSpeed == 0 )
            m_fSpeed = 0.25f;
        else if( iSpeed == 1 )
            m_fSpeed = 0.5f;
        else if( iSpeed == 2 )
            m_fSpeed = 0.75f;
        else if( iSpeed == 3 )
            m_fSpeed = 1.0f;
        else if( iSpeed == 4 )
            m_fSpeed = 1.5f;
        else if( iSpeed == 5 )
            m_fSpeed = 2.0f;

        RegQueryValueEx( hkey, TEXT("Mesh Interval"), NULL, &dwType, (BYTE*)&m_dwMeshInterval, &dwLength);
        if( m_dwMeshInterval > 9999 )
            m_dwMeshInterval = 9999;

        RegQueryValueEx( hkey, TEXT("Color Interval"), NULL, &dwType, (BYTE*)&m_dwColorInterval, &dwLength);
        if( m_dwColorInterval > 9999 )
            m_dwColorInterval = 9999;

        RegQueryValueEx( hkey, TEXT("Texture Interval"), NULL, &dwType, (BYTE*)&m_dwTextureInterval, &dwLength);
        if( m_dwTextureInterval > 9999 )
            m_dwTextureInterval = 9999;

        ReadScreenSettings( hkey );

        RegCloseKey( hkey );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: WriteRegistry()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT CMoireScreensaver::WriteRegistry()
{
    HKEY hkey;

    if( ERROR_SUCCESS == RegCreateKeyEx( HKEY_CURRENT_USER, m_strRegPath, 
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL ) )
    {
        RegSetValueEx( hkey, TEXT("Bright Colors"), NULL, REG_DWORD, (BYTE*)&m_bBrightColors, sizeof(DWORD) );

        RegSetValueEx( hkey, TEXT("Mesh"), NULL, REG_DWORD, (BYTE*)&m_iMesh, sizeof(DWORD) );

        RegSetValueEx( hkey, TEXT("Texture"), NULL, REG_DWORD, (BYTE*)&m_iTexture, sizeof(DWORD));

        RegSetValueEx( hkey, TEXT("Color Scheme"), NULL, REG_DWORD, (BYTE*)&m_iColorScheme, sizeof(DWORD));

        DWORD iSpeed;
        if( m_fSpeed == 0.25f )
            iSpeed = 0;
        else if( m_fSpeed == 0.5f )
            iSpeed = 1;
        else if( m_fSpeed == 0.75f )
            iSpeed = 2;
        else if( m_fSpeed == 1.0f )
            iSpeed = 3;
        else if( m_fSpeed == 1.5f )
            iSpeed = 4;
        else if( m_fSpeed == 2.0f )
            iSpeed = 5;
        RegSetValueEx( hkey, TEXT("Speed"), NULL, REG_DWORD, (BYTE*)&iSpeed, sizeof(DWORD));

        RegSetValueEx( hkey, TEXT("Mesh Interval"), NULL, REG_DWORD, (BYTE*)&m_dwMeshInterval, sizeof(DWORD));

        RegSetValueEx( hkey, TEXT("Color Interval"), NULL, REG_DWORD, (BYTE*)&m_dwColorInterval, sizeof(DWORD));

        RegSetValueEx( hkey, TEXT("Texture Interval"), NULL, REG_DWORD, (BYTE*)&m_dwTextureInterval, sizeof(DWORD));

        WriteScreenSettings( hkey );

        RegCloseKey( hkey );
    }

    return S_OK;
}
