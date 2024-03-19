// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * music.c:  Play MIDI files.
 *
 *  There are 2 types of music dealt with in this source file:  midi, and music.
 *  Midi and music sounds are both played by the MIDI device; music is just a MIDI sound
 *  used as background music.  If a music sound is playing, a midi sound first pauses
 *  the background music; the music resumes when the midi sound is done.  When the music 
 *  ends, it automatically loops back to the beginning.
 */

#include "client.h"

#include <cctype>
#include <regex>
#include <string>

static char music_dir[] = "resource";   /* Directory for sound files */

static Bool has_midi = False;    /* Can system play MIDI files? */

static Bool playing_midi = False;  /* Is a MIDI file currently playing as an effect? */
static Bool playing_music = False;  /* Is a MIDI file currently playing as background? */
static Bool isMusicPaused = False;   /* Is background music paused? */

static UINT midi_element;  /* Currently playing MIDI file ID */
static UINT midi_bg_music_element;  /* Currently playing background music ID */

static ID    bg_music = 0;     /* Resource id of background music MIDI file; 0 if none */
static ID    latest_music = 0;   /* Resource id of most recent music file played - regardless of type */
static ID    paused_music = 0;   /* Resource id of the paused music file */
static DWORD music_pos = 0;    /* Current position in background music when paused */
static DWORD time_format;      /* Time format of current music MIDI file */

#ifdef M59_MSS
static HDIGDRIVER hDigitalDriver;
static HSAMPLE hseqBackground;
static HSAMPLE hseqImmediate;
static BYTE *pMIDIBackground;
static BYTE *pMIDIImmediate;
#endif

enum {SOUND_MIDI, SOUND_MUSIC};

/* local functions */
static DWORD OpenMidiFile(const char *lpszMIDIFileName);
static DWORD RestartMidiFile(DWORD device);
static void PauseMusic(void);
static void UnpauseMusic(void);
/******************************************************************************/
/*
 * MusicInitialize: Check for a MIDI device, and open it if present.
 */
void MusicInitialize(void)
{
   if (config.soundLibrary == LIBRARY_NIL)
   {
      has_midi = FALSE;
      return;
   }

#ifdef M59_MSS
	hDigitalDriver = NULL;
	hseqBackground = NULL;
	hseqImmediate = NULL;

	// it's ok to do this more than once, and we do,
	//	once in SoundInitialize and once here
	AIL_startup();

   // Open first MIDI device, trying the MIDI Mapper first
   hDigitalDriver = AIL_open_digital_driver(44100, 16, MSS_MC_USE_SYSTEM_CONFIG, 0);

	if (hDigitalDriver == NULL)
	{
		debug(( "MSS digital sound failed to initialize, error = %s.\n", AIL_last_error() ));
		has_midi = False;
		return;
	}

	hseqBackground = AIL_allocate_sample_handle( hDigitalDriver );
	hseqImmediate = AIL_allocate_sample_handle( hDigitalDriver );

	if( ( hseqBackground == NULL ) || ( hseqImmediate == NULL ) )
	{
		debug(( "MSS digital sound failed to allocate 2 handles.\n" ));
		has_midi = False;
		return;
	}

	debug(( "MSS digital sound initialized successfully.\n" ));
	has_midi = True;
#else
   MCI_SYSINFO_PARMS mciSysinfoParms;
   DWORD num_devices, retval;
   
   mciSysinfoParms.lpstrReturn = (LPSTR) &num_devices;
   mciSysinfoParms.dwRetSize = sizeof(num_devices);
   mciSysinfoParms.wDeviceType = MCI_DEVTYPE_SEQUENCER;
   retval = mciSendCommand(0, MCI_SYSINFO, MCI_SYSINFO_QUANTITY,
                           (DWORD_PTR) &mciSysinfoParms);
   if (retval == 0 && num_devices > 0)
   {
      has_midi = True;
      ResetMusicVolume();
   }
#endif // M59_MSS
}
/******************************************************************************/
void MusicClose(void)
{
   if (!has_midi)
      return;

#ifdef M59_MSS
	AIL_release_sample_handle(hseqBackground);
	AIL_release_sample_handle(hseqImmediate);

	if (pMIDIImmediate)
		AIL_mem_free_lock(pMIDIImmediate);
	if (pMIDIBackground)
		AIL_mem_free_lock(pMIDIBackground);

	AIL_shutdown();
#else
   mciSendCommand(MCI_ALL_DEVICE_ID, MCI_CLOSE, 0, 0);
#endif
}
/******************************************************************************/
//	Not used by MSS version
/*
 * OpenMidiFile:  Open midi file for playing.
 *   For background room music opens midi_bg_music_element device.
 *   For any other music (jala) opens midi_element device.
 *   Returns 0 if successful; MCI error code otherwise.
 */
