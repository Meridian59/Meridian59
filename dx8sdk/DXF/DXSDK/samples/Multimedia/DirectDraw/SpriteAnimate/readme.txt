//-----------------------------------------------------------------------------
// Sample Name: SpriteAnimate Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  SpriteAnimate demonstrates a simple technique to animate DirectDraw surfaces. 

Path
====
  Source: DXSDK\Samples\Multimedia\DDraw\SpriteAnimate

  Executable: DXSDK\Samples\Multimedia\DDraw\Bin

User's Guide
============
  SpriteAnimate requires no user input. Press the ESC key to quit the program.

Programming Notes
=================
  For details on how to setup a full-screen DirectDraw app, see the FullScreenMode 
  sample. 
  
  One simple method to animate sprites in DirectDraw is author a single bitmap
  file to contain many frames of animation.  The program then stores the current frame 
  indicator in each sprite's state.  From this current frame indicator, it can 
  progmatically derive a src rect that encompasses only a single frame 
  of animation in the off-screen plain surface.  The rect then is blited from the
  off-screen plain surface to the back buffer.  
  
  InitDirectDraw() in the sample shows how to build an cached array of these source rects.
  DisplayFrame() then access this array based on each sprite's current frame.
  
  
 
  
