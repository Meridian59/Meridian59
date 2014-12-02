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

static char music_dir[] = "resource";   /* Directory for sound files */
char filename[MAX_PATH];

static Bool has_midi = False;    /* Can system play MIDI files? */

static Bool playing_midi = False;  /* Is a MIDI file currently playing as an effect? */
static Bool playing_music = False;  /* Is a MIDI file currently playing as background? */

static UINT midi_element;  /* Currently playing MIDI file ID */

static ID    bg_music = 0;     /* Resource id of background music MIDI file; 0 if none */
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
static DWORD OpenMidiFile(LPSTR lpszMIDIFileName);
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
                           (DWORD) &mciSysinfoParms);
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
 *   Returns 0 if successful; MCI error code otherwise.
 */
DWORD OpenMidiFile(LPSTR lpszMIDIFileName)
{
   DWORD dwReturn;
   MCI_OPEN_PARMS mciOpenParms;
   //char current_dir[MAX_PATH], filename[MAX_PATH];
   char current_dir[MAX_PATH];

   // Seems not to work in Windows 95 sometimes without full pathname
   if (!GetWorkingDirectory(current_dir, MAX_PATH))
      debug(("Unable to get current directory!\n"));

   sprintf(filename, "%s%s", current_dir, lpszMIDIFileName);
   debug(("music filename = %s\n", filename));

   memset(&mciOpenParms, 0, sizeof(MCI_OPEN_PARMS));
   mciOpenParms.lpstrDeviceType = "MPEGVideo";
   mciOpenParms.lpstrElementName = filename;
   if (dwReturn = mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE,
                                 (DWORD)(LPVOID) &mciOpenParms)) 
      return dwReturn;

   midi_element = mciOpenParms.wDeviceID;
   debug(("midi element = %d\n", midi_element));
   return 0;
}
/******************************************************************************/
/*
 * PlayMidiFile:  Play given MIDI file and notify given window when done.
 *   Returns 0 if successful, MCI error code otherwise.
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

   // Set volume
   float vol = ((float) config.music_volume) / CONFIG_MAX_VOLUME;
   AIL_set_sample_volume_levels(hseqImmediate, vol, vol );

   // start playing
   AIL_start_sample(hseqImmediate);

   // Set end-of-sample callback so we can unpause
   //	the background music when done playing.
   AIL_register_EOS_callback(hseqImmediate, MIDIDoneCallback);

   debug(( "Playing music file %s.\n", fname ));
   playing_midi = True;
   return 0;
#else
   {
      DWORD dwReturn;
      MCI_PLAY_PARMS mciPlayParms;

      if ((dwReturn = OpenMidiFile(fname)) != 0)
      {
        return dwReturn;
      }

      /*
       * Begin playback. The window procedure function
       * for the parent window is notified with an
       * MM_MCINOTIFY message when playback is complete.
       * The window procedure then closes the device.
       */
      mciPlayParms.dwCallback = (DWORD) hWndNotify;
      if (dwReturn = mciSendCommand(midi_element, MCI_PLAY,
                                    MCI_NOTIFY, (DWORD)(LPVOID) &mciPlayParms)) 
      {
         mciSendCommand(midi_element, MCI_CLOSE, 0, 0);
         return dwReturn;
      }
      
      debug(("Playing MIDI file, element = %d\n", midi_element));
      ResetMusicVolume();
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
DWORD PlayMusicFile(HWND hWndNotify, char *fname)
{
   if (!has_midi)
      return 0;

#ifdef M59_MSS
   char *ext;

   // If a sequence was paused, resume it
   if (music_pos != 0)
   {
      UnpauseMusic();
      return 0;
   }

   // free memory from previous background music
   if (pMIDIBackground)
      AIL_mem_free_lock(pMIDIBackground);

   // First try MP3 file
   ext = strstr( _strlwr( fname ), ".mid" );
   if( ext != NULL )
      strcpy( ext, ".mp3" );

   // load the file
   pMIDIBackground = (BYTE *) AIL_file_read( fname, NULL );
   if( !pMIDIBackground )
   {
      // Next try xmi file
      ext = strstr(fname, ".mp3" );
      if( ext != NULL )
         strcpy( ext, ".xmi" );
      
      pMIDIBackground = (BYTE *) AIL_file_read( fname, NULL );
      if( !pMIDIBackground )
      {
         debug(( "Failed to load music file %s.\n", fname ));
         return 0;
      }
   }

   // initialize the sequence
   if (!AIL_set_named_sample_file(hseqBackground, fname, pMIDIBackground,
                                  AIL_file_size(fname), 0 ) )
   {
      debug(( "Failed to init music sequence %s.\n", fname ));
      return 0;
   }

   // set to loop indefinitely
   AIL_set_sample_loop_count( hseqBackground, 0 );

   // Set volume
   float vol = ((float) config.music_volume) / CONFIG_MAX_VOLUME;
   AIL_set_sample_volume_levels(hseqBackground, vol, vol );

   // start playing
   AIL_start_sample( hseqBackground );
   debug(( "Playing music file %s.\n", fname ));
   playing_music = True;
   return 0;


#else
   DWORD dwReturn;
   MCI_PLAY_PARMS mciPlayParms;
   char temp[81];

   if ((dwReturn = OpenMidiFile(fname)) != 0)
     {
       debug(("OpenMidiFile error code = %d\n", dwReturn));
       mciGetErrorString(dwReturn, temp, 80);
       debug((temp));
       return dwReturn;
     }

   /* If already playing music, pick up where we left off */
   if (music_pos != 0)
      UnpauseMusic();

   /*
    * Begin playback. The window procedure function
    * for the parent window is notified with an
    * MM_MCINOTIFY message when playback is complete.
    * The window procedure then closes the device.
    */
   mciPlayParms.dwCallback = (DWORD) hWndNotify;
   if (dwReturn = mciSendCommand(midi_element, MCI_PLAY,
                                 MCI_NOTIFY, (DWORD)(LPVOID) &mciPlayParms)) 
   {
      mciGetErrorString(dwReturn, temp, 80);
      debug((temp));
      
      mciSendCommand(midi_element, MCI_CLOSE, 0, 0);
      
      return dwReturn;
   }

   debug(("Playing music file, element = %d\n", midi_element));
   ResetMusicVolume();
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

   dwReturn = mciSendCommand(midi_element, MCI_SEEK,
                             MCI_SEEK_TO_START, (DWORD)(LPVOID) NULL);
   
   mciPlayParms.dwCallback = (DWORD) hMain;
   mciPlayParms.dwFrom = 0;
   if (dwReturn = mciSendCommand(midi_element, MCI_PLAY,
                                 MCI_NOTIFY, (DWORD)(LPVOID) &mciPlayParms)) {
      mciSendCommand(device, MCI_CLOSE, 0, 0);
      
   }
   return dwReturn;
}
/******************************************************************************/
/* 
 * PauseMusic:  Store current position of background music.  Assumes that
 *   music is playing on MIDI device.
 */
