//-----------------------------------------------------------------------------
// 
// Sample Name: WindowedMode Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  WindowedMode demonstrates the tasks required to initialize and run 
  a windowed DirectDraw application.

Path
====
  Source: DXSDK\Samples\Multimedia\DDraw\WindowedMode

  Executable: DXSDK\Samples\Multimedia\DDraw\Bin

User's Guide
============
  WindowedMode requires no user input. Press the ESC key to quit the program.

Programming Notes
=================
  The basic tasks to author a simple windowed DirectDraw application are as follows:
  
  Initialize DirectDraw: 
    1. Register a window class and create a window.   
    2. Call DirectDrawCreateEx to create a DirectDraw object
    3. Call SetCooperativeLevel to set the DirectDraw cooperative level 
       to normal.
    4. Call CreateSurface to obtain a pointer to the primary surface.  In windowed mode,
       the primary surface is also the desktop window.  So the pixel format for the 
       primary surface is based on the user's selection of display resolution. 
    5. Create a back-buffer.  In windowed mode, this is should be an off-screen 
       plain buffer. The back buffer is typically the same size as the window's 
       client rect, but could be created with any size.  
    6. Test to see if the display mode is in palettized color (8-bit or less).  
       If it is then a palette needs to be created. This sample displays a single 
       bitmap so it can read the bitmap palette info to read and create a DirectDraw 
       palette.  After a palette is created, call SetPalette to set the palette for 
       the primary surface. 
    7. Call CreateClipper and SetClipper to create a clipper object, and attach it
       to the primary surface.  This will keep DirectDraw from blting on top of 
       any windows which happen to overlap.      
    8. Create an off-screen plain DirectDraw surface, and load media content into it.  
       For example, this sample calls DDUtil_CreateSurfaceFromBitmap() to do just 
       this. 

  When the app is idle, and it is not hidden or minimized then render the 
  next frame by:
    1. If movement or animation is involved in the app, then calculate how much 
       time has passed since the last time the frame was displayed.
    2. Move or animate the app state based on how much time has passed.
    3. Draw the current state into the off-screen plain backbuffer.
    4. Call Blt to blt the contents of the off-screen plain backbuffer into 
       the primary surface.
  
  If the display resolution changes or an exclusive mode DirectDraw app is run, then 
  the DirectDraw surface may be lost (resulting in a DirectDraw call returning 
  DDERR_SURFACELOST), then handle it by:
    1. Call RestoreAllSurfaces to have DirectDraw restore all the surfaces.  
    2. Restoring a surface doesn't reload any content that existed in the surface 
       prior to it being lost. So you must now redraw the graphics the surfaces 
       once held.  For example, this sample handles this by calling 
       DDUtil_ReDrawBitmapOnDDS()

  In windowed mode, handle the following windows messages:
    1. WM_PAINT: This is sent when all or a part of the window is needs to be 
       redrawn.  The app may not be active at the time this is called, so if this 
       is not handled then the window will appear blank.  To avoid this, make a 
       call to draw the next frame here.
    2. WM_QUERYNEWPALETTE: This is sent when in a 8-bit desktop and the another 
       window set a new palette, but now this window has control so it needs to reset
       its palette.  The easy way to make this happen in DirectDraw is just to call
       SetPalette.  This will force DirectDraw to realize the DirectDrawPalette 
       attached to it. 
    3. WM_MOVE: This is sent when the window is moving.  Record the new window position
       here since the blt to the primary surface needs to know the window position.
      
      
      
      