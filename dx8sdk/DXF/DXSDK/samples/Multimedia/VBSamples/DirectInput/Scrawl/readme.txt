//-----------------------------------------------------------------------------
// Name: Scrawlb DirectInput Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The Scrawlb sample illustrates the use of DirectInput to create a simple drawing program.
   It shows the use of callbacks to receive mouse movement events.

   
Path
====
   Source:     DXSDK\Samples\Multimedia\Vbsamples\DirectInput\ScrawlB
   Executable: DXSDK\Samples\Multimedia\vbsamples\DirectInput\Bin


User's Guide
============
   Left Click and drag on the canvas to draw
   Right Click will bring up a pop up menu with choices
   About              Application information
   Speed 1            Slow mouse movement
   Speed 2            Fast mouse movement
   Speed 3            Faster mouse movement
   Clear              Clear the canvas
   Release Mouse      Change from pencil to cursor pointer


Programming Notes
=================
  The application subclasses the Display window to capture ENTERMENU messages so that
  the cursor can be reset when selecting the menu. This is done through a call to
  SetWindowLong. Note that failure to comment out these lines while running the sample from
  within the Visual Basic environment will result in undefined behavior.

   
