//------------------------------------------------------------------------------
// File: Bdasampl.cpp
//
// Desc: Sample code implementing BDA graph building.
//
// Copyright (c) 2000-2001, Microsoft Corporation. All rights reserved.
//------------------------------------------------------------------------------

#include "bdasampl.h"
#include "graph.h"
#include "resource.h"

#include <initguid.h>
#include <objbase.h>


// Globals
HWND                hwndMain;
HWND                g_hwndDlg;
HINSTANCE           hInst;
TCHAR               szAppName[]  = TEXT("BDASampl");
TCHAR               szAppTitle[] = TEXT("BDA Sample");

CBDAFilterGraph*    g_pfg = NULL;

const int MAJOR_CHANNEL_LOWER_LIMIT = -1;
const int MAJOR_CHANNEL_UPPER_LIMIT = 126;
const int MINOR_CHANNEL_LOWER_LIMIT = -1;
const int MINOR_CHANNEL_UPPER_LIMIT = 126;

INT WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine,
    INT nCmdShow)
{
    MSG         msg;
    HWND        hwnd;
    WNDCLASS    wndclass;
    HACCEL      hAccel;

    hInst   = hInstance;

    //initialize COM library
    HRESULT hr = CoInitializeEx (NULL, COINIT_MULTITHREADED);
    if (FAILED (hr))
    {
        MessageBox(
            NULL, 
            TEXT("Failed to load COM library!"),
            TEXT("Initialization Error"), 
            MB_ICONEXCLAMATION
            );
        return 0;
    }

    wndclass.style         = 0; //CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc   = WndProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = hInst;
    wndclass.hIcon         = LoadIcon(hInst, TEXT("BDASAMPLICON"));
    wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
    wndclass.lpszMenuName  = szAppName;
    wndclass.lpszClassName = szAppName;
    RegisterClass(&wndclass);

    hwnd = CreateWindow(szAppName, szAppTitle, WS_OVERLAPPEDWINDOW | 
                WS_CLIPCHILDREN, 200, 200, 500, 280, NULL, NULL, hInst, NULL);
    ASSERT(hwnd);

    // Create the BDA filter graph and initialize its components
    g_pfg = new CBDAFilterGraph();
    ASSERT(g_pfg);

    // If the graph failed to build, don't go any further.
    if (!g_pfg)
    {
        MessageBox(hwnd, TEXT("Failed to create the filter graph!"),
                   TEXT("Initialization Error"), MB_ICONEXCLAMATION);
        return 0;
    }
    
    ShowWindow(hwnd, nCmdShow);
    hwndMain = hwnd;

    hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(ACC_GRAPH));

    while(GetMessage(&msg, NULL, 0, 0) > 0)
    {
        if(!TranslateAccelerator(hwnd, hAccel, &msg))
        {
            //if (!IsDialogMessage (hwnd, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    // Release the BDA components and clean up
    delete g_pfg;

    CoUninitialize ();
    
    return msg.wParam;
}


// WndProc                                                                    
LRESULT CALLBACK
WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_CREATE:
        {
            DbgInitialise (hInst);
            break
            ;
        }
    case WM_SIZE:
        {
            if(g_pfg->m_fGraphBuilt)
                g_pfg->SetVideoWindow(hwndMain);
            break;
        }
        
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDM_BUILD_ATSC:
            {
                if(FAILED(g_pfg->BuildGraph(ATSC)))
                {
                    ErrorMessageBox(TEXT("Could not Build the ATSC BDA FilterGraph."));
                }
                else
                {
                    g_pfg->SetVideoWindow(hwndMain);
                }
        
                HMENU hMenu = GetSubMenu (GetMenu (hwnd), 1);

                EnableMenuItem (hMenu, IDM_BUILD_ATSC, MF_GRAYED | MF_BYCOMMAND);

                EnableMenuItem (
                    hMenu, 
                    IDM_STOP_GRAPH, 
                    (g_pfg->m_fGraphRunning) ? (MF_BYCOMMAND|MF_ENABLED) : (MF_BYCOMMAND|MF_GRAYED) 
                    );

                EnableMenuItem (
                    hMenu, 
                    IDM_SELECT_CHANNEL, 
                    (g_pfg->m_fGraphBuilt) ? (MF_BYCOMMAND|MF_ENABLED) : (MF_BYCOMMAND|MF_GRAYED) );
                break;
            }
            
        case IDM_RUN_GRAPH:
            {
                if(g_pfg->m_fGraphBuilt)
                {   
                    if(!g_pfg->m_fGraphRunning)
                    {
                        if(FAILED(g_pfg->RunGraph()))
                        {
                            ErrorMessageBox(TEXT("Could not play the FilterGraph."));
                        }
                    }
                }
                else
                {
                    ErrorMessageBox(TEXT("The FilterGraph is not yet built."));
                }

                break;
            }
            
        case IDM_STOP_GRAPH:
            {
                if(g_pfg->m_fGraphBuilt)
                {
                    if(g_pfg->m_fGraphRunning)
                    {
                        if(FAILED(g_pfg->StopGraph()))
                        {
                            ErrorMessageBox(TEXT("Could not stop the FilterGraph,"));
                        }
                    }
                }
                else
                {
                    ErrorMessageBox(TEXT("The FilterGraph is not yet built."));
                }
                HMENU hMenu = GetSubMenu (GetMenu (hwnd), 1);
                EnableMenuItem (
                    hMenu, 
                    IDM_SELECT_CHANNEL, 
                    MF_BYCOMMAND | MF_GRAYED);
                break;
            }
            
        case IDM_SELECT_CHANNEL:
            {
                if(g_pfg->m_fGraphBuilt)
                {
                    g_hwndDlg = reinterpret_cast <HWND> ( DialogBox(
                        hInst, 
                        MAKEINTRESOURCE(IDD_SELECT_CHANNEL),
                        hwnd, 
                        reinterpret_cast<DLGPROC>(SelectChannelDlgProc)
                        ) );
                }
                else
                {
                    ErrorMessageBox(TEXT("The FilterGraph is not yet built."));
                }
                break;
            }
            
        case IDM_ABOUT:
            {
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), hwnd, 
                          reinterpret_cast<DLGPROC>(AboutDlgProc));
                break;
            }
            
        case IDM_EXIT:
            {
                DbgTerminate();
                PostQuitMessage(0);
                break;
            }
            
        default:
            break;
        }

    case WM_INITMENU:
        if(g_pfg->m_fGraphFailure)
        {
            EnableMenuItem((HMENU)wParam, IDM_BUILD_ATSC, MF_BYCOMMAND| MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDM_RUN_GRAPH, MF_BYCOMMAND| MF_GRAYED);
            EnableMenuItem((HMENU)wParam, IDM_STOP_GRAPH, MF_BYCOMMAND| MF_GRAYED);
        }
        else
        {
            EnableMenuItem((HMENU)wParam, IDM_RUN_GRAPH, 
                g_pfg->m_fGraphRunning ? MF_BYCOMMAND|MF_GRAYED : MF_BYCOMMAND|MF_ENABLED);

            EnableMenuItem((HMENU)wParam, IDM_BUILD_ATSC, 
                g_pfg->m_fGraphBuilt ? MF_BYCOMMAND|MF_GRAYED : MF_BYCOMMAND|MF_ENABLED);
        
            // we can stop viewing if it's currently viewing
            EnableMenuItem((HMENU)wParam, IDM_STOP_GRAPH, 
                (g_pfg->m_fGraphRunning) ? MF_BYCOMMAND|MF_ENABLED : MF_BYCOMMAND|MF_GRAYED);
        }
        EnableMenuItem(
            (HMENU)wParam, 
            IDM_SELECT_CHANNEL, 
            g_pfg->m_fGraphBuilt ? MF_BYCOMMAND|MF_ENABLED : MF_BYCOMMAND|MF_GRAYED);

        break;

    case WM_CLOSE:
    case WM_DESTROY:
        DbgTerminate();
        PostQuitMessage(0);
        break;
        
    default:
        break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}


