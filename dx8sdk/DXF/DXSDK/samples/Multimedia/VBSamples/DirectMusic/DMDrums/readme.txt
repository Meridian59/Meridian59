//-----------------------------------------------------------------------------
// 
// Sample Name: DMDrums Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
The DMDrums sample illustrates playing Motif's from a 
DirectMusic segment, as well as playing grooves from that 
same segment using different instrument sets.

  
Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectMusic\DMDrums

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectMusic\bin


User's Guide
============
Clicking any of the drum buttons will play that Drum based on the 
current grove and instrument set.  You can also click the Play 
button to play a default sample of that instrument set.  Volume and 
Tempo can also be changed.

Programming Notes
=================
On startup drums!.sgt and drums!.sty are loaded, and all motifs are
loaded from the style.  All available bands are loaded next, and the 
list of 'Grooves' are loaded.  We then play each motif whenever the 
corresponding button is pressed.  If the Play button is stopped we 
play the default segment.

Whenever a new band is selected we unload the current band, and then
load a new band (based on what is selected in the list box), and download
that band.  We also call SetMasterGrooveLevel whenever the groove has changed.

Finally, we can turn on and off environmental reverb by changing the default
audio path for DirectMusic.
