F//-----------------------------------------------------------------------------
// Name: SphereMap Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The SphereMap sample demonstrates an enviroment-mapping technique called 
   sphere-mapping. Environment-mapping is a technique in which the environment
   surrounding a 3D object (such as the lights, etc.) are put into a texture
   map, so that the object can have complex lighting effects without expensive
   lighting calculations.

   Note that not all cards support all features for all the various environment
   mapping techniques (such as cubemapping and projected textures). For more
   information on environment mapping, cubemapping, and projected textures, 
   refer to the DirectX SDK documentation. 


Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\EnvMapping\SphereMap
   Executable: DXSDK\Samples\Multimedia\D3D\Bin


User's Guide
============
   The following keys are implemented. The dropdown menus can be used for the
   same controls.
      <Enter>     Starts and stops the scene
      <Space>     Advances the scene by a small increment
      <F2>        Prompts user to select a new rendering device or display mode
      <Alt+Enter> Toggles between fullscreen and windowed modes
      <Esc>       Exits the app.


Programming Notes
=================
   Sphere-mapping uses a precomputed (at model time) texture map which contains
   the entire environment as reflected by a chrome sphere. The idea is to
   consider each vertex, compute it's normal, find where the normal matches up
   on the chrome sphere, and then assign that texture coordinate to the vertex.

   The math involves computations for each vertex for every frame.  This sample 
   uses a vertex shader to perform these calculations.  Basically, the vertex 
   shader computes a camera-space reflection vector, given the camera position, 
   vertex position, and vertex normal.  Then, it computes the half-angle vector 
   between the reflection vector, and the view direction.  This vector can then 
   be scaled and offset by 0.5 to index into the spheremap texture.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common
