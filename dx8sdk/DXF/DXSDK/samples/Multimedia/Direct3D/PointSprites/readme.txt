//-----------------------------------------------------------------------------
// Name: PointSprites Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The PointSprites sample shows how to use the new Direct3D point sprites
   feature. A point sprite is simply a forward-facing, textured quad that is
   referenced only by (x,y,z) position coordinates. Point sprites are most
   often used for particle systems and related effects.

   Note that not all cards support all features for point sprites. For more
   information on point sprites, refer to the DirectX SDK documentation. 


Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\PointSprites
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
   Without Direct3D's support, point sprites can be implemented with four
   vertices, that are oriented each frame towards the eyepoint (much like a
   billboard). With Direct3D, though, you can refer to each point sprite by
   just it's center position and a radius. This saves heavily on processor
   computation time and on bandwidth uploading vertex information to the
   graphics card.

   In this sample, a particle system is implemented using point sprites. Each
   particle is actually implemented using multiple alpha-blended point sprites,
   giving the particle a motion-blur effect.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common
