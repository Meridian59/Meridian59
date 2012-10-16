//-----------------------------------------------------------------------------
// 
// Sample Name: PlayAudio Sample
// 
// Copyright (c) 1999-2001 Microsoft Corporation. All rights reserved.
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
  Source: DXSDK\Samples\Multimedia\DirectMusic\PlayAudio

  Executable: DXSDK\Samples\Multimedia\DirectMusic\Bin

User's Guide
============
  Play the default segment, or load another wave, MIDI, or DirectMusic 
  Producer segment file by clicking Segment File. Adjust the tempo and 
  volume by using the sliders
  
Programming Notes
=================
  This and other DirectMusic samples uses the DirectMusic sample 
  framework, CMusicManager and CMusicSegment to help encapsulate some of 
  the common functionality of DirectMusic.  The framework is contained 
  in dmutil.cpp.
  
  This is how the sample works:
  
  * Upon WM_INITDIALOG.  See OnInitDialog()
        1. Create a Win32 event, g_hDMusicMessageEvent.  This will be 
           used by DirectMusic to signal the app whenever a DirectMusic 
           notification comes in.
        2. Create a help class CMusicManager called g_pMusicManager.  
        3. Initialize the CMusicManager class.  This does the following. 
           See CMusicManager::Initialize() in dmutil.cpp
                - Creates a IDirectMusicLoader8 using CoCreateInstance
                - Creates a IDirectMusicPerformance8 using CoCreateInstance
                - Calls IDirectMusicPerformance8::InitAudio to init DirectMusic 
                  using a standard audio path.
        4. Call IDirectMusicPerformance8::AddNotificationType() passing in 
           GUID_NOTIFICATION_SEGMENT. This will make DirectMusic tell us about any 
           segment notifications that come in.  This is needed to by this
           sample to know when the segment has ended.  However DirectMusic
           games may not care when the segment has ended.           
        5. Call IDirectMusicPerformance8::SetNotificationHandle() passing
           in the Win32 event, g_hDMusicMessageEvent.  This tells DirectMusic
           to signal this event when a notification is available.

  * Setting up the app message loop.  See WinMain()
        1. Create the dialog using CreateDialog().  
        2. In a loop call MsgWaitForMultipleObjects() passing in 
           g_hDMusicMessageEvent.  This will tell us when g_hDMusicMessageEvent
           is signaled.  Above we have told DirectMusic to signal this event 
           whenever a DirectMusic notification has come in.
        3. If WAIT_OBJECT_0 is returned, then call ProcessDirectMusicMessages(),
           See below for details.
        4. If WAIT_OBJECT_0 + 1 is returned, then Windows msgs are available, so
           do standard msg processing using PeekMessage().
           
  * When "Open File" is clicked.  See OnOpenSoundFile()
        1. Get the file name from using GetOpenFileName().
        2. Release the any old g_pMusicSegment.
        3. Call CMusicManager::CollectGarbage().  See dmutil.cpp.
           This calls IDirectMusicLoader8::CollectGarbage which 
           collects any garbage from any old segment that was present.  
           This is done because some sophisticated segments, in particular 
           ones that include segment trigger tracks or script tracks, may 
           have a cyclic reference. For example, a segment trigger that 
           references another segment that references the first segment, also 
           via a segment trigger track.
        4. Call CMusicManager::SetSearchDirectory(). See dmutil.cpp
           This calls IDirectMusicLoader8::SetSearchDirectory() 
           passing in the GUID_DirectMusicAllTypes and a directory.
           This will tell DirectMusic where to look for files that 
           are referenced inside of segments.
        5. Call CMusicManager::CreateSegmentFromFile() to create a
           CMusicSegment called g_pMusicSegment from the file.  
           See dmutil.cpp.  This does the following:
                - Calls IDirectMusicLoader8::LoadObjectFromFile() to 
                  load the IDirectMusicSegment8 into pSegment.
                - Creates CMusicSegment passing in pSegment.
                - If the file is a pure MIDI file then it calls 
                  IDirectMusicSegment8::SetParam passing in 
                  GUID_StandardMIDIFile to DirectMusic this.  This makes 
                  sure that patch changes are handled correctly.                  
                - If requested, it calls IDirectMusicSegment8::Download()
                  this will download the segment's bands to the synthesizer.  
                  Some apps may want to wait before calling this to because 
                  the download allocates memory for the instruments. The 
                  more instruments currently downloaded, the more memory 
                  is in use by the synthesizer.
                  
   * When "Play" is clicked.  See OnPlayAudio()
        1. If the UI says the sound should be looped, then call 
           CMusicSegment::SetRepeats passing in DMUS_SEG_REPEAT_INFINITE,
           otherwise call CMusicSegment::SetRepeats passing in 0.
        2. Call CMusicSegment::Play() which calls 
           IDirectMusicPerformance8::PlaySegmentEx().  See dmutil.cpp.
        
   * Upon a DirectMusic notification. See ProcessDirectMusicMessages().
        This sample wants to know if the primary segment has stopped playing
        so it can updated the UI so tell the user that they can play 
        the sound again. This is rather complex, but typically apps 
        will not need this functionality.  Here is what has to be done:
        
        1. Call IDirectMusicPerformance8::GetNotificationPMsg() in a loop
           to process each PMsg that has occurred.
        2. Switch off the pPMsg->dwNotificationOption.  This sample 
           only handles it if its a DMUS_NOTIFICATION_SEGEND.  This tells 
           us that segment has ended.
        3. Call QueryInterface on the pPMsg->punkUser, quering for a 
           IDirectMusicSegmentState8.
        4. Using the IDirectMusicSegmentState8, call GetSegment to 
           get a IDirectMusicSegment* of the segment it refers to.  
           This call may fail is the segment may have gone away before this
           notification was handled.
        5. Call QueryInterface IDirectMusicSegment to get a IDirectMusicSegment8
        6. Compare this pointer to the IDirectMusicSegment8 pointer 
           in g_pMusicSegment, to see if this was the primary segment.
           This may not always be the case since segments can have segments enbedded
           inside of them, we only want handle when the primary segment has
           stopped playing.  If it has, then update the UI
        7. Cleanup all the interfaces.
        
