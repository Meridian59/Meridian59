//-----------------------------------------------------------------------------
// Name: BumpEarth Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The BumpEarth program demonstrates the bump mapping capabilities of Direct3D. 
   Bumpmapping is a texture blending technique used to render the appearance of 
   rough, bumpy surfaces. This sample renders a rotating, bumpmapped planet Earth.

   Note that not all cards support all features for all the various bumpmapping
   techniques (some hardware has no, or limited, bumpmapping support). For more
   information on bumpmapping, refer to the DirectX SDK documentation. 


Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\BumpMapping\BumpEarth
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
   Bumpmapping is an advanced multitexture blending technique that can be used
   to render the appearance of rough, bumpy surfaces. The bump map itself is a 
   texture that stores the perturbation data. Bumpmapping requires two
   textures, actually. One is an environment map, which contains the lights 
   that you see in the scene. The other is the actual bumpmapping, which 
   contain values (stored as du and dv) used to "bump" the environment maps 
   texture coordinates. Some bumpmaps also contain luminance values to control 
   the "shininess" of a particular texel.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common
