//-----------------------------------------------------------------------------
// File: AudioScripts.cpp
//
// Desc: Plays a script file using DirectMusic
//
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <basetsd.h>
#include <commdlg.h>
#include <objbase.h>
#include <stdio.h>
#include <dmusicc.h>
#include <dmusici.h>
#include <dxerr8.h>
#include "resource.h"
#include <tchar.h>
#include "DMUtil.h"
#include "DXUtil.h"




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT OnInitDialog( HWND hDlg );
HRESULT OnChangeScriptFile( HWND hDlg, TCHAR* strFileName );
HRESULT OnCallRoutine( HWND hDlg, TCHAR* strRoutine );
HRESULT UpdateVariables( HWND hDlg );
HRESULT SetVariable( HWND hDlg, int nIDDlgItem );




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
CMusicManager* g_pMusicManager = NULL;
CMusicScript*  g_pMusicScript  = NULL;
HINSTANCE      g_hInst         = NULL;




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, 
                      INT nCmdShow )
{
    g_hInst = hInst;
    
    // Display the main dialog box.
    DialogBox( hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc );
    
    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: MainDlgProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    HRESULT hr;

    switch( msg ) 
    {
        case WM_INITDIALOG:
            if( FAILED( hr = OnInitDialog( hDlg ) ) )
            {
                DXTRACE_ERR( TEXT("OnInitDialog"), hr );
                MessageBox( hDlg, "Error initializing DirectMusic.  Sample will now exit.", 
                                  "DirectMusic Sample", MB_OK | MB_ICONERROR );
                EndDialog( hDlg, 0 );
                return TRUE;
            }
            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDC_COMBO_SCRIPT:
                    if ( HIWORD(wParam) == CBN_SELCHANGE )
                    {
                        char strFile[MAX_PATH] = "";
                        GetDlgItemText( hDlg, IDC_COMBO_SCRIPT, strFile, MAX_PATH );
                        if( FAILED( hr = OnChangeScriptFile( hDlg, strFile ) ) )
                        {
                            DXTRACE_ERR( TEXT("OnChangeScriptFile"), hr );
                            MessageBox( hDlg, "Error loading script.  Sample will now exit.", 
                                              "DirectMusic Sample", MB_OK | MB_ICONERROR );
                            EndDialog( hDlg, 0 );
                            return TRUE;
                        }
                        break;
                    }

                case IDC_BUTTON_ROUTINE1:
                case IDC_BUTTON_ROUTINE2:
                case IDC_BUTTON_ROUTINE3:
                    if ( HIWORD(wParam) == BN_CLICKED )
                    {
                        TCHAR pstrRoutine[] = TEXT("RoutineX");
                        pstrRoutine[_tcslen(pstrRoutine) - 1] = '1' + (LOWORD(wParam) - IDC_BUTTON_ROUTINE1);
                        if( FAILED( hr = OnCallRoutine(hDlg, pstrRoutine) ) ) 
                        {
                            DXTRACE_ERR( TEXT("OnCallRoutine"), hr );
                            MessageBox( hDlg, "Error calling routine.  Sample will now exit.", 
                                              "DirectMusic Sample", MB_OK | MB_ICONERROR );
                            EndDialog( hDlg, 0 );
                            return TRUE;
                        }
                    }
                    break;

                case IDC_EDIT_VARIABLE1:
                case IDC_EDIT_VARIABLE2:
                    if ( HIWORD(wParam) == EN_KILLFOCUS )
                    {
                        if( FAILED( hr = SetVariable( hDlg, LOWORD(wParam) ) ) )
                        {
                            DXTRACE_ERR( TEXT("SetVariable"), hr );
                            MessageBox( hDlg, "Error setting variable.  Sample will now exit.", 
                                              "DirectMusic Sample", MB_OK | MB_ICONERROR );
                            EndDialog( hDlg, 0 );
                            return TRUE;
                        }
                    }
                    break;

                case IDOK:
                    if ( HIWORD(wParam) == BN_CLICKED )
                    {
                        // This is sent by edit boxes to click the default button
                        // (which we don't have) when Enter is pressed.
                        HWND hwndFocus = GetFocus();
                        int nIDDlgItem = 0;
                        if ( hwndFocus == GetDlgItem(hDlg, IDC_EDIT_VARIABLE1) )
                            nIDDlgItem = IDC_EDIT_VARIABLE1;
                        else if ( hwndFocus == GetDlgItem(hDlg, IDC_EDIT_VARIABLE2) )
                            nIDDlgItem = IDC_EDIT_VARIABLE2;

                        if ( nIDDlgItem != 0)
                        {
                            if( FAILED( hr = SetVariable( hDlg, nIDDlgItem ) ) )
                            {
                                DXTRACE_ERR( TEXT("SetVariable"), hr );
                                MessageBox( hDlg, "Error setting variable.  Sample will now exit.", 
                                                  "DirectMusic Sample", MB_OK | MB_ICONERROR );
                                EndDialog( hDlg, 0 );
                                return TRUE;
                            }

                            // Select the value to show that Enter committed it
                            SendMessage(hwndFocus, EM_SETSEL, 0, -1);
                        }
                    }
                    break;

                case IDCANCEL:
                    EndDialog( hDlg, 0 );
                    break;

                default:
                    return FALSE; // Didn't handle message
            }
            break;

        case WM_DESTROY:
            // Cleanup everything
            SAFE_DELETE( g_pMusicScript );
            SAFE_DELETE( g_pMusicManager );
            break; 

        default:
            return FALSE; // Didn't handle message
    }

    return TRUE; // Handled message
}




