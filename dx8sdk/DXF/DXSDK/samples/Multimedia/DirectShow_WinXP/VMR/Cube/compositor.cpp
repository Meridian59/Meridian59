//------------------------------------------------------------------------------
// File: compositor.cpp
//
// Desc: DirectShow sample code - VMR-based Cube video player
//
// Copyright (c) 1994 - 2001, Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <streams.h>
#include <dvdmedia.h>
#include <mmreg.h>
#include <commctrl.h>

#include "project.h"
#include <stdarg.h>
#include <stdio.h>


/*****************************Private*Routine******************************\
* MakeD3DVector
*
\**************************************************************************/
D3DVECTOR
MakeD3DVector( float x, float y, float z )
{
    D3DVECTOR ret;
    ret.x = x;
    ret.y = y;
    ret.z = z;
    return ret;
}

/*****************************Private*Routine******************************\
* MakeD3DVertex
*
\**************************************************************************/
D3DVERTEX
MakeD3DVertex( const D3DVECTOR& p, const D3DVECTOR& n, float tu, float tv )
{
    D3DVERTEX ret;
    ret.x = p.x;  ret.y = p.y;  ret.z = p.z;
    ret.nx = n.x; ret.ny = n.y; ret.nz = n.z;
    ret.tu = tu; ret.tv = tv;
    return ret;

}

/*****************************Private*Routine******************************\
* Name: MatrixMultiply()
* Desc: Multiplies matrix B onto matrix A, as in: [Q] = [B] * [A].
\**************************************************************************/
VOID MatrixMultiply( D3DMATRIX& q, D3DMATRIX& a, D3DMATRIX& b )
{
    FLOAT* pA = (FLOAT*)&a;
    FLOAT* pB = (FLOAT*)&b;
    FLOAT  pM[16];

    ZeroMemory( pM, sizeof(D3DMATRIX) );

    for( DWORD i=0; i<4; i++ ) 
        for( DWORD j=0; j<4; j++ ) 
            for( DWORD k=0; k<4; k++ ) 
                pM[4*i+j] += pA[4*k+j] * pB[4*i+k];

    memcpy( &q, pM, sizeof(D3DMATRIX) );
}

