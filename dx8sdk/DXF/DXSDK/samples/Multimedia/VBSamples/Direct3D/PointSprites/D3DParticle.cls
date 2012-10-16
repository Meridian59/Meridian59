VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "CParticle"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit


'-----------------------------------------------------------------------------
' Global data for the particles
'-----------------------------------------------------------------------------

Private Type CUSTOMVERTEX
    v As D3DVECTOR
    color As Long
    tu As Single
    tv As Single
End Type

Const D3DFVF_COLORVERTEX = (D3DFVF_XYZ Or D3DFVF_DIFFUSE Or D3DFVF_TEX1)

Private Type PARTICLE
    m_bSpark As Boolean         ' Spark? or real particle?

    m_vPos As D3DVECTOR         ' Current position
    m_vVel As D3DVECTOR         ' Current velocity
    
    m_vPos0 As D3DVECTOR        ' Initial Position
    m_vVel0 As D3DVECTOR        ' Initial Velocity
    m_fTime0 As Single          ' Time of creation

    m_clrDiffuse  As D3DCOLORVALUE       ' Initial diffuse color
    m_clrFade As D3DCOLORVALUE           ' Faded diffuse color
    m_fFade As Single           ' Fade progression
    
    iNext  As Long              ' Next particle in list
    
End Type

Dim m_Particles() As PARTICLE   'we leave 0 element unused to make code convenient
                                'so think of this as a 1 based array
Dim m_fRadius As Single
Dim m_MaxParticles As Long

Dim m_NumParticles As Long
Dim m_ParticlesLim As Long

Dim m_iFree As Long     'index of first free particle (0 = empty)
Dim m_iUsed As Long     'index of first particle in list (0 = empty)
Dim m_iLast As Long

'Geometry
Dim m_VertB As Direct3DVertexBuffer8
Dim m_IndxB As Direct3DIndexBuffer8

Dim m_Verts() As CUSTOMVERTEX

Dim m_binit As Boolean
'-----------------------------------------------------------------------------
' Name: Init
' Desc:
'-----------------------------------------------------------------------------
Sub Init(MaxParticles As Long, fRadius As Single)

    m_fRadius = fRadius
    m_MaxParticles = MaxParticles
    m_NumParticles = 0
    m_ParticlesLim = 1800

    m_iFree = 0
    m_iUsed = 0
    
    Set m_VertB = Nothing
    Set m_IndxB = Nothing
    m_binit = True
    ReDim m_Verts(MaxParticles * 6)
    ReDim m_Particles(m_ParticlesLim)
End Sub



'-----------------------------------------------------------------------------
' Name: InitDeviceObjects
' Desc:
'-----------------------------------------------------------------------------
Sub InitDeviceObjects(dev As Direct3DDevice8)

    Dim v As CUSTOMVERTEX
    Dim j As Long, i As Long
    Dim indices() As Integer
    Dim indxbuffsize As Long
    
    ' Create the particle system's vertex buffer and index buffer.
    ' Each particle requires four vertices and 6 indices
    Set m_VertB = dev.CreateVertexBuffer(4 * m_MaxParticles * Len(v), D3DUSAGE_SOFTWAREPROCESSING, D3DFVF_COLORVERTEX, D3DPOOL_MANAGED)

    indxbuffsize = 6 * m_MaxParticles * 4   'each entry is 4 bytes (vb integer)
    Set m_IndxB = dev.CreateIndexBuffer(indxbuffsize, D3DUSAGE_SOFTWAREPROCESSING, D3DFMT_INDEX16, D3DPOOL_MANAGED)
    
    
    ' Fill the index buffer
    ReDim indices(6 * m_MaxParticles)   'we have 2 triangles per particle
    j = 0
    For i = 0 To m_MaxParticles - 1
        indices(j) = 4 * i + 0: j = j + 1
        indices(j) = 4 * i + 3: j = j + 1
        indices(j) = 4 * i + 1: j = j + 1
        indices(j) = 4 * i + 1: j = j + 1
        indices(j) = 4 * i + 3: j = j + 1
        indices(j) = 4 * i + 2: j = j + 1
    Next

    ' Set the data on the d3d buffer
    D3DIndexBuffer8SetData m_IndxB, 0, indxbuffsize, 0, indices(0)
    
