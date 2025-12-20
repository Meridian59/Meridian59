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
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/efx.h>

// STB Vorbis for OGG decoding (compiled separately with STB_VORBIS_NO_PUSHDATA_API)
extern "C" int stb_vorbis_decode_filename(const char *filename, int *channels, int *sample_rate, short **output);

// Configuration
static const int MAX_AUDIO_SOURCES = 32;
static const int MAX_AUDIO_BUFFERS = 256;

// Global state
static ALCdevice* g_device = NULL;
static ALCcontext* g_context = NULL;
static ALuint g_sources[MAX_AUDIO_SOURCES];
static ALuint g_buffers[MAX_AUDIO_BUFFERS];
static int g_numSources = 0;
static int g_numBuffers = 0;
static bool g_initialized = false;

// Buffer cache for loaded WAV files
struct BufferCacheEntry {
   char filename[MAX_PATH];
   ALuint buffer;
};

static BufferCacheEntry g_bufferCache[MAX_AUDIO_BUFFERS];
static int g_cacheCount = 0;

// Music state
static ALuint g_musicSource = 0;
static ALuint g_musicBuffer = 0;
static bool g_musicPlaying = false;

// Master volume
static float g_masterVolume = 1.0f;
static float g_musicVolume = 1.0f;

/*
 * AudioInit: Initialize OpenAL audio system
 */
bool AudioInit(HWND hWnd)
{
   if (g_initialized)
      return true;

   // Open default audio device
   g_device = alcOpenDevice(NULL);
   if (!g_device)
   {
      debug(("AudioInit: Failed to open audio device\n"));
      return false;
   }

   // Create audio context
   g_context = alcCreateContext(g_device, NULL);
   if (!g_context)
   {
      debug(("AudioInit: Failed to create audio context\n"));
      alcCloseDevice(g_device);
      g_device = NULL;
      return false;
   }

   // Make context current
   if (!alcMakeContextCurrent(g_context))
   {
      debug(("AudioInit: Failed to make context current\n"));
      alcDestroyContext(g_context);
      alcCloseDevice(g_device);
      g_context = NULL;
      g_device = NULL;
      return false;
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
      return false;
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

   // Use linear distance model for predictable 3D audio falloff
   alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);

   g_initialized = true;
   debug(("AudioInit: OpenAL initialized successfully\n"));
   
   return true;
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

   g_initialized = false;
   debug(("AudioShutdown: OpenAL shut down\n"));
}

/*
 * LoadOGGFile: Returns an OpenAL buffer ID for the decoded OGG file, or 0 on failure.
 * Decodes an OGG Vorbis file via stb_vorbis and uploads to OpenAL.
 * Note: OGG buffers are NOT cached because music playback deletes buffers
 * after use. Sound effects use WAV format which is cached.
 */
static ALuint LoadOGGFile(const char* filename)
{
   ALuint buffer = 0;
   int channels, sample_rate;
   short* decoded;
   int num_samples;
   ALenum format;

   debug(("LoadOGGFile: Attempting to load %s\n", filename));

   // Check if file exists
   FILE* test_file = NULL;
   if (fopen_s(&test_file, filename, "rb") != 0 || !test_file)
   {
      debug(("LoadOGGFile: File not found or cannot open: %s\n", filename));
      return 0;
   }
   fclose(test_file);

   // Decode entire OGG file
   num_samples = stb_vorbis_decode_filename(filename, &channels, &sample_rate, &decoded);

   debug(("LoadOGGFile: stb_vorbis_decode_filename returned %d samples\n", num_samples));

   if (num_samples <= 0)
   {
      debug(("LoadOGGFile: Failed to decode %s (error code: %d)\n", filename, num_samples));
      return 0;
   }

   debug(("LoadOGGFile: Decoded %s - %d samples, %d channels, %d Hz\n", 
          filename, num_samples, channels, sample_rate));

   // Determine OpenAL format
   if (channels == 1)
      format = AL_FORMAT_MONO16;
   else if (channels == 2)
      format = AL_FORMAT_STEREO16;
   else
   {
      debug(("LoadOGGFile: Unsupported channel count %d\n", channels));
      free(decoded);
      return 0;
   }

   // Create OpenAL buffer
   alGenBuffers(1, &buffer);
   if (alGetError() != AL_NO_ERROR)
   {
      debug(("LoadOGGFile: Failed to create buffer\n"));
      free(decoded);
      return 0;
   }

   // Upload PCM data to buffer
   alBufferData(buffer, format, decoded, num_samples * channels * sizeof(short), sample_rate);
   free(decoded);

   if (alGetError() != AL_NO_ERROR)
   {
      debug(("LoadOGGFile: Failed to upload buffer data\n"));
      alDeleteBuffers(1, &buffer);
      return 0;
   }

   debug(("LoadOGGFile [OpenAL]: %s OK\n", filename));
   return buffer;
}

