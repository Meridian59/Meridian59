//-----------------------------------------------------------------------------
// 
// Sample Name: VoiceManagement Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  The VoiceManagement sample shows how to implement dynamic voice management 
  when creating DirectSound secondary buffers.

Path
====
  Source: DXSDK\Samples\Multimedia\DSound\VoiceManagement

  Executable: DXSDK\Samples\Multimedia\DSound\Bin

User's Guide
============
  Load a wave file by clicking Sound File. Select Voice Allocation Flags, 
  Buffer Priority, and Voice Management Flags options. Note that the effect 
  of the selected options is described under Expected Behavior. Create the 
  buffer and play the sound by clicking Play. 

Programming Notes
=================
  For a simpler example of how to setup a DirectSound buffer without 
  voice management flags, see the PlaySound sample. 
  
  To use voice management flags, the buffer must be created using 
  DSBCAPS_LOCDEFER otherwise DirectSound will not be able to dynamically 
  place the buffer in either hardware or software at runtime. 
  
  When playing the buffer just call IDirectSoundBuffer::Play with a 
  valid combination of voice management flags.  Run the sample to observe
  the expected result of any combination of flags.

