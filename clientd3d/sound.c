// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
 * sound.c:  Deal with playing sound files.
 *
 *  Sounds are played through a wave mixer DLL, allowing up to 8 simultaneous sounds.
 *  When a sound ends, the main window procedure receives a message and calls SoundDone
 *  to close the handle of the wave file.
 */
#include "client.h"
static char sound_dir[] = "resource";   /* Directory for sound files */

#ifdef M59_MSS
#define SOUND_MAX_VOICES 16
static int iNumVoices;

// user defined data associated with sample handles
#define SOUND_USER_ADDRESS 0  // memory address of sample
#define SOUND_USER_X 1        // x and y position of sound source
#define SOUND_USER_Y 2
#define SOUND_USER_RADIUS 3   // maximum distance sound is audible
#define SOUND_USER_MAXVOL 4   // maximum volume of sound (when distance = 0)

static HSAMPLE SampleHandle[SOUND_MAX_VOICES];  // array of sample handles

static 	HDIGDRIVER hDriver;                    // handle to the digital audio driver
#else
#define SOUND_MAX_VOICES 8
static int iNumVoices;           // keeps a count of playing samples

static HANDLE hMixSession;       // Handle for Wavemix DLL

static Bool wave_open = False;   // Is Wavemix open & ready to play sounds?

typedef struct {
LPMIXWAVE hSample;
int x;
int y;
int radius;
int cur_vol;
int max_vol;
} SAMPLE;                                 // struct to hold sample data

static SAMPLE samples[SOUND_MAX_VOICES];  // array of sample data

#endif

/******************************************************************************/
/*
 * SoundInitialize:  Called at startup; initialize sound and MIDI devices.
 */
void SoundInitialize(void)
{

   if (config.soundLibrary == LIBRARY_NIL)
   {
      wave_open = FALSE;
      return;
   }

#ifdef M59_MSS

   {
      int i;
      for( i = 0; i < SOUND_MAX_VOICES; i++ )
         SampleHandle[i] = NULL;

        wave_open = FALSE;
        debug(("SoundInitialize: starting library\n"));

      // it's ok to do this more than once, and we do,
      //	once in MusicInitialize and once here
      AIL_startup();

      // prefer a 22K device, to match our WAV files
      hDriver = WAVE_init_driver(22050, 8, 1);
      // next choice is a 44K device
      if( hDriver == NULL )
         hDriver = WAVE_init_driver(44100, 8, 1);
      // finally try an 11K device
      if( hDriver == NULL )
         hDriver = WAVE_init_driver(11025, 8, 1);

      // prefer a 22K device, to match WAV files
      if( hDriver == NULL )
         hDriver = WAVE_init_driver(22050, 8, 1);
      // next choice is a 44K device
      if( hDriver == NULL )
         hDriver = WAVE_init_driver(44100, 8, 1);
      // finally try an 11K device
      if( hDriver == NULL )
         hDriver = WAVE_init_driver(11025, 8, 1);
      if( hDriver == NULL )
      {
         debug(( "MSS digital sound failed to initialize.\n" ));
         wave_open = FALSE;
         return;
      }

      // allocate sample handles
      for( i=0; i < SOUND_MAX_VOICES; i++ )
      {
         SampleHandle[i] = AIL_allocate_sample_handle( hDriver );

         // clear user data (sample addresses, looper locations)
         AIL_set_sample_user_data( SampleHandle[i], SOUND_USER_ADDRESS, 0 );
         AIL_set_sample_user_data( SampleHandle[i], SOUND_USER_X, 0 );
         AIL_set_sample_user_data( SampleHandle[i], SOUND_USER_Y, 0 );
         AIL_set_sample_user_data( SampleHandle[i], SOUND_USER_RADIUS, 0 );
         AIL_set_sample_user_data( SampleHandle[i], SOUND_USER_MAXVOL, 0 );

         if( SampleHandle[i] == NULL )
            break;
      }

      iNumVoices = i;
      debug(( "MSS digital sound initialized successfully.\n" ));
      debug(( "%i simultaneous voices available.\n", iNumVoices ));

      wave_open = True;
   }

#else

   {
   MCI_SYSINFO_PARMS mciSysinfoParms;
   DWORD num_devices, retval;
   Bool has_wave = False;

   /* See if WAV player is present */
   mciSysinfoParms.lpstrReturn = (LPSTR) &num_devices;
   mciSysinfoParms.dwRetSize = sizeof(num_devices);
   mciSysinfoParms.wDeviceType = MCI_DEVTYPE_WAVEFORM_AUDIO;
   retval = mciSendCommand(0, MCI_SYSINFO, MCI_SYSINFO_QUANTITY,
                           (DWORD) &mciSysinfoParms);
   if (retval == 0 && num_devices > 0)
      has_wave = True;

   /* Start Wave mixer */
   if (has_wave)
   {
      if ((hMixSession = WaveMixInit()) == NULL)
      {
         debug(("WaveMixInit returned error\n"));
         wave_open = False;
      }
      else 
         wave_open = True;

      if (WaveMixOpenChannel(hMixSession, 0, WMIX_ALL))
      {
         debug(("WaveMixOpenChannel returned error\n"));
         wave_open = False;
      }
      if (WaveMixActivate(hMixSession, TRUE) != MMSYSERR_NOERROR)
      {
         debug(("WaveMixActivate returned error\n"));
         wave_open = False;
      }
   }
   }
#endif
}
#ifdef M59_MSS
/******************************************************************************/
/*
 * WAVE_init_driver: Initialize a digital output driver
 */
