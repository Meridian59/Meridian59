VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Point Sprites"
   ClientHeight    =   4050
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   5055
   Icon            =   "PointSprites.frx":0000
   LinkTopic       =   "Form1"
   ScaleHeight     =   270
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   337
   StartUpPosition =   3  'Windows Default
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       PointSprites.frm
'  Content:    Sample showing how to use point sprites to do particle effects
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Option Explicit
Option Compare Text

Private Type CUSTOMVERTEX
    v As D3DVECTOR
    color As Long
    tu As Single
    tv As Single
End Type
Const D3DFVF_COLORVERTEX = (D3DFVF_XYZ Or D3DFVF_DIFFUSE Or D3DFVF_TEX1)

Const GROUND_GRIDSIZE = 8
Const GROUND_WIDTH = 256
Const GROUND_HEIGHT = 256
Const GROUND_TILE = 32
Const GROUND_COLOR = &HBBEEEEEE

Private Enum PARTICLE_COLORS
    COLOR_WHITE = 0
    COLOR_RED = 1
    COLOR_GREEN = 2
    COLOR_BLUE = 3
    NUM_COLORS = 4
End Enum

Dim g_clrColor(4) As D3DCOLORVALUE
Dim g_clrColorFade(4) As D3DCOLORVALUE

Dim m_media As String

Dim m_ParticleSystem As CParticle
Dim m_ParticleTexture As Direct3DTexture8
Dim m_NumParticlesToEmit As Long
Dim m_bStaticParticle As Boolean
Dim m_nParticleColor As Long

Dim m_GroundTexture As Direct3DTexture8
Dim m_NumGroundVertices As Long
Dim m_NumGroundIndices As Long
Dim m_GroundIB As Direct3DIndexBuffer8
Dim m_GroundVB As Direct3DVertexBuffer8
Dim m_planeGround As D3DPLANE

Dim m_bDrawReflection As Boolean
Dim m_bCanDoAlphaBlend  As Boolean
Dim m_bCanDoClipPlanes  As Boolean
Dim m_bDrawHelp As Boolean
    
Dim m_matView As D3DMATRIX
Dim m_matOrientation As D3DMATRIX

Dim m_vPosition As D3DVECTOR
Dim m_vVelocity As D3DVECTOR
Dim m_fYaw              As Single
Dim m_fYawVelocity      As Single
Dim m_fPitch            As Single
Dim m_fPitchVelocity    As Single

Dim m_fElapsedTime As Single

Dim m_bKey(256) As Boolean

Dim g_fTime As Single
Dim g_fLastTime As Single

Dim m_grVerts() As CUSTOMVERTEX
Dim m_grVerts2() As CUSTOMVERTEX

Dim m_binit As Boolean
Dim m_bMinimized As Boolean
Dim m_bStopSim As Boolean

Const kMaxParticles = 128
Const kParticleRadius = 0.01

'-----------------------------------------------------------------------------
' Name: Form_KeyPress()
' Desc:
'-----------------------------------------------------------------------------
Private Sub Form_KeyPress(KeyAscii As Integer)
    If Chr$(KeyAscii) = "r" Then m_bDrawReflection = Not m_bDrawReflection
End Sub

