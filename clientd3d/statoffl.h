// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * offline.h:  Header file for offline.c
 */

#ifndef _OFFLINE_H
#define _OFFLINE_H

#define MODULE_OFFLINE_ID 1

#ifdef __cplusplus
extern "C" {
#endif

void OfflineInit(void);
void OfflineExit(void);
void OfflineCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void OfflineExpose(HWND hwnd);
void OfflineResize(int xsize, int ysize);
void OfflineSetFocus(HWND hwnd, HWND hwndOldFocus);

M59EXPORT void OfflineConnect(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _OFFLINE_H */
