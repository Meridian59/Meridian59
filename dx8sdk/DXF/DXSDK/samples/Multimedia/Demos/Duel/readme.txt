Duel Sample Game
------------------

This program requires less than 1 Meg of video ram.

The commands which this game recognizes are listed on the opening screen
and in the help dialog (F1).

    ENTER	    - Starts game and multiplayer session setup
    LEFT ARROW	    - Turn left
    RIGHT ARROW	    - Turn right
    UP ARROW	    - Accelerate forward
    DOWN ARROW	    - Accelerate backward
    NUMPAD 5	    - Stop moving
    SPACEBAR	    - Fire
    ESC, F12        - Quit
    F1              - Help - list of commands
    F5              - toggle frame rate display


To play multiplayer one machine hosts a game, the others join.

To play using TCP/IP over the Internet, the people who are joining the
game must enter the IP address of the machine that hosted the game.
You can find out the IP address of your machine by running "winipcfg".
If you have a net card and a modem installed, you will need to make
sure you read the IP address of the modem connection to the Internet.
The modem will show up as a "PPP Adapter".  DirectPlay will not work
through proxies or firewalls.

This game allows the selection of which DirectPlay messaging protocol
is used.  The host machine can select the optimized DirectPlay
guaranteed protocol for the game session in the setup wizard.  Reliable
and asynchronous messaging, when supported by the service provider or
by using the DirectPlay Protocol, can be toggled on and off with the R
and A keys.  The protocol status will be displayed in the caption bar.

Also see the DPLAUNCH sample as an example of lobby launching this game.

Note: MSVC may include older versions of the DirectX header files and
libraries.  In order to avoid compile errors, make sure the path to the
latest DirectX header files and libraries are listed BEFORE the MSVC header
files and libraries through the Tools -> Options -> Directories menu.

In order to support multi-byte character sets like Kanji, the dialogs in the
rc file need to be changed to use the "SYSTEM" font and the rc file needs to
be recompiled.




