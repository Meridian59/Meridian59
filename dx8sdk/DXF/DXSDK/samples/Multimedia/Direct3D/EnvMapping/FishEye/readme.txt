//-----------------------------------------------------------------------------
// Name: FishEye Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The FishEye sample shows a fish eye lens effect that can be achieved using
   cubemaps. 
   

Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\EnvMapping\FishEye
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


Programming Notes
=================
   The scene is actually rendered into the surfaces of a cubemap (rather than
   the back buffer). Then, the cubemap is used as an environment-map (see the 
   CubeMap sample for more information about cube-mapping) to reflect the
   scene in some distorted geometry...in this case, some geometry approximating
   a fish eye lens.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common
