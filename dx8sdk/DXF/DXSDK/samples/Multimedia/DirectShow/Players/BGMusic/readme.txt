DirectShow Sample -- BGMusic
----------------------------

This sample shows how to swap source filters in a filter graph.  It will
cycle through three MP3 files, displaying the name of each file as it is played.
The MP3 files used by this sample are located in the Media directory of the
DirectX SDK ( <sdk root>\samples\Multimedia\Media ).

When the application begins, it renders the first audio file in the sequence 
and sets a timer.  Whenever the timer fires, the application removes the current
source filter and replaces it with a source filter for the next file.  
Since the files are all of the same type, there is no need to rebuild the 
entire graph.
