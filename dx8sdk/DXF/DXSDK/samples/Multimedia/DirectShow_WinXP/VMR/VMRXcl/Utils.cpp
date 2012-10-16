//----------------------------------------------------------------------------
//  File:   Utils.h
//
//  Desc:   DirectShow sample code
//          External (global) utilities specific for VMRXcl app
//
//  Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//----------------------------------------------------------------------------
#include "project.h"
#include "utils.h"

//-----------------------------------------------------------------------------------------
//  Function:   hresultNameLookup
//  Purpose:    returns a string value for the given hresult
//  Arguments:  HRESULT that needs verifying
//  Returns:    string
//-----------------------------------------------------------------------------------------*/
const char * hresultNameLookup(HRESULT hres)
{
    switch(hres)
    {
    case VFW_E_CANNOT_RENDER:
        return "VFW_E_CANNOT_RENDER";
        break;
    case VFW_E_INVALID_FILE_FORMAT:
        return "VFW_E_INVALID_FILE_FORMAT";
        break;
    case VFW_E_NOT_FOUND:
        return "VFW_E_NOT_FOUND";
        break;
    case VFW_E_NOT_IN_GRAPH:
        return "VFW_E_NOT_IN_GRAPH";
        break;
    case VFW_E_UNKNOWN_FILE_TYPE:
        return "VFW_E_UNKNOWN_FILE_TYPE";
        break;
    case VFW_E_UNSUPPORTED_STREAM:
        return "VFW_E_UNSUPPORTED_STREAM";
        break;
    case VFW_E_CANNOT_CONNECT:
        return "VFW_E_CANNOT_CONNECT";
        break;
    case VFW_E_CANNOT_LOAD_SOURCE_FILTER:
        return "VFW_E_CANNOT_LOAD_SOURCE_FILTER";
        break;
    case VFW_S_PARTIAL_RENDER:
        return "VFW_S_PARTIAL_RENDER";
        break;
    case VFW_S_VIDEO_NOT_RENDERED:
        return "VFW_S_VIDEO_NOT_RENDERED";
        break;
    case VFW_S_AUDIO_NOT_RENDERED:
        return "VFW_S_AUDIO_NOT_RENDERED";
        break;
    case VFW_S_DUPLICATE_NAME:
        return "VFW_S_DUPLICATE_NAME";
        break;
    case VFW_S_MEDIA_TYPE_IGNORED:
        return "VFW_S_MEDIA_TYPE_IGNORED";
        break;
    case E_INVALIDARG:
        return "E_INVALIDARG";
        break;
    case DDERR_INCOMPATIBLEPRIMARY:
        return "DDERR_INCOMPATIBLEPRIMARY";
        break;
    case DDERR_INVALIDCAPS:
        return "DDERR_INVALIDCAPS";
        break;
    case DDERR_INVALIDOBJECT :
        return "DDERR_INVALIDOBJECT";
        break;
    case DDERR_INVALIDPIXELFORMAT:
        return "DDERR_INVALIDPIXELFORMAT";
        break;
    case DDERR_NOALPHAHW :
        return "DDERR_NOALPHAHW";
        break;
    case DDERR_NOCOOPERATIVELEVELSET :
        return "DDERR_NOCOOPERATIVELEVELSET";
        break;
    case DDERR_NODIRECTDRAWHW :
        return "DDERR_NODIRECTDRAWHW";
        break;
    case DDERR_NOEMULATION :
        return "DDERR_NOEMULATION";
        break;
    case VFW_E_BUFFERS_OUTSTANDING:
        return "VFW_E_BUFFERS_OUTSTANDING";
        break;
    case DDERR_NOEXCLUSIVEMODE :
        return "DDERR_NOEXCLUSIVEMODE ";
        break;
    case DDERR_NOFLIPHW:
        return "DDERR_NOFLIPHW";
        break;
    case DDERR_NOMIPMAPHW:
        return "DDERR_NOMIPMAPHW";
        break;
    case DDERR_NOOVERLAYHW :
        return "DDERR_NOOVERLAYHW ";
        break;
    case E_OUTOFMEMORY:
        return "E_OUTOFMEMORY";
        break;
    case VFW_E_NO_DISPLAY_PALETTE:
        return "VFW_E_NO_DISPLAY_PALETTE";
        break;
    case VFW_E_NO_COLOR_KEY_FOUND:
        return "VFW_E_NO_COLOR_KEY_FOUND";
        break;
    case VFW_E_PALETTE_SET:
        return "VFW_E_PALETTE_SET";
        break;
    case DDERR_NOZBUFFERHW :
        return "DDERR_NOZBUFFERHW ";
        break;
    case DDERR_OUTOFVIDEOMEMORY :
        return "DDERR_OUTOFVIDEOMEMORY";
        break;
    case DDERR_PRIMARYSURFACEALREADYEXISTS:
        return "DDERR_PRIMARYSURFACEALREADYEXISTS ";
        break;
    case DDERR_UNSUPPORTEDMODE:
        return "DDERR_UNSUPPORTEDMODE";
        break;
    case VFW_E_NO_ADVISE_SET:
        return "VFW_E_NO_ADVISE_SET";
        break;
    case S_OK:
        return "S_OK";
        break;
    case S_FALSE:
        return "S_FALSE";
        break;
    case VFW_S_CONNECTIONS_DEFERRED:
        return "VFW_S_CONNECTIONS_DEFERRED";
        break;
    case 0x80040154:
        return "Class not registered";
        break;
    case E_FAIL:
        return "E_FAIL";
        break;
    case VFW_E_DVD_OPERATION_INHIBITED:
        return "VFW_E_DVD_OPERATION_INHIBITED";
        break;
    case VFW_E_DVD_INVALIDDOMAIN:
        return "VFW_E_DVD_INVALIDDOMAIN";
        break;
    case E_NOTIMPL:
        return "E_NOTIMPL";
        break;
    case VFW_E_WRONG_STATE:
        return "VFW_E_WRONG_STATE";
        break;
    case E_PROP_SET_UNSUPPORTED:
        return "E_PROP_SET_UNSUPPORTED";
        break;
    case VFW_E_NO_PALETTE_AVAILABLE:
        return "VFW_E_NO_PALETTE_AVAILABLE";
        break;
    case E_UNEXPECTED:
        return "E_UNEXPECTED";
        break;
    case VFW_E_DVD_NO_BUTTON:
        return "VFW_E_DVD_NO_BUTTON";
        break;
    case VFW_E_DVD_GRAPHNOTREADY:
        return "VFW_E_DVD_GRAPHNOTREADY";
        break;
    case VFW_E_NOT_OVERLAY_CONNECTION:
        return "VFW_E_NOT_OVERLAY_CONNECTION";
        break;
    case VFW_E_DVD_RENDERFAIL:
        return "VFW_E_DVD_RENDERFAIL";
        break;
    case VFW_E_NOT_CONNECTED:
        return "VFW_E_NOT_CONNECTED";
        break;
    case E_NOINTERFACE:
        return "E_NOINTERFACE";
        break;
    case VFW_E_NO_COLOR_KEY_SET :
        return "VFW_E_NO_COLOR_KEY_SET ";
        break;
    case VFW_E_NO_INTERFACE:
        return "VFW_E_NO_INTERFACE";
        break;
    case 0x8004020c:
        return "VFW_E_BUFFER_NOTSET";
        break;
    case 0x80040225:
        return "VFW_E_NOT_PAUSED";
    case 0x80070002:
        return "System cannot find the file specified";
        break;
    case 0x80070003:
        return "System cannot find the path specified";
        break;
    case VFW_E_DVD_DECNOTENOUGH:
        return "VFW_E_DVD_DECNOTENOUGH";
        break;
    case VFW_E_ADVISE_ALREADY_SET:
        return "VFW_E_ADVISE_ALREADY_SET";
        break;
    case VFW_E_DVD_CMD_CANCELLED:
        return "VFW_E_DVD_CMD_CANCELLED";
        break;
    case VFW_E_DVD_MENU_DOES_NOT_EXIST:
        return "VFW_E_DVD_MENU_DOES_NOT_EXIST";
        break;
    case VFW_E_DVD_WRONG_SPEED:
        return "VFW_E_DVD_WRONG_SPEED";
        break;
    case VFW_S_DVD_NON_ONE_SEQUENTIAL:
        return "VFW_S_DVD_NON_ONE_SEQUENTIAL";
        break;
    case E_POINTER:
        return "E_POINTER";
        break;
    case VFW_E_DVD_NOT_IN_KARAOKE_MODE:
        return "VFW_E_DVD_NOT_IN_KARAOKE_MODE";
        break;
    case VFW_E_DVD_INVALID_DISC:
        return "VFW_E_DVD_INVALID_DISC";
        break;
    case VFW_E_DVD_STREAM_DISABLED:
        return "VFW_E_DVD_STREAM_DISABLED";
        break;
    case VFW_E_NOT_STOPPED:
        return "VFW_E_NOT_STOPPED";
        break;
    default:        
        return "Unrecognized";
        break;
    }
}

    
/*-----------------------------------------------------------------------------------------
|   Function:   MySleep
|   Purpose:    if the application is in automated mode, then sleep func is turned off
|   Arguments:  checks m_bAutomatedStatus to see if the func is in automation
|   Returns:    true if automated, false otherwist
\-----------------------------------------------------------------------------------------*/
     
