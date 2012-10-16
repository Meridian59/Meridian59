//----------------------------------------------------------------------------
//  File:   BltAlpha.h
//
//  Desc:   DirectShow sample code
//          Header file and class description for CAlphaBlt,
//          texture surface to be used in PresentImage of the
//          customized allocator-presenter
//
//  Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------

#ifndef __INITDDSTRUCT_DEFINED
#define __INITDDSTRUCT_DEFINED
#include <math.h>
#include <d3dxmath.h>
#include "d3dtextr.h"
#include "d3dfont.h"
#include "utils.h"
#include "resrc1.h"
#include "d3dutil.h"

extern struct SceneSettings g_ss;

template <typename T>
__inline void INITDDSTRUCT(T& dd)
{
    ZeroMemory(&dd, sizeof(dd));
    dd.dwSize = sizeof(dd);
}
#endif

#ifndef __RELEASE_DEFINED
#define __RELEASE_DEFINED
template<typename T>
__inline void RELEASE( T* &p )
{
    if( p ) {
        p->Release();
        p = NULL;
    }
}
#endif

#ifndef CHECK_HR
    #define CHECK_HR(expr) do { if (FAILED(expr)) __leave; } while(0);
#endif

#define COMPLVERTSIZE 640 // must be divisible by 4!!!
#define pi 3.1415926535f

//----------------------------------------------------------------------------
// CAlphaBlt
// 
// Desc: texture surface to be used in PresentImage of the
//       customized allocator-presenter
//----------------------------------------------------------------------------
class CAlphaBlt
{
private:

    LPDIRECTDRAW7               m_pDD;
    LPDIRECT3D7                 m_pD3D;
    LPDIRECT3DDEVICE7           m_pD3DDevice;
    LPDIRECTDRAWSURFACE7        m_lpDDBackBuffer;

    LPDIRECTDRAWSURFACE7        m_lpDDMirror;
    LPDIRECTDRAWSURFACE7        m_lpDDM32;
    LPDIRECTDRAWSURFACE7        m_lpDDM16;
    DDSURFACEDESC2              m_ddsdM32;
    DDSURFACEDESC2              m_ddsdM16;

    bool                        m_fPowerOf2;
    bool                        m_fSquare;
    bool                        m_bNeedToRestoreMenu;
    TextureContainer *          m_ptxtrMenu;
    CD3DFont *                  m_pFont;

    typedef struct Vertex
    {
        float x, y, z, rhw;
        D3DCOLOR clr;
        float tu, tv;
    } Vertex;

    Vertex pVertices[4];            // primitive for rotating effect
    Vertex pVComplex[COMPLVERTSIZE];// primitive for twisting effect
    Vertex pVMenu[4];               // primitive for menu effect


