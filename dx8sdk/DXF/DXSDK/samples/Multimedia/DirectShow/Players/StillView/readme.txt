DirectShow Sample -- StillView
------------------------------

This application scans a directory for image files (BMP, GIF, JPG, TGA) and displays 
a list of the relevant file names.  Click a file name in the listbox to display its
image on the Image Viewer screen, along with image information such as its video 
dimensions and file size.

StillView also displays information about the filter graphs that it creates, 
including the names of the filters and their corresponding pins.  If a selected filter
supports a property page, you can click the Filter Properties button to view the page.

Note:  This sample requires Microsoft Foundation Class Library 4.2 (Mfc42.dll).

Note: In order to launch GraphEdit to view the currently selected file, the GraphEdt.exe
utility must exist in a directory on your search path (like c:\windows or c:\winnt).
