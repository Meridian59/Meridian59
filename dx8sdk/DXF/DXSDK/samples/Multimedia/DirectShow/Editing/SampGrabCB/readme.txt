DirectShow Editing Sample - SampleGrabberCallback
-------------------------------------------------

This C++ console app demonstates the use of the IMediaDet interface
to create a graph that contains a sample grabber filter.
It shows how to use the sample grabber and a COM object callback 
to display information about media samples in a running video file.

Usage: SampGrabCB <media filename>


Sample output:

Grabbing samples from c:\video\wink.mpg.
Sample received = 00000  Clock = 170787198
Sample received = 00001  Clock = 170787208
Sample received = 00002  Clock = 170787228
Sample received = 00003  Clock = 170787235
Sample received = 00004  Clock = 170787239
Sample received = 00005  Clock = 170787244
Sample received = 00006  Clock = 170787250
Sample received = 00007  Clock = 170787254
Sample received = 00008  Clock = 170787260
Sample received = 00009  Clock = 170787268
Sample received = 00010  Clock = 170787272
Sample grabbing complete.