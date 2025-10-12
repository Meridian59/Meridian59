# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Meridian 59 is an open-source MMORPG consisting of:

- **blakserv**: C-based game server that manages game logic, player sessions, and world state
- **clientd3d**: Windows client application with Direct3D rendering for 3D graphics
- **blakcomp**: Blakod compiler that compiles game logic written in the custom Blakod language
- **kod**: Game logic written in Blakod (custom scripting language for game rules, NPCs, spells, items)
- **resource**: Game assets (graphics, audio, rooms, textures)
- **module**: Client-side game modules and interfaces

## Build Commands

### Windows (Primary Platform)
```bash
# Prerequisites: Visual Studio 2022 Community Edition
# Ensure vcvars32.bat is in PATH

# Debug build (default)
nmake

# Release build  
nmake RELEASE=1

# Final build (release + no debug strings)
nmake FINAL=1

# Clean all builds
nmake clean
```

### Linux (Server Only)
```bash
cd blakserv
make -f makefile.linux
```

## Architecture Overview

### Server Architecture (blakserv/)
- **Core Systems**: `main.c`, `game.c`, `session.c` - main game loop and session management
- **Object System**: `object.c`, `class.c` - handles game objects and Blakod class instances  
- **Message System**: `message.c`, `sendmsg.c` - inter-object communication via Blakod messages
- **Resource Management**: `loadrsc.c`, `saversc.c` - manages game resources and assets
- **Account/User**: `account.c`, `user.c` - player account and character management
- **Network**: `async.c`, `channel.c` - handles client connections and network I/O
- **Game State**: `loadgame.c`, `savegame.c` - persistent world state management
- **Admin Interface**: `admin.c`, `admincons.c` - server administration console

### Client Architecture (clientd3d/)
- **Main Client**: `client.c`, `game.c` - main client logic and game state
- **3D Rendering**: `d3drender*.c` - Direct3D-based 3D world rendering
- **2D Interface**: `draw.c`, `intrface.c` - 2D UI and interface rendering
- **Network**: `protocol.c`, `server.c` - server communication protocols
- **Game Systems**: `move.c`, `object.c` - movement and object management
- **Audio**: `sound.c`, `music.c` - sound effects and music playback
- **Graphics**: `bitmap.c`, `palette.c` - bitmap handling and color management

### Game Logic (kod/)
- **Base Classes**: `object.kod`, `active.kod`, `passive.kod` - fundamental object hierarchy
- **Game Objects**: `item/`, `passive/` - items, spells, NPCs, rooms defined in Blakod
- **Room System**: `object/active/holder/nomoveon/room/` - game world rooms and areas
- **Spells & Skills**: `object/passive/spell/`, `object/passive/skill/` - magic and abilities
- **Utilities**: `util/` - game systems like guilds, quests, statistics

### Blakod Language
- Custom scripting language for game logic
- Object-oriented with message passing
- Compiled by `blakcomp/` into bytecode executed by server
- File extension: `.kod`
- Include files: `.khd` (headers)

## Development Workflow

1. **Server Changes**: Edit C files in `blakserv/`, rebuild with `nmake`, restart server
2. **Client Changes**: Edit C files in `clientd3d/`, rebuild with `nmake`, restart client  
3. **Game Logic Changes**: Edit `.kod` files, rebuild with `nmake` (auto-compiles Blakod)
4. **Resource Changes**: Update files in `resource/`, rebuild with `nmake`

## Running the Game

### Server Setup
```bash
cd run/server
./blakserv.exe
# In admin console: create account admin username password
# Then: create admin <account_id>
# Use "save game" to persist changes
```

### Client Setup
```bash
cd run/localclient  
./meridian.exe /U:username /W:password /H:localhost /P:5959
```

## Key File Locations

- **Server Executable**: `run/server/blakserv.exe`
- **Client Executable**: `run/localclient/meridian.exe`  
- **Server Config**: `run/server/blakserv.cfg`
- **Client Config**: `run/localclient/meridian.ini`
- **Build Configuration**: `common.mak`, `rules.mak`
- **Game Assets**: `resource/graphics/`, `resource/audio/`
- **Compiled Blakod**: `run/server/loadkod/`

## Platform Support

- **Windows**: Full client and server support (primary platform)
- **Linux**: Server only (requires manual blakserv.cfg adjustments)
- **Dependencies**: zlib, libpng, libarchive (included in `external/`)

## Important Notes

- Game assets (graphics, audio, rooms) are NOT included in this repository
- "Meridian" is a registered trademark - permission required for use
- Uses custom Blakod language extensively for game logic
- Server state is persistent - use admin "save game" command after changes
- Build system uses nmake (Visual Studio) on Windows