static HDIGDRIVER WAVE_init_driver( DWORD rate, WORD bits, WORD chans )
{
   char buf[128];

   HDIGDRIVER DIG = AIL_open_digital_driver(rate, bits, chans, 0);

   if (DIG == NULL)
   {
      debug((AIL_last_error()));
      return NULL;
   }

   strcpy(buf,"Device: ");
   AIL_digital_configuration(DIG,0,0,buf+strlen(buf));
   debug(("%s\n",buf));

   return DIG;
}
#endif
/******************************************************************************/
/*
 * SoundClose: Close all sound and music devices.
 */
void SoundClose(void)
{
   if (wave_open)
   {
#ifdef M59_MSS

      int npFreeUps = 0, i, j;
      void* apSampleAddresses[SOUND_MAX_VOICES];

      void *pSampleAddress;

      debug(("SoundClose: ending samples\n"));

      // end any samples playing to clean up their handles and memory

      for (j = 0; j < SOUND_MAX_VOICES; j++)
      apSampleAddresses[j] = NULL;

      for ( i = 0; i < iNumVoices; i++ )
      {
         //debug(("Releasing sound handle %i\n",i ));

         pSampleAddress = (void *)AIL_sample_user_data(SampleHandle[i], SOUND_USER_ADDRESS);
         AIL_release_sample_handle(SampleHandle[i]);

         if (pSampleAddress)
         {
            for (j = 0; j < npFreeUps; j++)
            {
               if (apSampleAddresses[j] == pSampleAddress)
               {
                  break;
               }
            }
            if (j >= npFreeUps)
               apSampleAddresses[npFreeUps++] = pSampleAddress;
         }
      }

      debug(("SoundClose: pausing\n"));

      Sleep(1500);

      debug(("SoundClose: freeing samples\n"));

      for (j = 0; j < npFreeUps; j++)
      AIL_mem_free_lock(apSampleAddresses[j]);

      debug(("SoundClose: done\n"));

      // Don't want to shutdown, since music may still be playing
      // Done in MusicClose() instead
      //AIL_shutdown();

#else
      WaveMixCloseChannel(hMixSession, 0, WMIX_ALL);
      WaveMixCloseSession(hMixSession);
#endif
      wave_open = False;
   }
}
/******************************************************************************/
/*
 * PlayWaveFile:  Send given wave file to mixer to play.
 *   Sound is played at given volume, where 0 = silence and MAX_VOLUME = normal volume
 *	 Looping sounds update volume as player moves using row, col for sound source location
 *		and radius and max_vol as attenuation parameters
 * Returns 0 on success; nonzero on error.
 */
