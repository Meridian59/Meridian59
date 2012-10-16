//-----------------------------------------------------------------------------
// 
// Sample Name: Air Hockey Demo
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
This application demonstrates a simple peer-peer (or single player) Air Hockey 
Game.

In Single player mode, a crude computer AI will be your opponent.  You can 
dictate numerous settings in the game, including overall 'speed' of the game as
well as score to win (although the computer always enforces the rule that you 
must win by 2).

The multiplayer mode is similar to the single player mode, with the exception of
no computer AI.  

Please note, if you try to run this sample from the source tree it will most likely 
fail due to lack of media.  The 'models' and 'sounds' folders must be in the same folder 
as where you run the sample from (by default these folders are in the 'bin' folder).

Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\Demos\AirHockey

  Executable: DXSDK\Samples\Multimedia\VBSamples\Demos\bin


User's Guide
============
Press Space to launch the puck.  Use the mouse (or arrow keys on your keyboard, or joystick)
to control your paddle.

Programming Notes
=================
This sample shows many of the directX components working together.
