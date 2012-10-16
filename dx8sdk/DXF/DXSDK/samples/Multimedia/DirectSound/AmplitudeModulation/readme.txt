//-----------------------------------------------------------------------------
// 
// Sample Name: AmplitudeModulation Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  The AmplitudeModulation sample shows how to apply an effect to a 
  DirectSound secondary buffer and modify the parameters of the effect.  
  
Path
====
  Source: DXSDK\Samples\Multimedia\DSound\AmplitudeModulation

  Executable: DXSDK\Samples\Multimedia\DSound\Bin

User's Guide
============
  Play the default sound or load another wave file by clicking Sound File. 
  Change the parameters of the effect by selecting one of the Wave Form 
  options and moving the slider to change the modulation rate.
  
Programming Notes
=================  
  For a simpler example of how to setup a DirectSound buffer without a 
  DirectSound effect, see the PlaySound sample. 
  
  * To set an effect on a buffer
        1. Make sure the buffer is created with the DSBCAPS_CTRLFX flag.
        2. Fill out a DSEFFECTDESC struct setting the guidDSFXClass 
           to the GUID of the effect desired.
        3. Call IDirectSoundBuffer8::SetFX passing in the DSEFFECTDESC struct.  
        4. Call IDirectSoundBuffer8::GetObjectInPath to get a interface
           pointer to the effect in the buffer, such as IDirectSoundFXGargle.

  * To control various parameters of the gargle effect:
        1. Fill out a DSFXGargle struct with desired params
        2. Call IDirectSoundFXGargle::SetAllParameters
            
   
  
  