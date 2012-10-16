DirectShow Sample -- PlayWndASF
-------------------------------

Usage:

    playwndasf <media filename>

Description:

    This sample is an interactive audio/video media file player.
    It uses DirectShow to play any Windows Media audio or video media files
    (ASF, WMA, WMV) that are not protected by Digital Rights Management (DRM).  
    The video will appear in a window on the screen, and you can use a mouse 
    to move the window.

    ----------------------------------------------------------------------------
    NOTE: To build this sample, you must install the Microsoft Windows Media
    Format SDK and obtain a software certificate.  After you obtain the software
    certificate, build the sample by linking two additional libraries: 
    		WMStub.lib and WMVCore.lib.
    ----------------------------------------------------------------------------

    If the media has a video component, PlayWndASF will read the video's
    default size and adjust the player's client area to allow the video
    to play at its preferred default size (taking into account the size of
    caption bar & borders).

    You may mute the audio by pressing 'M'.  You may toggle full-screen mode
    by pressing 'F'.  You can pause/resume playback with 'P' and stop/rewind
    with 'S'.  To close the media clip, hit ESC, F12, X, or Q.

    If the media is audio-only, the player will display a small default window.

    You can specify a media file as the only command line argument:
    	Ex:  PlayWndASF \\myserver\mediafiles\video\sample.asf

    If no file is specified, the application will automatically display the
    Windows OpenFile dialog so that you can choose a file.

    Use the menu bar or the close button to exit the application.


Accessing media files

    The media file may exist in your hard disk, CD-ROM, or on a network server.

 If the file is on a network server:
    You may read a media file from a network server if you provide the full URL.
    For example, "PlayWndASF http://myserver/mediafiles/video/myvideo.asf" will
    read the file from the server into system memory.  This approach will fail
    if the media file is too large to fit in memory.


Limitations:

    This sample will only render media files that are supported by the
    DirectShow subsystem.  If you attempt to play a video (AVI, QuickTime, MPEG)
    that is encoded with an unsupported Codec, you will only see a black
    screen or no visible change to the display, although you should hear the
    associated audio component if it uses a supported format.

    This sample will not play .ASX files.


User Input:

    Simple user input is supported through the keyboard or through the 
    application's main menu bar.

    Keyboard                    Action
    --------                    ------
    P                           Play/Pause toggle
    S                           Stop and Rewind to beginning
    M                           Audio mute toggle
    F                           Full-screen mode toggle
    Spacebar                    Step one frame
    Quit to menu (closes file)  ESC or Q or X or F12


NOTE:  This sample enforces a minimum window size (currently 200x120) to prevent
window problems when small videos are resized.  You may not resize a video to play
in a window smaller than the defined minimum size.

