//-----------------------------------------------------------------------------
// 
// Sample Name: 3DAudio Sample
// 
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//
// GM/GS® Sound Set Copyright ©1996, Roland Corporation U.S.
// 
//-----------------------------------------------------------------------------



Description
===========
  The 3DAudio sample application shows how to create a 3-D audiopath in 
  a DirectMusic performance, how to obtain an interface to a 3-D buffer 
  and listener in that path, and how to modify the parameters of the buffer 
  and listener.

Path
====
  Source: DXSDK\Samples\Multimedia\DirectMusic\3DAudio

  Executable: DXSDK\Samples\Multimedia\DirectMusic\Bin

User's Guide
============
  Click Segment File and load a wave, MIDI, or DirectMusic Producer segment 
  file. Play the segment. The position of the sound source is shown as a red 
  dot on the graph, where the x-axis is from left to right and the z-axis is 
  from bottom to top. Change the range of movement on the two axes by using the 
  sliders. 

  The listener is located at the center of the graph, and has its default 
  orientation, looking along the positive z-axis; that is, toward the top of 
  the screen. The sound source moves to the listener's left and right and to 
  the listener's front and rear, but does not move above and below the listener.

  The sliders in the center of the window control the properties of the listener; 
  that is, the global sound properties. If you click Defer Settings, changes are 
  not applied until you click Apply Settings. 

Programming Notes
=================
  The 3DAudio sample is very similar in form to the PlayAudio sample.  For 
  detailed programming notes on the basics this sample, refer to Programming 
  Notes section of the PlayAudio sample.
  

   

  