'-----------------------------------------------------------------------------
' Name: Form_Load()
' Desc:
'-----------------------------------------------------------------------------
Private Sub Form_Load()
    Me.Show
    DoEvents
    
    'setup defaults
    Init
    
    ' Initialize D3D
    ' Note: D3DUtil_Init will attempt to use D3D Hardware acceleartion.
    ' If it is not available it attempt to use the Software Reference Rasterizer.
    ' If all fail it will display a message box indicating so.
    '
    m_binit = D3DUtil_Init(Me.hwnd, True, 0, 0, D3DDEVTYPE_HAL, Me)
    If Not (m_binit) Then End

    ' find Media and set media path
    m_media = FindMediaDir("ground2.bmp")
    D3DUtil_SetMediaPath m_media
    
    ' Set initial state
    OneTimeSceneInit
    
    ' Load Mesh and textures from media
    InitDeviceObjects
    
    ' Set device render states, lighting, camera
    RestoreDeviceObjects

    ' Start Timer
    DXUtil_Timer TIMER_start
    
    ' Start our timer
    DXUtil_Timer TIMER_start
    
    ' Run the simulation forever
    ' See Form_Keydown for exit processing
    Do While True
    
        ' Increment the simulation
        FrameMove
        
        ' Render one image of the simulation
        If Render Then
            
            ' Present the image to the screen
            D3DUtil_PresentAll g_focushwnd
        End If
        
        ' Allow for events to get processed
        DoEvents
        
    Loop
    
End Sub

'-----------------------------------------------------------------------------
' Name: Form_KeyDown()
' Desc: Process key messages for exit and change device
'-----------------------------------------------------------------------------
Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
     
     m_bKey(KeyCode) = True
     
     Select Case KeyCode
        
        Case vbKeyEscape
            Unload Me
            
        Case vbKeyF2
                
            ' Pause the timer
            DXUtil_Timer TIMER_STOP
            m_bStopSim = True

            ' Bring up the device selection dialog
            ' we pass in the form so the selection process
            ' can make calls into InitDeviceObjects
            ' and RestoreDeviceObjects
            frmSelectDevice.SelectDevice Me
            
            ' Restart the timer
            m_bStopSim = False
            DXUtil_Timer TIMER_start
            
        Case vbKeyReturn
        
            ' Check for Alt-Enter if not pressed exit
            If Shift <> 4 Then Exit Sub
            
            ' stop simulation
            DXUtil_Timer TIMER_STOP
            m_bStopSim = True

            
            ' If we are windowed go fullscreen
            ' If we are fullscreen returned to windowed
            If g_d3dpp.Windowed Then
                 D3DUtil_ResetFullscreen
            Else
                 D3DUtil_ResetWindowed
            End If
                             
            ' Call Restore after ever mode change
            ' because calling reset looses state that needs to
            ' be reinitialized
            RestoreDeviceObjects
           
            ' Restart simulation
            DXUtil_Timer TIMER_STOP
            m_bStopSim = False
    
    End Select
End Sub

'-----------------------------------------------------------------------------
' Name: Form_KeyUp()
' Desc: Process key messages for exit and change device
'-----------------------------------------------------------------------------
Private Sub Form_KeyUp(KeyCode As Integer, Shift As Integer)
    m_bKey(KeyCode) = False
End Sub


Private Sub Form_MouseDown(Button As Integer, Shift As Integer, x As Single, y As Single)
    DXUtil_Timer (TIMER_STOP)
    m_bStopSim = True
End Sub

Private Sub Form_MouseUp(Button As Integer, Shift As Integer, x As Single, y As Single)
   DXUtil_Timer (TIMER_start)
    m_bStopSim = False

End Sub

'-----------------------------------------------------------------------------
' Name: Form_Resize()
' Desc: hadle resizing of the D3D backbuffer
'-----------------------------------------------------------------------------
Private Sub Form_Resize()

    ' If D3D is not initialized then exit
    If Not m_binit Then Exit Sub
    
    ' If we are in a minimized state stop the timer and exit
    If Me.WindowState = vbMinimized Then
        DXUtil_Timer TIMER_STOP
        m_bMinimized = True
        m_bStopSim = True
        Exit Sub
        
    ' If we just went from a minimized state to maximized
    ' restart the timer
    Else
        If m_bMinimized = True Then
            DXUtil_Timer TIMER_start
            m_bMinimized = False
            m_bStopSim = False
        End If
    End If
            
    ' Dont let the window get too small
    If Me.ScaleWidth < 10 Then
        Me.width = Screen.TwipsPerPixelX * 10
        Exit Sub
    End If
    
    If Me.ScaleHeight < 10 Then
        Me.height = Screen.TwipsPerPixelY * 10
        Exit Sub
    End If
    
    
    m_ParticleSystem.DeleteDeviceObjects
    Set m_ParticleSystem = Nothing
    Set m_ParticleSystem = New CParticle
    
    'reset and resize our D3D backbuffer to the size of the window
    D3DUtil_ResizeWindowed Me.hwnd
    
    'All state get losts after a reset so we need to reinitialze it here
    RestoreDeviceObjects
    DXUtil_Timer TIMER_STOP
    m_ParticleSystem.Init kMaxParticles, kParticleRadius
    m_ParticleSystem.InitDeviceObjects g_dev
    DXUtil_Timer TIMER_RESET
    
