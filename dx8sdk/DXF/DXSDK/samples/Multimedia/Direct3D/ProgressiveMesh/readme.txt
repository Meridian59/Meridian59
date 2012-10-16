//-----------------------------------------------------------------------------
// Name: ProgressiveMesh Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The ProgressiveMesh sample illustrates how to load and optimize a file-based
   mesh using the D3DX mesh utility functions. A progressive mesh is one in
   which the vertex information is stored internally in a special tree which
   can be accessed to render the mesh with any given number of vertices. This
   procedure is fast, so progressive meshes are ideal for level-of-detail
   scenarios, where objects in the distance are render with fewer polygons.

   For more info on D3DX, refer to the DirectX SDK documentation.


Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\ProgressiveMesh
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

	  <Up>        Adds one vertex to the progressive mesh
	  <Down>      Subtracts one vertex to the progressive mesh
	  <Page Up>   Adds 100 vertices to the progressive mesh
	  <Page Dn>   Subtracts 100 vertices to the progressive mesh
	  <Home>      Displays all available vertices for the progressive mesh
	  <End>       Displays the minimum vertices for the progressive mesh


Programming Notes
=================
   Many Direct3D samples on the DirectX SDK use file-based meshes. This sample
   is a good sample to look to see the bare bones code necessary for loading a
   mesh. Note that the D3DX mesh loading functionality collapses the frame 
   hierarchy of a .x file into one mesh.

   The main difference for using progressive meshes is the call to 
   SetNumVertices() for the mesh.
   
   For other samples, the bare bones D3DX mesh functionality is wrapped in a 
   common class class CD3DMesh. If it is desired to keep the frame hierarchy,
   the common class CD3DFile can be used.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common