void PauseMusic(void)
{
#ifdef M59_MSS
   if (!has_midi)
      return;

   if( AIL_sample_status( hseqBackground ) == SMP_PLAYING )
      AIL_stop_sample( hseqBackground );
   // indicate we are paused
   music_pos = 1;
   debug(( "Pausing music.\n" ));
#else
   MCI_STATUS_PARMS mciStatusParms;

   if (!has_midi)
      return;

   mciStatusParms.dwItem = MCI_STATUS_POSITION;
   mciSendCommand(midi_element, MCI_STATUS, 
                  MCI_STATUS_ITEM, (DWORD)(LPVOID) &mciStatusParms);
   music_pos = mciStatusParms.dwReturn;

   /* Get time format */
   mciStatusParms.dwItem = MCI_STATUS_TIME_FORMAT;
   mciSendCommand(midi_element, MCI_STATUS, 
                  MCI_STATUS_ITEM, (DWORD)(LPVOID) &mciStatusParms);
   time_format = mciStatusParms.dwReturn;

   debug(("Pausing, position = %ld\n", music_pos));
#endif
}
/******************************************************************************/
/*
 * UnpauseMusic:  Restore saved position of background music.
 *   Assumes that background music has been restarted.
 */
void UnpauseMusic(void)
{
#ifdef M59_MSS
   if (!has_midi)
      return;

   if (music_pos)
      AIL_resume_sample(hseqBackground);
   else
      AIL_start_sample(hseqBackground);
   debug(( "Unpausing music.\n" ));
#else
   MCI_SEEK_PARMS mciSeekParms;
   MCI_SET_PARMS  mciSetParms;

   if (!has_midi)
      return;

   /* Set time format */
   mciSetParms.dwTimeFormat = time_format;
   mciSendCommand(midi_element, MCI_SET,
      MCI_SET_TIME_FORMAT, (DWORD)(LPVOID) &mciSetParms);

   mciSeekParms.dwTo = music_pos;
   mciSendCommand(midi_element, MCI_SEEK,
      MCI_TO, (DWORD)(LPVOID) &mciSeekParms);
   debug(("Unpausing to  position = %ld\n", music_pos));
#endif
}
/******************************************************************************/
/*
 * PlayMidiRsc:  Play MIDI file associated with given resource number.
 */
