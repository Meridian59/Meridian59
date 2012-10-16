//-----------------------------------------------------------------------------
// File: Wizard.cpp
//
// Desc: UI routines
//
// Copyright (C) 1995-2001 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------
#include "duel.h"
#include "prsht.h"
#include "wizard.h"
#include "lobby.h"
#include "DPUtil.h"
#include "gameproc.h"


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
extern HINSTANCE     g_hInst;          // Program instance
extern HWND          g_hwndMain;       // Main window
extern LPDIRECTPLAY4 g_pDP;            // DPlay object pointer
extern BOOL          g_bHostPlayer;    // Is the user hosting/joining a game
extern DPID          g_LocalPlayerDPID;// Player id
extern LPGUID        g_pAppGUID;       // Duel's guid
extern BOOL          g_bUseProtocol;   // DPlay Protocol messaging

static BOOL          gbWaitForLobby;   // We will Wait for Settings from Lobby
static HFONT         ghTitleFont;      // Font for titles on setup wizard
static HFONT         ghHeadingFont;    // Font for headings on setup wizard
static int           gnSession;        // Index for tree view images
static int           gnPlayer;         // Index for tree view images
static HKEY          ghDuelKey = NULL; // Duel registry key handle
static DWORD         gdwDuelDisp;      // Key created or opened
static HTREEITEM     ghtiSession;      // Points to a visible session item in
                                       // tree ctl used for inserting players
static HWND          ghWndSPCtl;       // Handle to service provider control


                                       
                                       
