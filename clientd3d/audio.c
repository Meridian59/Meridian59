// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* audio.c:  Deals with playing WAV (sound) and MP3 (music).
*           Uses the irrklang audio library (free for non-commercial use).
*           irrklang is capabale of playing 3D sound as well as 2D.
*/

#include "client.h"

using namespace irrklang;

/*********************************************************************/

typedef struct
{
   ISound* id;
   BYTE    flags;
} AudioInfo;

/*********************************************************************/

#define MAXSOUNDS 24                  // maximum simutaneously played sounds
#define AUDIODIR  "resource"          // folder with wav and mp3 files
#define ROLLOFF   0.00016f            // sets how volume decreases with distance
#define MAXDIST   (FINENESS * 32.0f)  // beyond this distance, volume is always 0

BOOL          initialized   = FALSE;  // switch for Init/Shutdown
char*         musicname     = NULL;   // filename of current background music
ISound*       music         = NULL;   // soundid of current background music
char*         lastmusicname = NULL;   // saves the last played music for restart
ISoundEngine* soundengine   = NULL;   // the irrklang engine object

AudioInfo *sounds;

/*********************************************************************/

class SoundStopEventReceiver : public ISoundStopEventReceiver
{
   virtual void OnSoundStopped(ISound* sound, E_STOP_EVENT_CAUSE reason, void* userData) override
   {
      // ignore for music
      if (sound == music)
         return;

      SoundStop(sound);
   }
};

SoundStopEventReceiver* stophandler = new SoundStopEventReceiver();

/*********************************************************************/

BOOL AudioInit()
{
   if (initialized)
      return FALSE;

   // get devicelist
   ISoundDeviceList* deviceList = 
      ::irrklang::createSoundDeviceList();

   if (!deviceList)
      return FALSE;

   // see how many we got
   ik_s32 devicecount = deviceList->getDeviceCount();

   // cleanup list
   deviceList->drop();

   // no devices? bad, there should be at least a NULL device..
   if (devicecount == 0)
      return FALSE;

   // sound engine options, may be adjusted
   irrklang::E_SOUND_ENGINE_OPTIONS options = (irrklang::E_SOUND_ENGINE_OPTIONS)
      (irrklang::E_SOUND_ENGINE_OPTIONS::ESEO_MULTI_THREADED |
       irrklang::E_SOUND_ENGINE_OPTIONS::ESEO_LOAD_PLUGINS |
       irrklang::E_SOUND_ENGINE_OPTIONS::ESEO_USE_3D_BUFFERS);

   irrklang::E_SOUND_OUTPUT_DRIVER driver = irrklang::E_SOUND_OUTPUT_DRIVER::ESOD_AUTO_DETECT;

   // try to initialize irrKlang
   soundengine = ::irrklang::createIrrKlangDevice(driver, options);
	
   if (!soundengine)
      return FALSE;

   // sound engine properties
   soundengine->setDefault3DSoundMaxDistance(MAXDIST);
   soundengine->setDefault3DSoundMinDistance(0.0f);
   soundengine->setRolloffFactor(ROLLOFF);

   // mark initialized
   initialized = true;

   // Allocate memory for sound IDs
   sounds = (AudioInfo *)SafeMalloc(MAXSOUNDS * sizeof(AudioInfo));
   memset(sounds, 0, MAXSOUNDS * sizeof(AudioInfo));

   return TRUE;
}

BOOL AudioShutdown()
{
   if (!initialized)
      return FALSE;

   MusicStop();
   SoundStopAll();

   // shutdown irrklang
   soundengine->stopAllSounds();
   soundengine->drop();
	
   // reset pointers
   soundengine   = NULL;
   music         = NULL;
   musicname     = NULL;
   lastmusicname = NULL;

   // erase sounds data
   for (int i = 0; i < MAXSOUNDS; i++)
   {
      sounds[i].id = 0;
      sounds[i].flags = 0;
   }

   // Free sound mem.
   SafeFree(sounds);

   // mark not initialized
   initialized = false;

   return TRUE;
}

BOOL AudioSetVolume(ISound* soundid, int volume)
{
   if (!initialized || !soundid)
      return FALSE;

   // set volume, irrklang is [0.0-1.0], volume is [0-100]
   soundid->setVolume((float)volume * 0.01f);

   return TRUE;
}

BOOL AudioPlay2D(char *file, int volume, bool loop, ISound** soundid)
{
   // must provide valid ptr for output id and file
   if (!initialized || !soundid || !file)
      return FALSE;

   // check if sound is already loaded to irrklang
   ISoundSource* soundsrc = soundengine->getSoundSource(file, false);
	
   // add it if not
   if (!soundsrc)	
      soundsrc = soundengine->addSoundSourceFromFile(file);
	
   if (!soundsrc)
      return FALSE;

   // prepare 2d playback 
   *soundid = soundengine->play2D(soundsrc,
      loop, true, true, false);

   if (!*soundid)
      return FALSE;

   // register callback when audio finishes
   (*soundid)->setSoundStopEventReceiver(stophandler);

   // set volume
   AudioSetVolume(*soundid, volume);

   // now start playback
   (*soundid)->setIsPaused(false);

   return TRUE;
}

