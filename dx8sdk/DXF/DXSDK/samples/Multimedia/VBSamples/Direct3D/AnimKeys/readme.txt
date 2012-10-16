//-----------------------------------------------------------------------------
// 
// Sample Name: AnimKeys Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
The AnimKeys sample illustrates how to use the d3d framework to load an x-file with
key framed animation and playback the animation.nt 

  
Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\Direct3D\AnimKeys

  Executable: DXSDK\Samples\Multimedia\VBSamples\Direct3D\Bin


User's Guide
============
   The following keys are implemented. The dropdown menus can be used for the
   same controls.

      <F2>        Prompts user to select a new rendering device or display mode
      <Alt+Enter> Toggles between fullscreen and windowed modes
      <Esc>       Exits the app.
	


Programming Notes
=================
   Note that the last argument passed to D3DUtil_LoadFromFile is a CD3DAnimation 
   class that is the parent to any animations that are found in the xfile. 
   Subsequently Animation.SetTime can be used to pose the model.


   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   classes and modules can be found in the following directory:
      DXSDK\Samples\Multimedia\VBSamples\Common

