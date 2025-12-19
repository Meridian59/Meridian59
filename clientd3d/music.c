// Meridian 59
/*
 * music.c:  Music playback via OpenAL Soft (OGG/WAV).
 *
 * MIDI/MP3 file extensions are mapped to .ogg for compatibility.
 */

#include "client.h"

#include <algorithm>
#include <cctype>
#include <regex>
#include <string>

static const char music_dir[] = "resource";

static bool playing_music = false;  /* Is a music file currently playing as background? */
static ID bg_music = 0;     /* Resource id of background music file; 0 if none */
static ID latest_music = 0; /* Most recent music resource id */
static char current_music_file[MAX_PATH] = ""; /* Filename of currently playing music */

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
/*
 * PlayMidiFile: Returns true if music started successfully.
 * Legacy shim for BP_PLAY_MIDI; maps .mid/.midi/.mp3 to .ogg.
 */
bool PlayMidiFile(HWND hWndNotify, char *fname)
{
   (void)hWndNotify;
   if (!fname) return false;
   std::string filename(fname);
   std::string lower = filename;
   std::transform(lower.begin(), lower.end(), lower.begin(),
                  [](unsigned char c){ return std::tolower(c); });
   lower = std::regex_replace(lower, std::regex("\\.(mid|midi|mp3)$"), ".ogg");
   if (MusicPlay(lower.c_str(), true))
   {
      playing_music = true;
      debug(("PlayMidiFile (legacy compat): OpenAL playing %s\n", lower.c_str()));
      return true;
   }
   debug(("PlayMidiFile (legacy compat): failed to play %s\n", lower.c_str()));
   return false;
}

/******************************************************************************/
// Returns true if music started successfully.
bool PlayMusicFile(HWND hWndNotify, const char *fname)
{
   (void)hWndNotify;
   if (!fname) return false;
   PlayMusicFileInternal(fname);
   return playing_music;
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
   if (MusicPlay(filename.c_str(), true))
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
      if (MusicPlay(pathbuf.c_str(), true))
      {
         playing_music = true;
         debug(("PlayMusicFile: OpenAL playing %s\n", pathbuf.c_str()));
         return;
      }
   }

   debug(("PlayMusicFile: failed to play %s\n", filename.c_str()));
}

/******************************************************************************/
/*
 * PlayMidiRsc: Legacy compatibility shim. Forwards to PlayMusicRsc.
 */
void PlayMidiRsc(ID rsc)
{
   PlayMusicRsc(rsc);
}

/******************************************************************************/
void PlayMusicRsc(ID rsc)
{
   char *filename;
   char fname[MAX_PATH + FILENAME_MAX];

   debug(("PlayMusicRsc %d\n", rsc));
   /* Begin looping sound transition: mark current looping sounds so newly-started
      sounds for the new room can be registered and protected. */
   Sound_BeginLoopingSoundTransition();

   if (rsc == 0)
   {
      // Stop any current background music only; leave ambient sounds alone.
      MusicStop();
      playing_music = false;
      bg_music = 0;
      latest_music = 0;
      current_music_file[0] = '\0';
      return;
   }

   latest_music = rsc;
   bg_music = rsc;

   if (!config.play_music)
      return;

   // Look up the filename for this resource
   if ((filename = LookupNameRsc(rsc)) == NULL)
      return;

   snprintf(fname, sizeof(fname), "%s\\%.*s", music_dir, FILENAME_MAX, filename);

   // Apply legacy extension conversion (.mid/.midi/.mp3 -> .ogg)
   std::string fname_str(fname);
   std::string lower = fname_str;
   std::transform(lower.begin(), lower.end(), lower.begin(),
                  [](unsigned char c){ return std::tolower(c); });
   if (std::regex_search(lower, std::regex("\\.(mid|midi|mp3)$")))
   {
      fname_str = std::regex_replace(lower, std::regex("\\.(mid|midi|mp3)$"), ".ogg");
      strncpy(fname, fname_str.c_str(), sizeof(fname) - 1);
      fname[sizeof(fname) - 1] = '\0';
   }

   // Check if this is the same music file already playing (by filename, not resource ID)
   // Different rooms may use different resource IDs for the same music file
   if (playing_music && _stricmp(current_music_file, fname) == 0)
   {
      debug(("PlayMusicRsc: same music file already playing (%s), not restarting\n", fname));
      return;
   }

   // Stop current background music; ambient sounds remain unaffected here.
   MusicStop();
   playing_music = false;

   // Store the new music filename before playing
   strncpy(current_music_file, fname, sizeof(current_music_file) - 1);
   current_music_file[sizeof(current_music_file) - 1] = '\0';

   PlayMusicFile(hMain, fname);
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
      current_music_file[0] = '\0';
      return;
   }

   if ((filename = LookupNameRsc(rsc)) == NULL)
      return;

   latest_music = rsc;
   snprintf(fname, sizeof(fname), "%s\\%.*s", music_dir, FILENAME_MAX, filename);

   // Check if the same music file is already playing
   if (playing_music && _stricmp(current_music_file, fname) == 0)
   {
      debug(("NewMusic: same music file already playing (%s), not restarting\n", fname));
      return;
   }

   // Stop current music before playing new one
   MusicStop();
   playing_music = false;

   // Store the new music filename before playing
   strncpy(current_music_file, fname, sizeof(current_music_file) - 1);
   current_music_file[sizeof(current_music_file) - 1] = '\0';

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
