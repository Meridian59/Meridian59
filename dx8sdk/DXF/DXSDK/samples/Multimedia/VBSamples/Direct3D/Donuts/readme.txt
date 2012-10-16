//-----------------------------------------------------------------------------
// 
// Sample Name: Donuts Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
The Donuts sample illustrates how to use d3d to create a 2d sprite engine.

  
Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\Direct3D\Donuts

  Executable: DXSDK\Samples\Multimedia\VBSamples\Direct3D\Bin


User's Guide
============
   <Alt-Enter>  will bring you fullscreen to 640x480 and back to windowed mode
   <Esc>	exits the application
  	


Programming Notes
=================   

   The background is rendered with 2 TLVertex (screen space) triangles. The sprites
   are defined as an array of user defined type that is a container for sprite properties
   such as position, direction, speed and size. Each of the sprites is rendered as 2
   TLVertex triangles each frame.
