//-----------------------------------------------------------------------------
// Name: OptimizedMesh Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The OptimizedMesh sample illustrates how to load and optimize a file-based
   mesh using the D3DX mesh utility functions.

   For more info on D3DX, refer to the DirectX SDK documentation.


Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\OptimizedMesh
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
   Many Direct3D samples on the DirectX SDK use file-based meshes. This sample
   is a good sample to look to see the bare bones code necessary for loading a
   mesh. Note that the D3DX mesh loading functionality collapses the frame 
   hierarchy of a .x file into one mesh.
   
   For other samples, the bare bones D3DX mesh functionality is wrapped in a 
   common class class CD3DMesh. If it is desired to keep the frame hierarchy,
   the common class CD3DFile can be used.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common

