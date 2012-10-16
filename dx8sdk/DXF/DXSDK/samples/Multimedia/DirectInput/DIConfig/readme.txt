//-----------------------------------------------------------------------------
// 
// Sample Name: DIConfig 
// 
// Copyright (c) 2000 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  The DIConfig code demonstrates the implementation of a configuration 
  user interface based upon the DirectInput Mapper technology. This 
  sample code is *very* complex, and is intended to be taken as a
  reference implementation more than a learning tool. 

Path
====
  Source: DXSDK\Samples\Multimedia\DInput\DIConfig

  Executable: DXSDK\Samples\Multimedia\DInput\Bin

User's Guide
============
  Not applicable. This is not a sample usable by end-users. 
  

Programming Notes
=================
  This code generates a binary called diconfig.dll, which contains all 
  of the functionality used in the default Mapper UI. This code is very 
  complex and is intended as a reference implementation of a 
  DirectInput-based configuration user interface. 

  Please review the comments within the code itself for documentation
  on this sample. The following major features are supported in this 
  sample:
    Display of device images.  
    Display and reconfiguration of devices.
    Support for multiple device views, to illustrate alternate viewing 
     angles.
    Support for control activation overlays.
    Use of the GetImageInfo method.
    Device "ownership" for multi-user applications (same machine).
    Persistence of user settings by way of SetActionMap.