/*
 * MusicPlay: Play background music file
 */
bool MusicPlay(const char* filename, bool loop)
{
   ALuint newBuffer;
   ALint sourceState;

   if (!g_initialized)
   {
      debug(("MusicPlay: OpenAL not initialized!\n"));
      return false;
   }

   debug(("MusicPlay: %s (loop=%d)\n", filename, loop));

   // Always stop any currently playing music unconditionally
   // This ensures we stop even if our flag got out of sync with OpenAL state
   alSourceStop(g_musicSource);
   alSourcei(g_musicSource, AL_BUFFER, 0);  // Detach buffer

   // Verify the source actually stopped (defensive check)
   alGetSourcei(g_musicSource, AL_SOURCE_STATE, &sourceState);
   if (sourceState == AL_PLAYING)
   {
      debug(("MusicPlay: Warning - source still playing after stop!\n"));
   }

   // Delete old buffer if we had one (now safe since source is stopped and detached)
   if (g_musicBuffer != 0)
   {
      alDeleteBuffers(1, &g_musicBuffer);
      g_musicBuffer = 0;
   }

   g_musicPlaying = false;

   // Load OGG file (filename already includes path)
   newBuffer = LoadOGGFile(filename);
   if (newBuffer == 0)
   {
      debug(("MusicPlay: Failed to load %s\n", filename));
      return false;
   }

   g_musicBuffer = newBuffer;

   // Attach buffer to music source
   alSourcei(g_musicSource, AL_BUFFER, g_musicBuffer);
   alSourcei(g_musicSource, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
   alSourcei(g_musicSource, AL_SOURCE_RELATIVE, AL_TRUE);
   alSource3f(g_musicSource, AL_POSITION, 0.0f, 0.0f, 0.0f);
   alSourcef(g_musicSource, AL_GAIN, (float)config.music_volume / 100.0f);

   // Start playing
   alSourcePlay(g_musicSource);

   if (alGetError() != AL_NO_ERROR)
   {
      debug(("MusicPlay: Failed to play music\n"));
      return false;
   }

   g_musicPlaying = true;
   debug(("MusicPlay: Now playing %s\n", filename));
   return true;
}

/*
 * MusicStop: Stop background music
 */
void MusicStop(void)
{
   if (!g_initialized || !g_musicSource)
      return;

   alSourceStop(g_musicSource);
   // Detach buffer from source before any future buffer deletion
   alSourcei(g_musicSource, AL_BUFFER, 0);
   g_musicPlaying = false;
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
bool MusicIsPlaying(void)
{
   return g_musicPlaying;
}

/*
 * LoadWAVFile: Load a WAV file into an OpenAL buffer
 * Returns buffer ID on success, 0 on failure
 * Handles non-standard WAV files with extra chunks before fmt
 */
static ALuint LoadWAVFile(const char* filename)
{
   FILE* file;
   char chunkID[4];
   DWORD chunkSize, riffSize;
   WORD audioFormat = 0, numChannels = 0, bitsPerSample = 0;
   DWORD sampleRate = 0;
   BYTE* data = NULL;
   DWORD dataSize = 0;
   ALuint buffer = 0;
   ALenum format;
   bool foundFmt = false;
   bool foundData = false;
   long riffEnd;

   // Check cache first
   for (int i = 0; i < g_cacheCount; i++)
   {
      if (_stricmp(g_bufferCache[i].filename, filename) == 0)
      {
         return g_bufferCache[i].buffer;
      }
   }

   // Open file
   if (fopen_s(&file, filename, "rb") != 0 || file == NULL)
   {
      debug(("LoadWAVFile: Cannot open %s\n", filename));
      return 0;
   }

   // Read RIFF header
   if (fread(chunkID, 1, 4, file) != 4 || memcmp(chunkID, "RIFF", 4) != 0)
   {
      debug(("LoadWAVFile: Not a WAV file: %s\n", filename));
      fclose(file);
      return 0;
   }

   if (fread(&riffSize, 4, 1, file) != 1)
   {
      debug(("LoadWAVFile: Truncated header: %s\n", filename));
      fclose(file);
      return 0;
   }
   riffEnd = 8 + riffSize;  // Position where RIFF chunk ends

   if (fread(chunkID, 1, 4, file) != 4 || memcmp(chunkID, "WAVE", 4) != 0)
   {
      debug(("LoadWAVFile: Not a WAV file: %s\n", filename));
      fclose(file);
      return 0;
   }

   // Search for fmt and data chunks
   while (!foundData && ftell(file) < riffEnd)
   {
      if (fread(chunkID, 1, 4, file) != 4)
         break;
      if (fread(&chunkSize, 4, 1, file) != 1)
         break;

      if (memcmp(chunkID, "fmt ", 4) == 0)
      {
         // Read format chunk
         WORD blockAlign;
         DWORD byteRate;
         if (chunkSize < 16)
         {
            debug(("LoadWAVFile: fmt chunk too small: %s\n", filename));
            break;
         }
         fread(&audioFormat, 2, 1, file);
         fread(&numChannels, 2, 1, file);
         fread(&sampleRate, 4, 1, file);
         fread(&byteRate, 4, 1, file);
         fread(&blockAlign, 2, 1, file);
         fread(&bitsPerSample, 2, 1, file);
         // Skip any extra format bytes
         if (chunkSize > 16)
            fseek(file, chunkSize - 16, SEEK_CUR);
         foundFmt = true;
      }
      else if (memcmp(chunkID, "data", 4) == 0)
      {
         if (!foundFmt)
         {
            debug(("LoadWAVFile: data chunk before fmt chunk: %s\n", filename));
            break;
         }
         dataSize = chunkSize;
         data = (BYTE*)malloc(dataSize);
         if (data && fread(data, 1, dataSize, file) == dataSize)
            foundData = true;
         else
         {
            free(data);
            data = NULL;
         }
         break;
      }
      else
      {
         // Skip unknown chunk (JUNK, LIST, bext, etc.)
         fseek(file, chunkSize, SEEK_CUR);
      }
   }

   fclose(file);

   if (!foundFmt)
   {
      debug(("LoadWAVFile: Missing fmt chunk: %s\n", filename));
      return 0;
   }

   if (!data)
   {
      debug(("LoadWAVFile: No data chunk: %s\n", filename));
      return 0;
   }

   // Determine OpenAL format
   if (numChannels == 1)
      format = (bitsPerSample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
   else
      format = (bitsPerSample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;

   // Create OpenAL buffer
   alGenBuffers(1, &buffer);
   if (alGetError() != AL_NO_ERROR)
   {
      debug(("LoadWAVFile: Failed to generate buffer\n"));
      free(data);
      return 0;
   }

   alBufferData(buffer, format, data, dataSize, sampleRate);
   free(data);

   if (alGetError() != AL_NO_ERROR)
   {
      debug(("LoadWAVFile: Failed to buffer data\n"));
      alDeleteBuffers(1, &buffer);
      return 0;
   }

   // Add to cache
   if (g_cacheCount < MAX_AUDIO_BUFFERS)
   {
      strncpy_s(g_bufferCache[g_cacheCount].filename, MAX_PATH, filename, _TRUNCATE);
      g_bufferCache[g_cacheCount].buffer = buffer;
      g_cacheCount++;
   }

   debug(("LoadWAVFile [OpenAL]: %s OK\n", filename));
   return buffer;
}

/*
 * SoundPlay: Returns true if the sound was successfully started.
 * Plays a sound file via OpenAL with volume control, optional looping,
 * and 3D positioning based on row/col coordinates.
 * Supports OGG and WAV formats (tries OGG first for better quality/compression).
 */
bool SoundPlay(const char* filename, int volume, BYTE flags,
               int src_row, int src_col, int radius, int max_vol)
{
   if (!g_initialized)
      return false;

   // Quick configuration escape for looping and random sounds
   if ((flags & SF_LOOP) && (!config.play_loop_sounds))
      return true;
   if ((flags & SF_RANDOM_PLACE) && (!config.play_random_sounds))
      return true;

   // Dispatch to appropriate loader based on file extension
   ALuint buffer = 0;
   const char* ext = strrchr(filename, '.');
   if (ext && (_stricmp(ext, ".ogg") == 0))
      buffer = LoadOGGFile(filename);
   else
      buffer = LoadWAVFile(filename);
   if (buffer == 0)
      return false;

   // For looping sounds, check if this buffer is already playing to avoid duplicates
   if (flags & SF_LOOP)
   {
      for (int i = 0; i < g_numSources; i++)
      {
         ALint state;
         alGetSourcei(g_sources[i], AL_SOURCE_STATE, &state);
         if (state == AL_PLAYING || state == AL_PAUSED)
         {
            ALint sourceBuffer;
            alGetSourcei(g_sources[i], AL_BUFFER, &sourceBuffer);
            if (sourceBuffer == (ALint)buffer)
            {
               // Already playing this looping sound - no need to restart
               // OpenAL handles distance attenuation automatically via listener position
               return true;
            }
         }
      }
   }

   // Find available source
   int sourceIndex = -1;
   for (int i = 0; i < g_numSources; i++)
   {
      ALint state;
      alGetSourcei(g_sources[i], AL_SOURCE_STATE, &state);
      if (state != AL_PLAYING && state != AL_PAUSED)
      {
         sourceIndex = i;
         break;
      }
   }

   if (sourceIndex == -1)
   {
      debug(("SoundPlay: No available sources\n"));
      return false;
   }

   ALuint source = g_sources[sourceIndex];

   // Attach buffer
   alSourcei(source, AL_BUFFER, buffer);

   // Determine if this is a positional sound (has valid position coordinates)
   bool isPositional = (flags & SF_LOOP) && (src_row > 0 || src_col > 0);

   if (isPositional)
   {
      // 3D positional sound - negate X to convert game coords to OpenAL coords
      alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);
      alSource3f(source, AL_POSITION, -(float)src_col, 0.0f, (float)src_row);

      // Distance attenuation: full volume within 1 tile, silent at radius
      alSourcef(source, AL_REFERENCE_DISTANCE, 1.0f);
      alSourcef(source, AL_MAX_DISTANCE, (float)radius);
      alSourcef(source, AL_ROLLOFF_FACTOR, 1.0f);

      float gain = (float)max_vol / (float)MAX_VOLUME;
      gain *= (float)config.ambient_volume / 100.0f;
      alSourcef(source, AL_GAIN, gain);

      debug(("SoundPlay: 3D sound at (%d,%d), radius=%d, gain=%.2f\n",
             src_col, src_row, radius, gain));
   }
   else
   {
      // Non-positional sound: play at listener position
      alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
      alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);

      // Set volume (convert from 0-255 to 0.0-1.0 for legacy interface)
      float gain = (float)volume / 255.0f;
      if (flags & SF_LOOP)
         gain *= (float)config.ambient_volume / 100.0f;
      else
         gain *= (float)config.sound_volume / 100.0f;
      alSourcef(source, AL_GAIN, gain);

      debug(("SoundPlay: Starting sound, gain=%.2f, loop=%d\n",
             gain, (flags & SF_LOOP) ? 1 : 0));
   }

   // Set looping
   alSourcei(source, AL_LOOPING, (flags & SF_LOOP) ? AL_TRUE : AL_FALSE);

   // Random pitch variation if requested
   if (flags & SF_RANDOM_PITCH)
   {
      float pitch = 0.9f + (rand() % 21) / 100.0f; // 0.9 to 1.1
      alSourcef(source, AL_PITCH, pitch);
   }
   else
   {
      alSourcef(source, AL_PITCH, 1.0f);
   }

   // Play
   alSourcePlay(source);

   if (alGetError() != AL_NO_ERROR)
   {
      debug(("SoundPlay: Failed to play\n"));
      return false;
   }

   return true;
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

   // Negate X to convert game coords to OpenAL coords (left-handed to right-handed)
   alListener3f(AL_POSITION, -x, y, z);

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

/*
 * Audio_StopSourcesForFilename: Stop any sources currently playing the
 * buffer associated with `filename` (case-insensitive match against cache).
 */
void Audio_StopSourcesForFilename(const char* filename)
{
   if (!g_initialized || filename == NULL)
      return;

   // Find buffer in cache
   ALuint targetBuffer = 0;
   for (int i = 0; i < g_cacheCount; i++)
   {
      if (_stricmp(g_bufferCache[i].filename, filename) == 0)
      {
         targetBuffer = g_bufferCache[i].buffer;
         break;
      }
   }
   if (targetBuffer == 0)
      return;

   // Stop any source using this buffer
   for (int i = 0; i < g_numSources; i++)
   {
      ALint buf = 0;
      alGetSourcei(g_sources[i], AL_BUFFER, &buf);
      if ((ALuint)buf == targetBuffer)
      {
         alSourceStop(g_sources[i]);
         alSourcei(g_sources[i], AL_BUFFER, 0);
      }
   }
}


