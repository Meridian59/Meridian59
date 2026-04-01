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

#include <functional>
#include <list>
#include <unordered_map>
#include <filesystem>
#include <thread>
#include <chrono>
#include <mutex>

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

// Configuration
static const int MAX_AUDIO_SOURCES = 32;
static const int MAX_CACHED_BUFFERS = 256;

// Global state
static ALCdevice* g_device = NULL;
static ALCcontext* g_context = NULL;
static ALuint g_sources[MAX_AUDIO_SOURCES];
static int g_numSources = 0;
static bool g_initialized = false;

// LRU buffer cache: list ordered by recency (front = newest), map for O(1) lookup.
struct CacheNode {
   std::string filename;
   ALuint buffer;
};

struct CaseInsensitiveHash {
   size_t operator()(const std::string& s) const {
      size_t hash = 0;
      for (char c : s)
      {
         // Standard hash mixing formula to spread values evenly
         hash ^= std::tolower(static_cast<unsigned char>(c)) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
      }
      return hash;
   }
};

// Case-insensitive equality using standard C++ locale-independent comparison
struct CaseInsensitiveEqual {
   bool operator()(const std::string& a, const std::string& b) const {
      if (a.size() != b.size())
         return false;
      for (size_t i = 0; i < a.size(); ++i)
      {
         if (std::tolower(static_cast<unsigned char>(a[i])) !=
             std::tolower(static_cast<unsigned char>(b[i])))
            return false;
      }
      return true;
   }
};

static std::list<CacheNode> g_cacheList;
static std::unordered_map<std::string, std::list<CacheNode>::iterator,
                          CaseInsensitiveHash, CaseInsensitiveEqual> g_cacheMap;

// Music streaming state
static const int STREAM_NUM_BUFFERS = 4;
static const int STREAM_BUFFER_SAMPLES = 4096;

struct MusicStream {
   stb_vorbis* vorbis;
   ALuint source;
   ALuint buffers[4];
   ALenum format;
   int channels;
   int sample_rate;
   bool looping;
   bool playing;
   bool finished;
};

static MusicStream g_music = {};

/* Music thread: command types sent from main thread to music thread. */
enum MusicCmd {
   MUSIC_CMD_NONE,
   MUSIC_CMD_PLAY,
   MUSIC_CMD_STOP,
   MUSIC_CMD_VOLUME,
};

static std::mutex g_musicCS;
static HANDLE g_musicThread = NULL;
static bool g_musicThreadRunning = false;

/*
 * Pending command slot (main thread writes, music thread reads).
 * This is a single slot, not a queue. Only one command can be pending
 * at a time. If the main thread posts two commands before the music
 * thread wakes, the second overwrites the first.
 */
static MusicCmd g_musicCmd = MUSIC_CMD_NONE;
static std::string g_musicCmdFile;
static bool g_musicCmdLoop = false;
static float g_musicCmdVolume = 1.0f;

