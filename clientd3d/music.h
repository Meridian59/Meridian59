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

/* Legacy MSS include removed — MSS-specific types are hidden behind
	`M59_MSS` when present. Use neutral prototypes so headers don't
	expose MSS/wavemix types to new contributors. */

void MusicInitialize(void);
void MusicClose(void);

DWORD PlayMidiFile(HWND hWndNotify, char *fname);
M59EXPORT DWORD PlayMusicFile(HWND hWndNotify, const char *fname);
M59EXPORT void PlayMidiRsc(ID rsc);
M59EXPORT void PlayMusicRsc(ID rsc);

/* Legacy MSS callback (takes MSS HSAMPLE); use `void *` here so the
	header doesn't require `mss.h`. The implementation may cast the
	pointer if needed, but the header never exposes MSS types. */
void MIDIDoneCallback(void *S);
void MusicDone(UINT device);

M59EXPORT void MusicAbort(void);
void MusicStart(void);
void NewMusic(WPARAM type, ID rsc);
void ResetMusicVolume();

#endif /* #ifndef _MUSIC_H */