/*****************************Private*Routine******************************\
* CreateCube()
*
* Sets up the vertices for a cube. Don't forget to set the texture
* coordinates for each vertex.
*
\**************************************************************************/
VOID
CreateCube(
    D3DVERTEX* pVertices,
    StreamSize * pSize
    )
{
    // Define the normals for the cube
    D3DVECTOR n0 = MakeD3DVector( 0.0f, 0.0f, 1.0f ); // Front face
    D3DVECTOR n4 = MakeD3DVector( 1.0f, 0.0f, 0.0f ); // Right face
    D3DVECTOR n1 = MakeD3DVector( 0.0f, 0.0f, 1.0f ); // Back face
    D3DVECTOR n5 = MakeD3DVector(-1.0f, 0.0f, 0.0f ); // Left face
    D3DVECTOR n2 = MakeD3DVector( 0.0f, 1.0f, 0.0f ); // Top face
    D3DVECTOR n3 = MakeD3DVector( 0.0f,-1.0f, 0.0f ); // Bottom face

    // Front face
    *pVertices++ = MakeD3DVertex( MakeD3DVector(-1.0f, 1.0f,-1.0f), n0, 0.0f, 0.0f );
    *pVertices++ = MakeD3DVertex( MakeD3DVector( 1.0f, 1.0f,-1.0f), n0, pSize[0].cx  , 0.0f );
    *pVertices++ = MakeD3DVertex( MakeD3DVector(-1.0f,-1.0f,-1.0f), n0, 0.0f, pSize[0].cy   );
    *pVertices++ = MakeD3DVertex( MakeD3DVector( 1.0f,-1.0f,-1.0f), n0, pSize[0].cx  , pSize[0].cy   );

     // Right face
    *pVertices++ = MakeD3DVertex( MakeD3DVector( 1.0f, 1.0f,-1.0f), n4, 0.0f, 0.0f );
    *pVertices++ = MakeD3DVertex( MakeD3DVector( 1.0f, 1.0f, 1.0f), n4, pSize[1].cx  , 0.0f );
    *pVertices++ = MakeD3DVertex( MakeD3DVector( 1.0f,-1.0f,-1.0f), n4, 0.0f, pSize[1].cy   );
    *pVertices++ = MakeD3DVertex( MakeD3DVector( 1.0f,-1.0f, 1.0f), n4, pSize[1].cx  , pSize[1].cy   );

    // Top face
    *pVertices++ = MakeD3DVertex( MakeD3DVector(-1.0f, 1.0f, 1.0f), n2, 0.0f, 0.0f );
    *pVertices++ = MakeD3DVertex( MakeD3DVector( 1.0f, 1.0f, 1.0f), n2, pSize[2].cx  , 0.0f );
    *pVertices++ = MakeD3DVertex( MakeD3DVector(-1.0f, 1.0f,-1.0f), n2, 0.0f, pSize[2].cy   );
    *pVertices++ = MakeD3DVertex( MakeD3DVector( 1.0f, 1.0f,-1.0f), n2, pSize[2].cx  , pSize[2].cy   );

    // Back face
    *pVertices++ = MakeD3DVertex( MakeD3DVector(-1.0f, 1.0f, 1.0f), n1, pSize[0].cx  , 0.0f );
    *pVertices++ = MakeD3DVertex( MakeD3DVector(-1.0f,-1.0f, 1.0f), n1, pSize[0].cx  , pSize[0].cy   );
    *pVertices++ = MakeD3DVertex( MakeD3DVector( 1.0f, 1.0f, 1.0f), n1, 0.0f, 0.0f );
    *pVertices++ = MakeD3DVertex( MakeD3DVector( 1.0f,-1.0f, 1.0f), n1, 0.0f, pSize[0].cy   );

    // Left face
    *pVertices++ = MakeD3DVertex( MakeD3DVector(-1.0f, 1.0f,-1.0f), n5, pSize[1].cx  , 0.0f );
    *pVertices++ = MakeD3DVertex( MakeD3DVector(-1.0f,-1.0f,-1.0f), n5, pSize[1].cx  , pSize[1].cy   );
    *pVertices++ = MakeD3DVertex( MakeD3DVector(-1.0f, 1.0f, 1.0f), n5, 0.0f, 0.0f );
    *pVertices++ = MakeD3DVertex( MakeD3DVector(-1.0f,-1.0f, 1.0f), n5, 0.0f, pSize[1].cy   );

    // Bottom face
    *pVertices++ = MakeD3DVertex( MakeD3DVector(-1.0f,-1.0f, 1.0f), n3, 0.0f, 0.0f );
    *pVertices++ = MakeD3DVertex( MakeD3DVector(-1.0f,-1.0f,-1.0f), n3, 0.0f, pSize[2].cy   );
    *pVertices++ = MakeD3DVertex( MakeD3DVector( 1.0f,-1.0f, 1.0f), n3, pSize[2].cx  , 0.0f );
    *pVertices++ = MakeD3DVertex( MakeD3DVector( 1.0f,-1.0f,-1.0f), n3, pSize[2].cx  , pSize[2].cy   );
}