//-----------------------------------------------------------------------------
// Name: SetupFonts()
// Desc: Initializes font structures (used for wizard controls)
//-----------------------------------------------------------------------------
BOOL SetupFonts()
{
    LOGFONT lf;
    TCHAR   strFontName[MAX_FONTNAME];
    HDC hDC = GetDC(g_hwndMain);
    if (!hDC)
        return FALSE;

    LoadString(g_hInst, IDS_WIZARD_FONTNAME, strFontName, MAX_FONTNAME);

    ZeroMemory(&lf,sizeof(lf));
    lf.lfHeight = -MulDiv(11,GetDeviceCaps(hDC, LOGPIXELSY),72);
    lf.lfWeight = 500;
    lf.lfItalic = TRUE;
    _tcscpy( lf.lfFaceName, strFontName );
    ghTitleFont = CreateFontIndirect(&lf);

    ZeroMemory(&lf,sizeof(lf));
    lf.lfHeight = -MulDiv(11,GetDeviceCaps(hDC, LOGPIXELSY),72);
    lf.lfWeight = 500;
    _tcscpy( lf.lfFaceName, strFontName );
    ghHeadingFont = CreateFontIndirect(&lf);

    ReleaseDC(g_hwndMain, hDC);

    if (!ghTitleFont || !ghHeadingFont)
    {
        CleanupFonts();
        return FALSE;
    }
    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: CleanupFonts()
// Desc: Cleans up font structures
//-----------------------------------------------------------------------------
VOID CleanupFonts()
{
    if (ghTitleFont)
        DeleteObject(ghTitleFont);
    if (ghHeadingFont)
        DeleteObject(ghHeadingFont);
    ghTitleFont = NULL;
    ghHeadingFont = NULL;
}




//-----------------------------------------------------------------------------
// Name: RegSet()
// Desc: Stores a data value in the registry
//-----------------------------------------------------------------------------
LONG RegSet( const TCHAR* strName, const BYTE* pData, DWORD dwSize )
{
#ifdef UNICODE
    dwSize *= 2; // calc number of bytes
#endif
    return RegSetValueEx( ghDuelKey, strName, 0, REG_SZ, pData, dwSize );
}




//-----------------------------------------------------------------------------
// Name: RegGet()
// Desc: Queries a value from the registry
//-----------------------------------------------------------------------------
LONG RegGet( const TCHAR* strName, BYTE* pData, DWORD* pdwDataSize )
{
    DWORD dwType;

    return RegQueryValueEx( ghDuelKey, strName, NULL, &dwType, pData,
                            pdwDataSize );
}




//-----------------------------------------------------------------------------
// Name: DoWizard()
// Desc: Creates and launches a wizard (property sheets) for user input
//-----------------------------------------------------------------------------
DWORD WINAPI DoWizard(LPVOID pv)
{
    PROPSHEETPAGE psp[4];
    PROPSHEETHEADER psh;
    TCHAR strTitle1[MAX_WINDOWTITLE];
    TCHAR strTitle2[MAX_WINDOWTITLE];
    TCHAR strTitle3[MAX_WINDOWTITLE];
    TCHAR strTitle4[MAX_WINDOWTITLE];
    TCHAR strTitle5[MAX_WINDOWTITLE];

    CoInitialize(NULL);
    
    // Setup the property pages

    LoadString(g_hInst, IDS_WIZARD_TITLE_GS, strTitle1, MAX_WINDOWTITLE); 
    LoadString(g_hInst, IDS_WIZARD_TITLE_SP, strTitle2, MAX_WINDOWTITLE); 
    LoadString(g_hInst, IDS_WIZARD_TITLE_JS, strTitle3, MAX_WINDOWTITLE); 
    LoadString(g_hInst, IDS_WIZARD_TITLE_HS, strTitle4, MAX_WINDOWTITLE); 
    LoadString(g_hInst, IDS_WIZARD_TITLE,    strTitle5, MAX_WINDOWTITLE); 

    psp[0].dwSize    = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags   = PSP_USETITLE;
    psp[0].hInstance = g_hInst;
#ifdef NONAMELESSUNION
    psp[0].u.pszTemplate = MAKEINTRESOURCE(IDD_GAMESETUP);
    psp[0].u2.pszIcon    = NULL; 
#else
    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_GAMESETUP);
    psp[0].pszIcon     = NULL; 
#endif
    psp[0].pfnDlgProc  = (DLGPROC)DlgProcGameSetup;
    psp[0].pszTitle    = strTitle1;
    psp[0].lParam      = 0;
    psp[0].pfnCallback = NULL;

    psp[1].dwSize    = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags   = PSP_USETITLE;
    psp[1].hInstance = g_hInst;
#ifdef NONAMELESSUNION
    psp[1].u.pszTemplate = MAKEINTRESOURCE(IDD_CHOOSEPROVIDER);
    psp[1].u2.pszIcon    = NULL; 
#else
    psp[1].pszTemplate = MAKEINTRESOURCE(IDD_CHOOSEPROVIDER);
    psp[1].pszIcon     = NULL; 
#endif
    psp[1].pfnDlgProc  = (DLGPROC)DlgProcChooseProvider;
    psp[1].pszTitle    = strTitle2;
    psp[1].lParam      = 0;
    psp[1].pfnCallback = NULL;

    psp[2].dwSize    = sizeof(PROPSHEETPAGE);
    psp[2].dwFlags   = PSP_USETITLE;
    psp[2].hInstance = g_hInst;
#ifdef NONAMELESSUNION
    psp[2].u.pszTemplate = MAKEINTRESOURCE(IDD_JOINSESSION);
    psp[2].u2.pszIcon    = NULL; 
#else
    psp[2].pszTemplate = MAKEINTRESOURCE(IDD_JOINSESSION);
    psp[2].pszIcon     = NULL; 
#endif
    psp[2].pfnDlgProc  = (DLGPROC)DlgProcJoinSession;
    psp[2].pszTitle    = strTitle3;
    psp[2].lParam      = 0;
    psp[2].pfnCallback = NULL;

    psp[3].dwSize    = sizeof(PROPSHEETPAGE);
    psp[3].dwFlags   = PSP_USETITLE;
    psp[3].hInstance = g_hInst;
#ifdef NONAMELESSUNION
    psp[3].u.pszTemplate = MAKEINTRESOURCE(IDD_HOSTSESSION);
    psp[3].u2.pszIcon    = NULL; 
#else
    psp[3].pszTemplate = MAKEINTRESOURCE(IDD_HOSTSESSION);
    psp[3].pszIcon     = NULL; 
#endif
    psp[3].pfnDlgProc  = (DLGPROC)DlgProcHostSession;
    psp[3].pszTitle    = strTitle4;
    psp[3].lParam      = 0;
    psp[3].pfnCallback = NULL;
    
    psh.dwSize     = sizeof(PROPSHEETHEADER);
    psh.dwFlags    = PSH_PROPSHEETPAGE | PSH_WIZARD;
    psh.hwndParent = g_hwndMain;
    psh.hInstance  = g_hInst;
#ifdef NONAMELESSUNION
    psh.u.pszIcon     = NULL;
    psh.u2.nStartPage = 0;
    psh.u3.ppsp       = (LPCPROPSHEETPAGE) &psp;
#else
    psh.pszIcon    = NULL;
    psh.nStartPage = 0;
    psh.ppsp       = (LPCPROPSHEETPAGE) &psp;
#endif
    psh.pszCaption  = strTitle5;
    psh.nPages      = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.pfnCallback = NULL;

    // open/create duel registry key 
    if( ERROR_SUCCESS != RegCreateKeyEx( HKEY_CURRENT_USER, DUEL_KEY, 0, NULL,
                                         REG_OPTION_NON_VOLATILE,
                                         KEY_ALL_ACCESS, NULL, &ghDuelKey,
                                         &gdwDuelDisp ) )
    {
        TRACE(_T("Failed to create/open registry key\n"));
        ShowError(IDS_WIN_ERROR);
        goto ABORT;
    }

    if (!SetupFonts())
    {
        TRACE(_T("Failed to create fonts\n"));
        ShowError(IDS_WIN_ERROR);
        goto ABORT;
    }

    // launch the wizard
    if (PropertySheet(&psh) < 0)
    {
        TRACE(_T("PropertySheet failed (comctl32.dll)\n"));
        ShowError(IDS_WIN_ERROR);
        goto ABORT;
    }

    CleanupFonts();
    
    // Are we going to wait for a lobby to connect to us?
    if (gbWaitForLobby)
        gbWaitForLobby = DPLobbyWait();
        // gbWaitForLobby now means we successfully got our connection settings

    // Do we have our settings or a game session?
    if( gbWaitForLobby || DPUtil_IsDPlayInitialized() )
    {
        // success
        CoUninitialize();
        PostMessage(g_hwndMain, UM_LAUNCH, (WPARAM)gbWaitForLobby, 0);
        gbWaitForLobby = FALSE;     // done with this flag
        return 0;
    }

ABORT:
    // failure
    CoUninitialize();
    PostMessage(g_hwndMain, UM_ABORT, 0, 0);
    return 1;
}




