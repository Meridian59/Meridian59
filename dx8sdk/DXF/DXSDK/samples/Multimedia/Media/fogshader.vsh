vs.1.0
def c40, 0.0f,0.0f,0.0f,0.0f;
;transform into projection space
m4x4 r0,v0,c8
max r0.z,c40.z,r0.z //clamp to 0
max r0.w,c12.x,r0.w //clamp to near clip plane
mov oPos,r0
add r0.w,r0.w,-c12.x
mul r0.w,r0.w,c12.y
;Load into diffuse
mov oD0.xyzw,r0.w
;load into texture
mov oT0.x,r0.w
mov oT0.y,c12.x