//-----------------------------------------------------------------------------
// Name: OnInitDialog()
// Desc: Initializes the dialogs (sets up UI controls, etc.)
//-----------------------------------------------------------------------------
HRESULT OnInitDialog( HWND hDlg )
{
    HRESULT hr;

    // Load the icon
    HICON hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( IDR_MAINFRAME ) );

    // Set the icon for this dialog.
    SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    g_pMusicManager = new CMusicManager();

    if( FAILED( hr = g_pMusicManager->Initialize( hDlg ) ) )
        return hr;

    // Set the default media path (something like C:\MSSDK\SAMPLES\MULTIMEDIA\MEDIA)
    // to be used as the search directory for finding DirectMusic content.
    g_pMusicManager->SetSearchDirectory( DXUtil_GetDXSDKMediaPath() );

    // Fill the dropdown script choices
    SendDlgItemMessage( hDlg, IDC_COMBO_SCRIPT, CB_ADDSTRING, 
                        0, (LPARAM) "ScriptDemoBasic.spt" );
    SendDlgItemMessage( hDlg, IDC_COMBO_SCRIPT, CB_ADDSTRING, 
                        0, (LPARAM) "ScriptDemoBaseball.spt" );

    // Load ScriptDemoBasic
    OnChangeScriptFile( hDlg, "ScriptDemoBasic.spt" );
    SendDlgItemMessage( hDlg, IDC_COMBO_SCRIPT, CB_SELECTSTRING, 
                        -1, (LPARAM) "ScriptDemoBasic.spt" );

    if( g_pMusicScript )
    {
        if( FAILED( hr = UpdateVariables( hDlg ) ) )
            return hr;
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: OnChangeScriptFile()
// Desc: Called when the user changes script files from the dropdown.
//-----------------------------------------------------------------------------
HRESULT OnChangeScriptFile( HWND hDlg, TCHAR* strFileName )
{
    HRESULT hr;

    // Free any previous script, and make a new one
    SAFE_DELETE( g_pMusicScript );

    // Have the loader collect any garbage now that the old 
    // script has been released
    g_pMusicManager->CollectGarbage();

    // Load the script file
    if( FAILED( hr = g_pMusicManager->CreateScriptFromFile( &g_pMusicScript, strFileName ) ) )
        return hr;

    // Get and display a string from the script describing itself.
    // Here we will ignore any errors.  The script doesn't have to tell 
    // about itself if it doesn't want to.
    if( SUCCEEDED( g_pMusicScript->CallRoutine( TEXT("GetAbout") ) ) )
    {
        // Scripts are normally designed to return Number and Object variables.
        // Retrieving a string is rarely used, and requires use of the more complicated
        // VARIANT data type.
        VARIANT varValue;
        VariantInit(&varValue);
        if( SUCCEEDED( g_pMusicScript->GetScript()->GetVariableVariant( L"About", 
                                                                        &varValue, NULL ) ) )
        {
            if (varValue.vt == VT_BSTR)
            {
                char strAbout[5000] = "";
                wcstombs( strAbout, varValue.bstrVal, 5000 );
                strAbout[4999] = '\0';
                SetDlgItemText( hDlg, IDC_EDIT_ABOUT, strAbout );
            }

            // The returned VARIANT contains data fields that 
            // must be freed or memory will be leaked
            VariantClear(&varValue); 
        }
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: OnCallRoutine()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT OnCallRoutine( HWND hDlg, TCHAR* strRoutine )
{
    HRESULT hr;

    if( FAILED( hr = g_pMusicScript->CallRoutine( strRoutine ) ) )
        return DXTRACE_ERR( TEXT("CallRoutine"), hr );

    if( FAILED( hr = UpdateVariables( hDlg ) ) )
        return DXTRACE_ERR( TEXT("UpdateVariables"), hr );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: UpdateVariables()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT UpdateVariables( HWND hDlg )
{
    LONG lVal = 0;
    HRESULT hr;

    if( FAILED( hr = g_pMusicScript->GetVariableNumber( TEXT("Variable1"), &lVal ) ) )
        return DXTRACE_ERR( TEXT("GetVariableNumber"), hr );
    SetDlgItemInt( hDlg, IDC_EDIT_VARIABLE1, lVal, TRUE );

    if( FAILED( hr = g_pMusicScript->GetVariableNumber( TEXT("Variable2"), &lVal ) ) )
        return DXTRACE_ERR( TEXT("GetVariableNumber"), hr );
    SetDlgItemInt( hDlg, IDC_EDIT_VARIABLE2, lVal, TRUE );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SetVariable()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT SetVariable( HWND hDlg, int nIDDlgItem )
{
    HRESULT hr;
    BOOL    fIntConvertOK = FALSE;
    int     iVal = GetDlgItemInt( hDlg, nIDDlgItem, &fIntConvertOK, TRUE );
    TCHAR* pstrVariable = (nIDDlgItem == IDC_EDIT_VARIABLE1) ? TEXT("Variable1") : TEXT("Variable2");

    hr = g_pMusicScript->SetVariableNumber( pstrVariable, iVal );
    if( FAILED(hr) )
        return hr;

    return S_OK;
}
