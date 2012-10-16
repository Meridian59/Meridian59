//-----------------------------------------------------------------------------
// 
// Sample Name: BarGraph Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
The BarGraph sample describes how one might use Direct3D for graphic visualization.
It makes heavy use of the RenderToSurface features of D3DX to render text and bitmaps
dynamically.

  
Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\Direct3D\BarGraph

  Executable: DXSDK\Samples\Multimedia\VBSamples\Direct3D\Bin


User's Guide
============
  right click to bring up a pop up menu.
  from this menu you can load new data from a .csv (comma delimeted file)
  such a file can be exported from excel or any spreadsheet package.

  The file must be formated such that the first row and columns are headers
  that title the data. They can contain the tag TEXTURE:filename.bmp to indicate
  that the header contains a picture. the rest of the data must be numeric
  see bargraphdata.csv in Mssd\Samples\Multimedia\VBSamples\Media for an example

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

