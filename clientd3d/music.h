// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * music.h: Music playback API
 *
 * Public API (exported to DLLs):
 *   PlayMusicFile  - Play music by filename (.ogg, auto-converts .mid/.mp3)
 *   PlayMusicRsc   - Play music by resource ID
 *   MusicAbort     - Stop all music
 *
 * Internal:
 *   MusicClose     - Shutdown
 *   MusicRestart   - Resume music when user re-enables in settings
 *   NewMusic       - Handle server music message
 *   ResetMusicVolume - Apply config volume
 */

#ifndef _MUSIC_H
#define _MUSIC_H

void MusicClose(void);

M59EXPORT bool PlayMusicFile(HWND hWndNotify, const char *fname);
M59EXPORT void PlayMusicRsc(ID rsc);
M59EXPORT void MusicAbort(void);
void MusicRestart(void);
void NewMusic(WPARAM type, ID rsc);
void ResetMusicVolume();

#endif /* #ifndef _MUSIC_H */