//-----------------------------------------------------------------------------
// Name: DPEnumConnectionsCallback()
// Desc: Creates SP list box entries and associates connection data with them
//-----------------------------------------------------------------------------
BOOL FAR PASCAL DPEnumConnectionsCallback( const GUID* pguidSP,
                                           VOID* pConnection, DWORD dwSize,
                                           const DPNAME* pName, DWORD dwFlags,
                                           VOID* pContext )
{
    HWND    hWnd = (HWND)pContext;
    LRESULT iIndex;
    VOID*   pConnectionBuffer;

    // Store service provider name in combo box
#ifdef UNICODE
    iIndex = SendMessage( hWnd, LB_ADDSTRING, 0, (LPARAM)pName->lpszShortName );
#else
    iIndex = SendMessage( hWnd, LB_ADDSTRING, 0, (LPARAM)pName->lpszShortNameA );
#endif

    if( iIndex == CB_ERR )
        return TRUE;

    // Make space for Connection Shortcut
    pConnectionBuffer = GlobalAllocPtr( GHND, dwSize );
    if( pConnectionBuffer == NULL )
        return TRUE;

    // Store pointer to GUID in combo box
    memcpy( pConnectionBuffer, pConnection, dwSize );

    if( iIndex != LB_ERR )
        SendMessage( hWnd, LB_SETITEMDATA, iIndex, (LPARAM)pConnectionBuffer );

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: DlgProcChooseProvider()
// Desc: Dialog procedure for the choose service provider dialog
//-----------------------------------------------------------------------------
BOOL CALLBACK DlgProcChooseProvider( HWND hDlg, UINT msg, WPARAM wParam,
                                     LPARAM lParam )
{
    VOID*        pCon = NULL;
    static  LONG iIndex;
    static  HWND hWndCtl;
    HRESULT      hr;

    switch( msg )
    {
        case WM_NOTIFY:
            switch( ((NMHDR FAR *)lParam)->code )
            {
                case PSN_SETACTIVE:
                    PropSheet_SetWizButtons( GetParent(hDlg),
                                             PSWIZB_BACK | PSWIZB_NEXT );
                    return TRUE;

                case PSN_WIZNEXT: 
                    // Release previously selected DPlay object, if any.
                    DPUtil_Release();
                    if( iIndex != LB_ERR )
                    {
                        pCon = (VOID*)SendMessage( hWndCtl, LB_GETITEMDATA,
                                                   iIndex, 0 );
                        if( pCon )
                        {
                            // Create directplay object
                            if( SUCCEEDED( DPUtil_InitDirectPlay( pCon ) ) )
                            {
                                if( g_bHostPlayer ) // Skip to host page
                                    SetWindowLong( hDlg, DWL_MSGRESULT,
                                                   IDD_HOSTSESSION );
                                return TRUE;
                            }
                            else
                            {
                                ShowError(IDS_DPLAY_ERROR_IDC);
                            }
                        }
                    }
                    SetWindowLong( hDlg, DWL_MSGRESULT, -1 );
                    return TRUE;

                case PSN_QUERYCANCEL:
                    ReleaseSPData();
                    DPUtil_Release();       // Indicates no game session
                    return TRUE;
            }
            break;

        case WM_INITDIALOG:
            SendDlgItemMessage( hDlg, IDC_SP_TITLE, WM_SETFONT,
                                (WPARAM)ghTitleFont, MAKELPARAM(TRUE,0) );

            hWndCtl = GetDlgItem( hDlg, IDC_SERVICEPROVIDERS );
            if( hWndCtl == NULL )
                return TRUE;

            // Remember the service provider control. used later in freeing
            // sp information.
            ghWndSPCtl = hWndCtl;

            if( FAILED( hr = DPUtil_InitDirectPlay( pCon ) ) )
            {
                // Class not available, so tell them to get DPlay 6 or later
                if( (hr == REGDB_E_CLASSNOTREG) || (hr == REGDB_E_IIDNOTREG) ||
                    (hr == E_NOINTERFACE))
                    ShowError(IDS_DPLAY_ERROR_CLSID);
                else
                    ShowError(IDS_DPLAY_ERROR_IDC);
                
                SetWindowLong( hDlg, DWL_MSGRESULT, -1 );
                PostQuitMessage( -1 );
                return TRUE;
            }

            //Insert all available connections into the connection listbox.
            g_pDP->EnumConnections( g_pAppGUID, DPEnumConnectionsCallback,
                                    hWndCtl, 0 );
            SetFocus( hWndCtl );

            SendMessage( hWndCtl, LB_SETCURSEL, iIndex, 0 );
            iIndex = SendMessage( hWndCtl, LB_GETCURSEL, 0, 0 );
            return TRUE;

        case WM_COMMAND:
            if( HIWORD(wParam) == LBN_SELCHANGE )
            {
                iIndex = SendMessage( hWndCtl, LB_GETCURSEL, 0, 0 );
                return TRUE;
            }
            break;
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: DlgProcGameSetup()
// Desc: Dialog procedure for the Game Setup Dialog.
//-----------------------------------------------------------------------------
BOOL CALLBACK DlgProcGameSetup (HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static BOOL bWaitForLobby;
    switch (msg)
    {
    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code) 
        {
            case PSN_SETACTIVE:
                if (bWaitForLobby)
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_FINISH);
                else    
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
                return(TRUE);
            
            case PSN_WIZNEXT: 
                if (g_bHostPlayer)
                    g_bUseProtocol = (SendDlgItemMessage(hDlg, IDC_USEPROTOCOL,
                                                        BM_GETCHECK, 0, 0)
                                     == BST_CHECKED);
                else
                    g_bUseProtocol = FALSE;
                return(TRUE);

            case PSN_WIZFINISH:     // only if WaitForLobby selected 
                gbWaitForLobby = bWaitForLobby;
                // fall thru
            case PSN_QUERYCANCEL:
                ReleaseSPData();
                DPUtil_Release();       // Indicates no game session
                return(TRUE);
        }
        break;

    case WM_INITDIALOG:
        // setup title fonts
        SendDlgItemMessage(hDlg, IDC_GAMESETUP_TITLE, WM_SETFONT, (WPARAM)ghTitleFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(hDlg, IDC_JOINSESSION, WM_SETFONT, (WPARAM)ghHeadingFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(hDlg, IDC_HOSTSESSION, WM_SETFONT, (WPARAM)ghHeadingFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(hDlg, IDC_WAITFORLOBBY, WM_SETFONT, (WPARAM)ghHeadingFont, MAKELPARAM(TRUE,0));
        SendDlgItemMessage(hDlg, IDC_USEPROTOCOL, WM_SETFONT, (WPARAM)ghHeadingFont, MAKELPARAM(TRUE,0));
        // host by default
        g_bHostPlayer = TRUE;
        gbWaitForLobby = bWaitForLobby = FALSE;
        SendDlgItemMessage(hDlg, IDC_HOSTSESSION, BM_SETCHECK, 1, 0);
        SendDlgItemMessage(hDlg, IDC_JOINSESSION, BM_SETCHECK, 0, 0);
        SendDlgItemMessage(hDlg, IDC_WAITFORLOBBY, BM_SETCHECK, 0, 0);
        SendDlgItemMessage(hDlg, IDC_USEPROTOCOL, BM_SETCHECK, g_bUseProtocol, 0);
        return(TRUE);

    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED)
            switch (LOWORD(wParam))
            {
                // Enable Use Protocol check box only if Hosting is selected
                // Enable Finish Button only if WaitForLobby is selected
            case IDC_HOSTSESSION:
                g_bHostPlayer = TRUE;
                EnableWindow(GetDlgItem(hDlg, IDC_USEPROTOCOL), TRUE);
                if (bWaitForLobby)
                {
                    bWaitForLobby = FALSE;
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
                }
                return(TRUE);

            case IDC_JOINSESSION:
                if (g_bHostPlayer)
                {
                    g_bHostPlayer = FALSE;
                    EnableWindow(GetDlgItem(hDlg, IDC_USEPROTOCOL), FALSE);
                }
                if (bWaitForLobby)
                {
                    bWaitForLobby = FALSE;
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
                }
                return(TRUE);

            case IDC_WAITFORLOBBY:
                bWaitForLobby = TRUE;
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_FINISH);
                if (g_bHostPlayer)
                {
                    g_bHostPlayer = FALSE;
                    EnableWindow(GetDlgItem(hDlg, IDC_USEPROTOCOL), FALSE);
                }
                return(TRUE);
            }
        break;
    }
    return(FALSE);
}




//-----------------------------------------------------------------------------
// Name: EnumPlayer()
// Desc: EnumeratePlayer callback. Inserts player information into the passed
//       in tree view control.
//-----------------------------------------------------------------------------
BOOL WINAPI EnumPlayer( DPID pidID, DWORD dwPlayerType, const DPNAME* pName,
                        DWORD dwFlags, VOID* pContext )
{
    HWND      hWnd = (HWND)pContext;
    HTREEITEM hItem;

#ifdef UNICODE
    hItem = AddItemToTree( hWnd, pName->lpszShortName, 0, -1 );
#else
    hItem = AddItemToTree( hWnd, pName->lpszShortNameA, 0, -1 );
#endif

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: EnumSession()
// Desc: EnumSessions callback. Inserts session description information in the
//       passed in tree view control.
//-----------------------------------------------------------------------------
BOOL WINAPI EnumSession( const DPSESSIONDESC2* pDPSessionDesc,
                         DWORD* pdwTimeOut, DWORD dwFlags, VOID* pContext )
{
    HWND      hWnd = (HWND)pContext;
    HTREEITEM hItem;
    TREEDATA* pdata;

    if( dwFlags & DPESC_TIMEDOUT )
        return FALSE;       // don't try again

    if( hWnd == NULL )
        return FALSE;

    // allocate memory to remember the guid
    pdata = (TREEDATA*)malloc(sizeof(TREEDATA));
    if( NULL == pdata )
        return FALSE;

    pdata->guid      = pDPSessionDesc->guidInstance;
    pdata->dwRefresh = 1;
    
#ifdef UNICODE
    hItem = AddItemToTree( hWnd, pDPSessionDesc->lpszSessionName,
                           (DWORD)pdata, 1);
#else
    hItem = AddItemToTree( hWnd, pDPSessionDesc->lpszSessionNameA,
                           (DWORD)pdata, 1);
#endif

    if( hItem )
    {
        // It was new
        TreeView_SelectItem( hWnd, hItem );
        // Enumerate players for new session
        DPUtil_EnumPlayers( (GUID*)&pDPSessionDesc->guidInstance, EnumPlayer,
                          (VOID*)hWnd, DPENUMPLAYERS_SESSION );
    }
    else 
    {
        // It was already in the tree
        free( pdata );
    }

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: ResetTreeRefresh()
// Desc:
//-----------------------------------------------------------------------------
VOID ResetTreeRefresh( HWND hwndTV )
{
    TV_ITEM   item;
    HTREEITEM hItem;
    TREEDATA* psearchdata;

    memset( &item, 0, sizeof(item) );
    
    // Get the first app item (child of the running apps item)
    hItem = (HTREEITEM)SendMessage( hwndTV, TVM_GETNEXTITEM, TVGN_ROOT,(LPARAM) NULL);    
    
    while (hItem)
    {
        // gets its data
        item.mask = TVIF_PARAM;
        item.hItem = hItem;
        SendMessage( hwndTV, TVM_GETITEM, 0, (LPARAM)&item );
        // set the dwRefresh to 0
        psearchdata = (TREEDATA*)item.lParam;
        if (psearchdata) psearchdata->dwRefresh = 0;
        SendMessage( hwndTV, TVM_SETITEM, 0, (LPARAM)&item );

        // get the next one
        hItem = (HTREEITEM)SendMessage( hwndTV, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem );    
    }    
    
    return ;
}




//-----------------------------------------------------------------------------
// Name: RemoveStaleSessions()
// Desc: 
//-----------------------------------------------------------------------------
VOID RemoveStaleSessions( HWND hwndTV )
{
    TV_ITEM   item;
    HTREEITEM hItem,hItemNext;
    TREEDATA* psearchdata;

    memset(&item,0,sizeof(item));
    
    // get the first app item (child of the running apps item)
    hItem = (HTREEITEM)SendMessage( hwndTV, TVM_GETNEXTITEM, TVGN_ROOT,(LPARAM) NULL);    
    
    while (hItem)
    {
        // get the next one
        hItemNext = (HTREEITEM)SendMessage( hwndTV, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem );    
        
        // gets its data
        item.mask = TVIF_PARAM;
        item.hItem = hItem;
        SendMessage( hwndTV, TVM_GETITEM, 0, (LPARAM)&item );

        // if dwRefresh is still 0, it wasn't enum'ed
        psearchdata = (TREEDATA*)item.lParam;
        if (psearchdata && (0 == psearchdata->dwRefresh))
        {
            free(psearchdata);
            SendMessage( hwndTV, TVM_DELETEITEM, 0, (LPARAM)hItem );
        }

        hItem = hItemNext;
    }    
    
    return;
}




//-----------------------------------------------------------------------------
// Name: DlgProcJoinSession()
// Desc: Dialog procedure for Join Session Dialog
//-----------------------------------------------------------------------------
BOOL CALLBACK DlgProcJoinSession( HWND hDlg, UINT msg, WPARAM wParam,
                                  LPARAM lParam )
{
    static NM_TREEVIEW nmtv;
    static HWND        hWndCtl;
    static TCHAR       strPlayerName[MAX_PLAYERNAME+1];
    static HANDLE      dphEvent = NULL;
    static UINT        idTimer = 0;

    TV_ITEM   tvItem;
    TCHAR     strSessionName[MAX_SESSIONNAME+1];
    DWORD     dwPNameSize;
    HRESULT   hr = DP_OK;
    HTREEITEM htiCur, htiNext;
    GUID*     pGuid;

    switch( msg )
    {
        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code) 
            {
                case PSN_SETACTIVE:
					// Only the back button is valid at this point
					PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK);

                    if( hWndCtl )
                        TreeView_DeleteAllItems( hWndCtl );

                    // Enum sessions and let dplay decide the timeout
                    hr = DPUtil_EnumSessions( 0, EnumSession, (VOID*)hWndCtl,
                                            DPENUMSESSIONS_ASYNC );
                    if( FAILED(hr) )
                        return FALSE;

                    // Enumerate players for all sessions
                    ghtiSession = TreeView_GetFirstVisible( hWndCtl );
        
                    while( ghtiSession )
                    {
                        // delete previous players from display
                        if ((htiNext = htiCur = TreeView_GetChild(hWndCtl, ghtiSession)) 
                            != (HTREEITEM)0)
                        {
                            do
                            {
                                htiNext = TreeView_GetNextSibling(hWndCtl, htiCur);
                                TreeView_DeleteItem(hWndCtl, htiCur);
                                htiCur = htiNext;
                            } while (htiNext);
                        }

                        tvItem.hItem = ghtiSession;
                        tvItem.pszText = strSessionName;
                        tvItem.cchTextMax = MAX_SESSIONNAME;
                        TreeView_GetItem(hWndCtl, &tvItem);
                        // enumerate players for selected session
                        DPUtil_EnumPlayers( (GUID*)tvItem.lParam, EnumPlayer, (LPVOID) hWndCtl, DPENUMPLAYERS_SESSION);
                        ghtiSession = TreeView_GetNextItem(hWndCtl, ghtiSession, TVGN_NEXTVISIBLE);
                    }

                    // set Finish button highlite
                    if (GetDlgItemText(hDlg, IDC_JOIN_PLAYERNAME, strPlayerName, MAX_PLAYERNAME) && 
                        (htiCur = TreeView_GetSelection(hWndCtl)))
                    {


                        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_FINISH);
                    }
                    else
                    {
                        PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_BACK|PSWIZB_DISABLEDFINISH);
                    }

                    SetFocus(hWndCtl);
                    
                    // start up timer
                    idTimer = SetTimer(hDlg, ENUM_TIMER_ID, ENUM_TIMEOUT, NULL);
            
                    return(TRUE);

                case PSN_WIZFINISH:
                    if (idTimer)
                    {
                        KillTimer(hDlg, idTimer);
                        idTimer = 0;
                    }
                                
                    // add user selections to registry
                    if (ghDuelKey) 
                    {
                        RegSet(TEXT("PlayerName"), (CONST BYTE *)strPlayerName, 
                                sizeof(strPlayerName));
                    }
        
                    // get the session guid
                    if (nmtv.itemNew.lParam)
                    {
                        // user selected a session item, so just grab its lParam
                        pGuid = (GUID*)nmtv.itemNew.lParam;
                    }
                    else
                    {
                        // user selected a player item, so grab its parent's (session) lParam
                        htiCur = TreeView_GetParent(hWndCtl, nmtv.itemNew.hItem);
                        if (!htiCur)
                        {
                            // fail finish
                            ShowError(IDS_WIZARD_ERROR_GSG);
                            SetWindowLong(hDlg, DWL_MSGRESULT, -1);
                            return (TRUE);
                        }

                        tvItem.hItem = htiCur;
                        tvItem.pszText = strSessionName;
                        tvItem.cchTextMax = MAX_SESSIONNAME;
                        TreeView_GetItem(hWndCtl, &tvItem);                 
                        pGuid = (GUID*)tvItem.lParam;
                    }

                    // Open session
                    if( FAILED( hr = DPUtil_OpenSession( pGuid ) ) )
                    {
                        // Fail finish
                        ShowError(IDS_DPLAY_ERROR_JS);
                        SetWindowLong(hDlg, DWL_MSGRESULT, -1);
                        return TRUE;
                    }

                    // create player
                    if( FAILED( hr = DPUtil_CreatePlayer( &g_LocalPlayerDPID, strPlayerName,
                                                         NULL, NULL, 0 ) ) )
                    {
                        // fail finish
                        ShowError(IDS_DPLAY_ERROR_CP);
                        SetWindowLong(hDlg, DWL_MSGRESULT, -1);
                        return (TRUE);
                    }

                    // everything went well, release allocated memory and finish
                    ReleaseSessionData(hWndCtl);
                    ReleaseSPData();                
                    return(TRUE);

                case PSN_QUERYCANCEL:
                    if (idTimer)
                    {
                        KillTimer(hDlg, idTimer);
                        idTimer = 0;
                    }
                    DPUtil_EnumSessions(0, EnumSession, (LPVOID)hWndCtl,
                                      DPENUMSESSIONS_STOPASYNC);
                    ReleaseSessionData(hWndCtl);
                    ReleaseSPData();
                    DPUtil_Release();       // Indicates no game session
                    return(TRUE);

                case PSN_WIZBACK:
                    if (idTimer)
                    {
                        KillTimer(hDlg, idTimer);
                        idTimer = 0;
                    }
                    DPUtil_EnumSessions(0, EnumSession, (LPVOID)hWndCtl,
                                      DPENUMSESSIONS_STOPASYNC);
                    ReleaseSessionData(hWndCtl);
                    return(TRUE);

                case TVN_SELCHANGING:
                    nmtv = *((NM_TREEVIEW *) lParam);

                    // set Finish button status 
                    if (GetDlgItemText(hDlg, IDC_JOIN_PLAYERNAME, strPlayerName, MAX_PLAYERNAME) && 
                        (htiCur = TreeView_GetSelection(hWndCtl)))
                    {
                        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_FINISH);
                    }
                    else
                    {
                        PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_BACK|PSWIZB_DISABLEDFINISH);
                    }
                    return(FALSE);

                case NM_CLICK:
                    return(FALSE);
            }
            break;
        case WM_INITDIALOG:
            // setup title fonts
            SendDlgItemMessage(hDlg, IDC_JOIN_SESSION_TITLE, WM_SETFONT, (WPARAM)ghHeadingFont, MAKELPARAM(TRUE,0));
            SendDlgItemMessage(hDlg, IDC_JOIN_PLAYER_TITLE, WM_SETFONT, (WPARAM)ghHeadingFont, MAKELPARAM(TRUE,0));

            // setup user's previous data
            dwPNameSize = MAX_PLAYERNAME+1;
            strPlayerName[0]=0;
            if (ghDuelKey && (RegGet(TEXT("PlayerName"),(LPBYTE)strPlayerName,&dwPNameSize) == ERROR_SUCCESS))  
                SetDlgItemText(hDlg, IDC_JOIN_PLAYERNAME, strPlayerName);

            hWndCtl = GetDlgItem(hDlg, IDC_JOIN_SESSION);
            if (hWndCtl == NULL) return(TRUE);
            InitTreeViewImageLists(hWndCtl);
            return(TRUE);
            
        case WM_TIMER:

            // guard against leftover timer messages after timer has been killed
            if (!idTimer) break;
            
            ResetTreeRefresh(hWndCtl);
            // enum sessions and let dplay decide the timeout
            hr = DPUtil_EnumSessions(0, EnumSession, (LPVOID) hWndCtl,
                                   DPENUMSESSIONS_ASYNC);
            if (FAILED(hr) && hr != DPERR_CONNECTING)
            {
                KillTimer(hDlg, idTimer);
                idTimer = 0;
                if (hr != DPERR_USERCANCEL)
                    ShowError(IDS_DPLAY_ERROR_ES);
            }

            RemoveStaleSessions(hWndCtl);
            
                // set Finish button highlite
            if (GetDlgItemText(hDlg, IDC_JOIN_PLAYERNAME, strPlayerName, MAX_PLAYERNAME) && 
                (htiCur = TreeView_GetSelection(hWndCtl)))
            {
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_FINISH);
            }
            else
            {
                PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_BACK|PSWIZB_DISABLEDFINISH);
            }
            
            break;
            
        case WM_COMMAND:

            switch (LOWORD(wParam))
            {
                case IDC_JOIN_PLAYERNAME:
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        // set Finish button status 
                        if (GetDlgItemText(hDlg, IDC_JOIN_PLAYERNAME, strPlayerName, MAX_PLAYERNAME) && 
                            (htiCur = TreeView_GetSelection(hWndCtl)))
                        {
                            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_FINISH);
                        }
                        else
                        {
                            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_DISABLEDFINISH);
                        }
                    }
                    break;
            }
            break;
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: DlgProcHostSession()
// Desc: Dialog proc for Host Session Dialog
//-----------------------------------------------------------------------------
BOOL CALLBACK DlgProcHostSession(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static TCHAR strSessionName[MAX_SESSIONNAME+1];
    static TCHAR strPlayerName[MAX_PLAYERNAME+1];
    HRESULT hr;
    DWORD   dwPNameSize, dwSNameSize;

    switch( msg )
    {
        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code) 
            {
                case PSN_SETACTIVE:
                    if (GetDlgItemText(hDlg, IDC_HOST_SESSIONNAME, strSessionName, MAX_SESSIONNAME) && 
                        GetDlgItemText(hDlg, IDC_HOST_PLAYERNAME, strPlayerName, MAX_PLAYERNAME))
                    {
                        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_FINISH);
                    }
                    else
                    {
                        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_DISABLEDFINISH);
                    }
                    return(TRUE);
        
                case PSN_WIZFINISH:
                    // add user selections to registry
                    if (ghDuelKey) 
                    {
                        RegSet(TEXT("PlayerName"), (CONST BYTE *)strPlayerName, sizeof(strPlayerName));
                        RegSet(TEXT("SessionName"), (CONST BYTE *)strSessionName, sizeof(strSessionName));
                    }

                    // create session
                    if ((hr = DPUtil_CreateSession(strSessionName)) != DP_OK)
                    {
                        // fail finish
                        ShowError(IDS_DPLAY_ERROR_CS);
                        SetWindowLong(hDlg, DWL_MSGRESULT, -1);
                        return (TRUE);
                    }

                    // create player
                    if( FAILED( hr = DPUtil_CreatePlayer( &g_LocalPlayerDPID, strPlayerName,
                                                          NULL, NULL, 0 ) ) )
                    {
                        ShowError(IDS_DPLAY_ERROR_CP);
                        SetWindowLong(hDlg, DWL_MSGRESULT, -1);
                        return (TRUE);
                    }

                    // everything went well, release allocated memory and finish
                    ReleaseSPData();
                    return(TRUE);
                
                case PSN_WIZBACK:
                    SetWindowLong(hDlg, DWL_MSGRESULT, IDD_CHOOSEPROVIDER);
                    return(TRUE);

                case PSN_QUERYCANCEL:
                    // release allocated memory
                    ReleaseSPData();
                    // release dplay
                    DPUtil_Release();       // Indicates no game session
                    return(TRUE);
            }
            break;
        case WM_INITDIALOG:
            // setup title font
            SendDlgItemMessage(hDlg, IDC_HOST_TITLE, WM_SETFONT, (WPARAM)ghTitleFont, MAKELPARAM(TRUE,0));
            SendDlgItemMessage(hDlg, IDC_HOST_SESSION_TITLE, WM_SETFONT, (WPARAM)ghHeadingFont, MAKELPARAM(TRUE,0));
            SendDlgItemMessage(hDlg, IDC_HOST_PLAYER_TITLE, WM_SETFONT, (WPARAM)ghHeadingFont, MAKELPARAM(TRUE,0));

            dwPNameSize = MAX_PLAYERNAME+1;
            dwSNameSize = MAX_SESSIONNAME+1;
            strPlayerName[0]=0;
            strSessionName[0]=0;
            if (ghDuelKey)
            {
                if (RegGet(TEXT("PlayerName"), (LPBYTE)strPlayerName, &dwPNameSize) == ERROR_SUCCESS)   
                    SetDlgItemText(hDlg, IDC_HOST_PLAYERNAME, strPlayerName);
                if (RegGet(TEXT("SessionName"), (LPBYTE)strSessionName, &dwSNameSize) == ERROR_SUCCESS) 
                    SetDlgItemText(hDlg, IDC_HOST_SESSIONNAME, strSessionName);
            }
            
            return(TRUE);

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case IDC_HOST_SESSIONNAME:
                if (HIWORD(wParam) == EN_CHANGE)
                {
                    if (GetDlgItemText(hDlg, IDC_HOST_SESSIONNAME, strSessionName, MAX_SESSIONNAME) && 
                        GetDlgItemText(hDlg, IDC_HOST_PLAYERNAME, strPlayerName, MAX_PLAYERNAME))
                    {
                        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_FINISH);
                    }
                    else
                    {
                        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_DISABLEDFINISH);
                    }

                    return TRUE;
                }
                break;

            case IDC_HOST_PLAYERNAME:
                if (HIWORD(wParam) == EN_CHANGE)
                {
                    if (GetDlgItemText(hDlg, IDC_HOST_SESSIONNAME, strSessionName, MAX_SESSIONNAME) && 
                        GetDlgItemText(hDlg, IDC_HOST_PLAYERNAME, strPlayerName, MAX_PLAYERNAME))
                    {
                        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_FINISH);
                    }
                    else
                    {
                        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_DISABLEDFINISH);
                    }
                }
                break;
            }
            break;
    }

    return FALSE;
}




