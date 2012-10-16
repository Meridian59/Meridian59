//-----------------------------------------------------------------------------
// File: Readme.txt
//
// Desc: Readme for GetDXVersion() sample
//
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

The purpose of this sample is to show the return results from a call to the
function GetDXVersion().  When you run GETDXVER.EXE, it will display a MessageBox
with the installed DirectX version.

You can determine which version of DirectX is installed on a system by
thoroughly querying for various DirectX object interfaces. The GetDXVersion
sample function shows one way this might be done. However, real-world
applications should not rely on this function, and should always query the
DirectX objects for all necessary functionality during startup.

