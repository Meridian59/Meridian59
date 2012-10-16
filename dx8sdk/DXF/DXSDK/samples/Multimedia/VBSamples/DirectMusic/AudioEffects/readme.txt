//-----------------------------------------------------------------------------
// 
// Sample Name: VB Audio Effects Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
//
// GM/GS® Sound Set Copyright ©1996, Roland Corporation U.S.
// 
//-----------------------------------------------------------------------------



Description
===========
The sample demonstrates:
How to use standard effects (FX) with DirectMusic


Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectMusic\AudioEffects

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectMusic\Bin


User's Guide
============
- make sure a sound file is loaded (can be WAV, MID, SGT, or RMI)

- by default, no FX are enabled.  try playing the sound to see what it
  orginally sounds like.

- enable one or more FXs by adding them to the 'In use' listbox and Apply them

- Hit play to hear the FX applied.


Programming Notes
=================
To Enable a standard effect, ultimately, you need to obtain a DirectSoundSecondaryBuffer.
Fill one or more DSEFFECTDESC structs, and pass them into DirectSoundSecondaryBuffer.SetFX.  