bool MySleep(DWORD  dwTime)
{
    HANDLE hNeverHappensEvent;
    hNeverHappensEvent = CreateEvent(NULL, FALSE, FALSE, "EVENTTHATNEVERHAPPENS");
    WaitForSingleObject( hNeverHappensEvent, dwTime);
    return false;

} // end of checkHResult method


void ReportDDrawSurfDesc( DDSURFACEDESC2 ddsd)
{
    char szFlags[4096];
    char szMsg[4096];

    OutputDebugString("*** Surface description ***\n");
    
    SurfaceDescHelper( ddsd.dwFlags, szFlags);
    OutputDebugString(szFlags);

    sprintf( szMsg, "  dwWidth x dwHeight: %ld x %ld\n", ddsd.dwWidth, ddsd.dwHeight);
    OutputDebugString(szMsg);

    sprintf( szMsg, "  lPitch: %ld\n", ddsd.lPitch);
    OutputDebugString(szMsg);
    OutputDebugString("  (dwLinearSize)\n");

    sprintf( szMsg, "  dwBackBufferCount: %ld\n", ddsd.dwBackBufferCount);
    OutputDebugString(szMsg);

    sprintf( szMsg, "  dwMipMapCount: %ld\n", ddsd.dwMipMapCount);
    OutputDebugString(szMsg);
    OutputDebugString("  (dwRefreshRate)");

    sprintf( szMsg, "  dwAlphaBitDepth: %ld\n", (LONG)ddsd.dwAlphaBitDepth);
    OutputDebugString(szMsg);

    sprintf( szMsg, "  lpSurface: %x\n", (LONG_PTR)(ddsd.lpSurface));
    OutputDebugString(szMsg);

    ReportPixelFormat( ddsd.ddpfPixelFormat );
    ReportDDSCAPS2( ddsd.ddsCaps );

    sprintf( szMsg, "  dwTextureStage: %ld\n", ddsd.dwTextureStage);
    OutputDebugString(szMsg);

    OutputDebugString("***************************\n");
}

