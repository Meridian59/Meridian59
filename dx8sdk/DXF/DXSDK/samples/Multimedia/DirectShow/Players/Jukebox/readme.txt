DirectShow Sample -- Jukebox
----------------------------

Description

Video jukebox application.

This application scans a directory for media files and displays a list of the 
relevant file names.  The user can play an individual file or play all of the 
media files in order.  The jukebox also displays information about the filter 
graphs that it creates, including the names of the filters, the names of their
corresponding pins, and the event codes that are generated.

Note:  This sample requires Microsoft Foundation Class Library 4.2 (Mfc42.dll).

Note: In order to launch GraphEdit to view the currently selected file, the GraphEdt.exe
utility must exist in a directory on your search path (like c:\windows or c:\winnt).


User's Guide

If a directory name is specified as a command-line argument, the jukebox scans 
that directory at startup.  Otherwise, it scans the default SDK media directory, 
which is located at Samples\Multimedia\Media under the SDK root directory.  
The jukebox displays a list of all the media files in the directory, from which 
the user can select a file to play.

When you select a video file from the files list, Jukebox will display its
first video frame in the "Video Screen" window.  If you select an audio-only
file, the video screen will be painted gray.

The jukebox offers the following user-interface elements:

Play, Stop, Pause, and FrameStep buttons: Use these buttons to control graph
    playback.  (The FrameStep button might be disabled, if the graph does not
    support the IVideoFrameStep interface.) 

Thru and Loop buttons: Click the Thru button to play through the entire file list, 
    starting from the current selection.  Click the Loop button to loop the same 
    file repeatedly.  These two buttons are mutually exclusive. 

Mute button: Mutes the audio. 

Filters, Input Pins, and Output Pins: When the jukebox creates a graph, 
    it displays a list of the filters in the graph.  If the user selects one 
    of the filter names, the jukebox displays a list of the filter's input pins 
    and a list of the filter's output pins. 

Display Events: If this box is checked, the jukebox displays the event codes 
    that it receives.  To clear the list, click the Clear button. 

Properties button: To view a filter's property pages, select the filter name 
    and click the Properties button.  If the filter does not support 
    a property page, the button is disabled. 
