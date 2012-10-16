//-----------------------------------------------------------------------------
// Name: Cull Direct3D Sample
// 
// Copyright (c) 2000-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The Cull sample illustrates how to cull objects whose object bounding box 
   (OBB) does not intersect the view frustum.  By not passing these objects
   to D3D, you save the time that would be spent by D3D transforming and 
   lighting these objects which will never be visible.  The time savings could 
   be significant if there are many such objects, and/or if the objects contain 
   many vertices.

   More elaborate and efficient culling can be done by creating hierarchies
   of objects, with bounding boxes around groups of objects, so that not every 
   object's OBB has to be compared to the view frustum.
   
   It is more efficient to do this OBB/frustum intersection calculation in your 
   own code than to use D3D to transform the OBB and check the resulting clip 
   flags.

   You can adapt the culling routines in this sample meet the needs of programs 
   that you write.


Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\Cull
   Executable: DXSDK\Samples\Multimedia\D3D\Bin


User's Guide
============
   When you run this program, you'll see the same scene (a bunch of teapots)
   rendered into two viewports.  The right viewport uses the view frustum that
   the code will cull against.  The left viewport has an independent camera,
   and shows the right viewport's frustum as a visible object, so you can see
   where culling should be happening.  50 teapots are randomly placed in the
   scene, and they are rendered along with their semitransparent OBB's.

   The teapots are colored as follows to indicate their cull status:

   Dark Green:    The object was quickly determined to be inside the frustum
                  (CS_INSIDE)
   Light Green:   The object was determined (after a fair bit of work) to be 
                     inside the frustum (CS_INSIDE_SLOW)
   Dark Red:      The object was quickly determined to be outside the frustum
                  (CS_OUTSIDE)
   Light Red:     The object was determined (after a fair bit of work) to be
                     outside the frustum (CS_OUTSIDE_SLOW)
   
   You should only ever see green teapots in the right window.  Note that
   most teapots are either dark green or dark red, indicating that the slower
   tests are not needed for the majority of cases.

   To move the camera of the right viewport, click on the right side of the 
   window, then use the camera keys listed below to move around.
   
   To move the camera of the left viewport, click on the left side of the 
   window, then use the camera keys listed below to move around.

   You can also rotate the teapots to set up particular relationships against
   the view frustum.  You cannot move the teapots, but you can get the same 
   effect by moving the frustum instead.

   The following keys are implemented. The dropdown menus can be used for the
   same controls.
      <F1>               Shows help or available commands.
      <F2>               Prompts user to select a new rendering device or 
	                        display mode
      <Alt+Enter>        Toggles between fullscreen and windowed modes
      <Esc>              Exits the app.
	  <W, S, Arrow Keys> Move the camera
	  <Q, E, A, Z>       Rotate the camera
	  <Y, U, H, J>       Rotate the teapots
	  <N>                Snap the left camera to match the right camera
	  <M>                Snap the right camera to the original position

Programming Notes
=================
   The OBB/viewport intersection algorithm used by this program is:
   1) If any OBB corner pt is inside the frustum, return CS_INSIDE
   2) Else if all OBB corner pts are outside a single frustum plane, 
      return CS_OUTSIDE
   3) Else if any frustum edge penetrates a face of the OBB, return 
      CS_INSIDE_SLOW
   4) Else if any OBB edge penetrates a face of the frustum, return
      CS_INSIDE_SLOW
   5) Else if any point in the frustum is outside any plane of the 
      OBB, return CS_OUTSIDE_SLOW
   6) Else return CS_INSIDE_SLOW

   The distinction between INSIDE and INSIDE_SLOW, and between OUTSIDE and 
   OUTSIDE_SLOW, is only provided here for educational purposes.  In a
   shipping app, you probably would combine the cullstates into just 
   INSIDE and OUTSIDE, since all you usually need to know is whether the OBB 
   is inside or outside the frustum.

   The culling code shown here is written in a straightforward way for 
   readability.  It is not optimized for performance.  Additional optimizations
   can be made, especially if the bounding box is a regular box (e.g., the front
   and back faces are parallel).  Or this algorithm could be generalized to work
   for arbitrary convex bounding hulls to allow tighter fitting against the 
   underlying models.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common