    //----------------------------------------------------------------------------
    // CAlphaBlt::Rotate
    // 
    // Desc: 3D transformation of pVertices that provide rotation around Z and Y 
    // 
    // Parameters:
    //      theta       - angle of rotation around Z axis
    //      thetaY      - angle of rotation around Y axis
    //      pVertices   - array of (Vertex*) to be transformed
    //      nSize       - number of vertices
    //      (cx2, cy2)  - center of rotation
    //----------------------------------------------------------------------------
    void Rotate(float theta, float thetaY, 
                CAlphaBlt::Vertex * pVertices, 
                int nSize, float cx2, float cy2)
    {
        D3DXMATRIX  mtrV;
        D3DXMATRIX  mtrRotZ;
        D3DXMATRIX  mtrRotY;
        D3DXMATRIX  mtrPrs;
        D3DXMATRIX  mtrRes;

        float pi2 = 1.57079632675f; // pi/2.

        // initialize mtrV
        mtrV.m[0][0] = pVertices[0].x - cx2;    
        mtrV.m[1][0] = pVertices[0].y - cy2;    
        mtrV.m[2][0] = pVertices[0].z;          
        mtrV.m[3][0] = 0.f;                     

        mtrV.m[0][1] = pVertices[1].x - cx2;    
        mtrV.m[1][1] = pVertices[1].y - cy2;    
        mtrV.m[2][1] = pVertices[1].z;  
        mtrV.m[3][1] = 0;

        mtrV.m[0][2] = pVertices[2].x - cx2;    
        mtrV.m[1][2] = pVertices[2].y - cy2;    
        mtrV.m[2][2] = pVertices[2].z;  
        mtrV.m[3][2] = 0;               

        mtrV.m[0][3] = pVertices[3].x - cx2;
        mtrV.m[1][3] = pVertices[3].y - cy2;
        mtrV.m[2][3] = pVertices[3].z;
        mtrV.m[3][3] = 0;
        
        D3DXMatrixRotationZ( &mtrRotZ,  theta); 
        D3DXMatrixRotationY( &mtrRotY,  thetaY); 
        D3DXMatrixPerspectiveFov( &mtrPrs, pi2, 1.1f, 0.f, 1.f);


        // mtrRotZ * mtrV
        D3DXMatrixMultiply( &mtrRes, &mtrRotZ, &mtrV);

        // mtrRotY * mtrRotZ * mtrV
        D3DXMatrixMultiply( &mtrV, &mtrRotY, &mtrRes);

        // mtrPrs * mtrRotY * mtrRotZ * mtrV
        D3DXMatrixMultiply( &mtrRes, &mtrPrs, &mtrV);
        
        // here, mtrRes has what we need; copy it back to pVertices
        pVertices[0].x = mtrRes.m[0][0] + cx2;  
        pVertices[1].x = mtrRes.m[0][1] + cx2;  
        pVertices[2].x = mtrRes.m[0][2] + cx2;  
        pVertices[3].x = mtrRes.m[0][3] + cx2;

        pVertices[0].y = mtrRes.m[1][0] + cy2;  
        pVertices[1].y = mtrRes.m[1][1] + cy2;  
        pVertices[2].y = mtrRes.m[1][2] + cy2;  
        pVertices[3].y = mtrRes.m[1][3] + cy2;
        
        pVertices[0].z = mtrRes.m[2][0];    
        pVertices[1].z = mtrRes.m[2][1];    
        pVertices[2].z = mtrRes.m[2][2];    
        pVertices[3].z = mtrRes.m[2][3];
        return;
    }   
    
    //----------------------------------------------------------------------------
    // IsSurfaceBlendable
    //
    // Checks the DD surface description and the given
    // alpha value to determine if this surface is blendable.
    //
    //----------------------------------------------------------------------------
    bool
    IsSurfaceBlendable(
        DDSURFACEDESC2& ddsd,
        BYTE fAlpha
        )
    {
        // Is the surface already a D3D texture ?
        if (ddsd.ddsCaps.dwCaps & DDSCAPS_TEXTURE) {
            return true;
        }

        // OK we have to mirror the surface
        return false;
    }

    //----------------------------------------------------------------------------
    // MirrorSourceSurface
    //
    // The mirror surface cab be either 16 or 32 bit RGB depending
    // upon the format of the source surface.
    //
    // Of course it should have the "texture" flag set and should be in VRAM.  
    // If we can't create the surface then the AlphaBlt should fail
    //----------------------------------------------------------------------------
    HRESULT MirrorSourceSurface(
        LPDIRECTDRAWSURFACE7 lpDDS,
        DDSURFACEDESC2& ddsd
        )
    {
        HRESULT hr = DD_OK;
        DWORD dwMirrorBitDepth = 0;
        DDSURFACEDESC2 ddsdMirror={0};


        //
        // OK - is it suitable for our needs.
        //
        // Apply the following rules:
        //  if ddsd is a FOURCC surface the mirror should be 32 bit
        //  if ddsd is RGB then the mirror's bit depth should match
        //      that of ddsd.
        //
        // Also, the mirror must be large enough to actually hold
        // the surface to be blended
        //

        m_lpDDMirror = NULL;

        if (ddsd.ddpfPixelFormat.dwFlags == DDPF_FOURCC ||
            ddsd.ddpfPixelFormat.dwRGBBitCount == 32) {

            if (ddsd.dwWidth > m_ddsdM32.dwWidth ||
                ddsd.dwHeight > m_ddsdM32.dwHeight) {

                RELEASE(m_lpDDM32);
            }

            if (!m_lpDDM32) {
                dwMirrorBitDepth = 32;
            }
            else {
                m_lpDDMirror = m_lpDDM32;
                ddsdMirror = m_ddsdM32;
            }
        }
        else if (ddsd.ddpfPixelFormat.dwRGBBitCount == 16) {

            if (ddsd.dwWidth > m_ddsdM16.dwWidth ||
                ddsd.dwHeight > m_ddsdM16.dwHeight) {

                RELEASE(m_lpDDM16);
            }

            if (!m_lpDDM16) {
                dwMirrorBitDepth = 16;
            }
            else {
                m_lpDDMirror = m_lpDDM16;
                ddsdMirror = m_ddsdM16;
            }
        }
        else {

            // No support for RGB24 or RGB8!
            return E_INVALIDARG;
        }

        if (!m_lpDDMirror) {

            INITDDSTRUCT(ddsdMirror);
            ddsdMirror.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
            ddsdMirror.ddpfPixelFormat.dwFlags = DDPF_RGB;
            ddsdMirror.ddpfPixelFormat.dwRGBBitCount = dwMirrorBitDepth;

            switch (dwMirrorBitDepth) {
            case 16:
                ddsdMirror.ddpfPixelFormat.dwRBitMask = 0x0000F800;
                ddsdMirror.ddpfPixelFormat.dwGBitMask = 0x000007E0;
                ddsdMirror.ddpfPixelFormat.dwBBitMask = 0x0000001F;
                break;

            case 32:
                ddsdMirror.ddpfPixelFormat.dwRBitMask = 0x00FF0000;
                ddsdMirror.ddpfPixelFormat.dwGBitMask = 0x0000FF00;
                ddsdMirror.ddpfPixelFormat.dwBBitMask = 0x000000FF;
                break;
            }

            ddsdMirror.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;
            ddsdMirror.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;

            if (m_fPowerOf2) {

                for (ddsdMirror.dwWidth = 1;
                     ddsd.dwWidth > ddsdMirror.dwWidth;
                     ddsdMirror.dwWidth <<= 1);

                for (ddsdMirror.dwHeight = 1;
                     ddsd.dwHeight > ddsdMirror.dwHeight;
                     ddsdMirror.dwHeight <<= 1);
            }
            else {
                ddsdMirror.dwWidth = ddsd.dwWidth;
                ddsdMirror.dwHeight = ddsd.dwHeight;
            }

            if (m_fSquare) {

                if (ddsdMirror.dwHeight > ddsdMirror.dwWidth) {
                    ddsdMirror.dwWidth = ddsdMirror.dwHeight;
                }

                if (ddsdMirror.dwWidth > ddsdMirror.dwHeight) {
                    ddsdMirror.dwHeight = ddsdMirror.dwWidth;
                }
            }

            __try {

                // Attempt to create the surface with theses settings
                CHECK_HR(hr = m_pDD->CreateSurface(&ddsdMirror, &m_lpDDMirror, NULL));

                INITDDSTRUCT(ddsdMirror);
                CHECK_HR(hr =  m_lpDDMirror->GetSurfaceDesc(&ddsdMirror));

                switch (dwMirrorBitDepth) {
                case 16:
                    m_ddsdM16 = ddsdMirror;
                    m_lpDDM16 = m_lpDDMirror;
                    break;

                case 32:
                    m_ddsdM32 = ddsdMirror;
                    m_lpDDM32 = m_lpDDMirror;
                    break;
                }

            } __finally {}
        }

        if (hr == DD_OK) {

            __try {
                RECT rc = {0, 0, ddsd.dwWidth, ddsd.dwHeight};
                CHECK_HR(hr = m_lpDDMirror->Blt(&rc, lpDDS, &rc, DDBLT_WAIT, NULL));
                ddsd = ddsdMirror;
            } __finally {}
        }

        return hr;
    }

public:

