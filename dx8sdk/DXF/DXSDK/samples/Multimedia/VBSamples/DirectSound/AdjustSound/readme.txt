//-----------------------------------------------------------------------------
// 
// Sample Name: VB AdjustSound Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  AdjustSound sample shows how to load and play a wave file using
  a DirectSound buffer and adjust its focus, frequency, pan, and volume.
  
Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectSound\AdjustSound

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectSound\Bin

User's Guide
============
  Load a wave file by clicking Sound File. Select Focus and Buffer Mixing 
  options; note that the various settings are explained under Expected Behavior 
  as you select them. Click Play. If you don't hear any sound, check the Status 
  pane. The application might fail to create the buffer in hardware if this 
  option has been selected. 

  By using the sliders you can adjust the frequency, pan, and volume dynamically 
  as the buffer is playing.

Programming Notes
=================
  To set the focus of a buffer call DirectSound.CreateSoundBufferFromFile with 
  DSBCAPS_GLOBALFOCUS or DSBCAPS_STICKYFOCUS or neither of these flags. 
    
  To set the memory location of a buffer call DirectSound.CreateSoundBufferFromFile 
  with DSBCAPS_LOCHARDWARE or DSBCAPS_LOCSOFTWARE or neither of these flags. 
    
  To control various aspects of DirectSound buffer:
    To adjust the frequency call DirectSoundSecondaryBuffer.SetFrequency
    To adjust the pan call DirectSoundSecondaryBuffer.SetPan
    To adjust the volume call DirectSoundSecondaryBuffer.SetVolume
    
    
  
  
