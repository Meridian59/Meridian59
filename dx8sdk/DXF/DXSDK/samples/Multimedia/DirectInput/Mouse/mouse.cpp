//-----------------------------------------------------------------------------
// File: Mouse.cpp
//
// Desc: The Mouse sample show how to use a DirectInput mouse device and 
//       the differences between cooperative levels and data styles. 
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <tchar.h>
#include <windows.h>
#include <basetsd.h>
#include <dinput.h>
#include <stdio.h>
#include "resource.h"




//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT OnInitDialog( HWND hDlg );
VOID    UpdateUI( HWND hDlg );
HRESULT OnCreateDevice( HWND hDlg );
HRESULT ReadImmediateData( HWND hDlg );
HRESULT ReadBufferedData( HWND hDlg );
VOID    FreeDirectInput();




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

#define SAMPLE_BUFFER_SIZE  16      // arbitrary number of buffer elements
LPDIRECTINPUT8       g_pDI    = NULL;         
LPDIRECTINPUTDEVICE8 g_pMouse = NULL;     




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, int )
{
    // Display the main dialog box.
    DialogBox( hInst, MAKEINTRESOURCE(IDD_MOUSE), NULL, MainDlgProc );
    
    return TRUE;
}





//-----------------------------------------------------------------------------
// Name: MainDlgProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    switch( msg ) 
    {
        case WM_INITDIALOG:
            OnInitDialog( hDlg );
            break;
        
        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDCANCEL:
                    EndDialog( hDlg, 0 ); 
                    break;

                case IDC_EXCLUSIVE:
                case IDC_NONEXCLUSIVE:
                case IDC_FOREGROUND:
                case IDC_BACKGROUND:
                case IDC_IMMEDIATE:
                case IDC_BUFFERED:
                    UpdateUI( hDlg );
                    break;

                case IDM_CREATEDEVICE:
                case IDM_RELEASEDEVICE:
                case IDC_CREATEDEVICE:
                    if( NULL == g_pMouse )
                    {                    
                        if( FAILED( OnCreateDevice( hDlg ) ) )
                        {
                            MessageBox( hDlg, _T("CreateDevice() failed. ")
                                              _T("The sample will now exit. "), 
                                              _T("Mouse"), MB_ICONERROR | MB_OK );
                            FreeDirectInput();
                        }

                        SetFocus( GetDlgItem( hDlg, IDC_CREATEDEVICE ) );
                    }
                    else
                    {
                        FreeDirectInput();
                    }

                    UpdateUI( hDlg );
                    break;

                default:
                    return FALSE; // Message not handled 
            }       
            break;

        case WM_ENTERMENULOOP:
            // Release the device, so if we are in exclusive mode the 
            // cursor will reappear
            if( g_pMouse )
            {
                g_pMouse->Unacquire();
                KillTimer( hDlg, 0 );  // Stop timer, so device is not re-acquired
                SetDlgItemText( hDlg, IDC_DATA, TEXT("Unacquired") );
            }
            break;

        case WM_EXITMENULOOP:
            // Make sure the device is acquired when coming out of a menu loop
            if( g_pMouse )
            {
                g_pMouse->Acquire();
                SetTimer( hDlg, 0, 1000 / 12, NULL ); // Start timer again
            }
            break;

        case WM_ACTIVATE:
            if( WA_INACTIVE != wParam && g_pMouse )
            {
                // Make sure the device is acquired, if we are gaining focus.
                g_pMouse->Acquire();
            }
            break;
        
        case WM_TIMER:
            // Update the input device every timer message
            {
                BOOL bImmediate = ( IsDlgButtonChecked( hDlg, IDC_IMMEDIATE  ) == BST_CHECKED );

                if( bImmediate )
                {
                    if( FAILED( ReadImmediateData( hDlg ) ) )
                    {
                        KillTimer( hDlg, 0 );    
                        MessageBox( NULL, _T("Error Reading Input State. ")
                                          _T("The sample will now exit. "), 
                                          _T("Mouse"), MB_ICONERROR | MB_OK );
                        EndDialog( hDlg, TRUE ); 
                    }
                }
                else
                {
                    if( FAILED( ReadBufferedData( hDlg ) ) )
                    {
                        KillTimer( hDlg, 0 );    
                        MessageBox( NULL, _T("Error Reading Input State. ")
                                          _T("The sample will now exit. "), 
                                          _T("Mouse"), MB_ICONERROR | MB_OK );
                        EndDialog( hDlg, TRUE ); 
                    }
                }
            }
            break;
        
        case WM_DESTROY:
            // Cleanup everything
            KillTimer( hDlg, 0 );    
            FreeDirectInput();    
            break;

        default:
            return FALSE; // Message not handled 
    }

    return TRUE; // Message handled 
}