void PlayMidiRsc(ID rsc)
{
   debug(("PlayMidiRsc %d\n", rsc));

   if (!config.play_music || !has_midi)      
      return;

   if (playing_music)
   {
      PauseMusic();
      playing_midi = True;  /* Don't let music start */
#ifndef M59_MSS
      mciSendCommand(midi_element, MCI_CLOSE, 0, 0);
#endif
      PostMessage(hMain, BK_NEWSOUND, SOUND_MIDI, rsc);
      return;
   }

#ifndef M59_MSS
   /* If sound is already going, stop it and prepare to start new one */
   if (playing_midi)
   {
      mciSendCommand(midi_element, MCI_CLOSE, 0, 0); 
      PostMessage(hMain, BK_NEWSOUND, SOUND_MIDI, rsc);
      return;
   }
#endif
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
   if (playing_music)
   { 
      if (rsc != 0 && bg_music != 0 && 
          !stricmp(LookupNameRsc(rsc), LookupNameRsc(bg_music)))
      {
         debug(("Already playing that music.\n" ));
         return;
      }
   }
   
   MusicAbort();

   bg_music = rsc;

   if (!config.play_music || !has_midi || rsc == 0)
      return;

   playing_music = True;
   music_pos = 0;

#ifndef M59_MSS
   /* If sound is already going, wait for it to end */
   if (playing_midi)
      return;
#endif   
   NewMusic(SOUND_MUSIC, rsc);
}
/******************************************************************************/
/*
 * NewMusic:  Start playing new music.  type gives type of sound (MIDI, 
 *   music).  rsc gives resource id of file.
 */
void NewMusic(WPARAM type, ID rsc)
{
   char *filename, fname[MAX_PATH + FILENAME_MAX];
   
   // NULL rsc => abort midi in progress
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
         mciSendCommand(midi_element, MCI_CLOSE, 0, 0); 
#endif
         playing_music = False;
         return;
      }
      return;
   }

   if( (filename = LookupNameRsc(rsc)) == NULL )
      return;

   sprintf(fname, "%s\\%.*s", music_dir, FILENAME_MAX, filename);

   switch (type)
   {
   case SOUND_MIDI:
      PlayMidiFile(hMain, fname);
#ifndef M59_MSS
      debug(("NewMusic MIDI, element = %d\n", midi_element));
#endif
      break;

   case SOUND_MUSIC:
      PlayMusicFile(hMain, fname);
#ifndef M59_MSS
      debug(("NewMusic music, element = %d\n", midi_element));
#endif
      break;
   }
}
#ifdef M59_MSS
/******************************************************************************/
/*
 * MIDIDoneCallback:  A MIDI file has finished playing;
 *                   unpause the background music if necessary
 */
void AILCALLBACK MIDIDoneCallback(HSAMPLE S)
{
   debug(( "At callback...\n" ));
   if (playing_music)
      UnpauseMusic();
}
#else
/******************************************************************************/
//	Not used by MSS version
/*
 * MusicDone:  A MIDI file has finished playing; restart background music 
 *   if appropriate.
 */
void MusicDone(UINT device)
{
   if (playing_midi && device == midi_element)
   {
      playing_midi = False;
      mciSendCommand(midi_element, MCI_CLOSE, 0, 0);   

      debug(("Stopping MIDI device %d\n", device));

      /* Restart background music if appropriate */
      if (playing_music)
         PostMessage(hMain, BK_NEWSOUND, SOUND_MUSIC, bg_music);
      return;
   }

   /* Loop background music */
   if (playing_music && device == midi_element)
   {
      debug(("Stopping Music device %d\n", device));
      RestartMidiFile(midi_element);
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
   }

   playing_music = False;
   music_pos = 0;
}
/******************************************************************************/
/*
 * MusicStart:  Start playing bg music if any (used when player toggles music on).
 */
void MusicStart(void)
{
   if (state == STATE_GAME)
      PlayMusicRsc(bg_music);
}
/******************************************************************************/
/*
 * ResetMusicVolume:  Change music volume in response to a config change
 */
void ResetMusicVolume()
{
   if (!has_midi)
      return;
      
   char command[MAX_PATH + 128];

#ifdef M59_MSS
   // Set volume
   float vol = ((float) config.music_volume) / CONFIG_MAX_VOLUME;
   if (hseqImmediate != NULL)
      AIL_set_sample_volume_levels(hseqImmediate, vol, vol );
   if (hseqBackground != NULL)
      AIL_set_sample_volume_levels(hseqBackground, vol, vol );
#else
   sprintf(command, "setaudio \"%s\" volume to %i", filename, config.music_volume * 10);
   mciSendString(command,NULL,0,NULL);
#endif
}
