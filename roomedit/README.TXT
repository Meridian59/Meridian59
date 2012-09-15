                              =================
                              WinDEU 5.24 final
                              =================

Written by:
	Renaud Paquay (E-Mail rpa@info.fundp.ac.be)
	Last modified: Apr 20 1995
	Member of the DEU team and main WinDEU programmer

********************************************
* THIS IS THE FINAL VERSION OF WINDEU 5.24 *
********************************************

Note:
   The original WinDEU 5.23 distribution files were named W16DEU52.ZIP,
   W32DEU52.ZIP and WSCDEU52.ZIP.
   The WinDEU 5.24 distribution files are W16DEU524.ZIP, W32DEU524.ZIP
   and WSCDEU524.ZIP. On a FAT (DOS) system partition, these files will
   be truncated to the same name of WinDEU 5.23 distribution files.

There are currently two versions of WinDEU 5.24:

. A Windows 3.1 (Win16) version, zipped in W16DEU524.ZIP.
. A Windows NT (Win32) version, zipped in W32DEU524.ZIP. You
  need Windows NT or Win32s for Windows 3.1 to run this version.
  WinDEU32 is faster than WinDEU, thanks to 32-bit memory
  management.
. The WinDEU sources are zipped in WSCDEU524.ZIP.

See the LICENSE file for WinDEU usage.

------------
INSTALLATION
------------

Instructions to install WinDEU:

. Create a directory to install WinDEU
. Unzip (using -d option) W16DEU524.ZIP (or W32DEU524.ZIP) in a 
  new directory
. If you want 3D controls in the dialog boxes, copy the ctl3dv2.dll
  (and/or ctl3d32.dll) file from the ctl3dll directory to your Windows system
  directory (usualy c:\windows\system).
  IMPORTANT: Don't put the ctl3dv2.dll (and/or ctl3d32.dll) in the WinDEU
             directory, or the 'dll' will tell you it's no correctly 
             installed when you start WinDEU.
. Edit the windeu.ini file to suit your configuration (entering your
  doom/doom2/heretic directory is essential).
. Add an WinDEU program item in one of your program group.
. Run WinDEU by clicking to program item.
. If you are new to WinDEU, look at the 'Tutorial' in the help menu.


-------------
* IMPORTANT *
-------------

1) Before running WinDEU, you *MUST* edit the 'WINDEU.INI' file and, at the
   line corresponding to the DOOM.WAD file, write the full path name for
   this file (i.e. C:\GAMES\DOOM\DOOM.WAD). WinDEU WON'T WORK without this
   path (and file). Furthermore, you need the commercial (or registered)
   version of DOOM (1 or 2) to be able to SAVE levels data.

2) Since Feb 23 1995, WinDEU has a contextual help file system. Use it
   at any time by pressing the F1 key.


----------------------------------
Differences between DEU and WinDEU
----------------------------------

- In the level editor, WinDEU allows the use of the right mouse button 
  to bring context sensitive sub-menus. These menus are different when
  the mouse pointer is exactly on an object or not.
  In any case, the WinDEU choose the objects manipulated as below:
  If no object is HIGHLIGHTENED (GREEN),
	 the current object (YELLOW) is selected for the operation.
  If at least one object is highlightened (green),
	the operation will act on these objects, keeping in mind the selection 
	order, BUT without using the current object (yellow).

- See also EDITING HINTS


-------------
EDITING HINTS
-------------

- To edit an object (or group of objects), you can double click on
  ONE OF THE SELECTED OBJECTS. You can also press RETURN (or ENTER) or
  use the right mouse button.

- In the LineDef Edit dialog box, double click on the texture name (in
  the list box, or on the static controls) to open a window with the
  corresponding texture. You don't have to close this new window to
  go back to the LineDef Edit Window. Just click on it, it's still active.

- You get the same functionnalities in the Sector Edit dialog box and
  in the preferences dialog box.

- To select more than one object, click on the object with the SHIFT key
  pressed. It will be added in the selection list.

- The selection boxes are now dynamicaly additive or not. If the SHIFT key
  is pressed at the time of the left mosue button click, the selection box
  will be additive.

- To move an object (or group of objects), you only have to click on one
  selected object, keep the button pushed and move the objects to their
  destination (an automatic scrolling of the window will occur when the 
  pointer goes near the borders, just like in DEU).

- If you want to move in the level editor window without hightening objects
  (yellow), press the CTRL key while moving the mouse.

- You can move an information window by clicking on it, and moving the
  mouse. If you want it to stay fixed, uncheck the 
  'Window|Auto layout' menu item. If you want to replace it
  at its default position select the 'Window|Layou Now' menu item.


----------------
REVISION HISTORY
----------------

See Whatsnew.txt file.

------
FUTURE
------

The current version of WinDEU is based on the 5.21 source of DEU. I will
port DEU 5.3 to WinDEU as soon as possible.

The DEU team will try to integrate all DEU new functionalities in all 
versions of DEU (DOS, Windows, X11, OS/2). 
Just wait for news... ;)


-----------
SUGGESTIONS
-----------

If you have suggestions or bug reports, you can E-Mail me to:
rpa@info.fundp.ac.be
(Renaud Paquay)
