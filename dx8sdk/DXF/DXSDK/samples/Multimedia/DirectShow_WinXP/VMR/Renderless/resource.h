//------------------------------------------------------------------------------
// File: resourcevcdplyer.h
//
// Desc: DirectShow sample code - resource header file for RenderLess player
//
// Copyright (c) 1995 - 2001, Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#define IDC_STATIC                      -1

#define STR_MAX_STRING_LEN              256

#define IDX_SEPARATOR                   -1
#define IDX_1                           0
#define IDX_2                           1
#define IDX_3                           2
#define IDX_4                           3
#define IDX_5                           4
#define IDX_6                           5
#define IDX_7                           6
#define IDX_8                           7

#define IDX_10                          9
#define IDX_11                          10
#define DEFAULT_TBAR_SIZE               6
#define NUMBER_OF_BITMAPS               11

#define ID_TOOLBAR                      9
#define IDD_ABOUTBOX                    20

#define IDR_MAIN_MENU                   101
#define IDR_TOOLBAR                     102
#define IDR_VIDEOCD_ICON                103
#define IDR_ACCELERATOR                 104

#define IDM_FILE_OPEN                   40001
#define IDM_FILE_CLOSE                  40002
#define IDM_FILE_EXIT                   40003
#define IDM_FILE_ABOUT                  40004


// Toolbar commands
#define IDM_MOVIE_STOP                  40010
#define IDM_MOVIE_PLAY                  40011
#define IDM_MOVIE_PAUSE                 40012
#define IDM_FULL_SCREEN                 40013
#define IDM_MOVIE_FULLSCREEN            40014

#define MENU_STRING_BASE                1000

        // File
#define STR_FILE_OPEN           IDM_FILE_OPEN  + MENU_STRING_BASE
#define STR_FILE_CLOSE          IDM_FILE_CLOSE + MENU_STRING_BASE
#define STR_FILE_EXIT           IDM_FILE_EXIT  + MENU_STRING_BASE


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


// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NO_MFC                     1
#define _APS_NEXT_RESOURCE_VALUE        101
#define _APS_NEXT_COMMAND_VALUE         40002
#define _APS_NEXT_CONTROL_VALUE         1000
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif
