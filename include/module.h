// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * module.h:  Header file common to all client modules, and to client itself.
 */

#ifndef _MODULE_H
#define _MODULE_H

/********************************************************************/
/* 
 * Definitions for module interface
 */
/********************************************************************/
// Client data sent to modules
typedef struct {
   HWND         hMain;                        // Handle of main window
   Config      *config;
   room_type   *current_room;
   player_info *player;
   HWND        *hCurrentDlg;                  // Handle of active modeless dialog
   HINSTANCE    hInst;                        // Instance handle of client
   char        *ini_file;                     // Name of our private configuration file
   list_type   *current_users;                // List of users currently in game
   HMENU        main_menu;                    // Handle of main window menu
   Effects     *effects;                      // Special effects status
   HPALETTE     hPal;                         // Main palette
   HWND         hToolTips;                    // Tooltip control
   char        *szAppName;                    // Program name
   Bool        *map;                          // True when map is being drawn (instead of 3D view)
   Bool        *first_load;                   // True when the FIRST module is loaded
   BYTE        (*light_palettes)[NUM_PALETTES][NUM_COLORS]; // Palettes at different light levels
   DWORD       *latency;                      // Server latency in milliseconds
} ClientInfo;

// Info kept on each loaded module
typedef struct {
   HMODULE      handle;
   int         event_mask;   // Events for which module wants notifications
   int         priority;     // Order in which modules called; 1 = first
   ID          rsc;          // Resource # of filename, 0 if none
   int         module_id;    // Unique module identifier
} ModuleInfo;

// Ordinal values of functions exported from DLL
enum {
   MODULE_GETMODULEINFO = 1,
   MODULE_SERVERMESSAGE = 2,
   MODULE_KEYPRESS      = 3,
   MODULE_USERACTION    = 4,
   MODULE_MOUSECLICK    = 5,
   MODULE_FONTCHANGED   = 6,
   MODULE_COLORCHANGED  = 7,
   MODULE_MENUITEM      = 8,
   MODULE_MODULEMESSAGE = 10,
   MODULE_STATECHANGED  = 11,
   MODULE_WINDOWMSG     = 12,
   MODULE_ANIMATE       = 13,
   MODULE_TOOLBUTTON    = 14,
   MODULE_EXIT          = 16,
   MODULE_TEXTCOMMAND   = 17,
   MODULE_RESIZE        = 18,
   MODULE_USERCHANGED   = 19,
   MODULE_REDRAW        = 20,
   MODULE_DRAWITEM      = 21,
   MODULE_RESETDATA     = 22,
   MODULE_INVENTORY     = 23,
   MODULE_SETCURSOR     = 24,
   MODULE_NEWROOM       = 25,
   MODULE_SETFOCUS      = 26,
   MODULE_CONFIGCHANGED = 27,
};

// Event types--each module can request notification for any of these types
#define EVENT_SERVERMSG     0x00000001       // Message arrived from server
#define EVENT_KEY           0x00000002       // Key pressed
#define EVENT_USERACTION    0x00000004       // User is doing a game action
#define EVENT_MOUSECLICK    0x00000008       // Left mouse button clicked
#define EVENT_FONTCHANGED   0x00000010       // User changed a font
#define EVENT_COLORCHANGED  0x00000020       // User changed a color
#define EVENT_MENUITEM      0x00000040       // Menu item selected
#define EVENT_MODULEMSG     0x00000100       // Message sent from one module to another
#define EVENT_STATECHANGED  0x00000200       // Game state has changed
#define EVENT_WINDOWMSG     0x00000400       // Main window received a Windows message
#define EVENT_ANIMATE       0x00000800       // Animation timer has gone off
#define EVENT_TOOLBUTTON    0x00001000       // Toolbar button was pressed
#define EVENT_TEXTCOMMAND   0x00002000       // Text command was entered
#define EVENT_RESIZE        0x00004000       // Main window was resized
#define EVENT_USERCHANGED   0x00008000       // Player's graphic changed
#define EVENT_REDRAW        0x00010000       // Entire interface is being redrawn
#define EVENT_DRAWITEM      0x00020000       // Processing a WM_DRAWITEM message
#define EVENT_RESETDATA     0x00040000       // Asking for all new data from server
#define EVENT_INVENTORY     0x00080000       // Player's inventory changed
#define EVENT_SETCURSOR     0x00100000       // Main window cursor needs to be set
#define EVENT_NEWROOM       0x00200000       // Player has entered new room
#define EVENT_SETFOCUS      0x00400000       // Main window got focus
#define EVENT_CONFIGCHANGED 0x00800000       // User changed preferences

// Priority levels -- determines order of handling events among modules
#define PRIORITY_FIRST   1  
#define PRIORITY_HIGH    5
#define PRIORITY_NORMAL  10
#define PRIORITY_LOW     20
#define PRIORITY_IGNORE  0       // Means that module is never called


// Sub-commands for EVENT_INVENTORY (command is first parameter to handler, data is second)
#define INVENTORY_SET     1       // data is NULL; player.inventory has new items
#define INVENTORY_ADD     2       // data is object_node of new object
#define INVENTORY_REMOVE  3       // data is ID of object removed
#define INVENTORY_USE     4       // data is ID of object used
#define INVENTORY_UNUSE   5       // data is ID of object unused
#define INVENTORY_CHANGE  6       // data is ID of object changed
#define INVENTORY_USELIST 7       // data is list of object_nodes of objects in use

#endif /* #ifndef _MODULE_H */