    //----------------------------------------------------------------------------
    // Constructor
    //----------------------------------------------------------------------------
    CAlphaBlt(LPDIRECTDRAWSURFACE7 lpDDSDst, HRESULT* phr) :
        m_pDD(NULL),
        m_pD3D(NULL),
        m_pD3DDevice(NULL),
        m_lpDDBackBuffer(NULL),
        m_lpDDMirror(NULL),
        m_lpDDM32(NULL),
        m_lpDDM16(NULL),
        m_ptxtrMenu( NULL),
        m_pFont(NULL),
        m_fPowerOf2(false),
        m_fSquare(false),
        m_bNeedToRestoreMenu(false)
    {
        ZeroMemory(&m_ddsdM32, sizeof(m_ddsdM32));
        ZeroMemory(&m_ddsdM16, sizeof(m_ddsdM16));

        HRESULT hr;
        hr = lpDDSDst->GetDDInterface((LPVOID *)&m_pDD);
        if (FAILED(hr)) {
            m_pDD = NULL;
            *phr = hr;
        }

        if (SUCCEEDED(hr)) {
            hr = m_pDD->QueryInterface(IID_IDirect3D7, (LPVOID *)&m_pD3D);
            if (FAILED(hr)) {
                m_pD3D = NULL;
                *phr = hr;
            }
        }

        if (SUCCEEDED(hr)) {
            hr = m_pD3D->CreateDevice(IID_IDirect3DHALDevice,
                                      lpDDSDst,
                                      &m_pD3DDevice);
            if (FAILED(hr)) {
                m_pD3DDevice = NULL;
                *phr = hr;
            }
            else {
                m_lpDDBackBuffer = lpDDSDst;
                m_lpDDBackBuffer->AddRef();
            }
        }

        if (SUCCEEDED(hr)) {

            D3DDEVICEDESC7 ddDesc;
            if (DD_OK == m_pD3DDevice->GetCaps(&ddDesc)) {

                if (ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_POW2) {
                    m_fPowerOf2 = true;
                }

                if (ddDesc.dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_SQUAREONLY) {
                    m_fSquare = true;
                }
            }
            else {
                *phr = hr;
            }
        }

        // here, so far we have D3Device, let's  Restore texture surface
        // it actually loads the bitmap from the resource onto a texture surface
        m_ptxtrMenu = new TextureContainer(
            "IDB_MENU", 0, 0, IDB_MENU);

        if( m_ptxtrMenu) 
        {
            hr = m_ptxtrMenu->LoadImageData();
            if( m_ptxtrMenu->m_hbmBitmap )
            {
                BITMAP bm;
                GetObject( m_ptxtrMenu->m_hbmBitmap, sizeof(BITMAP), &bm );
                m_ptxtrMenu->m_dwWidth  = (DWORD)bm.bmWidth;
                m_ptxtrMenu->m_dwHeight = (DWORD)bm.bmHeight;
                m_ptxtrMenu->m_dwBPP    = (DWORD)bm.bmBitsPixel;
            }
            
            hr = m_ptxtrMenu->Restore(m_pD3DDevice);
            if( FAILED(hr))
            {
                delete m_ptxtrMenu;
                m_ptxtrMenu = NULL;
            }
        }

        // load the font 
        m_pFont = new CD3DFont( TEXT("Comic Sans MS"),18,0);
        if( m_pFont )
        {
            m_pFont->InitDeviceObjects( m_pDD, m_pD3DDevice);
        }

        // setup menu primitive
        pVMenu[0].x = 0.f;
        pVMenu[0].y = 0.f;
        pVMenu[0].z = 0.0f;
        pVMenu[0].rhw = 2.0f;
        pVMenu[0].clr = RGBA_MAKE(0xff, 0xff, 0xff, 0xff);

        pVMenu[1].x = 60.f;
        pVMenu[1].y = 0.f;
        pVMenu[1].z = 0.0f;
        pVMenu[1].rhw = 2.0f;
        pVMenu[1].clr = RGBA_MAKE(0xff, 0xff, 0xff, 0xff);

        pVMenu[2].x = 0.f;
        pVMenu[2].y = 480.f;
        pVMenu[2].z = 0.0f;
        pVMenu[2].rhw = 2.0f;
        pVMenu[2].clr = RGBA_MAKE(0xff, 0xff, 0xff, 0xff);

        pVMenu[3].x = 60.f;
        pVMenu[3].y = 480.f;
        pVMenu[3].z = 0.0f;
        pVMenu[3].rhw = 2.0f;
        pVMenu[3].clr = RGBA_MAKE(0xff, 0xff, 0xff, 0xff);

        //
        // Setup the SRC info
        //
        pVMenu[0].tu = 0.f;
        pVMenu[0].tv = 0.f;

        pVMenu[1].tu = 1.f;
        pVMenu[1].tv = 0.f;

        pVMenu[2].tu = 0.f;
        pVMenu[2].tv = 1.f;

        pVMenu[3].tu = 1.f;
        pVMenu[3].tv = 1.f;
   }

