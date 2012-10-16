DirectShow_WinXP SDK Samples README
-----------------------------------

The samples in the DirectShow_WinXP directory demonstrate DirectShow features
that are available only on Windows XP (or later platforms).  While you can
build these samples on any Windows system, you can run them only on Windows XP.

You must also ensure that the Windows version (WINVER) is set to 0x501 when
building these samples to enable Windows XP features and to take advantage of 
updated segments in relevant header files used for Windows XP development.
(Some DirectShow headers have special sections that pertain to Windows XP
features and capabilities, and these sections are conditionally compiled
according to the currently defined Windows version.)

---------------------------------------------------------------------------
NOTE: The DirectShow_WinXP samples do not yet build properly for UNICODE.
---------------------------------------------------------------------------

The DirectShow_WinXP SDK samples are subdivided into directories according
to their major function:

	VideoControl -  Microsoft Video Control for Windows XP, which provides
			many features used by the Broadcast Digital Architecture

	VMR	      - Windows XP Video Mixing Renderer

A brief summary of the DirectShow_WinXP samples is provided below.  For more
detailed information, view the Readme.txt in each sample's directory.


Video Control Samples
----------------------

C++ - 	This sample demonstrates using the Microsoft Video Control to render 
	ATSC digital television in a window.

VisualBasic - This sample demonstrates using the Microsoft Video Control 
	to render digital television in a window, using several available 
	network providers.  You may also change channels, adjust volume, 
	and view device information.

HTML - 	These HTML files demonstrate how to display ATSC, DVB, or Analog 
	television on a Web page, using the Microsoft Video Control.

VMR Samples
------------

Cube - 	Demonstrates using the Video Mixing Renderer and a plug-in compositor 
	to render three videos simultaneously on a spinning cube.

Renderless - Demonstrates using the Video Mixing Renderer and a custom 
	allocator-presenter to render video in a window.

TxtPlayer - Demonstrates using the Video Mixing Renderer and a custom 
	allocator-presenter to render alpha-blended text over a running video.

VMRMix - Demonstrates how to use the VMR in a mixing mode with several sources, 
	how to apply a bitmap image with a color key over the video, 
	and how to take advantage of the IVMRMixerControl interface to manage 
	source and destination rectangles and alpha-level for each media stream.

VMRPlayer - Demonstrates using the Video Mixing Renderer to alpha blend one or two 
	running videos and a static image, all of which can be moved and resized. 

VMRXCL - Demonstrates how to use the VMR in DirectDraw exclusive mode and how to 
	implement a user-provided (customized) Allocator-Presenter for the VMR.  
	It also contains useful utilities to manage bitmaps (as textures) and 
	TrueType fonts for text over video.

