//-----------------------------------------------------------------------------
// Name: Lights Direct3D Tutorial
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The Lights tutorial shows how to use dynamic lighting in Direct3D.

   
Path
====
   Source:     DXSDK\Samples\Multimedia\VBSamples\D3D\Tutorials\Tut04_Lights


Programming Notes
=================
   Dynamic lighting makes 3D objects look more realistic. Lights come in a few
   flavors, notably point lights and directional lights. Geometry gets lit by
   every light in the scene, so adding lights increases rendering time. Point
   lights have a poistion and are computationally more expensive than directional
   lights, which only have a direction (as if the light source is infinitely far
   away). Internal Direct3D lighting calculations require surface normals, so note
   that normals are added to the vertices. Also, material properties can be set,
   which describe how the surface interacts with the light (i.e. it's color).

