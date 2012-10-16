//-----------------------------------------------------------------------------
// 
// Sample Name: VB PlayMotif Sample
// 
// Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
//
// GM/GS® Sound Set Copyright ©1996, Roland Corporation U.S.
// 
//-----------------------------------------------------------------------------



Description
===========
  The PlayMotif sample demonstrates how a motif played as a secondary 
  segment can be aligned to the rhythm of the primary segment in various ways.

Path
====
  Source: DXSDK\Samples\Multimedia\VBSamples\DirectMusic\PlayMotif

  Executable: DXSDK\Samples\Multimedia\VBSamples\DirectMusic\Bin

User's Guide
============
  Play the default segment, or load another DirectMusic Producer segment 
  that contains motifs. Select one of the patterns in the list box and 
  one of the Align Option buttons, and then click Play Motif. Note how 
  the motif does not begin playing until an appropriate boundary in the 
  primary segment has been reached.

Programming Notes
=================
  The PlayMotif sample is very similar in form to the PlayAudio sample.  For 
  detailed programming notes on the basics this sample, refer to Programming 
  Notes section of the PlayAudio sample.
  
  The PlayMotif differs by letting the user play any of motifs contained 
  inside the segment. Here's how:
  
  * When loading the file it does the same steps as the PlayAudio 
    sample, but also:  
        1. It loops thru each style in the segment, searching it for 
           motifs.  It calls DirectMusicSegment8.GetStyle passing 
           an increasing style index to get each of the styles.  When 
           this returns error then there are no more styles.
        2. For each style, it calls DirectMusicStyle.GetMotifCount.
           It then loops through each Motif, and stores the motif name 
           in the list box.
        3. With the motif name it calls DirectMusicStyle::GetMotif
           to get a DirectMusicSegment pointer to the motif, and
           stores this for later use.
           
   * When "Play Motif" is clicked.  See cmdPlayMotif_Click().
        1. It gets the desired alignment option from the UI.  
        2. It gets the selected motif from our interal list.
        3. It calls DirectMusicPerformance::PlaySegmentEx passing in
           the motif's DirectMusicSegment and flags which have 
           DMUS_SEGF_SECONDARY as well as any alignment option.
       
   * When DirectMusic notifications occur, it is similar to PlayAudio but 
     now the app also takes note of any motif starting or stopping and
     updates the play count.  If the play count is greater than zero then
     it updates the UI to show that the motif is playing.  Most games
     would not need this functionality, but here's how its done: 
     See DirectXEvent8_DXCallback.
        - Call DirectMusicPerformance8::GetNotificationPMsg.
        - Check if the pPMsg->lNotificationOption. 
        - If it is a DMUS_NOTIFICATION_SEGSTART.  This tells 
          us that a segment has ended.  It may be for a motif or the primary
          or some embedded segment in the primary segment.  
        - If it is a DMUS_NOTIFICATION_SEGEND.  This tells 
          us that a segment has ended.  It may be for a motif or the primary
          or some embedded segment in the primary segment.  
        - For either SEGSTART or SEGEND the code is similar:
            1. Get a DirectMusicSegmentState8 from pPMsg.User.
            2. Using the IDirectMusicSegmentState8, call GetSegment to 
               get a DirectMusicSegment of the segment it refers to.  
               This call may fail is the segment may have gone away before this
               notification was handled.
            4. Compare this segment to the primary segment to see if this was 
               the primary segment.  If it was, then update the UI. If its not 
               then compare it to each of the motif's segments.  If a match is 
               found update the UI accordingly.
            5. Cleanup all the interfaces.
