DirectShow Sample -- CutScene
-----------------------------

Usage:

    cutscene <media filename>

Description:

    Cutscene is a simple fullscreen movie player sample that enables you to
    add movie playback to your application without needing to learn the 
    specifics of DirectShow programming.

    The application's WinMain() calls PlayCutscene(), a generic function that 
    you can use to add easy playback of movies or cutscenes to the beginning 
    of your game.  All of the code needed for movie playback is contained 
    within cutscene.cpp and cutscene.h and can be added to your project.

    Cutscene creates a hidden window to handle keyboard input.  A user can 
    press SPACE, ENTER, or ESCAPE keys to cause the playback to end.  
    When playback ends, the window is destroyed.

Return values:

    If movie playback continues to the end without error, 
        PlayCutscene() returns S_OK.

    If movie playback is interrupted by the user, PlayCutscene returns S_FALSE.
        (S_FALSE is not an error and will not fail in the FAILED() macro.)

    If there is an error, PlayCutscene returns an HRESULT error code.

