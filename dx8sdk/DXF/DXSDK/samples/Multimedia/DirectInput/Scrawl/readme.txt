//-----------------------------------------------------------------------------
// 
// Sample Name: Scrawl Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  The Scrawl application demonstrates use of the mouse in exclusive mode in a 
  windowed application.

Path
====
  Source: DXSDK\Samples\Multimedia\DInput\Scrawl

  Executable: DXSDK\Samples\Multimedia\DInput\Bin

User's Guide
============
  The main mouse button is always the left button, and the secondary button is 
  always the right button, regardless of any settings the user may have made in 
  Control Panel.

  To scrawl, hold down the left button and move the mouse. Click the right 
  mouse button to invoke a pop-up menu. From the pop-up menu you can clear the 
  client window, set the mouse sensitivity, or close the application.

Programming Notes
=================
  The Scrawl application demonstrates many aspects of DirectInput programming, 
  including the following:
  	Using the mouse in exclusive mode in a windowed application.

  	Releasing the mouse when Windows needs to use it for menu access.

  	Reacquiring the mouse when Windows no longer needs it.

  	Reading buffered device data.

  	Deferring screen updates till movement on both axes has been fully 
  processed.

  	Event notifications of device activity.

  	Restricting the cursor to an arbitrary region.

  	Scaling raw mouse coordinates before using them.

  	Using relative axis mode.
  

   

  