/*****************************Private*Routine******************************\
* FrameMove
*
* Called once per frame, the call is used for animating the scene. The
* device is used for changing various render states, and the timekey is
* used for timing of the dynamics of the scene.
*
\**************************************************************************/
HRESULT
CMpegMovie::FrameMove(
    LPDIRECT3DDEVICE7 pd3dDevice,
    FLOAT fTimeKey
    )
{
    HRESULT hr = S_OK;

    __try {

        // Set the view matrix so that the camera is backed out along the z-axis,
        // and looks down on the cube (rotating along the x-axis by -0.5 radians).
        D3DMATRIX matView;
        ZeroMemory(&matView, sizeof(D3DMATRIX));

        matView._11 = 1.0f;
        matView._22 =  (FLOAT)cos(-0.5f);
        matView._23 =  (FLOAT)sin(-0.5f);
        matView._32 = -(FLOAT)sin(-0.5f);
        matView._33 =  (FLOAT)cos(-0.5f);
        matView._43 = 5.0f;
        matView._44 = 1.0f;
        CHECK_HR(hr = pd3dDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, &matView));

        // Set the world matrix to rotate along the y-axis, in sync with the timekey
        D3DMATRIX matRotate, matRotate2, matWorld;
        ZeroMemory(&matRotate, sizeof(D3DMATRIX));
        ZeroMemory(&matRotate2, sizeof(D3DMATRIX));
        ZeroMemory(&matWorld, sizeof(D3DMATRIX));

        // rotate along y axis
        matRotate._11 =  (FLOAT)cos(fTimeKey);
        matRotate._13 =  (FLOAT)sin(fTimeKey);
        matRotate._22 =  1.0f;
        matRotate._31 = -(FLOAT)sin(fTimeKey);
        matRotate._33 =  (FLOAT)cos(fTimeKey);
        matRotate._44 =  1.0f;

        // rotate along z axis
        matRotate2._11 = (FLOAT)cos(fTimeKey);
        matRotate2._12 = (FLOAT)sin(fTimeKey);
        matRotate2._21 = -(FLOAT)sin(fTimeKey);
        matRotate2._22 = (FLOAT)cos(fTimeKey);
        matRotate2._33 =  1.0f;
        matRotate2._44 =  1.0f;

        MatrixMultiply(matWorld, matRotate, matRotate2);
        CHECK_HR(hr = pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &matWorld));
    }
    __finally {}

    return hr;
}

/******************************Public*Routine******************************\
* GetSourceRectFromMediaType
*
\**************************************************************************/
RECT
GetSourceRectFromMediaType(
    const AM_MEDIA_TYPE *pMediaType
    )
{
    RECT rRect = {0,0,0,0};
    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        return rRect;
    }

    if (!(pMediaType->pbFormat))
    {
        return rRect;
    }

    if ((pMediaType->formattype == FORMAT_VideoInfo) &&
        (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER)))
    {
        rRect = (((VIDEOINFOHEADER*)(pMediaType->pbFormat))->rcSource);
    }
    else if ((pMediaType->formattype == FORMAT_VideoInfo2) &&
             (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER2)))
    {
        rRect = (((VIDEOINFOHEADER2*)(pMediaType->pbFormat))->rcSource);
    }

    // DShow filters have the habit of sometimes not setting the src and dst
    // rectangles, in this case we should imply that the these rectangles
    // should be the same width and height as the bitmap in the media format.
    if (IsRectEmpty(&rRect)) 
    {
        LPBITMAPINFOHEADER lpbi = GetbmiHeader(pMediaType);

        if (lpbi) 
        {
            SetRect(&rRect, 0, 0, abs(lpbi->biWidth), abs(lpbi->biHeight));
        }
    }

    return rRect;
}

/*****************************Private*Routine******************************\
* GetbmiHeader
*
* Returns the bitmap info header associated with the specified CMediaType.
* Returns NULL if the CMediaType is not either of FORMAT_VideoInfo or
* FORMAT_VideoInfo2.
*
\**************************************************************************/
LPBITMAPINFOHEADER
GetbmiHeader(
    const AM_MEDIA_TYPE *pMediaType
    )
{
    AMTRACE((TEXT("GetbmiHeader")));

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        return NULL;
    }

    if (!(pMediaType->pbFormat))
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType->pbFormat is NULL")));
        return NULL;
    }

    LPBITMAPINFOHEADER lpHeader = NULL;
    if ((pMediaType->formattype == FORMAT_VideoInfo) &&
        (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER)))
    {
        lpHeader = &(((VIDEOINFOHEADER*)(pMediaType->pbFormat))->bmiHeader);
    }
    else if ((pMediaType->formattype == FORMAT_VideoInfo2) &&
             (pMediaType->cbFormat >= sizeof(VIDEOINFOHEADER2)))
    {
        lpHeader = &(((VIDEOINFOHEADER2*)(pMediaType->pbFormat))->bmiHeader);
    }

    return lpHeader;
}

/*****************************Private*Routine******************************\
* AllocateTextureMirror
*
\**************************************************************************/