End Sub

'-----------------------------------------------------------------------------
' Name: Form_Unload()
' Desc:
'-----------------------------------------------------------------------------
Private Sub Form_Unload(Cancel As Integer)
    DeleteDeviceObjects
    End
End Sub

'-----------------------------------------------------------------------------
' Name: Init()
' Desc: Constructor
'-----------------------------------------------------------------------------
Sub Init()
    Me.Caption = "PointSprites: Using particle effects"
        
       
    
    Set m_ParticleSystem = New CParticle
    m_ParticleSystem.Init kMaxParticles, kParticleRadius
    
    Set m_ParticleTexture = Nothing
    
    m_NumParticlesToEmit = 10
    m_bStaticParticle = True
    m_nParticleColor = COLOR_WHITE

    Set m_GroundTexture = Nothing
    m_NumGroundVertices = (GROUND_GRIDSIZE + 1) * (GROUND_GRIDSIZE + 1)
    m_NumGroundIndices = (GROUND_GRIDSIZE * GROUND_GRIDSIZE) * 6
    Set m_GroundVB = Nothing
    Set m_GroundIB = Nothing
    m_planeGround = D3DPLANE4(0, 1, 0, 0)

    m_bDrawReflection = False
    m_bCanDoAlphaBlend = False
    m_bCanDoClipPlanes = False
    m_bDrawHelp = False
    

    m_vPosition = vec3(0, 3, -4)
    m_vVelocity = vec3(0, 0, 0)
    m_fYaw = 0
    m_fYawVelocity = 0
    m_fPitch = 0.5
    m_fPitchVelocity = 0


    g_clrColor(0) = ColorValue4(1, 1, 1, 1)
    g_clrColor(1) = ColorValue4(1, 0.5, 0.5, 1)
    g_clrColor(2) = ColorValue4(0.5, 1, 0.5, 1)
    g_clrColor(3) = ColorValue4(0.125, 0.5, 1, 1)



    g_clrColorFade(0) = ColorValue4(1, 0.25, 0.25, 1)
    g_clrColorFade(1) = ColorValue4(1, 0.25, 0.25, 1)
    g_clrColorFade(2) = ColorValue4(0.25, 0.75, 0.25, 1)
    g_clrColorFade(3) = ColorValue4(0.125, 0.25, 0.75, 1)


End Sub





'-----------------------------------------------------------------------------
' Name: OneTimeSceneInit()
' Desc: Called during initial app startup, this function performs all the
'       permanent initialization.
'-----------------------------------------------------------------------------
Sub OneTimeSceneInit()
    D3DXMatrixTranslation m_matView, 0, 0, 10
    D3DXMatrixTranslation m_matOrientation, 0, 0, 0
End Sub


'-----------------------------------------------------------------------------
' Name: FrameMove()
' Desc: Called once per frame, the call is the entry point for animating
'       the scene.
'-----------------------------------------------------------------------------

