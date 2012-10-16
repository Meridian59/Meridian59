//-----------------------------------------------------------------------------
// 
// Sample Name: GammaControl Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  GammaControl demonstrates how to use IDirectDrawGammaControl to adjust
  how bright the system displays the DirectDraw surface.

Path
====
  Source: DXSDK\Samples\Multimedia\DDraw\GammaControl

  Executable: DXSDK\Samples\Multimedia\DDraw\Bin

User's Guide
============
  Click the left mouse button to decrease the gamma ramp or the right mouse button
  to increase it.  Press the ESC key to quit the program.

Programming Notes
=================
  For details on how to setup a full-screen DirectDraw app, see the FullScreenMode 
  sample. 
  
  To adjust the gamma, first check to see if the device supports it.  Call
  IDirectDraw::GetCaps to check if DDCAPS2_PRIMARYGAMMA is set.  If it is, then
  you can set the gamma ramp by calling IDirectDrawGammaControl::SetGammaRamp.  
  
  For simplicity this sample creates a gamma ramp which is linear for all 
  color components.  The ramp runs from 0 at index 0 linearly up to a user 
  defined number at index 255 for each of the color components.  However, the ramp 
  does not need to be linear.
  
  
