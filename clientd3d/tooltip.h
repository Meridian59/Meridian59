// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * tooltip.h:  Header file for tooltip.c
 */

#ifndef _TOOLTIP_H
#define _TOOLTIP_H

void TooltipCreate(void);
void TooltipDestroy(void);
void TooltipsEnable(Bool enable);
M59EXPORT void TooltipForwardMessage(MSG *msg);

M59EXPORT void TooltipAddRectangle(HWND hwnd, RECT *rect, HINSTANCE hModule, int name);
M59EXPORT void TooltipAddWindow(HWND hwnd, HINSTANCE hModule, int name);
M59EXPORT void TooltipAddWindowCallback(HWND hwnd, HINSTANCE hModule);

HWND TooltipGetControl(void);
void TooltipReset(void);

#endif /* #ifndef _TOOLTIP_H */
