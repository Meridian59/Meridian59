//-----------------------------------------------------------------------------
// Name: ActionMap DirectInput Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The ActionMap sample illustrates the use of DirectInput's Action Mapping feature.
   This feature allows you to assign various keys to constants that are kept in
   a queue that the application can read at any time. This sample also shows how
   those mappings can be user configurable.

   
Path
====
   Source:     DXSDK\Samples\Multimedia\Vbsamples\DirectInput\ActionMap
   Executable: DXSDK\Samples\Multimedia\vbsamples\DirectInput\Bin


User's Guide
============
   None
	

Programming Notes
=================
   ActionMap.cls should not be used unmodified. For the purpose of this sample it will
   query for any and all input devices and does not differentiate where the input is coming
   from. Most applications will want to modify the class to respond to only one given input
   or differentiate the input devices into different players or purposes.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   classes and modules can be found in the following directory:
      DXSDK\Samples\Multimedia\VBSamples\Common

