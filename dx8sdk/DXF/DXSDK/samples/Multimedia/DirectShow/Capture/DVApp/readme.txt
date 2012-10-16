DirectShow Sample -- DVApp
--------------------------

Digital Video (DV) capture application.

This sample demonstrates how to build various types of filter graphs for
controlling DV camcorders. It also shows how to perform capture, preview, 
transmit, and device control with a DV camcorder.

User's Guide

The DVApp application supports the following modes:

- Preview: Renders DV from the camcorder to a video window. 
- DV to type-1 file: Captures DV data from the camcorder to a type-1 DV file. 
- Type-1 file to DV: Transmits data from a type-1 DV file to the camcorder. 
- DV to type-2 file: Captures DV data from the camcorder to a type-2 DV file. 
- Type-2 file to DV: Transmits data from a type-2 DV file to the camcorder. 

The capture and transmit modes also perform preview. Each of those modes 
has a No Preview option as well, which disables preview. Capturing 
without preview is more efficient and can reduce the number of dropped frames. 
For information about type-1 files versus type-2 files, see 
"Digital Video in DirectShow".

The application starts in preview mode. To select another mode, choose a mode 
from the Graph Mode menu. For each mode, DVApp builds a filter graph that 
supports the functionality of that mode. To save the graph as a GraphEdit (.grf)
file, select Save Graph to File from the File menu. Quit DVApp before opening 
the file in GraphEdit.

To capture to a file:

- From the Graph Mode menu, select a DV to File mode (type 1 or type 2, 
	with or without preview). 
- From the File menu, choose Set Output File and enter a file name. 
- Click Record. 
- If the camcorder is in VTR mode, click Play. 
- To stop capturing, click Stop. 

To transmit from a file to the camcorder:

- From the Graph Mode menu, select a File to DV mode (type 1 or type 2, 
	with or without preview). 
- From the File menu, click Set Input File and select a DV file. The file 
    must match the selected mode (type 1 or type 2). 
- Click Play. 
- To record the data to tape, click Record. 
- To stop transmitting, click Stop. 

If the camcorder is in VTR mode, the user can control the transport mechanism 
through the application's VCR-style buttons. To seek the tape, enter the target 
timecode and click the seek button.

To limit how much data the application captures, choose Capture Size from the 
File menu.

To check the tape format (NTSC or PAL), choose Check Tape from the Options menu.

To change the size of the preview window, choose Change Decode Size from the 
Options menu. 


For more information about this sample, see the "Sample Capture Applications"
section in the DirectShow section of the DirectX SDK documentation.