DWORD OpenMidiFile(const char *lpszMIDIFileName, UINT device)
{
   DWORD dwReturn;
   MCI_OPEN_PARMS mciOpenParms;
   char current_dir[MAX_PATH], filename[MAX_PATH];

   // Seems not to work in Windows 95 sometimes without full pathname
   if (!GetWorkingDirectory(current_dir, MAX_PATH))
      debug(("Unable to get current directory!\n"));

   sprintf(filename, "%s%s", current_dir, lpszMIDIFileName);
   debug(("music filename = %s\n", filename));
   // Is it a background music or jala music file?
   if (device == midi_bg_music_element)
   {
      if (playing_music != 0)
      {
         mciSendCommand(midi_bg_music_element, MCI_CLOSE, 0, 0);
      }
   }
   else if (device == midi_element)
   {
      if (playing_midi != 0)
      {
         mciSendCommand(midi_element, MCI_STOP, 0, 0);
         if (dwReturn = mciSendCommand(midi_element, MCI_CLOSE, 0, 0))
         {
            // Just debug don't return
            debug(("Midi Element - mciClose problem\n"));
         }
      }
   }
   memset(&mciOpenParms, 0, sizeof(MCI_OPEN_PARMS));
   mciOpenParms.lpstrDeviceType = "sequencer";
   mciOpenParms.lpstrElementName = filename;
   debug(("Loading song into memory \n"));
   if (dwReturn = mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT,
                              (DWORD_PTR)(LPVOID) &mciOpenParms))
   { 
      debug(("can't MCI open new song \n"));
      return dwReturn;
   }
   // Save element ID and set flag
   if (device == midi_bg_music_element)
   {
      midi_bg_music_element = mciOpenParms.wDeviceID;
      playing_music = 1;
      debug(("midi_bg_music_element = %d\n", midi_bg_music_element));
   }
   else
   {
      midi_element = mciOpenParms.wDeviceID;
      playing_midi = 1;
      debug(("midi element = %d\n", midi_element));
   }
   return 0;
}
/******************************************************************************/
/*
 * PlayMidiFile:  Play given MIDI file and notify given window when done.
 *   Returns 0 if successful, MCI error code otherwise.
 *   Only used by Jala music files - never background music 
 *   even if it is a midi file.
 */
