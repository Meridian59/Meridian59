Windows XP DirectShow Sample -- C++ VideoControl
------------------------------------------------

This sample demonstrates using the Microsoft Video Control to view
ATSC digital television in a window.  This is a simple ATL application
that merely hosts the control and allows you to adjust the channel
up or down.

When CPPVideoControl launches, it will look for an ATSC Network
provider on your system.  If a working tuner is found, CPPVideoControl 
builds the digital television filter graph, tunes to channel 46, 
and displays the broadcast signal.  It may take several seconds to 
completely build the digital television filter graph, so please be patient.


Requirements
------------

- Windows XP operating system

- BDA-compatible ATSC digital tuner card, such as the Broadlogic DTA-100 receiver.

- MPEG-2 decoder (for example, a software DVD decoder)

