//-----------------------------------------------------------------------------
// 
// Sample Name: SkinnedMesh Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
The SkinnedMesh sample illustrates how to use the d3d framework to load an x-file with
skinning and animation information in it. 

  
Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\Direct3D\SkinnedMesh

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
   Note that the last argument passed to D3DUtil_LoadFromFileAsSkin is a CD3DAnimation 
   class that is the parent to any animations that are found in the xfile. 
   Animation.SetTime must be called but will not pose the model.
   Character.UpdateFrames computes the matrices for all joints on the character
   Character.RenderSkin will render the character using the loaded skin
   

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   classes and modules can be found in the following directory:
      DXSDK\Samples\Multimedia\VBSamples\Common

   The modeling exporters in the extras directory of the SDK can export to x with skinning infromation.