Sub FrameMove()
            

    If m_bStopSim = True Then Exit Sub
        

    g_fTime = DXUtil_Timer(TIMER_GETAPPTIME) * 1.3
    m_fElapsedTime = g_fTime - g_fLastTime
    g_fLastTime = g_fTime
    If m_fElapsedTime < 0 Then Exit Sub
        
    ' Slow things down for the REF device
    If (g_devType = D3DDEVTYPE_REF) Then m_fElapsedTime = 0.05

    Dim fSpeed As Single
    Dim fAngularSpeed
    
    fSpeed = 5 * m_fElapsedTime
    fAngularSpeed = 1 * m_fElapsedTime

    ' Slowdown the camera movement
    D3DXVec3Scale m_vVelocity, m_vVelocity, 0.9
    m_fYawVelocity = m_fYawVelocity * 0.9
    m_fPitchVelocity = m_fPitchVelocity * 0.9

    ' Process keyboard input
    If (m_bKey(vbKeyRight)) Then m_vVelocity.x = m_vVelocity.x + fSpeed        '  Slide Right
    If (m_bKey(vbKeyLeft)) Then m_vVelocity.x = m_vVelocity.x - fSpeed         '  Slide Left
    If (m_bKey(vbKeyUp)) Then m_vVelocity.y = m_vVelocity.y + fSpeed           '  Move up
    If (m_bKey(vbKeyDown)) Then m_vVelocity.y = m_vVelocity.y - fSpeed         '  Move down
    
    If (m_bKey(vbKeyW)) Then m_vVelocity.z = m_vVelocity.z + fSpeed            '  Move Forward
    If (m_bKey(vbKeyS)) Then m_vVelocity.z = m_vVelocity.z - fSpeed            '  Move Backward
    
    If (m_bKey(vbKeyE)) Then m_fYawVelocity = m_fYawVelocity + fSpeed          '  Yaw right
    If (m_bKey(vbKeyQ)) Then m_fYawVelocity = m_fYawVelocity - fSpeed          '  Yaw left
    
    If (m_bKey(vbKeyZ)) Then m_fPitchVelocity = m_fPitchVelocity + fSpeed      '  turn down
    If (m_bKey(vbKeyA)) Then m_fPitchVelocity = m_fPitchVelocity - fSpeed      '  turn up
    
    If (m_bKey(vbKeyAdd)) Then
        If (m_NumParticlesToEmit < 10) Then m_NumParticlesToEmit = m_NumParticlesToEmit + 1
    End If
    If (m_bKey(vbKeySubtract)) Then
        If (m_NumParticlesToEmit > 0) Then m_NumParticlesToEmit = m_NumParticlesToEmit - 1
    End If
    

    ' Update the position vector
    Dim vT As D3DVECTOR, vTemp As D3DVECTOR
    D3DXVec3Scale vTemp, m_vVelocity, fSpeed
    D3DXVec3Add vT, vT, vTemp
    D3DXVec3TransformNormal vT, vT, m_matOrientation
    
    D3DXVec3Add m_vPosition, m_vPosition, vT
    
    If (m_vPosition.y < 1) Then m_vPosition.y = 1

    ' Update the yaw-pitch-rotation vector
    m_fYaw = m_fYaw + fAngularSpeed * m_fYawVelocity
    m_fPitch = m_fPitch + fAngularSpeed * m_fPitchVelocity
    If (m_fPitch < 0) Then m_fPitch = 0
    If (m_fPitch > g_pi / 2) Then m_fPitch = g_pi / 2

    Dim qR As D3DQUATERNION, det As Single
    D3DXQuaternionRotationYawPitchRoll qR, m_fYaw, m_fPitch, 0
    D3DXMatrixAffineTransformation m_matOrientation, 1.25, vec3(0, 0, 0), qR, m_vPosition
    D3DXMatrixInverse m_matView, det, m_matOrientation

    ' Update particle system
    If (m_bStaticParticle) Then
        m_ParticleSystem.Update m_fElapsedTime, m_NumParticlesToEmit, _
                         g_clrColor(m_nParticleColor), _
                         g_clrColorFade(m_nParticleColor), 8, _
                         vec3(0, 0, 0)
    Else
        m_ParticleSystem.Update m_fElapsedTime, m_NumParticlesToEmit, _
                         g_clrColor(m_nParticleColor), _
                         g_clrColorFade(m_nParticleColor), 8, _
                         vec3(3 * Sin(g_fTime), 0, 3 * Cos(g_fTime))

    End If
    