static void MusicStreamUpdate(void);
static void MusicStopPlayback(void);
static DWORD WINAPI MusicThreadProc(LPVOID param);

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

   g_device = alcOpenDevice(NULL);
   if (!g_device)
   {
      debug(("AudioInit: Failed to open audio device\n"));
      return false;
   }

   g_context = alcCreateContext(g_device, NULL);
   if (!g_context)
   {
      debug(("AudioInit: Failed to create audio context\n"));
      alcCloseDevice(g_device);
      g_device = NULL;
      return false;
   }

   if (!alcMakeContextCurrent(g_context))
   {
      debug(("AudioInit: Failed to make context current\n"));
      alcDestroyContext(g_context);
      alcCloseDevice(g_device);
      g_context = NULL;
      g_device = NULL;
      return false;
   }

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

   alGenSources(1, &g_music.source);
   if (alGetError() != AL_NO_ERROR)
   {
      debug(("AudioInit: Failed to generate music source\n"));
   }

   alGenBuffers(STREAM_NUM_BUFFERS, g_music.buffers);
   if (alGetError() != AL_NO_ERROR)
   {
      debug(("AudioInit: Failed to generate music stream buffers\n"));
   }

   alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
   alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
   ALfloat listenerOri[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f }; // forward, up
   alListenerfv(AL_ORIENTATION, listenerOri);

   // Use linear distance model for predictable 3D audio falloff
   alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);

   g_initialized = true;

   g_musicVolume = (float)config.music_volume / 100.0f;

   g_musicThreadRunning = true;
   g_musicThread = CreateThread(NULL, 0, MusicThreadProc, NULL, 0, NULL);
   if (!g_musicThread)
   {
      debug(("AudioInit: Failed to create music thread\n"));
      g_musicThreadRunning = false;
   }

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

   /* Shut down the music thread before releasing any OpenAL resources. */
   if (g_musicThread)
   {
      g_musicThreadRunning = false;
      WaitForSingleObject(g_musicThread, 2000);
      CloseHandle(g_musicThread);
      g_musicThread = NULL;
   }

   SoundStopAll();
   MusicStop();

   if (g_music.source)
   {
      alDeleteSources(1, &g_music.source);
      g_music.source = 0;
   }

   alDeleteBuffers(STREAM_NUM_BUFFERS, g_music.buffers);

   if (g_numSources > 0)
   {
      alDeleteSources(g_numSources, g_sources);
      g_numSources = 0;
   }

   for (auto& node : g_cacheList)
   {
      alDeleteBuffers(1, &node.buffer);
   }
   g_cacheList.clear();
   g_cacheMap.clear();

   if (g_context)
   {
      alcMakeContextCurrent(NULL);
      alcDestroyContext(g_context);
      g_context = NULL;
   }

   if (g_device)
   {
      alcCloseDevice(g_device);
      g_device = NULL;
   }

   g_initialized = false;
   debug(("AudioShutdown: OpenAL shut down\n"));
}

/*
 * IsBufferInUse: Returns true if the buffer is currently attached to a playing or
 * paused source.
 */
static bool IsBufferInUse(ALuint buffer)
{
   for (int i = 0; i < g_numSources; i++)
   {
      ALint state, sourceBuffer;
      alGetSourcei(g_sources[i], AL_SOURCE_STATE, &state);
      if (state == AL_PLAYING || state == AL_PAUSED)
      {
         alGetSourcei(g_sources[i], AL_BUFFER, &sourceBuffer);
         if (sourceBuffer == (ALint)buffer)
            return true;
      }
   }
   return false;
}

/*
 * FindCachedBuffer: Returns cached buffer ID for filename, or 0 if not cached.
 * On cache hit, moves the entry to front of LRU list (marks as recently used).
 */
static ALuint FindCachedBuffer(const char* filename)
{
   auto it = g_cacheMap.find(filename);
   if (it == g_cacheMap.end())
      return 0;

   // Move to front of list (mark as most recently used)
   g_cacheList.splice(g_cacheList.begin(), g_cacheList, it->second);
   return it->second->buffer;
}

/*
 * CacheBuffer: Stores buffer in cache for later reuse.
 * If cache is full, evicts the least-recently-used buffer that is not currently playing.
 */
static void CacheBuffer(const char* filename, ALuint buffer)
{
   // Don't cache if already present
   if (g_cacheMap.find(filename) != g_cacheMap.end())
      return;

   // If cache is full, evict least-recently-used entries until we have room
   while (g_cacheList.size() >= MAX_CACHED_BUFFERS)
   {
      // Find LRU entry that's not in use (search from back = oldest)
      bool evicted = false;
      for (auto it = g_cacheList.rbegin(); it != g_cacheList.rend(); ++it)
      {
         if (!IsBufferInUse(it->buffer))
         {
            // Delete the OpenAL buffer
            alDeleteBuffers(1, &it->buffer);
            
            // Remove from map and list
            g_cacheMap.erase(it->filename);
            // Convert reverse iterator to forward iterator for erase
            g_cacheList.erase(std::next(it).base());
            evicted = true;
            break;
         }
      }
      
      // If all buffers are in use, we can't cache this one
      if (!evicted)
      {
         debug(("CacheBuffer: cache full and all buffers in use, not caching %s\n", filename));
         return;
      }
   }

   // Add new entry at front (most recently used)
   g_cacheList.push_front({filename, buffer});
   g_cacheMap[filename] = g_cacheList.begin();
}

