vs.1.0
;------------------------------------------------------------------------------
; Constants specified by the app
;    c0      = (0,0,0,0)
;    c1      = (1,1,1,1)
;    c2      = (0,1,2,3)
;    c3      = (4,5,6,7)
;    c4-c7   = matWorld0
;    c8-c11  = matWorld1
;    c12-c15 = matViewProj
;    c20     = light direction
;    c21     = material diffuse color * light diffuse color
;    c22     = material ambient color
;
; Vertex components (as specified in the vertex DECL)
;    v0    = Position
;    v1.x  = Blend weight
;    v3    = Normal
;    v7    = Texcoords
;------------------------------------------------------------------------------


;------------------------------------------------------------------------------
; Vertex blending
;------------------------------------------------------------------------------

; Transform position for world0 matrix
dp4 r0.x, v0, c4
dp4 r0.y, v0, c5
dp4 r0.z, v0, c6
dp4 r0.w, v0, c7

; Transform position for world1 matrix
dp4 r1.x, v0, c8
dp4 r1.y, v0, c9
dp4 r1.z, v0, c10
dp4 r1.w, v0, c11

; Lerp the two positions r0 and r1 into r2
mul r0, r0, v1.x     ; v0 * weight
add r2, c1.x, -v1.x  ; r2 = 1 - weight
mad r2, r1, r2, r0   ; pos = (1-weight)*v1 + v0*weight

; Transform to projection space
dp4 oPos.x, r2, c12 
dp4 oPos.y, r2, c13
dp4 oPos.z, r2, c14
dp4 oPos.w, r2, c15


;------------------------------------------------------------------------------
; Lighting calculation
;------------------------------------------------------------------------------

; Transform normal for world0 matrix
dp4 r0.x, v3, c4
dp4 r0.y, v3, c5
dp4 r0.z, v3, c6
dp4 r0.w, v3, c7

; Transform normal for world1 matrix
dp4 r1.x, v3, c8
dp4 r1.y, v3, c9
dp4 r1.z, v3, c10
dp4 r1.w, v3, c11

; Lerp the two normals r0 and r1 into r2
mul r0, r0, v1.x     ; v0 * weight
add r2, c1.x, -v1.x  ; r2 = 1 - weight
mad r2, r1, r2, r0   ; normal = (1-weight)*v1 + v0*weight

; Do the lighting calculation
dp3 r1.x, r2, c20    ; r1 = normal dot light
max r1, r1.x, c0     ; if dot < 0 then dot = 0
mul r0, r1.x, c21    ; Multiply with diffuse
add r0, r0, c22      ; Add in ambient
min oD0, r0, c1.x    ; clamp if > 1


;------------------------------------------------------------------------------
; Texture coordinates
;------------------------------------------------------------------------------

; Just copy the texture coordinates
mov oT0,  v7