//-----------------------------------------------------------------------------
// Name: OnInitDialog()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
HRESULT OnInitDialog( HWND hDlg )
{
    // Load the icon
#ifdef _WIN64
    HINSTANCE hInst = (HINSTANCE) GetWindowLongPtr( hDlg, GWLP_HINSTANCE );
#else
    HINSTANCE hInst = (HINSTANCE) GetWindowLong( hDlg, GWL_HINSTANCE );
#endif
    HICON hIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDI_MAIN ) );

    // Set the icon for this dialog.
    PostMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    PostMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    // Check the 'exclusive', 'foreground', and 'immediate' buttons by default. 
    CheckRadioButton( hDlg, IDC_EXCLUSIVE,  IDC_NONEXCLUSIVE, IDC_EXCLUSIVE );
    CheckRadioButton( hDlg, IDC_FOREGROUND, IDC_BACKGROUND,   IDC_FOREGROUND );
    CheckRadioButton( hDlg, IDC_IMMEDIATE,  IDC_BUFFERED,     IDC_IMMEDIATE );

    UpdateUI( hDlg );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: UpdateUI()
// Desc: Enables/disables the UI, and sets the dialog behavior text based on the UI
//-----------------------------------------------------------------------------
VOID UpdateUI( HWND hDlg )
{
    TCHAR   strExcepted[2048];
    BOOL    bExclusive;
    BOOL    bForeground;
    BOOL    bImmediate;

    // Detrimine where the buffer would like to be allocated 
    bExclusive         = ( IsDlgButtonChecked( hDlg, IDC_EXCLUSIVE  ) == BST_CHECKED );
    bForeground        = ( IsDlgButtonChecked( hDlg, IDC_FOREGROUND ) == BST_CHECKED );
    bImmediate         = ( IsDlgButtonChecked( hDlg, IDC_IMMEDIATE  ) == BST_CHECKED );

    if( g_pMouse )
    {
        SetDlgItemText( hDlg, IDC_CREATEDEVICE, TEXT("&Release Device") );
        EnableMenuItem( GetMenu( hDlg ), IDM_RELEASEDEVICE, MF_ENABLED );
        EnableMenuItem( GetMenu( hDlg ), IDM_CREATEDEVICE,  MF_GRAYED );
        SetDlgItemText( hDlg, IDC_DATA, TEXT("") );

        EnableWindow( GetDlgItem( hDlg, IDC_EXCLUSIVE    ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_NONEXCLUSIVE ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_FOREGROUND   ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_BACKGROUND   ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_IMMEDIATE    ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_BUFFERED     ), FALSE );

        if( bExclusive )
            SetDlgItemText( hDlg, IDC_HELP_TEXT, 
                            TEXT("Press Enter to release the mouse device, ") \
                            TEXT("and display the cursor again.") );
    }
    else
    {
        SetDlgItemText( hDlg, IDC_CREATEDEVICE, TEXT("&Create Device") );
        EnableMenuItem( GetMenu( hDlg ), IDM_RELEASEDEVICE, MF_GRAYED );
        EnableMenuItem( GetMenu( hDlg ), IDM_CREATEDEVICE,  MF_ENABLED );
        SetDlgItemText( hDlg, IDC_DATA, TEXT("Device not created. Choose settings and click 'Create Device' then move mouse to see results") );   
        SetDlgItemText( hDlg, IDC_HELP_TEXT, TEXT("") );

        EnableWindow( GetDlgItem( hDlg, IDC_EXCLUSIVE    ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_NONEXCLUSIVE ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_FOREGROUND   ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_BACKGROUND   ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_IMMEDIATE    ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_BUFFERED     ), TRUE );
    }

    // Figure what the user should expect based on the dialog choice
    if( !bForeground && bExclusive )
    {
        _tcscpy( strExcepted, TEXT("For security reasons, background exclusive ") \
                             TEXT("mouse access is not allowed.\n\n") );
    }
    else
    {
        if( bForeground )
        {
            _tcscpy( strExcepted, TEXT("Foreground cooperative level means that the ") \
                                 TEXT("application has access to data only when in the ") \
                                 TEXT("foreground or, in other words, has the input focus. ") \
                                 TEXT("If the application moves to the background, ") \
                                 TEXT("the device is automatically unacquired, or made ") \
                                 TEXT("unavailable.\n\n") );
        }
        else
        {
            _tcscpy( strExcepted, TEXT("Background cooperative level really means ") \
                                 TEXT("foreground and background. A device with a ") \
                                 TEXT("background cooperative level can be acquired ") \
                                 TEXT("and used by an application at any time.\n\n") );
        }

        if( bExclusive )
        {
            _tcscat( strExcepted, TEXT("Exclusive mode prevents other applications from ") \
                                 TEXT("also acquiring the device exclusively. The fact ") \
                                 TEXT("that your application is using a device at the ") \
                                 TEXT("exclusive level does not mean that other ") \
                                 TEXT("applications cannot get data from the device. ") \
                                 TEXT("Windows itself requires exclusive access to the ") \
                                 TEXT("mouse because mouse events such as a click on ") \
                                 TEXT("an inactive window could force an application ") \
                                 TEXT("to unacquire the device, with potentially harmful ") \
                                 TEXT("results, such as a loss of data from the input ") \
                                 TEXT("buffer. Therefore, when an application has ") \
                                 TEXT("exclusive access to the mouse, Windows is not ") \
                                 TEXT("allowed any access at all. No mouse messages are ") \
                                 TEXT("generated. A further side effect is that the ") \
                                 TEXT("cursor disappears. When accessing the menu, the sample ") \
                                 TEXT("releases the mouse so the mouse is displayed again.\n\n"));
        }
        else
        {
            _tcscat( strExcepted, TEXT("Nonexclusive mode means that other applications ") \
                                 TEXT("can acquire device in exclusive or nonexclusive mode.\n\n"));
        }

        if( bImmediate )
        {
            _tcscat( strExcepted, TEXT("Immediate data is a snapshot of the current ") \
                                 TEXT("state of a device. It provides no data about ") \
                                 TEXT("what has happened with the device since the ") \
                                 TEXT("last call, apart from implicit information that ") \
                                 TEXT("you can derive by comparing the current state with ") \
                                 TEXT("the last one. Events in between calls are lost.\n\n") );
        }
        else
        {
            _tcscat( strExcepted, TEXT("Buffered data is a record of events that are stored ") \
                                 TEXT("until an application retrieves them. With buffered ") \
                                 TEXT("data, events are stored until you are ready to deal ") \
                                 TEXT("with them. If the buffer overflows, new data is lost.\n\n") );                             
        }

        _tcscat( strExcepted, TEXT("The sample will read the mouse 12 times a second. ") \
                             TEXT("Typically an application would poll the mouse ") \
                             TEXT("much faster than this, but this slow rate is simply ") \
                             TEXT("for the purposes of demonstration.") );
    }

    // Tell the user what to expect
    SetDlgItemText( hDlg, IDC_BEHAVIOR, strExcepted );
}