void ReportDDSCAPS2( DDSCAPS2 ddscaps )
{
    char sz[4096];

    strcpy( sz, "  DDSCAPS2::dwCaps: ");
    if( ddscaps.dwCaps & DDSCAPS_3DDEVICE ) strcat( sz, "DDSCAPS_3DDEVICE, ");
    if( ddscaps.dwCaps & DDSCAPS_ALLOCONLOAD ) strcat( sz, "DDSCAPS_ALLOCONLOAD, ");
    if( ddscaps.dwCaps & DDSCAPS_ALPHA ) strcat( sz, "DDSCAPS_ALPHA, ");
    if( ddscaps.dwCaps & DDSCAPS_BACKBUFFER ) strcat( sz, "DDSCAPS_BACKBUFFER, ");
    if( ddscaps.dwCaps & DDSCAPS_COMPLEX ) strcat( sz, "DDSCAPS_COMPLEX, ");
    if( ddscaps.dwCaps & DDSCAPS_FLIP ) strcat( sz, "DDSCAPS_FLIP, ");
    if( ddscaps.dwCaps & DDSCAPS_FRONTBUFFER ) strcat( sz, "DDSCAPS_FRONTBUFFER, ");
    if( ddscaps.dwCaps & DDSCAPS_HWCODEC ) strcat( sz, "DDSCAPS_HWCODEC, ");
    if( ddscaps.dwCaps & DDSCAPS_LIVEVIDEO ) strcat( sz, "DDSCAPS_LIVEVIDEO, ");
    if( ddscaps.dwCaps & DDSCAPS_LOCALVIDMEM ) strcat( sz, "DDSCAPS_LOCALVIDMEM, ");
    if( ddscaps.dwCaps & DDSCAPS_MIPMAP ) strcat( sz, "DDSCAPS_MIPMAP, ");
    if( ddscaps.dwCaps & DDSCAPS_MODEX ) strcat( sz, "DDSCAPS_MODEX, ");
    if( ddscaps.dwCaps & DDSCAPS_NONLOCALVIDMEM ) strcat( sz, "DDSCAPS_NONLOCALVIDMEM, ");
    if( ddscaps.dwCaps & DDSCAPS_OFFSCREENPLAIN ) strcat( sz, "DDSCAPS_OFFSCREENPLAIN, ");
    if( ddscaps.dwCaps & DDSCAPS_OPTIMIZED ) strcat( sz, "DDSCAPS_OPTIMIZED, ");
    if( ddscaps.dwCaps & DDSCAPS_OVERLAY ) strcat( sz, "DDSCAPS_OVERLAY, ");
    if( ddscaps.dwCaps & DDSCAPS_OWNDC ) strcat( sz, "DDSCAPS_OWNDC, ");
    if( ddscaps.dwCaps & DDSCAPS_PALETTE ) strcat( sz, "DDSCAPS_PALETTE, ");
    if( ddscaps.dwCaps & DDSCAPS_PRIMARYSURFACE ) strcat( sz, "DDSCAPS_PRIMARYSURFACE, ");
    if( ddscaps.dwCaps & DDSCAPS_STANDARDVGAMODE ) strcat( sz, "DDSCAPS_STANDARDVGAMODE, ");
    if( ddscaps.dwCaps & DDSCAPS_SYSTEMMEMORY ) strcat( sz, "DDSCAPS_SYSTEMMEMORY, ");
    if( ddscaps.dwCaps & DDSCAPS_TEXTURE ) strcat( sz, "DDSCAPS_TEXTURE, ");
    if( ddscaps.dwCaps & DDSCAPS_VIDEOMEMORY ) strcat( sz, "DDSCAPS_VIDEOMEMORY, ");
    if( ddscaps.dwCaps & DDSCAPS_VIDEOPORT ) strcat( sz, "DDSCAPS_VIDEOPORT, ");
    if( ddscaps.dwCaps & DDSCAPS_VISIBLE ) strcat( sz, "DDSCAPS_VISIBLE, ");
    if( ddscaps.dwCaps & DDSCAPS_WRITEONLY ) strcat( sz, "DDSCAPS_WRITEONLY, ");
    if( ddscaps.dwCaps & DDSCAPS_ZBUFFER ) strcat( sz, "DDSCAPS_ZBUFFER, ");

    strcat( sz, "\n");
    OutputDebugString(sz);
    strcpy( sz, "  DDSCAPS2::dwCaps2: ");

    if( ddscaps.dwCaps2 & DDSCAPS2_CUBEMAP ) strcat( sz, "DDSCAPS2_CUBEMAP, ");
    if( ddscaps.dwCaps2 & DDSCAPS2_CUBEMAP_POSITIVEX ) strcat( sz, "DDSCAPS2_CUBEMAP_POSITIVEX, ");
    if( ddscaps.dwCaps2 & DDSCAPS2_CUBEMAP_NEGATIVEX ) strcat( sz, "DDSCAPS2_CUBEMAP_NEGATIVEX, ");
    if( ddscaps.dwCaps2 & DDSCAPS2_CUBEMAP_POSITIVEY ) strcat( sz, "DDSCAPS2_CUBEMAP_POSITIVEY, ");
    if( ddscaps.dwCaps2 & DDSCAPS2_CUBEMAP_NEGATIVEY ) strcat( sz, "DDSCAPS2_CUBEMAP_NEGATIVEY, ");
    if( ddscaps.dwCaps2 & DDSCAPS2_CUBEMAP_POSITIVEZ ) strcat( sz, "DDSCAPS2_CUBEMAP_POSITIVEZ, ");
    if( ddscaps.dwCaps2 & DDSCAPS2_CUBEMAP_NEGATIVEZ ) strcat( sz, "DDSCAPS2_CUBEMAP_NEGATIVEZ, ");
    if( ddscaps.dwCaps2 & DDSCAPS2_CUBEMAP_ALLFACES ) strcat( sz, "DDSCAPS2_CUBEMAP_ALLFACES, ");
    if( ddscaps.dwCaps2 & DDSCAPS2_D3DTEXTUREMANAGE ) strcat( sz, "DDSCAPS2_D3DTEXTUREMANAGE, ");
    if( ddscaps.dwCaps2 & DDSCAPS2_DONOTPERSIST ) strcat( sz, "DDSCAPS2_DONOTPERSIST, ");
    if( ddscaps.dwCaps2 & DDSCAPS2_HARDWAREDEINTERLACE ) strcat( sz, "DDSCAPS2_HARDWAREDEINTERLACE, ");
    if( ddscaps.dwCaps2 & DDSCAPS2_HINTANTIALIASING ) strcat( sz, "DDSCAPS2_HINTANTIALIASING, ");
    if( ddscaps.dwCaps2 & DDSCAPS2_HINTDYNAMIC ) strcat( sz, "DDSCAPS2_HINTDYNAMIC, ");
    if( ddscaps.dwCaps2 & DDSCAPS2_HINTSTATIC ) strcat( sz, "DDSCAPS2_HINTSTATIC, ");
    if( ddscaps.dwCaps2 & DDSCAPS2_MIPMAPSUBLEVEL ) strcat( sz, "DDSCAPS2_MIPMAPSUBLEVEL, ");
    if( ddscaps.dwCaps2 & DDSCAPS2_OPAQUE ) strcat( sz, "DDSCAPS2_OPAQUE, ");
    if( ddscaps.dwCaps2 & DDSCAPS2_STEREOSURFACELEFT ) strcat( sz, "DDSCAPS2_STEREOSURFACELEFT, ");
    if( ddscaps.dwCaps2 & DDSCAPS2_TEXTUREMANAGE ) strcat( sz, "DDSCAPS2_TEXTUREMANAGE, ");
    
    strcat( sz, "\n");
    OutputDebugString(sz);
}


