// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * toolbar.h:  Header file for toolbar.c
 */

#ifndef _TOOLBAR_H
#define _TOOLBAR_H

#define TOOLBAR_X	10 + EDGETREAT_WIDTH
#define TOOLBAR_Y	2 + EDGETREAT_HEIGHT

#define TOOLBAR_BUTTON_WIDTH  24
#define TOOLBAR_BUTTON_HEIGHT 20

#define TOOLBAR_BUTTON_TOP_SPACING 0
#define TOOLBAR_BUTTON_LEFT_SPACING 0

#define TOOLBAR_SEPARATOR_WIDTH 5

// Structure to pass to ToolbarAddButton to create a button
typedef struct {
   int     action;      // Action to perform when button pressed
   void   *data;        // Data associated with action
   int     action2;     // Action to perform when button untoggled (A_NOACTION if not a toggle)
   void   *data2;       // Data associated with untoggle action
   HMODULE hModule;     // Module to load name resource string from
   int     bitmap_id;   // Resource identifier for bitmap to display in button; 0 for separator
   int     name;        // Resource identifier for name string (0 if none)
} AddButton;

// Data per toolbar button
typedef struct {
   int     index;        // Button number, starting from the left counting from 0
   HWND    hwnd;         // Window handle of button
   int     action;       // Action to do when button is clicked
   void   *action_data;  // Data for action
   int     action2;      // Action to perform when button untoggled (A_NOACTION if not a toggle)
   void   *action_data2; // Data associated with untoggle action
   BYTE   *bits;         // Pointer to bits of bitmap for button
   int     x;            // x position of button on main window
   Bool    pressed;      // True when button is pressed in (only applies if button is a toggle)
} Button;

void ToolbarCreate(void);
void ToolbarDestroy(void);
void ToolbarResize(int xsize, int ysize, AREA view);

M59EXPORT Bool __cdecl ToolbarAddButton(AddButton *s);
M59EXPORT Bool ToolbarSetButtonState(int action, void *action_data, Bool state);
M59EXPORT void ToolbarGetUnionRect(RECT* prcRect);

void ToolbarCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
Bool ToolbarDrawButton(HWND hwnd, const DRAWITEMSTRUCT *lpdis);

#endif /* #ifndef _TOOLBAR_H */