DWORD PlayMidiFile(HWND hWndNotify, char *fname)
{
   if (!has_midi)
      return 0;

#ifdef M59_MSS
	// If a sample was playing, end it
	if( AIL_sample_status( hseqImmediate ) != SMP_DONE )
		AIL_end_sample( hseqImmediate );

	// free memory from previous MIDI file
	if( pMIDIImmediate )
		AIL_mem_free_lock( pMIDIImmediate );

	// First try MP3 file
	char *ext = strstr( _strlwr( fname ), ".mid" );
	if( ext != NULL )
		strcpy( ext, ".mp3" );

	// load the file
	pMIDIImmediate = (BYTE *) AIL_file_read( fname, NULL );
	if( !pMIDIImmediate )
	{
      // Next try xmi file
      ext = strstr(fname, ".mp3" );
      if( ext != NULL )
         strcpy( ext, ".xmi" );
      
      pMIDIImmediate = (BYTE *) AIL_file_read( fname, NULL );
      if( !pMIDIImmediate )
      {
         debug(( "Failed to load music file %s.\n", fname ));
         return 0;
      }
	}

	// Initialize the sample
	if (!AIL_set_named_sample_file(hseqImmediate, fname, pMIDIImmediate,
                                  AIL_file_size(fname), 0))
	{
		debug(( "Failed to init music file.\n" ));
		return 0;
	}
   // Loop the MIDI until game actions stop it
	AIL_set_sample_loop_count( hseqImmediate, 0 );
   // Set volume
   float vol = ((float) config.music_volume) / CONFIG_MAX_VOLUME;
	AIL_set_sample_volume_levels(hseqImmediate, vol, vol );

	// start playing
	AIL_start_sample(hseqImmediate);

	debug(( "Playing midi music file %s. LatestMusic=%d \n", fname, latest_music ));
	playing_midi = True;
	return 0;
#else
   {
      DWORD dwReturn;
      MCI_PLAY_PARMS mciPlayParms;

      if ((dwReturn = OpenMidiFile(fname, midi_element)) != 0)
      {
         debug(("OpenMidiFile error - can't open \n"));
         return dwReturn;
      }

      /*
       * Begin playback. The window procedure function
       * for the parent window is notified with an
       * MM_MCINOTIFY message when playback is complete.
       * The window procedure then closes the device.
       */
      memset(&mciPlayParms, 0, sizeof(MCI_PLAY_PARMS));

      mciPlayParms.dwCallback = (DWORD_PTR) hWndNotify;
      if (dwReturn = mciSendCommand(midi_element, MCI_PLAY,
                                    MCI_NOTIFY, (DWORD_PTR)(LPVOID) &mciPlayParms)) 
      {
         debug(("Midi Element - mciPlay problem\n"));
         mciSendCommand(midi_element, MCI_CLOSE, 0, 0);
         return dwReturn;
      }
      
      debug(("Playing MIDI file, element = %d\n", midi_element));
      playing_midi = True;
      return 0;
   }
#endif
}
/******************************************************************************/
/*
 * PlayMusicFile:  Play given MIDI file and notify given window when done.
 *   If background music had been playing, it picks up where it left off.
 *   Returns 0 if successful, MCI error code otherwise.
 */
