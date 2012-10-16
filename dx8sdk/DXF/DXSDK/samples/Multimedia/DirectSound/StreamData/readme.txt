//-----------------------------------------------------------------------------
// 
// Sample Name: StreamData Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  The StreamData sample shows how to stream a wave file through a DirectSound 
  secondary buffer. It is similar to the PlaySound sample, but adds support 
  for streaming.

Path
====
  Source: DXSDK\Samples\Multimedia\DSound\StreamData

  Executable: DXSDK\Samples\Multimedia\DSound\Bin

User's Guide
============
  Load a wave file by clicking Sound File. Select Loop Sound if you want it 
  to play repeatedly. Click Play.

Programming Notes
=================
  For details on how to setup a non-streaming DirectSound buffer, see the 
  PlaySound sample. 

  The basic tasks to stream data from a wav file to a DirectSound buffer are
  as follows:
  
  * Set up DirectSound: 
     1. Call DirectSoundCreate to create a IDirectSound object
     2. Call IDirectSound::SetCooperativeLevel to set the cooperative level.
     3. Set the primary buffer format.  This sample calls 
        DSUtil_SetPrimaryBufferFormat() to do just this. 
       
  * Create a DirectSound buffer and set up the notifications:
     1. Read the wav file to get the wav file size, and the wav format 
        in the format a WAVEFORMATEX structure.
     2. Choose a DirectSound buffer size.  This is the amount of data that 
        DirectSound stores at once.  You re-fill this buffer as sound plays 
        from this buffer.  This is best for large sounds files that are not 
        possible to load all at once.  For this sample, the buffer size is 
        ~3 seconds of data.
     3. Create a DirectSound buffer using the buffer size, and the wav file's 
        format.  Also pass in DSBCAPS_CTRLPOSITIONNOTIFY flag.  This allows the 
        buffer to send notification events to tell us whenever sound has finished 
        playing.  However, using this flags limits the buffer to software only,
        since hardware can not signal position notifications. 
     4. Set up the notifications on the buffer by calling 
        IDirectSoundBuffer::SetNotificationPositions.  See InitDSoundNotification() 
        for an example of how this is done.  When DirectSound plays past a 
        notification position it signals an Win32 event.  When this event is signaled,
        it is safe to fill that segment of data in the buffer with a new piece of 
        sound.
        
  * Play the DirectSound buffer:
     1. Call IDirectSoundBuffer::Restore on the buffer if the buffer was lost.
     2. Next, fill the DirectSound buffer will the maximum amount of sound data.  
        Since all the sound can not fit into this buffer will be filled with new 
        sound data as this sound plays.  
     3. Call IDirectSoundBuffer::Play with the DSBPLAY_LOOPING flag set to 
        start the buffer playing.  The looping flag needs to be set since the 
        buffer will need to continue playing after the end of the buffer is 
        reached since typically more sound needs to be played.  
     
  * Check to see if a notification is signaled:
      1. Typically in the message pump check to see if the event was signaled.  
         The event tells us that a segment of data has been played so this 
         piece need to be filled. MsgWaitForMultipleObjects() works well as 
         the message pump for this purpose.  
      2. If the event has been signaled, then lock the section of the buffer 
         than has just been played and fill it with the next segment of wav 
         data.  See HandleNotification() for how this works.  
                
  * When the entire sound has played:      
     When handling the event notification, keep track of how much data has 
     been put in the buffer.  When the entire wav file has been put into the 
     buffer, and after DirectSound has played it all it is necessary to manually
     stop the buffer since the buffer will continuously loop otherwise.  
         
  * Free DirectSound:
     Simply call Release() on all the DirectSound objects that were created.
  

