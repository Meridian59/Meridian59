//----------------------------------------------------------------------------
// File: client.cpp
//
// Desc: This simple program builds upon the 5th tutorial, and is the client 
//       code for the server in the client/server model
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
IDirectPlay8Client*                 g_pDPClient         = NULL;
IDirectPlay8Address*                g_pDeviceAddress    = NULL;
IDirectPlay8Address*                g_pHostAddress      = NULL;
BOOL                                g_bHost;
HOST_NODE*                          g_pHostList         = NULL;
CRITICAL_SECTION                    g_csHostList;


// This GUID allows DirectPlay to find other instances of the same game on
// the network.  So it must be unique for every game, and the same for 
// every instance of that game.  // {1AD4CA3B-AC68-4d9b-9522-BE59CD485276}
GUID g_guidApp = { 0x1ad4ca3b, 0xac68, 0x4d9b, { 0x95, 0x22, 0xbe, 0x59, 0xcd, 0x48, 0x52, 0x76 } };


//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
HRESULT WINAPI DirectPlayMessageHandler(PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer);
BOOL    IsServiceProviderValid(const GUID* pGuidSP);
HRESULT InitDirectPlay();
HRESULT CreateDeviceAddress();
HRESULT CreateHostAddress(WCHAR* pwszHost);
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

    if( FAILED( hr = CreateDeviceAddress() ) )
    {
        printf("Failed CreatingDeviceAddress:  0x%X\n", hr);
        goto LCleanup;
    }

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

    // Present User with Choices
    do
    {
        printf("Please select one.\n1.  Exit\n2.  Send Data\n");
        scanf("%d", &iUserChoice);

        if( iUserChoice == USER_SEND )
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

    // ShutDown COM
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

    // Create the IDirectPlay8Client Object
    if( FAILED( hr = CoCreateInstance(CLSID_DirectPlay8Client, NULL, 
                                    CLSCTX_INPROC_SERVER,
                                    IID_IDirectPlay8Client, 
                                    (LPVOID*) &g_pDPClient ) ) )
    {
        printf("Failed Creating the IDirectPlay8Client Object:  0x%X\n", hr);
        goto LCleanup;
    }

    // Init DirectPlay
    if( FAILED( hr = g_pDPClient->Initialize(NULL, DirectPlayMessageHandler, 0 ) ) )
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

    hr = g_pDPClient->EnumServiceProviders(&CLSID_DP8SP_TCPIP, NULL, NULL, &dwSize, &dwItems, 0);

    if( hr != DPNERR_BUFFERTOOSMALL)
    {
        printf("Failed Enumerating Service Providers:  0x%x\n", hr);
        goto LCleanup;
    }

    pdnSPInfo = (DPN_SERVICE_PROVIDER_INFO*) new BYTE[dwSize];

    if( FAILED( hr = g_pDPClient->EnumServiceProviders(&CLSID_DP8SP_TCPIP, NULL, pdnSPInfo, &dwSize, &dwItems, 0 ) ) )
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
HRESULT WINAPI DirectPlayMessageHandler(PVOID pvUserContext, DWORD dwMessageId, PVOID pMsgBuffer)
{
    HRESULT     hr = S_OK;

    switch (dwMessageId)
    {
        case DPN_MSGID_ENUM_HOSTS_RESPONSE:
        {
            PDPNMSG_ENUM_HOSTS_RESPONSE     pEnumHostsResponseMsg;
            const DPN_APPLICATION_DESC*     pAppDesc;
            HOST_NODE*                      pHostNode = NULL;
            WCHAR*                          pwszSession = NULL;

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
    
        case DPN_MSGID_TERMINATE_SESSION:
        {
            PDPNMSG_TERMINATE_SESSION   pTermSessionMsg;

            pTermSessionMsg = (PDPNMSG_TERMINATE_SESSION) pMsgBuffer;

            printf("\nThe Session has been terminated!\n");
            break;
        }
    
        case DPN_MSGID_RECEIVE:
        {
            PDPNMSG_RECEIVE     pMsg;

            pMsg = (PDPNMSG_RECEIVE) pMsgBuffer;

            printf("\nReceived Message:  %S\n", (WCHAR*)pMsg->pReceiveData);
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
    if( FAILED( hr = g_pDPClient->EnumHosts(&dpAppDesc,          // pApplicationDesc
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
        hr = g_pDPClient->Connect(&dpnAppDesc,  // pdnAppDesc
                            pHostAddress,       // pHostAddr
                            g_pDeviceAddress,   // pDeviceInfo
                            NULL,               // pdnSecurity
                            NULL,               // pdnCredentials
                            NULL, 0,            // pvUserConnectData/Size
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

    if( FAILED( hr = g_pDPClient->Send(&dpnBuffer,             // pBufferDesc
                                    1,                         // cBufferDesc
                                    0,                         // dwTimeOut
                                    NULL,                      // pvAsyncContext
                                    NULL,                      // pvAsyncHandle
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
    HOST_NODE*                  pHostNode = NULL;
    HOST_NODE*                  pHostNodetmp = NULL;

    // Shutdown DirectPlay
    if( g_pDPClient)
        g_pDPClient->Close(0);

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
    SAFE_RELEASE(g_pDPClient);

    DeleteCriticalSection(&g_csHostList);
}







