//-----------------------------------------------------------------------------
// 
// Sample Name: VB PlayAudio Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
//
// GM/GS® Sound Set Copyright ©1996, Roland Corporation U.S.
// 
//-----------------------------------------------------------------------------


Description
===========
  The PlayAudio sample shows how to load a segment and play it on an 
  audiopath, how to use DirectMusic notifications, and how to change 
  global performance parameters.
  
Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectMusic\PlayAudio

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectMusic\Bin

User's Guide
============
  Play the default segment, or load another wave, MIDI, or DirectMusic 
  Producer segment file by clicking Audio File. Adjust the tempo and 
  volume by using the sliders
  
Programming Notes
=================
  This is how the sample works:
  
  * Upon Form_Load
        1. Create the DirectMusic objects.  
        2. Initialize DirectMusic.  This creates a default standard audio path
        3. Call DirectMusicPerformance8.AddNotificationType passing in 
           DMUS_NOTIFY_ON_SEGMENT. This will make DirectMusic tell us about any 
           segment notifications that come in.  This is needed to by this
           sample to know when the segment has ended.  However DirectMusic
           games may not care when the segment has ended.           
        4. Create a DirectX event, dmEvent.  This will be used by DirectMusic
           to signal the app whenever a DirectMusic notification comes in.
        5. Call DirectMusicPerformance8.SetNotificationHandle passing
           in the DirectX event, dmEvent.  This tells DirectMusic
           to signal this event when a notification is available.

  * When "Open File" is clicked.  See cmdOpen_Click()
        1. Get the file name from the common dialog.
        2. Get rid of any old segment.
        3. Call DirectMusicLoader8.SetSearchDirectory.
           This will tell DirectMusic where to look for files that 
           are referenced inside of segments.
        4. Call DirectMusicLoader8.LoadSegmentFromFile
	5. If the file is a pure MIDI file then it calls 
	   DirectMusicSegment8.SetStandardMidiFile This makes 
           sure that patch changes are handled correctly.
	6. Calls DirectMusicSegment8.Download
           this will download the segment's bands to the synthesizer.  
           Some apps may want to wait before calling this to because 
           the download allocates memory for the instruments. The 
           more instruments currently downloaded, the more memory 
           is in use by the synthesizer.
                  
   * When "Play" is clicked.  See cmdPlay_Click()
        1. If the UI says the sound should be looped, then call 
           DirectMusicSegment8.SetRepeats passing in INFINITE,
           otherwise call DirectMusicSegment8.SetRepeats passing in 0.
        2. Call DirectMusicPerformance8.PlaySegmentEx() 
        
   * Upon a DirectMusic notification. See DirectXEvent8_DXCallback().
        This sample wants to know if the primary segment has stopped playing
        so it can updated the UI so tell the user that they can play 
        the sound again. 
        
        1. Call IDirectMusicPerformance8.GetNotificationPMSG 
        2. Switch off the pPMsg->dwNotificationOption.  This sample 
           only handles it if its a DMUS_NOTIFICATION_SEGEND.  This tells 
           us that segment has ended.
        
