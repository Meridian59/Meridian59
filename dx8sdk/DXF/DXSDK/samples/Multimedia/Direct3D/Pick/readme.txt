//-----------------------------------------------------------------------------
// Name: Pick Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The Pick sample shows how to implement "picking" which is finding which
   triangle in a mesh is intersected by a ray. In this case, the ray comes 
   from mouse coordinates.


Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\Pick
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
   
   Use the mouse to "pick" any spot in the mesh to see that triangle.


Programming Notes
=================
   When the user clicks the mouse, the code reads the screen coordinates of the 
   cursor.  These coordinates are converted, via the projection and view matrices,
   into a ray which goes from the eye point through the point clicked on the screen
   and into the scene.  This ray is passed to IntersectTriangle along with each
   triangle of the loaded model to determine which triangles, if any, are 
   intersected by the ray.  The texture coordinates of the intersected triangle
   is also determined.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common
