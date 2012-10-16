//-----------------------------------------------------------------------------
// Name: Moire Direct3D Sample
// 
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The Moire sample shows how to use the DXSDK screensaver framework to write
   a screensaver that uses Direct3D.  The screensaver framework is very similar
   to the sample application framework, using many methods and variables with 
   the same names.  After writing a program with the screensaver framework, one
   ends up with a fully-functional Windows screensaver rather than a regular 
   Windows application.

   The Moire screensaver appears as a mesmerizing sequence of spinning lines
   and colors.  It uses texture transformation and alpha blending to create a 
   highly animated scene, even though the polygons that make up the scene do 
   not move at all.
   

Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\Screensavers\Moire
   Executable: DXSDK\Samples\Multimedia\D3D\Bin


User's Guide
============
   Moire.scr can be started in five modes: configuration, preview, full, test, 
   and password-change.  You can choose some modes by right-clicking the 
   moire.scr file and choosing Configure or Preview.  Or you can start moire.scr 
   from the command line with the following command-line parameters:
      -c          Configuration mode
	  -t          Test mode
	  -p          Preview mode
	  -a          Password-change mode
	  -s          Full mode

   When the screensaver is running in full mode, press any key or move the 
   mouse to exit.
	

Programming Notes
=================
   Programs that use the screensaver framework are very similar to programs
   that use the D3D sample application framework.  Each screensaver needs to
   create a class derived from the main application class, CD3DScreensaver.
   The screensaver implements its own versions of the virtual functions 
   FrameMove(), Render(), InitDeviceObjects(), etc., that provide 
   functionality specific to each screensaver.

   Screensavers can be written to be multimonitor-compatible, without much
   extra effort.  If you do not want your screensaver to run on multiple
   monitors, you can just set the m_bOneScreenOnly variable to TRUE.  But
   by default, this value is false, and your program needs to be able to
   handle multiple D3D devices.  The function SetDevice() will be called each
   time the device changes.  The way that Moire deals with this is to create
   a structure called DeviceObjects which contains all device-specific 
   pointers and values.  CMoireScreensaver holds an array of DeviceObjects
   structures, called m_DeviceObjectsArray.  When SetDevice() is called, 
   m_pDeviceObjects is changed to point to the DeviceObjects structure for
   the specified device.  When rendering, m_rcRenderTotal refers to the 
   rendering area that spans all monitors, and m_rcRenderCurDevice refers 
   to the rendering area for the current device's monitor.  The function
   SetProjectionMatrix shows one way to set up a projection matrix that
   makes proper use of these variables to render a scene that optionally
   spans all the monitors, or renders a copy of the scene on each monitor,
   depending on the value of m_bAllScreensSame (which can be controlled by
   the user in the configuration dialog, if you want).

   The ReadSettings() function is called by the screensaver framework at
   program startup time, to read various screensaver settings from the 
   registry.  DoConfig() is called when the user wants to configure the
   screensaver settings.  The program should respond to this by creating
   a dialog box with controls for the various screensaver settings.  This
   dialog box should also have a button called "Display Settings" which, 
   when pressed, should call DoScreenSettingsDialog().  This common dialog
   allows the user to configure what renderer and display mode should be 
   used on each monitor.  You should set the member variable m_strRegPath 
   to a registry path that will hold the screensaver's settings.  You can 
   use this variable in your registry read/write functions.  The screensaver
   framework will also use this variable to store information about the
   default display mode in some cases.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      Dxsdk\Samples\Multimedia\Common

