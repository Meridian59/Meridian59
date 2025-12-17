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

void SoundInitialize(void);

DWORD PlayMidiFile(HWND hWndNotify, char *fname);
M59EXPORT UINT PlayWaveFile(HWND hwnd, const char *fname, int volume,
                                     BYTE flags, int src_row, int src_col, int radius,
                                     int max_vol);

/* SoundDone is invoked via the MM_WOM_DONE message; with OpenAL this is
   effectively a no-op. The third parameter comes from the window message
   LPARAM, so use `LPARAM` here to match call sites. */
void SoundDone(HWND hwnd, int iChannel, LPARAM lpWave);

M59EXPORT void PlayWaveRsc(ID rsc, int volume, BYTE flags, int row, int col, int radius, int max_vol);
M59EXPORT void PlayMidiRsc(ID rsc);
M59EXPORT void PlayMusicRsc(ID rsc);

/* Ambient transition helpers: call `Sound_BeginAmbientTransition()` before
    loading a new room (or when music/room transition starts). As looping
    ambients are started they should be registered via
    `Sound_RegisterAmbientFilenameWithCategory()` so they are protected from
    cleanup. After the room's ambients have been started, call
    `Sound_EndAmbientTransition()` to stop any ambients that were present
    previously but not re-registered.
 */
typedef enum {
      SOUND_CAT_ACTION = 0,  // one-shot action sounds
      SOUND_CAT_MUSIC,       // background music (OGG)
      SOUND_CAT_STEADY,      // steady sounds like fountains
      SOUND_CAT_ATMOS,       // atmospheric sounds like birds
      SOUND_CAT_UI           // UI/global FX
} SoundCategory;

void Sound_BeginAmbientTransition(void);
void Sound_RegisterAmbientFilenameWithCategory(const char *filename, SoundCategory cat);
/* Convenience: register without category (heuristic). */
void Sound_RegisterAmbientFilename(const char *filename);
void Sound_EndAmbientTransition(void);

M59EXPORT void SoundAbort(void);
void NewSound(WPARAM type, ID rsc);
void UpdateLoopingSounds( int px, int py);

#endif /* #ifndef _SOUND_H */
