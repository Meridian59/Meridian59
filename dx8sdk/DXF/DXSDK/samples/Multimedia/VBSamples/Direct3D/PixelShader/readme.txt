//-----------------------------------------------------------------------------
// Name: PixelShader Direct3D Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   This sample shows some of the effects that can be achieved using pixel 
   shaders. Each of the 8 thumbnails shown is the result of using a different
   pixel shader to render a rectangle
   
   Pixel shaders use a set of instructions, executed by the 3D
   device on a per-pixel basis, that can affect the colorof the 
   pixel based on a varient of inputs. Pixel shaders can be used in place of
   the texture stage pipeline.

   Note that not all cards may support all the various features pixel shaders.
   For more information on pixel shaders, refer to the DirectX SDK 
   documentation. 


Path
====
   Source:     DXSDK\Samples\Multimedia\VBSamples\Direct3D\PixelShader
   Executable: DXSDK\Samples\Multimedia\VBSamples\Direct3D\Bin


User's Guide
============
   This sample has no user interaction


Programming Notes
=================
   Programming pixel shaders is not a trivial task. Please read any pixel
   shader-specific documentation accompanying the DirectX SDK.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   classes and modules can be found in the following directory:
      DXSDK\Samples\Multimedia\VBSamples\Common