End Sub



'-----------------------------------------------------------------------------
' Name: Render()
' Desc: Called once per frame, the call is the entry point for 3d
'       rendering. This function sets up render states, clears the
'       viewport, and renders the scene.
'-----------------------------------------------------------------------------
Function Render() As Boolean
    Dim v As CUSTOMVERTEX
    Dim hr As Long
    
     'See what state the device is in.
    Render = False
    hr = g_dev.TestCooperativeLevel
    If hr = D3DERR_DEVICENOTRESET Then
        g_dev.Reset g_d3dpp
        RestoreDeviceObjects
    End If
    
    'dont bother rendering if we are not ready yet
    If hr <> 0 Then Exit Function
    Render = True
    ' Clear the backbuffer
    D3DUtil_ClearAll &HFF&
    
    
    With g_dev
        .BeginScene
                
        
        ' Draw reflection of particles
        If (m_bDrawReflection) Then
            Dim matReflectedView As D3DMATRIX
            
            D3DXMatrixReflect matReflectedView, m_planeGround
            D3DXMatrixMultiply matReflectedView, matReflectedView, m_matView

            .SetTransform D3DTS_VIEW, matReflectedView
            'Dim clipplane As D3DCLIPPLANE
            'LSet clipplane = m_planeGround
            '.SetClipPlane 0, clipplane
            .SetRenderState D3DRS_CLIPPLANEENABLE, D3DCLIPPLANE0

            ' Draw particles
            .SetTexture 0, m_ParticleTexture
            .SetRenderState D3DRS_ZWRITEENABLE, 0 'FALSE
            .SetRenderState D3DRS_ALPHABLENDENABLE, 1 'TRUE

            m_ParticleSystem.Render g_dev

            .SetRenderState D3DRS_ALPHABLENDENABLE, 0 'False
            .SetRenderState D3DRS_ZWRITEENABLE, 1 'True

            .SetRenderState D3DRS_CLIPPLANEENABLE, 0 'FALSE

            .SetRenderState D3DRS_ALPHABLENDENABLE, 1 'True
            .SetRenderState D3DRS_SRCBLEND, D3DBLEND_SRCALPHA
            .SetRenderState D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA
            .SetTextureStageState 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE
        End If


        .SetRenderState D3DRS_ALPHABLENDENABLE, 0 'False
        .SetRenderState D3DRS_ZWRITEENABLE, 1 'True
        .SetRenderState D3DRS_CLIPPLANEENABLE, 0 'FALSE

        .SetRenderState D3DRS_ALPHABLENDENABLE, 1 '1 'True
        .SetRenderState D3DRS_SRCBLEND, D3DBLEND_SRCALPHA
        .SetRenderState D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA
        .SetTextureStageState 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE
        
        
        ' Draw ground
        .SetTransform D3DTS_VIEW, m_matView
        .SetTexture 0, m_GroundTexture
        .SetVertexShader D3DFVF_COLORVERTEX
        .SetStreamSource 0, m_GroundVB, Len(v)
        .SetIndices m_GroundIB, 0
        .DrawIndexedPrimitive D3DPT_TRIANGLELIST, _
                                            0, m_NumGroundVertices, _
                                            0, (m_NumGroundIndices / 3)

        ' Draw particles
        .SetRenderState D3DRS_ALPHABLENDENABLE, 1 'True
        .SetRenderState D3DRS_SRCBLEND, D3DBLEND_ONE
        .SetRenderState D3DRS_DESTBLEND, D3DBLEND_ONE
        .SetTextureStageState 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1

        .SetRenderState D3DRS_ZWRITEENABLE, 0 'False
        .SetRenderState D3DRS_ZENABLE, 1 'TRUE

        .SetTexture 0, m_ParticleTexture

        .SetRenderState D3DRS_ZENABLE, 0  'False

        .SetTexture 0, m_ParticleTexture
        m_ParticleSystem.Render g_dev

        .SetRenderState D3DRS_ALPHABLENDENABLE, 0 'False
        .SetRenderState D3DRS_ZWRITEENABLE, 1 'True


        .EndScene
    End With

