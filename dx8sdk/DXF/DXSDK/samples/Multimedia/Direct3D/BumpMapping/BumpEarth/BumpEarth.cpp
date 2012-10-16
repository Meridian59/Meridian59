//-----------------------------------------------------------------------------
// File: BumpEarth.cpp
//
// Desc: Direct3D environment mapping / bump mapping sample. The technique
//       used perturbs the environment map to simulate bump mapping.
//
//
// Copyright (c) 1997-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <math.h>
#include <stdio.h>
#include <D3DX8.h>
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DUtil.h"
#include "DXUtil.h"
#include "resource.h"




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------

// Vertex with 2nd set of tex coords (for bumpmapped environment map)
struct BUMPVERTEX
{
    D3DXVECTOR3 p;
    D3DXVECTOR3 n;
    FLOAT       tu1, tv1;
    FLOAT       tu2, tv2;
};

#define D3DFVF_BUMPVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2)

// Converts a FLOAT to a DWORD for use in SetRenderState() calls
inline DWORD F2DW( FLOAT f ) { return *((DWORD*)&f); }




//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    CD3DFont*     m_pFont;                  // A font to output text

    CD3DArcBall   m_ArcBall;                // ArcBall used for mouse input

    LPDIRECT3DTEXTURE8 m_pBlockTexture;     // A blank, gray texture
    LPDIRECT3DTEXTURE8 m_pEarthTexture;     // The Earth texture
    LPDIRECT3DTEXTURE8 m_pEnvMapTexture;    // The environment map
    LPDIRECT3DTEXTURE8 m_pEarthBumpTexture; // Source for the bumpmap
    LPDIRECT3DTEXTURE8 m_psBumpMap;         // The actual bumpmap

    D3DFORMAT     m_BumpMapFormat;         // Bumpmap texture format
    LPDIRECT3DVERTEXBUFFER8 m_pEarthVB;    // Geometry for the Earth
    DWORD         m_dwNumSphereVertices;

    BOOL          m_bHighTesselation;      // User options
    BOOL          m_bTextureOn;
    BOOL          m_bBumpMapOn;
    BOOL          m_bEnvMapOn;
    BOOL          m_bDeviceValidationFailed;

    // Internal functions
    VOID    SetMenuStates();
    HRESULT CreateEarthVertexBuffer();
    VOID    ApplyEnvironmentMap();
    HRESULT InitBumpMap();

protected:
    HRESULT OneTimeSceneInit();
    HRESULT InitDeviceObjects();
    HRESULT RestoreDeviceObjects();
    HRESULT InvalidateDeviceObjects();
    HRESULT DeleteDeviceObjects();
    HRESULT Render();
    HRESULT FrameMove();
    HRESULT FinalCleanup();
    HRESULT ConfirmDevice( D3DCAPS8*, DWORD, D3DFORMAT );

public:
    CMyD3DApplication();

    LRESULT MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
};




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything, and goes into a
//       message-processing loop. Idle time is used to render the scene.
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
    CMyD3DApplication d3dApp;

    if( FAILED( d3dApp.Create( hInst ) ) )
        return 0;

    return d3dApp.Run();
}




//-----------------------------------------------------------------------------
// Name: CMyD3DApplication()
// Desc: Application constructor. Sets attributes for the app.
//-----------------------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
    m_strWindowTitle    = _T("BumpEarth: Direct3D BumpMapping Demo");
    m_bUseDepthBuffer   = TRUE;
    m_bShowCursorWhenFullscreen = TRUE;

    m_psBumpMap         = NULL;
    m_bTextureOn        = TRUE;
    m_bBumpMapOn        = TRUE;
    m_bEnvMapOn         = TRUE;
    m_bHighTesselation  = TRUE;

    m_pBlockTexture     = NULL;
    m_pEarthTexture     = NULL;
    m_pEarthBumpTexture = NULL;
    m_pEnvMapTexture    = NULL;
    m_bDeviceValidationFailed = FALSE;

    m_pFont             = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_pEarthVB          = NULL;
}




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
    // Set cursor to indicate that user can move the object with the mouse
