//-----------------------------------------------------------------------------
// File: keyboard.cpp
//
// Desc: The Keyboard sample show how to use a DirectInput keyboard device 
//       and the differences between cooperative levels and data styles. 
//
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define STRICT
#include <tchar.h>
#include <windows.h>
#include <basetsd.h>
#include <dinput.h>
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

#define SAMPLE_BUFFER_SIZE 8  // arbitrary number of buffer elements

LPDIRECTINPUT8       g_pDI       = NULL;         
LPDIRECTINPUTDEVICE8 g_pKeyboard = NULL;     




//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for 
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
int WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, int )
{
    // Display the main dialog box.
    DialogBox( hInst, MAKEINTRESOURCE(IDD_KEYBOARD), NULL, MainDlgProc );
    
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
                case IDC_WINDOWSKEY:
                    UpdateUI( hDlg );
                    break;

                case IDC_CREATEDEVICE:
                    if( NULL == g_pKeyboard )
                    {
                        if( FAILED( OnCreateDevice( hDlg ) ) )
                        {
                            MessageBox( hDlg, _T("CreateDevice() failed. ")
                                              _T("The sample will now exit."), 
                                              _T("Keyboard"), MB_ICONERROR | MB_OK );
                            FreeDirectInput();
                        }
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

        case WM_ACTIVATE:
            if( WA_INACTIVE != wParam && g_pKeyboard )
            {
                // Make sure the device is acquired, if we are gaining focus.
                g_pKeyboard->Acquire();
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
                        MessageBox( NULL, _T("Error reading input state. ")
                                          _T("The sample will now exit."), 
                                          _T("Keyboard"), MB_ICONERROR | MB_OK );
                        EndDialog( hDlg, TRUE ); 
                    }
                }
                else
                {
                    if( FAILED( ReadBufferedData( hDlg ) ) )
                    {
                        KillTimer( hDlg, 0 );    
                        MessageBox( NULL, _T("Error reading input state. ")
                                          _T("The sample will now exit."), 
                                          _T("Keyboard"), MB_ICONERROR | MB_OK );
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
    HICON hIcon = LoadIcon( GetModuleHandle(NULL), MAKEINTRESOURCE( IDI_MAIN ) );

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
    BOOL    bDisableWindowsKey;

    // Detrimine where the buffer would like to be allocated 
    bExclusive         = ( IsDlgButtonChecked( hDlg, IDC_EXCLUSIVE  ) == BST_CHECKED );
    bForeground        = ( IsDlgButtonChecked( hDlg, IDC_FOREGROUND ) == BST_CHECKED );
    bImmediate         = ( IsDlgButtonChecked( hDlg, IDC_IMMEDIATE  ) == BST_CHECKED );
    bDisableWindowsKey = ( IsDlgButtonChecked( hDlg, IDC_WINDOWSKEY ) == BST_CHECKED );

    if( g_pKeyboard )
    {
        SetDlgItemText( hDlg, IDC_CREATEDEVICE, TEXT("Release Device") );
        SetDlgItemText( hDlg, IDC_DATA, TEXT("") );

        EnableWindow( GetDlgItem( hDlg, IDC_EXCLUSIVE    ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_NONEXCLUSIVE ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_FOREGROUND   ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_BACKGROUND   ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_IMMEDIATE    ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_BUFFERED     ), FALSE );
        EnableWindow( GetDlgItem( hDlg, IDC_WINDOWSKEY   ), FALSE );
    }
    else
    {
        SetDlgItemText( hDlg, IDC_CREATEDEVICE, TEXT("&Create Device") );
        SetDlgItemText( hDlg, IDC_DATA, 
                        TEXT("Device not created. Choose settings and click 'Create Device' then type to see results") );   

        EnableWindow( GetDlgItem( hDlg, IDC_EXCLUSIVE    ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_NONEXCLUSIVE ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_FOREGROUND   ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_BACKGROUND   ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_IMMEDIATE    ), TRUE );
        EnableWindow( GetDlgItem( hDlg, IDC_BUFFERED     ), TRUE );

        if( !bExclusive && bForeground )
            EnableWindow( GetDlgItem( hDlg, IDC_WINDOWSKEY ), TRUE );
        else
            EnableWindow( GetDlgItem( hDlg, IDC_WINDOWSKEY ), FALSE );
    }

    // Figure what the user should expect based on the dialog choice
    if( !bForeground && bExclusive )
    {
        _tcscpy( strExcepted, TEXT("For security reasons, background exclusive ") \
                             TEXT("keyboard access is not allowed.\n\n") );
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
                                 TEXT("When an application has exclusive access to the ") \
                                 TEXT("keyboard, DirectInput suppresses all keyboard ") \
                                 TEXT("messages including the Windows key except ") \
                                 TEXT("CTRL+ALT+DEL and ALT+TAB\n\n") );
        }
        else
        {
            _tcscat( strExcepted, TEXT("Nonexclusive mode means that other applications ") \
                                 TEXT("can acquire device in exclusive or nonexclusive mode. ") );

            if( bDisableWindowsKey )
            {
                _tcscat( strExcepted, TEXT("The Windows key will also be disabled so that ") \
                                     TEXT("users cannot inadvertently break out of the ") \
                                     TEXT("application. ") );
            }

            _tcscat( strExcepted, TEXT("\n\n") );
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

        _tcscat( strExcepted, TEXT("The sample will read the keyboard 12 times a second. ") \
                             TEXT("Typically an application would poll the keyboard ") \
                             TEXT("much faster than this, but this slow rate is simply ") \
                             TEXT("for the purposes of demonstration.") );
    }

    // Tell the user what to expect
    SetDlgItemText( hDlg, IDC_BEHAVIOR, strExcepted );
}




//-----------------------------------------------------------------------------
// Name: OnCreateDevice()
// Desc: Setups a the keyboard device using the flags from the dialog.
//-----------------------------------------------------------------------------
HRESULT OnCreateDevice( HWND hDlg )
{
    HRESULT hr;
    BOOL    bExclusive;
    BOOL    bForeground;
    BOOL    bImmediate;
    BOOL    bDisableWindowsKey;
    DWORD   dwCoopFlags;
#ifdef _WIN64
    HINSTANCE hInst = (HINSTANCE) GetWindowLongPtr( hDlg, GWLP_HINSTANCE );
#else
    HINSTANCE hInst = (HINSTANCE) GetWindowLong( hDlg, GWL_HINSTANCE );
#endif

    // Cleanup any previous call first
    KillTimer( hDlg, 0 );    
    FreeDirectInput();

    // Detrimine where the buffer would like to be allocated 
    bExclusive         = ( IsDlgButtonChecked( hDlg, IDC_EXCLUSIVE  ) == BST_CHECKED );
    bForeground        = ( IsDlgButtonChecked( hDlg, IDC_FOREGROUND ) == BST_CHECKED );
    bImmediate         = ( IsDlgButtonChecked( hDlg, IDC_IMMEDIATE  ) == BST_CHECKED );
    bDisableWindowsKey = ( IsDlgButtonChecked( hDlg, IDC_WINDOWSKEY ) == BST_CHECKED );

    if( bExclusive )
        dwCoopFlags = DISCL_EXCLUSIVE;
    else
        dwCoopFlags = DISCL_NONEXCLUSIVE;

    if( bForeground )
        dwCoopFlags |= DISCL_FOREGROUND;
    else
        dwCoopFlags |= DISCL_BACKGROUND;

    // Disabling the windows key is only allowed only if we are in foreground nonexclusive
    if( bDisableWindowsKey && !bExclusive && bForeground )
        dwCoopFlags |= DISCL_NOWINKEY;

    // Create a DInput object
    if( FAILED( hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                                         IID_IDirectInput8, (VOID**)&g_pDI, NULL ) ) )
        return hr;
    
    // Obtain an interface to the system keyboard device.
    if( FAILED( hr = g_pDI->CreateDevice( GUID_SysKeyboard, &g_pKeyboard, NULL ) ) )
        return hr;
    
    // Set the data format to "keyboard format" - a predefined data format 
    //
    // A data format specifies which controls on a device we
    // are interested in, and how they should be reported.
    //
    // This tells DirectInput that we will be passing an array
    // of 256 bytes to IDirectInputDevice::GetDeviceState.
    if( FAILED( hr = g_pKeyboard->SetDataFormat( &c_dfDIKeyboard ) ) )
        return hr;
    
    // Set the cooperativity level to let DirectInput know how
    // this device should interact with the system and with other
    // DirectInput applications.
    hr = g_pKeyboard->SetCooperativeLevel( hDlg, dwCoopFlags );
    if( hr == DIERR_UNSUPPORTED && !bForeground && bExclusive )
    {
        FreeDirectInput();
        MessageBox( hDlg, _T("SetCooperativeLevel() returned DIERR_UNSUPPORTED.\n")
                          _T("For security reasons, background exclusive keyboard\n")
                          _T("access is not allowed."), _T("Keyboard"), MB_OK );
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
        // Set the buffer size to DINPUT_BUFFERSIZE (defined above) elements.
        //
        // The buffer size is a DWORD property associated with the device.
        DIPROPDWORD dipdw;

        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj        = 0;
        dipdw.diph.dwHow        = DIPH_DEVICE;
        dipdw.dwData            = SAMPLE_BUFFER_SIZE; // Arbitary buffer size

        if( FAILED( hr = g_pKeyboard->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) ) )
            return hr;
    }

    // Acquire the newly created device
    g_pKeyboard->Acquire();

    // Set a timer to go off 12 times a second, to read input
    // Note: Typically an application would poll the keyboard
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
    HRESULT hr;
    TCHAR   strNewText[256*5 + 1] = TEXT("");
    TCHAR   strElement[10];    
    BYTE    diks[256];   // DirectInput keyboard state buffer 
    int     i;

    if( NULL == g_pKeyboard ) 
        return S_OK;
    
    // Get the input's device state, and put the state in dims
    ZeroMemory( &diks, sizeof(diks) );
    hr = g_pKeyboard->GetDeviceState( sizeof(diks), &diks );
    if( FAILED(hr) ) 
    {
        // DirectInput may be telling us that the input stream has been
        // interrupted.  We aren't tracking any state between polls, so
        // we don't have any special reset that needs to be done.
        // We just re-acquire and try again.
        
        // If input is lost then acquire and keep trying 
        hr = g_pKeyboard->Acquire();
        while( hr == DIERR_INPUTLOST ) 
            hr = g_pKeyboard->Acquire();

        // Update the dialog text 
        if( hr == DIERR_OTHERAPPHASPRIO || hr == DIERR_NOTACQUIRED ) 
            SetDlgItemText( hDlg, IDC_DATA, TEXT("Unacquired") );

        // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        // may occur when the app is minimized or in the process of 
        // switching, so just try again later 
        return S_OK; 
    }
    
    // Make a string of the index values of the keys that are down
    for( i = 0; i < 256; i++ ) 
    {
        if( diks[i] & 0x80 ) 
        {
            wsprintf( strElement, TEXT("0x%02x "), i );
            _tcscat( strNewText, strElement );
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
// Name: ReadBufferedData()
// Desc: Read the input device's state when in buffered mode and display it.
//-----------------------------------------------------------------------------
HRESULT ReadBufferedData( HWND hDlg )
{
    TCHAR              strNewText[256*5 + 1] = TEXT("");
    TCHAR              strLetter[10];    
    DIDEVICEOBJECTDATA didod[ SAMPLE_BUFFER_SIZE ];  // Receives buffered data 
    DWORD              dwElements;
    DWORD              i;
    HRESULT            hr;

    if( NULL == g_pKeyboard ) 
        return S_OK;
    
    dwElements = SAMPLE_BUFFER_SIZE;
    hr = g_pKeyboard->GetDeviceData( sizeof(DIDEVICEOBJECTDATA),
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
        hr = g_pKeyboard->Acquire();
        while( hr == DIERR_INPUTLOST ) 
            hr = g_pKeyboard->Acquire();

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
        wsprintf( strLetter, TEXT("0x%02x%s "), didod[ i ].dwOfs,
                                         (didod[ i ].dwData & 0x80) ? TEXT("D") : TEXT("U"));
        _tcscat( strNewText, strLetter );
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
    if( g_pKeyboard ) 
        g_pKeyboard->Unacquire();
    
    // Release any DirectInput objects.
    SAFE_RELEASE( g_pKeyboard );
    SAFE_RELEASE( g_pDI );
}






