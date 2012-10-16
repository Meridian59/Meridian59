DirectShow Sample -- GargleDMO
------------------------------

Description

This sample is a Microsoft DirectX Media Object (DMO) that creates a 
gargle effect, similar to the Gargle Filter sample for Microsoft DirectShow.

This sample demonstrates how to implement the the following interfaces:

    IMediaObject 
    IMediaObjectInPlace 
    IMediaParamInfo 
    IMediaParams 


User's Guide

To use this DMO, build and register GargleDMO.dll. The sample project builds 
several intermediate libraries.  To build the DLL in Microsoft Visual Studio,
select Batch Build from the Build menu.

You can use the DMO in the DMO Demo sample application, or with the 
DirectShow GraphEdit utility.  GraphEdit automatically wraps the DMO in the 
DMO Wrapper filter.

You can also use the DMO in Microsoft DirectMusic applications, including 
the Microsoft DirectMusic Producer application included with the SDK. 
In DirectMusic Producer, use the Audiopath Designer to insert the DMO into an 
audiopath.  To control media parameters, use the Parameter Control track. 
Insert a curve in the track and assign it to control any of the properties 
on the DMO.  For more information, see the Audiopath Designer and Parameter 
Control Track topics in the DirectMusic Producer application help.

The Gargle DMO provides a property page for the user to set the shape of the 
modulating waveform (square or triangular) and the frequency.  It also supports
a custom interface, IGargleDMOSample, for setting these properties.


For more information about this sample, see "DirectShow Samples" in the
DirectX 8 SDK documentation.