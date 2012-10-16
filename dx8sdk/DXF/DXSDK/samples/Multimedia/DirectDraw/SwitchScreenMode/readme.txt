//-----------------------------------------------------------------------------
// 
// Sample Name: SwitchScreenMode Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  SwitchScreenMode demonstrates how to author a DirectDraw application to handle
  switching between both full-screen and windowed DirectDraw modes.

Path
====
  Source: DXSDK\Samples\Multimedia\DDraw\SwitchScreenMode

  Executable: DXSDK\Samples\Multimedia\DDraw\Bin

User's Guide
============
  Press Alt-Enter to switch from windowed mode to full-screen and back again.  
  Press the ESC key to quit the program.

Programming Notes
=================
  For details on how to setup a full-screen DirectDraw app, see the FullScreenMode 
  sample. 
  
  For details on how to setup a windowed mode DirectDraw app, see the WindowedMode
  sample. 
  
  When combining these the two mode, be sure to re-create all DirectDraw surfaces
  since the pixel format for full-screen mode typically does not match the 
  desktop pixel format.  
  