//-----------------------------------------------------------------------------
// Name: InitTreeViewImageLists()
// Desc: Creates an image list, adds bitmaps to  it, and associates the image
//       list with a tree-view control. 
//-----------------------------------------------------------------------------
BOOL InitTreeViewImageLists(HWND hwndTV) 
{ 
    HIMAGELIST himl;  // handle of image list 
    HBITMAP hbmp;     // handle of bitmap 
 
    // Create the image list. 
    if ((himl = ImageList_Create(CX_BITMAP, CY_BITMAP, 
            FALSE, NUM_BITMAPS, 0)) == NULL) 
        return FALSE; 
 
    // Add the session and player bitmaps. 
    hbmp = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_CLOSED_SESSION)); 
    gnSession = ImageList_Add(himl, hbmp, (HBITMAP) NULL); 
    DeleteObject(hbmp); 
 
    hbmp = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_PLAYER)); 
    gnPlayer = ImageList_Add(himl, hbmp, (HBITMAP) NULL); 
    DeleteObject(hbmp); 
 
    // Fail if not all of the images were added. 
    if (ImageList_GetImageCount(himl) < 2) 
        return FALSE; 
 
    // Associate the image list with the tree-view control. 
    TreeView_SetImageList(hwndTV, himl, TVSIL_NORMAL); 
 
    return TRUE; 
} 
   