DWORD PlayMusicFile(HWND hWndNotify, const char *fname)
{
	if (!has_midi)
		return 0;

#ifdef M59_MSS
	// If a sequence was paused and we are trying to play that music
   // then unpause it.
	if (isMusicPaused)
	{
      if (paused_music == bg_music)
      {
		   UnpauseMusic();
		   return 0;
      }
      else
      {
         // The music was paused but we need different music so reset the flag
         isMusicPaused = False;
      }
	}
   if (playing_midi)
   {
      playing_midi = False;
      AIL_end_sample(hseqImmediate);
   }

   if (playing_music)
   {
      playing_music = False;
      AIL_end_sample(hseqBackground);
   }
	// free memory from previous background music
	if (pMIDIBackground)
		AIL_mem_free_lock(pMIDIBackground);

	// First try MP3 file
	std::string filename(fname);
	std::transform(filename.begin(), filename.end(), filename.begin(),
                 [](unsigned char c){ return std::tolower(c); });
	filename = std::regex_replace(filename, std::regex("\\.mid$"), ".mp3");

	// load the file
	pMIDIBackground = (BYTE *) AIL_file_read( filename.c_str(), NULL);
	if( !pMIDIBackground )
	{
      // Next try xmi file
	  filename = std::regex_replace(filename, std::regex("\\.mp3$"), ".xmi");
      
      pMIDIBackground = (BYTE *) AIL_file_read( filename.c_str(), NULL);
      if( !pMIDIBackground )
      {
         debug(( "Failed to load music file %s.\n", filename.c_str() ));
         return 0;
      }
	}

	// initialize the sequence
	if (!AIL_set_named_sample_file(hseqBackground, filename.c_str(), pMIDIBackground,
                                  AIL_file_size(filename.c_str()), 0 ) )
	{
		debug(( "Failed to init music sequence %s.\n", filename.c_str() ));
		return 0;
	}

	// set to loop indefinitely
	AIL_set_sample_loop_count( hseqBackground, 0 );

   // Set volume
   float vol = ((float) config.music_volume) / CONFIG_MAX_VOLUME;
	AIL_set_sample_volume_levels(hseqBackground, vol, vol );

	// start playing
	AIL_start_sample( hseqBackground );
	debug(( "Playing music file %s.\n", filename.c_str() ));
	playing_music = True;
	return 0;


#else
   DWORD dwReturn;
   MCI_PLAY_PARMS mciPlayParms;
   char temp[81];

   // If already playing music, pick up where we left off
   if (isMusicPaused)
   {
      if (paused_music == bg_music)
      {
		   UnpauseMusic();
		   return 0;
      }
      else
      {
         // The music was paused but we need different music so reset the flag
         isMusicPaused = False;
      }
   }

   if ((dwReturn = OpenMidiFile(fname, midi_bg_music_element)) != 0)
     {
       debug(("OpenMidiFile error code = %d\n", dwReturn));
       mciGetErrorString(dwReturn, temp, 80);
       strcat(temp, " \n");
       debug((temp));
       return dwReturn;
     }

   if (playing_midi)
   {
      // We're trying to play BG music, but Midi music is still playing.
      // This can happen if we toggle music off and on
      // with an active Jala spell.
      mciSendCommand(midi_element, MCI_STOP, 0, 0);
      mciSendCommand(midi_element, MCI_CLOSE, 0, 0);
      // No need to return - try to close
      // If we can't close try to play BG music.
      playing_midi = False;
   }
   /*
    * Begin playback. The window procedure function
    * for the parent window is notified with an
    * MM_MCINOTIFY message when playback is complete.
    * The window procedure then closes the device.
    */
   mciPlayParms.dwCallback = (DWORD_PTR) hWndNotify;
   if (dwReturn = mciSendCommand(midi_bg_music_element, MCI_PLAY,
                                 MCI_NOTIFY, (DWORD_PTR)(LPVOID) &mciPlayParms)) 
   {
      mciGetErrorString(dwReturn, temp, 80);
      strcat(temp, " \n");
      debug((temp));
      
      mciSendCommand(midi_bg_music_element, MCI_CLOSE, 0, 0);
      
      return dwReturn;
   }

   debug(("Playing music file, element = %d\n", midi_bg_music_element));
   playing_music = True;
   return 0;
#endif
}
/******************************************************************************/
//	Not used by MSS version
/*
 * RestartMidiFile:  Rewind given device to beginning, and resume playing.
 */
DWORD RestartMidiFile(DWORD device)
{
   DWORD dwReturn;
   MCI_PLAY_PARMS mciPlayParms;

   dwReturn = mciSendCommand(device, MCI_SEEK,
                             MCI_SEEK_TO_START, (DWORD_PTR)(LPVOID) NULL);
   
   mciPlayParms.dwCallback = (DWORD_PTR) hMain;
   mciPlayParms.dwFrom = 0;
   if (dwReturn = mciSendCommand(device, MCI_PLAY,
                                 MCI_NOTIFY, (DWORD_PTR)(LPVOID) &mciPlayParms)) {
      mciSendCommand(device, MCI_CLOSE, 0, 0);
      
   }
   debug(("Restarting MIDI file, element = %d\n", device));
   return dwReturn;
}
/******************************************************************************/
/* 
 * PauseMusic:  Store current position of background music.  Assumes that
 *   music is playing on MIDI device.
 *   Only room background music can be paused.
 */
