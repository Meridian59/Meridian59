Self Shadowed Bumpmaps

This projects includes all source and content for the self shadowing bump map algorithm presented at the 2001 GDC lecture by Dan Baker and Chas Boyd. This app will run without pixel shaders, as long as the hardware has rendertargets and DOT3. However, it runs much more efficently with pixel shaders and with better visual results.


Source files 

     main.cpp        App main file
     shadowset.cpp   All the code for the self-shadowed bump map
    
     d3dapp.cpp      The App framework already included on the DX 8 sdk
     d3dfont.cpp     These files are provided for conveince only
     d3dutil.cpp     and should be replaced with newer files from the
     dxutil.cpp      sdk on newer releases of DirectX.

     bumpshader.vsh  - used for pixel shader emmulation
     bumpshader2.vsh - used for pixel shader emmulation
     bumpshader3.vsh - diffuse bump map vertex shader
     bumpshader4.vsh - Shadow map vertex shader

     shadowbumpshader.psh - horizon map basis computer pixel shader



Media files
     sphere.x        - the earth
     earth.bmp       - earth texture
     earthbump.bmp   - heightmap for earth

Camera Controls:
	'S'             - Zoom Out
	'W'             - Zoom In
	Arrow Keys      - Move camera
	NumPad arrows   - pitch camera (numlock should be on)
	Numlock '7','9' - Roll camera

Other Controls:
	'2'             - Toggle Self Shadowing
	'3'             - Toggle Diffuse bump mapping
	'4'             - Reset position
	'5'             - Auto rotate


Mouse:
	Left Mouse rotates earth
	Right Mouse moves light