void ReportPixelFormat( DDPIXELFORMAT ddpf)
{
    char szFlags[4096];
    char szMsg[MAX_PATH];

    PixelFormatHelper( ddpf.dwFlags, szFlags);

    OutputDebugString(szFlags);

    sprintf( szMsg, "    dwFourCC: %ld\n", ddpf.dwFourCC);
    OutputDebugString(szMsg);

    sprintf( szMsg, "    dwRGBBitCount: %ld\n", ddpf.dwRGBBitCount);
    OutputDebugString(szMsg);
    OutputDebugString("    (dwYUVBitCount, dwZBufferBitDepth, dwAlphaBitDepth, dwLuminanceBitCount, dwBumpBitCount)\n");

    sprintf( szMsg, "    dwRBitMask: %ld\n", ddpf.dwRBitMask);
    OutputDebugString(szMsg);
    OutputDebugString("    (dwYBitMask, dwStencilBitDepth, dwLuminanceBitMask, dwBumpDuBitMask)\n");

    sprintf( szMsg, "    dwGBitMask: %ld\n", ddpf.dwGBitMask);
    OutputDebugString(szMsg);
    OutputDebugString("    (dwUBitMask, dwZBitMask, dwBumpDvBitMask)\n");

    sprintf( szMsg, "    dwBBitMask: %ld\n", ddpf.dwBBitMask);
    OutputDebugString(szMsg);
    OutputDebugString("    (dwVBitMask, dwStencilBitMask, dwBumpLuminanceBitMask)\n");

    sprintf( szMsg, "    dwRGBAlphaBitMask: %ld\n", ddpf.dwRGBAlphaBitMask);
    OutputDebugString(szMsg);
    OutputDebugString("    (dwYUVAlphaBitMask, dwLuminanceAlphaBitMask, dwRGBZBitMask, dwYUVZBitMask)\n");
}

