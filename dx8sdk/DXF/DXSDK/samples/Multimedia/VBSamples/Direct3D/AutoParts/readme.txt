//-----------------------------------------------------------------------------
// 
// Sample Name: AutoParts Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
The AutoParts sample illustrates the use of Picking against a 3D database.

  
Path
====
  Source: DXSDF\Samples\Multimedia\VBSamples\Direct3D\AutoParts

  Executable: DXSDF\Samples\Multimedia\VBSamples\Direct3D\Bin


User's Guide
============
  click where it says "click here" in the Tree view to the left to load the engine model.
  The model can be rotated by holding the left mouse button down and dragging.
  When a part has been clicked on, it is described in the lower left hand corner.
  The Add and Remove from Invoice button manage the Invoice list but the Order button has 
  no function.
  	


Programming Notes
=================   
   Each object in the Engine model is named. These names are unique and can be used to cross
   reference a database (a custom text database in this example for the sake of not requiring
   the installation of MDAC). From that database more detailed information is gahtered on the 
   part such as price and part number.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   classes and modules can be found in the following directory:
      DXSDF\Samples\Multimedia\VBSamples\Common

