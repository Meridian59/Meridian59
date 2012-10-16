//-----------------------------------------------------------------------------
// Name: ClipMirror Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The ClipMirror sample demonstrates the use of custom-defined clip planes.
   A 3D scene is rendered normally, and then again in a 2nd pass as if reflected
   in a planar mirror. Clip planes are used to clip the reflected scene to the
   edges of the mirror.


Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\ClipMirror
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

   The mouse is also used in this sample to control the viewing position.


Programming Notes
=================
   The main feature of this sample is the use of clip planes. The rectangular 
   mirror has four edges, so four clip planes are used. Each plane is defined 
   by the eye point and two vertices of one edge of the mirror. With the clip
   planes in place, the view matrix is reflected in the mirror's plance, and 
   then the scene geometry (the teapot object) can be rendered as normal. 
   Afterwards, a semi-transparent rectangle is drawn to represent the mirror
   itself.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common

