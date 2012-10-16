//-----------------------------------------------------------------------------
// 
// Sample Name: FullScreenMode Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  FullScreenMode demonstrates the tasks required to initialize and run 
  a full-screen DirectDraw application.

Path
====
  Source: DXSDK\Samples\Multimedia\DDraw\FullScreenMode

  Executable: DXSDK\Samples\Multimedia\DDraw\Bin

User's Guide
============
  FullScreenMode requires no user input. Press the ESC key to quit the program.

Programming Notes
=================
  The basic tasks to author a simple full-screen DirectDraw application are as follows:
  
  Initialize DirectDraw: 
    1. Register a window class and create a window.  
    2. Call DirectDrawCreateEx to create a DirectDraw object
    3. Call SetCooperativeLevel to set the DirectDraw cooperative level 
       to exclusive and full-screen. 
    4. Call SetDisplayMode to set the display mode, for example 640x480x8.
    5. Call CreateSurface to create a flipable primary surface with 1 back buffer.
    6. Call GetAttachedSurface to obtain a pointer to the back buffer.
    7. If the display mode was set to palettized color, a palette is needs 
       to be created.  This sample displays a single bitmap so it can read the 
       bitmap palette info to read and create a DirectDraw palette.  After a palette 
       is created, call SetPalette to set the palette for the primary surface.
    8. Create an off-screen plain DirectDraw surface, and load media content into it.  
       For example, this sample calls DDUtil_CreateSurfaceFromBitmap() to do just 
       this. 

  When the app is idle, and it is not hidden or minimized then render the next 
  frame as follows:
    1. If movement or animation is involved in the app, then calculate how much 
       time has passed since the last time the frame was displayed.
    2. Move or animate the app state based on how much time has passed.
    3. Draw the current state into the backbuffer.
    4. Call Flip to flip the contents of the backbuffer into the primary surface.  
  
  If the user alt-tabs away from the app, then the DirectDraw surface may be lost 
  (resulting in a DirectDraw call returning DDERR_SURFACELOST), then handle it by:
    1. Call RestoreAllSurfaces to have DirectDraw restore all the surfaces.  
    2. Restoring a surface doesn't reload any content that existed in the surface 
       prior to it being lost. So you must now redraw the graphics the surfaces 
       once held.  For example, this sample handles this by calling 
       DDUtil_ReDrawBitmapOnDDS()


  
  
