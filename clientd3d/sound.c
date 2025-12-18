// Meridian 59 sound.c - sound effects via OpenAL Soft

#include "client.h"

static char sound_dir[] = "resource";   /* Directory for sound files */
static bool wave_open = false;

/* Ambient tracking for room transitions */
static const int MAX_AMBIENTS = 256;
struct AmbientEntry {
	char *name;
	int category;
};

static AmbientEntry prev_ambients[MAX_AMBIENTS];
static int prev_ambient_count = 0;
static AmbientEntry curr_ambients[MAX_AMBIENTS];
static int curr_ambient_count = 0;
static bool ambient_cleanup_pending = false;

// Forward to audio layer helper
void Audio_StopSourcesForFilename(const char* filename);

static void free_ambient_list_entries(AmbientEntry *list, int *count)
{
	for (int i = 0; i < *count; i++)
	{
		free(list[i].name);
		list[i].name = NULL;
		list[i].category = 0;
	}
	*count = 0;
}

/* Mark current looping ambients for cleanup; newly registered ambients
   will be recorded in curr_ambients and protected from cleanup. */
void Sound_BeginAmbientTransition(void)
{
	// move current (curr) into prev
	free_ambient_list_entries(prev_ambients, &prev_ambient_count);
	for (int i = 0; i < curr_ambient_count && i < MAX_AMBIENTS; i++)
	{
		prev_ambients[i].name = _strdup(curr_ambients[i].name);
		prev_ambients[i].category = curr_ambients[i].category;
	}
	prev_ambient_count = curr_ambient_count;
	// clear current
	free_ambient_list_entries(curr_ambients, &curr_ambient_count);
	ambient_cleanup_pending = true;
}

/* Register a looping ambient filename as active for the new room. */
static int infer_category_from_name(const char *name)
{
	if (!name) return SOUND_CAT_ATMOS;
	char lower[MAX_PATH];
	strncpy_s(lower, MAX_PATH, name, _TRUNCATE);
	_strlwr_s(lower, MAX_PATH);
	// Basic heuristics
	if (strstr(lower, "fountain") || strstr(lower, "water") || strstr(lower, "flow") || strstr(lower, "gurgle"))
		return SOUND_CAT_STEADY;
	return SOUND_CAT_ATMOS;
}

void Sound_RegisterAmbientFilenameWithCategory(const char *filename, int category)
{
	if (!filename || filename[0] == '\0')
		return;

	// add to curr if not present
	for (int i = 0; i < curr_ambient_count; i++)
	{
		if (_stricmp(curr_ambients[i].name, filename) == 0)
			return;
	}
	if (curr_ambient_count < MAX_AMBIENTS)
	{
		curr_ambients[curr_ambient_count].name = _strdup(filename);
		curr_ambients[curr_ambient_count].category = category;
		curr_ambient_count++;
	}

	// If cleanup pending, remove this filename from prev list so it won't be stopped
	if (ambient_cleanup_pending)
	{
		for (int i = 0; i < prev_ambient_count; i++)
		{
			if (_stricmp(prev_ambients[i].name, filename) == 0)
			{
				free(prev_ambients[i].name);
				// shift down
				for (int j = i; j < prev_ambient_count - 1; j++)
					prev_ambients[j] = prev_ambients[j+1];
				prev_ambients[prev_ambient_count - 1].name = NULL;
				prev_ambient_count--;
				i--;
			}
		}
	}
}

void Sound_RegisterAmbientFilename(const char *filename)
{
	int cat = infer_category_from_name(filename);
	Sound_RegisterAmbientFilenameWithCategory(filename, cat);
}

/* Stop any ambients that remain in prev_ambients (not re-registered) */
void Sound_EndAmbientTransition(void)
{
	if (!ambient_cleanup_pending)
		return;

	for (int i = 0; i < prev_ambient_count; i++)
	{
		if (prev_ambients[i].name)
			Audio_StopSourcesForFilename(prev_ambients[i].name);
	}

	// Only clear prev_ambients - keep curr_ambients so they become prev on next transition!
	free_ambient_list_entries(prev_ambients, &prev_ambient_count);
	ambient_cleanup_pending = false;
}

void SoundInitialize(void)
{
	/* Initialize audio backend here if desired. For now, mark closed. */
	wave_open = false;
}

M59EXPORT UINT PlayWaveFile(HWND hwnd, const char *fname, int volume,
							BYTE flags, int src_row, int src_col, int radius,
							int max_vol)
{
	char pathbuf[MAX_PATH];
	bool played = false;
	const char *actual_path = NULL;  // Track which path was actually used

	(void)hwnd; /* callback window unused for OpenAL path */

	if (!fname || fname[0] == '\0')
		return 1;

	/* If name appears to be a bare filename (no path), try resource dir first
	   to avoid noisy "cannot open" logs for the working directory lookup. */
	bool has_path = (strchr(fname, '\\') != NULL) || (strchr(fname, '/') != NULL) || (strchr(fname, ':') != NULL);
	if (!has_path)
	{
		snprintf(pathbuf, MAX_PATH, "%s\\%s", sound_dir, fname);
		debug(("PlayWaveFile: trying %s\n", pathbuf));
		played = SoundPlayWave(pathbuf, volume, flags, src_row, src_col, radius, max_vol);
		if (played)
		{
			actual_path = pathbuf;
		}
		else
		{
			debug(("PlayWaveFile: trying %s\n", fname));
			played = SoundPlayWave(fname, volume, flags, src_row, src_col, radius, max_vol);
			if (played)
				actual_path = fname;
		}
	}
	else
	{
		debug(("PlayWaveFile: trying %s\n", fname));
		played = SoundPlayWave(fname, volume, flags, src_row, src_col, radius, max_vol);
		if (played)
			actual_path = fname;
	}

	/* If this is a looping ambient and it played, register the actual path used
	   so Audio_StopSourcesForFilename can find it in the buffer cache. */
	if (played && (flags & SF_LOOP) && actual_path)
	{
		Sound_RegisterAmbientFilename(actual_path);
	}

	return played ? 0 : 1;
}

/* SoundDone is invoked via MM_WOM_DONE; match window message LPARAM signature */
void SoundDone(HWND hwnd, int iChannel, LPARAM lpWave)
{
	(void)hwnd; (void)iChannel; (void)lpWave;
	/* No cleanup required for OpenAL one-shot playback here. */
}

M59EXPORT void PlayWaveRsc(ID rsc, int volume, BYTE flags, int row, int col, int radius, int max_vol)
{
	char *name;

	if (rsc == 0)
		return;

	/* Lookup resource filename */
	name = LookupRsc(rsc);
	if (name == NULL)
		return;

	debug(("PlayWaveRsc: rsc=%d -> %s\n", rsc, name));

	/* Forward to PlayWaveFile which handles path resolution and ambient tracking */
	PlayWaveFile(hMain, name, volume, flags, row, col, radius, max_vol);
}

/* PlayMidiRsc (legacy shim) and PlayMusicRsc implemented in music.c */

M59EXPORT void SoundAbort(void)
{
	if (wave_open)
	{
		/* Stop all playing sounds via OpenAL backend if implemented */
	}
}

void NewSound(WPARAM type, ID rsc)
{
	(void)type; (void)rsc;
}

void UpdateLoopingSounds(int px, int py)
{
	(void)px; (void)py;
}