UINT PlayWaveFile(HWND hwnd, char *fname, int volume, BYTE flags, int src_row, int src_col, int radius, int max_vol)
{
   if (!wave_open)
      return TRUE;

   // Quick configuration escape for especially annoying sound types.
   if ((flags & SF_LOOP) && (!config.play_loop_sounds))
      return 0;
   if ((flags & SF_RANDOM_PLACE) && (!config.play_random_sounds))
      return 0;
      
#ifdef M59_MSS

   int i, iRandomPitch;
   void FAR *pSample;
   debug(("Reading sound file %s.\n",fname));
   // let Miles allocate the memory
   pSample=AIL_file_read( fname, NULL );

   if( pSample == NULL )
   {
      debug(( "Error reading sound file %s.\n", fname ));
      return 1L;
   }

   // Find an HSAMPLE to play it
   for( i=0; i < iNumVoices; i++ )
   {
      if( AIL_sample_user_data( SampleHandle[i], SOUND_USER_ADDRESS ) == 0 )
         break;
   }

   // If no samples available, ignore request
   if( i == iNumVoices )
   {
      debug(("Too many sounds to play at once!\n"));
      AIL_mem_free_lock( pSample );
      return 1L;
   }

   if (!AIL_set_named_sample_file(SampleHandle[i], fname, pSample,
                                  AIL_file_size(fname), 0))
   {
      debug(("Error initializing sound sample.\n" ));
      AIL_mem_free_lock( pSample );
      return 1L;
   }

   // Set initial volume and rate
   float vol = (float) volume / MAX_VOLUME;
   vol = vol * config.sound_volume / CONFIG_MAX_VOLUME;
   AIL_set_sample_volume_levels( SampleHandle[i], vol, vol );

   // If random pitch flag is set, diddle the pitch  by +/- 10%
   if( flags & SF_RANDOM_PITCH )
   {
      iRandomPitch = ( rand() % 21 - 10 + 100 ) * AIL_sample_playback_rate( SampleHandle[i] ) / 100;
      AIL_set_sample_playback_rate( SampleHandle[i],iRandomPitch);
   }

   // Set it to loop if flag is set
   if( flags & SF_LOOP )
   {
      AIL_set_sample_loop_count( SampleHandle[i], 0 );	// 0 = infinite
      // debug(( "Looping sound on voice %i.\n",i ));

      // save x and y pos of sound source, and cutoff radius and max volume
      // debug(("Sound source at: (%i,%i)\n",src_col,src_row ));
      AIL_set_sample_user_data( SampleHandle[i], SOUND_USER_X, src_col );
      AIL_set_sample_user_data( SampleHandle[i], SOUND_USER_Y, src_row );
      AIL_set_sample_user_data( SampleHandle[i], SOUND_USER_RADIUS, radius );
      AIL_set_sample_user_data( SampleHandle[i], SOUND_USER_MAXVOL, max_vol );
   // debug(("Setting volume to %i.\n",volume));
   }

   // Finally, activate sample
   //	debug(("Playing sound with voice %i, flags=%i.\n",i,flags));
   AIL_start_sample( SampleHandle[i] );
   AIL_set_sample_user_data( SampleHandle[i], SOUND_USER_ADDRESS, (S32) pSample );

   //	register callback to free memory allocated
   AIL_register_EOS_callback( SampleHandle[i], SoundDoneCallback );

   return 0;
#else
   MIXPLAYPARAMS params;
   LPMIXWAVE lpWave;
   UINT retval;
   
   // if there are already too many channels in use, don't continue
   if (iNumVoices >= SOUND_MAX_VOICES)
       return 0;
   
   // don't even continue if volume is 0xFFFF
   if (volume == 0)
      return 0;
      
   if (volume > max_vol)
      volume = max_vol;
      
   volume = volume * config.sound_volume / CONFIG_MAX_VOLUME;
   
   lpWave = WaveMixOpenWave(hMixSession, fname, hInst, WMIX_FILE, volume);
   
   if (lpWave == NULL)
   {
      debug(("Error opening wave file %s\n", fname));
      return 1;
   }
   
   /* Send sound to any available channel; play exactly once */
   /* hwnd is sent a MM_WOM_DONE message when the sound has finished playing */
   params.wSize = sizeof(MIXPLAYPARAMS);
   params.hMixSession = hMixSession;
   params.iChannel = 0;
   params.lpMixWave = lpWave;
   params.hWndNotify = hwnd;
   params.dwFlags = WMIX_CLEARQUEUE | WMIX_HIPRIORITY | WMIX_USELRUCHANNEL;
   params.wLoops = (flags & SF_LOOP) ? 0xFFFF : 0;
   
   if (retval = WaveMixPlay(&params)) 
      debug(("WaveMixPlay failed, error = %d\n", retval));

   return retval;
#endif
}
/******************************************************************************/
/*
 * PlayWaveRsc:  Play wave file associated with given resource number.
 *   Sound is played at given volume, where 0 = silence and MAX_VOLUME = normal volume
 *	 Looping sounds update volume as player moves using row, col for sound source location
 *		and radius and max_vol as attenuation parameters
 */
void PlayWaveRsc(ID rsc, int volume, BYTE flags, int row, int col, int radius, int max_vol)
{
   char *filename, fname[MAX_PATH + FILENAME_MAX], game_path[MAX_PATH];
   
   if (!config.play_sound || !wave_open)
      return;

   if ((filename = LookupNameRsc(rsc)) == NULL)
   {
      debug(("Couldn't find filename for wave resource %s\n", rsc));
      return;
   }

   GetGamePath(game_path);
   sprintf(fname, "%s%s\\%.*s", game_path, sound_dir, FILENAME_MAX, filename);
   
   PlayWaveFile(hMain, fname, volume, flags, row, col, radius, max_vol);
}
/******************************************************************************/
/*
 * SoundDone:  A sound has finished playing; close appropriate device element.
 */