    //----------------------------------------------------------------------------
    // Destructor
    //----------------------------------------------------------------------------
   ~CAlphaBlt()
    {
        if( m_ptxtrMenu )
        {
            delete m_ptxtrMenu;
            m_ptxtrMenu = NULL;
        }

        if( m_pFont )
        {
            delete m_pFont;
            m_pFont = NULL;
        }

        RELEASE(m_lpDDBackBuffer);
        RELEASE(m_lpDDM32);
        RELEASE(m_lpDDM16);

        RELEASE(m_pD3DDevice);
        RELEASE(m_pD3D);
        RELEASE(m_pDD);
    }

    //----------------------------------------------------------------------------
    // GetMenuTexture
    //
    // returns pointer to texture's DD surface or NULL otherwise
    //----------------------------------------------------------------------------
    LPDIRECTDRAWSURFACE7 GetMenuTexture()
    {
        if( m_ptxtrMenu && m_ptxtrMenu->m_pddsSurface )
        {
            return (m_ptxtrMenu->m_pddsSurface);
        }
        return NULL;
    }

    //----------------------------------------------------------------------------
    // SetMenuRestoreFlag
    //
    // sets m_bNeedToRestoreMenu = true; (called from CMpegMovie::RestoreDDRawSurfaces() )
    //----------------------------------------------------------------------------
    void SetMenuRestoreFlag()
    {
        m_bNeedToRestoreMenu = true;
    }

    //----------------------------------------------------------------------------
    // GetFont
    //----------------------------------------------------------------------------
    CD3DFont * GetFont()
    {
        return m_pFont;
    }

    //----------------------------------------------------------------------------
    // PrepareVerticesRotate
    //
    // This is the 'main' transformation function for foration effect.
    // It generates rotation angles from 'time' variable that is just
    // a static counter of images from CMpegMovie:PresentImage function
    //
    // Parameters:
    //          lpDst   - destination rectangle
    //          lpSrc   - source rectangle
    //          alpha   - alpha level for the surface
    //          fWid    - width of the surface obtained by CMpegMovie:PresentImage
    //          fHgt    - height of the surface obtained by CMpegMovie:PresentImage
    //----------------------------------------------------------------------------
    void PrepareVerticesRotate( RECT* lpDst, RECT* lpSrc, BYTE alpha,
                                float fWid, float fHgt)
    {
        float RotRadZ = 0.f;
        float RotRadY = 0.f;
            
        //
        // Setup the DST info
        //
        pVertices[0].x = (float)lpDst->left;
        pVertices[0].y = (float)lpDst->top;
        pVertices[0].z = 0.5f;
        pVertices[0].rhw = 2.0f;
        pVertices[0].clr = RGBA_MAKE(0xff, 0xff, 0xff, alpha);

        pVertices[1].x = (float)lpDst->right;
        pVertices[1].y = (float)lpDst->top;
        pVertices[1].z = 0.5f;
        pVertices[1].rhw = 2.0f;
        pVertices[1].clr = RGBA_MAKE(0xff, 0xff, 0xff, alpha);

        pVertices[2].x = (float)lpDst->left;
        pVertices[2].y = (float)lpDst->bottom;
        pVertices[2].z = 0.5f;
        pVertices[2].rhw = 2.0f;
        pVertices[2].clr = RGBA_MAKE(0xff, 0xff, 0xff, alpha);

        pVertices[3].x = (float)lpDst->right;
        pVertices[3].y = (float)lpDst->bottom;
        pVertices[3].z = 0.5f;
        pVertices[3].rhw = 2.0f;
        pVertices[3].clr = RGBA_MAKE(0xff, 0xff, 0xff, alpha);

        //
        // Setup the SRC info
        //
        pVertices[0].tu = ((float)lpSrc->left) / fWid;
        pVertices[0].tv = ((float)lpSrc->top) / fHgt;

        pVertices[1].tu = ((float)lpSrc->right) / fWid;
        pVertices[1].tv = ((float)lpSrc->top) / fHgt;

        pVertices[2].tu = ((float)lpSrc->left) / fWid;
        pVertices[2].tv = ((float)lpSrc->bottom) / fHgt;

        pVertices[3].tu = ((float)lpSrc->right) / fWid;
        pVertices[3].tv = ((float)lpSrc->bottom) / fHgt;

        // g_ss.nGradZ is an angle in grades, so calculate radians
        RotRadZ = (float) (g_ss.nGradZ++) * pi / 180.f;
        if( !g_ss.bRotateZ)
        {
            g_ss.nGradZ--;
        }

        RotRadY = (float) (g_ss.nGradY++) * pi / 180.f;
        if( !g_ss.bRotateY)
        {
            g_ss.nGradY--;
        }

        // to avoid stack overflow, limit counters withing 360 grades
        if( g_ss.nGradZ > 360) 
        {
            g_ss.nGradZ = 0;
        }

        if( g_ss.nGradY > 360) 
        {
            g_ss.nGradY = 0;
        }
    
        // and finally rotate the primitive pVertices
        // NOTE that rotation center is hardcoded for the case 
        // 640x480 display mode
        Rotate(RotRadZ, RotRadY, pVertices, 4, 320.f, 240.f);
    }

