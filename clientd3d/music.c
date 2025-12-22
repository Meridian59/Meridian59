// Meridian 59
/*
 * music.c:  Music playback via OpenAL Soft (OGG/WAV).
 *
 * MIDI/MP3 file extensions are mapped to .ogg for compatibility.
 */

#include "client.h"

#include <cctype>
#include <filesystem>

namespace fs = std::filesystem;

static const char music_dir[] = "resource";

static bool playing_music = false;  /* Is a music file currently playing as background? */
static ID bg_music = 0;     /* Resource id of background music file; 0 if none */
static ID latest_music = 0; /* Most recent music resource id */
static std::string current_music_file; /* Filename of currently playing music */

/* local functions */
static void PlayMusicFileInternal(const std::string& fname);

/*
 * ConvertLegacyMusicExtension: Returns path with .ogg extension if original
 * was .mid/.midi/.mp3, otherwise returns path unchanged.
 */
static std::string ConvertLegacyMusicExtension(const std::string& filepath)
{
   fs::path p(filepath);
   std::string ext = p.extension().string();
   std::transform(ext.begin(), ext.end(), ext.begin(),
                  [](unsigned char c){ return std::tolower(c); });
   if (ext == ".mid" || ext == ".midi" || ext == ".mp3")
   {
      p.replace_extension(".ogg");
   }
   return p.string();
}

/******************************************************************************/
void MusicClose(void)
{
   MusicStop();
   playing_music = false;
}

/******************************************************************************/
/*
 * PlayMusicFile: Returns true if the new music file started playing, false if
 * it failed to load or play. Note: any previously playing music is stopped
 * before attempting to load the new file, regardless of success or failure.
 */
bool PlayMusicFile(HWND hWndNotify, const char *fname)
{
   if (!fname) return false;
   PlayMusicFileInternal(fname);
   return playing_music;
}

static void PlayMusicFileInternal(const std::string& fname)
{
   std::string filename = ConvertLegacyMusicExtension(fname);

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
   if (!fs::path(filename).has_parent_path())
   {
      std::string pathbuf = (fs::path(music_dir) / filename).string();
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
void PlayMusicRsc(ID rsc)
{
   char *filename;

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
      current_music_file.clear();
      return;
   }

   latest_music = rsc;
   bg_music = rsc;

   if (!config.play_music)
      return;

   // Look up the filename for this resource
   if ((filename = LookupNameRsc(rsc)) == NULL)
      return;

   std::string fname = ConvertLegacyMusicExtension((fs::path(music_dir) / filename).string());

   // Check if this is the same music file already playing (by filename, not resource ID)
   // Different rooms may use different resource IDs for the same music file
   if (playing_music && _stricmp(current_music_file.c_str(), fname.c_str()) == 0)
   {
      debug(("PlayMusicRsc: same music file already playing (%s), not restarting\n", fname.c_str()));
      return;
   }

   // Stop current background music; ambient sounds remain unaffected here.
   MusicStop();
   playing_music = false;

   // Store the new music filename before playing
   current_music_file = fname;

   PlayMusicFile(hMain, fname.c_str());
}

/******************************************************************************/
void NewMusic(WPARAM type, ID rsc)
{
   char *filename;

   if (!rsc)
   {
      MusicStop();
      playing_music = false;
      current_music_file.clear();
      return;
   }

   if ((filename = LookupNameRsc(rsc)) == NULL)
      return;

   latest_music = rsc;
   std::string fname = (fs::path(music_dir) / filename).string();

   // Check if the same music file is already playing
   if (playing_music && _stricmp(current_music_file.c_str(), fname.c_str()) == 0)
   {
      debug(("NewMusic: same music file already playing (%s), not restarting\n", fname.c_str()));
      return;
   }

   // Stop current music before playing new one
   MusicStop();
   playing_music = false;

   // Store the new music filename before playing
   current_music_file = fname;

   PlayMusicFile(hMain, fname.c_str());
}

/******************************************************************************/
void MusicAbort(void)
{
   MusicStop();
   playing_music = false;
}

/******************************************************************************/
/*
 * MusicRestart: Resume playing the most recent background music.
 * Called when user re-enables music in settings.
 */
void MusicRestart(void)
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
