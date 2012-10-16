//-----------------------------------------------------------------------------
// Name: Vertices Direct3D Tutorial
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The Vertices tutorial demonstrates the necessary API to render vertices
   using Direct3D.

   
Path
====
   Source:     DXSDK\Samples\Multimedia\VBSamples\D3D\Tutorials\Tut02_Vertices


Programming Notes
=================
   To render geometry in Direct3D, a vertex buffer must be created and filled
   with vertices that described the geometry. Vertices can have many components
   including positions, normals, blend weights, colors, and texture
   coordinates. This simple tutorial uses vertices with only positions and
   colors. The important parts of the tutorial are vertex buffer creation,
   locking and filling the vertex buffer, and rendering the vertex buffer.

