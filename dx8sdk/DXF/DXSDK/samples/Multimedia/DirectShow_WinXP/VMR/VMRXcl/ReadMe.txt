Windows XP DirectShow Sample -- VMRXCL
--------------------------------------

Description: This application shows capabilities of the new
    video mixing renderer (VMR) that is the default video 
    renderer in Windows XP.  In particular, it demonstrates 
    how to use the VMR in exclusive DirectDraw mode and 
    how to implement a user-provided (customized) Allocator-Presenter
    for the VMR.  Also, it contains useful utilities to manage 
    bitmaps (as textures) and TrueType fonts for text over video.

Usage:
    Upon initialization, VMRXCL asks you to specify a video file.
    The application switches to DirectDraw exclusive mode, after setting
    the display mode to 640 x 480 x 32bpp.  A bitmap-based menu on the 
    left side of the screen provides interactivity.
    (From top to bottom, the menu items are: 
	- Show statistics
	- Pause
	- Run
	- Rotate in XY plane
	- Rotate in YX plane
	- 'Twist' non-linear effect
	- Exit

    Right click over the menu button to activate its text hint.
    Left click to hide text hints.

Troubleshooting:
    Depending on the capabilities of your video driver, text may be disabled.
    You may also experience glitches with bitmaps applied over the video.

NOTE: The speed of the 3D animation is directly related to the frame rate of
    the video file being played.

