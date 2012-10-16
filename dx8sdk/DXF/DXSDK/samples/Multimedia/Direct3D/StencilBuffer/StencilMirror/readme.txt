//-----------------------------------------------------------------------------
// Name: StencilMirror Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The StencilMirror sample uses stencil buffers to implement a mirror effect.
   In the sample, a watery terrain scene is rendered with the water reflecting
   a helicopter that flies above.

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
   Source:     DXSDK\Samples\Multimedia\D3D\StencilBuffer\StencilMirror
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
   In this sample, a stencil buffer is used to create the effect of a 
   reflection coming off the water. The geometry of the water is rendered into
   the stencil buffer. Then, the stencil buffer is used as a mask to render the
   scene again, this time with the geometry translated and rendered upside
   down, to appear as if it was reflected in the mirror.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common
