//-----------------------------------------------------------------------------
// Name: Water Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The Water sample illustrates using D3DX techniques stored in shader files.

   The sample shows a square pound inside a building, with rippling water
   effects including water caustics.

   
Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\Water
   Executable: DXSDK\Samples\Multimedia\D3D\Bin


User's Guide
============
   The following keys are implemented. The dropdown menus can be used for the
   same controls.
      <Enter>     Starts and stops the scene
      <Space>     Advances the scene by a small increment
      <F1>        Shows help or available commands.
      <F2>        Prompts user to select a new rendering device or display mode
      <Alt+Enter> Toggles between fullscreen and windowed modes
      <Esc>       Exits the app.
      <D>         Starts a water drop
      NumPad      Navigates the scene
	

Programming Notes
=================
   The D3DX shaders technique is the focus of this sample. 
   
   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common

