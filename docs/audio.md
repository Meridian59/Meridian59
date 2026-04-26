# Meridian 59 Audio System

## Overview

The client audio system was refactored in PR #1293 to replace the proprietary Miles Sound System (MSS) and WaveMix with [OpenAL Soft](https://github.com/kcat/openal-soft), an open-source, cross-platform audio library.

## Architecture

```mermaid
graph TD
    subgraph "Server Protocol"
        BP_PLAY_MIDI[BP_PLAY_MIDI]
        BP_PLAY_MUSIC[BP_PLAY_MUSIC]
        BP_PLAY_WAVE[BP_PLAY_WAVE]
    end

    subgraph "Protocol Handlers (server.c)"
        HandlePlayMidi[HandlePlayMidi]
        HandlePlayMusic[HandlePlayMusic]
        HandlePlayWave[HandlePlayWave]
    end

    subgraph "Music API (music.c)"
        PlayMusicRsc[PlayMusicRsc]
        PlayMusicFile[PlayMusicFile]
        MusicRestart[MusicRestart]
    end

    subgraph "Sound API (sound.c)"
        GamePlaySound[GamePlaySound]
        PlayWaveRsc[PlayWaveRsc]
    end

    subgraph "OpenAL Engine (audio_openal.c)"
        AudioInit[AudioInit]
        MusicPlay[MusicPlay]
        SoundPlay[SoundPlay]
        AudioShutdown[AudioShutdown]
    end

    subgraph "Audio Hardware"
        OpenAL[OpenAL Soft]
    end

    BP_PLAY_MIDI --> HandlePlayMidi
    BP_PLAY_MUSIC --> HandlePlayMusic
    BP_PLAY_WAVE --> HandlePlayWave

    HandlePlayMidi --> PlayMusicRsc
    HandlePlayMusic --> PlayMusicRsc
    HandlePlayWave --> GamePlaySound

    PlayMusicRsc --> PlayMusicFile
    PlayMusicFile --> MusicPlay
    MusicRestart --> PlayMusicRsc

    GamePlaySound --> PlayWaveRsc
    PlayWaveRsc --> SoundPlay

    MusicPlay --> OpenAL
    SoundPlay --> OpenAL
```

## Before vs After

| Aspect | Before (MSS/WaveMix) | After (OpenAL Soft) |
|--------|---------------------|---------------------|
| **License** | Proprietary (cannot distribute) | LGPL (open source) |
| **Music Format** | MIDI, MP3 | OGG Vorbis |
| **Sound Format** | WAV | WAV, OGG Vorbis |
| **3D Audio** | None | Positional audio with distance falloff |
| **Caching** | None | LRU buffer cache (256 entries) |
| **Platform** | Windows only | Cross-platform |

## Function Reference

### Engine Layer (audio_openal.c)

| Function | Purpose |
|----------|---------|
| `AudioInit(hwnd)` | Initialize OpenAL device, context, and sources |
| `AudioShutdown()` | Clean up all audio resources |
| `MusicPlay(filename, loop)` | Open OGG for streaming playback |
| `MusicStop()` | Stop music playback |
| `MusicIsPlaying()` | Returns true if music is currently streaming |
| `MusicSetVolume(volume)` | Set music volume (0.0 - 1.0) |
| `SoundPlay(filename, volume, flags, ...)` | Play sound effect with optional 3D positioning |
| `SoundStopAll()` | Stop all sound effects |
| `AudioUpdateListener(x, y, z, ...)` | Update listener position for 3D audio |
| `AudioUpdateTrackedSources()` | Refresh positions of sources attached to moving game objects |

### Music API (music.c)

| Function | Purpose |
|----------|---------|
| `PlayMusicFile(hwnd, filename)` | Play music by filename (handles .mid/.mp3 -> .ogg) |
| `PlayMusicRsc(rsc)` | Play music by resource ID |
| `MusicAbort()` | Stop all music |
| `MusicRestart()` | Resume music when user re-enables in settings |
| `MusicClose()` | Shutdown music system |
| `NewMusic(type, rsc)` | Handle server music message |
| `ResetMusicVolume()` | Apply volume from config |

### Sound API (sound.c)

| Function | Purpose |
|----------|---------|
| `PlayWaveRsc(rsc, ...)` | Play sound effect by resource ID |
| `PlayWaveFile(filename, ...)` | Play sound effect by filename |
| `GamePlaySound(rsc, ...)` | Entry point from server protocol |
| `SoundAbort()` | Stop all sounds |

