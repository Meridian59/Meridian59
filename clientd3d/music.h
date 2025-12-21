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

void MusicInitialize(void);
void MusicClose(void);

/* Legacy compatibility shims for old protocol */
bool PlayMidiFile(HWND hWndNotify, char *fname);
M59EXPORT bool PlayMusicFile(HWND hWndNotify, const char *fname);
M59EXPORT void PlayMusicRsc(ID rsc);

void MusicDone(UINT device);

M59EXPORT void MusicAbort(void);
void MusicStart(void);
void NewMusic(WPARAM type, ID rsc);
void ResetMusicVolume();

#endif /* #ifndef _MUSIC_H */
