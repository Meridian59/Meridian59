//-----------------------------------------------------------------------------
// 
// Sample Name: VoiceConnect Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  VoiceConnect shows how to network other players using DirectPlay to
  start a DirectPlay Voice chat session.  After joining or creating a 
  session, the players may use computer microphone to talk to one other.  
  Other players may join the game in progress at any time.  

Path
====
  Source: DXSDK\Samples\Multimedia\DirectPlay\VoiceConnect 

  Executable: DXSDK\Samples\Multimedia\DirectPlay\Bin

User's Guide
============
  Refer to User's Guide section of the SimplePeer sample for information
  on how to connect.  After connecting, the host will be presented with a 
  dialog to select the voice codec to use.  Typical voice applications will
  what automatically select one, or present this to the user in some other 
  fashion.  Once the chat session has begin, any client may alter the playback
  or capture settings by clicking on "Setup".

Programming Notes
=================
  The VoiceConnect sample is very similar in form to the SimplePeer sample.  For 
  detailed programming notes on the basics this sample, refer to Programming 
  Notes section of the SimplePeer sample.

  The VoiceConnect differs by letting clients use DirectPlay Voice to talk
  to each other using a computer microphone.  Here's how this is done:
  
  * Initialize DirectPlayVoice.  See NetVoice.cpp 
       Before using voice you need a to establish a DirectPlay Voice session.
       So after the DirectPlay session as been created, we call 
       CNetVoice::Init().  In this simple sample, we this when
       the main dialog is initializes.  This does the following:
       
         - Tests the audio setup with DirectPlay Voice. 
           See VoiceSessionTestAudioSetup() in NetVoice.cpp. 
              1. Creates a IDirectPlayVoiceTest with CoCreateInstance()
              2. Calls IDirectPlayVoiceTest::CheckAudioSetup() with the 
                 DVFLAGS_QUERYONLY flag.  This will return DVERR_RUNSETUP 
                 if the setup wizard needs to be run.  If it happens then 
                 we call IDirectPlayVoiceTest::CheckAudioSetup() without
                 the DVFLAGS_QUERYONLY flag.  This will run the dialog
                 based DirectPlay Voice setup wizard.  If it returns success 
                 then we can continue on, otherwise handle the error in
                 some fashion.
              3. Release the IDirectPlayVoiceTest
              
         - If hosting, then we create the voice session:  
           See VoiceSessionCreate() in NetVoice.cpp
              1. Create a IDirectPlayVoiceServer with CoCreateInstance().
                 Keep this interface around since we'll need it later on.
              2. Call IDirectPlayVoiceServer::Initialize to register the 
                 voice server message handler.  We also pass in the 
                 DirectPlay interface here.
              3. Fill out a DVSESSIONDESC struct with the session type, and
                 the desired voice codec.
              4. Call IDirectPlayVoiceServer::StartSession() passing in
                 the DVSESSIONDESC struct.
          
          - If either hosting or joining, then connect to the session. 
            See VoiceSessionConnect() in NetVoice.cpp
            
              1. Create a IDirectPlayVoiceClient with CoCreateInstance()
                 Keep this interface around since we'll need it later on.
              2. Call IDirectPlayVoiceClient::Initialize to register the 
                 voice client message handler.  We also pass in the 
                 DirectPlay interface here.
              3. Fill out a DVSOUNDDEVICECONFIG struct with the guids
                 for the DirectSound playback and capture devices.
              3. Fill out a DVCLIENTCONFIG struct.  This struct is 
                 filled out when with setup dlg.  More complex apps
                 may want to do this a different way.  See VoiceConfigDlgProc() 
                 in VoiceConnect.cpp.
              4. Call IDirectPlayVoiceClient::Connect() with the 
                 DVSOUNDDEVICECONFIG struct and the DVCLIENTCONFIG struct.
              5. Call IDirectPlayVoiceClient::SetTransmitTargets()
                 to the desired default target.  This sample sets the
                 target to DVID_ALLPLAYERS.
              6. Call IDirectPlayVoiceClient::GetSoundDeviceConfig() and
                 check for DVSOUNDCONFIG_HALFDUPLEX to figure out if this 
                 client is in half-duplex mode.  In half-duplex mode, the 
                 client can not talk, but only listen. 

  * Handle voice client messages.  See DirectPlayVoiceClientMessageHandler                 
      - Upon DVMSGID_CREATEVOICEPLAYER:
            1. Since DPN_MSGID_CREATE_PLAYER set a player context, and that message
               is guaranteed to be processed first.  We call GetPlayerContext()
               to get the player's context value and cast the pointer into a 
               player struct.  We enter a global CS lock here and everytime
               we access a player struct.  This is needed to make the struct 
               thread safe, since the same player struct may be accessed by
               multiple threads at the same time.               
            2. Now that we have the player's struct we addref it.  This 
               extra count will be for voice layer.  Upon DVMSGID_DELETEVOICEPLAYER
               we will release it.  This ensures that any voice messages
               that come in will be working with a valid player struct.  
            3. We check the DVMSG_CREATEVOICEPLAYER for DVPLAYERCAPS_HALFDUPLEX
               to see if this player is in half-duplex mode.  If so, then
               we store that value in the player's struct.  Later
               we will update the UI according.  See DisplayPlayersInChat()
               for how this is done.
            4. Similar to DirectPlay, DirectPlay Voice stores 
               context values for each voice player.  So in this sample we 
               store the same value in both locations by setting 
               pCreateVoicePlayerMsg->pvPlayerContext.
       - Upon DVMSGID_DELETEVOICEPLAYER:
            1. We update the UI accordingly if the local player isn't going away.                              
            2. We release the player's struct since we addref'ed it 
               in DVMSGID_CREATEVOICEPLAYER
       - Upon DVMSGID_HOSTMIGRATED:
            1. If DVMSG_HOSTMIGRATED has a non-NULL pdvServerInterface, then
               we have become the new host.  The sample updates the UI to 
               show this.
       - Upon DVMSGID_GAINFOCUS and DVMSGID_LOSTFOCUS:
            This means that the app has gained or lost focus.  Apps that 
            have lost focus can not talk.  When focus is re-gained they can.
            In this sample, it updates the UI accordingly.
       - Upon DVMSGID_RECORDSTART
              DVMSGID_RECORDSTOP, 
              DVMSGID_PLAYERVOICESTART,
              DVMSGID_PLAYERVOICESTOP:
            These message lets us know when talking has stopped or started for 
            the local client, and for remove clients.  This sample handles
            these messages by updating the UI accordingly.
      
  * Clean up DirectPlay Voice.  See bottom of CNetVoice::Free in NetVoice.cpp
        1. Disconnect from the voice session.  See VoiceSessionDisconnect()
            - Calls IDirectPlayVoiceClient::Disconnect()
            - Releases the IDirectPlayVoiceClient
        2. Destroy the voice session if this is the host.  See VoiceSessionDestroy() 
            - Calls IDirectPlayVoiceServer::StopSession()
            - Releases the IDirectPlayVoiceServer
           


