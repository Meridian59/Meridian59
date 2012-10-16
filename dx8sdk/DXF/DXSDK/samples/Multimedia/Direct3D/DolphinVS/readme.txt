//-----------------------------------------------------------------------------
// Name: DolphinVS Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The DolphinVS sample shows an underwater scene of a dolphin swimming, with
   caustic effects on the dolphin and seafloor. The dolphin is animated using
   a technique called "tweening". The underwater effect simply uses fog, and
   the water caustics use an animated set of textures. These effects are
   achieved using vertex shaders


Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\DolphinVS
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
   Several things are happening in this sample. First of all is the use of 
   vertex shaders. Two vertex shaders are used: one for the dolphin, and one
   for the seafloor. The vertex shaders are assembly instructions which are 
   assembled from two files, DolphinTween.vsh and SeaFloor.vsh.

   The dolphin is tweened (a form of morphing) by passing three versions of
   the dolphin down in multiple vertex streams. The vertex shader takes the
   weighted positions of each mesh and produces and output position. The vertex
   is then lit, gets tex coords computed and gets fog added.

   The seafloor is handled similiarly, just with no need for tweening. The
   caustic effects are added in a separate alpha-blended pass, using an 
   animated set of 32 textures. The texture coordinates for the caustics are
   generated in the vertex shaders, stored as TEXCOORDINDEX stage 1.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common

