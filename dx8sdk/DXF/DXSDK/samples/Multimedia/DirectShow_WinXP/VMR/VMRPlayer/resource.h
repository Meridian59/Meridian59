// Copyright (c) 1995-2001  Microsoft Corporation.  All Rights Reserved.
//
// These are indexes used by the toolbar.
//
#define IDC_STATIC                      -1

#define IDX_SEPARATOR                   -1
#define IDX_1                           0
#define IDX_2                           1
#define IDX_3                           2
#define IDX_4                           3
#define IDX_5                           4
#define IDX_6                           5
#define IDX_7                           6
#define IDX_8                           7
#define IDX_9                           8
#define IDX_10                          9
#define IDX_11                          10
#define IDX_12                          11
#define DEFAULT_TBAR_SIZE               10
#define NUMBER_OF_BITMAPS               12

#define ID_STATUSBAR                    28
#define ID_TOOLBAR                      29
#define ID_TRACKBAR                     30

#define IDR_MAIN_MENU                   101
#define IDR_TOOLBAR                     102
#define IDR_VIDEOCD_ICON                103
#define IDR_ACCELERATOR                 104
#define IDR_VMR                         105
#define IDD_ABOUTBOX                    200

#define IDC_XPOS_TRK                    1000
#define IDC_YPOS_TRK                    1001
#define IDC_XPOS                        1002
#define IDC_YPOS                        1003
#define IDC_XSIZE_TRK                   1004
#define IDC_YSIZE_TRK                   1005
#define IDC_XSIZE                       1006
#define IDC_YSIZE                       1007
#define IDC_IMAGE_ENABLE                1008
#define IDC_ALPHA_TRK2                  1009
#define IDC_ALPHA                       1010

#define IDM_FILE_OPEN                   40001
#define IDM_FILE_OPEN2                  40005
#define IDM_FILE_CLOSE                  40002
#define IDM_FILE_EXIT                   40003

#define IDM_APP_IMAGE                   42000
#define IDM_STREAM_A                    42001
#define IDM_STREAM_B                    42002
#define IDM_CAPTURE_IMAGE               42003
#define IDM_DISPLAY_CAPTURED_IMAGE      42004

#define IDM_HELP_ABOUT                  40102

// Toolbar commands
#define IDM_MOVIE_STOP                  40010
#define IDM_MOVIE_PLAY                  40011
#define IDM_MOVIE_PREVTRACK             40012
#define IDM_MOVIE_PAUSE                 40013
#define IDM_MOVIE_SKIP_FORE             40014
#define IDM_MOVIE_SKIP_BACK             40015
#define IDM_MOVIE_STEP                  40021

#define MENU_STRING_BASE                1000

// Different time formats
#define IDM_TIME                        40150
#define IDM_FRAME                       40151
#define IDM_FIELD                       40152
#define IDM_SAMPLE                      40153
#define IDM_BYTES                       40154

        // File
#define STR_FILE_OPEN           IDM_FILE_OPEN  + MENU_STRING_BASE
#define STR_FILE_OPEN2          IDM_FILE_OPEN2 + MENU_STRING_BASE
#define STR_FILE_CLOSE          IDM_FILE_CLOSE + MENU_STRING_BASE
#define STR_FILE_EXIT           IDM_FILE_EXIT  + MENU_STRING_BASE

        // Properties Menu
#define STR_APP_IMAGE			IDM_APP_IMAGE  + MENU_STRING_BASE
#define STR_STREAM_A			IDM_STREAM_A   + MENU_STRING_BASE
#define STR_STREAM_B			IDM_STREAM_B   + MENU_STRING_BASE
#define STR_CAPTURE_IMAGE		IDM_CAPTURE_IMAGE + MENU_STRING_BASE
#define STR_DISPLAY_CAPTURED_IMAGE		IDM_DISPLAY_CAPTURED_IMAGE + MENU_STRING_BASE

        // Help Menu            HELP_MENU_BASE
#define STR_HELP_ABOUT          IDM_HELP_ABOUT + MENU_STRING_BASE

        // System Menu
#define STR_SYSMENU_RESTORE     1800
#define STR_SYSMENU_MOVE        1801
#define STR_SYSMENU_MINIMIZE    1802
#define STR_SYSMENU_CLOSE       1803
#define STR_SYSMENU_MAXIMIZE    1804
#define STR_SYSMENU_TASK_LIST   1805

#define STR_FILE_FILTER         2000
#define STR_APP_TITLE           2001
#define STR_APP_TITLE_LOADED    2002

#define IDD_AUDIOPROP           4000 
#define IDD_VIDEOPROP           4001

#define STR_MAX_STRING_LEN      256
