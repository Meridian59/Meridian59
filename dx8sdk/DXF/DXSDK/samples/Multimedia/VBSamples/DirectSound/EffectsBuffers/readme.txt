//-----------------------------------------------------------------------------
// 
// Sample Name: Effects Form Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
//
// GM/GS® Sound Set Copyright ©1996, Roland Corporation U.S.
// 
//-----------------------------------------------------------------------------



Description
===========
The sample demonstrates:

(1) how to use standard effects (FX) with DirectSound
(2) how to manipulate FX parameters - and what the results sounds like


Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectSound\EffectsForm

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectSound\Bin


User's Guide
============
- make sure a sound file is loaded (can be WAV)

- by default, no FX are enabled.  try playing the sound to see what it
  orginally sounds like.

- enable one or more FXs by adding them to the list of in use effects.

- Hit Apply to introduce the effects.

- Hit play to hear the FX applied.

- you can adjust parameters for any FX by selecting the effect in the 
  in use list box, and clicking the modify effects button beneath it.
  If you are adjusting parameters for an active FX while sound is playing, 
  you will hearing the difference immediately.


Programming Notes
=================
Fill one or more DSEFFECTDESC structs, and pass them into
DirectSoundSecondaryBuffer8.SetFX. 

When modify effects is clicked call GetObjectInPath to retreive an effects 
interface, and then you can call SetAllParameters to modify the effects as
they are playing.