## API Consolidation (PR #1293)

The following functions were removed to simplify the API:

| Removed | Reason |
|---------|--------|
| `MusicInitialize()` | Folded into `AudioInit()` |
| `PlayMidiFile()` | Redundant - `PlayMusicFile()` handles .mid conversion |
| `PlayMidiRsc()` | Redundant - `HandlePlayMidi` now calls `PlayMusicRsc` directly |
| `MusicDone()` | Was an empty stub |
| `MusicStart()` | Renamed to `MusicRestart()` for clarity |

## File Format Conversion

Legacy music files (.mid, .midi, .mp3) are automatically mapped to .ogg:

```
main.mid  -> main.ogg
theme.mp3 -> theme.ogg
```

This mapping happens in `ConvertLegacyMusicExtension()` in music.c.

Sound effects also prefer .ogg over .wav. When a .wav file is requested, the audio system first checks if an .ogg version exists and uses that instead.

### File Naming Convention

**Important:** Music and sound effect files must not share the same base name.

Since both music (.mid) and sounds (.wav) now resolve to .ogg, a naming conflict occurs
if both `forest.mid` (music) and `forest.wav` (sound effect) exist - they would both
map to `forest.ogg`.

Existing conflicts were resolved in PR #1307. When adding new audio:

- Use descriptive, unique names for all audio files
- Prefix or suffix to distinguish purpose if needed (e.g., `forest_ambient.ogg` vs `forest_theme.ogg`)
- Check that no existing music or sound uses the same base name

## Buffer Caching

Sound effects are cached using an LRU (Least Recently Used) strategy:

- **Cache size:** 256 buffers maximum
- **Eviction:** Oldest unused buffer is evicted when cache is full
- **Protection:** Buffers currently playing are never evicted
- **Lookup:** O(1) via case-insensitive hash map

Music is NOT cached because tracks are large and typically don't repeat rapidly. Instead, music uses streaming playback (see below).

## Music Streaming

Music files are played via streaming rather than full-file decoding. This eliminates the loading hitch that occurred when decompressing entire OGG files (30-50 MB of decoded PCM from 2-7 MB OGG files) in a single blocking call on the main thread.

### How It Works

On `MusicPlay()`, only the OGG headers are parsed (~1ms). Audio is decoded in small
chunks and fed to OpenAL through a ring buffer:

| Parameter | Value |
|-----------|-------|
| Buffer count | 4 |
| Samples per buffer | 4096 (~93ms at 44100 Hz) |
| Total audio runway | ~0.37 seconds |
| Memory per buffer | 16,384 bytes (stereo 16-bit) |
| Total streaming memory | ~64 KB (vs 30-50 MB full decode) |

Music streaming runs on a dedicated background thread (`MusicThreadProc`) started in `AudioInit()`. The thread decodes OGG data and rotates OpenAL buffers independently of the main thread, so buffer refills cannot be starved by UI activity.

`MusicPlay()`, `MusicStop()`, and `MusicSetVolume()` post commands to the thread and return immediately. `MusicStreamUpdate()` queries `AL_BUFFERS_PROCESSED` to find consumed buffers, decodes the next chunk, and re-queues. If the source underruns, it restarts automatically. Looping seeks the decoder back to the start at EOF.

```mermaid
graph LR
    subgraph "Buffer Ring (4 x 16KB)"
        B1["Buf 1<br/>Playing"]
        B2["Buf 2<br/>Queued"]
        B3["Buf 3<br/>Queued"]
        B4["Buf 4<br/>Decoding"]
    end

    subgraph "Music Thread"
        T1["Decode + refill buffers"]
    end

    subgraph "Main Thread"
        M1["MusicPlay / Stop / SetVolume"]
    end

    M1 --> T1
    T1 --> B4

    style B1 fill:#2f9e44,color:#fff
    style B2 fill:#1864ab,color:#fff
    style B3 fill:#1864ab,color:#fff
    style B4 fill:#e67700,color:#fff
```

## 3D Positional Audio

### Which Sounds Are Positional?

Not all sounds use 3D positioning. This matches the original MSS behavior where all sounds played at equal volume in both stereo channels (no panning).