/*
 * ParseOGGFile: Returns an OpenAL buffer ID for the decoded OGG file, or 0 on failure.
 * Decodes an OGG Vorbis file via stb_vorbis and uploads to OpenAL.
 */
static ALuint ParseOGGFile(const char* filename)
{
   ALuint buffer = 0;
   int channels, sample_rate;
   short* decoded;
   int num_samples;
   ALenum format;

   // Check if file exists
   if (!std::filesystem::exists(filename))
   {
      debug(("ParseOGGFile: File not found: %s\n", filename));
      return 0;
   }

   // Decode entire OGG file
   num_samples = stb_vorbis_decode_filename(filename, &channels, &sample_rate, &decoded);

   if (num_samples <= 0)
   {
      debug(("ParseOGGFile: Failed to decode %s (error code: %d)\n", filename, num_samples));
      return 0;
   }

   // Determine OpenAL format
   if (channels == 1)
      format = AL_FORMAT_MONO16;
   else if (channels == 2)
      format = AL_FORMAT_STEREO16;
   else
   {
      debug(("ParseOGGFile: Unsupported channel count %d\n", channels));
      free(decoded);
      return 0;
   }

   // Create OpenAL buffer
   alGenBuffers(1, &buffer);
   if (alGetError() != AL_NO_ERROR)
   {
      debug(("ParseOGGFile: Failed to create buffer\n"));
      free(decoded);
      return 0;
   }

   // Upload PCM data to buffer
   alBufferData(buffer, format, decoded, num_samples * channels * sizeof(short), sample_rate);
   free(decoded);

   if (alGetError() != AL_NO_ERROR)
   {
      debug(("ParseOGGFile: Failed to upload buffer data\n"));
      alDeleteBuffers(1, &buffer);
      return 0;
   }

   return buffer;
}

/*
 * MusicStreamFillBuffer: Returns true if audio was written to the buffer,
 *   false if the stream ended without producing samples.
 */
static bool MusicStreamFillBuffer(MusicStream* ms, ALuint buffer)
{
   short pcm[STREAM_BUFFER_SAMPLES * 2];  // Max stereo interleaved
   int num_shorts = STREAM_BUFFER_SAMPLES * ms->channels;
   int samples = 0;

   // Decode a chunk. stb_vorbis returns the number of samples per channel.
   samples = stb_vorbis_get_samples_short_interleaved(
      ms->vorbis, ms->channels, pcm, num_shorts);

   if (samples == 0)
   {
      int err = stb_vorbis_get_error(ms->vorbis);
      debug(("MusicStreamFillBuffer: decode returned 0 samples (error=%d, looping=%d)\n",
             err, (int)ms->looping));

      // End of file. If looping, seek to start and try again.
      if (ms->looping)
      {
         stb_vorbis_seek_start(ms->vorbis);
         samples = stb_vorbis_get_samples_short_interleaved(
            ms->vorbis, ms->channels, pcm, num_shorts);
      }

      if (samples == 0)
      {
         debug(("MusicStreamFillBuffer: still 0 after seek, marking finished\n"));
         ms->finished = true;
         return false;
      }
   }

   alBufferData(buffer, ms->format, pcm,
      samples * ms->channels * (int)sizeof(short), ms->sample_rate);

   ALenum bufErr = alGetError();
   if (bufErr != AL_NO_ERROR)
   {
      debug(("MusicStreamFillBuffer: alBufferData failed (err=0x%X, buf=%u, samples=%d)\n",
             bufErr, buffer, samples));
      return false;
   }

   return true;
}

/*
 * MusicPlay: Requests the music thread to open an OGG file for streaming
 *   playback. Returns true if the command was posted. Actual playback
 *   starts asynchronously on the music thread.
 */
