//-----------------------------------------------------------------------------
// 
// Sample Name: AudioScripts Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
//
// GM/GS® Sound Set Copyright ©1996, Roland Corporation U.S.
// 
//-----------------------------------------------------------------------------



Description
===========
  The AudioScripts sample demonstrates how an application and a DirectMusic 
  script work together. The script reads and writes to variables in the 
  application, and the application calls routines in the script that 
  play segments.

  The sample also demonstrates how waves can be played as variations 
  in a segment.

Path
====
  Source: DXSDK\Samples\Multimedia\DirectMusic\AudioScripts

  Executable: DXSDK\Samples\Multimedia\DirectMusic\Bin

User's Guide
============
  Select ScriptDemoBasic.spt from the Script File list box. Play a segment 
  by clicking Routine 1. Click Routine 2 to play an ending and stop playback. 
  Play the segment again and click Routine 3 several times. Note how Variable 
  1 reflects the number of times the button has been clicked, and how the 
  music changes in response to each click.

  Select ScriptDemoBaseball.spt from the list box. Click Routine 1 to play 
  various calls from a vendor. Click Routine 2 to play various musical motifs. 
  Change the score by entering different values in the Variable1 and Variable2 
  text boxes. Click Routine 3 to hear the score. If the score for either team 
  exceeds 10, the crowd roars and then falls silent.
