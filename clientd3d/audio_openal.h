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

// Audio system initialization and shutdown
bool AudioInit(HWND hWnd);
void AudioShutdown(void);

// Music control (for background music via streaming playback)
bool MusicPlay(const std::string& filename, bool loop);
void MusicStop(void);
void MusicSetVolume(float volume); // 0.0 to 1.0
bool MusicIsPlaying(void);

// Returns true if sound started successfully; supports OGG and WAV formats,
// looping, and 3D positioning. Coordinates are in tile units.
// source_obj is the ID of the game object that emits the sound (0 = none).
// When non-zero, the source is registered for per-frame position updates.
bool SoundPlay(const char* filename, int volume, BYTE flags, 
               int src_row, int src_col, int radius, int max_vol,
               ID source_obj);

void SoundStopAll(void);

// Stop only looping sounds
void SoundStopLooping(void);

// Stop any playing sources that are using the given filename's buffer
void Audio_StopSourcesForFilename(const char* filename);

// Update positions of sources tracked to a moving game object.  Call once
// per frame.  Stopped sources and missing objects are pruned automatically.
void Audio_UpdateTrackedSources(void);

// Sets the listener's position and facing direction for 3D audio
void AudioUpdateListener(float x, float y, float z, 
                         float forwardX, float forwardY, float forwardZ);

#endif // _AUDIO_OPENAL_H
