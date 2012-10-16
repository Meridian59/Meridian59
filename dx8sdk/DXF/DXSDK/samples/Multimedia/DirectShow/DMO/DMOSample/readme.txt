DirectShow Sample -- DMOSample
------------------------------

DMO Sample is a sample Microsoft DirectX Media Object (DMO). It takes MPEG-1 
video packets as input and produces two output streams. The first output stream
contains video frames in RGB 565 format. Each frame displays the time code 
taken from the MPEG-1 video, and is otherwise blank.  The second output stream 
is an optional text stream with the time code. 

DMO Sample uses the IMediaObjectImpl base-class template to implement the 
IMediaObject interface.  For information about using this template, see 
"Using the DMO Base Class" in the DirectX 8 SDK documentation. 
The Active Template Library (ATL) handles various COM details, including 
registration, aggregation, IUnknown, and the DLL entry points.


For more information about this sample, see "DirectShow Samples" in the
DirectX 8 SDK documentation.