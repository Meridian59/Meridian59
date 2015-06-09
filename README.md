Meridian 59 v1.0, September 2012.
Andrew Kirmse and Chris Kirmse

Copyright 1994-2012 Andrew Kirmse and Chris Kirmse
All rights reserved.  Meridian is a registered trademark.


Play Meridian 59
--------------
You can create an account at the [Open Meridian Project homepage]
(http://openmeridian.org) and download the [client using the launcher]
(http://openmeridian.org/patcher). In addition to 103 you can access
[German](http://www.meridian59.de) and [South Korean](http://www.meridian59.co.kr/)
servers via the patcher interface. You can find a list of all known
[servers](http://wiki.openmeridian.org/index.php/Server_List) on our
[wiki](http://wiki.openmeridian.org/index.php/Main_Page).


License
--------------
This project is distributed under a license that is described in the
LICENSE file.  The license does not cover the game content (artwork,
rooms, audio, etc.), which are not included.

Note that "Meridian" is a registered trademark and you may not use it
without the written permission of the owners.

The license requires that if you redistribute this code in any form,
you must make the source code available, including any changes you
make.  We would love it if you would contribute your changes back to
the original source so that everyone can benefit.


What's included and not included
--------------
The source to the client, server, game code, Blakod compiler, room
editor, and all associated tools are included.  The source code to the
compression library and audio library are not included.


Build Instructions
--------------
These [instructions](http://wiki.openmeridian.org/index.php/Detailed_Build_Instructions)
can also be found on our [wiki](http://wiki.openmeridian.org/index.php/Main_Page).

0. Install [Microsoft Visual Studio 2013](http://www.visualstudio.com/en-us/products/visual-studio-express-vs.aspx)
or greater.
0. Download the source code, either with a git client or with the
"Download ZIP" option from your chosen repository.

### Visual Studio GUI build
0. If you prefer the Visual Studio graphical interface, open
Meridian59.sln from the root folder of the codebase. Click on the
BUILD menu and select Build Solution (or press CTRL+SHIFT+B) to build.

### Makefile build
0. Locate your Visual Studio install folder, usually something like
`"C:\Program Files (x86)\Microsoft Visual Studio 12.0"`.
Navigate to the Common folder, and then the Tools folder. Example:
`"C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools"`.
0. Create a shortcut (by right-clicking on vsvars32.bat and selecting
Create shortcut) called "Meridian Development Shell" on your desktop
or in your start menu with the following property:
Target: `%windir%\system32\cmd.exe /k "C:\Program Files (x86)\Microsoft
Visual Studio 12.0\Common7\Tools\vsvars32.bat"`
0. OPTIONAL: set the "Start In" property of your shortcut to the folder
that contains the meridian source code for ease of use.
0. Open the Meridian Development Shell and navigate to the folder
containing the source code, then enter `nmake debug=1` to compile.

Getting Started: Server
--------------
0. After compilation completes, browse to the `.\run\server folder`,
and double click `blakserv.exe` to start the server.
0. Go to the `Administration` tab on the server's interface and enter
the command: `create account admin username password` (with your
desired username and password). You will see a message saying `Created
ACCOUNT 4` or similar.
0. Then create a character slot on that account with `create admin 4`,
using whichever number the previous line returned instead of 4.
0. You'll now be able to log in with this account name and password.
Be sure to "save game" from the server interface to save this new
account.

Getting Started: Client
--------------
You will need to obtain the client graphics before you can run the
client locally. To do this, [download the patcher](http://openmeridian.org/patcher)
and use that program to download a copy of the 103 or 112 client.
When this is installed, building the client (via makefile or VS
solution) will automatically copy the needed resources to the
appropriate directory. If for some reason this isn't done, copy
the files manually from the 103/112 client's resource directory to
your repo's .\run\localclient\resource directory. Running postbuild.bat
from the root directory of the repo will also perform the copy function.


0. After compilation completes, the client is located at
`.\run\localclient`.
0. You can point your local client at your local server by running the
client `meridian.exe` with command line flags, like this:
`meridian.exe /U:username /W:password /H:localhost /P:5959`.
0. Building the client will generate a shortcut to `meridian.exe`,
with these flags, however if this shortcut isn't present in your
client directory, you can create it by making the shortcut,
right-clicking it and selecting Properties, and adding
`/H:localhost /P:5959` after the existing link in the `Target:` box.

Note that any time you recompile KOD code, changes need to be loaded
into your local blakserv server by clicking the 'reload system' arrow
icon, next to the 'save game' disk icon.

Third-Party Code
--------------
Meridian uses zlib and libarchive.  zlib was installed from binaries.
libarchive was built from source, with the following cmake environment
variables set to use zlib:

* `ZLIB_INCLUDE_DIR` points to the include directory
* `ZLIB_LIBRARY` points to lib/zdll.lib

Contact Information
--------------
For further information please join the #Meridian59 channel on
irc.esper.net. You can also join us on the
[forums](http://openmeridian.org/forums)
where you can ask any questions about the game or the codebase.

Forked from the [original Meridian 59 codebase]
(https://github.com/Meridian59/Meridian59). Original README file
included as README.old.