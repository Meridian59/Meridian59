Windows XP DirectShow Sample -- VMRMix
--------------------------------------

Description: This application shows capabilities of the new
    video mixing renderer (VMR) that is the default video 
    renderer in Windows XP.  In particular, it demonstrates 
    how to use the VMR in a mixing mode with several sources, 
    how to apply a bitmap image with a color key over the video, 
    and how to take advantage of the IVMRMixerControl interface 
    to manage source and destination rectangles and alpha-level 
    for each media stream.

Usage:
    Upon initialization, VMRMix asks the user to specify a 
    media folder that contains at least two valid media files, 
    after which it loads media settings from that folder.  
    The user is asked to specify playback options: 
    number of source files, size of the playback window, and 
    whether to display a static bitmap image.  When the user 
    clicks on the 'Play' button, a new window appears to mix 
    the selected streams.  The demonstration lasts until the 
    longest media file reaches the end.  You can interrupt the 
    demonstration  by closing the playback window.

Troubleshooting:
    This application was originally created as a stress test, 
    so it uses more system resources when displaying a maximum 
    number of streams and when using "full-screen" mode.  If 
    video is freezing or slows down, try selecting fewer sources 
    and turn off the "full-screen" option.
