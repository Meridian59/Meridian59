//-----------------------------------------------------------------------------
// 
// Sample Name: Play3DSound Sample
// 
// Copyright (c) 1999 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  The Play3DSound sample shows how to create a 3-D sound buffer and 
  manipulate its properties. It is similar to the 3DAudio sample but does not 
  use an audiopath.

Path
====
  Source: DXSDK\Samples\Multimedia\DSound\Play3DSound

  Executable: DXSDK\Samples\Multimedia\DSound\Bin

User's Guide
============
  Click Sound... and load a wave file. Play the Buffer. The position of the 
  sound source is shown as a triangle on the graph, where the x-axis is from 
  left to right and the z-axis is from bottom to top. Change the range of 
  movement on the two axes by holding down the mouse button and moving the mouse. 

  The listener is located at the center of the graph, and has its default 
  orientation, looking along the positive z-axis; that is, toward the top of 
  the screen. The sound source can move to the listener's left and right and to 
  the listener's front and rear, but does not move above and below the listener.

Programming Notes
=================
  
  * To create a DirectSound3DBuffer object
        1. Fill out a DSBUFFERDESC struct with 
           DSBCAPS_CTRL3D and the 3D virtualization guid 
        2. Call DirectSound.CreateSoundBufferFromFile passing in the DSBUFFERDESC
           This will create a secondary buffer with 3D control.
        3. Call DirectSoundSecondaryBuffer.Get3DBuffer to query for the 
           DirectSound3DBuffer
  
  * Set the postion of the 3D buffer
        1. Call DirectSound3DBuffer.SetPosition
  
  
             
           
