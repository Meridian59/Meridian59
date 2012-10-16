//-----------------------------------------------------------------------------
// Name: ShadowVolume Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The ShadowVolume sample uses stencil buffers to implement real-time shadows.
   In the sample, a complex object is rendered and used as a shadow-caster, to
   cast real-time shadows on itself and on the terrain below.

   Stencil buffers are a depth buffer technique that can be updated as
   geometry is rendered, and used again as a mask for drawing more geometry.
   Common effects include mirrors, shadows (an advanced technique), dissolves,
   etc..

   Note that not all cards support all features for all the various stencil
   buffer techniques (some hardware has no, or limited, stencil buffer
   support). For more information on stencil buffers, refer to the DirectX SDK
   documentation. 


Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\StencilBuffer\ShadowVolume
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
   Real-time shadows is a fairly advanced technique. Each frame, or as the
   geometry or lights in the scene are moves, an object called a shadow volume
   is computed. A shadow volume is an actual 3D object which is the siholuette
   of the shadowcasting object, as pretruded away from the light source.

   In this sample, the 3D object which casts shadows is a bi-plane. Each frame,
   the sihlouette of the plane is computed (using an edge detection algorithm,
   in which sihlouette edges are found because the normals of adjacent polygons
   will have opposing normals with respect to the light vector). The resulting
   edge list (the sihlouette) is pretuded into a 3D object away from the light
   source. This 3D object is known as the shadow volume, as everypoint inside
   the volume is inside a shadow.

   Next, the shadow volume is rendering into the stencil buffer twice. First,
   only forward-facing polygons are rendering, and the stencil buffer values 
   are incremented each time. Then the back-facing polygons of the shadow
   volume are drawm, decrementing values in the stencil buffer. Normally, all
   incremented and decremented values would cancel each other out. However,
   because the scene was already rendered with normal geometry (the plane and
   the terrain, in this case), some pixels will fail the zbuffer test as the 
   shadowvolume is rendered. Any values left in the stencil buffer correspond
   to pixels that are in the shadow.

   Finally, these remaining stencil buffer contents are used as a mask, as a
   large all-encompassing black quad is alpha-blended into the scene. With the
   stencil buffer as a mask, only pixels in shadow are darkened

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common
