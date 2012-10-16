DirectShow Sample -- PlayCap
----------------------------

PlayCap is a simple video capture device previewer.  Unlike AMCap, it does
not allow you to capture data to a file.  Instead, it searches for the first
available video capture device registered in the system, connects to it,
and begins displaying its video data in a small window.  

It demonstrates a simple example of using the ICaptureGraphBuilder2 and 
ICreateDevEnum interfaces to build a capture graph.  This type of simple 
previewing might be used in a videoconferencing application, for example.

