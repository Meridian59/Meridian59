//------------------------------------------------------------------------------
// File: AMCap.h
//
// Desc: DirectShow sample code - audio/video capture.
//
// Copyright (c) 1993-2001 Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#define ID_APP      1000

/* Menu Items */
#define MENU_EXIT           	4
#define MENU_SET_CAP_FILE	5
#define MENU_ALLOC_CAP_FILE	6
#define MENU_START_CAP		7
#define MENU_STOP_CAP		8
#define MENU_CAP_CC		9
#define MENU_CAP_AUDIO		12
#define MENU_AUDIOFORMAT	13
#define MENU_FRAMERATE		14
#define MENU_PREVIEW		15
#define MENU_VDEVICE0		16
#define MENU_VDEVICE1		17
#define MENU_VDEVICE2		18
#define MENU_VDEVICE3		19
#define MENU_VDEVICE4		20
#define MENU_VDEVICE5		21
#define MENU_VDEVICE6		22
#define MENU_VDEVICE7		23
#define MENU_VDEVICE8		24
#define MENU_VDEVICE9		25
#define MENU_ADEVICE0		26
#define MENU_ADEVICE1		27
#define MENU_ADEVICE2		28
#define MENU_ADEVICE3		29
#define MENU_ADEVICE4		30
#define MENU_ADEVICE5		31
#define MENU_ADEVICE6		32
#define MENU_ADEVICE7		33
#define MENU_ADEVICE8		34
#define MENU_ADEVICE9		35
#define MENU_ABOUT              36
#define MENU_SAVE_CAP_FILE	37
#define MENU_NOMASTER		38
#define MENU_AUDIOMASTER	39
#define MENU_VIDEOMASTER	40
#define MENU_TIMELIMIT		41
#define MENU_DIALOG0		42
#define MENU_DIALOG1		43
#define MENU_DIALOG2		44
#define MENU_DIALOG3		45
#define MENU_DIALOG4		46
#define MENU_DIALOG5		47
#define MENU_DIALOG6		48
#define MENU_DIALOG7		49
#define MENU_DIALOG8		50
#define MENU_DIALOG9		51
#define MENU_DIALOGA		52
#define MENU_DIALOGB		53
#define MENU_DIALOGC		54
#define MENU_DIALOGD		55
#define MENU_DIALOGE		56
#define MENU_DIALOGF		57	// !!! more?

// Dialogs
#define IDD_ABOUT           	    600
#define IDD_AllocCapFileSpace       601
#define IDD_FrameRateDialog         602
#define IDD_PressAKeyDialog         603
#define IDD_TimeLimitDialog         604

// defines for dialogs
#define IDD_SetCapFileFree          400
#define IDD_SetCapFileSize          401
#define IDC_FRAMERATE		    402
#define IDC_CAPFILENAME		    403
#define IDC_TIMELIMIT		    404
#define IDC_USETIMELIMIT	    405
#define IDC_USEFRAMERATE	    406

// window messages
#define WM_FGNOTIFY	WM_USER+1

// device notification definitions
#if (WINVER < 0x0500)

#define DBT_DEVTYP_DEVICEINTERFACE      0x00000005  // device interface class
#define DEVICE_NOTIFY_WINDOW_HANDLE     0x00000000
typedef  PVOID           HDEVNOTIFY;

#endif

extern "C"
{
    typedef BOOL (/* WINUSERAPI */ WINAPI *PUnregisterDeviceNotification)(
        IN HDEVNOTIFY Handle
        );

    typedef HDEVNOTIFY (/* WINUSERAPI */ WINAPI *PRegisterDeviceNotificationA)(
        IN HANDLE hRecipient,
        IN LPVOID NotificationFilter,
        IN DWORD Flags
        );

    typedef HDEVNOTIFY (/* WINUSERAPI */ WINAPI *PRegisterDeviceNotificationW)(
        IN HANDLE hRecipient,
        IN LPVOID NotificationFilter,
        IN DWORD Flags
        );
}
#ifdef UNICODE
#define PRegisterDeviceNotification  PRegisterDeviceNotificationW
#else
#define PRegisterDeviceNotification  PRegisterDeviceNotificationA
#endif // !UNICODE

#if (WINVER < 0x0500)

typedef struct _DEV_BROADCAST_DEVICEINTERFACE_A {
    DWORD       dbcc_size;
    DWORD       dbcc_devicetype;
    DWORD       dbcc_reserved;
    GUID        dbcc_classguid;
    char        dbcc_name[1];
} DEV_BROADCAST_DEVICEINTERFACE_A, *PDEV_BROADCAST_DEVICEINTERFACE_A;

typedef struct _DEV_BROADCAST_DEVICEINTERFACE_W {
    DWORD       dbcc_size;
    DWORD       dbcc_devicetype;
    DWORD       dbcc_reserved;
    GUID        dbcc_classguid;
    wchar_t     dbcc_name[1];
} DEV_BROADCAST_DEVICEINTERFACE_W, *PDEV_BROADCAST_DEVICEINTERFACE_W;

#ifdef UNICODE
typedef DEV_BROADCAST_DEVICEINTERFACE_W   DEV_BROADCAST_DEVICEINTERFACE;
typedef PDEV_BROADCAST_DEVICEINTERFACE_W  PDEV_BROADCAST_DEVICEINTERFACE;
#else
typedef DEV_BROADCAST_DEVICEINTERFACE_A   DEV_BROADCAST_DEVICEINTERFACE;
typedef PDEV_BROADCAST_DEVICEINTERFACE_A  PDEV_BROADCAST_DEVICEINTERFACE;
#endif // UNICODE
#endif // WINVER
