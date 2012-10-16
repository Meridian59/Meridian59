//-----------------------------------------------------------------------------
// Name: MFCFog Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The MFCFog sample illustrates how to use Direct3D with MFC, using a
   CFormView. Various controls are used to control fog parameters for the 3D
   scene. 


Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\MFCFog
   Executable: DXSDK\Samples\Multimedia\D3D\Bin


User's Guide
============
   All user interaction for this sample is implemented through the visible MFC
   controls (sliders, radio buttons, etc.). The user is encouraged to play with
   controls and observe the various effects they have on the rendered 3D scene.

  
Programming Notes
=================
   All the MFC code is contained with the CFormView class' derived member
   functions. The code for the MFC and D3D initialization code can be found in
   the d3dapp.cpp source file. This file can be easily ported to work with
   another app, by simply stripping out the fog-related code.

   The Direct3D fog code is all contained in fog.cpp. Here you'll find functions
   to initialize, animate, render, and cleanup the scene.

   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common