End Function






'-----------------------------------------------------------------------------
' Name: InitDeviceObjects()
' Desc: Initialize scene objects.
'-----------------------------------------------------------------------------
Function InitDeviceObjects() As Boolean
    
    Dim i As Long
    Dim v As CUSTOMVERTEX
    
    Set m_GroundTexture = D3DUtil_CreateTexture(g_dev, "Ground2.bmp", D3DFMT_UNKNOWN)
    
    Set m_ParticleTexture = D3DUtil_CreateTexture(g_dev, "Particle.bmp", D3DFMT_UNKNOWN)

          
    ' Check if we can do the reflection effect
    m_bCanDoAlphaBlend = ((g_d3dCaps.SrcBlendCaps And D3DPBLENDCAPS_SRCALPHA) = D3DPBLENDCAPS_SRCALPHA) And _
                         ((g_d3dCaps.DestBlendCaps And D3DPBLENDCAPS_INVSRCALPHA) = D3DPBLENDCAPS_INVSRCALPHA)
    m_bCanDoClipPlanes = (g_d3dCaps.MaxUserClipPlanes >= 1)
    
    ' Note: all HW with Software Vertex Processing can do clipplanes
    m_bCanDoClipPlanes = True
        
    If (m_bCanDoAlphaBlend And m_bCanDoClipPlanes) Then m_bDrawReflection = True

    ' Create ground object
        
    ' Create vertex buffer for ground object
    Set m_GroundVB = g_dev.CreateVertexBuffer(m_NumGroundVertices * Len(v), _
                      0, D3DFVF_COLORVERTEX, D3DPOOL_MANAGED)
        

    ' Fill vertex buffer
    
     Dim zz As Long, xx As Long
     
     ReDim m_grVerts(GROUND_GRIDSIZE * GROUND_GRIDSIZE * 6)
     
     
     i = 0
     For zz = 0 To GROUND_GRIDSIZE
        For xx = 0 To GROUND_GRIDSIZE
            
            m_grVerts(i).v.x = GROUND_WIDTH * ((xx / GROUND_GRIDSIZE) - 0.5)
            m_grVerts(i).v.y = 0
            m_grVerts(i).v.z = GROUND_HEIGHT * ((zz / GROUND_GRIDSIZE) - 0.5)
            m_grVerts(i).color = GROUND_COLOR
            m_grVerts(i).tu = xx * (GROUND_TILE / GROUND_GRIDSIZE)
            m_grVerts(i).tv = zz * (GROUND_TILE / GROUND_GRIDSIZE)
            i = i + 1
        Next
    Next

    D3DVertexBuffer8SetData m_GroundVB, 0, Len(v) * (GROUND_GRIDSIZE + 1) * (GROUND_GRIDSIZE + 1), 0, m_grVerts(0)
    
    Dim vtx As Long
    Dim m_Indices() As Integer
    ReDim m_Indices(m_NumGroundIndices * 4)
    Dim z As Long, x As Long
    
    ' Create the index buffer
    Set m_GroundIB = g_dev.CreateIndexBuffer(m_NumGroundIndices * 2, _
                            0, _
                            D3DFMT_INDEX16, D3DPOOL_MANAGED)
            
    ' Fill in indices
    i = 0
    For z = 0 To GROUND_GRIDSIZE - 1
        For x = 0 To GROUND_GRIDSIZE - 1
                
                vtx = x + z * (GROUND_GRIDSIZE + 1)
                m_Indices(i) = vtx + 1: i = i + 1
                m_Indices(i) = vtx + 0: i = i + 1
                m_Indices(i) = vtx + 0 + (GROUND_GRIDSIZE + 1): i = i + 1
                m_Indices(i) = vtx + 1: i = i + 1
                m_Indices(i) = vtx + 0 + (GROUND_GRIDSIZE + 1): i = i + 1
                m_Indices(i) = vtx + 1 + (GROUND_GRIDSIZE + 1): i = i + 1
            
        Next
    Next
    
    D3DIndexBuffer8SetData m_GroundIB, 0, 2 * m_NumGroundIndices, 0, m_Indices(0)
    
    ' Initialize the particle system
    m_ParticleSystem.InitDeviceObjects g_dev
        
    InitDeviceObjects = True
