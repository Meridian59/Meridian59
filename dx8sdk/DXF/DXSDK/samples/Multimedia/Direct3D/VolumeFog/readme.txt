//-----------------------------------------------------------------------------
// Name: Volume Fog Direct3D Sample
// 
// Copyright (c) 1998-2001 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


Description
===========
   The Volume Fog sample shows the per-pixel density volumetric rendering 
   technique. The fog volume is modeled as a polygonal mesh, and the 
   density of the fog at every pixel is computed by subtracting the front
   side of the fog volume from the back side. The fog is mixed with the 
   scene by accumulating an in/out test at every pixel -- that is, back-facing
   fog polygons will add, while front-facing ones will subtract. If the value
   is non zero, then the scene intersects the fog and the scene's depth value
   is used. In order to get better results, this demo uses 12 bits of precision
   by encoding high and low bits in different color channels. 


Path
====
   Source:     DXSDK\Samples\Multimedia\D3D\volumefog
   Executable: DXSDK\Samples\Multimedia\D3D\Bin


User's Guide
============
   The following keys are implemented. 
   <J>    Move object backward on the Z axis
   <M>    Move Object forward on the z Axis
   <H>    Move Object forward on the X axis
   <K>    Move object backward on the X axis
   <N>    Move object forward on the y axis
   <Y>    Move object backward on the y axis

   Camera Controls:
   <LEFT>    Slide Left
   <RIGHT>   Slide Right
   <DOWN>    Slide Down
   <UP>      Slide Up
   <W>       Move forward
   <S>       Move backward
   <NUMPAD8> Pitch Down
   <NUMPAD2> Pitch Up
   <NUMPAD4> Turn Right
   <NUMPAD6> Turn Left
   <NUMPAD9> Roll CW
   <NUMPAD7> Roll CCW

   Mouse Controls:
   Rotates Fog Volume.
      

Programming Notes
=================
Introduction
   The article "Volumetric Rendering in Real-Time," printed in the 2001 GDC 
   Proceedings, covered the basis of volumetric depth rendering, but at the 
   time of the writing, no pixel-shader-compliant hardware was available. 
   This supplement describes a process designed to achieve two goals: to get 
   more precision out of an 8 bit part, and to allow the creation of concave 
   fog volumes.


Handling Concavity
   Computing the distance of fog for the convex case was relatively simple. 
   Recall that the front side of the fog volume was subtracted away from the 
   back side (where the depth is measured in number of units from the camera). 
   Unfortunately, this does not work with concave fog volumes because at any 
   given pixel, it may have two back sides and two front sides.  The solution 
   is intuitive and has sound mathematical backing: sum all of the front 
   sides and subtract them from the summed back sides. 
 
   So now, computing concavity is as simple as adding the multiple front 
   sides and subtracting them from the multiple back sides. Clearly, a meager 
   8 bits won’t be enough for this.  Every bit added would allow another 
   summation and subtraction, and allow for more complex fog scenes.

   There is an important assumption being made about the fog volume. Is must 
   be a continuous, orientable hull. That is, it cannot have any holes in 
   it. Every ray cast through the volume must enter through hull the same 
   number of times it exits.  

