//-----------------------------------------------------------------------------
// 
// Sample Name: FFConst Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  This application applies raw forces to a force feedback input device, 
  illustrating how a simulator-type application can use force feedback to 
  generate forces computed by a physics engine.

  You must have a force feedback device connected to your system in order to 
  run the application.

Path
====
  Source: DXSDK\Samples\Multimedia\DInput\FFConst

  Executable: DXSDK\Samples\Multimedia\DInput\Bin

User's Guide
============
  When you run the application, it displays a window with a crosshair and a 
  black spot in it. Click the mouse anywhere within the window's client area to 
  move the black spot. (Note that moving the device itself does not do 
  anything.) FFConst exerts a constant force on the device from the 
  direction of the spot, in proportion to the distance from the crosshair. You 
  can also hold down the mouse button and move the spot continuously.

Programming Notes
=================
  This sample program enumerates the input devices and acquires the first 
  force-feedback device that it finds. If none are detected, it displays a 
  message and terminates.

  When the user moves the black spot, the joySetForcesXY function converts the 
  cursor coordinates to a force direction and magnitude. This data is used to 
  modify the parameters of the constant force effect.

