Windows XP DirectShow Sample -- Visual Basic Video Control
-----------------------------------------------------------

This sample demonstrates using the Microsoft Video Control to view
digital television in a window.

When VBVideoControl launches, it presents you with a list of network
providers.  Choose a network provider that is available in your area
(such as ATSC Digital Antenna TV) and click the Power On button.
VBVideoControl will attempt to build a filter graph with the 
selected network provider.  

If successful, it will update the Visual Basic form to display 
additional buttons, which you may use to change channels, adjust volume, 
display device information, and select tuners.  You may click Power Off 
to clear the form and select another tuner.

NOTE: It may take several seconds to completely build the digital
television filter graph, so please be patient.


Requirements
------------

- Windows XP (or greater) operating system

- BDA-compatible digital tuner card, such as the Broadlogic DTA-100 receiver.

  - If you select ATSC digital TV, you will need an ATSC digital tuner card.
  - If you select DVB digital TV, you will need a DVB digital tuner card.
  - If you select NTSC Analog TV, you will need an NTSC Analog tuner card.

- MPEG-2 decoder (for example, a software DVD decoder)