    //----------------------------------------------------------------------------
    // PrepareVerticesTwist
    //
    // This is the 'main' transformation function for "twist" effect.
    // It generates transformation parameters 'time' variable that is just
    // a static counter of images from CMpegMovie:PresentImage function
    //
    // Parameters:
    //          lpDst   - destination rectangle
    //          lpSrc   - source rectangle
    //          alpha   - alpha level for the surface
    //          fWid    - width of the surface obtained by CMpegMovie:PresentImage
    //          fHgt    - height of the surface obtained by CMpegMovie:PresentImage
    //
    // Other:   put your own effects here
    //          The effect I use is as following:
    //          (a) move picture down to create 'slipping' effect
    //          (b) srink lower part of the picture (parameter c applied to each verticle
    //              so that it would look like a wine glass stem (cosine is what we need)
    //              Shrinking coefficient depends on Y-coordinate of a verticle
    //          (c) twist lower part of the picture around Y axis, angle depends on 
    //              Y-coordinate of a verticle
    //----------------------------------------------------------------------------
    void PrepareVerticesTwist(  RECT* lpDst, RECT* lpSrc, BYTE alpha,
                                float fW, float fH)
    {
        CAlphaBlt::Vertex * V = pVComplex;
        float W = (float)(lpDst->right - lpDst->left);
        float H = (float)(lpDst->bottom - lpDst->top);
        float sW = (float)(lpSrc->right - lpSrc->left)/fW;
        float sH = (float)(lpSrc->bottom - lpSrc->top);
        float Ht = 480 - H;
        float c;
        float radY = 0.f;
        float radX = pi/4.f;
        float x;
        double costh;
        double sinth;
        D3DXMATRIX  mtrPrs;
        D3DXMATRIX  mtrOrg;
        D3DXMATRIX  mtrRes;

        int N = COMPLVERTSIZE/2;

        g_ss.nDy = g_ss.nDy + 3;
        if( g_ss.nDy > 480 )
        {
            g_ss.nDy = 0;
        }

        for( int i=0; i<COMPLVERTSIZE; i=i+2)
        {
            V[i  ].x   = (float)(lpDst->left);
            V[i+1].x   = (float)(lpDst->right);
            V[i  ].y   = V[i+1].y = (float)(i * H/(N-1)) + g_ss.nDy;
            V[i  ].z   = V[i+1].z = 0.5f;
            V[i  ].rhw = V[i+1].rhw = 2.f;
            V[i  ].clr = V[i+1].clr = RGBA_MAKE(0xff, 0xff, 0xff, alpha);
            V[i  ].tu  = ((float)lpSrc->left) / fW;
            V[i+1].tu  = ((float)lpSrc->right) / fW;
            V[i  ].tv  = V[i+1].tv = (float)(lpSrc->top + sH * (float)i/(N-1.f)/2.f)/fH;
            
            
            if( V[i].y >= H )
            {
                
                c = (float)( W * (1. + cos(pi * (480. - V[i].y)/Ht)) / 4.);
                //c *= 0.25f;
                V[i  ].x += c;
                V[i+1].x -= c;
                
                radY = pi * ( V[i].y - H ) / 2.f / Ht;

                costh = cos(radY);
                sinth = sin(radY);

                x = V[i].x - 320.f;
                V[i].x = (float)(costh * x - sinth * V[i].z) + 320.f;
                V[i].z = (float)(sinth * x + costh * V[i].z);

                x = V[i+1].x - 320.f;
                V[i+1].x = (float)(costh * x - sinth * V[i+1].z) + 320.f;
                V[i+1].z = (float)(sinth * x + costh * V[i+1].z);
                                
            } // if

        }// for i

        
        // now let's implement projection
        D3DXMatrixPerspectiveFov( &mtrPrs, pi/2.f, 1.1f, 0.f, 1.f);
        for( i=0; i<COMPLVERTSIZE; i = i+4)
        {
            mtrOrg.m[0][0] = V[i].x;
            mtrOrg.m[1][0] = V[i].y;
            mtrOrg.m[2][0] = V[i].z;
            mtrOrg.m[3][0] = 0.f;

            mtrOrg.m[0][1] = V[i+1].x;
            mtrOrg.m[1][1] = V[i+1].y;
            mtrOrg.m[2][1] = V[i+1].z;
            mtrOrg.m[3][1] = 0.f;

            mtrOrg.m[0][2] = V[i+2].x;
            mtrOrg.m[1][2] = V[i+2].y;
            mtrOrg.m[2][2] = V[i+2].z;
            mtrOrg.m[3][2] = 0.f;

            mtrOrg.m[0][3] = V[i+3].x;
            mtrOrg.m[1][3] = V[i+3].y;
            mtrOrg.m[2][3] = V[i+3].z;
            mtrOrg.m[3][3] = 0.f;

            D3DXMatrixMultiply( &mtrRes, &mtrPrs, &mtrOrg);

            V[i  ].x = mtrRes.m[0][0];
            V[i  ].y = mtrRes.m[1][0];
            V[i  ].z = mtrRes.m[2][0];

            V[i+1].x = mtrRes.m[0][1];
            V[i+1].y = mtrRes.m[1][1];
            V[i+1].z = mtrRes.m[2][1];

            V[i+2].x = mtrRes.m[0][2];
            V[i+2].y = mtrRes.m[1][2];
            V[i+2].z = mtrRes.m[2][2];

            V[i+3].x = mtrRes.m[0][3];
            V[i+3].y = mtrRes.m[1][3];
            V[i+3].z = mtrRes.m[2][3];
        }// for
        
        return;
    }

