vs.1.0
;------------------------------------------------------------------------------
; Constants specified by the app
;    c0      = ( 0, 0, 0, 0 )
;    c1      = ( 1, 0.5, 2, 4 )
;    c4-c7   = world-view-projection matrix
;    c8-c11  = world-view matrix
;    c12-c15 = view matrix
;    c20     = light direction
;    c21     = material diffuse color * light diffuse color
;    c22     = material ambient color
;    c28     = projection matrix
;
; Vertex components (as specified in the vertex DECL)
;    v0    = Position
;    v3    = Normal
;    v6    = Texcoords
;------------------------------------------------------------------------------


;------------------------------------------------------------------------------
; Vertex transformation
;------------------------------------------------------------------------------

; Transform to view space (world matrix is identity)
m4x4 r9, v0, c12

; Transform to projection space
m4x4 r10, r9, c28

; Store output position
mov oPos, r10


;------------------------------------------------------------------------------
; Lighting calculation
;------------------------------------------------------------------------------

dp3 r1.x, v3, c20    ; r1 = normal dot light
max r1, r1.x, c0.x     ; if dot < 0 then dot = 0
mul r0, r1.x, c21    ; Multiply with diffuse
add r0, r0, c22      ; Add in ambient
min oD0, r0, c1.x    ; clamp if > 1


;------------------------------------------------------------------------------
; Texture coordinates
;------------------------------------------------------------------------------

; Gen tex coords from vertex xz position
mul r0.xy, c24.x, r9.xz
add oT0.xy, r0.xy, c24.zw

;------------------------------------------------------------------------------
; Fog calculation
;------------------------------------------------------------------------------

; compute fog factor f = (fog_end - dist)*(1/(fog_end-fog_start))
add r0.x, -r9.z, c23.y
mul r0.x, r0.x, c23.z
max r0.x, r0.x, c0.x       ; clamp fog to > 0.0
min oFog.x, r0.x, c1.x     ; clamp fog to < 1.0



