//-----------------------------------------------------------------------------
// File: DShowTextures.cpp
//
// Desc: DirectShow sample code - adds support for DirectShow videos playing 
//       on a DirectX 8.0 texture surface. Turns the D3D texture tutorial into 
//       a recreation of the VideoTex sample from previous versions of DirectX.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "Textures.h"
#include "DShowTextures.h"
#include "DXUtil.h"

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------
#define SOURCE_FILE     TEXT("skiing.avi")

// An application can advertise the existence of its filter graph
// by registering the graph with a global Running Object Table (ROT).
// The GraphEdit application can detect and remotely view the running
// filter graph, allowing you to 'spy' on the graph with GraphEdit.
//
// To enable registration in this sample, define REGISTER_FILTERGRAPH.
//
#define REGISTER_FILTERGRAPH

//-----------------------------------------------------------------------------
// Global DirectShow pointers
//-----------------------------------------------------------------------------
CComPtr<IGraphBuilder>  g_pGB;          // GraphBuilder
CComPtr<IMediaControl>  g_pMC;          // Media Control
CComPtr<IMediaPosition> g_pMP;          // Media Postion
CComPtr<IMediaEvent>    g_pME;          // Media Event

D3DFORMAT               g_TextureFormat; // Texture format

//-----------------------------------------------------------------------------
// InitDShowTextureRenderer : Create DirectShow filter graph and run the graph
//-----------------------------------------------------------------------------
HRESULT InitDShowTextureRenderer(LPDIRECT3DTEXTURE8 pTexture)
{
    HRESULT hr = S_OK;
    CComPtr<IBaseFilter>    pFTR;           // Texture Renderer Filter
    CComPtr<IPin>           pFTRPinIn;      // Texture Renderer Input Pin
    CComPtr<IBaseFilter>    pFSrc;          // Source Filter
    CComPtr<IPin>           pFSrcPinOut;    // Source Filter Output Pin   
    CTextureRenderer        *pCTR;          // DShow Texture renderer
    
    // Create the filter graph
    if (FAILED(g_pGB.CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC)))
        return E_FAIL;

#ifdef REGISTER_FILTERGRAPH
    // Register the graph in the Running Object Table (for debug purposes)
    AddToROT(g_pGB);
#endif
    
    // Create the Texture Renderer object
    pCTR = new CTextureRenderer(NULL, &hr);
    if (FAILED(hr))                                      
    {
        Msg(TEXT("Could not create texture renderer object!  hr=0x%x"), hr);
        return E_FAIL;
    }
    
    // Get a pointer to the IBaseFilter on the TextureRenderer, add it to graph
    pFTR = pCTR;
    if (FAILED(hr = g_pGB->AddFilter(pFTR, L"TEXTURERENDERER")))
    {
        Msg(TEXT("Could not add renderer filter to graph!  hr=0x%x"), hr);
        return hr;
    }
    
    // Determine the file to load based on DirectX Media path (from SDK)
    // Use a helper function included in DXUtils.cpp
    TCHAR strFileName[MAX_PATH];
    WCHAR wFileName[MAX_PATH];
    lstrcpy( strFileName, DXUtil_GetDXSDKMediaPath() );
    lstrcat( strFileName, SOURCE_FILE );

    #ifndef UNICODE
        MultiByteToWideChar(CP_ACP, 0, strFileName, -1, wFileName, MAX_PATH);
    #else
        lstrcpy(wFileName, strFileName);
    #endif

    // Add the source filter
    if (FAILED(hr = g_pGB->AddSourceFilter (wFileName, L"SOURCE", &pFSrc)))
    {
        Msg(TEXT("Could not create source filter to graph!  hr=0x%x"), hr);
        return hr;
    }

    // Find the source's output pin and the renderer's input pin
    if (FAILED(hr = pFTR->FindPin(L"In", &pFTRPinIn)))
    {
        Msg(TEXT("Could not find input pin!  hr=0x%x"), hr);
        return hr;
    }

    if (FAILED(hr = pFSrc->FindPin(L"Output", &pFSrcPinOut)))
    {
        Msg(TEXT("Could not find output pin!  hr=0x%x"), hr);
        return hr;
    }
    
    // Connect these two filters
    if (FAILED(hr = g_pGB->Connect(pFSrcPinOut, pFTRPinIn)))
    {
        Msg(TEXT("Could not connect pins!  hr=0x%x"), hr);
        return hr;
    }
   
    // Get the graph's media control, event & position interfaces
    g_pGB.QueryInterface(&g_pMC);
    g_pGB.QueryInterface(&g_pMP);
    g_pGB.QueryInterface(&g_pME);
    
    // Start the graph running;
    if (FAILED(hr = g_pMC->Run()))
    {
        Msg(TEXT("Could not run the DirectShow graph!  hr=0x%x"), hr);
        return hr;
    }
    return S_OK;
}


