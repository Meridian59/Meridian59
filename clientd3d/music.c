// Meridian 59
/*
 * music.c:  Music playback (OGG/WAV only).
 *
 * Background music is played via the OpenAL engine. Legacy MIDI/MP3
 * extensions are mapped to .ogg for compatibility. Keep this file C++.
 */

#include "client.h"

#include <algorithm>
#include <cctype>
#include <regex>
#include <string>
#include "audio_openal.h"

static char music_dir[] = "resource";   /* Directory for music files */

static bool playing_music = false;  /* Is a music file currently playing as background? */
static ID bg_music = 0;     /* Resource id of background music file; 0 if none */
static ID latest_music = 0; /* Most recent music resource id */

enum {SOUND_MUSIC};

/* local functions */
static void PlayMusicFileInternal(const char *fname);

/******************************************************************************/
void MusicInitialize(void)
{
   ResetMusicVolume();
}

/******************************************************************************/
void MusicClose(void)
{
   MusicStop();
   playing_music = false;
}

/******************************************************************************/
DWORD PlayMidiFile(HWND hWndNotify, char *fname)
{
   (void)hWndNotify;
   if (!fname) return 1;
   std::string filename(fname);
   std::string lower = filename;
   std::transform(lower.begin(), lower.end(), lower.begin(),
                  [](unsigned char c){ return std::tolower(c); });
   lower = std::regex_replace(lower, std::regex("\\.(mid|midi|mp3)$"), ".ogg");
   if (MusicPlay(lower.c_str(), TRUE))
   {
      playing_music = true;
      debug(("PlayMidiFile: OpenAL playing %s\n", lower.c_str()));
      return 0;
   }
   debug(("PlayMidiFile: failed to play %s\n", lower.c_str()));
   return 1;
}

/******************************************************************************/
DWORD PlayMusicFile(HWND hWndNotify, const char *fname)
{
   (void)hWndNotify;
   if (!fname) return 1;
   PlayMusicFileInternal(fname);
   return playing_music ? 0 : 1;
}

static void PlayMusicFileInternal(const char *fname)
{
   if (!fname) return;
   std::string filename(fname);
   std::string lower = filename;
   std::transform(lower.begin(), lower.end(), lower.begin(),
                  [](unsigned char c){ return std::tolower(c); });

   // Map legacy extensions to .ogg when appropriate
   if (std::regex_search(lower, std::regex("\\.(mid|midi|mp3)$")))
   {
      filename = std::regex_replace(lower, std::regex("\\.(mid|midi|mp3)$"), ".ogg");
   }

   // Stop any currently playing music before attempting new track
   MusicStop();

   // Try to play the filename directly first
   if (MusicPlay(filename.c_str(), TRUE))
   {
      playing_music = true;
      debug(("PlayMusicFile: OpenAL playing %s\n", filename.c_str()));
      return;
   }

   // If that failed and filename is a bare name, try prefixed with music_dir
   bool has_path = (filename.find('\\') != std::string::npos) ||
                   (filename.find('/') != std::string::npos) ||
                   (filename.find(':') != std::string::npos);
   if (!has_path)
   {
      std::string pathbuf = std::string(music_dir) + "\\" + filename;
      if (MusicPlay(pathbuf.c_str(), TRUE))
      {
         playing_music = true;
         debug(("PlayMusicFile: OpenAL playing %s\n", pathbuf.c_str()));
         return;
      }
   }

   debug(("PlayMusicFile: failed to play %s\n", filename.c_str()));
}

/******************************************************************************/
void PlayMidiRsc(ID rsc)
{
   PlayMusicRsc(rsc);
}

/******************************************************************************/
void PlayMusicRsc(ID rsc)
{
   debug(("PlayMusicRsc %d\n", rsc));
   /* Begin ambient transition: mark current ambients so newly-started
      ambients for the new room can be registered and protected. */
   Sound_BeginAmbientTransition();

   if (rsc == 0)
   {
      // Stop any current background music only; leave ambient sounds alone.
      MusicStop();
      playing_music = false;
      bg_music = 0;
      latest_music = 0;
      return;
   }

   latest_music = rsc;
   bg_music = rsc;

   if (!config.play_music)
      return;

   // Stop current background music; ambient sounds remain unaffected here.
   MusicStop();
   playing_music = false;

   NewMusic(SOUND_MUSIC, rsc);
}

/******************************************************************************/
void NewMusic(WPARAM type, ID rsc)
{
   char *filename;
   char fname[MAX_PATH + FILENAME_MAX];

   (void)type;

   if (!rsc)
   {
      MusicStop();
      playing_music = false;
      return;
   }

   if ((filename = LookupNameRsc(rsc)) == NULL)
      return;

   latest_music = rsc;
   snprintf(fname, sizeof(fname), "%s\\%.*s", music_dir, FILENAME_MAX, filename);

   PlayMusicFile(hMain, fname);
}

/******************************************************************************/
void MusicDone(UINT device)
{
   (void)device;
}

/******************************************************************************/
void MusicAbort(void)
{
   MusicStop();
   playing_music = false;
}

/******************************************************************************/
void MusicStart(void)
{
   if (state == STATE_GAME && latest_music)
   {
      PlayMusicRsc(latest_music);
   }
}

/******************************************************************************/
void ResetMusicVolume()
{
   MusicSetVolume((float)config.music_volume / 100.0f);
}
