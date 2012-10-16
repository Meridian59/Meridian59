//----------------------------------------------------------------------------
// File: HostMigration.cpp
//
// Desc: This simple program builds upon the last tutorial and adds host migration
//
// Copyright (c) 2000-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#define INITGUID
#define _WIN32_DCOM
#include <stdio.h>
#include <dplay8.h>



//-----------------------------------------------------------------------------
// App specific structures 
//-----------------------------------------------------------------------------
struct HOST_NODE
{
    DPN_APPLICATION_DESC*   pAppDesc;
    IDirectPlay8Address*    pHostAddress;
    WCHAR*                  pwszSessionName;

    HOST_NODE*              pNext;
};


//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
IDirectPlay8Peer*                   g_pDP               = NULL;
IDirectPlay8Address*                g_pDeviceAddress    = NULL;
IDirectPlay8Address*                g_pHostAddress      = NULL;
BOOL                                g_bHost;
HOST_NODE*                          g_pHostList         = NULL;
CRITICAL_SECTION                    g_csHostList;
DPNID                               g_dpnidLocalPlayer  = 0;


// This GUID allows DirectPlay to find other instances of the same game on
// the network.  So it must be unique for every game, and the same for 
// every instance of that game.  // {2C6FEE8B-3088-4b74-B38F-A16D02DAD246}
GUID g_guidApp = { 0x2c6fee8b, 0x3088, 0x4b74, { 0xb3, 0x8f, 0xa1, 0x6d, 0x2, 0xda, 0xd2, 0x46 } };


//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
HRESULT WINAPI DirectPlayMessageHandler(PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer);
BOOL    IsServiceProviderValid(const GUID* pGuidSP);
HRESULT InitDirectPlay();
HRESULT CreateDeviceAddress();
HRESULT CreateHostAddress(WCHAR* pwszHost);
HRESULT HostSession();
HRESULT EnumDirectPlayHosts();
HRESULT ConnectToSession();
HRESULT SendDirectPlayMessage();
void    CleanupDirectPlay();


//-----------------------------------------------------------------------------
// Miscellaneous helper functions
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)          {if(p) {delete (p);     (p)=NULL;}}
#define SAFE_DELETE_ARRAY(p)    {if(p) {delete[] (p);   (p)=NULL;}}
#define SAFE_RELEASE(p)         {if(p) {(p)->Release(); (p)=NULL;}}

#define USER_HOST       1
#define USER_CONNECT    2
#define USER_EXIT       1
#define USER_SEND       2




