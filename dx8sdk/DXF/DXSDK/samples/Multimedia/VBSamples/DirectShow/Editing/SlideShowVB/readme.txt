DirectShow Sample -- SlideShow
------------------------------

Description

Microsoft Visual Basic application that creates a slide show from a series 
of video clips or image files.


User's Guide

This sample application demonstrates the following tasks:

- Using the MediaDet object to obtain a poster frame from a source clip. 
- Saving a timeline as an XTL file. 
- Writing a timeline to an AVI file. 
- Loading media clips. 

To use this application, do the following:

- Drag video files or image files from Windows Explorer to the cells in the
    application window, starting from the cell in the upper-left corner.

- Enter the length of time that each clip should play, in seconds. 

- Select a transition, using the combo box. The transitions have a 
    fixed two-second duration. 

- To preview the slide show, click Play. 

- To save the slide show as an XTL project, click Write XTL.  You can then
    play your new XTL file using the XTLTest, XTLTest VB, and DexterVB 
    sample applications.

- To save the slide show as an AVI file, click Write AVI. 

For more information about this sample, see the SlideShowVB.DOC documentation
in the resources\doc subdirectory.  This documentation contains screenshots
that demonstrate how to create a simple slideshow.


NOTE: The list of transitions is hard-coded into the application; consequently,
some of the transitions in the combo box may not be present on your development
machine.  If you select a transition that is not installed on your machine,
then the default "Slide" transition will be used instead.

The DirectX Media 6 SDK includes several third-party transitions in the form
of DirectX Transforms.  To enable these transitions, you can install the 
DirectX Media 6 SDK, which is available only on the DirectX 7.0a CD-ROM.  
You can order this CD-ROM by following the "DirectX 7.0a SDK CD-ROM" link on 
http://www.microsoft.com/directx/dxmedia.

