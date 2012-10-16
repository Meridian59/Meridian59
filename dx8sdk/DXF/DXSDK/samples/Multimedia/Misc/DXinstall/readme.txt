//-----------------------------------------------------------------------------
// 
// Sample Name: DXInstall Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
  DXInstall is an example of how to use DirectXSetup interfaces to install the 
  DirectX subsystem and DirectX drivers. It shows how to use a callback 
  function to present messages and get user input through a custom interface, 
  in this case a simple modeless dialog box.

Path
====
  Source: DXSDK\Samples\Multimedia\Misc\DXInstall

User's Guide
============
  First copy the entire contents of the Redist\DirectX8 folder from the DirectX 
  SDK CD into the same folder as DXInstall.exe. In your development environment, 
  set the working directory to this folder as well. (In Microsoft(r) Visual C++(r), 
  this setting is on the Debug page of the Project Settings dialog box.)

  Run the program and select Start Install from the File menu. DirectSetup 
  performs a simulated installation of DirectX (see Programming Notes) and 
  advises you of its progress in a modeless dialog box. The Options menu allows 
  you to change the level of messages shown. However, if you are performing 
  only a simulated installation, you will never see problem or update messages.

  Choose Get Version from the File menu. The program shows the version and 
  revision number of DirectX currently installed on the system.

Programming Notes
=================

  By default, the program passes DSETUP_TESTINSTALL to the DirectXSetup 
  function. This means that no files are actually copied, nor is the registry 
  modified. To perform a real installation, delete this flag from the call.

  DXInstall employs a callback function to monitor the progress of installation 
  and intercept messages. Depending on the user's preferred warning level, as 
  tracked in g_fStatus, messages may be ignored or presented to the user in a 
  modeless dialog box. If user input is required, the appropriate buttons are 
  displayed and the GetReply function monitors the message queue until one of 
  the buttons is pressed.



  

