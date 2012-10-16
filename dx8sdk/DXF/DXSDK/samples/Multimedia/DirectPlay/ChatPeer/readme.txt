//-----------------------------------------------------------------------------
// 
// Sample Name: ChatPeer Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  ChatPeer is similar in form to SimplePeer.  Once a player hosts or connects
  to a session, the players can chat with either other by passing text 
  strings.
  
Path
====
  Source: DXSDK\Samples\Multimedia\DirectPlay\ChatPeer 

  Executable: DXSDK\Samples\Multimedia\DirectPlay\Bin

User's Guide
============
  Refer to User's Guide section of the SimplePeer sample.

Programming Notes
=================
  The ChatPeer sample is very similar in form to the SimplePeer sample.  For 
  detailed programming notes on the basics this sample, refer to Programming 
  Notes section of the SimplePeer sample.

  The ChatPeer differs by letting clients send text strings to all players
  connected to the session.
  
  * The "Send" button is pressed.  See SendChatMessage().
        1. Retrieves the text string from the dialog.
        2. Fills a app defined struct, GAMEMSG_CHAT.  This struct has
           a message type ID as the first BYTE.  This lets our app
           figure out what type of app message they received, however 
           ChatPeer only uses one app defined message. See StagedPeer
           for a more complex example of this process.
        3. Fills out a DPN_BUFFER_DESC struct using the GAMEMSG_CHAT buffer.
        4. Calls IDirectPlay8Peer::SendTo() with the DPN_BUFFER_DESC.  It passes 
           DPNID_ALL_PLAYERS_GROUP so this message goes to everyone. 
   
  * Handle DirectPlay system messages.  See DirectPlayMessageHandler()

        The ChatPeer handles the typical messages as described in the 
        SimplePeer programming notes, and in addition:
        
        - Upon DPN_MSGID_RECEIVE message:         
           1. It casts pReceiveMsg->pReceiveData into a generic app defined 
              structure, GAMEMSG_GENERIC.  This helps to figure out what structure 
              is really contained in pReceiveMsg->pReceiveData. For this simple 
              example, if the ID is GAME_MSGID_CHAT it casts the buffer to a 
              GAMEMSG_CHAT*.              
           2. It then creates a new APP_QUEUED_DATA struct which contains a pointer 
              to the GAMEMSG_CHAT buffer, and the a DirectPlay handle linked to the 
              GAMEMSG_CHAT buffer.  
           3. We then post a user defined message, WM_APP_CHAT, to the dialog thread 
              with the lParam equal to a pointer to the APP_QUEUED_DATA struct and
              the wParam equal to the DPNID of the player who sent us the buffer.
              We post a message since this lets us return from the DirectPlay message
              handler quickly.  In a complex game if the handler threads take too 
              long its possible that a backlog of network data may arise.
           4. We return from DirectPlayMessageHandler() with the result code 
              of DPNSUCCESS_PENDING.  This error code tells DirectPlay that the buffer 
              we got in the DPN_MSGID_RECEIVE message is still in use by our app.  
              This allows us to not to have to copy the buffer, but instead pass a 
              pointer of it off to a worker thread.  This simple sample just uses 
              the main dialog thread to process queued data.  For a more complex 
              example of this process see the DataRelay sample.
            
  * Upon receiving WM_APP_CHAT in the message loop
        This is posted to the message loop by one DirectPlay message handler threads
        whenever we receive a chat message.  See above for more detail.  Here's what 
        happens:
        
        1. Cast the wParam to a DPNID.  This is the player that sent the message. 
        2. Calls IDirectPlay8Peer::GetPlayerContext().  This retrieves a user 
           specified pointer (which we cast to a APP_PLAYER_INFO*) associated with 
           this player.  See the SimplePeer programming notes for more info on how 
           this is setup.  Also, note that since player can be deleted at any time, 
           we need to use ref counting to make sure this player context struct isn't 
           deleted while we are still using it.  
        3. Cast the lParam into a APP_QUEUED_DATA*.  This tells us the buffer, and the DirectPlay
           handle associated with that buffer.
        4. Process the buffer by adding it's text string to the dialog.
        5. Release the APP_PLAYER_INFO* since we are done using it.
        6. Now DirectPlay can free the receive buffer since we are done with it.  
           So return the buffer to DirectPlay by calling IDirectPlay8Peer::ReturnBuffer, 
           passing in the DirectPlay buffer handle.  
        7. Deletes the APP_QUEUED_DATA from the heap
        
                
   