End Sub

'-----------------------------------------------------------------------------
' Name: DeleteDeviceObjects
' Desc:
'-----------------------------------------------------------------------------
Sub DeleteDeviceObjects()
    Set m_VertB = Nothing
    Set m_IndxB = Nothing
End Sub

'-----------------------------------------------------------------------------
' Name: Class_Terminate
' Desc:
'-----------------------------------------------------------------------------
Private Sub Class_Terminate()
    DeleteDeviceObjects
    ReDim m_Particles(0)
End Sub



'-----------------------------------------------------------------------------
' Name:
' Desc:
'-----------------------------------------------------------------------------
Sub Update(fSecsPerFrame As Single, NumParticlesToEmit As Long, _
                                  clrEmitColor As D3DCOLORVALUE, _
                                  clrFadeColor As D3DCOLORVALUE, _
                                 fEmitVel As Single, _
                                  vPosition As D3DVECTOR)

    If m_binit = False Then Exit Sub
        
    
    Static fTime As Single
    Dim i As Long
    Dim iSpark As Long, inew As Long
    Dim fRand1  As Single, fRand2 As Single
    
    'advance simulation
    fTime = fTime + fSecsPerFrame
        
    
    Dim iCurrent As Long
    Dim iPrevious As Long
    iCurrent = m_iUsed
    iPrevious = 0
    
    'For each particle in the In Use list ...
    'calculate its age and test if the particle is too old
    
    Do While (iCurrent > 0)
        
        With m_Particles(iCurrent)
        
    
        
        Dim ft As Single
        Dim fGravity As Single
        
        'Calculate current lifetime
        ft = fTime - .m_fTime0
        
        'normal particle become sparks at the end of their
        'life time and have different behaviour
        'that we define here
        If .m_bSpark Then
        
            'sparks float higher and fade faster
            fGravity = -5#
            .m_fFade = .m_fFade - fSecsPerFrame * 2.25
            
        Else
            'other particles fall to ground faster but live longer
            fGravity = -9.8
                    
        End If


        'Our newposition computed from velocity and initial position
        'pNew=pInit+t*velocity + accl * t * t
        
        'the first terms
        .m_vPos.x = .m_vVel0.x * ft + .m_vPos0.x
        .m_vPos.y = .m_vVel0.y * ft + .m_vPos0.y
        .m_vPos.z = .m_vVel0.z * ft + .m_vPos0.z
        
        'we add gravity in for the accleration terms on y axis
        .m_vPos.y = .m_vPos.y + (0.5 * fGravity) * (ft * ft)
        
        'compute new Velocity given acceleartion
        'vNew=vInit+t*vCurrent
        .m_vVel.y = .m_vVel0.y + fGravity * ft
        
        'clamp fading to zero
        If (.m_fFade < 0#) Then .m_fFade = 0
            
        
        'Normal particles die and turn into 5 sparks when they are
        'above a certain height from the ground
        
        'Sparks die when they fall below the surface
        
        'We test here if any particle is dead

        If (.m_vPos.y < m_fRadius) Then  '

            'if we have a normal particle
            'lets turn it into 5 sparks
            If (Not .m_bSpark) Then
            
                For i = 0 To 4
            
                    'If there are particles in the free list, use them
                    If (m_iFree) Then
                        iSpark = m_iFree
                        m_iFree = m_Particles(m_iFree).iNext
            
                    'other wise get a new one
                    Else
                        If m_iLast >= m_ParticlesLim Then
'                           'm_bReset = True
                            Exit For
                        End If
                        m_iLast = m_iLast + 1
                        iSpark = m_iLast
                                                                                                
                    End If
            
                    'put this new particle on the used list
                    m_Particles(iSpark).iNext = m_iUsed
                    m_iUsed = iSpark
                    m_NumParticles = m_NumParticles + 1
            
                    'have the spark start out in the same position
                    'as where the normal particle is now
                    m_Particles(iSpark).m_bSpark = True
                    m_Particles(iSpark).m_vPos0 = .m_vPos
                    m_Particles(iSpark).m_vPos0.y = m_fRadius
            
                    fRand1 = Rnd(1) * g_pi * 2
                    fRand2 = Rnd(1) * g_pi * 0.25
            
                    'have the sparks velocity vere off from the normal particle
                    m_Particles(iSpark).m_vVel0.x = .m_vVel.x * 0.25 + Cos(fRand1) * Sin(fRand2)
                    m_Particles(iSpark).m_vVel0.z = .m_vVel.z * 0.25 + Sin(fRand1) * Sin(fRand2)
                    m_Particles(iSpark).m_vVel0.y = Cos(fRand2) * Rnd(1) * 1.5
            
                    'set the sparks current position = initial position
                    'set the sparks current velocitu = initial velocity
                    m_Particles(iSpark).m_vPos = .m_vPos0
                    m_Particles(iSpark).m_vVel = .m_vVel0
            
                    ' set the initial color of the particle to be that of
                    'what it was as a normal particle
                    D3DXColorLerp m_Particles(iSpark).m_clrDiffuse, .m_clrFade, .m_clrDiffuse, .m_fFade
            
                    'set the spark to fade to blue
                    m_Particles(iSpark).m_clrFade = ColorValue4(0#, 0#, 0#, 1#)
            
                    'set its life time indicator to be newly created
                    m_Particles(iSpark).m_fFade = 1#
            
                    'save the time of creation
                    m_Particles(iSpark).m_fTime0 = fTime
                Next
            End If

            ' Kill the current particle
            'remove it form used list
            'put it on free list
            If iPrevious > 0 Then
                m_Particles(iPrevious).iNext = .iNext
            Else
                m_iUsed = .iNext
            End If
            
            Dim iTemp As Long
            iTemp = .iNext
            .iNext = m_iFree
            m_iFree = iCurrent
            iCurrent = iTemp
            
            m_NumParticles = m_NumParticles - 1
                   
        Else
            iPrevious = iCurrent
            iCurrent = .iNext
    End If
    End With
    Loop

    ' Emit new particles
    Dim NumParticlesEmit As Long
    NumParticlesEmit = m_NumParticles + NumParticlesToEmit

    Do While (m_NumParticles < m_ParticlesLim And m_NumParticles < NumParticlesEmit)

        ' If there is a particle in the free list, use it
        If (m_iFree) Then
            inew = m_iFree
            m_iFree = m_Particles(m_iFree).iNext
        
         'other wise get an new one
        Else
            If m_iLast >= m_ParticlesLim Then
                Exit Do
            End If
            m_iLast = m_iLast + 1
            inew = m_iLast
            
        End If
        
        'put it on the used list
        'put this new particle on the used list
        m_Particles(inew).iNext = m_iUsed
        m_iUsed = inew
        
        m_NumParticles = m_NumParticles + 1

        ' Emit new particle
        
        fRand1 = Rnd(1) * g_pi * 2
        fRand2 = Rnd(1) * g_pi * 0.25

        With m_Particles(inew)
            .m_bSpark = False
            D3DXVec3Add .m_vPos0, vPosition, vec3(0, m_fRadius, 0)
            .m_vVel0.x = Cos(fRand1) * Sin(fRand2) * 2.5
            .m_vVel0.z = Sin(fRand1) * Sin(fRand2) * 2.5
            .m_vVel0.y = Cos(fRand2) * (Rnd(1) * fEmitVel)
            .m_vPos = .m_vPos0
            .m_vVel = .m_vVel0
            .m_clrDiffuse = clrEmitColor
            .m_clrFade = clrFadeColor
            .m_fFade = 1
            .m_fTime0 = fTime
        End With
    Loop
    
End Sub



 
'-----------------------------------------------------------------------------
' Name: Render()
' Desc: Renders the particle system using either pointsprites
'
'-----------------------------------------------------------------------------
Sub Render(dev As Direct3DDevice8)
    Dim v As CUSTOMVERTEX
    Dim iCurrent  As Long, i As Long
    
    With dev
    
        Dim DWFloat0 As Long
        Dim DWFloat1 As Long
        Dim DWFloatp08 As Long
        DWFloat0 = FtoDW(0)
        DWFloat1 = FtoDW(1)
        DWFloatp08 = FtoDW(0.08)
        ' Set the render states for using point sprites
        .SetRenderState D3DRS_POINTSPRITE_ENABLE, 1 'True
        .SetRenderState D3DRS_POINTSCALE_ENABLE, 1 'True
        .SetRenderState D3DRS_POINTSIZE, DWFloatp08
        .SetRenderState D3DRS_POINTSIZE_MIN, DWFloat0
        .SetRenderState D3DRS_POINTSCALE_A, DWFloat0
        .SetRenderState D3DRS_POINTSCALE_B, DWFloat0
        .SetRenderState D3DRS_POINTSCALE_C, DWFloat1
    
        ' Set up the vertex buffer to be rendered
        .SetStreamSource 0, m_VertB, Len(v)
        .SetVertexShader D3DFVF_COLORVERTEX
        
    End With
    
    Dim NumParticlesToRender  As Long
    
    ' Render each particle
    iCurrent = m_iUsed
    
    Dim vPos As D3DVECTOR, vVel As D3DVECTOR
    Dim fLengthSq As Single, steps As Long
    Do While (iCurrent <> 0)
    
      With m_Particles(iCurrent)
        vPos = .m_vPos
        vVel = .m_vVel
        fLengthSq = D3DXVec3LengthSq(vVel)
        
        
        If (fLengthSq < 1#) Then
            steps = 2
        ElseIf (fLengthSq < 4#) Then
            steps = 3
        ElseIf (fLengthSq < 9#) Then
            steps = 4
        ElseIf (fLengthSq < 12.25) Then
            steps = 5
        ElseIf (fLengthSq < 16#) Then
            steps = 6
        ElseIf (fLengthSq < 20.25) Then
            steps = 7
        Else
            steps = 8
        End If

        D3DXVec3Scale vVel, vVel, (-0.04 / steps)
        
        Dim clrDiffuse As D3DCOLORVALUE
        
        D3DXColorLerp clrDiffuse, .m_clrFade, .m_clrDiffuse, .m_fFade
        Dim clrDiffuseLong As Long
        clrDiffuseLong = D3DCOLORVALUEtoLONG(clrDiffuse)
        
        
        Dim iVert As Long
        
        ' Render each particle a bunch of times to get a blurring effect
        For i = 0 To steps - 1
            
            m_Verts(iVert).v = vPos
            m_Verts(iVert).color = clrDiffuseLong
                        
            NumParticlesToRender = NumParticlesToRender + 1
            iVert = iVert + 1
            If (NumParticlesToRender = m_MaxParticles) Then
            
                ' we have a full Vertex buffer
                D3DVertexBuffer8SetData m_VertB, 0, Len(v) * m_MaxParticles, 0, m_Verts(0)
                dev.DrawPrimitive D3DPT_POINTLIST, 0, NumParticlesToRender


                NumParticlesToRender = 0
                iVert = 0
            End If

            D3DXVec3Add vPos, vPos, vVel
            
        Next
        iCurrent = .iNext
       
      End With
        
    Loop
    
    ' Render any remaining particles
    If (NumParticlesToRender <> 0) Then
        D3DVertexBuffer8SetData m_VertB, 0, Len(v) * NumParticlesToRender, 0, m_Verts(0)
        g_dev.DrawPrimitive D3DPT_POINTLIST, 0, NumParticlesToRender
    End If

    ' Reset render states
    g_dev.SetRenderState D3DRS_POINTSPRITE_ENABLE, 0 'False
    g_dev.SetRenderState D3DRS_POINTSCALE_ENABLE, 0 'False
        
    
End Sub




'-----------------------------------------------------------------------------
' Name: RenderLights
' Desc:
'-----------------------------------------------------------------------------
Sub RenderLights(dev As Direct3DDevice8)


    Dim vTL As D3DVECTOR, vBL As D3DVECTOR, vBR As D3DVECTOR, vTR As D3DVECTOR
    Dim v As CUSTOMVERTEX
    
    vTL = vec3(-1, 0, 1):    vBL = vec3(-1, 0, -1)
    vBR = vec3(1, 0, -1):    vTR = vec3(1, 0, 1)

    With dev
        .SetStreamSource 0, m_VertB, Len(v)
        .SetVertexShader D3DFVF_COLORVERTEX
        .SetIndices m_IndxB, 0
    End With
    
    Dim iCurrent As Long
    
    Dim NumParticlesToRender
    Dim fY As Single
    Dim fSize As Single
    Dim clrDiffuse As D3DCOLORVALUE
    Dim clrDiffuseTemp As D3DCOLORVALUE
    Dim lngDiffuse As Long
    Dim vPos As D3DVECTOR
    Dim vTemp As D3DVECTOR
    Dim j As Long
    
    iCurrent = m_iUsed
    
    Do While (iCurrent <> 0)
    
        
      With m_Particles(iCurrent)
        fY = .m_vPos.y
        
        
        'if the particle is close to the ground we will add some lights effects
        If (fY < 1) Then
        
            'make sure particle cant go below ground
            If (fY < 0) Then fY = 0

            fSize = fY * 0.25 + m_fRadius

            
            D3DXColorLerp clrDiffuse, .m_clrFade, .m_clrDiffuse, .m_fFade
            D3DXColorScale clrDiffuseTemp, clrDiffuse, (1 - fY) * 0.5
            lngDiffuse = D3DCOLORVALUEtoLONG(clrDiffuseTemp)
            vPos = vec3(.m_vPos.x, 0#, .m_vPos.z)

            
            D3DXVec3Scale vTemp, vTR, fSize
            D3DXVec3Add m_Verts(j).v, vPos, vTemp
            m_Verts(j).color = lngDiffuse
            m_Verts(j).tu = 0: m_Verts(j).tv = 0
            j = j + 1
            
            D3DXVec3Scale vTemp, vBR, fSize
            D3DXVec3Add m_Verts(j).v, vPos, vTemp
            m_Verts(j).color = lngDiffuse
            m_Verts(j).tu = 0: m_Verts(j).tv = 1
            j = j + 1
            
            D3DXVec3Scale vTemp, vBL, fSize
            D3DXVec3Add m_Verts(j).v, vPos, vTemp
            m_Verts(j).color = lngDiffuse
            m_Verts(j).tu = 1: m_Verts(j).tv = 1
            j = j + 1
            
            D3DXVec3Scale vTemp, vTL, fSize
            D3DXVec3Add m_Verts(j).v, vPos, vTemp
            m_Verts(j).color = lngDiffuse
            m_Verts(j).tu = 1: m_Verts(j).tv = 0
            j = j + 1

            
            NumParticlesToRender = NumParticlesToRender + 1

            If (NumParticlesToRender = m_MaxParticles) Then
                D3DVertexBuffer8SetData m_VertB, 0, Len(v) * 4 * NumParticlesToRender, 0, m_Verts(0)
                
                dev.DrawIndexedPrimitive D3DPT_TRIANGLELIST, _
                                                  0, NumParticlesToRender * 4, _
                                                  0, NumParticlesToRender * 2

                NumParticlesToRender = 0
                j = 0
            End If
          
          
          
        End If
        
        iCurrent = .iNext
        
      End With
                
    Loop


    ' Render remaining particles
    If (NumParticlesToRender <> 0) Then
        D3DVertexBuffer8SetData m_VertB, 0, Len(v) * 4 * NumParticlesToRender, 0, m_Verts(0)
         
        dev.DrawIndexedPrimitive D3DPT_TRIANGLELIST, _
                                          0, NumParticlesToRender * 4, _
                                          0, NumParticlesToRender * 2
    End If
    
End Sub