void SurfaceDescHelper( DWORD dwFlags, char * pszFlags )
{
    if( !pszFlags )
    {
        return;
    }
    strcpy( pszFlags, "  dwFlags: ");

    if( dwFlags & DDSD_ALL )
        strcat( pszFlags, "DDSD_ALL\n");

    if( dwFlags & DDSD_ALPHABITDEPTH  )
        strcat( pszFlags, "DDSD_ALPHABITDEPTH\n");

    if( dwFlags & DDSD_BACKBUFFERCOUNT  )
        strcat( pszFlags, "DDSD_BACKBUFFERCOUNT\n");

    if( dwFlags & DDSD_CAPS  )
        strcat( pszFlags, "DDSD_CAPS\n");

    if( dwFlags & DDSD_CKDESTBLT   )
        strcat( pszFlags, "DDSD_CKDESTBLT\n");

    if( dwFlags & DDSD_CKDESTOVERLAY   )
        strcat( pszFlags, "DDSD_CKDESTOVERLAY\n");

    if( dwFlags & DDSD_CKSRCBLT   )
        strcat( pszFlags, "DDSD_CKSRCBLT\n");

    if( dwFlags & DDSD_CKSRCOVERLAY    )
        strcat( pszFlags, "DDSD_CKSRCOVERLAY\n");

    if( dwFlags & DDSD_HEIGHT    )
        strcat( pszFlags, "DDSD_HEIGHT\n");

    if( dwFlags & DDSD_LINEARSIZE    )
        strcat( pszFlags, "DDSD_LINEARSIZE\n");

    if( dwFlags & DDSD_LPSURFACE    )
        strcat( pszFlags, "DDSD_LPSURFACE\n");

    if( dwFlags & DDSD_MIPMAPCOUNT     )
        strcat( pszFlags, "DDSD_MIPMAPCOUNT\n");

    if( dwFlags & DDSD_PITCH     )
        strcat( pszFlags, "DDSD_PITCH\n");

    if( dwFlags & DDSD_PIXELFORMAT     )
        strcat( pszFlags, "DDSD_PIXELFORMAT\n");

    if( dwFlags & DDSD_REFRESHRATE     )
        strcat( pszFlags, "DDSD_REFRESHRATE\n");

    if( dwFlags & DDSD_TEXTURESTAGE      )
        strcat( pszFlags, "DDSD_TEXTURESTAGE\n");

    if( dwFlags & DDSD_WIDTH      )
        strcat( pszFlags, "DDSD_WIDTH\n");

    strcat(pszFlags, "\n");
}


