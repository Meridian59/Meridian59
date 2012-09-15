// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * graphctl.h:  Header file for graphctl.c
 *
 * Styles:
 *
 * GCS_INPUT:      Accept mouse and keyboard input to change graph's value.
 * GCS_SLIDER:     Draw slider under graph
 * GCS_LIMITBAR:   Draw "limit bar", showing maximum attainable value of graph
 * GCS_NUMBER:     Draw number showing actual value of graph position
 *
 * Messages specific to graph custom control:
 * 
 * GRPH_RANGESET:  Sets range of graph.  New min is in wParam; new max is in lParam.
 *                 Must have max >= min.
 *                 Returns 0.
 * GRPH_MINGET:    Returns minimum of range of graph
 * GRPH_MAXGET:    Returns maximum of range of graph
 *
 * GRPH_POSSET:    Set current value of graph to lParam.
 *                 Returns old value, or -1 on error.
 *
 * GRPH_POSSETUSER: Internal message used when user wants to change graph's value. 
 *                 This message should be handled in subclassed controls to alter graph's behavior.
 *                 Set current value of graph to lParam.
 *                 Returns old value, or -1 on error.
 *
 * GRPH_POSGET:    Returns current value of graph.
 *
 * GRPH_LIMITSET:  Set current limit value of graph to lParam.
 *                 Returns old value, or -1 on error.
 *
 * GRPH_LIMITGET:  Returns current limit value of graph.
 *
 * GRPH_COLORGET:  Returns color corresponding to index in wParam.
 * GRPH_COLORSET:  Sets color corresponding to index in wParam to new color in lParam.
 *                 To use default color, pass -1 in lParam.  
 *                 Returns old color, or 0 on error (bad index). 
 *
 * Notification messages:
 *
 * GRPHN_POSCHANGING:  Value of graph given by wParam is about to change to LOWORD(lParam).
 */

#ifndef _GRAPHCTL_H
#define _GRAPHCTL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Styles */
#define GCS_INPUT    0x0001L    /* Include to get handle user input */
#define GCS_SLIDER   0x0002L    /* Include to display slider */
#define GCS_LIMITBAR 0x0004L    /* Include to display limit bar */
#define GCS_NUMBER   0x0008L    /* Include to display numeric value of bar graph */

/* Control-specific messages */
#define GRPH_COLORSET (WM_USER + 1000)
#define GRPH_COLORGET (WM_USER + 1001)
#define GRPH_RANGESET (WM_USER + 1002)
#define GRPH_POSSET   (WM_USER + 1004)
#define GRPH_POSGET   (WM_USER + 1005)
#define GRPH_POSSETUSER    (WM_USER + 1006)
#define GRPHN_POSCHANGING  (WM_USER + 1007)
#define GRPH_MINGET (WM_USER + 1008)
#define GRPH_MAXGET (WM_USER + 1009)
#define GRPH_LIMITSET   (WM_USER + 1010)
#define GRPH_LIMITGET   (WM_USER + 1011)

/* Color constants */
enum { 
   GRAPHCOLOR_FRAME = 0,   /* Graph frame */
   GRAPHCOLOR_BAR,         /* Filled area of graph */
   GRAPHCOLOR_BKGND,       /* Unfilled area of graph */
   GRAPHCOLOR_SLIDER,      /* Slider under graph */
   GRAPHCOLOR_SLIDERBKGND, /* Background around slider under graph */
   GRAPHCOLOR_LIMITBAR,    /* Limit bar color */
   GRAPH_NUMCOLORS,
};

Bool GraphCtlRegister(HINSTANCE hInst);
Bool GraphCtlUnregister(HINSTANCE hInst);
M59EXPORT char *GraphCtlGetClassName(void);
M59EXPORT long CALLBACK GraphCtlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus
};
#endif

#endif /* #ifndef _GRAPHCTL_H */