    //----------------------------------------------------------------------------
    // AlphaBlt
    // 
    // This function receives LPDIRECTDRAWSURFACE7 from ImagePresenter, calls
    // transformation functions to provide visual effects, sets the scene and
    // renders primitives
    //
    // Parameters: 
    //      lpDst    -- destination rectangle
    //      lpDDSSrc -- surface obtained by ImagePresenter
    //      lpSrc    -- source rectangle
    //      bAlpha   -- alpha value for the surface
    //----------------------------------------------------------------------------
    HRESULT
    AlphaBlt(RECT* lpDst,
             LPDIRECTDRAWSURFACE7 lpDDSSrc,
             RECT* lpSrc,
             BYTE  bAlpha
             )
    {
        HRESULT hr=S_OK;
        DDSURFACEDESC2 ddsd;

        int nVertSize = 0;

        CAlphaBlt::Vertex * pV = NULL; 

        __try {

            INITDDSTRUCT(ddsd);
            CHECK_HR(hr = lpDDSSrc->GetSurfaceDesc(&ddsd));

            if (!IsSurfaceBlendable(ddsd, bAlpha)) {
                CHECK_HR(hr = MirrorSourceSurface(lpDDSSrc, ddsd));
                lpDDSSrc = m_lpDDMirror;
            }
            
            float fWid = (float)ddsd.dwWidth;
            float fHgt = (float)ddsd.dwHeight;

            if( g_ss.bShowTwist )
            {
                nVertSize = COMPLVERTSIZE;
                PrepareVerticesTwist(lpDst, lpSrc, bAlpha, fWid, fHgt);
                pV = pVComplex;
            }
            else
            {
                nVertSize = 4;
                PrepareVerticesRotate(lpDst, lpSrc, bAlpha, fWid, fHgt);
                pV = pVertices;
            }

            BYTE alpha = bAlpha;

            m_pD3DDevice->SetTexture(0, lpDDSSrc);
            // if menu is defined, set it

            m_pD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
            m_pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
            m_pD3DDevice->SetRenderState(D3DRENDERSTATE_BLENDENABLE, TRUE);
            m_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
            m_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
            m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

            m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

            m_pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_ANISOTROPIC);
            m_pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_ANISOTROPIC);
            m_pD3DDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_LINEAR);

            m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            m_pD3DDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
            m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
            m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    
            //
            // Do the alpha BLT
            //
            CHECK_HR(hr = m_pD3DDevice->BeginScene());
            CHECK_HR(hr = m_pD3DDevice->Clear(0,NULL,D3DCLEAR_TARGET,0,0.5,0));
            CHECK_HR(hr = m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,
                                                    D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1,
                                                    pV, nVertSize, D3DDP_WAIT));

            // now, draw menu over the video
            
            // if user switched modes, menu texture may be lost; then restore
            if( m_bNeedToRestoreMenu )
            {
                DDSURFACEDESC2 ddsd;
                ZeroMemory( &ddsd, sizeof(ddsd));
                ddsd.dwSize = sizeof(ddsd);
                ddsd.dwFlags = DDSD_ALL;
                m_pDD->GetDisplayMode( &ddsd);

                // if we try to restore surface in a different display mode, we will fail
                // so make the check first
                if( ddsd.dwWidth == SCRN_WIDTH && 
                    ddsd.dwHeight == SCRN_HEIGHT && 
                    ddsd.ddpfPixelFormat.dwRGBBitCount == SCRN_BITDEPTH )
                {
                    hr = GetMenuTexture()->Restore();
                    if( FAILED(hr))
                    {
                        OutputDebugString("ERROR: Failed to restore menu texture");
                    }
                    else
                    {
                        // surface restored, but its image lost; reload
                        if( m_ptxtrMenu) 
                        {
                            hr = m_ptxtrMenu->LoadImageData();
                            if( FAILED(hr))
                            {
                                OutputDebugString("Failed to load the image from resource\n");
                            }
                            if( m_ptxtrMenu->m_hbmBitmap )
                            {
                                BITMAP bm;
                                GetObject( m_ptxtrMenu->m_hbmBitmap, sizeof(BITMAP), &bm );
                                m_ptxtrMenu->m_dwWidth  = (DWORD)bm.bmWidth;
                                m_ptxtrMenu->m_dwHeight = (DWORD)bm.bmHeight;
                                m_ptxtrMenu->m_dwBPP    = (DWORD)bm.bmBitsPixel;
                            }
        
                            hr = m_ptxtrMenu->Restore(m_pD3DDevice);
                            if( FAILED(hr))
                            {
                                delete m_ptxtrMenu;
                                m_ptxtrMenu = NULL;
                            }
                        }

                        m_bNeedToRestoreMenu = false;
                    }
                }
                else
                {
                    OutputDebugString("Failed to restore menu texture");
                }
            }

            m_pD3DDevice->SetTexture(0, m_ptxtrMenu->m_pddsSurface);

            m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            m_pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_ANISOTROPIC);
            m_pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_ANISOTROPIC);
            m_pD3DDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_LINEAR);
            // 
            m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            m_pD3DDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
            m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
            m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

            CHECK_HR(hr = m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,
                                                    D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1,
                                                    pVMenu, 4, D3DDP_WAIT));
            m_pD3DDevice->SetTexture(0, NULL);

            // and if necessary, draw the dext over the menu
            if( g_ss.bShowHelp )
            {
                HRESULT hr = m_pFont->DrawText((float)g_ss.nXHelp, (float)g_ss.nYHelp,
                                                RGBA_MAKE(0xFF, 0xFF, 0x00, 0xFF),
                                                g_ss.achHelp );
            }
            if( g_ss.bShowStatistics )
            {
                m_pFont->DrawText(  420, 0, 
                                    RGBA_MAKE(0x00, 0xFF, 0x00, 0xFF),
                                    g_ss.achFPS );
            }

            CHECK_HR(hr = m_pD3DDevice->EndScene());

        } __finally {
            m_pD3DDevice->SetTexture(0, NULL);
        }

        return hr;
    }

    //----------------------------------------------------------------------------
    // TextureSquare
    // true if texture is square
    //----------------------------------------------------------------------------
    bool TextureSquare() {
        return  m_fSquare;
    }

    //----------------------------------------------------------------------------
    // TexturePower2
    // true if texture size is of a power of 2
    //----------------------------------------------------------------------------
    bool TexturePower2() {
        return  m_fPowerOf2;
    }

};


