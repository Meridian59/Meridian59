//-----------------------------------------------------------------------------
// Name: CubeMap Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The CubeMap sample demonstrates an enviroment-mapping technique called 
   cube-mapping. Environment-mapping is a technique in which the environment
   surrounding a 3D object (such as the lights, etc.) are put into a texture
   map, so that the object can have complex lighting effects without expensive
   lighting calculations.

   Note that not all cards support all features for all the various environment
   mapping techniques (such as cubemapping and projected textures). For more
   information on environment mapping, cubemapping, and projected textures, 
   refer to the DirectX SDK documentation. 


Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\EnvMapping\CubeMap
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
   Cube-mapping is a technique which employs a 6-sided texture. Think of the
   being inside a wall-papered room, and having the wallpaper shrink-wrapped
   around an object. Cube-mapping is superior to sphere-mapping because the
   latter is inherently view-dependant (spheremaps are constructed for one
   particular viewpoint in mind). Cubemaps also have no geometry
   distortions, so they can be generated on the fly using SetRenderTarget()
   for each of the 6 cubemap's faces.

   Cube-mapping works with Direct3D texture coordinate generation. By setting
   D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR, Direct3D will generate cubemap 
   texture coordinates from the reflection vector for a vertex, thereby making
   this technique easy for environment-mapping effects where the environment
   is reflected in the object.

   On hardware which does not support cube-maps, this sample will dynamically
   create a spheremap each frame, and use that instead.  Creation of the
   spheremap is done using the ID3DXRenderToEnvMap interface.
   
   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common