// global data structures and static helper function for AllocateTextureMirror
const DDPIXELFORMAT g_rgTextMirFormats[] = {
    { sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 32, 0xff0000, 0xff00,  0xff, 0xff<<24ul},
    { sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 24, 0xff0000, 0xff00,  0xff, 0       },
    { sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16, 0x1f<<11, 0x3f<<5, 0x1f, 0       },
    { sizeof(DDPIXELFORMAT), DDPF_RGB, 0, 16, 0x1f<<10, 0x1f<<5, 0x1f, 0       }
};

const UINT g_cTextMirFormats = sizeof(g_rgTextMirFormats)/sizeof(DDPIXELFORMAT);

static UINT
NextPow2( UINT i )
{
    UINT ret = 1;
    while ( ret < i )
    {
        ret <<= 1;
    }
    return ret;
}


/*****************************Private*Routine******************************\
* AllocateTextureMirror
*
\**************************************************************************/
HRESULT
CMpegMovie::AllocateTextureMirror(
    LPDIRECTDRAWSURFACE7 pddsVideo,
    DWORD* lpdwWidth,
    DWORD* lpdwHeight, 
    bool bNonPow2Cond
    )
{
    HRESULT hr = E_UNEXPECTED;
    DDSURFACEDESC2 ddsd = {sizeof(ddsd)};

    __try {

        LPDIRECTDRAW7 pDD;

        CHECK_HR(pddsVideo->GetDDInterface((LPVOID*)&pDD));

        if (m_pDDSTextureMirror) {

            CHECK_HR(hr = m_pDDSTextureMirror->GetSurfaceDesc(&ddsd));

            // early out if mirror already exists and is large enough to accommodate pDDS
            if (ddsd.dwWidth >= *lpdwWidth && ddsd.dwHeight >= *lpdwHeight) {
                *lpdwWidth = ddsd.dwWidth;
                *lpdwHeight = ddsd.dwHeight;
                return S_OK;
            }

            *lpdwWidth = max(ddsd.dwWidth, *lpdwWidth);
            *lpdwHeight = max(ddsd.dwHeight, *lpdwHeight);
        }

        RELEASE(m_pDDSTextureMirror);

        // bump dimensions out to next power of 2
        // if the 3D hardware needs it that way

        if (!bNonPow2Cond) {
            *lpdwWidth = NextPow2(*lpdwWidth);
            *lpdwHeight = NextPow2(*lpdwHeight);
        }

        DDSURFACEDESC2 ddsd = {sizeof(ddsd)};
        ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
        ddsd.dwWidth = *lpdwWidth;
        ddsd.dwHeight = *lpdwHeight;
        ddsd.ddsCaps.dwCaps |= DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;

        // loop over texture formats and return as soon as CreateSurface succeeds
        for (UINT i = 0; i < g_cTextMirFormats; i++) {

            // create texture mirror
            ddsd.ddpfPixelFormat = g_rgTextMirFormats[i];
            hr = pDD->CreateSurface(&ddsd, &m_pDDSTextureMirror, NULL);
            if (SUCCEEDED(hr))
                break;
        }
        RELEASE(pDD);
    }
    __finally {}

    return hr;
}


/*****************************Private*Routine******************************\
* InitCompositionTarget
*
\**************************************************************************/
STDMETHODIMP
CMpegMovie::InitCompositionTarget(
    IUnknown* lpUnk,
    LPDIRECTDRAWSURFACE7 pddsRenderTarget
    )
{
    HRESULT hr = S_OK;

    __try {

        LPDIRECT3DDEVICE7 pd3dDevice = (LPDIRECT3DDEVICE7)lpUnk;
        DDSURFACEDESC2 ddsd = {sizeof(DDSURFACEDESC2)};

        CHECK_HR(hr = pddsRenderTarget->GetSurfaceDesc(&ddsd));

        D3DVIEWPORT7 vp = { 0, 0, ddsd.dwWidth, ddsd.dwHeight, 0.0f, 1.0f };
        CHECK_HR(hr = pd3dDevice->SetViewport(&vp));

        //
        // Remind the CompositeImage (see below) to init the cube vertices.
        // We can't do it here because we don't know the size of the movie
        // being played yet.
        //
        m_bInitCube = true;

        // For simplicity, just use ambient lighting and a white material
        D3DMATERIAL7 mtrl;
        ZeroMemory(&mtrl, sizeof(mtrl));
        mtrl.diffuse.r = mtrl.diffuse.g = mtrl.diffuse.b = 1.0f;
        mtrl.ambient.r = mtrl.ambient.g = mtrl.ambient.b = 1.0f;
        CHECK_HR(hr = pd3dDevice->SetMaterial(&mtrl));
        CHECK_HR(hr = pd3dDevice->SetRenderState(D3DRENDERSTATE_AMBIENT, 0xffffffff));

        // Set the projection matrix. Note that the view and world matrices are
        // set in the App_FrameMove() function, so they can be animated each
        // frame.
        D3DMATRIX matProj;
        ZeroMemory(&matProj, sizeof(D3DMATRIX));
        matProj._11 =  2.0f;
        matProj._22 =  2.0f;
        matProj._33 =  1.0f;
        matProj._34 =  1.0f;
        matProj._43 = -1.0f;
        CHECK_HR(hr = pd3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj));
    }
    __finally {
    };

    return hr;
}

