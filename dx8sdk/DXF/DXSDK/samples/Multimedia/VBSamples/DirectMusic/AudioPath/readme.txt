//-----------------------------------------------------------------------------
// 
// Sample Name: Audio Path Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
This sample shows the effects of playing multiple files across different audio
paths, how they can be mixed, and how music is played at the tempo of the primary
audio path.
  
Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectMusic\vbAudioPath

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectMusic\bin


User's Guide
============
You may add one file to the 'Primary' audio path, and 0 to 5 files to each of the two
secondary audio paths.  Each audio path can be played separately and has separate controls
for volume and 3D positioning.

Programming Notes
=================
At the start of the application we create 3 default audio paths.  We allow 1 audio file 
to be loaded into the first audio path, and 0 to 5 loaded in each of the next two.  When
Play is clicked on the first audio path, we call PlaySegmentEx with the DMUS_SEGF_DEFAULT
flag to clarify that this is the primary segment.  Each of the other audio paths call
PlaySegmentEX with the DMUS_SEGF_SECONDARY flag to play as secondary segments.

If Looping is selected on an audio path we call SetRepeats with INIFINITE, otherwise we 
call SetRepeats with 0.  To change the volume or 3D Positioning of any audio path we first
call GetObjectInPath to retreive a DirectSoundSecondaryBuffer or DirectSound3DBuffer, and 
then call SetVolume or SetPosition, respectively.
