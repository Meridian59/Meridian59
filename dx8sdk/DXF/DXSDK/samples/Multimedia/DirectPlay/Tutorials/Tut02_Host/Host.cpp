//----------------------------------------------------------------------------
// File: Host.cpp
//
// Desc: This simple program builds upon the EnumSP.cpp tutorial and adds 
//       creation of an Address Object and hosting a session
//
// Copyright (c) 2000-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define INITGUID
#define _WIN32_DCOM
#include <stdio.h>
#include <conio.h>
#include <dplay8.h>


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
IDirectPlay8Peer*                   g_pDP = NULL;
IDirectPlay8Address*                g_pDeviceAddress = NULL;

// This GUID allows DirectPlay to find other instances of the same game on
// the network.  So it must be unique for every game, and the same for 
// every instance of that game.  // {5e4ab2ee-6a50-4614-807e-c632807b5eb1}
GUID g_guidApp = {0x5e4ab2ee, 0x6a50, 0x4614, {0x80, 0x7e, 0xc6, 0x32, 0x80, 0x7b, 0x5e, 0xb1}};


//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
HRESULT WINAPI DirectPlayMessageHandler(PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer);
BOOL    IsServiceProviderValid(const GUID* pGuidSP);
HRESULT InitDirectPlay();
HRESULT CreateDeviceAddress();
HRESULT HostSession();
void    CleanupDirectPlay();


//-----------------------------------------------------------------------------
// Miscellaneous helper functions
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)          {if(p) {delete (p);     (p)=NULL;}}
#define SAFE_DELETE_ARRAY(p)    {if(p) {delete[] (p);   (p)=NULL;}}
#define SAFE_RELEASE(p)         {if(p) {(p)->Release(); (p)=NULL;}}




//-----------------------------------------------------------------------------
// Name: main()
// Desc: Entry point for the application.  
//-----------------------------------------------------------------------------
int main(int argc, char* argv[], char* envp[])
{
    HRESULT hr;

    // Init COM so we can use CoCreateInstance
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    // Init the DirectPlay system
    if( FAILED( hr = InitDirectPlay() ) )
    {
        printf("Failed Initializing DirectPlay:  0x%X\n", hr);
        goto LCleanup;
    }

    if( FAILED( hr = CreateDeviceAddress() ) )
    {
        printf("Failed CreatingDeviceAddress:  0x%X\n", hr);
        goto LCleanup;
    }

    if( FAILED( hr = HostSession() ) )
    {
        printf("Failed Hosting:  0x%X\n", hr);
        goto LCleanup;
    }

    // Wait for user action
    printf("Press a key to exit\n");
    _getch();

LCleanup:
    CleanupDirectPlay();

    // Cleanup COM
    CoUninitialize();

    return 0;
}




//-----------------------------------------------------------------------------
// Name: InitDirectPlay()
// Desc: Initialize DirectPlay
//-----------------------------------------------------------------------------
HRESULT InitDirectPlay()
{
    HRESULT     hr = S_OK;

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
    
    // Ensure that TCP/IP is a valid Service Provider
    if( FALSE == IsServiceProviderValid(&CLSID_DP8SP_TCPIP ) )
    {
        hr = E_FAIL;
        printf("Failed validating CLSID_DP8SP_TCPIP");
        goto LCleanup;
    }

LCleanup:
    return hr;
}




//-----------------------------------------------------------------------------
// Name: DirectPlayMessageHandler
// Desc: Handler for DirectPlay messages.  This tutorial doesn't repond to any
//       DirectPlay messages
//-----------------------------------------------------------------------------
HRESULT WINAPI DirectPlayMessageHandler(PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer)
{
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: IsServiceProviderValid()
// Desc: Return TRUE if the service provider is valid
//-----------------------------------------------------------------------------
BOOL IsServiceProviderValid(const GUID* pGuidSP)
{
    HRESULT                     hr;
    DPN_SERVICE_PROVIDER_INFO*  pdnSPInfo = NULL;
    DWORD                       dwItems = 0;
    DWORD                       dwSize = 0;

    hr = g_pDP->EnumServiceProviders( &CLSID_DP8SP_TCPIP, NULL, NULL, 
                                      &dwSize, &dwItems, 0 );

    if( hr != DPNERR_BUFFERTOOSMALL )
    {
        printf("Failed Enumerating Service Providers:  0x%x\n", hr);
        goto LCleanup;
    }

    pdnSPInfo = (DPN_SERVICE_PROVIDER_INFO*) new BYTE[dwSize];

    if( FAILED( hr = g_pDP->EnumServiceProviders( &CLSID_DP8SP_TCPIP, NULL, pdnSPInfo, 
                                                  &dwSize, &dwItems, 0 ) ) )
    {
        printf("Failed Enumerating Service Providers:  0x%x\n", hr);
        goto LCleanup;
    }

    // There are no items returned so the requested SP is not available
    if( dwItems == 0)
    {
        hr = E_FAIL;
    }

LCleanup:
    SAFE_DELETE_ARRAY(pdnSPInfo);
    
    if( SUCCEEDED(hr) )
        return TRUE;
    else
        return FALSE;
}




//-----------------------------------------------------------------------------
// Name: CreateDeviceAddress()
// Desc: Creates a device address
//-----------------------------------------------------------------------------
HRESULT CreateDeviceAddress()
{
    HRESULT         hr = S_OK;

    // Create our IDirectPlay8Address Device Address
    if( FAILED( hr = CoCreateInstance( CLSID_DirectPlay8Address, NULL,
                                       CLSCTX_INPROC_SERVER,
                                       IID_IDirectPlay8Address,
                                       (LPVOID*) &g_pDeviceAddress ) ) )
    {
        printf("Failed Creating the IDirectPlay8Address Object:  0x%X\n", hr);
        goto LCleanup;
    }
    
    // Set the SP for our Device Address
    if( FAILED( hr = g_pDeviceAddress->SetSP(&CLSID_DP8SP_TCPIP ) ) )
    {
        printf("Failed Setting the Service Provider:  0x%X\n", hr);
        goto LCleanup;
    }

LCleanup:
    return hr;
}




//-----------------------------------------------------------------------------
// Name: HostSession()
// Desc: Host a DirectPlay session
//-----------------------------------------------------------------------------
HRESULT HostSession()
{
    HRESULT                 hr = S_OK;
    DPN_APPLICATION_DESC    dpAppDesc;

    // Now set up the Application Description
    ZeroMemory(&dpAppDesc, sizeof(DPN_APPLICATION_DESC));
    dpAppDesc.dwSize = sizeof(DPN_APPLICATION_DESC);
    dpAppDesc.guidApplication = g_guidApp;

    // We are now ready to host the app
    if( FAILED( hr = g_pDP->Host( &dpAppDesc,            // AppDesc
                                  &g_pDeviceAddress, 1,   // Device Address
                                  NULL, NULL,             // Reserved
                                  NULL,                   // Player Context
                                  0 ) ) )                 // dwFlags
    {
        printf("Failed Hosting:  0x%X\n", hr);
        goto LCleanup;
    }
    else
    {
        printf("Currently Hosting...\n");
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

    SAFE_RELEASE(g_pDeviceAddress);
    SAFE_RELEASE(g_pDP);
}
