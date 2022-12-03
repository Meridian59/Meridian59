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

#ifdef M59_MSS
#include "mss.h"
#endif

void SoundInitialize(void);
void SoundClose(void);

DWORD PlayMidiFile(HWND hWndNotify, char *fname);
M59EXPORT UINT PlayWaveFile(HWND hwnd, char *fname, int volume, BYTE flags, int src_row, int src_col, int radius, int max_vol);

#ifdef M59_MSS
void AILCALLBACK SoundDoneCallback( HSAMPLE S );
void SoundDone(  HWND hwnd, WPARAM wParam, LPARAM lParam  );
static HDIGDRIVER WAVE_init_driver( DWORD rate, WORD bits, WORD chans );
#else
void SoundDone(HWND hwnd, int iChannel, LPMIXWAVE lpMixWave);
#endif

M59EXPORT void PlayWaveRsc(ID rsc, int volume, BYTE flags, int row, int col, int radius, int max_vol);
M59EXPORT void PlayMidiRsc(ID rsc);
M59EXPORT void PlayMusicRsc(ID rsc);

M59EXPORT void SoundAbort(void);
void NewSound(WPARAM type, ID rsc);
void UpdateLoopingSounds( int px, int py);

#endif /* #ifndef _SOUND_H */
