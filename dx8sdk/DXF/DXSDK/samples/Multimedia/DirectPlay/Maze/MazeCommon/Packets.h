//----------------------------------------------------------------------------
// File: packets.h
//
// Desc: see main.cpp
//
// Copyright (c) 1999-2001 Microsoft Corp. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef _PACKETS_H
#define _PACKETS_H

#include "Trig.h"

#pragma pack(push)
#pragma pack(1)

#define PACK_ARRAY_SIZE     10     //Set the number of elements in our pack array to 10.


//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
enum
{
    PACKETTYPE_SERVER_CONFIG,       // first packet sent 
    PACKETTYPE_CLIENT_VERSION,      // client responds to PACKETTYPE_SERVER_CONFIG w/ this 
    PACKETTYPE_SERVER_ACKVERSION,   // server then responds to PACKETTYPE_CLIENT_VERSION w/ this and game begins
    PACKETTYPE_CLIENT_POS,          // sent to server as client moves
    PACKETTYPE_SERVER_ACKPOS        // sent to client as server acks the PACKETTYPE_CLIENT_POS packets
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: Base class for packets sent from client to server
//-----------------------------------------------------------------------------
struct  ClientPacket
{
    ClientPacket();
    ClientPacket( WORD type ) :
        wType(type) {};

    WORD    wType;
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
struct  ClientPosPacket : public ClientPacket
{
    ClientPosPacket();
    ClientPosPacket( float x , float y, ANGLE cameraYaw )
        : ClientPacket( PACKETTYPE_CLIENT_POS ) , fX(x) , fY(y), aCameraYaw(cameraYaw) {};

    float   fX,fY;  
    ANGLE   aCameraYaw;
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
struct  ClientVersionPacket : public ClientPacket
{
    ClientVersionPacket();
    ClientVersionPacket( DWORD version )
        : ClientPacket( PACKETTYPE_CLIENT_VERSION ) , dwVersion(version) {};

    DWORD dwVersion;  
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: Structure containing client net configuration data. 
//       The server sends this to clients
//-----------------------------------------------------------------------------
struct  ClientNetConfig
{
    DWORD dwMazeWidth;
    DWORD dwMazeHeight;
    DWORD dwThreadWait;
    WORD  wUpdateRate;
    WORD  wTimeout;
    WORD  wClientPackSizeArray[PACK_ARRAY_SIZE]; //Array of 10 custom sizes passed in by user.
    WORD  wServerPackSizeArray[PACK_ARRAY_SIZE]; //Array of 10 custom server sizes.
    BYTE  ubClientPackIndex;    
    BYTE  ubServerPackIndex;    

    BYTE  ubReliableRate;  // Percentage of packets to be transmitted reliably
};


 

//-----------------------------------------------------------------------------
// Name: 
// Desc: Base class for packets sent from server to client
//-----------------------------------------------------------------------------
struct  ServerPacket
{
    ServerPacket();
    ServerPacket( WORD type ) : wType(type) {};

    WORD    wType;
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: Configuration data send from server to client
//-----------------------------------------------------------------------------
struct  ServerAckVersionPacket : public ServerPacket
{
    ServerAckVersionPacket();
    ServerAckVersionPacket( BOOL accepted, DWORD clientID ) :
        ServerPacket(PACKETTYPE_SERVER_ACKVERSION), bAccepted(accepted), dwClientID(clientID) {};

    DWORD dwClientID;
    BOOL bAccepted;
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: Configuration data send from server to client
//-----------------------------------------------------------------------------
struct  ServerConfigPacket : public ServerPacket
{
    ServerConfigPacket();
    ServerConfigPacket( const ClientNetConfig& config ) :
        ServerPacket(PACKETTYPE_SERVER_CONFIG) , Config(config) {};

    ClientNetConfig Config;
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: Chunk of client data for server send to a client
//-----------------------------------------------------------------------------
struct PlayerStatePacket
{
    DWORD   dwID;
    float   fX;
    float   fY;
    ANGLE   aCameraYaw;
};




//-----------------------------------------------------------------------------
// Name: 
// Desc: 
//-----------------------------------------------------------------------------
struct  ServerAckPacket : public ServerPacket
{
    ServerAckPacket();
    ServerAckPacket( DWORD playercount )
        : ServerPacket(PACKETTYPE_SERVER_ACKPOS), 
          wPlayerCount(WORD(playercount)) {};

    WORD    wPlayerCount;         // Count of total players on server
    WORD    wPlayerStatePacketCount;   // Count of following PlayerStatePacket structures
};


#pragma pack(pop)

#endif
