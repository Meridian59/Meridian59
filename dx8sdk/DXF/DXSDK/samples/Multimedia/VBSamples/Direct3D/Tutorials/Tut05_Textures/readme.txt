//-----------------------------------------------------------------------------
// Name: Textures Direct3D Tutorial
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The Textures tutorial shows how to use texture mapping in Direct3D.

   
Path
====
   Source:     DXSDK\Samples\Multimedia\VBSamples\D3D\Tutorials\Tut05_Textures


Programming Notes
=================
   Texture-mapping is like shrink-wrapping a wall paper to a 3D object. A classic
   example is applying an image of wood to an otherwise plain cube, to give the
   appearance as if the block is actually made of wood. Textures (in their 
   simplest form) are 2D images, usually loaded from an image file. This tutorial
   shows how to use D3DX to create a texture from a file-based image and apply it
   to a geometry. Textures require the vertices to have texture coordinates, and
   make use of certain RenderStates and TextureStageStates and show in the source
   code.