#ifdef _WIN64
    SetClassLongPtr( m_hWnd, GCLP_HCURSOR, (LONG_PTR)LoadCursor( NULL, IDC_SIZEALL ) );
#else
    SetClassLong( m_hWnd, GCL_HCURSOR, (LONG)LoadCursor( NULL, IDC_SIZEALL ) );
#endif
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ApplyEnvironmentMap()
// Desc: Performs a calculation on each of the vertices' normals to determine
//       what the texture coordinates should be for the environment map.
//-----------------------------------------------------------------------------
VOID CMyD3DApplication::ApplyEnvironmentMap()
{
    // Get the World-View(WV) matrix set
    D3DXMATRIX matWorld, matView, matWorldView;
    m_pd3dDevice->GetTransform( D3DTS_WORLD, &matWorld );
    m_pd3dDevice->GetTransform( D3DTS_VIEW,  &matView );
    D3DXMatrixMultiply( &matWorldView, &matWorld, &matView );

    // Lock the vertex buffer
    BUMPVERTEX* vtx;
    m_pEarthVB->Lock( 0, 0, (BYTE**)&vtx, 0 );

    // Establish constants used in sphere generation
    DWORD dwNumSphereRings    = m_bHighTesselation ? 15 :  5;
    DWORD dwNumSphereSegments = m_bHighTesselation ? 30 : 10;
    FLOAT fDeltaRingAngle = ( D3DX_PI / dwNumSphereRings );
    FLOAT fDeltaSegAngle  = ( 2.0f * D3DX_PI / dwNumSphereSegments );

    D3DXVECTOR4 vT;
    FLOAT fScale;

    // Generate the group of rings for the sphere
    for( DWORD ring = 0; ring < dwNumSphereRings; ring++ )
    {
        FLOAT r0 = sinf( (ring+0) * fDeltaRingAngle );
        FLOAT r1 = sinf( (ring+1) * fDeltaRingAngle );
        FLOAT y0 = cosf( (ring+0) * fDeltaRingAngle );
        FLOAT y1 = cosf( (ring+1) * fDeltaRingAngle );

        // Generate the group of segments for the current ring
        for( DWORD seg = 0; seg < (dwNumSphereSegments+1); seg++ )
        {
            FLOAT x0 =  r0 * sinf( seg * fDeltaSegAngle );
            FLOAT z0 =  r0 * cosf( seg * fDeltaSegAngle );
            FLOAT x1 =  r1 * sinf( seg * fDeltaSegAngle );
            FLOAT z1 =  r1 * cosf( seg * fDeltaSegAngle );

            // Add two vertices to the strip which makes up the sphere
            // (using the transformed normal to generate texture coords)
            (*vtx).p   = (*vtx).n   = D3DXVECTOR3(x0,y0,z0);
            (*vtx).tu1 = (*vtx).tu2 = -((FLOAT)seg)/dwNumSphereSegments;
            (*vtx).tv1 = (*vtx).tv2 = (ring+0)/(FLOAT)dwNumSphereRings;
            D3DXVec3Transform( &vT, &(*vtx).n, &matWorldView );
            fScale = 1.37f / D3DXVec4Length( &vT );
            (*vtx).tu1 = 0.5f + fScale*vT.x;
            (*vtx).tv1 = 0.5f - fScale*vT.y;
            vtx++;

            (*vtx).p   = (*vtx).n   = D3DXVECTOR3(x1,y1,z1);
            (*vtx).tu1 = (*vtx).tu2 = -((FLOAT)seg)/dwNumSphereSegments;
            (*vtx).tv1 = (*vtx).tv2 = (ring+1)/(FLOAT)dwNumSphereRings;
            D3DXVec3Transform( &vT, &(*vtx).n, &matWorldView );
            fScale = 1.37f / D3DXVec4Length( &vT );
            (*vtx).tu1 = 0.5f + fScale*vT.x;
            (*vtx).tv1 = 0.5f - fScale*vT.y;
            vtx++;
        }
    }

    m_pEarthVB->Unlock();
}