BOOL AudioPlay3D(char *file, int volume, bool loop, ISound** soundid, int x, int y)
{
   // must provide valid ptr for output id and file
   if (!initialized || !soundid || !file)
      return FALSE;

   // check if sound is already loaded to irrklang
   ISoundSource* soundsrc = soundengine->getSoundSource(file, false);

   // add it if not
   if (!soundsrc)
      soundsrc = soundengine->addSoundSourceFromFile(file);

   if (!soundsrc)
      return FALSE;

   // prepare 3d playback
   *soundid = soundengine->play3D(soundsrc, vec3df(x, 0, -y),
      loop, true, true, false);

   if (!*soundid)
      return FALSE;

   // register callback when audio finishes
   (*soundid)->setSoundStopEventReceiver(stophandler);

   // set volume
   AudioSetVolume(*soundid, volume);

   // now start playback
   (*soundid)->setIsPaused(false);

   return TRUE;
}

BOOL AudioStopFile(char *file)
{
   if (!initialized || !sounds || !file)
      return FALSE;

   // Get the sound source data.
   ISoundSource* soundsrc = soundengine->getSoundSource(file, false);

   if (!soundsrc)
      return FALSE;

   for (int i = 0; i < MAXSOUNDS; ++i)
      if (sounds[i].id && sounds[i].id->getSoundSource() == soundsrc)
         SoundStop(sounds[i].id);

   // Remove the sound.
   //soundengine->removeSoundSource(file);

   return TRUE;
}

BOOL AudioStop(ISound* soundid)
{
   if (!initialized || !soundid)
      return false;

   debug(("Stopping Audio: %i\n", soundid));

   // unset auto callback (or it may call this method again)
   soundid->setSoundStopEventReceiver(NULL);

   // stop if not yet finished
   if (!soundid->isFinished())
      soundid->stop();

   soundid->drop();

   return true;
}

/*********************************************************************/

BOOL MusicStop()
{
   // nothing to stop
   if (!initialized || !music)
      return FALSE;

   // stop it
   AudioStop(music);

   // keep last played file for restart
   lastmusicname = musicname;

   // reset tracked values
   musicname = NULL;
   music     = NULL;

   return TRUE;
}

BOOL MusicRestart()
{
	if (!lastmusicname)
		return FALSE;

	return MusicPlayFile(lastmusicname);
}

BOOL MusicPlayFile(char* file)
{
   // nothing to do if music is disabled, null file given
   // or trying to play current file (must compare by string rather than id)
   if (!config.play_music || !initialized || !file || (musicname && stricmp(file, musicname) == 0))
      return TRUE;

   debug(("Trying to play music: %s  (volume:%i)\n", file, config.music_volume));

   char current_dir[MAX_PATH];              // workdir
   char filename[MAX_PATH + FILENAME_MAX];  // full filename

   // get working directory
   if (!GetWorkingDirectory(current_dir, MAX_PATH))
      return FALSE;

   // combine workdir, resource subfoldername and file into filename
   sprintf(filename, "%s%s\\%s", current_dir, AUDIODIR, file);

   // stop currently playing music first
   MusicStop();

   // try start music playback
   if (!AudioPlay2D(filename, config.music_volume, true, &music))
      return FALSE;

   // save currently played music filename
   musicname = file;

   return TRUE;
}

BOOL MusicPlayResource(ID rsc)
{
   // nothing to do if music disabled
   if (!config.play_music) 
      return TRUE;

   // try get filename for resource id
   char* file = LookupNameRsc(rsc);

   if (!file)
      return FALSE;

   // call variant with filename parameter
   return MusicPlayFile(file);
}

BOOL MusicSetVolume()
{
   // no music? nothing to do but ok
   if (!music)
      return TRUE;

   // set volume to configsetting for music
   return AudioSetVolume(music, config.music_volume);
}

/*********************************************************************/

__inline int SoundGetFreeIndex()
{
   if (initialized && sounds)
      for (int i = 0; i < MAXSOUNDS; i++)	
         if (sounds[i].id == 0)		
            return i;

   return -1;
}

BOOL SoundStop(ISound* soundid)
{
   if (!initialized || !soundid || !sounds)
      return TRUE;

   // try remove it from our array
   for (int i = 0; i < MAXSOUNDS; i++)
   {
      if (sounds[i].id == soundid)
      {
         sounds[i].id    = 0;
         sounds[i].flags = 0;
         break;
      }
   }

   // try stop it
   if (!AudioStop(soundid))
      return FALSE;

   return TRUE;
}

