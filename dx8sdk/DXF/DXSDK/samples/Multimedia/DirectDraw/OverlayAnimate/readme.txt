//-----------------------------------------------------------------------------
// 
// Sample Name: OverlayAnimate Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  OverlayAnimate demonstrates how to use DirectDraw overlays.  

Path
====
  Source: DXSDK\Samples\Multimedia\DDraw\OverlayAnimate

  Executable: DXSDK\Samples\Multimedia\DDraw\Bin

User's Guide
============
  OverlayAnimate requires no user input. Press the ESC key to quit the program.

Programming Notes
=================
  For details on how to setup a windowed mode DirectDraw app, see the WindowedMode
  sample. 
  
  To use overlays in general do the following steps in addition to those needed to 
  author a windowed mode DirectDraw app:
    1. Check to see if hardware supports overlays - check IDirectDraw::GetCaps 
       for DDCAPS_OVERLAY.
    2. Size the window to meet the hardware overlay size restrictions.
    3. Create an overlay surface (create it with 1 backbuffer if needed), and 
       set its pixel format to a desired format that is supported by the device.  
    4. Set the dest color key on the overlay to the background color of the window. 
       Be sure to choose a color for the background of the window that Windows typically
       does not use otherwise, the overlay will be drawn on top of overlapping windows.
    5. Call UpdateOverlay to display or hide the overlay on the desktop.  WM_PAINT is 
       a good place for this.
    6. When WM_SIZE or WM_MOVE is sent, then update the src and dest rects and 
       check to make sure that they are within the hardware limits.
       
  To animate the overlay, instead of drawing to the off-screen plain back buffer as 
  in windowed mode case, just draw to the the overlay's backbuffer then flip it. 

  
  
