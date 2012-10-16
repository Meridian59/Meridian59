//----------------------------------------------------------------------------
// File: VoiceClientServer.h
//
// Desc: see voiceclient.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
// Defines, and constants
//-----------------------------------------------------------------------------
#define DPLAY_SAMPLE_KEY        TEXT("Software\\Microsoft\\DirectX DirectPlay Samples")
#define MAX_PLAYER_NAME         14
#define WM_APP_DISPLAY_PLAYERS  (WM_APP + 0)
#define WM_APP_UPDATE_STATS     (WM_APP + 1)

// This GUID allows DirectPlay to find other instances of the same game on
// the network.  So it must be unique for every game, and the same for 
// every instance of that game.  // {AD8EF550-EB5D-49b8-9C13-DC6CCDA33FC5}
GUID g_guidApp = { 0xad8ef550, 0xeb5d, 0x49b8, { 0x9c, 0x13, 0xdc, 0x6c, 0xcd, 0xa3, 0x3f, 0xc5 } };




//-----------------------------------------------------------------------------
// App specific DirectPlay messages and structures 
//-----------------------------------------------------------------------------
#define GAME_MSGID_CREATE_PLAYER    1
#define GAME_MSGID_DESTROY_PLAYER   2
#define GAME_MSGID_SET_ID           3

// Change compiler pack alignment to be BYTE aligned, and pop the current value
#pragma pack( push, 1 )

struct GAMEMSG_GENERIC
{
    DWORD dwType;
};

struct GAMEMSG_SET_ID : public GAMEMSG_GENERIC
{
    DWORD dpnidPlayer;                 // dpnid of the player 
};

struct GAMEMSG_CREATE_PLAYER : public GAMEMSG_GENERIC
{
    DWORD dpnidPlayer;                          // dpnid of the player created
    TCHAR strPlayerName[MAX_PLAYER_NAME];   // name of the player created
};

struct GAMEMSG_DESTROY_PLAYER : public GAMEMSG_GENERIC
{
    DWORD dpnidPlayer;                 // dpnid of the player destroyed
};

// Pop the old pack alignment
#pragma pack( pop )



