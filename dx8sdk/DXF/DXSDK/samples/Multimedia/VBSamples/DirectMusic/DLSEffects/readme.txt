//-----------------------------------------------------------------------------
// 
// Sample Name: DLS Effects Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
// 
//-----------------------------------------------------------------------------


Description
===========
This application demonstrates the use of Downloadable Sounds for sound
effects, and how to send MIDI notes.

The DLS instruments are taken from Boids.dls. That collection actually
contains only a single instrument, called Vocals. However, the instrument
is based on different wave samples for different "regions" or ranges
of notes. For example, the first speech sound is used when any note
between C3 and B3 is sent. The speech sounds are played at the correct
pitch only when the note is the lowest one in the region.

One of the samples, called Heartbeat, is valid for the range B7-B8.
Because this is not a vocal sample, we can reasonably vary the pitch
by playing various notes within that range, as determined by the
slider setting.

Heartbeat is also the only sample in the DLS collection that is based
on a looped wave. Hence it can be played continuously for up to the
maximum duration of a note. The other samples will play only once
regardless of the duration of the note sent.
  
Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectMusic\DLSEffects

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectMusic\bin


User's Guide
============
Clicking any of the 'Notes' will play the associated vocal.  You may also turn 
Heartbeat on or off, and change the pitch, or velocity of notes.

Programming Notes
=================
   This sample shows how to control notes in a DLS using SendNotePMsg, and SendMidiPmsg.
   Whenever a note is clicked, we will call SendNotePMsg to turn the note on, for a specific
   duration (which varies for each note).  When we want to turn the Heartbeat on we will call
   SendMidiPMsg, and then call SendNotePMsg once more to turn the Heartbeat off.
