//------------------------------------------------------------------------------
// File: Utils.h
//
// Desc: DirectShow sample code
//       Helper functions
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------
#include "stdafx.h"
#include "utils.h"
    
//-----------------------------------------------------------------------------------------
//  Function:   MyMessage
//  Purpose:    Displays a quick message box
//  Arguments:  Input strings that will be displayed
//  Returns:    button pushed in box
//-----------------------------------------------------------------------------------------*/
DWORD MyMessage(char *sQuestion, char *sTitle)
{
    int iReturn = AfxMessageBox( sQuestion, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON1);
    return iReturn == IDYES  ? FNS_PASS : FNS_FAIL;
}
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

//-----------------------------------------------------------------------------------------
//  Function:   MySleep
//  Purpose:    if the application is in automated mode, then sleep func is turned off
//  Arguments:  checks m_bAutomatedStatus to see if the func is in automation
//  Returns:    true if automated, false otherwist
//-----------------------------------------------------------------------------------------*/    
bool MySleep(DWORD  dwTime)
{
    HANDLE hNeverHappensEvent;
    hNeverHappensEvent = CreateEvent(NULL, FALSE, FALSE, "EVENTTHATNEVERHAPPENS");
    WaitForSingleObject( hNeverHappensEvent, dwTime);
    return false;

} // end of checkHResult method

//-----------------------------------------------------------------------------------------
//  Function:   ShellAbort
//  Purpose:    Prints a crash message text to the appropriate log(s)
//  Arguments:  none
//  Returns:    FNS_ABORTED
//-----------------------------------------------------------------------------------------*/
DWORD ShellAbort(CVMRCore *core)
{
    AfxMessageBox("Unhandled exception, press OK to abort...");
    exit(-1);
    return NULL;
}

//-----------------------------------------------------------------------------------------
//  Function:   GetMessageName
//  Purpose:    Updates Name to the string version of the windows message
//  Arguments:  Name, long pointer to a string that will be updated
//              msg - message id that we want displayed
//-----------------------------------------------------------------------------------------*/
void GetMessageName(LPSTR Name, UINT msg)
{   
    switch(msg)
    {
        // For put_MessageDrain and get_MessageDrain   

        case WM_KEYDOWN:         lstrcpy(Name, "WM_KEYDOWN");         break;
        case WM_KEYUP:           lstrcpy(Name, "WM_KEYUP");           break; 
        case WM_LBUTTONDBLCLK:   lstrcpy(Name, "WM_LBUTTONDBLCLK");   break;
        case WM_LBUTTONDOWN:     lstrcpy(Name, "WM_LBUTTONDOWN");     break;
        case WM_LBUTTONUP:       lstrcpy(Name, "WM_LBUTTONUP");       break;
        case WM_MBUTTONDBLCLK:   lstrcpy(Name, "WM_MBUTTONDBLCLK");   break;
        case WM_MBUTTONDOWN:     lstrcpy(Name, "WM_MBUTTONDOWN");     break;
        case WM_MBUTTONUP:       lstrcpy(Name, "WM_MBUTTONUP");       break;
        case WM_MOUSEACTIVATE:   lstrcpy(Name, "WM_MOUSEACTIVATE");   break;
        case WM_MOUSEMOVE:       lstrcpy(Name, "WM_MOUSEMOVE");       break;
        case WM_NCHITTEST:       lstrcpy(Name, "WM_NCHITTEST");       break;
        case WM_NCLBUTTONDBLCLK: lstrcpy(Name, "WM_NCLBUTTONDBLCLK"); break;
        case WM_NCLBUTTONDOWN:   lstrcpy(Name, "WM_NCLBUTTONDOWN");   break;
        case WM_NCLBUTTONUP:     lstrcpy(Name, "WM_NCLBUTTONUP");     break;
        case WM_NCMBUTTONDBLCLK: lstrcpy(Name, "WM_NCMBUTTONDBLCLK"); break;
        case WM_NCMBUTTONDOWN:   lstrcpy(Name, "WM_NCMBUTTONDOWN");   break;
        case WM_NCMBUTTONUP:     lstrcpy(Name, "WM_NCMBUTTONUP");     break;
        case WM_NCMOUSEMOVE:     lstrcpy(Name, "WM_NCMOUSEMOVE");     break;
        case WM_NCRBUTTONDBLCLK: lstrcpy(Name, "WM_NCRBUTTONDBLCLK"); break;
        case WM_NCRBUTTONDOWN:   lstrcpy(Name, "WM_NCRBUTTONDOWN");   break;
        case WM_NCRBUTTONUP:     lstrcpy(Name, "WM_NCRBUTTONUP");     break;
        case WM_RBUTTONDBLCLK:   lstrcpy(Name, "WM_RBUTTONDBLCLK");   break;
        case WM_RBUTTONDOWN:     lstrcpy(Name, "WM_RBUTTONDOWN");     break;
        case WM_RBUTTONUP:       lstrcpy(Name, "WM_RBUTTONUP");       break;

        // For NotifyOwnerMessage
            
        case WM_DEVMODECHANGE:     lstrcpy(Name,"WM_DEVMODECHANGE");     break;
        case WM_DISPLAYCHANGE:     lstrcpy(Name,"WM_DISPLAYCHANGE");     break;
        case WM_MOVE:              lstrcpy(Name,"WM_MOVE");              break;
        case WM_PALETTECHANGED:    lstrcpy(Name,"WM_PALETTECHANGED");    break;
        case WM_PALETTEISCHANGING: lstrcpy(Name,"WM_PALETTEISCHANGING"); break;
        case WM_QUERYNEWPALETTE:   lstrcpy(Name,"WM_QUERYNEWPALETTE");   break;
        case WM_SYSCOLORCHANGE:    lstrcpy(Name,"WM_SYSCOLORCHANGE");    break;

        default:                   wsprintf(Name, "Unknown Messgage %u", msg);
    }

    return;
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
        MessageBox(NULL, 
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
