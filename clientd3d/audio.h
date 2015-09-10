// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* audio.h
*
*/

#ifndef _AUDIO_H
#define _AUDIO_H

/***************** Music *********************/

M59EXPORT BOOL MusicStop(); 
M59EXPORT BOOL MusicPlayFile(char* file);
M59EXPORT BOOL MusicPlayResource(ID rsc);
M59EXPORT BOOL MusicSetVolume();

/***************** Sound *********************/

M59EXPORT BOOL SoundStop(MCIDEVICEID soundid);
M59EXPORT BOOL SoundStopAll(BYTE flags = 0);
M59EXPORT BOOL SoundPlayFile(char* file, int volume, BYTE flags);
M59EXPORT BOOL SoundPlayResource(ID rsc, int volume, BYTE flags);
M59EXPORT BOOL SoundSetListenerPosition(int x, int y);

#endif /* #ifndef _AUDIO_H */
