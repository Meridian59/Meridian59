// Meridian 59, Copyright 1994-2025 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.

/*
 * audio_openal.h: OpenAL Soft audio engine for Meridian 59
 */

#ifndef _AUDIO_OPENAL_H
#define _AUDIO_OPENAL_H

#ifdef __cplusplus
extern "C" {
#endif

// Audio system initialization and shutdown
BOOL AudioInit(HWND hWnd);
void AudioShutdown(void);

// Music control (for MP3/OGG background music)
BOOL MusicPlay(const char* filename, BOOL loop);
void MusicStop(void);
void MusicSetVolume(float volume); // 0.0 to 1.0
BOOL MusicIsPlaying(void);

// WAV sound playback (matches PlayWaveFile signature)
BOOL SoundPlayWave(const char* filename, int volume, BYTE flags, 
                   int src_row, int src_col, int radius, int max_vol);

// Stop all sounds
void SoundStopAll(void);

// Listener position update for 3D audio (called each frame)
void AudioUpdateListener(float x, float y, float z, 
                         float forwardX, float forwardY, float forwardZ);

#ifdef __cplusplus
}
#endif

#endif // _AUDIO_OPENAL_H