//-----------------------------------------------------------------------------
// Name: FindItemInTree()
// Desc:
//-----------------------------------------------------------------------------
HTREEITEM FindItemInTree( HWND hwndTV, TREEDATA* pdata )
{
    TV_ITEM   item;
    HTREEITEM hItem;
    TREEDATA* psearchdata;
       
    memset(&item,0,sizeof(item));
    
    // get the first app item (child of the running apps item)
    hItem = (HTREEITEM)SendMessage( hwndTV, TVM_GETNEXTITEM, TVGN_ROOT,(LPARAM) NULL);    
    
    while (hItem)
    {
        // gets its data
        item.mask = TVIF_PARAM;
        item.hItem = hItem;
        SendMessage( hwndTV, TVM_GETITEM, 0, (LPARAM)&item );
        // is this the one?
        psearchdata = (TREEDATA*)item.lParam;
        if( IsEqualGUID( psearchdata->guid, pdata->guid ) ) 
        {
            // mark the item as seen
            psearchdata->dwRefresh = 1;
            SendMessage( hwndTV, TVM_SETITEM, 0, (LPARAM)&item );
            return hItem;
        }
        // try the next one
        hItem = (HTREEITEM)SendMessage( hwndTV, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hItem );    
    }    
    
    return NULL;
}




