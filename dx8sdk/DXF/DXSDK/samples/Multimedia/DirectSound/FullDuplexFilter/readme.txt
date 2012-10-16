//-----------------------------------------------------------------------------
// 
// Sample Name: FullDuplexFilter Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  The FullDuplexFilter application shows how to capture and play back sounds 
  at the same time.

Path
====
  Source: \DXSDK\Samples\Multimedia\DSound\FullDuplexFilter

  Executable: \DXSDK\Samples\Multimedia\DSound\Bin

User's Guide
============
  Be sure your sound card is configured for full-duplex operation and that 
  you have a microphone attached. Choose a format from the Output Format 
  list. A list of formats appears in the Input Format list. Select one. 
  Click OK. If full duplex is successfully created, another dialog box is 
  displayed with a Record button. Click this button. Speak into the 
  microphone and your voice is heard over the speakers.

Programming Notes
=================
  FullDuplexFilter is designed to be a simple example of how one might go about 
  implementing full-duplex audio. It is designed primarily to show application 
  developers the proper sequence of steps for dealing with the sound devices 
  installed on the system for using full-duplex audio with waveIn as an input 
  source and DirectSound as the output device.

