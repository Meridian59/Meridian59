DirectShow Sample -- WebDVD
---------------------------

The WebDVD sample shows how to use the MSWebDVD Microsoft ActiveX control 
to perform many aspects of DVD-Video playback and navigation.

You can initiate playback by calling the Play method.  Once the DVD is playing,
you can alter the path or behavior of playback through the following methods:

- Stop: Stops playback. 

- Eject: Ejects the DVD disc from the drive. 

- Pause: Pauses playback for as long as you wish. 

- ActivateButton: Activates the currently selected button. 
    You can select a button by:
    
    - Hovering over the button to highlight it. 
      The DisableAutoMouseProcessing property must be false (the default value)
      for this to have an effect. 

    - Calling SelectAtPosition, SelectLeftButton, SelectRightButton, 
      SelectLowerButton, or SelectUpperButton. 

- PlayNextChapter: Plays the next chapter within the current title. 

- PlayPrevChapter: Plays the previous chapter within the same title. 

You can show six possible menus that may exist on a disc with the ShowMenu method:

    Title 
    Root 
    Subpicture 
    Audio 
    Angle 
    Chapter 

If the menu does not exist, you will get an error indicating that the specified
menu does not exist.  If video was playing when you selected a menu, then 
pressing the Resume button will return you to your previous position.


Video Resolutions
-----------------

WARNING: If your display resolution is set too high, you may receive an error when
attempting to play a DVD title.  For example, when you click Play, you may see an
error message box which indicates "Unable to play DVD video."  In that case, lower
your video resolution by reducing the "Screen Area" slider on the Display Properties
control panel application.  Different video cards will have different results.