/*****************************Private*Routine******************************\
* CompositeImage
*
\**************************************************************************/
STDMETHODIMP
CMpegMovie::CompositeImage(
    IUnknown* lpUnk,
    LPDIRECTDRAWSURFACE7 pddsRenderTarget,
    AM_MEDIA_TYPE* pmtRenderTarget,
    REFERENCE_TIME rtStart,
    REFERENCE_TIME rtEnd,
    DWORD dwMapped,
    VMRVIDEOSTREAMINFO* pStrmInfo,
    UINT cStreams
    )
{
    LPDIRECT3DDEVICE7 pd3dDevice = (LPDIRECT3DDEVICE7)lpUnk;
    HRESULT hr = S_OK;
    bool bInScene = false;

    __try 
    {
        LPDIRECTDRAWSURFACE7 pDDS;

        //
        // First time thru init the Cube compositor
        //
        if (m_bInitCube) 
        { 
            // if the dimensions are not 0, one of the surfaces is not a texture and we need to create a texture mirror
            if (m_dwTexMirrorWidth && m_dwTexMirrorHeight) 
            {
                hr = AllocateTextureMirror(pStrmInfo->pddsVideoSurface, &m_dwTexMirrorWidth, &m_dwTexMirrorHeight,
                    DoesSupportNonPow2CondCap(pd3dDevice));
            }

            m_bInitCube = false;
        }

        // Rotate the Cube
        CHECK_HR(hr = FrameMove(pd3dDevice, ConvertToMilliseconds(rtStart) * 0.0005f));

        // Clear the viewport to dark gray
        CHECK_HR(hr = pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0x00404040, 1.0f, 0L));

        cStreams = min(MAXSTREAMS, cStreams);

        StreamSize Sizes[MAXSTREAMS];
        for (DWORD j = 0; j < MAXSTREAMS; j++)
        {
            int iStream = j % cStreams;

            if (m_StreamInfo[pStrmInfo[iStream].dwStrmID].bTexture) 
            {
                Sizes[j].cx = (FLOAT)WIDTH(&m_StreamInfo[pStrmInfo[iStream].dwStrmID].SourceRect)  / (FLOAT)pStrmInfo[iStream].dwWidth;
                Sizes[j].cy = (FLOAT)HEIGHT(&m_StreamInfo[pStrmInfo[iStream].dwStrmID].SourceRect) / (FLOAT)pStrmInfo[iStream].dwHeight;
            }
            else 
            {
                Sizes[j].cx = (FLOAT)WIDTH(&m_StreamInfo[pStrmInfo[iStream].dwStrmID].SourceRect)  / (FLOAT)m_dwTexMirrorWidth;
                Sizes[j].cy = (FLOAT)HEIGHT(&m_StreamInfo[pStrmInfo[iStream].dwStrmID].SourceRect) / (FLOAT)m_dwTexMirrorHeight;
            }
        }

        CreateCube(m_pCubeVertices, Sizes);

        // Begin the scene
        CHECK_HR(hr = pd3dDevice->BeginScene());
        bInScene = true;

        for (unsigned int i = 0; i < MAXSTREAMS; i++)
        {
            int iStream = i % cStreams;
            //
            // Make sure we are dealing with a texture surface - if not
            // copy the stream to the mirror surface.
            //
            if (m_StreamInfo[pStrmInfo[iStream].dwStrmID].bTexture) 
            {
                pDDS = pStrmInfo[iStream].pddsVideoSurface;
            }
            else 
            {
                RECT r = { 0, 0, pStrmInfo->dwWidth, pStrmInfo->dwHeight };

                pDDS = m_pDDSTextureMirror;
                CHECK_HR(hr = pDDS->Blt(&r, pStrmInfo[iStream].pddsVideoSurface, &r, DDBLT_WAIT, NULL));
            }

            // Prepare the texture stages
            CHECK_HR(hr = pd3dDevice->SetTexture(0, pDDS));
            if (DoesSupportAnisoCap(pd3dDevice))
            {
                CHECK_HR(hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_ANISOTROPIC));
                CHECK_HR(hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_ANISOTROPIC));
                CHECK_HR(hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_MAXANISOTROPY, 4));
            }
            else if (DoesSupportLinearCap(pd3dDevice))
            {
                CHECK_HR(hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_LINEAR));
                CHECK_HR(hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_LINEAR));
            }
            else
            {
                CHECK_HR(hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_POINT));
                CHECK_HR(hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_POINT));
            }
            CHECK_HR(hr = pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP)); 

            // Draw the faces of the cube
            CHECK_HR(hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX,
                                       m_pCubeVertices+(4*i), 4, NULL));
            CHECK_HR(hr = pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX,
                                       m_pCubeVertices+(4*i)+12, 4, NULL));
        } // for i
    }
    __finally 
    {
        if (bInScene) {
            // End the scene.
            pd3dDevice->EndScene();
        }
    }
    return S_OK;
}

