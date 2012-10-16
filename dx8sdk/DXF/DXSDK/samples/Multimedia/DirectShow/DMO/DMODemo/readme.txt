DirectShow Sample -- DMODemo
----------------------------

Description
===========
  This sample application demonstrates how to use Microsoft DirectX Media 
  Objects (DMO).  It streams audio data from a WAV file through a DirectSound
  Audio Effect DMO to a DirectSound buffer.

  For DMOs which are processing data with one input and one output, where the
  media types of the input and output are the same, and the processing can be 
  done in place, the IMediaObjectInPlace interface can be used.  If a DMO 
  supports this interface, DMODemo will call process() on this interface to 
  process the data in place; otherwise, DMODemo will use methods of IMediaObject 
  to process the data.

    
User's Guide
============
  The sample application includes a project workspace for Microsoft Visual 
  Studio 6.0.  For information on setting up your build environment, see 
  "Setting Up the Build Environment" in the DirectX8 SDK documentation.  
  The project file builds dmodemo.exe. The user can load wave files, 
  select a DSound Audio Effect DMO, and play audio using the dialog controls.

Notes
=====
For this release, the WavesReverb DMO will only accept 16-bit audio input.  If you
attempt to connect the WavesReverb DMO to an 8-bit audio source, you will see an
error dialog describing a failure to set the input type.