End Function




'-----------------------------------------------------------------------------
' Name: VerifyDevice()
'-----------------------------------------------------------------------------
Function VerifyDevice(Behavior As Long, format As CONST_D3DFORMAT) As Boolean

    ' Make sure device can do ONE:ONE alphablending
    If (0 = (g_d3dCaps.SrcBlendCaps And D3DPBLENDCAPS_ONE) = D3DPBLENDCAPS_ONE) Then Exit Function
    If (0 = (g_d3dCaps.DestBlendCaps And D3DPBLENDCAPS_ONE) = D3DPBLENDCAPS_ONE) Then Exit Function
        
    ' We will run this app using software vertex processing
    If (Behavior = D3DCREATE_HARDWARE_VERTEXPROCESSING) Then Exit Function
    

    VerifyDevice = True
    
End Function



'-----------------------------------------------------------------------------
' Name: DeleteDeviceObjects()
' Desc: Called when the app is exitting, or the device is being changed,
'       this function deletes any device dependant objects.
'-----------------------------------------------------------------------------
Sub DeleteDeviceObjects()

    Set m_GroundTexture = Nothing
    Set m_ParticleTexture = Nothing

    Set m_GroundVB = Nothing
    Set m_GroundIB = Nothing

    If (m_ParticleSystem Is Nothing) Then Exit Sub
    m_ParticleSystem.DeleteDeviceObjects
    m_binit = False

End Sub





'-----------------------------------------------------------------------------
' Name: FinalCleanup()
' Desc: Called before the app exits, this function gives the app the chance
'       to cleanup after itself.
'-----------------------------------------------------------------------------
Sub FinalCleanup()
    
    Set m_GroundTexture = Nothing
    Set m_ParticleTexture = Nothing
    Set m_ParticleSystem = Nothing
End Sub


'-----------------------------------------------------------------------------
' Name: InvalidateDeviceObjects()
' Desc: Place code to release non managed objects here
'-----------------------------------------------------------------------------
Sub InvalidateDeviceObjects()
    'all objects are managed in this sample
End Sub

'-----------------------------------------------------------------------------
' Name: RestoreDeviceObjects()
' Desc:
'-----------------------------------------------------------------------------
Sub RestoreDeviceObjects()

    ' Set the world matrix
    Dim matWorld As D3DMATRIX
    D3DXMatrixIdentity matWorld
    g_dev.SetTransform D3DTS_WORLD, matWorld

    ' Set projection matrix
    Dim matProj As D3DMATRIX
    D3DXMatrixPerspectiveFovLH matProj, g_pi / 4, Me.ScaleHeight / Me.ScaleWidth, 0.1, 100
    g_dev.SetTransform D3DTS_PROJECTION, matProj

    ' Set renderstates
    With g_dev
        Call .SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR)
        Call .SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR)
        Call .SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR)
        Call .SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE)
        Call .SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1)
        Call .SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE)
        Call .SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE)
    
        Call .SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE)
        Call .SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE)
        Call .SetRenderState(D3DRS_LIGHTING, 0)     'FALSE
        Call .SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW)
        Call .SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT)
    End With

End Sub




