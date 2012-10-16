//-----------------------------------------------------------------------------
// 
// Sample Name: ScatterGraph Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
The ScatterGraph sample describes how one might use Direct3D for graphic visualization.
It makes heavy use of the RenderToSurface features of D3DX to render text and bitmaps
dynamically.

  
Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\Direct3D\ScatterGraph

  Executable: DXSDK\Samples\Multimedia\VBSamples\Direct3D\Bin


User's Guide
============
  right click to bring up a pop up menu for the following options

    Load Data From File           from this menu you can load new data from a .csv 
                                  (comma delimeted file) such a file can be exported
                                  from excel or any spreadsheet package.
  
    Reset Orientation             reset the viewpoint to a know state

    Show Connecting Lines         if the order of the data is important 
                                  this connects the data points
  
    Show Height Lines             makes it easier to see the Y value in comparison 
                                  to other values

    Show Foot lines               makes it easier to see the X Z relation ship

    Show Base plane		  plane where y=0

    Auto rotate			  turn on and off rotation
  
    

  Any csv file to be loaded must be formated such that the first row is a header. 
  The formating is as follows with [] indicating optional components:
  Name, X Axis Name, Y Axis Name, Z Axis Name, [Size Name], [Color Name]
  Entries for Axis and Size must be numeric. those for color must fit the format
  &HFF102030, where 10 20 30 is the red,green and blue component.

  see sampledata.csv in Mssd\Samples\Multimedia\VBSamples\Media for an example

  Holding the left mouse button and dragging will rotate the graph.

  Right Arrow  moves the camera right
  Left Arrow   moves the camera left
  Up Arrow     moves the camera up
  Down Arrow   moves the camera down
  W            moves the camera forward
  S            moves the camera backward
  E            rotates the camera right
  Q            rotates the camera left
  A            rotates the camera up
  Z            rotates the camera down

    
    	


Programming Notes
=================   
   

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   classes and modules can be found in the following directory:
      DXSDK\Samples\Multimedia\VBSamples\Common