//-----------------------------------------------------------------------------
// Name: OnCreateDevice()
// Desc: Setups a the mouse device using the flags from the dialog.
//-----------------------------------------------------------------------------
HRESULT OnCreateDevice( HWND hDlg )
{
    HRESULT hr;
    BOOL    bExclusive;
    BOOL    bForeground;
    BOOL    bImmediate;
    DWORD   dwCoopFlags;

    // Cleanup any previous call first
    KillTimer( hDlg, 0 );    
    FreeDirectInput();

    // Detrimine where the buffer would like to be allocated 
    bExclusive         = ( IsDlgButtonChecked( hDlg, IDC_EXCLUSIVE  ) == BST_CHECKED );
    bForeground        = ( IsDlgButtonChecked( hDlg, IDC_FOREGROUND ) == BST_CHECKED );
    bImmediate         = ( IsDlgButtonChecked( hDlg, IDC_IMMEDIATE  ) == BST_CHECKED );

    if( bExclusive )
        dwCoopFlags = DISCL_EXCLUSIVE;
    else
        dwCoopFlags = DISCL_NONEXCLUSIVE;

    if( bForeground )
        dwCoopFlags |= DISCL_FOREGROUND;
    else
        dwCoopFlags |= DISCL_BACKGROUND;

    // Create a DInput object
    if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                                         IID_IDirectInput8, (VOID**)&g_pDI, NULL ) ) )
        return hr;
    
    // Obtain an interface to the system mouse device.
    if( FAILED( hr = g_pDI->CreateDevice( GUID_SysMouse, &g_pMouse, NULL ) ) )
        return hr;
    
    // Set the data format to "mouse format" - a predefined data format 
    //
    // A data format specifies which controls on a device we
    // are interested in, and how they should be reported.
    //
    // This tells DirectInput that we will be passing a
    // DIMOUSESTATE2 structure to IDirectInputDevice::GetDeviceState.
    if( FAILED( hr = g_pMouse->SetDataFormat( &c_dfDIMouse2 ) ) )
        return hr;
    
    // Set the cooperativity level to let DirectInput know how
    // this device should interact with the system and with other
    // DirectInput applications.
    hr = g_pMouse->SetCooperativeLevel( hDlg, dwCoopFlags );
    if( hr == DIERR_UNSUPPORTED && !bForeground && bExclusive )
    {
        FreeDirectInput();
        MessageBox( hDlg, _T("SetCooperativeLevel() returned DIERR_UNSUPPORTED.\n")
                          _T("For security reasons, background exclusive mouse\n")
                          _T("access is not allowed."), 
                          _T("Mouse"), MB_OK );
        return S_OK;
    }

    if( FAILED(hr) )
        return hr;

    if( !bImmediate )
    {
        // IMPORTANT STEP TO USE BUFFERED DEVICE DATA!
        //
        // DirectInput uses unbuffered I/O (buffer size = 0) by default.
        // If you want to read buffered data, you need to set a nonzero
        // buffer size.
        //
        // Set the buffer size to SAMPLE_BUFFER_SIZE (defined above) elements.
        //
        // The buffer size is a DWORD property associated with the device.
        DIPROPDWORD dipdw;
        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj        = 0;
        dipdw.diph.dwHow        = DIPH_DEVICE;
        dipdw.dwData            = SAMPLE_BUFFER_SIZE; // Arbitary buffer size

        if( FAILED( hr = g_pMouse->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
            return hr;
    }

    // Acquire the newly created device
    g_pMouse->Acquire();

    // Set a timer to go off 12 times a second, to read input
    // Note: Typically an application would poll the mouse
    //       much faster than this, but this slow rate is simply 
    //       for the purposes of demonstration
    SetTimer( hDlg, 0, 1000 / 12, NULL );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ReadImmediateData()
// Desc: Read the input device's state when in immediate mode and display it.
//-----------------------------------------------------------------------------
HRESULT ReadImmediateData( HWND hDlg )
{
    HRESULT       hr;
    TCHAR         strNewText[128] = TEXT("");   // Output string
    DIMOUSESTATE2 dims2;      // DirectInput mouse state structure

    if( NULL == g_pMouse ) 
        return S_OK;
    
    // Get the input's device state, and put the state in dims
    ZeroMemory( &dims2, sizeof(dims2) );
    hr = g_pMouse->GetDeviceState( sizeof(DIMOUSESTATE2), &dims2 );
    if( FAILED(hr) ) 
    {
        // DirectInput may be telling us that the input stream has been
        // interrupted.  We aren't tracking any state between polls, so
        // we don't have any special reset that needs to be done.
        // We just re-acquire and try again.
        
        // If input is lost then acquire and keep trying 
        hr = g_pMouse->Acquire();
        while( hr == DIERR_INPUTLOST ) 
            hr = g_pMouse->Acquire();

        // Update the dialog text 
        if( hr == DIERR_OTHERAPPHASPRIO || 
            hr == DIERR_NOTACQUIRED ) 
            SetDlgItemText( hDlg, IDC_DATA, TEXT("Unacquired") );

        // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        // may occur when the app is minimized or in the process of 
        // switching, so just try again later 
        return S_OK; 
    }
    
    // The dims structure now has the state of the mouse, so 
    // display mouse coordinates (x, y, z) and buttons.
    _stprintf( strNewText, TEXT("(X=% 3.3d, Y=% 3.3d, Z=% 3.3d) B0=%c B1=%c B2=%c B3=%c B4=%c B5=%c B6=%c B7=%c"),
                         dims2.lX, dims2.lY, dims2.lZ,
                        (dims2.rgbButtons[0] & 0x80) ? '1' : '0',
                        (dims2.rgbButtons[1] & 0x80) ? '1' : '0',
                        (dims2.rgbButtons[2] & 0x80) ? '1' : '0',
                        (dims2.rgbButtons[3] & 0x80) ? '1' : '0',
                        (dims2.rgbButtons[4] & 0x80) ? '1' : '0',
                        (dims2.rgbButtons[5] & 0x80) ? '1' : '0',
                        (dims2.rgbButtons[6] & 0x80) ? '1' : '0',
                        (dims2.rgbButtons[7] & 0x80) ? '1' : '0');

    // Get the old text in the text box
    TCHAR strOldText[128];
    GetDlgItemText( hDlg, IDC_DATA, strOldText, 127 );
    
    // If nothing changed then don't repaint - avoid flicker
    if( 0 != lstrcmp( strOldText, strNewText ) ) 
        SetDlgItemText( hDlg, IDC_DATA, strNewText );
    
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ReadBufferedData()
// Desc: Read the input device's state when in buffered mode and display it.
//-----------------------------------------------------------------------------
HRESULT ReadBufferedData( HWND hDlg )
{
    TCHAR              strNewText[128] = TEXT(""); 
    DIDEVICEOBJECTDATA didod[ SAMPLE_BUFFER_SIZE ];  // Receives buffered data 
    DWORD              dwElements;
    DWORD              i;
    HRESULT            hr;

    if( NULL == g_pMouse ) 
        return S_OK;
    
    dwElements = SAMPLE_BUFFER_SIZE;
    hr = g_pMouse->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),
                                     didod, &dwElements, 0 );
    if( hr != DI_OK ) 
    {
        // We got an error or we got DI_BUFFEROVERFLOW.
        //
        // Either way, it means that continuous contact with the
        // device has been lost, either due to an external
        // interruption, or because the buffer overflowed
        // and some events were lost.
        //
        // Consequently, if a button was pressed at the time
        // the buffer overflowed or the connection was broken,
        // the corresponding "up" message might have been lost.
        //
        // But since our simple sample doesn't actually have
        // any state associated with button up or down events,
        // there is no state to reset.  (In a real game, ignoring
        // the buffer overflow would result in the game thinking
        // a key was held down when in fact it isn't; it's just
        // that the "up" event got lost because the buffer
        // overflowed.)
        //
        // If we want to be cleverer, we could do a
        // GetDeviceState() and compare the current state
        // against the state we think the device is in,
        // and process all the states that are currently
        // different from our private state.
        hr = g_pMouse->Acquire();
        while( hr == DIERR_INPUTLOST ) 
            hr = g_pMouse->Acquire();

        // Update the dialog text 
        if( hr == DIERR_OTHERAPPHASPRIO || 
            hr == DIERR_NOTACQUIRED ) 
            SetDlgItemText( hDlg, IDC_DATA, TEXT("Unacquired") );

        // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        // may occur when the app is minimized or in the process of 
        // switching, so just try again later 
        return S_OK; 
    }

    if( FAILED(hr) )  
        return hr;

    // Study each of the buffer elements and process them.
    //
    // Since we really don't do anything, our "processing"
    // consists merely of squirting the name into our
    // local buffer.
    for( i = 0; i < dwElements; i++ ) 
    {
        // this will display then scan code of the key
        // plus a 'D' - meaning the key was pressed 
        //   or a 'U' - meaning the key was released
        switch( didod[ i ].dwOfs )
        {
            case DIMOFS_BUTTON0:
                _tcscat( strNewText, TEXT("B0") );
                break;

            case DIMOFS_BUTTON1:
                _tcscat( strNewText, TEXT("B1") );
                break;

            case DIMOFS_BUTTON2:
                _tcscat( strNewText, TEXT("B2") );
                break;

            case DIMOFS_BUTTON3:
                _tcscat( strNewText, TEXT("B3") );
                break;

            case DIMOFS_X:
                _tcscat( strNewText, TEXT("X") );
                break;

            case DIMOFS_Y:
                _tcscat( strNewText, TEXT("Y") );
                break;

            case DIMOFS_Z:
                _tcscat( strNewText, TEXT("Z") );
                break;

            default:
                _tcscat( strNewText, TEXT("") );
        }

        switch( didod[ i ].dwOfs )
        {
            case DIMOFS_BUTTON0:
            case DIMOFS_BUTTON1:
            case DIMOFS_BUTTON2:
            case DIMOFS_BUTTON3:
                if( didod[ i ].dwData & 0x80 )
                    _tcscat( strNewText, TEXT("U ") );
                else
                    _tcscat( strNewText, TEXT("D ") );
                break;

            case DIMOFS_X:
            case DIMOFS_Y:
            case DIMOFS_Z:
            {
                TCHAR strCoordValue[20];
                wsprintf( strCoordValue, TEXT("%d "), didod[ i ].dwData );
                _tcscat( strNewText, strCoordValue );
                break;
            }
        }
    }

    // Get the old text in the text box
    TCHAR strOldText[128];
    GetDlgItemText( hDlg, IDC_DATA, strOldText, 127 );

    // If nothing changed then don't repaint - avoid flicker
    if( 0 != lstrcmp( strOldText, strNewText ) ) 
        SetDlgItemText( hDlg, IDC_DATA, strNewText );    

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: FreeDirectInput()
// Desc: Initialize the DirectInput variables.
//-----------------------------------------------------------------------------
VOID FreeDirectInput()
{
    // Unacquire the device one last time just in case 
    // the app tried to exit while the device is still acquired.
    if( g_pMouse ) 
        g_pMouse->Unacquire();
    
    // Release any DirectInput objects.
    SAFE_RELEASE( g_pMouse );
    SAFE_RELEASE( g_pDI );
}






