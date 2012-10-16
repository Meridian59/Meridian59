//-----------------------------------------------------------------------------
// 
// Sample Name: DMBoids Sample
// 
// Copyright © 1999-2001 Microsoft Corp. All Rights Reserved.
//
// GM/GS® Sound Set Copyright ©1996, Roland Corporation U.S.
// 
//-----------------------------------------------------------------------------



Description
===========
  DMBoids is a version of Boids that adds DirectMusic support. As objects fly 
  over a simple landscape, the music responds to user input and events on the 
  screen.

Path
====
  Source: DXSDK\Samples\Multimedia\Dmusic\DMBoids

  Executable: DXSDK\Samples\Multimedia\DMusic\Bin

User's Guide
============
  Press F10 to access the main menu. The Drivers menu allows you to change the 
  driver, device, and video mode. The application runs only in full-screen 
  modes.

  The A (alignment), C (cohesion) and O (obstacle) keys alter behavior of the 
  boids in various ways as long as they are held down. 

  Hold down the S key or the spacebar and the birds flock in closer. Release 
  the key and they spread apart. Note the use of motifs to track this
  behavior.

  Hold down the M key and the birds wander off their path. Notice that the 
  music completely changes. Release and the birds will eventually get back on 
  the path.

  Press the ESC key to quit.

Programming Notes
=================
  DirectMusic features illustrated include the following:
       Software synthesis with DLS. In addition to the musical instruments
  from  the GS sound set, the application uses custom downloadable sounds
  such as the voices that appear to come from the planets.

  	Composing and performing style-based segments.

  	Musical transitions using style-based motifs and segment cues.

  	Echo/articulation tool coded that uses the proximity of the birds to
  adjust the echoes and note durations of the music as it plays.
  

