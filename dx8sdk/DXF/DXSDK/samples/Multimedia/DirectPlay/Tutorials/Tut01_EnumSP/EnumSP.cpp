//----------------------------------------------------------------------------
// File: EnumSP.cpp
//
// Desc: This simple program inits DirectPlay and enumerates the available
//       DirectPlay Service Providers.
//
// Copyright (c) 2000-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define INITGUID
#define _WIN32_DCOM
#include <stdio.h>
#include <dplay8.h>


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
IDirectPlay8Peer* g_pDP = NULL;    // DirectPlay peer object


//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
HRESULT WINAPI DirectPlayMessageHandler(PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer);
HRESULT InitDirectPlay();
void    CleanupDirectPlay();


//-----------------------------------------------------------------------------
// Miscellaneous helper functions
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }




//-----------------------------------------------------------------------------
// Name: main()
// Desc: Entry point for the application.  
//-----------------------------------------------------------------------------
int main(int argc, char* argv[], char* envp[])
{
    HRESULT                     hr;
    DPN_SERVICE_PROVIDER_INFO*  pdnSPInfo       = NULL;
    DPN_SERVICE_PROVIDER_INFO*  pdnSPInfoEnum   = NULL;
    DWORD                       dwItems         = 0;
    DWORD                       dwSize          = 0;
    DWORD                       i;

    // Init COM so we can use CoCreateInstance
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    // Init the DirectPlay system
    if( FAILED( hr = InitDirectPlay() ) )
    {
        printf("Failed Initializing DirectPlay:  0x%X\n", hr);
        goto LCleanup;
    }

    // Enumerate all the Service Providers available
    hr = g_pDP->EnumServiceProviders(NULL, NULL, NULL, &dwSize, &dwItems, 0);

    if( hr != DPNERR_BUFFERTOOSMALL)
    {
        printf("Failed Enumerating Service Providers:  0x%X\n", hr);
        goto LCleanup;
    }

    pdnSPInfo = (DPN_SERVICE_PROVIDER_INFO*) new BYTE[dwSize];
    
    if( FAILED( hr = g_pDP->EnumServiceProviders(NULL, NULL, pdnSPInfo, &dwSize, &dwItems, 0 ) ) )
    {
        printf("Failed Enumerating Service Providers:  0x%X\n", hr);
        goto LCleanup;
    }

    // Run through each provider desc printing them all out
    pdnSPInfoEnum = pdnSPInfo;
    for (i = 0; i < dwItems; i++)
    {
        printf("Found Service Provider:  %S\n", pdnSPInfoEnum->pwszName);

        pdnSPInfoEnum++;
    }

LCleanup:

    SAFE_DELETE_ARRAY(pdnSPInfo);

    CleanupDirectPlay();
    
    // Cleanup COM
    CoUninitialize();

    return 0;
}




//-----------------------------------------------------------------------------
// Name: DirectPlayMessageHandler
// Desc: Handler for DirectPlay messages.  This tutorial doesn't repond to any
//       DirectPlay messages
//-----------------------------------------------------------------------------
HRESULT WINAPI DirectPlayMessageHandler( PVOID pvUserContext, DWORD dwMessageId, 
                                         PVOID pMsgBuffer)
{
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: InitDirectPlay()
// Desc: Initialize DirectPlay
//-----------------------------------------------------------------------------
HRESULT InitDirectPlay()
{
    HRESULT hr = S_OK;

    // Create the IDirectPlay8Peer Object
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8Peer, NULL, 
                                       CLSCTX_INPROC_SERVER,
                                       IID_IDirectPlay8Peer, 
                                       (LPVOID*) &g_pDP ) ) )
    {
        printf("Failed Creating the IDirectPlay8Peer Object:  0x%X\n", hr);
        goto LCleanup;
    }

    // Init DirectPlay
    if( FAILED( hr = g_pDP->Initialize(NULL, DirectPlayMessageHandler, 0 ) ) )
    {
        printf("Failed Initializing DirectPlay:  0x%X\n", hr);
        goto LCleanup;
    }
    
LCleanup:
    return hr;
}




//-----------------------------------------------------------------------------
// Name: CleanupDirectPlay()
// Desc: Cleanup DirectPlay
//-----------------------------------------------------------------------------
void CleanupDirectPlay()
{
    // Cleanup DirectPlay
    if( g_pDP)
        g_pDP->Close(0);

    SAFE_RELEASE(g_pDP);
}
