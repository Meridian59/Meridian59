//-----------------------------------------------------------------------------
// 
// Sample Name: SimpleAudioPath Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
//
// GM/GS® Sound Set Copyright ©1996, Roland Corporation U.S.
// 
//-----------------------------------------------------------------------------



Description
===========
  The AudioPath sample demonstrates how different sounds can be played 
  on an audiopath, and how the parameters of all sounds are affected 
  by changes made on the audiopath.

Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectMusic\SimpleAudioPath

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectMusic\Bin

User's Guide
============
  Click Lullaby, Snore, and Mumble to play different sounds. Adjust the 
  3-D position of the sounds by using the sliders. Click Rude Awakening 
  to play a different sound and stop all other sounds.

Programming Notes
=================
  The AudioPath differs by showing some of the various uses of an
  audiopath. Here's how:
  
  * Upon init. 
        1. Calls DirectMusicPerformance8::CreateStandardAudioPath passing
           in DMUS_APATH_DYNAMIC_3D to create a 3D audiopath.
        2. Loads all of the files needed for this sample.  
  
  * Upon 3D positoin slider change.  See SetPosition
        1. Calls DirectMusicAudioPath.GetObjectInPath on the 3D audiopath to
           get the DirectSound3DBuffer from it.
        2. Calls DirectSound3DBuffer.SetPosition to set a new 3D position on
           the buffer of the audiopath.
        
  * Upon button click.  See cmdSeg_Click.
        - If its the first button, "Lullaby", this plays the primary segment
          on the 3D audiopath by calling PlaySegmentEx passing in 
          DMUS_SEGF_DEFAULT and the 3D audiopath.
        - If its the second or third button, this plays a secondary segment
          on the 3D audiopath by calling PlaySegmentEx passing in 
          DMUS_SEGF_DEFAULT or DMUS_SEGF_SECONDARY and the 3D audiopath.
        - If its the forth button, "Rude Awakening", this plays a primary segment
          on the 3D audiopath by calling PlaySegmentEx passing in 
          the 3D audiopath, and setting the pFrom to the 3D audiopath. 
          This causes all currently playing segments to stop when this one starts.
 
