//-----------------------------------------------------------------------------
// 
// Sample Name: VB Deferred Effects Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
//
// GM/GS® Sound Set Copyright ©1996, Roland Corporation U.S.
// 
//-----------------------------------------------------------------------------



Description
===========
The sample demonstrates:
How to use standard effects (FX) with DirectSound and AcquireResources


Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectSound\DefferedEffects

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectSound\Bin


User's Guide
============
- make sure a sound file is loaded (can be WAV)

- by default, no FX are enabled.  try playing the sound to see what it
  orginally sounds like.

- enable one or more FXs by adding them to the 'In use' listbox 

- Click Apply to call AcquireResources and load the effects (the listbox will now show
  you were they were allocated, ie hardware or software).

- Hit play to hear the FX applied.


Programming Notes
=================
Fill one or more DSEFFECTDESC structs, and pass them into DirectSoundSecondaryBuffer.SetFX,
and then call AcquireResources before playing the buffer.