void PauseMusic(void)
{
   if (!has_midi)
      return;

#ifdef M59_MSS

	if( AIL_sample_status( hseqBackground ) == SMP_PLAYING )
   {
		AIL_stop_sample( hseqBackground );
   }
	// indicate we are paused
	isMusicPaused = True;
	debug(( "Pausing music.\n" ));
#else
   MCI_STATUS_PARMS mciStatusParms;

   mciStatusParms.dwItem = MCI_STATUS_POSITION;
   mciSendCommand(midi_bg_music_element, MCI_STATUS, 
                  MCI_STATUS_ITEM, (DWORD_PTR)(LPVOID) &mciStatusParms);
   music_pos = mciStatusParms.dwReturn;

   /* Get time format */
   mciStatusParms.dwItem = MCI_STATUS_TIME_FORMAT;
   mciSendCommand(midi_bg_music_element, MCI_STATUS, 
                  MCI_STATUS_ITEM, (DWORD_PTR)(LPVOID) &mciStatusParms);
   time_format = mciStatusParms.dwReturn;

   // Pause the music!
   MCI_GENERIC_PARMS mciPauseParms;
   mciSendCommand(midi_bg_music_element, MCI_PAUSE, 0, (DWORD_PTR)(LPVOID) &mciPauseParms);

   isMusicPaused = True;
   debug(("Pausing, position = %ld\n", music_pos));
#endif
}
/******************************************************************************/
/*
 * UnpauseMusic:  Restore saved position of background music.
 *   Only room background music can be unpaused.
 */
void UnpauseMusic(void)
{
   if (!has_midi)
      return;
#ifdef M59_MSS
   if (playing_midi)
   {
      playing_midi = False;
      AIL_end_sample(hseqImmediate);
   }
	if (isMusicPaused)
   {
      isMusicPaused = False;
		AIL_resume_sample(hseqBackground);
   }
	debug(( "Unpausing music. bg_music=%d, paused_music=%d\n", bg_music, paused_music));
#else
   DWORD dwReturn;
   char temp[81];
   MCI_PLAY_PARMS mciPlayParms;
   MCI_SEEK_PARMS mciSeekParms;
   MCI_SET_PARMS  mciSetParms;
   MCI_STATUS_PARMS mciStatusParms;

   /* Set time format */
   mciSetParms.dwTimeFormat = time_format;
   mciSendCommand(midi_bg_music_element, MCI_SET,
                  MCI_SET_TIME_FORMAT, (DWORD_PTR)(LPVOID) &mciSetParms);

   // Check to make sure the position we found is valid
   memset(&mciStatusParms, 0, sizeof(MCI_STATUS_PARMS));
   mciStatusParms.dwItem = MCI_STATUS_LENGTH;
   dwReturn = mciSendCommand(midi_bg_music_element, MCI_STATUS, 
                             MCI_STATUS_LENGTH, (DWORD_PTR)(LPVOID) &mciStatusParms);
   if (dwReturn == 0)
   {
      DWORD dwLength = mciStatusParms.dwReturn;
      if (music_pos > dwLength)
      {
         debug(("Music position is greater than length, setting to 0\n"));
         music_pos = 0;
      }
   }
   mciSeekParms.dwTo = music_pos;
   mciSendCommand(midi_bg_music_element, MCI_SEEK,
                  MCI_TO, (DWORD_PTR)(LPVOID) &mciSeekParms);
   // Unpause the music!
   mciPlayParms.dwFrom = music_pos;
   debug(("Music Position:%ld\n", music_pos));

   if (playing_midi)
   {
      /* Stop the Jala Song first! */
      debug(("Unpausing, stopping MIDI\n"));
      mciSendCommand(midi_element, MCI_STOP, 0, 0);
      if (dwReturn = mciSendCommand(midi_element, MCI_CLOSE, 0, 0))
      {
         debug(("mciClose problem for Jala music\n"));
         mciGetErrorString(dwReturn, temp, 80);
         strcat(temp, " \n");
         debug((temp));
      }
      else
      {
         playing_midi = False;
      }
   }

   isMusicPaused = False;
   mciSendCommand(midi_bg_music_element, MCI_PLAY, MCI_FROM | MCI_NOTIFY, (DWORD_PTR)(LPVOID) &mciPlayParms);

   debug(("Unpausing to  position = %ld\n", music_pos));
#endif
}
/******************************************************************************/
/*
 * PlayMidiRsc:  Play MIDI file associated with given resource number.
 *
 * This function handles pausing or stopping music and midi files and
 * the posts a message which is handled and sends us to NewMusic.
 */
