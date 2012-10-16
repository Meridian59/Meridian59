DirectShow Sample -- PlayCapMoniker
-----------------------------------

PlayCapMoniker is a simple video capture device previewer.  Unlike AMCap, it does
not allow you to capture data to a file.  Instead, it searches for the first
available video capture device registered in the system, connects to it,
and begins displaying its video data in a small window.  

While the PlayCap sample builds the capture graph manually using the
ICaptureGraphBuilder2 interface, this sample simply finds the moniker of the 
first available capture device, finds its display name, and uses RenderFile() to 
automatically build the graph.
