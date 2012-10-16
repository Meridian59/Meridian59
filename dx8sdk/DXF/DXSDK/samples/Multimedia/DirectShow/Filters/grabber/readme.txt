DirectShow Sample -- GrabberSample Filter
------------------------------------------

This sample provides modified source code for the SampleGrabber Filter.

To prevent conflicts with the existing SampleGrabber filter, the
following changes have been made:

- Filter is renamed to "GrabberSample" (to indicate that it is 
    an example implementation of a SampleGrabber-style filter)
    
- CLSID is changed to CLSID_GrabberSample

- The CLSID is a newly generated GUID


==========================================
Documentation
==========================================

This is a transform filter that you can insert inline of a media stream
which can "capture" the data travelling through the stream and notify
your application on a callback.  Since it is a transinplace transform,
it won't cause extra application loading because it intercepts the
media stream.  However, if your callback takes a long time to process,
this benefit will be greatly reduced.

This sample grabber example is quite simple, and it has the following
limitations:

- It doesn't synchronize itself with the main application thread, 
  so the app must deal with multithreading issues.  
  
- It doesn't buffer the data before calling back, so there's no 
  one-shot mode.


Ideas for extending this sample
-------------------------------
If you want to extend the capabilities of this example filter, the following
list provides some ideas that may make it more useful in your applications.

- Allow it to buffer samples and call back the application on a background thread

- Negotiate pin connections to force either an upstream or downstream allocator
  to be used

- Don't allow pre-roll samples to pass through, and check for zero-length samples

- Write a sample grabber and graph wrapper COM object to allow your application
  to pull samples synchronously from a stream


Limitations of the DirectX 8.0 SampleGrabber filter
---------------------------------------------------
The SampleGrabber that shipped with DirectX 8.0 had some issues which are
listed below.  The source code for the original SampleGrabber filter
was not made available.  This GrabberSample example filter should be used in 
your applications instead if you want to debug or modify its functionality.

- The DX8.0 filter will not work with any video that doesn't offer 
  VideoInfoHeader as the format type.  MPEG2 and DV data are NOT allowed.
  Video using VideoInfoHeader2 data would not connect with the SampleGrabber.
  
- The SampleGrabber filter's output pin does not support connecting 
  to the overlay mixer.

- It doesn't deal with field-based video

- One-shot mode didn't work correctly in some circumstances

- We intended to allow the sample grabber to function as a renderer in cases
  where only its input pin was connected, but this feature was abandoned.
  
- The buffering feature of the sample grabber works, but has limited value.


Suggestions for using the SampleGrabber and new GrabberSample filters
---------------------------------------------------------------------

- Don't use RenderFile and assume that the sample grabber will be inserted into a
  chain of filters exactly as you desire.  Verify that the SampleGrabber filter
  is connected properly and that it is using the expected video type.
  
- Always connect the output of the sample grabber to a downstream filter. 
  If you just want to get data from media streams, connect to a NULL Renderer.
  
- If you want the graph to run as fast as possible, turn off the graph clock
  via IMediaFilter::SetSyncSource( NULL ).

 