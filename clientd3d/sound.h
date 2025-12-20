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

M59EXPORT bool PlayWaveFile(HWND hwnd, const char *fname, int volume,
                                     BYTE flags, int src_row, int src_col, int radius,
                                     int max_vol);

M59EXPORT void PlayWaveRsc(ID rsc, int volume, BYTE flags, int row, int col, int radius, int max_vol);

void Sound_BeginLoopingSoundTransition(void);
void Sound_RegisterLoopingSound(const std::string &filename);
void Sound_EndLoopingSoundTransition(void);

M59EXPORT void SoundAbort(void);
void NewSound(WPARAM type, ID rsc);
void UpdateLoopingSounds(int px, int py, int angle);

#endif /* #ifndef _SOUND_H */
