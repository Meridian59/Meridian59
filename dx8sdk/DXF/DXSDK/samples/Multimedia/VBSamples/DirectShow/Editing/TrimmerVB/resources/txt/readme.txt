DirectShow Sample -- TrimmerVB
------------------------------

Description

Microsoft Visual Basic application that trims a source clip and 
writes the trimmed clip to a new file using "smart recompression".

This example demonstrates:

- Using the MediaDet to grab poster frames from a file

- Quickly creating a timeline and using smart recompression to write a file


User's Guide

This application enables the user to open a video source file, 
trim a desired amount from the front and the end of the clip, 
and then save the remaining portion to a new file.

To use this application, do the following:

- To choose a source file, click the Browse button. 
        The first frame of the clip appears in all three preview areas. 

- To select the start position, move the slider or click the frame step 
        buttons. The preview frame in the Current Video Frame area is updated. 

- To set the start position, click the Set Trim Preview Start Position button. 
        The poster frame in Video Start Frame area is updated. 

- To select the stop position, move the slider or click the frame step buttons. 

- To set the stop position, click the Set Trim Preview Stop Position button. 
        The poster frame in the Video Stop Frame area is updated. 
        The stop position must be greater than the start position.

- To write the new file, click the Write button.
        Wait for the progress indicator to complete. 

- To view the new file, click the Playback button. 
