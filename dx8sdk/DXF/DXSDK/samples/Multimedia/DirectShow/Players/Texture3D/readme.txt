DirectShow Sample -- Texture3D
------------------------------

Draws video on a Microsoft DirectX 8.0 texture surface.  This sample builds
on the Direct3D Tutorial05 sample, adding support for rendering video
onto a Direct3D surface.

Note:  This sample is known to fail on some systems (referencing
invalid memory), for which you may see an error message.  This failure occurs
during initialization of Direct3D.  We recommend updating your video card 
drivers to the most current revision, as that usually resolves the problem.
Since this sample uses interfaces specific to DirectX 8, we highly
recommend installing a newer video driver, if available.

Note:  This sample does not support changing the display properties 
of the monitor while the sample is running.