//-----------------------------------------------------------------------------
// CheckMovieStatus: If the movie has ended, rewind to beginning
//-----------------------------------------------------------------------------
void CheckMovieStatus(void)
{
    long lEventCode;
    long lParam1;
    long lParam2;
    HRESULT hr;

    // Check for completion events
    hr = g_pME->GetEvent(&lEventCode, (LONG_PTR *) &lParam1, (LONG_PTR *) &lParam2, 0);
    if (SUCCEEDED(hr))
    {
        if (EC_COMPLETE == lEventCode) 
        {
            hr = g_pMP->put_CurrentPosition(0);
        }

        // Free any memory associated with this event
        hr = g_pME->FreeEventParams(lEventCode, lParam1, lParam2);
    }
}


//-----------------------------------------------------------------------------
// CleanupDShow
//-----------------------------------------------------------------------------
void CleanupDShow(void)
{
#ifdef REGISTER_FILTERGRAPH
    // Pull graph from Running Object Table (Debug)
    RemoveFromROT();
#endif

    // Shut down the graph
    if (!(!g_pMC)) g_pMC->Stop();
    if (!(!g_pMC)) g_pMC.Release();
    if (!(!g_pME)) g_pME.Release();
    if (!(!g_pMP)) g_pMP.Release();
    if (!(!g_pGB)) g_pGB.Release();
}
    

//-----------------------------------------------------------------------------
// CTextureRenderer constructor
//-----------------------------------------------------------------------------
CTextureRenderer::CTextureRenderer( LPUNKNOWN pUnk, HRESULT *phr )
                                   : CBaseVideoRenderer(__uuidof(CLSID_TextureRenderer), 
                                   NAME("Texture Renderer"), pUnk, phr)
{
    // Store and AddRef the texture for our use.
    *phr = S_OK;
}


//-----------------------------------------------------------------------------
// CTextureRenderer destructor
//-----------------------------------------------------------------------------
CTextureRenderer::~CTextureRenderer()
{
    // Do nothing
}


//-----------------------------------------------------------------------------
// CheckMediaType: This method forces the graph to give us an R8G8B8 video
// type, making our copy to texture memory trivial.
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::CheckMediaType(const CMediaType *pmt)
{
    HRESULT   hr = E_FAIL;
    VIDEOINFO *pvi;
    
    // Reject the connection if this is not a video type
    if( *pmt->FormatType() != FORMAT_VideoInfo ) {
        return E_INVALIDARG;
    }
    
    // Only accept RGB24
    pvi = (VIDEOINFO *)pmt->Format();
    if(IsEqualGUID( *pmt->Type(),    MEDIATYPE_Video)  &&
       IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_RGB24))
    {
        hr = S_OK;
    }
    
    return hr;
}


