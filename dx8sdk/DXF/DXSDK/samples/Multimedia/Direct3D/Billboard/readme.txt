//-----------------------------------------------------------------------------
// Name: Billboard Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The Billboard sample illustrates the billboarding technique. Rather than
   rendering complex 3D models (such as a high-polygon tree model),
   billboarding renders a 2D image of the model and rotates it to always face
   the eyepoint. This technique is commonly used to render trees, clouds,
   smoke, explosions, and more. For more information, see 
   Common Techniques and Special Effects.

   The sample has a camera fly around a 3D scene with a tree-covered hill. The
   trees look like 3D objects, but they are actually 2-D billboarded images
   that are rotated towards the eye point. The hilly terrain and the skybox 
   (6-sided cube containing sky textures) are just objects loaded from .x
   files, used for visual effect, and are unrelated to the billboarding
   technique.
   
Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\Billboard
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
   The billboarding technique is the focus of this sample. Each frame, the
   camera is moved, so the viewpoint changes accordingly. As the viewpoint
   changes, a rotation matrix is generated to rotate the billboards about
   the y-axis so that they face the new viewpoint. The computation of the 
   billboard matrix occurs in the FrameMove() function. The trees are also
   sorted in that function, as required for proper alpha blanding, since 
   billboards typically have some transparent pixels. The trees are 
   rendered from a vertex buffer in the DrawTrees() function.
   
   Note that the billboards in this sample are constrained to rotate about the 
   y-axis only, as otherwise the tree trunks would appear to not be fixed to 
   the ground. In a 3D flight sim or space shooter, for effects like 
   explosions, billboards are typically not constrained to one axis.
   
   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common