| Sound Type | Positional? | Rationale |
|------------|-------------|----------|
| `SF_RANDOM_PLACE` | No | Periodic ambient sounds (server picks random location) |
| `SF_LOOP` at coords (1,1) or (2,2) | No | Background atmosphere using placeholder coordinates |
| `SF_LOOP` at real coords | Yes | Localized objects (fountains, torches, etc.) |
| Other sounds with coords | Yes | Combat, spells, object interactions |
| Sounds at (0,0) | No | No position specified, play centered |

### Distance Model

The client uses two different OpenAL distance models depending on whether a sound is a one-shot effect or a looping ambient emitter, and whether the Blakod explicitly specified a cutoff radius:

| Sound type | Blakod-set radius? | Distance model | Reference distance | Max distance | Behavior |
|------------|--------------------|----------------|--------------------|--------------|----------|
| `SF_LOOP` ambient (fountain, firepit, forge) | yes (always) | `AL_LINEAR_DISTANCE_CLAMPED` | 1 tile | radius | Linear falloff to silence at the Blakod-defined radius |
| One-shot with explicit radius (door, scripted area effect) | yes | `AL_LINEAR_DISTANCE_CLAMPED` | 1 tile | radius | Linear falloff to silence at the Blakod-defined radius |
| One-shot with no radius (combat, spells, generic effects) | no | `AL_INVERSE_DISTANCE_CLAMPED` | 2 tiles | 10000 (effectively unbounded) | Smooth `1/d` falloff, never reaches silence |

The Blakod communicates intent via the `cutoff_radius` parameter on `SomethingWaveRoom` / `WaveSendUser`:

- `cutoff_radius = 0` (default in `user.kod`'s `WaveSendUser`): no opinion. One-shots get the soft inverse falloff so distant combat/spells stay audible across the room, matching pre-OpenAL Miles Sound System behavior where `GamePlaySound()` computed `volume = MAX_VOLUME * 2 / distance` and never let a sound reach zero.
- `cutoff_radius = N` (Blakod set it explicitly): the Blakod author wants a hard cutoff at N tiles. Used by door open/close (radius=4) so a door slam in one corner of a large hall does not bleed across the whole room.

Loops always use the linear-clamped model with their Blakod-defined radius. Without the cutoff, every loaded ambient emitter (fountain in every room, firepit in every cave) would mix together forever with no way to bound CPU or audio mix complexity. `fountain.kod` defaults `piSoundRadius = 40` and firepits typically use 5.

Per-source distance models are enabled via `AL_EXT_source_distance_model` (`alEnable(AL_SOURCE_DISTANCE_MODEL)` at init), which lets each source pick its own model independently of the global default.

```mermaid
flowchart TD
    A["SoundPlay called"] --> B{"Has positional<br/>coordinates?"}
    B -->|"No"| C["Centered on local listener<br/>(SOURCE_RELATIVE, no falloff)<br/>Plays equally L/R for this client only"]
    B -->|"Yes"| D{"SF_LOOP flag set?"}
    D -->|"Yes (fountain, firepit)"| E["AL_LINEAR_DISTANCE_CLAMPED<br/>ref=1, max=radius<br/>Hard cutoff at radius"]
    D -->|"No"| G{"Blakod-set radius?<br/>(radius > 0)"}
    G -->|"Yes (door, area effect)"| H["AL_LINEAR_DISTANCE_CLAMPED<br/>ref=1, max=radius<br/>Hard cutoff at radius"]
    G -->|"No (combat, spell)"| F["AL_INVERSE_DISTANCE_CLAMPED<br/>ref=2, max=10000<br/>Soft 1/d falloff, no cutoff"]

    style C fill:#1864ab,color:#fff
    style E fill:#2f9e44,color:#fff
    style H fill:#2f9e44,color:#fff
    style F fill:#2f9e44,color:#fff
```

### Coordinate System

All audio positioning uses **tile coordinates** (coarse grid), not fine coordinates:

- **Fine coords:** High precision (e.g., 39424, 56832), used for smooth player movement
- **Tile coords:** Coarse grid (e.g., 56, 18), conversion: `tile = fine >> LOG_FINENESS`

`GamePlaySound()` normalizes object positions (fine) to tile coords before calling audio. `UpdateLoopingSounds()` converts player position (fine) to tile coords for the listener. Server-provided ambient sound positions are already in tile coords.

The listener position is updated each frame via `AudioUpdateListener()`.

### Sources Attached to Moving Objects

When a sound is started for a specific game object (a monster, a player, a projectile owner), `GamePlaySound` forwards the object's `source_obj` ID all the way down to `SoundPlay`. If the resulting source is positional, the audio layer registers it in a small `g_trackedSources` table.

Each frame `UpdateLoopingSounds()` calls `AudioUpdateTrackedSources()`, which drops entries whose source has stopped (one-shots clean themselves up as they finish playing), looks up the object via `GetRoomObjectById`, and refreshes `AL_POSITION` from the object's current `motion.x/y` so the sound follows the object as it moves.

#### Untrack on missing object

If the object lookup fails, the entry is dropped from the tracker but the source is left playing at its last position. This is intentional. Object IDs can become temporarily unreachable when the server renumbers room contents (for example during a system save), even though the same prop is still present under a new ID. Stopping the source on every miss would briefly silence fountains, fire pits, and other looping world audio every save.

Edge cases worth noting:
- One-shots play to completion. The source finishes naturally and the next frame removes it via the "source no longer playing" branch.
- After a renumber, the server resends `BK_PLAY_WAVE` for the same prop with its new ID. For looping sounds, `SoundPlay` notices the WAV is already playing on a source and returns early without starting a duplicate, so there is no overlap. The existing loop keeps playing at its last position and is no longer tracked, which is fine for stationary props (fountains, fire pits). A moving object with a loop would stop following until the next room transition clears state.
- Genuine "this sound should stop" still flows through the server sending an explicit stop, which `Audio_StopSourcesForFilename` honors. Looping sources are also bounded by `SoundStopAll` on room transitions.

Without this, a sound emitted by a moving object stays anchored at the spot where the sound first started, regardless of where the object is now. Sounds started with `source_obj == 0` (UI sounds, fixed-position room emitters such as fountains and firepits) are never registered and remain at their initial position.

## Audio File Guidelines

When adding new sound files to the game, follow these guidelines for optimal playback:

### Sound Effects (Positional)

| Requirement | Value | Reason |
|-------------|-------|--------|
| **Channels** | Mono (1 channel) | OpenAL only applies 3D positioning to mono sources. Stereo sounds will ignore positioning and play centered. |
| **Format** | OGG Vorbis or WAV | OGG preferred for smaller file size |
| **Sample Rate** | 44100 Hz | Standard quality (22050 Hz also supported for legacy compatibility) |

### Music (Non-Positional)

| Requirement | Value | Reason |
|-------------|-------|--------|
| **Channels** | Stereo (2 channels) | Music plays centered, stereo preserves original mix |
| **Format** | OGG Vorbis | Required for music playback |
| **Sample Rate** | 44100 Hz | Standard music quality |

### Why Mono for 3D Audio?

OpenAL uses the mono audio data and applies HRTF/panning based on the sound's position relative to the listener. With stereo files, OpenAL cannot determine how to spatialize the left vs right channels, so it plays them as-is (no 3D effect).

If you notice a sound effect is not panning correctly when you move around it in-game, check if the source file is stereo and convert it to mono.

## Configuration

Audio settings in `meridian.ini`:

```ini
[Meridian]
MusicVolume=40      ; 0-100
SoundVolume=99      ; 0-100
AmbientVolume=100   ; 0-100 (looping/3D sounds)
```

## HRTF and Surround Sound

OpenAL Soft supports multiple audio output modes configured via `alsoft.ini` (user's AppData or game directory):

### HRTF (Headphone 3D Audio)

Head-Related Transfer Function simulates 3D audio for headphone users by applying filters that mimic how sound reaches your ears from different directions.

To enable HRTF, create or edit `alsoft.ini`:

```ini
[general]
hrtf = true
```

OpenAL Soft ships with built-in HRTF data. Additional HRTF profiles (.mhr files) can be placed in the OpenAL Soft data directory.

### Surround Sound (5.1 / 7.1)

OpenAL Soft automatically detects and uses your system's speaker configuration. For multi-channel setups (5.1, 7.1), 3D positional sounds will be correctly spatialized across all speakers.

To force a specific output mode in `alsoft.ini`:

```ini
[general]
channels = surround51   ; Options: mono, stereo, quad, surround51, surround61, surround71
```

No game configuration is required—OpenAL Soft handles speaker routing automatically based on your Windows audio device settings.

## Dependencies

- **OpenAL Soft 1.24.3** - Audio engine (`OpenAL32.lib`, `OpenAL32.dll`)
- **stb_vorbis** - OGG Vorbis decoder (single-header library)