//-----------------------------------------------------------------------------
// Name: AddItemToTree()
// Desc: Adds items to a tree-view control. 
//-----------------------------------------------------------------------------
HTREEITEM AddItemToTree( HWND hwndTV, TCHAR* strItem, DWORD dwData,
                         int nLevel ) 
{ 
    static HTREEITEM hPrev         = (HTREEITEM) TVI_FIRST; 
    static HTREEITEM hPrevRootItem = NULL; 
    static HTREEITEM hPrevLev2Item = NULL; 
    HTREEITEM        hItem;
    TV_ITEM          tvi; 
    TV_INSERTSTRUCT  tvins; 
    
    if( 1 == nLevel )
    {
        hItem = FindItemInTree( hwndTV, (TREEDATA*)dwData );
        if (hItem) return NULL;
    }
    memset( &tvi,0,sizeof(tvi) );
    memset( &tvins,0,sizeof(tvins) );

    tvi.mask = TVIF_TEXT | TVIF_IMAGE 
        | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    
    // Set the state
    if (nLevel == 1) 
    {
        tvi.mask |= TVIF_STATE; 
        tvi.state = TVIS_SELECTED;
    }

    // Set the text of the item. 
    tvi.pszText = strItem; 
 
   // Set the image
    if (nLevel == 1)
    {
        tvi.iImage = gnSession; 
        tvi.iSelectedImage = gnSession; 
    }
    else
    {
        tvi.iImage = gnPlayer; 
        tvi.iSelectedImage = gnPlayer; 
    }
 

    // Save the heading level in the item's application-defined 
    // data area. 
    tvi.lParam = (LPARAM) dwData; 
 
#ifdef NONAMELESSUNION
    tvins.u.item = tvi; 
#else
    tvins.item = tvi; 
#endif
    tvins.hInsertAfter = hPrev; 
 
    // Set the parent item based on the specified level. 
    if (nLevel == -1)
        tvins.hParent = hPrevRootItem;
    else if (nLevel == 1) 
        tvins.hParent = TVI_ROOT; 
    else if (nLevel == 2) 
        tvins.hParent = hPrevRootItem; 
    else
        tvins.hParent = hPrevLev2Item; 
 
    // Add the item to the tree-view control. 
    hPrev = (HTREEITEM) SendMessage(hwndTV, TVM_INSERTITEM, 0, 
         (LPARAM) (LPTV_INSERTSTRUCT) &tvins); 
 
    // Save the handle of the item. 
    if (nLevel == 1) 
        hPrevRootItem = hPrev; 
    else if (nLevel == 2) 
        hPrevLev2Item = hPrev; 
 
    return hPrev; 
} 
 