bool MusicPlay(const std::string& filename, bool loop)
{
   if (!g_initialized || !g_musicThread)
   {
      debug(("MusicPlay: not ready (init=%d, thread=%p)\n",
             (int)g_initialized, (void*)g_musicThread));
      return false;
   }

   if (!std::filesystem::exists(filename))
   {
      debug(("MusicPlay: File not found: %s\n", filename.c_str()));
      return false;
   }

   {
      std::lock_guard<std::mutex> lock(g_musicCS);
      g_musicCmd = MUSIC_CMD_PLAY;
      g_musicCmdFile = filename;
      g_musicCmdLoop = loop;
   }

   debug(("MusicPlay: queued '%s' looping=%d\n", filename.c_str(), (int)loop));
   return true;
}

/*
 * MusicStreamUpdate: Refills processed OpenAL buffers from the vorbis
 *   stream. Recovers from buffer underruns by restarting the source.
 */
static void MusicStreamUpdate(void)
{
   if (!g_initialized || !g_music.playing)
      return;

   // Drain any stale error from other OpenAL calls (SFX, etc.)
   alGetError();

   ALint state = 0;
   alGetSourcei(g_music.source, AL_SOURCE_STATE, &state);

   // Check how many buffers the source has finished playing
   ALint processed = 0;
   alGetSourcei(g_music.source, AL_BUFFERS_PROCESSED, &processed);

   while (processed > 0)
   {
      ALuint buf;
      alSourceUnqueueBuffers(g_music.source, 1, &buf);

      if (!g_music.finished)
      {
         if (MusicStreamFillBuffer(&g_music, buf))
         {
            alSourceQueueBuffers(g_music.source, 1, &buf);
         }
      }

      processed--;
   }

   if (state != AL_PLAYING)
   {
      ALint queued = 0;
      alGetSourcei(g_music.source, AL_BUFFERS_QUEUED, &queued);
      if (queued > 0)
      {
         // Source starved. Restart playback with whatever is queued.
         alSourcePlay(g_music.source);
      }
      else
      {
         debug(("MusicStreamUpdate: all buffers consumed, playback done\n"));
         g_music.playing = false;
      }
   }
}

/*
 * MusicStartPlayback: Opens a vorbis stream and begins playback.
 */
static void MusicStartPlayback(const std::string& filename, bool loop)
{
   alGetError();

   MusicStopPlayback();
   alGetError();

   int error = 0;
   g_music.vorbis = stb_vorbis_open_filename(filename.c_str(), &error, NULL);
   if (!g_music.vorbis)
   {
      debug(("MusicStartPlayback: Failed to open %s (error %d)\n", filename.c_str(), error));
      return;
   }

   stb_vorbis_info info = stb_vorbis_get_info(g_music.vorbis);
   g_music.channels = info.channels;
   g_music.sample_rate = info.sample_rate;
   g_music.looping = loop;
   g_music.finished = false;

   if (info.channels == 1)
      g_music.format = AL_FORMAT_MONO16;
   else
      g_music.format = AL_FORMAT_STEREO16;

   int queued = 0;
   for (int i = 0; i < STREAM_NUM_BUFFERS; i++)
   {
      if (MusicStreamFillBuffer(&g_music, g_music.buffers[i]))
         queued++;
      else
         break;
   }

   if (queued == 0)
   {
      debug(("MusicStartPlayback: No audio data in %s\n", filename.c_str()));
      stb_vorbis_close(g_music.vorbis);
      g_music.vorbis = NULL;
      return;
   }

   /* Setting AL_BUFFER to 0 detaches any previous buffer and resets the
    * source to AL_INITIAL so it can accept a new queue. AL_LOOPING is
    * off because OpenAL's looping only replays a single buffer, not the
    * whole queue. We loop manually by seeking the decoder back to the
    * start when it reaches end-of-file. */
   alSourcei(g_music.source, AL_BUFFER, 0);
   alSourcei(g_music.source, AL_LOOPING, AL_FALSE);
   alSourcei(g_music.source, AL_SOURCE_RELATIVE, AL_TRUE);
   alSource3f(g_music.source, AL_POSITION, 0.0f, 0.0f, 0.0f);
   alSourcef(g_music.source, AL_GAIN, g_musicVolume * g_masterVolume);

   alGetError();  // Clear errors before queue+play
   alSourceQueueBuffers(g_music.source, queued, g_music.buffers);
   ALenum queueErr = alGetError();
   alSourcePlay(g_music.source);
   ALenum playErr = alGetError();

   if (queueErr != AL_NO_ERROR || playErr != AL_NO_ERROR)
   {
      debug(("MusicStartPlayback: Failed - source=%u, queueErr=0x%X, playErr=0x%X\n",
             g_music.source, queueErr, playErr));
      stb_vorbis_close(g_music.vorbis);
      g_music.vorbis = NULL;
      return;
   }

   g_music.playing = true;

   debug(("MusicStartPlayback: streaming '%s' ch=%d rate=%d looping=%d source=%u\n",
          filename.c_str(), g_music.channels, g_music.sample_rate, (int)loop,
          g_music.source));
}

