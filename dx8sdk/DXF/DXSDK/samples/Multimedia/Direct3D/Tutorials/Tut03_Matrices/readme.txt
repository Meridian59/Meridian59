//-----------------------------------------------------------------------------
// Name: Matrices Direct3D Tutorial
// 
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The Matrices tutorial shows how to use 4x4 matrices to transform vertices
   in Direct3D.

   
Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\Tutorials\Tut03_Matrices


Programming Notes
=================
   To render vertices in 3D, certain mathematical transformations must be
   performed on the vertices. This includes the world transform (which
   translates, rotates, and scales the geometry), the view transform (which
   orients the camera, or view) and the projection transform (which projects
   the 3D scene into 2D viewport). Transforms are represented mathematically
   as 4x4 matrices. This tutorial introdcues the use of the D3DX helper
   library, which contains (amongst other things) functions to build and
   manipulate our 4x4 tranform matrices.