Getting Higher Precision
   Although most 3D hardware handle 32 bits, it is really four 
   8-bit channels. The way most hardware works today, there is only one place 
   where the fog depths could be summed up: the alpha blender.  The alpha 
   blender is typically used to blend on alpha textures by configuring the 
   source destination to multiply against the source alpha, and the 
   destination to multiply against the inverse alpha.  However, they can also 
   be used to add (or subtract) the source and destination color channels. 
   Unfortunately, there is no way to perform a carry operation here: If one 
   channel would exceed 255 for a color value, it simply saturates to 255. 
   In order to perform higher bit precision additions on the alpha blending 
   unit, the incoming data has to be formatted in a way which is compatible 
   with the way the alpha blender adds. To do this, the color channels can 
   hold different bits of the actual result, and most importantly, be allowed 
   some overlap in their bits. 

   This sample uses the following scheme: The red channel will contain the 
   upper 8 bits, and the blue channel will contain the lower 4 plus 3 carry 
   spots. The upper bit should not be used for reasons which are discussed 
   later. So the actual value encoded is Red*16+Blue.  Now, the alpha blender 
   will add multiple values in this format correctly up to 8 times before 
   there is any possibility of a carry bit not propagating. This limits the 
   fog hulls to ones which do not have concavity where looking on any 
   direction a ray might pass in and out of the volume more than 8 times. 
   Encoding the bits in which will be added cannot be done with a pixel 
   shader. There are two primary limitations. First, the color interpolators 
   are 8 bit as well. Since the depth is computed on a per vertex level, this 
   won’t let higher bit values into the independent color channels. Even if 
   the color channel had a higher precision, the pixel shader has no 
   instruction to capture the lower bits of a higher bit value.
   The alternative is to use a texture to hold the encoded depths. The 
   advantage of this is twofold. First, texture interpolaters have much 
   higher precision than color interpolaters, and second, no pixel shader is 
   needed for initial step of summing the font and back sides of the fog 
   volume. It should be possible, on parts with at least 12 bits precision in 
   the pixel shader, to emded the precision in a texture registers instead.
   Unfortunately, most hardware limits the dimensions of textures. 4096 is a 
   typical limitation. This amounts to 12 bits of precision to be encoded in 
   the texture. 12 bits, however, is vastly superior to 8 bits and can make 
   all the difference to making fog volumes practical. More precision could 
   be obtained by making the texture a sliding window, and breaking the 
   object into sizable chunks which would index into that depth, but this 
   sample does not do this.

Setting it all Up
   Three important details remain: The actual summing of the fog sides, 
   compensating for objects inside the fog, and the final subtraction.
   The summing is done in three steps. 
   
   First, the scene needs to be rendered 
   to set a Z buffer. This will prevent fog pixels from being drawn which are 
   behind some totally occluding objects. In a real application, this z could 
   be shared from the pass which draws the geometry. The Z is then write 
   disabled so that fog writes will not update the z buffer. 
   
   After this, the summing is exactly as expected. The app simply draws all 
   the forward facing polygons in one buffer, adding up their results, and 
   then draws all the backward facing polygons in another buffer. There is 
   one potential problem, however. In order to sum the depths of the fog 
   volume, the alpha blend constants need to be set to one for the 
   destination and one for the source, thereby adding the incoming pixel with 
   the one already in the buffer.
   Unfortunately, this does not take into account objects inside the fog that 
   are acting as a surrogate fog cover.  In this case, the scene itself must 
   be added to scene since the far end of the fog would have been rejected by 
   the Z test. 

   At first, this looks like an easy solution. In the previous article, the 
   buffers were set up so that they were initialized to the scene’s depth 
   value. This way, fog depth values would replace any depth value in the 
   scene if they were in front of it (i.e. the Z test succeeds) -- but if no 
   fog was present the scene would act as the fog cover.
   
   This cannot be done for general concavity, however.  While technically 
   correct in the convex case, in the concave case there may be pixels at 
   which the fog volumes are rendered multiple times on the front side and 
   multiple sides on the backside. For these pixels, if the there was part 
   of an object in between fog layers than the front buffer would be the sum 
   of n front sides, and the back side would be sum of n-1 back sides. But 
   since the fog cover was replaced by the fog, there are now more entry 
   points then exit points. The result is painfully obvious: parts of the 
   scene suddenly loose all fog when they should have some.

   The solution requires knowing which scenarios where the scene’s w depth 
   should be added and which scenarios the scene’s w depth should be ignored. 
   Fortunately, this is not difficult to find. The only situation where the 
   scene’s w depth should be added to the total fog depth are those pixels 
   where the object is in between the front side of a fog volume and its 
   corresponding backside.  

   The above question can be thought of as asking the question: did the ray ever 
   leave the fog volume? Since the fog hulls are required to be continuous, 
   then if the answer is no then part of the scene must have blocked the ray.  
   This test can be performed by a standard inside outside test.
   
   To perform an inside/outside test each time a fog pixel is rendered, the 
   alpha value is incremented. If the alpha values of the far fog distances 
   is subtracted to the corresponding point on the near fog distance, then 
   values greater then 1 indicate the ray stopped inside the volume. Values 
   of 0 indicate that the ray left the fog volume.
   
   To set this test up, the alpha buffer of the near and far w depth buffers 
   must be cleared to 0.  Each time a fog pixel is rendered, the alpha will 
   be incremented by the hex value 0x10. This value was used because the 
   pixel shader must perform a 1 or 0 logical operation. A small positive 
   value must be mapped to 1.0 in the pixel shader, a step which requires 
   multiple shifts. Due to instruction count restraints the intial value 
   has to be at least 0x10 for the shifts to saturate a non-zero value to one.
   The rest is straightforward: all the front sides and all the backsides 
   are summed up in their independent buffers. The scene is also drawn in its 
   own buffer. Then all three buffers are ran through the final pass where 
   the scene’s w depth is added on only if the differences of the alpha 
   values is not 0.

   This requires a lengthy pixel shader. A great deal of care must be taken 
   to avoid potential precision pitfalls. The following pixel shader performs 
   the required math, although it requires every instruction slot of the 
   pixel shader and nearly every register. Unfortunately, with no carry bit, 
   there is no way to achieve a full 8 bit value at the end of the 
   computation, so it must settle for 8.

