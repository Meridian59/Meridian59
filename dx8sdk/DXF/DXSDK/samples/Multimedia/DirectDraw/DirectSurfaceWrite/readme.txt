//-----------------------------------------------------------------------------
// 
// Sample Name: DirectSurfaceWrite Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  DirectSurfaceWrite demonstrates how to write directly to a DirectDraw surface.

Path
====
  Source:     DXSDK\Samples\Multimedia\DDraw\DirectSurfaceWrite

  Executable: DXSDK\Samples\Multimedia\DDraw\Bin

User's Guide
============
  DirectSurfaceWrite requires no user input. Press the ESC key to quit the program.

Programming Notes
=================
  For details on how to setup a full-screen DirectDraw app, see the FullScreenMode 
  sample. 

  To write directly on a DirectDraw surface first call IDirectDrawSurface::Lock 
  to obtain a pointer directly into the memory of the DirectDraw surface.  While
  the surface is locked, the surface can not be blted or flipped onto other surfaces. 
  The surface's pixel format will tell you the data format the surface stores pixels
  in.  Be sure to advance the surface pointer by the surface pitch instead of the surface 
  width, since the surface may be wider than its width.  After the drawing is finished 
  call IDirectDrawSurface::Unlock to allow the surface to blt to other surfaces.  See 
  DrawSprite() in this sample for an example of how this is done.  
  
