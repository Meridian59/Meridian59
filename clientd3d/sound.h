// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * sound.h:  Header file for sound.c
 */

#ifndef _SOUND_H
#define _SOUND_H

#include <string>

void SoundInitialize(void);

DWORD PlayMidiFile(HWND hWndNotify, char *fname);
M59EXPORT UINT PlayWaveFile(HWND hwnd, const char *fname, int volume,
                                     BYTE flags, int src_row, int src_col, int radius,
                                     int max_vol);

M59EXPORT void PlayWaveRsc(ID rsc, int volume, BYTE flags, int row, int col, int radius, int max_vol);
M59EXPORT void PlayMidiRsc(ID rsc);
M59EXPORT void PlayMusicRsc(ID rsc);

/* Looping sound transition helpers: call `Sound_BeginLoopingSoundTransition()`
   before loading a new room. As looping sounds are started they will be
   registered automatically. After room loading completes, call
   `Sound_EndLoopingSoundTransition()` to stop any looping sounds from the
   previous room that were not re-registered for the new room.
 */
void Sound_BeginLoopingSoundTransition(void);
void Sound_RegisterLoopingSound(const std::string &filename);
void Sound_EndLoopingSoundTransition(void);

M59EXPORT void SoundAbort(void);
void NewSound(WPARAM type, ID rsc);
void UpdateLoopingSounds( int px, int py);

#endif /* #ifndef _SOUND_H */