void PlayMidiRsc(ID rsc)
{
   debug(("PlayMidiRsc %d, bg_music is:%d \n", rsc, bg_music));
   // Save the rsc as latest_music in case our music is off in the config
   // This way if we toggle it on we have the correct rsc to play.
   latest_music = rsc;

   if (!config.play_music || !has_midi)      
      return;
   // playing_music is still true when we are paused 
   // so check both status before pausing
   if (playing_music && !isMusicPaused)
   {
      paused_music = bg_music;
      PauseMusic();
#ifndef M59_MSS
      // Kill the current Midi then POST - NewMusic type = midi.
      mciSendCommand(midi_element, MCI_CLOSE, 0, 0);
      playing_midi = False;
#endif
      PostMessage(hMain, BK_NEWSOUND, SOUND_MIDI, rsc);
      return;
   }

#ifndef M59_MSS
   DWORD dwReturn;
   /* If NOT playing music and IF playing midi...*/
   /* Stop the midi and prepare to restart a new one. */
   if (playing_midi)
   {
      if (dwReturn = mciSendCommand(midi_element, MCI_CLOSE, 0, 0))
      {
         debug(("mciClose problem for Jala music\n"));
      }
      else
      {
         playing_midi = False;
      }

      PostMessage(hMain, BK_NEWSOUND, SOUND_MIDI, rsc);
      return;
   }
#endif
   if (playing_midi)
   {
      // If already playing midi we have to post the message 
      PostMessage(hMain, BK_NEWSOUND, SOUND_MIDI, rsc);
   }
   // If NOT playing music and NOT playing midi just call NewMusic.
   NewMusic(SOUND_MIDI, rsc);
}
/******************************************************************************/
/*
 * PlayMusicRsc:  Play MIDI file associated with given resource number as
 *   background music.
 */
void PlayMusicRsc(ID rsc)
{
   debug(("PlayMusicRsc %d\n", rsc));

   /* If we're already playing same music file, keep jammin' */
   if (playing_music && !isMusicPaused)
   { 
      if (rsc != 0 && bg_music != 0) 
	  {
        if (!stricmp(LookupNameRsc(rsc), LookupNameRsc(bg_music)))
        {
		  debug(("DEBUG Already playing that music.\n" ));
        return;
        }
        /* Playing music is true, not paused, need new bg music */
        /* so kill the current background music before continuing. */
        #ifdef M59_MSS
			   AIL_end_sample( hseqBackground );
        #else
			   mciSendCommand(midi_bg_music_element, MCI_CLOSE, 0, 0); 
         #endif
	  }
	}
   // Save the rsc as latest_music in case our music is off in the config
   latest_music = rsc;
   // Save the rsc as bg_music in case we need to pause and resume.
   bg_music = rsc;

   if (!config.play_music || !has_midi || rsc == 0)
      return;

   NewMusic(SOUND_MUSIC, rsc);
}
/******************************************************************************/
/*
 * NewMusic:  Start playing new music.  type gives type of sound (MIDI, 
 *   music).  rsc gives resource id of file.
 * Here we convert the rsc to a filename and send a message to
 * either PlayMusicFile or PlayMidiFile depending on the type.
 */