/*
 * MusicStopPlayback: Stops the source, unqueues buffers, and closes the
 *   decoder.
 */
static void MusicStopPlayback(void)
{
   alSourceStop(g_music.source);

   ALint queued = 0;
   alGetSourcei(g_music.source, AL_BUFFERS_QUEUED, &queued);
   while (queued > 0)
   {
      ALuint buf;
      alSourceUnqueueBuffers(g_music.source, 1, &buf);
      queued--;
   }

   alSourcei(g_music.source, AL_BUFFER, 0);

   if (g_music.vorbis)
   {
      stb_vorbis_close(g_music.vorbis);
      g_music.vorbis = NULL;
   }

   g_music.playing = false;
   g_music.finished = false;
}

/*
 * MusicThreadProc: Background thread that services music streaming buffers
 *   and processes commands from the main thread. Runs independently of the
 *   Windows message pump so buffer refills cannot be starved by UI activity.
 */
static DWORD WINAPI MusicThreadProc(LPVOID param)
{
   while (g_musicThreadRunning)
   {
      MusicCmd cmd = MUSIC_CMD_NONE;
      std::string cmdFile;
      bool cmdLoop = false;
      float cmdVolume = 1.0f;

      {
         std::lock_guard<std::mutex> lock(g_musicCS);
         cmd = g_musicCmd;
         if (cmd != MUSIC_CMD_NONE)
         {
            cmdFile = g_musicCmdFile;
            cmdLoop = g_musicCmdLoop;
            cmdVolume = g_musicCmdVolume;
            g_musicCmd = MUSIC_CMD_NONE;
         }
      }

      switch (cmd)
      {
      case MUSIC_CMD_PLAY:
         MusicStartPlayback(cmdFile, cmdLoop);
         break;
      case MUSIC_CMD_STOP:
         MusicStopPlayback();
         break;
      case MUSIC_CMD_VOLUME:
         if (g_music.source)
            alSourcef(g_music.source, AL_GAIN, cmdVolume * g_masterVolume);
         break;
      default:
         break;
      }

      MusicStreamUpdate();
      std::this_thread::sleep_for(std::chrono::milliseconds(75));
   }

   MusicStopPlayback();
   return 0;
}

/*
 * MusicStop: Posts a stop command to the music thread.
 */
void MusicStop(void)
{
   if (!g_initialized)
      return;

   debug(("MusicStop: called (playing=%d, vorbis=%p)\n",
          (int)g_music.playing, (void*)g_music.vorbis));

   if (g_musicThread)
   {
      std::lock_guard<std::mutex> lock(g_musicCS);
      g_musicCmd = MUSIC_CMD_STOP;
   }
   else
   {
      MusicStopPlayback();
   }
}

/*
 * MusicSetVolume: Posts a volume change to the music thread.
 */
void MusicSetVolume(float volume)
{
   g_musicVolume = volume;
   if (!g_initialized || !g_music.source)
      return;

   if (g_musicThread)
   {
      std::lock_guard<std::mutex> lock(g_musicCS);
      g_musicCmdVolume = volume;
      g_musicCmd = MUSIC_CMD_VOLUME;
   }
   else
   {
      alSourcef(g_music.source, AL_GAIN, volume * g_masterVolume);
   }
}