// AboutDlgProc
//
// Dialog Procedure for the "about" dialog box.
//
BOOL CALLBACK 
AboutDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg) 
    {
        case WM_COMMAND:
            EndDialog(hwnd, TRUE);
            return TRUE;
    
        case WM_INITDIALOG:
            return TRUE;
    }
    return FALSE;
}


// SelectChannelDlgProc
// Dialog Procedure for the "Select Channel" dialog box.
//                                                                              
BOOL CALLBACK
SelectChannelDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LONG    lChannelMaj  = g_pfg->GetMajorChannel ();
    LONG    lChannelMin  = g_pfg->GetMinorChannel ();
    BOOL    bRet    = FALSE;
    
    switch(message)
    {
    case WM_INITDIALOG:
        {
            //refresh the controls
            SetDlgItemInt (hDlg, IDC_MAJOR_CHANNEL, lChannelMaj, TRUE);
            SetDlgItemInt (hDlg, IDC_MINOR_CHANNEL, lChannelMin, TRUE);
            //set the spin controls
            HWND hWndSpin = GetDlgItem (hDlg, IDC_SPIN_MAJOR);
            ::SendMessage(
                hWndSpin, 
                UDM_SETRANGE32, 
                static_cast <WPARAM> (MINOR_CHANNEL_LOWER_LIMIT),
                static_cast <LPARAM> (MAJOR_CHANNEL_UPPER_LIMIT)
                ); 
            hWndSpin = GetDlgItem (hDlg, IDC_SPIN_MINOR);
            ::SendMessage(
                hWndSpin, 
                UDM_SETRANGE32, 
                static_cast <WPARAM> (MINOR_CHANNEL_LOWER_LIMIT), 
                static_cast <LPARAM> (MINOR_CHANNEL_UPPER_LIMIT)
                ); 
            break;
        }
    case WM_DESTROY:
        {
            EndDialog (hDlg, 0);
            return TRUE;
        }
    case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
            case IDC_ENTER:
                {
                    lChannelMaj = (LONG) GetDlgItemInt(hDlg, IDC_MAJOR_CHANNEL, &bRet, TRUE);
                    lChannelMin = (LONG) GetDlgItemInt(hDlg, IDC_MINOR_CHANNEL, &bRet, TRUE);
                    g_pfg->ChangeChannel (lChannelMaj, lChannelMin);
                    break;
                }
            case IDOK:
                {
                    lChannelMaj = (LONG) GetDlgItemInt(hDlg, IDC_MAJOR_CHANNEL, &bRet, TRUE);
                    lChannelMin = (LONG) GetDlgItemInt(hDlg, IDC_MINOR_CHANNEL, &bRet, TRUE);
                    g_pfg->ChangeChannel (lChannelMaj, lChannelMin);
                }
            case IDCANCEL:
                {
                    EndDialog (hDlg, 0);
                    break;
                }
            }
            break;
        }
    }
    return FALSE;
}


// ErrorMessageBox
//
// Opens a Message box with a error message in it.  The user can     
// select the OK button to continue.
//
VOID
ErrorMessageBox(LPTSTR sz,...)
{
    static TCHAR    ach[2000];
    va_list         va;

    va_start(va, sz);
    wvsprintf(ach, sz, va);
    va_end(va);

    MessageBox(hwndMain, ach, NULL, MB_OK|MB_ICONEXCLAMATION|MB_TASKMODAL);
}

