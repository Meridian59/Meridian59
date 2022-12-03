// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * music.h:  Header file for music.c
 */

#ifndef _MUSIC_H
#define _MUSIC_H

#ifdef M59_MSS
#include "mss.h"
#endif

void MusicInitialize(void);
void MusicClose(void);

DWORD PlayMidiFile(HWND hWndNotify, char *fname);
M59EXPORT DWORD PlayMusicFile(HWND hWndNotify, const char *fname);
M59EXPORT void PlayMidiRsc(ID rsc);
M59EXPORT void PlayMusicRsc(ID rsc);

#ifdef M59_MSS
void AILEXPORT MIDIDoneCallback(HSAMPLE S);
#else
void MusicDone(UINT device);
#endif

M59EXPORT void MusicAbort(void);
void MusicStart(void);
void NewMusic(WPARAM type, ID rsc);
void ResetMusicVolume();

#endif /* #ifndef _MUSIC_H */