//-----------------------------------------------------------------------------
// Name: main()
// Desc: Entry point for the application.  
//-----------------------------------------------------------------------------
int main(int argc, char* argv[], char* envp[])
{
    HRESULT                     hr;
    int                         iUserChoice;

    // Init COM so we can use CoCreateInstance
    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    // Init the DirectPlay system
    if( FAILED( hr = InitDirectPlay() ) )
    {
        printf("Failed Initializing DirectPlay:  0x%X\n", hr);
        goto LCleanup;
    }

    InitializeCriticalSection(&g_csHostList);

    // Get the necessary user input on whether they are hosting or connecting
    do
    {
        printf("Please select one.\n1.  Host\n2.  Connect\n");
        scanf("%d", &iUserChoice);
    } while (iUserChoice != USER_HOST && iUserChoice != USER_CONNECT);


    if( FAILED( hr = CreateDeviceAddress() ) )
    {
        printf("Failed CreatingDeviceAddress:  0x%X\n", hr);
        goto LCleanup;
    }

    if( iUserChoice == USER_HOST)
    {
        if( FAILED( hr = HostSession() ) )
        {
            printf("Failed Hosting:  0x%X\n", hr);
            goto LCleanup;
        }
    }
    else
    {
        if( FAILED( hr = EnumDirectPlayHosts() ) )
        {
            printf("Failed Enumerating Host:  0x%X\n", hr);
            goto LCleanup;
        }

        if( FAILED( hr = ConnectToSession() ) )
        {
            printf("Failed Connect to Host:  0x%X\n", hr);
            goto LCleanup;
        }
        else
        {
            printf("\nConnection Successful.\n");
        }
    }

    // Present User with Choices
    do
    {
        printf("Please select one.\n1.  Exit\n2.  Send Data\n");
        scanf("%d", &iUserChoice);

        if( iUserChoice == USER_SEND)
        {
            if( FAILED( hr = SendDirectPlayMessage() ) )
            {
                printf("Failed To Send Data:  0x%X\n", hr);
                goto LCleanup;
            }
        }
    } while (iUserChoice != USER_EXIT);

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
// Name: IsServiceProviderValid()
// Desc: Return TRUE if the service provider is valid
//-----------------------------------------------------------------------------
BOOL IsServiceProviderValid(const GUID* pGuidSP)
{
    HRESULT                     hr;
    DPN_SERVICE_PROVIDER_INFO*  pdnSPInfo = NULL;
    DWORD                       dwItems = 0;
    DWORD                       dwSize = 0;

    hr = g_pDP->EnumServiceProviders( &CLSID_DP8SP_TCPIP, NULL, NULL, &dwSize, &dwItems, 0);

    if( hr != DPNERR_BUFFERTOOSMALL)
    {
        printf("Failed Enumerating Service Providers:  0x%x\n", hr);
        goto LCleanup;
    }

    pdnSPInfo = (DPN_SERVICE_PROVIDER_INFO*) new BYTE[dwSize];

    if( FAILED( hr = g_pDP->EnumServiceProviders( &CLSID_DP8SP_TCPIP, NULL, pdnSPInfo, &dwSize, &dwItems, 0 ) ) )
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
// Name: DirectPlayMessageHandler
// Desc: Handler for DirectPlay messages.  
//-----------------------------------------------------------------------------
HRESULT WINAPI DirectPlayMessageHandler( PVOID pvUserContext, DWORD dwMessageId, 
                                         PVOID pMsgBuffer )
{
    HRESULT     hr = S_OK;

    switch (dwMessageId)
    {
        case DPN_MSGID_ENUM_HOSTS_RESPONSE:
        {
            PDPNMSG_ENUM_HOSTS_RESPONSE pEnumHostsResponseMsg;
            const DPN_APPLICATION_DESC* pAppDesc;
            HOST_NODE*                  pHostNode   = NULL;
            WCHAR*                      pwszSession = NULL;

            pEnumHostsResponseMsg = (PDPNMSG_ENUM_HOSTS_RESPONSE) pMsgBuffer;
            pAppDesc = pEnumHostsResponseMsg->pApplicationDescription;

            // Insert each host response if it isn't already present
            EnterCriticalSection(&g_csHostList);

            for (pHostNode = g_pHostList; pHostNode; pHostNode = pHostNode->pNext)
            {
                if( pAppDesc->guidInstance == pHostNode->pAppDesc->guidInstance)
                {
                    // This host is already in the list
                    pHostNode = NULL;
                    goto Break_ENUM_HOSTS_RESPONSE;
                }
            }

            // This host session is not in the list then so insert it.
            pHostNode = new HOST_NODE;
            if( pHostNode == NULL)
            {
                goto Break_ENUM_HOSTS_RESPONSE;
            }

            ZeroMemory(pHostNode, sizeof(HOST_NODE));

            // Copy the Host Address
            if( FAILED( pEnumHostsResponseMsg->pAddressSender->Duplicate(&pHostNode->pHostAddress ) ) )
            {
                goto Break_ENUM_HOSTS_RESPONSE;
            }

            pHostNode->pAppDesc = new DPN_APPLICATION_DESC;

            if( pHostNode == NULL)
            {
                goto Break_ENUM_HOSTS_RESPONSE;
            }

            ZeroMemory(pHostNode->pAppDesc, sizeof(DPN_APPLICATION_DESC));
            memcpy(pHostNode->pAppDesc, pAppDesc, sizeof(DPN_APPLICATION_DESC));

            // Null out all the pointers we aren't copying
            pHostNode->pAppDesc->pwszSessionName = NULL;
            pHostNode->pAppDesc->pwszPassword = NULL;
            pHostNode->pAppDesc->pvReservedData = NULL;
            pHostNode->pAppDesc->dwReservedDataSize = 0;
            pHostNode->pAppDesc->pvApplicationReservedData = NULL;
            pHostNode->pAppDesc->dwApplicationReservedDataSize = 0;
            
            if( pAppDesc->pwszSessionName)
            {
                pwszSession = new WCHAR[wcslen(pAppDesc->pwszSessionName) + 1];
                
                if( pwszSession)
                {
                    wcscpy(pwszSession, pAppDesc->pwszSessionName);
                }
            }

            pHostNode->pwszSessionName = pwszSession;

            // Insert it onto the front of the list
            pHostNode->pNext = g_pHostList ? g_pHostList->pNext : NULL;
            g_pHostList = pHostNode;
            pHostNode = NULL;

Break_ENUM_HOSTS_RESPONSE:
            LeaveCriticalSection(&g_csHostList);

            if( pHostNode)
            {
                SAFE_RELEASE(pHostNode->pHostAddress);

                SAFE_DELETE(pHostNode->pAppDesc);

                delete pHostNode;
            }

            break;
        }
    
        case DPN_MSGID_RECEIVE:
        {
            PDPNMSG_RECEIVE     pMsg;

            pMsg = (PDPNMSG_RECEIVE) pMsgBuffer;

            printf("\nReceived Message:  %S\n", (WCHAR*)pMsg->pReceiveData);
            break;
        }
    
        case DPN_MSGID_HOST_MIGRATE:
        {
            PDPNMSG_HOST_MIGRATE    pHostMigrateMsg;

            pHostMigrateMsg = (PDPNMSG_HOST_MIGRATE) pMsgBuffer;

            printf("\nHost Migration Has Occured.\n");

            // See if we are the new host
            if( pHostMigrateMsg->dpnidNewHost == g_dpnidLocalPlayer)
                printf("You are the New Host\n");
            else
                printf("DPNID of New Host is %d\n", pHostMigrateMsg->dpnidNewHost);

            break;
        }
    
        case DPN_MSGID_CREATE_PLAYER:
        {
            PDPNMSG_CREATE_PLAYER   pCreatePlayerMsg;
            DWORD                   dwSize = 0;
            DPN_PLAYER_INFO*        pdpPlayerInfo = NULL;

            pCreatePlayerMsg = (PDPNMSG_CREATE_PLAYER)pMsgBuffer;

            // check to see if we are the player being created
            hr = g_pDP->GetPeerInfo(pCreatePlayerMsg->dpnidPlayer, pdpPlayerInfo, &dwSize, 0);

            if( FAILED(hr) && hr != DPNERR_BUFFERTOOSMALL)
            {
                printf("Failed GetPeerInfo:  0x%X\n", hr);
                return hr;
            }

            pdpPlayerInfo = (DPN_PLAYER_INFO*) new BYTE[dwSize];
            ZeroMemory(pdpPlayerInfo, dwSize);
            pdpPlayerInfo->dwSize = sizeof(DPN_PLAYER_INFO);

            if( FAILED( hr = g_pDP->GetPeerInfo(pCreatePlayerMsg->dpnidPlayer, pdpPlayerInfo, &dwSize, 0 ) ) )
            {
                printf("Failed GetPeerInfo:  0x%X\n", hr);
                goto Error_DPN_MSGID_CREATE_PLAYER;
            }

            if( pdpPlayerInfo->dwPlayerFlags & DPNPLAYER_LOCAL)
                g_dpnidLocalPlayer = pCreatePlayerMsg->dpnidPlayer;

Error_DPN_MSGID_CREATE_PLAYER:
            SAFE_DELETE_ARRAY(pdpPlayerInfo);
            break;
        }
    }
    return hr;
}




//-----------------------------------------------------------------------------
// Name: EnumDirectPlayHosts()
// Desc: Enumerates the hosts
//-----------------------------------------------------------------------------
HRESULT EnumDirectPlayHosts()
{
    HRESULT                 hr = S_OK;
    WCHAR                   wszHost[128];
    DPN_APPLICATION_DESC    dpAppDesc;
    WCHAR*                  pwszURL = NULL;

    // Prompt for the hostname/ip
    printf("\nPlease enter the IP address of host:\n");
    wscanf(L"%ls", wszHost);

    if( FAILED( hr = CreateHostAddress(wszHost ) ) )
    {
        printf("Failed Creating Host Address:  0x%X\n", hr);
        goto LCleanup;
    }

    // Now set up the Application Description
    ZeroMemory(&dpAppDesc, sizeof(DPN_APPLICATION_DESC));
    dpAppDesc.dwSize = sizeof(DPN_APPLICATION_DESC);
    dpAppDesc.guidApplication = g_guidApp;

    // We now have the host address so lets enum
    if( FAILED( hr = g_pDP->EnumHosts(&dpAppDesc,            // pApplicationDesc
                                        g_pHostAddress,     // pdpaddrHost
                                        g_pDeviceAddress,   // pdpaddrDeviceInfo
                                        NULL, 0,            // pvUserEnumData, size
                                        4,                  // dwEnumCount
                                        0,                  // dwRetryInterval
                                        0,                  // dwTimeOut
                                        NULL,               // pvUserContext
                                        NULL,               // pAsyncHandle
                                        DPNENUMHOSTS_SYNC ) ) )// dwFlags
    {
        printf("Failed Enumerating the Hosts:  0x%X\n", hr);
        goto LCleanup;
    }

LCleanup:
    return hr;
}




//-----------------------------------------------------------------------------
// Name: CreateDeviceAddress()
// Desc: Creates a device address
//-----------------------------------------------------------------------------
HRESULT CreateDeviceAddress()
{
    HRESULT         hr = S_OK;

    // Create our IDirectPlay8Address Device Address
    if( FAILED( hr = CoCreateInstance(CLSID_DirectPlay8Address, NULL,
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
// Name: CreateHostAddress()
// Desc: Creates a host address
//-----------------------------------------------------------------------------
HRESULT CreateHostAddress(WCHAR* pwszHost)
{
    HRESULT         hr = S_OK;

    // Create our IDirectPlay8Address Host Address
    if( FAILED( hr = CoCreateInstance(CLSID_DirectPlay8Address, NULL,
                                    CLSCTX_INPROC_SERVER,
                                    IID_IDirectPlay8Address,
                                    (LPVOID*) &g_pHostAddress ) ) )
    {
        printf("Failed Creating the IDirectPlay8Address Object:  0x%X\n", hr);
        goto LCleanup;
    }
    
    // Set the SP for our Host Address
    if( FAILED( hr = g_pHostAddress->SetSP(&CLSID_DP8SP_TCPIP ) ) )
    {
        printf("Failed Setting the Service Provider:  0x%X\n", hr);
        goto LCleanup;
    }

    // Set the hostname into the address
    if( FAILED( hr = g_pHostAddress->AddComponent(DPNA_KEY_HOSTNAME, pwszHost,
                                                    2*(wcslen(pwszHost) + 1), /*bytes*/
                                                    DPNA_DATATYPE_STRING ) ) )
    {
        printf("Failed Adding Hostname to Host Address:  0x%X\n", hr);
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
    WCHAR                   wszSession[128];


    // Prompt the user for the session name
    printf("\nPlease Enter a Session Name.\n");
    wscanf(L"%ls", wszSession);

    // Now set up the Application Description
    ZeroMemory(&dpAppDesc, sizeof(DPN_APPLICATION_DESC));
    dpAppDesc.dwSize = sizeof(DPN_APPLICATION_DESC);
    dpAppDesc.guidApplication = g_guidApp;
    dpAppDesc.pwszSessionName = wszSession;
    dpAppDesc.dwFlags = DPNSESSION_MIGRATE_HOST;

    // We are now ready to host the app
    if( FAILED( hr = g_pDP->Host( &dpAppDesc,             // AppDesc
                                  &g_pDeviceAddress, 1,   // Device Address
                                  NULL, NULL,             // Reserved
                                  NULL,                   // Player Context
                                  0 ) ) )                    // dwFlags
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
// Name: ConnectToSession()
// Desc: Connects to a DirectPlay session
//-----------------------------------------------------------------------------
HRESULT ConnectToSession()
{
    HRESULT                     hr = E_FAIL;
    DPN_APPLICATION_DESC        dpnAppDesc;
    IDirectPlay8Address*        pHostAddress = NULL;


    ZeroMemory(&dpnAppDesc, sizeof(DPN_APPLICATION_DESC));
    dpnAppDesc.dwSize = sizeof(DPN_APPLICATION_DESC);
    dpnAppDesc.guidApplication = g_guidApp;

    // Simply connect to the first one in the list
    EnterCriticalSection(&g_csHostList);

    if( g_pHostList && SUCCEEDED(hr = g_pHostList->pHostAddress->Duplicate(&pHostAddress ) ) )
    {
        hr = g_pDP->Connect(&dpnAppDesc,        // pdnAppDesc
                            pHostAddress,       // pHostAddr
                            g_pDeviceAddress,   // pDeviceInfo
                            NULL,               // pdnSecurity
                            NULL,               // pdnCredentials
                            NULL, 0,            // pvUserConnectData/Size
                            NULL,               // pvPlayerContext
                            NULL,               // pvAsyncContext
                            NULL,               // pvAsyncHandle
                            DPNCONNECT_SYNC);   // dwFlags

        if( FAILED( hr))
            printf("Failed Connecting to Host:  0x%x\n", hr);
    }
    else
    {
        printf("Failed Duplicating Host Address:  0x%x\n", hr);
    }

    LeaveCriticalSection(&g_csHostList);

    SAFE_RELEASE(pHostAddress);
    return hr;
}




//-----------------------------------------------------------------------------
// Name: SendDirectPlayMessage()
// Desc: Sends a DirectPlay message to all players
//-----------------------------------------------------------------------------
HRESULT SendDirectPlayMessage()
{
    HRESULT         hr = S_OK;
    DPN_BUFFER_DESC dpnBuffer;
    WCHAR           wszData[256];

    // Get the data from the user
    printf("\nPlease Enter a String.\n");
    wscanf(L"%ls", wszData);

    dpnBuffer.pBufferData = (BYTE*) wszData;
    dpnBuffer.dwBufferSize = 2 * (wcslen(wszData) + 1);

    if( FAILED( hr = g_pDP->SendTo( DPNID_ALL_PLAYERS_GROUP,  // dpnid
                                    &dpnBuffer,             // pBufferDesc
                                    1,                      // cBufferDesc
                                    0,                      // dwTimeOut
                                    NULL,                   // pvAsyncContext
                                    NULL,                   // pvAsyncHandle
                                    DPNSEND_SYNC |
                                    DPNSEND_NOLOOPBACK ) ) )   // dwFlags
    {
        printf("Failed Sending Data:  0x%x\n", hr);
    }
    return hr;
}




//-----------------------------------------------------------------------------
// Name: CleanupDirectPlay()
// Desc: Cleanup DirectPlay
//-----------------------------------------------------------------------------
void CleanupDirectPlay()
{
    HOST_NODE* pHostNode = NULL;
    HOST_NODE* pHostNodetmp = NULL;

    // Cleanup DirectPlay
    if( g_pDP)
        g_pDP->Close(0);

    // Clean up Host list
    EnterCriticalSection(&g_csHostList);
    
    pHostNode = g_pHostList;
    while( pHostNode != NULL )
    {       
        SAFE_RELEASE(pHostNode->pHostAddress);
        SAFE_DELETE(pHostNode->pAppDesc);
        SAFE_DELETE(pHostNode->pwszSessionName);

        pHostNodetmp = pHostNode;
        pHostNode    = pHostNode->pNext;
        SAFE_DELETE(pHostNodetmp);
    }

    LeaveCriticalSection(&g_csHostList);

    SAFE_RELEASE(g_pDeviceAddress);
    SAFE_RELEASE(g_pHostAddress);
    SAFE_RELEASE(g_pDP);

    DeleteCriticalSection(&g_csHostList);
}
    
