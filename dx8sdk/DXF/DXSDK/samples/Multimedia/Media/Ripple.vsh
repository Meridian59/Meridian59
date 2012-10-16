vs.1.0
; Constants:
;
;  c0-c3  - View+Projection matrix
;
;  c4.x   - time
;  c4.y   - 0
;  c4.z   - 0.5
;  c4.w   - 1.0
;
;  c7.x   - pi
;  c7.y   - 1/2pi
;  c7.z   - 2pi
;  c7.w   - 0.05
;
;  c10    - first 4 taylor coefficients for sin(x)
;  c11    - first 4 taylor coefficients for cos(x)



; Decompress position
mov r0.x, v0.x
mov r0.y, c4.w       ; 1
mov r0.z, v0.y
mov r0.w, c4.w       ; 1

; Compute theta from distance and time
mov r4.xz, r0        ; xz
mov r4.y, c4.y       ; y = 0
dp3 r4.x, r4, r4     ; d2
rsq r4.x, r4.x
rcp r4.x, r4.x       ; d
mul r4.xyz, r4, c4.x     ; scale by time

; Clamp theta to -pi..pi
add r4.x, r4.x, c7.x
mul r4.x, r4.x, c7.y
frc r4.xy, r4.x
mul r4.x, r4.x, c7.z
add r4.x, r4.x,-c7.x

; Compute first 4 values in sin and cos series
mov r5.x, c4.w       ; d^0
mov r4.x, r4.x       ; d^1
mul r5.y, r4.x, r4.x ; d^2
mul r4.y, r4.x, r5.y ; d^3
mul r5.z, r5.y, r5.y ; d^4
mul r4.z, r4.x, r5.z ; d^5
mul r5.w, r5.y, r5.z ; d^6
mul r4.w, r4.x, r5.w ; d^7

mul r4, r4, c10      ; sin
dp4 r4.x, r4, c4.w

mul r5, r5, c11      ; cos
dp4 r5.x, r5, c4.w

; Set color
add r5.x, -r5.x, c4.w ; + 1.0
mul oD0, r5.x, c4.z   ; * 0.5

; Scale height
mul r0.y, r4.x, c7.w

; Transform position
dp4 oPos.x, r0, c0
dp4 oPos.y, r0, c1
dp4 oPos.z, r0, c2
dp4 oPos.w, r0, c3