void NewMusic(WPARAM type, ID rsc)
{
   char *filename, fname[MAX_PATH + FILENAME_MAX];
	// NULL rsc => abort any songs in progress
	if( !rsc )
	{
		if( ( type == SOUND_MIDI ) && ( playing_midi ) )
		{
#ifdef M59_MSS
			AIL_end_sample( hseqImmediate );
#else
			mciSendCommand(midi_element, MCI_CLOSE, 0, 0);
#endif
			playing_midi = False;
		}
		else if( ( type == SOUND_MUSIC ) && ( playing_music ) )
		{
#ifdef M59_MSS
			AIL_end_sample( hseqBackground );
#else
			mciSendCommand(midi_bg_music_element, MCI_CLOSE, 0, 0); 
#endif
			playing_music = False;
			return;
		}
		return;
	}

	if( (filename = LookupNameRsc(rsc)) == NULL )
		return;
   // Sometimes we get here via POST and may need to resave latest_music variable.
   latest_music = rsc;
   sprintf(fname, "%s\\%.*s", music_dir, FILENAME_MAX, filename);

   switch (type)
  {
   case SOUND_MIDI:
      if (playing_music && !isMusicPaused)
      {
         paused_music = bg_music;
         PauseMusic();
      }
      PlayMidiFile(hMain, fname);
#ifndef M59_MSS
      debug(("NewMusic MIDI, element = %d\n", midi_element));
#endif
      break;

   case SOUND_MUSIC:
      latest_music = rsc;
      PlayMusicFile(hMain, fname);
#ifndef M59_MSS
      debug(("NewMusic music, element = %d\n", midi_bg_music_element));
#endif
      break;
   }
}
/******************************************************************************/
//	Not used by MSS version
/*
 * MusicDone:  A MIDI file has finished playing; restart background music 
 *   if appropriate.
 */
#ifndef M59_MSS
void MusicDone(UINT device)
{
   /* Loop Jala song */
   if (playing_midi && device == midi_element)
   {
      RestartMidiFile(midi_element);
      return;
   }

   /* Loop background music */
   if (playing_music && device == midi_bg_music_element)
   {
      debug(("Looping Music device %d\n", device));
      RestartMidiFile(midi_bg_music_element);
      return;
   }
}
#endif
/******************************************************************************/
/*
 * MusicAbort:  Turn off currently playing music
 */
void MusicAbort(void)
{
   if (!has_midi)
      return;

   if (playing_music || playing_midi)
   {
#ifdef M59_MSS
	   AIL_end_sample(hseqBackground);
	   AIL_end_sample(hseqImmediate);
#else
      mciSendCommand(MCI_ALL_DEVICE_ID, MCI_CLOSE, 0, 0);
#endif
      // reset all the flags
      playing_midi = False;
      playing_music = False;
      isMusicPaused = False;
   }
}
/******************************************************************************/
/*
 * MusicStart:  Start playing bg music if any (used when player toggles music on).
 */
void MusicStart(void)
{
   if (state == STATE_GAME)
   {
      if (latest_music == bg_music)
      {
         PlayMusicRsc(latest_music);
      }
      else
      {
         PlayMidiRsc(latest_music);
      }
   }
}
/******************************************************************************/
/*
 * ResetMusicVolume:  Change music volume in response to a config change
 */
void ResetMusicVolume()
{
   if (!has_midi)
      return;

#ifdef M59_MSS
   // Set volume
   float vol = ((float) config.music_volume) / CONFIG_MAX_VOLUME;
   if (hseqImmediate != NULL)
      AIL_set_sample_volume_levels(hseqImmediate, vol, vol );
   if (hseqBackground != NULL)
      AIL_set_sample_volume_levels(hseqBackground, vol, vol );
#else
   // TODO: Implement via MCI
#endif
}