/*****************************Private*Routine******************************\
* SetStreamMediaType
*
\**************************************************************************/
HRESULT CMpegMovie::SetStreamMediaType(DWORD dwStrmID, AM_MEDIA_TYPE *pmt, BOOL fTexture)
{
    if (pmt)
    {
        m_StreamInfo[dwStrmID].bTexture = fTexture;
        m_StreamInfo[dwStrmID].SourceRect = GetSourceRectFromMediaType(pmt);

        if (!fTexture) 
        {
            // choose the largest measure in each dimension
            m_dwTexMirrorWidth = max(m_dwTexMirrorWidth, (DWORD)WIDTH(&m_StreamInfo[dwStrmID].SourceRect));
            m_dwTexMirrorHeight = max(m_dwTexMirrorHeight, (DWORD)HEIGHT(&m_StreamInfo[dwStrmID].SourceRect));    
        }
    }
    return S_OK;
}

/*****************************Private*Routine******************************\
* DoesSupportNonPow2CondCap
*
\**************************************************************************/
bool CMpegMovie::DoesSupportNonPow2CondCap(LPDIRECT3DDEVICE7 pD3DDevice)
{
    ASSERT(pD3DDevice);
    D3DDEVICEDESC7 ddDesc;
    if (!(DD_OK == pD3DDevice->GetCaps(&ddDesc)))
    {
        return false;
    }

    return (ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL) ? true : false;
}

/*****************************Private*Routine******************************\
* DoesSupportAnisoCap
*
\**************************************************************************/
bool CMpegMovie::DoesSupportAnisoCap(LPDIRECT3DDEVICE7 pD3DDevice)
{
    ASSERT(pD3DDevice);
    D3DDEVICEDESC7 ddDesc;
    if (!(DD_OK == pD3DDevice->GetCaps(&ddDesc)))
    {
        return false;
    }

    bool bMag = (ddDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFANISOTROPIC) ? true : false;
    bool bMin = (ddDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFANISOTROPIC) ? true : false;
    return (bMag && bMin);
}

/*****************************Private*Routine******************************\
* DoesSupportLinearCap
*
\**************************************************************************/
bool CMpegMovie::DoesSupportLinearCap(LPDIRECT3DDEVICE7 pD3DDevice)
{
    ASSERT(pD3DDevice);
    D3DDEVICEDESC7 ddDesc;
    if (!(DD_OK == pD3DDevice->GetCaps(&ddDesc)))
    {
        return false;
    }

    bool bMag = (ddDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) ? true : false;
    bool bMin = (ddDesc.dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) ? true : false;
    return (bMag && bMin);
}