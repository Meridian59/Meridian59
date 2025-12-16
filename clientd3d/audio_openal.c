// Meridian 59, Copyright 1994-2025 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.

/*
 * audio_openal.c: OpenAL Soft audio engine implementation for Meridian 59
 */

#include "client.h"
#include "audio_openal.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/efx.h>

// Configuration
#define MAX_AUDIO_SOURCES 32
#define MAX_AUDIO_BUFFERS 256

// Global state
static ALCdevice* g_device = NULL;
static ALCcontext* g_context = NULL;
static ALuint g_sources[MAX_AUDIO_SOURCES];
static ALuint g_buffers[MAX_AUDIO_BUFFERS];
static int g_numSources = 0;
static int g_numBuffers = 0;
static BOOL g_initialized = FALSE;

// Music state
static ALuint g_musicSource = 0;
static ALuint g_musicBuffer = 0;
static BOOL g_musicPlaying = FALSE;

// Master volume
static float g_masterVolume = 1.0f;
static float g_musicVolume = 1.0f;

/*
 * AudioInit: Initialize OpenAL audio system
 */
BOOL AudioInit(HWND hWnd)
{
   if (g_initialized)
   {
      debug(("AudioInit: Already initialized\n"));
      return TRUE;
   }

   // Open default audio device
   g_device = alcOpenDevice(NULL);
   if (!g_device)
   {
      debug(("AudioInit: Failed to open audio device\n"));
      return FALSE;
   }

   // Create audio context
   g_context = alcCreateContext(g_device, NULL);
   if (!g_context)
   {
      debug(("AudioInit: Failed to create audio context\n"));
      alcCloseDevice(g_device);
      g_device = NULL;
      return FALSE;
   }

   // Make context current
   if (!alcMakeContextCurrent(g_context))
   {
      debug(("AudioInit: Failed to make context current\n"));
      alcDestroyContext(g_context);
      alcCloseDevice(g_device);
      g_context = NULL;
      g_device = NULL;
      return FALSE;
   }

   // Generate audio sources
   alGenSources(MAX_AUDIO_SOURCES, g_sources);
   if (alGetError() != AL_NO_ERROR)
   {
      debug(("AudioInit: Failed to generate audio sources\n"));
      alcMakeContextCurrent(NULL);
      alcDestroyContext(g_context);
      alcCloseDevice(g_device);
      g_context = NULL;
      g_device = NULL;
      return FALSE;
   }
   g_numSources = MAX_AUDIO_SOURCES;

   // Create dedicated music source
   alGenSources(1, &g_musicSource);
   if (alGetError() != AL_NO_ERROR)
   {
      debug(("AudioInit: Failed to generate music source\n"));
   }

   // Set default listener properties
   alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
   alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
   ALfloat listenerOri[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f }; // forward, up
   alListenerfv(AL_ORIENTATION, listenerOri);

   g_initialized = TRUE;
   debug(("AudioInit: OpenAL initialized successfully\n"));
   
   return TRUE;
}

/*
 * AudioShutdown: Clean up audio system
 */
void AudioShutdown(void)
{
   if (!g_initialized)
      return;

   // Stop all sounds
   SoundStopAll();
   MusicStop();

   // Delete music source
   if (g_musicSource)
   {
      alDeleteSources(1, &g_musicSource);
      g_musicSource = 0;
   }

   // Delete sound sources
   if (g_numSources > 0)
   {
      alDeleteSources(g_numSources, g_sources);
      g_numSources = 0;
   }

   // Delete buffers
   if (g_numBuffers > 0)
   {
      alDeleteBuffers(g_numBuffers, g_buffers);
      g_numBuffers = 0;
   }

   // Destroy context
   if (g_context)
   {
      alcMakeContextCurrent(NULL);
      alcDestroyContext(g_context);
      g_context = NULL;
   }

   // Close device
   if (g_device)
   {
      alcCloseDevice(g_device);
      g_device = NULL;
   }

   g_initialized = FALSE;
   debug(("AudioShutdown: OpenAL shut down\n"));
}

/*
 * MusicPlay: Play background music file
 */
BOOL MusicPlay(const char* filename, BOOL loop)
{
   // TODO: Implement music playback
   debug(("MusicPlay: %s (loop=%d)\n", filename, loop));
   return FALSE;
}

/*
 * MusicStop: Stop background music
 */
void MusicStop(void)
{
   if (!g_initialized || !g_musicSource)
      return;

   alSourceStop(g_musicSource);
   g_musicPlaying = FALSE;
}

/*
 * MusicSetVolume: Set music volume
 */
void MusicSetVolume(float volume)
{
   g_musicVolume = volume;
   if (g_initialized && g_musicSource)
   {
      alSourcef(g_musicSource, AL_GAIN, volume * g_masterVolume);
   }
}

/*
 * MusicIsPlaying: Check if music is playing
 */
BOOL MusicIsPlaying(void)
{
   return g_musicPlaying;
}

/*
 * SoundPlayWave: Play WAV file with 3D positioning
 * Matches PlayWaveFile signature from sound.c
 */
BOOL SoundPlayWave(const char* filename, int volume, BYTE flags,
                   int src_row, int src_col, int radius, int max_vol)
{
   if (!g_initialized)
      return FALSE;

   // TODO: Implement WAV playback
   // - Load WAV file
   // - Find available source
   // - Set volume, looping, 3D position
   // - Play
   debug(("SoundPlayWave: %s (vol=%d, flags=%d)\n", filename, volume, flags));
   return FALSE;
}

/*
 * AudioUpdateListener: Update listener position and orientation for 3D audio
 * Should be called each frame with player position and camera direction
 */
void AudioUpdateListener(float x, float y, float z,
                         float forwardX, float forwardY, float forwardZ)
{
   if (!g_initialized)
      return;

   // Update position
   alListener3f(AL_POSITION, x, y, z);

   // Update orientation (forward + up vectors)
   ALfloat ori[] = { forwardX, forwardY, forwardZ, 0.0f, 1.0f, 0.0f };
   alListenerfv(AL_ORIENTATION, ori);
}

/*
 * SoundStopAll: Stop all playing sounds
 */
void SoundStopAll(void)
{
   if (!g_initialized)
      return;

   for (int i = 0; i < g_numSources; i++)
   {
      alSourceStop(g_sources[i]);
   }
}