//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Animates the scene
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    // Update the Earth's rotation angle
    static FLOAT fRotationAngle = 0.0f;
    if( FALSE == m_ArcBall.IsBeingDragged() )
        fRotationAngle += m_fElapsedTime;

    // Setup viewing postion from ArcBall
    D3DXMATRIX matWorld;
    D3DXMatrixRotationY( &matWorld, -fRotationAngle );
    D3DXMatrixMultiply( &matWorld, &matWorld, m_ArcBall.GetRotationMatrix() );
    D3DXMatrixMultiply( &matWorld, &matWorld, m_ArcBall.GetTranslationMatrix() );
    m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );

    D3DXMATRIX  matView;
    D3DXVECTOR3 vEyePt    = D3DXVECTOR3( 0.0f, 0.0f, -3.0f );
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
    D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
    m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

    // Apply the environment map
    ApplyEnvironmentMap();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Renders the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
    DWORD dwNumPasses;

    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                         0x00000000, 1.0f, 0L );

    if( FAILED( m_pd3dDevice->BeginScene() ) )
        return S_OK; // Don't return a "fatal" error

    m_pd3dDevice->SetRenderState( D3DRS_WRAP0, D3DWRAP_U | D3DWRAP_V );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

    if( m_bTextureOn )
        m_pd3dDevice->SetTexture( 0, m_pEarthTexture );
    else
        m_pd3dDevice->SetTexture( 0, m_pBlockTexture );

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

    if( m_bBumpMapOn && m_bEnvMapOn )
    {
        m_pd3dDevice->SetTexture( 1, m_psBumpMap );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1 );
        if( m_BumpMapFormat == D3DFMT_V8U8 )
            m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_BUMPENVMAP );
        else
            m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_BUMPENVMAPLUMINANCE );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_BUMPENVMAT00, F2DW(0.5f) );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_BUMPENVMAT01, F2DW(0.0f) );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_BUMPENVMAT10, F2DW(0.0f) );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_BUMPENVMAT11, F2DW(0.5f) );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_BUMPENVLSCALE, F2DW(4.0f) );
        m_pd3dDevice->SetTextureStageState( 1, D3DTSS_BUMPENVLOFFSET, F2DW(0.0f) );

        if( m_bEnvMapOn )
        {
            m_pd3dDevice->SetTexture( 2, m_pEnvMapTexture );
            m_pd3dDevice->SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 0 );
            m_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_ADD );
            m_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            m_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );
        }
        else
            m_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
    }
    else
    {
        if( m_bEnvMapOn )
        {
            m_pd3dDevice->SetTexture( 1, m_pEnvMapTexture );
            m_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
            m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD );
            m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
            m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
        }
        else
            m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

        m_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
    }

    m_pd3dDevice->SetStreamSource( 0, m_pEarthVB, sizeof(BUMPVERTEX) );
    m_pd3dDevice->SetVertexShader( D3DFVF_BUMPVERTEX );

    if( FAILED( m_pd3dDevice->ValidateDevice( &dwNumPasses ) ) )
    {
        // The right thing to do when device validation fails is to try
        // a different rendering technique.  This sample just warns the user.
        m_bDeviceValidationFailed = TRUE;
    }
    else
    {
        m_bDeviceValidationFailed = FALSE;
    }

    // Finally, draw the Earth
    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, m_dwNumSphereVertices-2 );

    // Restore texture stage states
    m_pd3dDevice->SetTexture( 0, NULL );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
    m_pd3dDevice->SetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_DISABLE );

    // Output statistics
    m_pFont->DrawText( 2,  0, D3DCOLOR_ARGB(255,255,255,0), m_strFrameStats );
    m_pFont->DrawText( 2, 20, D3DCOLOR_ARGB(255,255,255,0), m_strDeviceStats );

    if( m_bDeviceValidationFailed )
    {
        m_pFont->DrawText( 2, 40, D3DCOLOR_ARGB(255,255,0,0), 
            _T("Warning: Device validation failed.  Rendering may not look right.") );
    }

    m_pd3dDevice->EndScene();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitBumpMap()
