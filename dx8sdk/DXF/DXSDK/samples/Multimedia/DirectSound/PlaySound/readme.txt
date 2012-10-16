//-----------------------------------------------------------------------------
// 
// Sample Name: PlaySound Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  The PlaySound sample shows how to play a wave file in a DirectSound 
  secondary buffer.
    
Path
====
  Source: DXSDK\Samples\Multimedia\DSound\PlaySound

  Executable: DXSDK\Samples\Multimedia\DSound\Bin

User's Guide
============
  Load a wave file by clicking Sound File. Select Loop Sound if you want 
  it to play repeatedly. Click Play.

Programming Notes
=================
  The basic tasks write an application that supports DirectSound are as follows:
 
  * Set up DirectSound: 
     1. Call DirectSoundCreate to create a IDirectSound object
     2. Call IDirectSound::SetCooperativeLevel to set the cooperative level.
     3. Set the primary buffer format.  This sample calls 
        DSUtil_SetPrimaryBufferFormat() to do just this. 
       
  * Load a wav file into a DirectSound buffer:
     1. Read the wav file to get the wav file size, and the wav format 
        in the format a WAVEFORMATEX structure.
     2. If the wav file of reasonable size, then create a DirectSound buffer 
        big enough to hold the entire wav file, and set it's format to
        that of the wav file.  If the wav file large, then see the 
        StreamData sample for information on how to stream data into a 
        DirectSound buffer. 
     3. Next, fill the DirectSound buffer with wav data.  A pointer into the 
        buffer can be obtained by calling IDirectSoundBuffer::Lock.  After the 
        memory has been copied, call IDirectSoundBuffer::Unlock.

  * Play or stop the DirectSound buffer:
     1. First, check to see if the buffer was not lost.  If it was, then it will 
        need to be restored.
     2. To play the buffer call IDirectSoundBuffer::Play.
     3. To stop the buffer call IDirectSoundBuffer::Stop.
     
  * To check to see if the sound stopped:
     It may be useful to tell if a DirectSound buffer stopped playing.  An easy 
     way to do this would be to set a timer to trigger every so often.  When the 
     timer message is sent, call IDirectSoundBuffer::GetStatus to see if the 
     DSBSTATUS_PLAYING is set.  If it is not, then the sound has stopped. 
     
  * Handle restoring a DirectSound buffer:
     First call IDirectSoundBuffer::Restore. Next, fill the buffer with sound again 
     since the sound data was lost from when the buffer was lost.  
       
  * Free DirectSound:
     Simply call Release() on all the DirectSound objects that were created.
  