void PixelFormatHelper( DWORD dwFlags, char * pszFlags)
{
    if( !pszFlags )
    {
        return;
    }

    strcpy( pszFlags, "    dwFlags: ");

    if( dwFlags & DDPF_ALPHA )
        strcat( pszFlags, "DDPF_ALPHA, ");

    if( dwFlags & DDPF_ALPHAPIXELS  )
        strcat( pszFlags, "DDPF_ALPHAPIXELS, ");

    if( dwFlags & DDPF_ALPHAPREMULT  )
        strcat( pszFlags, "DDPF_ALPHAPREMULT, ");

    if( dwFlags & DDPF_BUMPLUMINANCE  )
        strcat( pszFlags, "DDPF_BUMPLUMINANCE, ");

    if( dwFlags & DDPF_BUMPDUDV   )
        strcat( pszFlags, "DDPF_BUMPDUDV, ");

    if( dwFlags & DDPF_COMPRESSED   )
        strcat( pszFlags, "DDPF_COMPRESSED, ");

    if( dwFlags & DDPF_FOURCC   )
        strcat( pszFlags, "DDPF_FOURCC, ");

    if( dwFlags & DDPF_LUMINANCE    )
        strcat( pszFlags, "DDPF_LUMINANCE, ");

    if( dwFlags & DDPF_PALETTEINDEXED1    )
        strcat( pszFlags, "DDPF_PALETTEINDEXED1, ");

    if( dwFlags & DDPF_PALETTEINDEXED2    )
        strcat( pszFlags, "DDPF_PALETTEINDEXED2, ");

    if( dwFlags & DDPF_PALETTEINDEXED4    )
        strcat( pszFlags, "DDPF_PALETTEINDEXED4, ");

    if( dwFlags & DDPF_PALETTEINDEXED8    )
        strcat( pszFlags, "DDPF_PALETTEINDEXED8, ");

    if( dwFlags & DDPF_PALETTEINDEXEDTO8  )
        strcat( pszFlags, "DDPF_PALETTEINDEXEDTO8, ");
    
    if( dwFlags & DDPF_RGB   )
        strcat( pszFlags, "DDPF_RGB, ");

    if( dwFlags & DDPF_RGBTOYUV    )
        strcat( pszFlags, "DDPF_RGBTOYUV, ");

    if( dwFlags & DDPF_STENCILBUFFER     )
        strcat( pszFlags, "DDPF_STENCILBUFFER, ");

    if( dwFlags & DDPF_YUV     )
        strcat( pszFlags, "DDPF_YUV, ");

    if( dwFlags & DDPF_ZBUFFER     )
        strcat( pszFlags, "DDPF_ZBUFFER, ");

    if( dwFlags & DDPF_ZPIXELS     )
        strcat( pszFlags, "DDPF_ZPIXELS, ");

    strcat( pszFlags, "\n");
}


/*****************************Private*Routine******************************\
* VerifyVMR
*
\**************************************************************************/
BOOL VerifyVMR(void)
{
    HRESULT hres;

    CoInitialize(NULL);

    // Verify that the VMR exists on this system
    IBaseFilter* pBF = NULL;
    hres = CoCreateInstance(CLSID_VideoMixingRenderer,
        NULL,
        CLSCTX_INPROC,
        IID_IBaseFilter,
        (LPVOID *)&pBF);
    if(SUCCEEDED(hres))
    {
        pBF->Release();
        CoUninitialize();
        return TRUE;
    }
    else
    {
        MessageBox(hwndApp, 
            TEXT("This application requires the Video Mixing Renderer, which is present\r\n")
            TEXT("only on Windows XP.\r\n\r\n")
            TEXT("The Video Mixing Renderer (VMR) is also not enabled when viewing a \r\n")
            TEXT("remote Windows XP machine through a Remote Desktop session.\r\n")
            TEXT("You can run VMR-enabled applications only on your local machine.")
            TEXT("\r\n\r\nThis sample will now exit."),
            TEXT("Video Mixing Renderer capabilities are required"), MB_OK);

        CoUninitialize();
        return FALSE;
    }
}