// Desc: Converts data from m_pEarthBumpTexture into the type of bump map requested
//       as m_BumpMapFormat into m_psBumpMap.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitBumpMap()
{
    LPDIRECT3DTEXTURE8 psBumpSrc = m_pEarthBumpTexture;
    D3DSURFACE_DESC    d3dsd;
    D3DLOCKED_RECT     d3dlr;

    psBumpSrc->GetLevelDesc( 0, &d3dsd );
    // Create the bumpmap's surface and texture objects
    if( FAILED( m_pd3dDevice->CreateTexture( d3dsd.Width, d3dsd.Height, 1, 0, 
        m_BumpMapFormat, D3DPOOL_MANAGED, &m_psBumpMap ) ) )
    {
        return E_FAIL;
    }

    // Fill the bits of the new texture surface with bits from
    // a private format.
    psBumpSrc->LockRect( 0, &d3dlr, 0, 0 );
    DWORD dwSrcPitch = (DWORD)d3dlr.Pitch;
    BYTE* pSrcTopRow = (BYTE*)d3dlr.pBits;
    BYTE* pSrcCurRow = pSrcTopRow;
    BYTE* pSrcBotRow = pSrcTopRow + (dwSrcPitch * (d3dsd.Height - 1) );

    m_psBumpMap->LockRect( 0, &d3dlr, 0, 0 );
    DWORD dwDstPitch = (DWORD)d3dlr.Pitch;
    BYTE* pDstTopRow = (BYTE*)d3dlr.pBits;
    BYTE* pDstCurRow = pDstTopRow;
    BYTE* pDstBotRow = pDstTopRow + (dwDstPitch * (d3dsd.Height - 1) );

    for( DWORD y=0; y<d3dsd.Height; y++ )
    {
        BYTE* pSrcB0; // addr of current pixel
        BYTE* pSrcB1; // addr of pixel below current pixel, wrapping to top if necessary
        BYTE* pSrcB2; // addr of pixel above current pixel, wrapping to bottom if necessary
        BYTE* pDstT;  // addr of dest pixel;

        pSrcB0 = pSrcCurRow;

        if( y == d3dsd.Height - 1)
            pSrcB1 = pSrcTopRow;
        else
            pSrcB1 = pSrcCurRow + dwSrcPitch;

        if( y == 0 )
            pSrcB2 = pSrcBotRow;
        else
            pSrcB2 = pSrcCurRow - dwSrcPitch;

        pDstT = pDstCurRow;

        for( DWORD x=0; x<d3dsd.Width; x++ )
        {
            LONG v00; // Current pixel
            LONG v01; // Pixel to the right of current pixel, wrapping to left edge if necessary
            LONG vM1; // Pixel to the left of current pixel, wrapping to right edge if necessary
            LONG v10; // Pixel one line below.
            LONG v1M; // Pixel one line above.

            v00 = *(pSrcB0+0);
            
            if( x == d3dsd.Width - 1 )
                v01 = *(pSrcCurRow);
            else
                v01 = *(pSrcB0+4);
            
            if( x == 0 )
                vM1 = *(pSrcCurRow + (4 * (d3dsd.Width - 1)));
            else
                vM1 = *(pSrcB0-4);
            v10 = *(pSrcB1+0);
            v1M = *(pSrcB2+0);

            LONG iDu = (vM1-v01); // The delta-u bump value
            LONG iDv = (v1M-v10); // The delta-v bump value

            // The luminance bump value (land masses are less shiny)
            WORD uL = ( v00>1 ) ? 63 : 127;

            switch( m_BumpMapFormat )
            {
                case D3DFMT_V8U8:
                    *pDstT++ = (BYTE)iDu;
                    *pDstT++ = (BYTE)iDv;
                    break;

                case D3DFMT_L6V5U5:
                    *(WORD*)pDstT  = (WORD)( ( (iDu>>3) & 0x1f ) <<  0 );
                    *(WORD*)pDstT |= (WORD)( ( (iDv>>3) & 0x1f ) <<  5 );
                    *(WORD*)pDstT |= (WORD)( ( ( uL>>2) & 0x3f ) << 10 );
                    pDstT += 2;
                    break;

                case D3DFMT_X8L8V8U8:
                    *pDstT++ = (BYTE)iDu;
                    *pDstT++ = (BYTE)iDv;
                    *pDstT++ = (BYTE)uL;
                    *pDstT++ = (BYTE)0L;
                    break;
            }

            // Move one pixel to the right (src is 32-bpp)
            pSrcB0+=4;
            pSrcB1+=4;
            pSrcB2+=4;
        }

        // Move to the next line
        pSrcCurRow += dwSrcPitch;
        pDstCurRow += dwDstPitch;
    }

    m_psBumpMap->UnlockRect(0);
    psBumpSrc->UnlockRect(0);

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    m_pFont->InitDeviceObjects( m_pd3dDevice );

    if( FAILED( D3DUtil_CreateTexture( m_pd3dDevice, _T("Block.bmp"),
                                       &m_pBlockTexture, D3DFMT_R5G6B5 ) ) )
        return D3DAPPERR_MEDIANOTFOUND;
    if( FAILED( D3DUtil_CreateTexture( m_pd3dDevice, _T("Earth.bmp"),
                                       &m_pEarthTexture, D3DFMT_R5G6B5 ) ) )
        return D3DAPPERR_MEDIANOTFOUND;
    if( FAILED( D3DUtil_CreateTexture( m_pd3dDevice, _T("Earthbump.bmp"),
                                       &m_pEarthBumpTexture, D3DFMT_A8R8G8B8 ) ) )
        return D3DAPPERR_MEDIANOTFOUND;
    if( FAILED( D3DUtil_CreateTexture( m_pd3dDevice, _T("EarthEnvMap.bmp"),
                                       &m_pEnvMapTexture, D3DFMT_R5G6B5 ) ) )
        return D3DAPPERR_MEDIANOTFOUND;

    // Find out which bump map texture are supported by this device
    BOOL bCanDoV8U8   = SUCCEEDED( m_pD3D->CheckDeviceFormat( m_d3dCaps.AdapterOrdinal,
                                   m_d3dCaps.DeviceType, m_d3dsdBackBuffer.Format,
                                   0, D3DRTYPE_TEXTURE,
                                   D3DFMT_V8U8 ) ) &&
                        (m_d3dCaps.TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAP );

    BOOL bCanDoL6V5U5 = SUCCEEDED( m_pD3D->CheckDeviceFormat( m_d3dCaps.AdapterOrdinal,
                                   m_d3dCaps.DeviceType, m_d3dsdBackBuffer.Format,
                                   0, D3DRTYPE_TEXTURE,
                                   D3DFMT_L6V5U5 ) ) &&
                        (m_d3dCaps.TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAPLUMINANCE );

    BOOL bCanDoL8V8U8 = SUCCEEDED( m_pD3D->CheckDeviceFormat( m_d3dCaps.AdapterOrdinal,
                                   m_d3dCaps.DeviceType, m_d3dsdBackBuffer.Format,
                                   0, D3DRTYPE_TEXTURE,
                                   D3DFMT_X8L8V8U8 ) ) &&
                        (m_d3dCaps.TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAPLUMINANCE );

    if( bCanDoV8U8 )        m_BumpMapFormat = D3DFMT_V8U8;
    else if( bCanDoL6V5U5 ) m_BumpMapFormat = D3DFMT_L6V5U5;
    else if( bCanDoL8V8U8 ) m_BumpMapFormat = D3DFMT_X8L8V8U8;
    else                    return E_FAIL;

    // Set menu states
    HMENU hMenu = GetMenu( m_hWnd );
    EnableMenuItem( hMenu, IDM_U8V8,   bCanDoV8U8   ? MF_ENABLED : MF_GRAYED );
    EnableMenuItem( hMenu, IDM_U5V5L6, bCanDoL6V5U5 ? MF_ENABLED : MF_GRAYED );
    EnableMenuItem( hMenu, IDM_U8V8L8, bCanDoL8V8U8 ? MF_ENABLED : MF_GRAYED );
    SetMenuStates();

    // Initialize earth geometry
    if( FAILED( CreateEarthVertexBuffer() ) )
        return E_FAIL;

    // Create and fill the bumpmap
    if( FAILED( InitBumpMap() ) )
        return E_FAIL;

    m_bDeviceValidationFailed = FALSE;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    m_pFont->RestoreDeviceObjects();

    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 2, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );
    m_pd3dDevice->SetTextureStageState( 2, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );
    m_pd3dDevice->SetTextureStageState( 2, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 2, D3DTSS_MINFILTER, D3DTEXF_LINEAR );

    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

    // Get the aspect ratio
    FLOAT fAspect = ((FLOAT)m_d3dsdBackBuffer.Width) / m_d3dsdBackBuffer.Height;

    // Set projection matrix
    D3DXMATRIX matProj;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, 0.1f, 25.0f );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Set the ArcBall parameters
    m_ArcBall.SetWindow( m_d3dsdBackBuffer.Width, m_d3dsdBackBuffer.Height, 3.0f );
    m_ArcBall.SetRadius( 1.0f );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc:
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    m_pFont->InvalidateDeviceObjects();
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
    SAFE_RELEASE( m_pBlockTexture );
    SAFE_RELEASE( m_pEarthTexture );
    SAFE_RELEASE( m_pEarthBumpTexture );
    SAFE_RELEASE( m_pEnvMapTexture );

    m_pFont->DeleteDeviceObjects();

    SAFE_RELEASE( m_psBumpMap );
    SAFE_RELEASE( m_pEarthVB );
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FinalCleanup()
// Desc: Called before the app exits, this function gives the app the chance
//       to cleanup after itself.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FinalCleanup()
{
    SAFE_DELETE( m_pFont );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device intialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::ConfirmDevice( D3DCAPS8* pCaps, DWORD dwBehavior,
                                          D3DFORMAT Format )
{
    if( dwBehavior & D3DCREATE_PUREDEVICE )
        return E_FAIL; // GetTransform doesn't work on PUREDEVICE

    // Device must be able to do bumpmapping
    if( pCaps->TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAPLUMINANCE )
    {
        // Accept devices that can create D3DFMT_X8L8V8U8 textures
        if( SUCCEEDED( m_pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal,
                                                  pCaps->DeviceType, Format,
                                                  0, D3DRTYPE_TEXTURE,
                                                  D3DFMT_X8L8V8U8 ) ) )
        {
            return S_OK;
        }

        // Accept devices that can create D3DFMT_L6V5U5 textures
        if( SUCCEEDED( m_pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal,
                                                  pCaps->DeviceType, Format,
                                                  0, D3DRTYPE_TEXTURE,
                                                  D3DFMT_L6V5U5 ) ) )
        {
            return S_OK;
        }
    }

    if( pCaps->TextureOpCaps & D3DTEXOPCAPS_BUMPENVMAP )
    {
        // Accept devices that can create D3DFMT_V8U8 textures
        if( SUCCEEDED( m_pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal,
                                                  pCaps->DeviceType, Format,
                                                  0, D3DRTYPE_TEXTURE,
                                                  D3DFMT_V8U8 ) ) )
        {
            return S_OK;
        }
    }

    // Else, reject the device
    return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: CreateEarthVertexBuffer()