//-----------------------------------------------------------------------------
// Name: ReleaseSessionData()
// Desc: 
//-----------------------------------------------------------------------------
VOID ReleaseSessionData(HWND hWndCtl)
{
    HTREEITEM htiSession = TreeView_GetRoot( hWndCtl );
    
    while( htiSession )
    {
        TCHAR   strSessionName[MAX_SESSIONNAME+1];
        TV_ITEM tvItem;
        tvItem.hItem      = htiSession;
        tvItem.pszText    = strSessionName;
        tvItem.cchTextMax = MAX_SESSIONNAME;

        TreeView_GetItem( hWndCtl, &tvItem );
        if( tvItem.lParam )
            free( (VOID*)tvItem.lParam );
        
        htiSession = TreeView_GetNextSibling( hWndCtl, htiSession );
    }
}




//-----------------------------------------------------------------------------
// Name: ReleaseSPData()
// Desc: Releases the memory allocated for service provider guids
//       depends on global variable ghWndSPControl
//-----------------------------------------------------------------------------
VOID ReleaseSPData()
{
    LPVOID lpCon = NULL;
    int count,index;

    if (ghWndSPCtl)
    {
        count = SendMessage(ghWndSPCtl, LB_GETCOUNT, 0, 0);
        for (index = 0; index < count; index++)
        {
            lpCon = (LPVOID) SendMessage(ghWndSPCtl, LB_GETITEMDATA, index, 0);
            if (lpCon) GlobalFreePtr(lpCon);
        }
    }
}



