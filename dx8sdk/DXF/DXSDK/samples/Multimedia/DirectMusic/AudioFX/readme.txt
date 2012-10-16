//-----------------------------------------------------------------------------
// 
// Sample Name: AudioFx Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//
// GM/GS® Sound Set Copyright ©1996, Roland Corporation U.S.
// 
//-----------------------------------------------------------------------------



Description
===========
The sample demonstrates:

(1) how to use standard effects (FX) with DirectMusic
(2) how to manipulate FX parameters - and what the results sounds like


Path
====
  Source: DXSDK\Samples\Multimedia\DirectMusic\AudioFx

  Executable: DXSDK\Samples\Multimedia\DirectMusic\Bin


User's Guide
============
- make sure a sound file is loaded (can be WAV, MID, or RMI), a default
  sound file should be loaded for you.

- by default, no FX are enabled.  try playing the sound to see what it
  orginally sounds like.

- enable one or more FXs by checking the checkboxes on the left, under
  the column "Enable".

- Hit play to hear the FX applied.

- you can adjust parameters for any FX by using the frame on the right.
  (choose which FX to adjust by choosing an option under the "Adjust"
  column on the left.  If you are adjusting parametesr for an active
  FX while sound is playing, you will hearing the difference immediately.


Programming Notes
=================
To Enable a stand effect, ultimately, you need to obtain a LPDIRECTSOUNDBUFFER8
interface pointer.  Fill one or more DSEFFECTDESC structs, and pass them into
IDirectSoundBuffer8::SetFX( ).  This functionality is encapsulated by the sample
in the CSoundFXManager.

Much of the code is for manipulating the UI, and is loosely coupled with
the FX code.