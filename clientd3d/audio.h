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

#define VOLUME_CUTOFF_DISTANCE 16

#include <irrKlang.h>

/***************** General *******************/

BOOL AudioInit();
BOOL AudioShutdown();

/***************** Music *********************/

M59EXPORT BOOL MusicStop();
M59EXPORT BOOL MusicRestart();
M59EXPORT BOOL MusicPlayFile(char* file);
M59EXPORT BOOL MusicPlayResource(ID rsc);
M59EXPORT BOOL MusicSetVolume();

/***************** Sound *********************/

M59EXPORT BOOL SoundStop(irrklang::ISound* soundid);
M59EXPORT BOOL SoundStopAll(BYTE flags = 0);
M59EXPORT BOOL SoundPlayFile(char* file, BYTE flags, int x = 0, int y = 0);
M59EXPORT BOOL SoundStopFile(char* file);
M59EXPORT BOOL SoundPlayResource(ID rsc, BYTE flags, int x = 0, int y = 0);
M59EXPORT BOOL SoundStopResource(ID rsc, ID obj);
M59EXPORT BOOL SoundSetVolume();
M59EXPORT BOOL SoundSetListenerPosition(int x, int y, int angle);

#endif /* #ifndef _AUDIO_H */
