DirectShow Sample -- DDrawXCL
-----------------------------


            DirectDraw Exclusive Mode Video Playback Capability
            ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


General Information:
====================
Now video can be played back from AVI, MPEG files as well as DVD titles
even in DirectDraw exclusive fullscreen mode.  This is expected to be
very helpful to games and interactive content development.

The DirectX SDK includes various components that together
offer this capability.  An updated Overlay Mixer filter in DirectShow
is now capable of using a DirectDraw object and primary surface provided
by an application, rather than insisting on using its own.

A new COM interface, IDDrawExclModeVideo, has been added on the Overlay 
Mixer filter which allows applications to specify DirectDraw parameters
and playback position, size, etc.  An app can do QueryInterface() on the
Overlay Mixer filter to get the interface.  This interface is fully 
documented in the SDK help documentation.

Also another new COM interface, IDDrawExclModeVideoCallback, has been
defined in the SDK.  An application should implement an object that supports
this interface to have a notification set up with the OverlayMixer about
any changes to the overlay size, position, color key, etc. This callback
mechanism allows any color key flashes and/or strips to be avoided.

We have also updated the DVD graph builder object and some methods of the
IDvdGraphBuilder interface which allows applications to use the updated
DVD graph builder object to pass the DirectDraw-related information to the
Overlay Mixer for use in DVD playback filter graph building operation.
Using a couple of other methods available on the IDDrawExclModeVideo
interface, applications can get the color key information for
superimposing graphics on top of video playing in the background. All of
these changes have been fully documented in the SDK help.

A sample application, DDrawXcl, has also been added to the SDK samples.
This application consists of:

* A DirectDraw object (DDrawObj.h, .cpp) that wraps all DirectDraw 
functionality using IDirectDraw* COM interfaces as well as the
IDDrawExclModeVideo interface.

* A video playback object (VidPlay.h, .cpp) that implements the details
of building a filter graph to play AVI/MPEG files as well as DVD titles.

* An overlay notification sink object that receives the overlay change
related callbacks from OverlayMixer and takes appropriate actions.

* A Windows application that uses the above objects to:

  - Go into fullscreen exclusive mode, switch to 800x600x8 (we just chose
    one particular mode), and create a primary surface with one back buffer.

  - Build a filter graph to playback the selected type of video using
    the DirectDraw object and surface provided by the application,
    starts playing it and sets a timer to fire every 0.1 second.

  - On the timer event, the application draws color key of the required 
    dimension on the back buffer, draws a ball that can be moved using 
    keystrokes, draws an updated number of flips completed, and flips the
    surfaces.

  - The callback notification object informs the DDraw object when color
    keying of the video area should be started/stopped.

This is one of the basic requirements for games and interactive 
content to play video using DirectShow inside the application.



The Limitations/Rules:
======================
1. The sequence of calls to get the application-created DirectDraw parameters
to be used by OverlayMixer is as follows --

   For DVD:
   - create the DVD graph builder object
   - call IDvdGraphBuilder::GetDvdInterface() for IDDrawExclModeVideo
     interface. This returns the IDDrawExclModeVideo interface of the
     OverlayMixer in the DVD playback filter graph.
   - call IDDrawExclModeVideo::SetDDrawObject() to set the DDraw object
   - call IDDrawExclModeVideo::SetDDrawSurface() to set the DDraw surface
   - call IDvdGraphBuilder::RenderDvdVideoVolume() to build a DVD graph
   - call IDvdGraphBuilder::GetDvdInterface() to get a IMixerPinConfig(2)
     interface pointer to the FIRST (only) input pin of the OverlayMixer.
   - get color key info using IMixerPinConfig(2)::GetColorKey() 
     method.  Also set the aspect ratio mode to stretching from the
     default letter box mode of the OverlayMixer.
   - call IDDrawExclModeVideo::SetCallbackInterface() with the pointer to the
     object implementing the IDDrawExclModeVideoCallback interface.
   - get the native video size and aspect ratio through
     IDDrawExclModeVideo::GetNativeVideoProps() method, transform the width
     and height based on the retrieved aspect ratio, and set the final video
     size and position using IDDrawExclModeVideo::SetDDrawParameters() method.

   For AVI/MPEG:
   - create an empty filter graph
   - call IGraphBuilder::RenderFile() on the specified file to build graph.
   - instantiate the OverlayMixer filter and add it to the graph.
   - call IDDrawExclModeVideo::SetDDrawObject()
   - call IDDrawExclModeVideo::SetDDrawSurface()
   - connect the decoded video to the Overlay Mixer's first input pin
   - remove the Video Renderer filter as it's a windowless playback case.
   - get IMixerPinConfig(2) interface from the first input pin of the
     OverlayMixer to get the color key through GetColorKey() method.
     Also set the aspect ratio mode to stretching from the default letter box 
     mode of the OverlayMixer.
   - call IDDrawExclModeVideo::SetCallbackInterface() with the pointer to the
     object implementing the IDDrawExclModeVideoCallback interface.
   - get the native video size and aspect ratio through
     IDDrawExclModeVideo::GetNativeVideoProps() method, transform the width
     and height based on the retrieved aspect ratio, and set the final video
     size and position using IDDrawExclModeVideo::SetDDrawParameters() method.

2. The object implementing IDDrawExclModeVideoCallback interface has been
kept very simple.  It stops color keying of the video area on the back buffer
when it gets the OnUpdateOverlay() call with input parameter bBefore=TRUE.
The color keying is turned on again on getting another OnUpdateOverlay() call
with bBefore=FALSE and with the dwFlags indicating that the overlay is visible.

3. The SetDDraw*() methods can be called only when the pins of the Overlay
Mixer are not connected.  Otherwise, the calls will fail.

4. The application has to draw colorkey at the right position on the (back 
buffer of the) primary surface to get the video to appear. In the exclusive 
mode, the Overlay Mixer doesn't draw color key at all.

5. For DVDs, the subpicture and line21 (closed caption) data are intentionally
not rendered in the exclusive mode. The DVD graph builder will not return
S_FALSE for this only.



Known Issues:
=============
1. The ball drawn on top of the video only moves when an arrow key is 
pressed AND released.  Holding the arrow key down doesn't move the ball.
This was a conscious decision made for the sample application.

2. The application stops playing the video and tears down the filter graph 
when it switches to non exclusive mode on getting an Alt-Tab keystroke. The
playback is not resumed on activating the application.

3. The application fills the whole back buffer with black color and then
Blts the color key to the target video rectangle. This is not required unless
the video is continually changing position. For video playing at the same
position, just erasing the previous position of the ball with color key 
should be sufficient. The comment at the top of the CDDrawObject::FillSurface()
method should hopefully not cause any confusion.

4. Some AVI files (MS-CRAM, Duck compressed) fail to connect to the 
OverlayMixer filter on some display cards.  This causes the sample app to
fail in building the graph. As a result, such files cannot be played back on
some systems.
