// Meridian 59, Copyright 1994-2012 Andrew Kirmse and Chris Kirmse.
// All rights reserved.
//
// This software is distributed under a license that is described in
// the LICENSE file that accompanies it.
//
// Meridian is a registered trademark.
/*
* audio.c:  Deals with playing WAV (sound) and MP3 (music).
*           Uses winmm.lib / windows components for the job.
*           Particularly Windows Media Player components.
*           Windows N-Edition users might have to 
*           install Media Feature Pack
* 
*/

#include "client.h"
#include <Digitalv.h>

/*********************************************************************/

typedef struct
{
	MCIDEVICEID id;
	BYTE        flags;
} AudioInfo;

/*********************************************************************/

#define MAXSOUNDS              8
#define AUDIODIR               "resource"   // folder with wav and mp3 files

int         aliascounter      = 0;           // used for unique sound aliases
char*       musicname         = NULL;        // filename of current background music
MCIDEVICEID music             = NULL;        // soundid of current background music

AudioInfo sounds[MAXSOUNDS] = { 
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, 
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, };

/*********************************************************************/

MCIERROR AudioPlay(char *file, bool loop, HWND notifywnd, MCIDEVICEID* soundid)
{
	MCIERROR error;
	MCI_PLAY_PARMS mciPlayParms;
	MCI_OPEN_PARMS mciOpenParms;
	char alias[16];

	// must provide valid ptr for output id
	if (!soundid)
		return MCIERR_MISSING_PARAMETER;

	/********************************************************************/
	// 1) Raise our unique 'soundalias' (implemented by simple counter)
	sprintf_s(alias, "%i", aliascounter);
	aliascounter++;

	/********************************************************************/
	// 2) Open File	
	// set params for opening ('MPEGVideo' supports WAV and MP3)
	// must provide an unique 'alias' ourself when playing same file simultaneously
	memset(&mciOpenParms, 0, sizeof(MCI_OPEN_PARMS));
	mciOpenParms.lpstrDeviceType = "MPEGVideo";
	mciOpenParms.lpstrElementName = file;
	mciOpenParms.lpstrAlias = alias;

	// execute
	error = mciSendCommand(0, MCI_OPEN,
		MCI_OPEN_ELEMENT | MCI_OPEN_TYPE | MCI_OPEN_SHAREABLE | MCI_OPEN_ALIAS,
		(DWORD_PTR)&mciOpenParms);

	// fail
	if (error)
		return error;

	/********************************************************************/
	// 3) Start Playback	
	// set params for playback
	// notifywindow receives MM_MCINOTIFY when complete)
	memset(&mciPlayParms, 0, sizeof(MCI_PLAY_PARMS));
	mciPlayParms.dwCallback = (DWORD)notifywnd;

	DWORD flags = MCI_NOTIFY;
	
	// possibly enable looping playback
	if (loop)
		flags |= MCI_DGV_PLAY_REPEAT;

	// execute
	error = mciSendCommand(mciOpenParms.wDeviceID, MCI_PLAY,
		flags, (DWORD_PTR)&mciPlayParms);

	// fail
	if (error)
	{
		mciSendCommand(mciOpenParms.wDeviceID, MCI_CLOSE, 0, 0);
		return error;
	}

	// set output parameter to id
	*soundid = mciOpenParms.wDeviceID;

	// all good
	return MMSYSERR_NOERROR;
}

MCIERROR AudioStop(MCIDEVICEID soundid)
{
	return mciSendCommand(soundid, MCI_CLOSE, 0, 0);
}

MCIERROR AudioSetVolume(MCIDEVICEID soundid, int volume)
{
	// set params for volume adjusting
	MCI_DGV_SETAUDIO_PARMS mciSetParms;
	memset(&mciSetParms, 0, sizeof(MCI_DGV_SETAUDIO_PARMS));
	mciSetParms.dwItem  = MCI_DGV_SETAUDIO_VOLUME;
	mciSetParms.dwValue = volume * 10; // our volume [0-100], mci [0-1000]

	// execute
	return mciSendCommand(soundid, MCI_SETAUDIO,
		MCI_DGV_SETAUDIO_ITEM | MCI_DGV_SETAUDIO_VALUE,
		(DWORD_PTR)&mciSetParms);
}

