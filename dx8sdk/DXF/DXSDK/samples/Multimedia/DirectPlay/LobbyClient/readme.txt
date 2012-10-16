//-----------------------------------------------------------------------------
// 
// Sample Name: LobbyClient Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  LobbyClient is a simple lobby client.  It displays all registered DirectPlay 
  applications on the local system.  It allows the 
  user to launch one or more of these applications using a chosen 
  service provider.  A launched lobbied application may be told to either 
  join or host a game.

Path
====
  Source: DXSDK\Samples\Multimedia\DirectPlay\LobbyClient 

  Executable: DXSDK\Samples\Multimedia\DirectPlay\Bin

User's Guide
============
  Choose launch settings from dialog, and click "Launch App".  The
  lobby client will launch the app selected based on the settings in the UI.
  The active connections list will display the handle to all the current 
  lobbied applications.  Clicking on "Send Message" will send a lobby message
  to the lobbied application.  This done mainly for demonstration purposes.  A more
  complex lobby client may want to use this functionality in a more meaningful way
  by passing a message that the lobbied application responds to.

Programming Notes
=================
    * Initialize DirectPlay. See InitDirectPlay()
        1. Init COM with CoInitialize()
        2. Create a IIDirectPlay8Peer with CoCreateInstance()
        3. Call IDirectPlay8Peer::Initialize to tell the interface about our 
           message handler
        4. Create a IDirectPlay8LobbyClient with CoCreateInstance()
        5. Call IDirectPlay8LobbyClient::Initialize to tell the interface 
           about our lobby message handler
        

    * Initialize dialog box. See OnInitDialog()
        1. Enumerate the registered lobbied applications and display them
           in the dialog listbox.  See EnumRegisteredApplications()
        2. Enumerate the DirectPlay service providers and display them
           in the dialog listbox.  See EnumServiceProviders()
        3. Whenever a new service provider is selected (and upon init) then
           enumerate the service provider's adapters.  See EnumAdapters().

    * When "Launch App" button is clicked.  See LaunchApp()
        1. Fill out a DPL_CONNECT_INFO struct.  This is complex
           since it contains the host and device addresses as well as
           the DPN_APPLICATION_DESC.  See LaunchApp() and 
           AllocAndInitConnectSettings().
        2. Call IDirectPlay8LobbyClient::ConnectApplication() passing in
           the DPL_CONNECT_INFO struct.
        3. Free the DPL_CONNECT_INFO struct. This is also complex since this 
           struct has a number of DPlay addresses.  See FreeConnectSettings().
        
   * Upon DirectPlay Lobby messages. See DirectPlayLobbyMessageHandler()
        - Upon DPL_MSGID_DISCONNECT:
            pDisconnectMsg->hDisconnectId will contain the handle of the 
            lobbied application that was disconnected, and 
            pDisconnectMsg->hrReason will be the reason.  This simple 
            sample just pops up a message box.
        - Upon DPL_MSGID_RECEIVE:
            The lobbied application sent the client data.  This simple sample 
            doesn't respond to any message.
        - Upon DPL_MSGID_SESSION_STATUS:
            A lobbied application has changed its status.  pStatusMsg->hSender 
            will one of several predefined status codes.  This simple
            sample just updated the UI showing that the lobby status has
            updated, however more complex lobby clients many want to take action.
        - Upon DPL_MSGID_CONNECTION_SETTINGS:
            A lobbied application has changed its connection settings.  This
            simple lobby client doesn't take any action however, more complex
            clients may want to take action.
            
   * When "Send Message" is clicked. See SendMsgToApp()
        Call IDirectPlay8LobbyClient::Send() with the handle of the 
        lobbied application to send the message to, and the buffer to send.
        
   * When "Disconnect" is clicked. See DisconnectFromApp()
        Call IDirectPlay8LobbyClient::ReleaseApplication() with the handle of 
        lobbied application to disconnect from.
            
            