//-----------------------------------------------------------------------------
// 
// Sample Name: SimplePeer Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  SimplePeer illustrates how to network other players using DirectPlay.  
  After joining or creating a session, a simple game 
  begins immediately.  Other players may join the game in progress at any time.  
  
  The game itself is very simple, passing a single DirectPlay message to all 
  connected players when the "Wave To other players" button is pressed.

Path
====
  Source: DXSDK\Samples\Multimedia\DirectPlay\SimplePeer

  Executable: DXSDK\Samples\Multimedia\DirectPlay\Bin

User's Guide
============
  Enter the player's name, and choose a connection type.  You can either choose 
  "Wait for Lobby Connection" to wait for a lobby connection or choose a 
  service provider.  Use the Multiplayer Games dialog to either search for an 
  active game to join, or to start a new game.  After game has been joined or 
  created, the game begins immediately.  Other players may join the game at 
  any time.  If host migration is on, then the host player may also leave 
  at anytime since DirectPlay will automatically migrate the host player.

Programming Notes
=================
  This sample was intended to be very simple, showing the basics of using 
  the DirectPlay API.  Here's a quick run through the various steps 
  of SimplePeer:
  
  * Initialize DirectPlay. See InitDirectPlay()
        1. Init COM with CoInitialize()
        2. Create a IDirectPlay8Peer with CoCreateInstance()
        3. Create a IDirectPlay8LobbiedApplication with CoCreateInstance()
        4. Call IDirectPlay8Peer::Initialize to tell the interface about our 
                message handler
        5. Call IDirectPlay8LobbiedApplication::Initialize to tell the interface 
                about our message handler
        6. Check if IDirectPlay8LobbiedApplication::Initialize returned a non-NULL 
                lobby client connection handle. If it did, then we know we were
                launched by a lobby client, so it may (but not necessarily) also 
                have sent us connection settings we can use to either host or join 
                a game.
        
  * Either host or connect to a DirectPlay game.  See WinMain().
        If we were launched from a lobby client, then we may have connection 
        settings that we can use either host or join a game.  If not, then we'll 
        need to prompt the user to determine how to connect. 

    - Host/connect using the connection settings obtained from lobby client.  See 
        CNetConnectWizard::ConnectUsingLobbySettings().
          
        1. Call IDirectPlay8LobbiedApplication::GetConnectionSettings() to get the
                connection settings from a lobby client.  Note: the connection 
                settings are also present in the DPL_MSGID_CONNECT message, but if 
                we use those we would need to make a deep copy of the data since 
                we don't own pConnectMsg->pdplConnectionSettings.
        2. Check dwFlags for DPLCONNECTSETTINGS_HOST.  This will tell us if we are 
                hosting or not.
        3. Call IDirectPlay8Peer::SetPeerInfo so other players know our player name.  
                This isn't necessary, but SetPeerInfo is an easy way to post data 
                such as the player name so that all of the other clients can access it.
        4. If hosting call IDirectPlay8Peer::Host(), otherwise call 
                IDirectPlay8Peer::Connect().                    
        5. Cleanup DPL_CONNECTION_SETTINGS.  
        
    - Host/connect by prompting user for connect settings.  See 
        CNetConnectWizard::DoConnectWizard():
        
        This sample calls upon the helper class CNetConnectWizard for this task.
        It uses dialog boxes to query the user what to do, however most games will 
        want to use a fanicer graphics layer such as Direct3D.  Here's what 
        CNetConnectWizard does after calling CNetConnectWizard::DoConnectWizard():
        
        1. CNetConnectWizard enumerates and displays DirectPlay's service providers with
                IDirectPlay8Peer::EnumServiceProviders. 
                See CNetConnectWizard::ConnectionsDlgFillListBox()
        2. When a service provider has been chosen via the UI...
                See CNetConnectWizard::ConnectionsDlgOnOK().         
            - If the user chooses "Wait for Lobby Connection", then it calls 
                IDirectPlay8LobbiedApplication::SetAppAvailable. This tells the 
                lobby client that our app is available for connection, so it 
                doesn't need to launch a new instance of the application.  Then 
                pops a dialog box that has timer.  The timer checks to see if 
                m_hLobbyConnectionEvent is signaled (it becomes signaled when a 
                DPL_MSGID_CONNECT is received).  When it is, then we can call 
                ConnectUsingLobbySettings().                
            - If the user chooses a SP, it creates a DirectPlay host and device 
                addresses calling CoCreateInstance.  Then it calls 
                IDirectPlay8Address::SetSP to pass SP's guid into the two 
                IDirectPlay8Address's.                 
        3. Call IDirectPlay8Peer::EnumHosts to enum all the games in progress on that SP.            
                See CNetConnectWizard::SessionsDlgEnumHosts().                
                We pass in a device address to specify which device to use,
                and a host address to specify the address of the host.  For TCP/IP, 
                the host address may contain just the SP to search the local subnet, 
                or it may have the SP and a have an IP address to search.  If too 
                little information is supplied and the flag DPNENUMHOSTS_OKTOQUERYFORADDRESSING 
                is passed then DirectPlay will popup a dialog box to prompt the 
                user for any extra  needed information.  More complex games would 
                probably want to not pass this flag, and supply the needed 
                information so as to avoid unnecessary dialog boxes.                
        4. Process DPN_MSGID_ENUM_HOSTS_RESPONSE messages that come in on the callback.
                Upon receiving these messages, put them in a data structure.  You 
                will need to deep copy the DPN_APPLICATION_DESC, and the pAddressSender.  
                Also be careful to not add duplicates to the list.  You will need 
                to manage this structure yourself including expiring old enumerations.  
                See CNetConnectWizard::SessionsDlgNoteEnumResponse() and 
                CNetConnectWizard::SessionsDlgExpireOldHostEnums().
        5. The wizard displays the list of current sessions (built with step #4) 
                and allows the user to either choose a game from the list or 
                create a new one.          
           - If joining a game from the list, it calls IDirectPlay8Peer::SetPeerInfo()
                to set the player's name, and then calls IDirectPlay8Peer::Connect()
                passing in the DPN_APPLICATION_DESC, as well as pAddressSender 
                from the selected game. This will async complete, so wait for 
                DPN_MSGID_CONNECT_COMPLETE, and read the connect result from msg.
                See CNetConnectWizard::SessionsDlgJoinGame(), and 
                CNetConnectWizard::SessionsDlgProc()'s WM_TIMER.
           - If creating a new game, it calls IDirectPlay8Peer::SetPeerInfo()
                to set the player's name, and then calls IDirectPlay8Peer::Host()
                passing in a DPN_APPLICATION_DESC filled with various info such
                as the game name, max players, and the app guid. It also passes in
                the IDirectPlay8Address that describes which SP to use.
                See CNetConnectWizard::SessionsDlgCreateGame().
                
        After either prompting the user with dialog boxes or connecting using 
        settings from a lobby connection, the IDirectPlay8Peer interface will 
        be connected to a DirectPlay session by either hosting a new one or join 
        an existing one. 
                
  * Handle DirectPlay system messages.  See DirectPlayMessageHandler()
        - Upon DPN_MSGID_CREATE_PLAYER it calls IDirectPlay8Peer::GetPeerInfo
                to retrieve the player's name.  It then creates a app specific
                structure for the player, APP_PLAYER_INFO.  It passes the 
                pointer to this structure to DirectPlay in the pvPlayerContext 
                field.  This prompts DirectPlay to return that pointer whenever 
                a message from or about that player is received, so instead of 
                traversing a data structure to find the player's structure the 
                pvPlayerContext will be pointing directly to the correct 
                structure.  Note that since player can be deleted at any time, 
                we need to use ref counting to make sure this player context struct 
                isn't deleted while we are still using it.  
        - Upon DPN_MSGID_DELETE_PLAYER it gets the player's structure from 
                pDeletePlayerMsg->pvPlayerContext and then decrements the
                ref count for the structure.   If the struct's ref count is zero,
                the struct is deleted.  
        - Upon DPN_MSGID_CONNECTION_TERMINATED it shuts down the dialog.
        - Upon DPN_MSGID_RECEIVE it casts pReceiveMsg->pReceiveData into 
                a generic app defined structure that helps it figure out 
                what structure is really contained in pReceiveMsg->pReceiveData.
                For this simple example, if the type is GAME_MSGID_WAVE it 
                just executes an simple action.
                
  * Send a DirectPlay packet. See WaveToAllPlayers().
        - If the user presses, "Wave to other players!" button then it 
                calls IDirectPlay8Peer::SendTo() with DPNID_ALL_PLAYERS_GROUP
                and a data message that is simply a DWORD containing GAME_MSGID_WAVE.                
                
  * Clean up.  See bottom of WinMain()
        1. Delete the connection wizard class.
        2. Call IDirectPlay8LobbiedApplication::Close()
        3. Release the IDirectPlay8LobbiedApplication
        4. Call IDirectPlay8Peer::Close()
        5. Release the IDirectPlay8Peer
        1. Free any app-specific resources
        6. Call CoUninitialize().
        