ps.1.1
def c1, 1.0f,0.0f,0.0f,0.0f
def c4, 0.0f,0.0f,1.0f,0.0f

tex t0		// near buffer B
tex t1		// far buffer A
tex t2              // scene buffer C

// input:
// b    = low bits (a)  (4 bits) 
// r   = high bits (b) (8 bits)
// intermediate output: 
// r1.b  = (a1 - a2) (can't be greater than 7 bits set )
// r1.r   = (b1 - b2)

sub r1.rgb,t1,t0
+sub_4x r1.a,t0,t1      //If this value is non zero, then 
mov_4x r0.a,r1.a        //the were not as many backs as 
mad r1.rgb,r0.a,t2,r1 //front and must add in the scene
dp3 t0.rgba,r1,c4        // move red component into alpha

// Need to shift r1.rgb 6 bits.  This could saturate
// to 255 if any other bits are set, but that is fine
// because in this case, the end result of the subtract 
// would have to be saturated since we can't be
// subtracting more than 127
mov_x4 r1.rgb,r1
dp3_x4 t1.rgba,r1,c1   // move into the alpha
add_x2  r0.a,t0.a,t1.a // the subtract was in 0-127
mov_d2   r0.a,r0.a     // chop off last bit else banding
+mov r0.rgb,c3         // load the fog color


   This pixel shader gives an alpha value which represents the density of fog, 
   and loads the fog color constant into the color channels. The Alpha 
   Blending stage can now be used to blend on the fog.  

   Finally, there is one situation which can cause serious problems: 
   clipping. If a part of the fog volume is clipped away by the camera 
   because the camera is partially in the fog, then part of the scene might 
   be in the fog. Previously, it was assumed the camera was either entirely 
   all the way in, or all the way out of the fog. This may not always be the 
   case.

   An alternative solution is to not allow polygons to get clipped. The 
   vertex shader can detect vertices which would get clipped away and snap 
   them to the near clip plane. The following vertex shader clips w depths 
   to the near clip plane, and z depths to zero.

// transform position into projection space
m4x4 r0,v0,c8
max r0.z,c40.z,r0.z       //clamp to 0
max r0.w,c12.x,r0.w     //clamp to near clip plane
mov oPos,r0

// Subtract the Near clipping plane
add r0.w,r0.w,-c12.x 

// Scale to give us the far clipping plane
mul r0.w,r0.w,c12.y

// load depth into texture, don’t care about y
mov oT0.xy,r0.w


   
   This sample makes use of common DirectX code (consisting of helper functions,
   etc.) that is shared with other samples on the DirectX SDK. All common
   headers and source code can be found in the following directory:
      DXSDK\Samples\Multimedia\Common