/*********************************************************************/

BOOL MusicStop()
{
	// nothing to stop
	if (!music)
		return TRUE;

	if (MMSYSERR_NOERROR != AudioStop(music))
		return FALSE;

	// reset tracked values
	musicname = NULL;
	music = NULL;

	return TRUE;
}

BOOL MusicPlayFile(char* file)
{
	// nothing to do if music is disabled, null file given
	// or trying to play current file (must compare by string rather than id)
	if (!config.play_music || !file || (musicname && stricmp(file, musicname) == 0))
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

	// try start music playback, no callback on finish 
	// because music is played in loop until manually ended
	if (MMSYSERR_NOERROR != AudioPlay(filename, true, NULL, &music))
		return FALSE;

	// set volume to configsetting for music
	if (MMSYSERR_NOERROR != AudioSetVolume(music, config.music_volume))
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
	if (MMSYSERR_NOERROR != AudioSetVolume(music, config.music_volume))
		return FALSE;

	return TRUE;
}

/*********************************************************************/

__inline int SoundGetFreeIndex()
{
	for (int i = 0; i < MAXSOUNDS; i++)	
		if (sounds[i].id == 0)		
			return i;
		
	return -1;
}

BOOL SoundStop(MCIDEVICEID soundid)
{
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
	if (MMSYSERR_NOERROR != AudioStop(soundid))
		return FALSE;

	return TRUE;
}

BOOL SoundStopAll(BYTE flags)
{
	// stop any matching flags (by default all)
	for (int i = 0; i < MAXSOUNDS; i++)
		if ((sounds[i].flags & flags) == flags)
			SoundStop(sounds[i].id);

	return TRUE;
}

BOOL SoundPlayFile(char* file, int volume, BYTE flags)
{
	bool isLoop = ((flags & SF_LOOP) == SF_LOOP);
	bool isRandom = ((flags & SF_RANDOM_PLACE) == SF_RANDOM_PLACE);

	// abort cases
	if (!config.play_sound || !file ||
		(!config.play_loop_sounds && isLoop) ||
		(!config.play_random_sounds && isRandom))
		return TRUE;

	// apply the config sound volume on the base volume
	// volume param and config value both should between 0-100 (%)
	// ex.: a sound at volume 50% and config volume 50% will be played at 25%
	float scale = (float)config.sound_volume / (float)CONFIG_MAX_VOLUME;
	volume = (int)((float)volume * scale);

	debug(("Trying to play sound: %s  (volume:%i)\n", file, volume));

	// get index where to store sound
	int index = SoundGetFreeIndex();

	// maximum simultaneously sounds reached
	if (index < 0)
		return FALSE;

	char current_dir[MAX_PATH];              // workdir
	char filename[MAX_PATH + FILENAME_MAX];  // full filename

	// get working directory
	if (!GetWorkingDirectory(current_dir, MAX_PATH))
		return FALSE;

	// combine workdir, resource subfoldername and file into filename
	sprintf(filename, "%s%s\\%s", current_dir, AUDIODIR, file);

	// try start sound playback, callback to mainwindow on finish
	// save id in our array
	if (MMSYSERR_NOERROR != AudioPlay(filename, isLoop, hMain, &sounds[index].id))
		return FALSE;

	// set volume to configsetting combined with invididual volume for sound
	if (MMSYSERR_NOERROR != AudioSetVolume(sounds[index].id, volume))
		return FALSE;

	// save flags
	sounds[index].flags = flags;

	return TRUE;
}

BOOL SoundPlayResource(ID rsc, int volume, BYTE flags)
{
	// nothing to do if sound disabled
	if (!config.play_sound)
		return TRUE;

	// try get filename for resource id
	char* file = LookupNameRsc(rsc);

	if (!file)
		return FALSE;

	// call variant with filename parameter
	return SoundPlayFile(file, volume, flags);
}

BOOL SoundSetListenerPosition(int x, int y)
{
	return TRUE;
}
