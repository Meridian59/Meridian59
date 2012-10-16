//-----------------------------------------------------------------------------
// Name: VolumeTexture Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The VolumeTexture sample illustrates how to use the new volume textures in
   Direct3D. Normally, a texture is thought of as a 2D image, which have a
   width and a height and whose "texels" are addressed with two coordinate, 
   tu and tv. Volume textures are the 3D counterparts, with a width, height,
   and depth, are are addressed with three coordinates, tu, tv, and tw.

   Volume textures can be used for interesting effects like patchy fog,
   explosions, etc..
   
   Note that not all cards support all features for volume textures. For more
   information on volume textures, refer to the DirectX SDK documentation. 


Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\VolumeTexture
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
   Volume textures are no more difficult to use than 3D textures. In this
   sample source code, look for the vertex declaration (which has a third 
   texture coordinate), texture creation (which also takes a depth dimension),
   and texture locking (again with the third dimension). The 3D rasterizer
   interpolates texel values much as it would for 2D textures.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common
