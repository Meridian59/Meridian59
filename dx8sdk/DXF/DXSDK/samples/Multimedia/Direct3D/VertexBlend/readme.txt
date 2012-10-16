//-----------------------------------------------------------------------------
// Name: VertexBlend Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The VertexBlend sample demonstrates a technique called vertex blending (also
   known as surface skinning). It displays a file-based object which is made to
   bend is various spots.

   Surface skinning is an impressive technique used for effects like smooth 
   joints and bulging muscles in character animations.

   Note that not all cards support all features for vertex blending. For more
   information on vertex blending, refer to the DirectX SDK documentation. 


Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\VertexBlend
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
   Vertex blending requires each vertex to have an associated blend weight. 
   Multiple world transforms are set up using SetTransformState() and the
   blend weights determine how much contribution each world matrix has when
   positioning each vertex.

   In this sample, a mesh is loaded using the common helper code. What is
   important is how a custom vertex and a custom FVF is declared and used
   to build the mesh (see the SetFVF() call for the mesh object). Without
   using the mesh helper code, the technique is the same: just create a
   vertex buffer full of vertices that have a blend weight, and use the
   appropriate FVF.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common