#ifdef M59_MSS
void SoundDone( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
   int i;
   HSAMPLE hSample;
   void *pSampleAddress;
   hSample = (HSAMPLE) wParam;

   for( i = 0; i < iNumVoices; i++ )
   {
      if( SampleHandle[i] == hSample )
      {
         if( AIL_sample_status( SampleHandle[i] ) == SMP_FREE )
         {
            // debug(("Sound handle %i already marked as free.\n", i));
            return;  //  SoundClose has already freed the memory, in this case
         }
         else if( AIL_sample_status( SampleHandle[i] ) != SMP_DONE )
         {
            debug(("Can't free memory, since sound %i still playing!\n", i));
            debug(("Re-posting message...\n"));
            PostMessage( hMain, MM_WOM_DONE, (WPARAM) hSample, 0 );
            return;
         }

         pSampleAddress = (void *)AIL_sample_user_data( SampleHandle[i], SOUND_USER_ADDRESS );

         AIL_mem_free_lock( pSampleAddress );
         AIL_set_sample_user_data( SampleHandle[i], SOUND_USER_ADDRESS, 0 );
         break;
      }
   }

   if( i >= iNumVoices )
      debug(("Sound error: tried to free memory for nonexistent handle!"));
}
#else
void SoundDone(HWND hwnd, int iChannel, LPMIXWAVE lpMixWave)
{

   WaveMixFreeWave(hMixSession, lpMixWave);
   // debug(("Stopping sound, channel = %d\n", iChannel));
   
}

#endif
/******************************************************************************/
/*
 * SoundDoneCallback:  A sound has finished playing; send a windows message to invoke SoundDone.
 *                Would love to call SoundDone directly, but can't call mem_free_lock from
 *                within the callback, so post a Windows message instead
 */                             /*  */
#ifdef M59_MSS
void AILCALLBACK SoundDoneCallback( HSAMPLE S )
{
   PostMessage( hMain, MM_WOM_DONE, (WPARAM) S, 0 );
}
#endif
/******************************************************************************/
/*
 * UpdateLoopingSounds:  Adjust the volume of each looping sound
 *    to reflect a change in the player's position
 */
void UpdateLoopingSounds( int px, int py)
{
#ifdef M59_MSS
   int i;

   for( i=0; i < iNumVoices; i++ )
   {
      if( AIL_sample_status( SampleHandle[i] ) != SMP_DONE )
      {
         if( AIL_sample_loop_count( SampleHandle[i] ) == 0 )
         {
            // looping sound volume falls off linearly w/ distance
            int vol = 0;
            int dx = px - (int)AIL_sample_user_data( SampleHandle[i], SOUND_USER_X );
            int dy = py - (int)AIL_sample_user_data( SampleHandle[i], SOUND_USER_Y );
            int cutoff = (int)AIL_sample_user_data( SampleHandle[i], SOUND_USER_RADIUS );
            int maxvolume = (int)AIL_sample_user_data( SampleHandle[i], SOUND_USER_MAXVOL );
            int distance = Distance(dx, dy);
            if (distance < cutoff)
            {
               vol = maxvolume - (distance * maxvolume / cutoff);
            }
            // debug(("Distance = %i\n",distance));
            // debug(("Setting volume to %i.\n",vol));
            float volume = (float) vol / MAX_VOLUME;
            volume = volume * config.sound_volume / CONFIG_MAX_VOLUME;
            AIL_set_sample_volume_levels( SampleHandle[i], volume, volume );
         }
      }
   }
#else
// Wavemix doesn't appear to support changing volume of an already playing wave.
// There is a newer version of wavemix (1.80) that I tried which claimed to be
// able to do so, but the method it uses changes the sound in such a way that the
// more you call it, the worst the wav sounds as it is played (really bad)
#endif
}
/******************************************************************************/
/*
 * SoundAbort:  Turn off currently playing sounds
 */
void SoundAbort(void)
{
   if (wave_open)
   {
#ifdef M59_MSS
      int i;
      // debug(("Shutting off all sounds.\n"));
      // end any samples playing to clean up their handles and memory
      for( i = 0; i < iNumVoices; i++ )
      {
         if( SampleHandle[i] != NULL )
            if( AIL_sample_status( SampleHandle[i] ) != SMP_DONE )
               AIL_end_sample( SampleHandle[i] );
      }
#else

      /* Stop all playing sounds */
      WaveMixFlushChannel(hMixSession, 0, WMIX_ALL | WMIX_NOREMIX);
   
#endif
   }
}