BOOL SoundStopAll(BYTE flags)
{
   // stop any matching flags (by default all)
   if (initialized && sounds)
      for (int i = 0; i < MAXSOUNDS; i++)
         if ((sounds[i].flags & flags) == flags)
            SoundStop(sounds[i].id);

   return TRUE;
}

BOOL SoundPlayFile(char* file, BYTE flags, int x, int y)
{
   bool isLoop   = ((flags & SF_LOOP) == SF_LOOP);
   bool isRandom = ((flags & SF_RANDOM_PLACE) == SF_RANDOM_PLACE);

   // abort cases
   if (!config.play_sound || !file || !sounds ||
      (!config.play_loop_sounds && isLoop) ||
      (!config.play_random_sounds && isRandom))
         return FALSE;

   // get index where to store sound
   int index = SoundGetFreeIndex();

   // maximum simultaneously sounds reached
   if (index < 0)
   {
      debug(("Skipping sound (max reached): %s\n", file));
      return FALSE;
   }

   char current_dir[MAX_PATH];              // workdir
   char filename[MAX_PATH + FILENAME_MAX];  // full filename

   // get working directory
   if (!GetWorkingDirectory(current_dir, MAX_PATH))
      return FALSE;

   // combine workdir, resource subfoldername and file into filename
   sprintf(filename, "%s%s\\%s", current_dir, AUDIODIR, file);

   if (x == 0 && y == 0)
   {
      debug(("Trying to play 2D sound: %s\n", file));

      // try start 2d sound playback with fixed volume no coordinates provided
      if (!AudioPlay2D(filename, config.sound_volume, isLoop, &sounds[index].id))
         return FALSE;
   }
   else
   {
      debug(("Trying to play 3D sound: %s at pos(%.5i,%.5i)\n", file, x, y));

      // try start 3d sound playback with coordinates provided
      if (!AudioPlay3D(filename, config.sound_volume, isLoop, &sounds[index].id, x, y))
         return FALSE;
   }

   // save flags
   sounds[index].flags = flags;

   return TRUE;
}

BOOL SoundStopFile(char* file, ID obj)
{
   // abort cases
   if (!initialized || !config.play_sound || !file)
      return FALSE;

   char current_dir[MAX_PATH];              // workdir
   char filename[MAX_PATH + FILENAME_MAX];  // full filename

   // get working directory
   if (!GetWorkingDirectory(current_dir, MAX_PATH))
      return FALSE;

   // combine workdir, resource subfoldername and file into filename
   sprintf(filename, "%s%s\\%s", current_dir, AUDIODIR, file);

   if (!AudioStopFile(filename))
     return FALSE;

   return TRUE;
}

BOOL SoundPlayResource(ID rsc, BYTE flags, int x, int y)
{
   // nothing to do if sound disabled
   if (!config.play_sound)
      return TRUE;

   // try get filename for resource id
   char* file = LookupNameRsc(rsc);

   if (!file)
      return FALSE;

   // call variant with filename parameter
   return SoundPlayFile(file, flags, x, y);
}

BOOL SoundStopResource(ID rsc, ID obj)
{
   // nothing to do if sound disabled
   if (!config.play_sound)
      return TRUE;

   // try get filename for resource id
   char* file = LookupNameRsc(rsc);

   if (!file)
      return FALSE;

   // call variant with filename parameter
   return SoundStopFile(file, obj);
}

BOOL SoundSetVolume()
{
   if (initialized && sounds)
      for (int i = 0; i < MAXSOUNDS; i++)
         if (sounds[i].id)
            AudioSetVolume(sounds[i].id, config.sound_volume);

   return TRUE;
}

BOOL SoundSetListenerPosition(int x, int y, int angle)
{
   if (!initialized)
      return FALSE;

   // remove full periods, convert pseudo degree (0-4096) to radian
   // and then to degree (afterwards angle is 0-360°)
   angle %= NUMDEGREES;
   angle = RADIANS_TO_DEGREES(DegToRad(angle));
	
   // turn the degree into a direction vector
   vec3df dir = vec3df(1, 0, 0);
   dir.rotateXZBy(angle, vec3df(0, 0, 0));

   //debug(("Move sound listener - angle:%.4i pos:(%.5i,%.5i) dir:(%.2f,%.2f,%.2f)\n", 
   //   angle, x, y, dir.X, dir.Y, dir.Z));

   // update listener position and direction
   soundengine->setListenerPosition(vec3df(x, 0, -y), vec3df(dir.X, 0, -dir.Z));

   return TRUE;
}