bool MusicIsPlaying(void)
{
   return g_music.playing;
}

/*
 * ParseWAVFile: Returns an OpenAL buffer ID for the loaded WAV file, or 0 on failure.
 * Handles non-standard WAV files with extra chunks before fmt.
 */
static ALuint ParseWAVFile(const char* filename)
{
   // Open file
   FILE* file;
   if (fopen_s(&file, filename, "rb") != 0 || file == NULL)
   {
      debug(("ParseWAVFile: Cannot open %s\n", filename));
      return 0;
   }

   // Read RIFF header
   char chunkID[4];
   if (fread(chunkID, 1, 4, file) != 4 || memcmp(chunkID, "RIFF", 4) != 0)
   {
      debug(("ParseWAVFile: Not a WAV file: %s\n", filename));
      fclose(file);
      return 0;
   }

   DWORD riffSize;
   if (fread(&riffSize, 4, 1, file) != 1)
   {
      debug(("ParseWAVFile: Truncated header: %s\n", filename));
      fclose(file);
      return 0;
   }
   long riffEnd = 8 + riffSize;  // Position where RIFF chunk ends

   if (fread(chunkID, 1, 4, file) != 4 || memcmp(chunkID, "WAVE", 4) != 0)
   {
      debug(("ParseWAVFile: Not a WAV file: %s\n", filename));
      fclose(file);
      return 0;
   }

   // Search for fmt and data chunks
   // These are set inside the loop but used after it exits
   WORD audioFormat = 0, numChannels = 0, bitsPerSample = 0;
   DWORD sampleRate = 0, dataSize = 0;
   BYTE* data = NULL;
   bool foundFmt = false;
   bool foundData = false;

   while (!foundData && ftell(file) < riffEnd)
   {
      DWORD chunkSize;
      if (fread(chunkID, 1, 4, file) != 4)
         break;
      if (fread(&chunkSize, 4, 1, file) != 1)
         break;

      if (memcmp(chunkID, "fmt ", 4) == 0)
      {
         if (chunkSize < 16)
         {
            debug(("ParseWAVFile: fmt chunk too small: %s\n", filename));
            break;
         }
         WORD blockAlign;
         DWORD byteRate;
         if (fread(&audioFormat, 2, 1, file) != 1 ||
             fread(&numChannels, 2, 1, file) != 1 ||
             fread(&sampleRate, 4, 1, file) != 1 ||
             fread(&byteRate, 4, 1, file) != 1 ||
             fread(&blockAlign, 2, 1, file) != 1 ||
             fread(&bitsPerSample, 2, 1, file) != 1)
         {
            debug(("ParseWAVFile: Failed to read fmt chunk: %s\n", filename));
            break;
         }
         // Skip any extra format bytes
         if (chunkSize > 16)
            fseek(file, chunkSize - 16, SEEK_CUR);
         foundFmt = true;
      }
      else if (memcmp(chunkID, "data", 4) == 0)
      {
         if (!foundFmt)
         {
            debug(("ParseWAVFile: data chunk before fmt chunk: %s\n", filename));
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
      debug(("ParseWAVFile: Missing fmt chunk: %s\n", filename));
      return 0;
   }

   if (!data)
   {
      debug(("ParseWAVFile: No data chunk: %s\n", filename));
      return 0;
   }

   // Determine OpenAL format
   ALenum format;
   if (numChannels == 1)
      format = (bitsPerSample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
   else
      format = (bitsPerSample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;

   // Create OpenAL buffer
   ALuint buffer;
   alGenBuffers(1, &buffer);
   if (alGetError() != AL_NO_ERROR)
   {
      debug(("ParseWAVFile: Failed to generate buffer\n"));
      free(data);
      return 0;
   }

   alBufferData(buffer, format, data, dataSize, sampleRate);
   free(data);

   if (alGetError() != AL_NO_ERROR)
   {
      debug(("ParseWAVFile: Failed to buffer data\n"));
      alDeleteBuffers(1, &buffer);
      return 0;
   }

   return buffer;
}

/*
 * LoadAudioBuffer: Returns an OpenAL buffer ID for the audio file, or 0 on failure.
 * Handles caching for sound effects. Determines format from file extension.
 * Prefers .ogg files; falls back to .wav if .ogg doesn't exist.
 */
static ALuint LoadAudioBuffer(const char* filename)
{
   // Check cache first
   ALuint buffer = FindCachedBuffer(filename);
   if (buffer != 0)
      return buffer;

   // Build path and prefer .ogg over .wav
   std::filesystem::path filepath(filename);
   std::string ext = filepath.extension().string();

   // If requested file is .wav, try .ogg first (new standard)
   if (iequals(ext, ".wav"))
   {
      std::filesystem::path oggPath = filepath;
      oggPath.replace_extension(".ogg");
      
      if (std::filesystem::exists(oggPath))
      {
         filepath = oggPath;
         ext = ".ogg";
      }
   }

   std::string finalPath = filepath.string();

   // Check cache again with potentially updated path
   buffer = FindCachedBuffer(finalPath.c_str());
   if (buffer != 0)
      return buffer;

   // Parse file based on extension
   if (iequals(ext, ".ogg"))
      buffer = ParseOGGFile(finalPath.c_str());
   else
      buffer = ParseWAVFile(finalPath.c_str());

   // Cache successful loads
   if (buffer != 0)
      CacheBuffer(finalPath.c_str(), buffer);

   return buffer;
}

/*
 * SoundPlay: Returns true if sound started. Supports OGG/WAV, 3D positioning, looping.
 */
bool SoundPlay(const char* filename, int volume, BYTE flags,
               int src_row, int src_col, int radius, int max_vol)
{
   if (!g_initialized)
      return false;

   // Check master sound toggle (Options menu and preferences checkbox)
   if (!config.play_sound)
      return false;

   // Check sub-toggles for looping and random sounds (preferences checkboxes)
   if ((flags & SF_LOOP) && (!config.play_loop_sounds))
      return false;
   if ((flags & SF_RANDOM_PLACE) && (!config.play_random_sounds))
      return false;

   ALuint buffer = LoadAudioBuffer(filename);
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

   alSourcei(source, AL_BUFFER, buffer);

   // Determine if sound should use 3D positional audio or play centered on player.
   // Non-positional (centered): SF_RANDOM_PLACE, or SF_LOOP at placeholder coords (<=2,<=2)
   // Positional (3D): all other sounds with valid coordinates
   bool isAmbientLoop = (flags & SF_LOOP) && (src_row <= 2) && (src_col <= 2);
   bool isPositional = (src_row > 0 || src_col > 0) && 
                       !(flags & SF_RANDOM_PLACE) &&
                       !isAmbientLoop;

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
   }
   else
   {
      // Non-positional sound: play at listener position
      alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
      alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);

      // Set volume (volume is 0-MAX_VOLUME, convert to 0.0-1.0)
      float gain = (float)volume / (float)MAX_VOLUME;
      if (flags & SF_LOOP)
         gain *= (float)config.ambient_volume / 100.0f;
      else
         gain *= (float)config.sound_volume / 100.0f;
      alSourcef(source, AL_GAIN, gain);
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
 * AudioUpdateListener: Update listener position/orientation for 3D audio.
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
 * SoundStopLooping: Stop looping sounds only, one-shots continue to finish.
 */
void SoundStopLooping(void)
{
   if (!g_initialized)
      return;

   for (int i = 0; i < g_numSources; i++)
   {
      ALint looping;
      alGetSourcei(g_sources[i], AL_LOOPING, &looping);
      if (looping == AL_TRUE)
      {
         alSourceStop(g_sources[i]);
      }
   }
}

/*
 * Audio_StopSourcesForFilename: Stop sources playing the given filename.
 */
void Audio_StopSourcesForFilename(const char* filename)
{
   if (!g_initialized || filename == NULL)
      return;

   ALuint targetBuffer = FindCachedBuffer(filename);
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