// Desc: Sets up the vertices for a bump-mapped sphere.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::CreateEarthVertexBuffer()
{
    SAFE_RELEASE( m_pEarthVB );

    // Choose a tesselation level
    DWORD dwNumSphereRings    = m_bHighTesselation ? 15 :  5;
    DWORD dwNumSphereSegments = m_bHighTesselation ? 30 : 10;
    m_dwNumSphereVertices = 2 * dwNumSphereRings * (dwNumSphereSegments+1);

    // Create the vertex buffer
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( m_dwNumSphereVertices*sizeof(BUMPVERTEX),
                                                  D3DUSAGE_WRITEONLY, D3DFVF_BUMPVERTEX,
                                                  D3DPOOL_MANAGED, &m_pEarthVB ) ) )
        return E_FAIL;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetMenuStates()
// Desc:
//-----------------------------------------------------------------------------
VOID CMyD3DApplication::SetMenuStates()
{
    HMENU hMenu = GetMenu( m_hWnd );

    CheckMenuItem( hMenu, IDM_TEXTURETOGGLE,
                    m_bTextureOn ? MF_CHECKED : MF_UNCHECKED );
    CheckMenuItem( hMenu, IDM_BUMPMAPTOGGLE,
                    m_bBumpMapOn ? MF_CHECKED : MF_UNCHECKED );
    CheckMenuItem( hMenu, IDM_ENVMAPTOGGLE,
                    m_bEnvMapOn ? MF_CHECKED : MF_UNCHECKED );

    CheckMenuItem( hMenu, IDM_U8V8L8,
                    m_BumpMapFormat==D3DFMT_X8L8V8U8 ? MF_CHECKED : MF_UNCHECKED );
    CheckMenuItem( hMenu, IDM_U5V5L6,
                    m_BumpMapFormat==D3DFMT_L6V5U5 ? MF_CHECKED : MF_UNCHECKED );
    CheckMenuItem( hMenu, IDM_U8V8,
                    m_BumpMapFormat==D3DFMT_V8U8 ? MF_CHECKED : MF_UNCHECKED );

    CheckMenuItem( hMenu, IDM_LOW_TESSELATION,
                    (!m_bHighTesselation) ? MF_CHECKED : MF_UNCHECKED );
    CheckMenuItem( hMenu, IDM_HIGH_TESSELATION,
                    m_bHighTesselation ? MF_CHECKED : MF_UNCHECKED );
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Message proc function to handle key and menu input
//-----------------------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam )
{
    // Pass mouse messages to the ArcBall so it can build internal matrices
    m_ArcBall.HandleMouseMessages( hWnd, uMsg, wParam, lParam );

    // Trap context menu
    if( WM_CONTEXTMENU == uMsg )
        return 0;

    // Handle menu commands
    if( WM_COMMAND == uMsg )
    {
        switch( LOWORD(wParam) )
        {
            case IDM_TEXTURETOGGLE:
                m_bTextureOn = !m_bTextureOn;
                break;

            case IDM_BUMPMAPTOGGLE:
                m_bBumpMapOn = !m_bBumpMapOn;
                break;

            case IDM_ENVMAPTOGGLE:
                m_bEnvMapOn  = !m_bEnvMapOn;
                break;

            case IDM_U8V8L8:
                SAFE_RELEASE( m_psBumpMap );
                m_BumpMapFormat = D3DFMT_X8L8V8U8;
                InitBumpMap();
                break;

            case IDM_U5V5L6:
                SAFE_RELEASE( m_psBumpMap );
                m_BumpMapFormat = D3DFMT_L6V5U5;
                InitBumpMap();
                break;

            case IDM_U8V8:
                SAFE_RELEASE( m_psBumpMap );
                m_BumpMapFormat = D3DFMT_V8U8;
                InitBumpMap();
                break;

            case IDM_LOW_TESSELATION:
                m_bHighTesselation = FALSE;
                CreateEarthVertexBuffer();
                break;

            case IDM_HIGH_TESSELATION:
                m_bHighTesselation = TRUE;
                CreateEarthVertexBuffer();
                break;
        }

        // Update the menus, in case any state changes occurred
        SetMenuStates();
    }

    // Pass remaining messages to default handler
    return CD3DApplication::MsgProc( hWnd, uMsg, wParam, lParam );
}



