// Meridian 59 sound.c - sound effects via OpenAL Soft

#include "client.h"

static const char sound_dir[] = "resource";
static bool wave_open = false;

/* Looping sound tracking for room transitions */
static std::vector<std::string> prev_looping;
static std::vector<std::string> curr_looping;
static bool looping_cleanup_pending = false;

// Forward to audio layer helper
void Audio_StopSourcesForFilename(const char* filename);

// Case-insensitive string comparison helper
static bool iequals(const std::string &a, const std::string &b)
{
	return _stricmp(a.c_str(), b.c_str()) == 0;
}

/* Mark current looping sounds for cleanup; newly registered sounds
   will be recorded in curr_looping and protected from cleanup. */
void Sound_BeginLoopingSoundTransition(void)
{
	// move current into prev
	prev_looping = std::move(curr_looping);
	curr_looping.clear();
	looping_cleanup_pending = true;
}

/* Register a looping sound filename as active for the new room. */
void Sound_RegisterLoopingSound(const std::string &filename)
{
	if (filename.empty())
		return;

	// add to curr if not present
	for (const auto &s : curr_looping)
	{
		if (iequals(s, filename))
			return;
	}
	curr_looping.push_back(filename);

	// If cleanup pending, remove this filename from prev list so it won't be stopped
	if (looping_cleanup_pending)
	{
		auto it = std::remove_if(prev_looping.begin(), prev_looping.end(),
			[&filename](const std::string &s) { return iequals(s, filename); });
		prev_looping.erase(it, prev_looping.end());
	}
}

/* Stop any looping sounds that remain in prev (not re-registered) */
void Sound_EndLoopingSoundTransition(void)
{
	if (!looping_cleanup_pending)
		return;

	for (const auto &s : prev_looping)
	{
		Audio_StopSourcesForFilename(s.c_str());
	}

	// Only clear prev - keep curr so they become prev on next transition
	prev_looping.clear();
	looping_cleanup_pending = false;
}

void SoundInitialize(void)
{
	if (AudioInit(hMain))
		wave_open = true;
	else
		wave_open = false;
}

/*
 * PlayWaveFile: Returns 0 on success, 1 on failure.
 * Plays a sound effect via OpenAL with optional 3D positioning and looping.
 * Looping sounds are automatically registered for room transition cleanup.
 *
 * Note: The function name is retained for legacy compatibility with external
 * modules that import it. Supports both WAV and OGG formats.
 */
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
		played = SoundPlay(pathbuf, volume, flags, src_row, src_col, radius, max_vol);
		if (played)
		{
			actual_path = pathbuf;
		}
		else
		{
			debug(("PlayWaveFile: trying %s\n", fname));
			played = SoundPlay(fname, volume, flags, src_row, src_col, radius, max_vol);
			if (played)
				actual_path = fname;
		}
	}
	else
	{
		debug(("PlayWaveFile: trying %s\n", fname));
		played = SoundPlay(fname, volume, flags, src_row, src_col, radius, max_vol);
		if (played)
			actual_path = fname;
	}

	/* If this is a looping sound and it played, register the actual path used
	   so Audio_StopSourcesForFilename can find it in the buffer cache. */
	if (played && (flags & SF_LOOP) && actual_path)
	{
		Sound_RegisterLoopingSound(actual_path);
	}

	return played ? 0 : 1;
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
		SoundStopAll();
		AudioShutdown();
		wave_open = false;
	}
}

void NewSound(WPARAM type, ID rsc)
{
	(void)type; (void)rsc;
}

/*
 * UpdateLoopingSounds: Updates 3D audio listener from player position and angle.
 * Angle uses game units (4096 = 360 degrees, 0 = east).
 */
void UpdateLoopingSounds(int px, int py, int angle)
{
	double radians = (double)angle * (2.0 * 3.14159265358979) / 4096.0;
	float forwardX = (float)cos(radians);
	float forwardZ = (float)-sin(radians);  // Negate so north faces -Z
	
	debug(("UpdateLoopingSounds: pos=(%d,%d), angle=%d, forward=(%.2f,%.2f)\n",
	       px, py, angle, forwardX, forwardZ));
	
	// Update OpenAL listener position and orientation
	AudioUpdateListener((float)px, 0.0f, (float)py, forwardX, 0.0f, forwardZ);
}
