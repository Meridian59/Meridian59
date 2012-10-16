//-----------------------------------------------------------------------------
// Name: DotProduct3 Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
  The DotProduct3 samples demonstrates an alternative approach to Direct3D
  bumpmapping. This technique is named after the mathematical operation which
  combines a light vector with a surface normal. The normals for a surface are
  traditional (x,y,z) vectors stored in RGBA format in a texture map (called a
  normal map, for this technique).

  Not all cards support DotProduct3 blending teture stages, but then not all
  cards support Direct3D bumpmapping. Refer to the DirectX SDK documentation
  for more information.


Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\BumpMapping\DotProduct3
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
   The lighting equation for simulating bump mapping invloves using the dot
   product of the surface normal and the lighting vector. The lighting vector
   is simply passed into the texture factor, and the normals are encoded in a
   texture map. The blend stages, then, look like
       SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_DOTPRODUCT3 );
       SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
       SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );

   The only trick then, is getting the normals stored in the texture. To do
   this, the components of a vector (XYZW) are each turned from a 32-bit
   floating value into a signed 8-bit integer and packed into a texture color
   (RGBA). The code show how to do this using a custom-generated normal map,
   as well as one built from an actual bumpmapping texture image.

   Note that not all cards support all features for all the various bumpmapping
   techniques (some hardware has no, or limited, bumpmapping support). For more
   information on bumpmapping, refer to the DirectX SDK documentation. 

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common