//-----------------------------------------------------------------------------
// SetMediaType: Graph connection has been made. 
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::SetMediaType(const CMediaType *pmt)
{
    HRESULT hr;

    // Retrive the size of this media type
    VIDEOINFO *pviBmp;                      // Bitmap info header
    pviBmp = (VIDEOINFO *)pmt->Format();
    m_lVidWidth  = pviBmp->bmiHeader.biWidth;
    m_lVidHeight = abs(pviBmp->bmiHeader.biHeight);
    m_lVidPitch = (m_lVidWidth * 3 + 3) & ~(3); // We are forcing RGB24

    // Create the texture that maps to this media type
    if( FAILED( hr = D3DXCreateTexture(g_pd3dDevice,
                    m_lVidWidth, m_lVidHeight,
                    1, 0, 
                    D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &g_pTexture ) ) )
    {
        Msg(TEXT("Could not create the D3DX texture!  hr=0x%x"), hr);
        return hr;
    }

    // D3DXCreateTexture can silently change the parameters on us
    D3DSURFACE_DESC ddsd;
    if ( FAILED( hr = g_pTexture->GetLevelDesc( 0, &ddsd ) ) ) {
        Msg(TEXT("Could not get level Description of D3DX texture! hr = 0x%x"), hr);
        return hr;
    }
    g_TextureFormat = ddsd.Format;
    if (g_TextureFormat != D3DFMT_A8R8G8B8 &&
        g_TextureFormat != D3DFMT_A1R5G5B5) {
        Msg(TEXT("Texture is format we can't handle! Format = 0x%x"), g_TextureFormat);
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// DoRenderSample: A sample has been delivered. Copy it to the texture.
//-----------------------------------------------------------------------------
HRESULT CTextureRenderer::DoRenderSample( IMediaSample * pSample )
{
    BYTE  *pBmpBuffer, *pTxtBuffer;     // Bitmap buffer, texture buffer
    LONG  lTxtPitch;                // Pitch of bitmap, texture
    
    // Get the video bitmap buffer
    pSample->GetPointer( &pBmpBuffer );

    // Lock the Texture
    D3DLOCKED_RECT d3dlr;
    if (FAILED(g_pTexture->LockRect(0, &d3dlr, 0, 0)))
        return E_FAIL;
    
    // Get the texture buffer & pitch
    pTxtBuffer = static_cast<byte *>(d3dlr.pBits);
    lTxtPitch = d3dlr.Pitch;
    
    // Copy the bits    
    // OPTIMIZATION OPPORTUNITY: Use a video and texture
    // format that allows a simpler copy than this one.
    if (g_TextureFormat == D3DFMT_A8R8G8B8) {
        for(int y = 0; y < m_lVidHeight; y++ ) {
            BYTE *pBmpBufferOld = pBmpBuffer;
            BYTE *pTxtBufferOld = pTxtBuffer;   
            for (int x = 0; x < m_lVidWidth; x++) {
                pTxtBuffer[0] = pBmpBuffer[0];
                pTxtBuffer[1] = pBmpBuffer[1];
                pTxtBuffer[2] = pBmpBuffer[2];
                pTxtBuffer[3] = 0xff;
                pTxtBuffer += 4;
                pBmpBuffer += 3;
            }
            pBmpBuffer = pBmpBufferOld + m_lVidPitch;
            pTxtBuffer = pTxtBufferOld + lTxtPitch;
        }
    }

    if (g_TextureFormat == D3DFMT_A1R5G5B5) {
        for(int y = 0; y < m_lVidHeight; y++ ) {
            BYTE *pBmpBufferOld = pBmpBuffer;
            BYTE *pTxtBufferOld = pTxtBuffer;   
            for (int x = 0; x < m_lVidWidth; x++) {
                *(WORD *)pTxtBuffer = (WORD)
                    (0x8000 +
                    ((pBmpBuffer[2] & 0xF8) << 7) +
                    ((pBmpBuffer[1] & 0xF8) << 2) +
                    (pBmpBuffer[0] >> 3));
				pTxtBuffer += 2;
                pBmpBuffer += 3;
            }
            pBmpBuffer = pBmpBufferOld + m_lVidPitch;
            pTxtBuffer = pTxtBufferOld + lTxtPitch;
        }
    }

        
    // Unlock the Texture
    if (FAILED(g_pTexture->UnlockRect(0)))
        return E_FAIL;
    
    return S_OK;
}


#ifdef REGISTER_FILTERGRAPH

//-----------------------------------------------------------------------------
// Running Object Table functions: Used to debug. By registering the graph
// in the running object table, GraphEdit is able to connect to the running
// graph. This code should be removed before the application is shipped in
// order to avoid third parties from spying on your graph.
//-----------------------------------------------------------------------------
DWORD dwROTReg = 0xfedcba98;

HRESULT AddToROT(IUnknown *pUnkGraph) 
{
    IMoniker * pmk;
    IRunningObjectTable *pirot;
    if (FAILED(GetRunningObjectTable(0, &pirot))) {
        return E_FAIL;
    }
    WCHAR wsz[256];
    wsprintfW(wsz, L"FilterGraph %08x  pid %08x", (DWORD_PTR) 0, GetCurrentProcessId());
    HRESULT hr = CreateItemMoniker(L"!", wsz, &pmk);
    if (SUCCEEDED(hr)) {
        hr = pirot->Register(0, pUnkGraph, pmk, &dwROTReg);
        pmk->Release();
    }
    pirot->Release();
    return hr;
}

void RemoveFromROT(void)
{
    IRunningObjectTable *pirot;
    if (SUCCEEDED(GetRunningObjectTable(0, &pirot))) {
        pirot->Revoke(dwROTReg);
        pirot->Release();
    }
}

#endif


//-----------------------------------------------------------------------------
// Msg: Display an error message box if needed
//-----------------------------------------------------------------------------
void Msg(TCHAR *szFormat, ...)
{
    TCHAR szBuffer[512];

    va_list pArgs;
    va_start(pArgs, szFormat);
    _vstprintf(szBuffer, szFormat, pArgs);
    va_end(pArgs);

    MessageBox(NULL, szBuffer, TEXT("DirectShow Texture3D Sample"), 
               MB_OK | MB_ICONERROR);
}
