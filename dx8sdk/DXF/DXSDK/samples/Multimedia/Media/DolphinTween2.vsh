;------------------------------------------------------------------------------
; Constants specified by the app
;    c0      = ( 0, 0, 0, 0 )
;    c1      = ( 1, 0.5, 2, 4 )
;    c2      = ( fWeight1, fWeight2, fWeight3, 0 )
;    c4-c7   = matWorldViewProjection
;    c8-c11  = matWorldView
;    c19     = light direction (in model space)
;    c21     = material diffuse color * light diffuse color
;    c22     = material ambient color
;
; Vertex components (as specified in the vertex DECL)
;    v0    = Position
;    v3    = Normal
;    v6    = Texcoords
;------------------------------------------------------------------------------
vs.1.1

;------------------------------------------------------------------------------
; Vertex transformation
;------------------------------------------------------------------------------

; Tween the 3 positions (v0,v1,v2) into one position
mul r0, v0, c2.x
mul r1, v1, c2.y
mul r2, v2, c2.z
add r3, r0, r1
add r3, r3, r2

; Transform position to the clipping space
m4x4 oPos, r3, c4

; Transform position to the camera space
m4x4 r9, r3, c8

;------------------------------------------------------------------------------
; Lighting calculation
;------------------------------------------------------------------------------

; Tween the 3 normals (v3,v4,v5) into one normal
mul r0, v3, c2.x
mul r1, v4, c2.y
mul r2, v5, c2.z
add r3, r0, r1
add r3, r3, r2

; Do the lighting calculation
dp3 r1.x, r3, c19    ; r1 = normal dot light
max r1.x, r1.x, c0.x   ; if dot < 0 then dot = 0
mul r0, r1.x, c21    ; Multiply with diffuse
add r0, r0, c22      ; Add in ambient
min oD0, r0, c1.x    ; clamp if > 1


;------------------------------------------------------------------------------
; Texture coordinates
;------------------------------------------------------------------------------

; Gen tex coords from vertex xz position
mul oT0.xy, c1.y, r9.xz

;------------------------------------------------------------------------------
; Fog calculation
;------------------------------------------------------------------------------

; compute fog factor f = (fog_end - dist)*(1/(fog_end-fog_start))
add r0.x, -r9.z, c23.y
mul r0.x, r0.x, c23.z
max r0.x, r0.x, c0.x       ; clamp fog to > 0.0
min oFog.x, r0.x, c1.x     ; clamp fog to < 1.0



