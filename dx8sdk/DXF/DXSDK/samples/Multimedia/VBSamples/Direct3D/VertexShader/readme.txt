//-----------------------------------------------------------------------------
// Name: VertexShader Direct3D Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   This sample shows some of the effects that can be achieved using vertex 
   shaders. Vertex shaders use a set of instructions, executed by the 3D
   device on a per-vertex basis, that can affect the properties of the 
   vertex (positions, normal, color, tex coords, etc.) in interesting ways.

   Note that not all cards may support all the various features vertex shaders.
   For more information on vertex shaders, refer to the DirectX SDK 
   documentation. 


Path
====
   Source:     DXSDK\Samples\Multimedia\VBSamples\Direct3D\VertexShader
   Executable: DXSDK\Samples\Multimedia\VBSamples\Direct3D\Bin


User's Guide
============
   The following keys are implemented. 

      <F2>        Prompts user to select a new rendering device or display mode
      <Alt+Enter> Toggles between fullscreen and windowed modes
      <Esc>       Exits the app.


Programming Notes
=================
   Programming vertex shaders is not a trivial task. Please read any vertex
   shader-specific documentation accompanying the DirectX SDK.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   classes and modules can be found in the following directory:
      DXSDK\Samples\Multimedia\VBSamples\Common
