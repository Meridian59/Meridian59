VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Vertex Blend"
   ClientHeight    =   4485
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   5640
   Icon            =   "vertexshader.frx":0000
   LinkTopic       =   "Form1"
   ScaleHeight     =   299
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   376
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
'  File:       VertexShader.frm
'  Content:    Example code showing how to use vertex shaders in D3D.
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Option Explicit


' Scene
Dim m_VB As Direct3DVertexBuffer8
Dim m_IB As Direct3DIndexBuffer8
Dim m_NumVertices As Long
Dim m_NumIndices As Long
Dim m_Shader As Long
Dim m_Size As Long

' Transforms
Dim m_matPosition As D3DMATRIX
Dim m_matView As D3DMATRIX
Dim m_matProj As D3DMATRIX

'Navigation
Dim m_bKey(256) As Boolean
Dim m_fSpeed As Single
Dim m_fAngularSpeed As Single

Dim m_vVelocity As D3DVECTOR
Dim m_vAngularVelocity As D3DVECTOR

'Shader
Dim m_Decl(3) As Long
Dim m_ShaderArray() As Long

Dim m_bInit As Boolean                  ' Indicates that d3d has been initialized
Dim m_bMinimized As Boolean             ' Indicates that display window is minimized

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
    m_bInit = D3DUtil_Init(Me.hwnd, True, 0, 0, D3DDEVTYPE_HAL, Me)
    If Not (m_bInit) Then End
    
    
    ' Create new D3D vertexbuffer objects and vertex shader
    InitDeviceObjects
    
    ' Sets the state for those objects and the current D3D device
    RestoreDeviceObjects
    
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
' Name: Form_Unload()
' Desc:
'-----------------------------------------------------------------------------
Private Sub Form_Unload(Cancel As Integer)
    DeleteDeviceObjects
    End
End Sub



'-----------------------------------------------------------------------------
' Name: Init()
' Desc: Sets attributes for the app.
'-----------------------------------------------------------------------------
Sub Init()


    Me.Caption = "VertexShader"
    
    Set m_IB = Nothing
    Set m_VB = Nothing
    m_Size = 32
    m_NumIndices = (m_Size - 1) * (m_Size - 1) * 6
    m_NumVertices = m_Size * m_Size
    m_Shader = 0
    
    m_fSpeed = 5
    m_fAngularSpeed = 1

    m_vVelocity = vec3(0, 0, 0)
    m_vAngularVelocity = vec3(0, 0, 0)
    
    
    ' Setup the view matrix
    Dim veye As D3DVECTOR, vat As D3DVECTOR, vUp As D3DVECTOR
    veye = vec3(2, 3, 3)
    vat = vec3(0, 0, 0)
    vUp = vec3(0, 1, 0)
    D3DXMatrixLookAtRH m_matView, veye, vat, vUp

    ' Set the position matrix
    Dim det As Single
    D3DXMatrixInverse m_matPosition, det, m_matView

End Sub



'-----------------------------------------------------------------------------
' Name: FrameMove()
' Desc: Called once per frame, the call is the entry point for animating
'       the scene.
'-----------------------------------------------------------------------------
Sub FrameMove()
    Dim fSecsPerFrame As Single
    Dim fTime As Single
    Dim det As Single
    
    fSecsPerFrame = DXUtil_Timer(TIMER_GETELLAPSEDTIME)
    fTime = DXUtil_Timer(TIMER_GETAPPTIME)

    ' Process keyboard input
    Dim vT As D3DVECTOR, vR As D3DVECTOR
    
    vT = vec3(0, 0, 0)
    vR = vec3(0, 0, 0)

    
    If (m_bKey(vbKeyA) Or m_bKey(vbKeyNumpad1) Or m_bKey(vbKeyLeft)) Then vT.x = vT.x - 1  ' Slide Left
    If (m_bKey(vbKeyD) Or m_bKey(vbKeyNumpad3) Or m_bKey(vbKeyRight)) Then vT.x = vT.x + 1 ' Slide Right
    If (m_bKey(vbKeyDown)) Then vT.y = vT.y - 1                                      ' Slide Down
    If (m_bKey(vbKeyUp)) Then vT.y = vT.y + 1                                        ' Slide Up
    If (m_bKey(vbKeyW)) Then vT.z = vT.z - 2                                         ' Move Forward
    If (m_bKey(vbKeyS)) Then vT.z = vT.z + 2                                         ' Move Backward
    If (m_bKey(vbKeyNumpad8)) Then vR.x = vR.x - 1                                   ' Pitch Down
    If (m_bKey(vbKeyNumpad2)) Then vR.x = vR.x + 1                                   ' Pitch Up
    If (m_bKey(vbKeyE) Or m_bKey(vbKeyNumpad6)) Then vR.y = vR.y - 1                 ' Turn Right
    If (m_bKey(vbKeyQ) Or m_bKey(vbKeyNumpad4)) Then vR.y = vR.y + 1                 ' Turn Left
    If (m_bKey(vbKeyNumpad9)) Then vR.z = vR.z - 2                                   ' Roll CW
    If (m_bKey(vbKeyNumpad7)) Then vR.z = vR.z + 2                                   ' Roll CCW

    m_vVelocity.x = m_vVelocity.x * 0.9 + vT.x * 0.1
    m_vVelocity.y = m_vVelocity.y * 0.9 + vT.y * 0.1
    m_vVelocity.z = m_vVelocity.z * 0.9 + vT.z * 0.1
    m_vAngularVelocity.x = m_vAngularVelocity.x * 0.9 + vR.x * 0.1
    m_vAngularVelocity.y = m_vAngularVelocity.x * 0.9 + vR.y * 0.1
    m_vAngularVelocity.z = m_vAngularVelocity.x * 0.9 + vR.z * 0.1

    ' Update position and view matricies
    Dim matT As D3DMATRIX, matR As D3DMATRIX, qR As D3DQUATERNION
    
    D3DXVec3Scale vT, m_vVelocity, fSecsPerFrame * m_fSpeed
    D3DXVec3Scale vR, m_vAngularVelocity, fSecsPerFrame * m_fAngularSpeed
    

    D3DXMatrixTranslation matT, vT.x, vT.y, vT.z
    D3DXMatrixMultiply m_matPosition, matT, m_matPosition

    D3DXQuaternionRotationYawPitchRoll qR, vR.y, vR.x, vR.z
    D3DXMatrixRotationQuaternion matR, qR

    D3DXMatrixMultiply m_matPosition, matR, m_matPosition
    D3DXMatrixInverse m_matView, det, m_matPosition
    g_dev.SetTransform D3DTS_VIEW, m_matView

    ' Set up the vertex shader constants
    
    Dim mat As D3DMATRIX
    Dim vA As D3DVECTOR4, vD As D3DVECTOR4
    Dim vSin As D3DVECTOR4, vCos As D3DVECTOR4
    
    D3DXMatrixMultiply mat, m_matView, m_matProj
    D3DXMatrixTranspose mat, mat

    vA = vec4(Sin(fTime) * 15, 0, 0.5, 1)
    vD = vec4(g_pi, 1 / (2 * g_pi), 2 * g_pi, 0.05)

    ' Taylor series coefficients for sin and cos
    vSin = vec4(1, -1 / 6, 1 / 120, -1 / 5040)
    vCos = vec4(1, -1 / 2, 1 / 24, -1 / 720)

    g_dev.SetVertexShaderConstant 0, mat, 4
    g_dev.SetVertexShaderConstant 4, vA, 1
    g_dev.SetVertexShaderConstant 7, vD, 1
    g_dev.SetVertexShaderConstant 10, vSin, 1
    g_dev.SetVertexShaderConstant 11, vCos, 1
   

End Sub



'-----------------------------------------------------------------------------
' Name: Render()
' Desc: Called once per frame, the call is the entry point for 3d
'       rendering. This function sets up render states, clears the
'       viewport, and renders the scene.
'-----------------------------------------------------------------------------
Function Render() As Boolean
    Dim v2 As D3DVECTOR2
    Dim hr As Long
    
    Render = False
    'See what state the device is in.
    hr = g_dev.TestCooperativeLevel
    If hr = D3DERR_DEVICENOTRESET Then
        g_dev.Reset g_d3dpp
        RestoreDeviceObjects
    End If
    
    'dont bother rendering if we are not ready yet
    If hr <> 0 Then Exit Function
    Render = True
    
    'Clear the scene
    D3DUtil_ClearAll &HFF&
    
    With g_dev
    
        ' Begin the scene
        .BeginScene
        
        .SetVertexShader m_Shader
        .SetStreamSource 0, m_VB, Len(v2)
        .SetIndices m_IB, 0
        
        .DrawIndexedPrimitive D3DPT_TRIANGLELIST, 0, m_NumVertices, _
                                            0, m_NumIndices / 3

        ' End the scene.
        .EndScene
    
    End With

End Function

'-----------------------------------------------------------------------------
' Name: RestoreDeviceObjects()
' Desc: Initialize scene objects.
'-----------------------------------------------------------------------------
Sub InitDeviceObjects()

    Dim Indices() As Integer    'Integer are 4 bytes wide in VB
    Dim Vertices() As D3DVECTOR2
    Dim v As D3DVECTOR2, x As Integer, y As Integer, i As Integer
    
            
    ' Fill in our index array with triangles indices to make a grid
    ReDim Indices(m_NumIndices)
    For y = 1 To m_Size - 1
        For x = 1 To m_Size - 1
            Indices(i) = (y - 1) * m_Size + (x - 1): i = i + 1
            Indices(i) = (y - 0) * m_Size + (x - 1): i = i + 1
            Indices(i) = (y - 1) * m_Size + (x - 0): i = i + 1
            Indices(i) = (y - 1) * m_Size + (x - 0): i = i + 1
            Indices(i) = (y - 0) * m_Size + (x - 1): i = i + 1
            Indices(i) = (y - 0) * m_Size + (x - 0): i = i + 1
        Next
    Next
    
    ' Create index buffer and copy the VB array into it
    Set m_IB = g_dev.CreateIndexBuffer(m_NumIndices * 2, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED)
    D3DIndexBuffer8SetData m_IB, 0, m_NumIndices * 2, 0, Indices(0)

    i = 0
        
    'Fill our vertex array with the coordinates of our grid
    ReDim Vertices(m_NumVertices)
    For y = 0 To m_Size - 1
        For x = 0 To m_Size - 1
            Vertices(i) = vec2(((CSng(x) / CSng(m_Size - 1)) - 0.5) * g_pi, _
                            ((CSng(y) / CSng(m_Size - 1)) - 0.5) * g_pi)
                           
            i = i + 1
        Next
    Next
    

    ' Create a vertex buffer and copy our vertex array into it
    Set m_VB = g_dev.CreateVertexBuffer(m_NumVertices * Len(v), 0, 0, D3DPOOL_MANAGED)
    D3DVertexBuffer8SetData m_VB, 0, m_NumVertices * Len(v), 0, Vertices(0)
    
    
    
    ' Create vertex shader
    Dim strVertexShaderPath As String
    Dim VShaderCode As D3DXBuffer
    
    
    
    m_Decl(0) = D3DVSD_STREAM(0)
    m_Decl(1) = D3DVSD_REG(D3DVSDE_POSITION, D3DVSDT_FLOAT2)
    m_Decl(2) = D3DVSD_END()
        

    ' Find the vertex shader file
    strVertexShaderPath = FindMediaDir("ripple.vsh") + "ripple.vsh"
    
    'Assemble the vertex shader from the file
    Set VShaderCode = g_d3dx.AssembleShaderFromFile(strVertexShaderPath, 0, "", Nothing)
            
    'Move VShader code into an array
    ReDim m_ShaderArray(VShaderCode.GetBufferSize() / 4)
    g_d3dx.BufferGetData VShaderCode, 0, 1, VShaderCode.GetBufferSize(), m_ShaderArray(0)

    Set VShaderCode = Nothing

   
End Sub



'-----------------------------------------------------------------------------
' Name: RestoreDeviceObjects()
' Desc: Initialize scene objects.
'-----------------------------------------------------------------------------
Sub RestoreDeviceObjects()

    Dim bufferdesc As D3DSURFACE_DESC
    g_dev.GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO).GetDesc bufferdesc
    
    ' Set up right handed projection matrix
    Dim fAspectRatio As Single
    fAspectRatio = bufferdesc.width / bufferdesc.height
    D3DXMatrixPerspectiveFovRH m_matProj, 60 * g_pi / 180, fAspectRatio, 0.1, 100
    g_dev.SetTransform D3DTS_PROJECTION, m_matProj

    ' Setup render states
    g_dev.SetRenderState D3DRS_LIGHTING, 0 'FALSE
    g_dev.SetRenderState D3DRS_CULLMODE, D3DCULL_NONE
    
     
    ' Create the vertex shader
    ' NOTE returns value in m_Shader
    g_dev.CreateVertexShader m_Decl(0), m_ShaderArray(0), m_Shader, 0
    

End Sub



'-----------------------------------------------------------------------------
' Name: InvalidateDeviceObjects()
' Desc:
'-----------------------------------------------------------------------------
Sub InvalidateDeviceObjects()
    On Local Error Resume Next
    g_dev.DeleteVertexShader m_Shader
End Sub



'-----------------------------------------------------------------------------
' Name: DeleteDeviceObjects()
' Desc: Called when the app is exitting, or the device is being changed,
'       this function deletes any device dependant objects.
'-----------------------------------------------------------------------------
Sub DeleteDeviceObjects()
    Set m_IB = Nothing
    Set m_VB = Nothing
    InvalidateDeviceObjects
    m_bInit = False
End Sub


'-----------------------------------------------------------------------------
' Name: FinalCleanup()
' Desc: Called before the app exits, this function gives the app the chance
'       to cleanup after itself.
'-----------------------------------------------------------------------------
Sub FinalCleanup()

End Sub


'-----------------------------------------------------------------------------
' Name: ConfirmDevice()
' Desc: Called during device intialization, this code checks the device
'       for some minimum set of capabilities
'-----------------------------------------------------------------------------
Function VerifyDevice(Behavior As Long, format As CONST_D3DFORMAT) As Boolean

    If (Behavior <> D3DCREATE_SOFTWARE_VERTEXPROCESSING) Then
        If (g_d3dCaps.VertexShaderVersion < D3DVS_VERSION(1, 0)) Then Exit Function
    End If
    VerifyDevice = True
End Function



'-----------------------------------------------------------------------------
' Name: Form_KeyDown()
' Desc: Process key messages for exit and change device
'-----------------------------------------------------------------------------
Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
     Dim hr As Long
     
     m_bKey(KeyCode) = True
    
     Select Case KeyCode
        
        Case vbKeyEscape
            Unload Me
            
        Case vbKeyF2
                
            ' Pause the timer
            DXUtil_Timer TIMER_STOP
            
            ' Bring up the device selection dialog
            ' we pass in the form so the selection process
            ' can make calls into InitDeviceObjects
            ' and RestoreDeviceObjects
            frmSelectDevice.SelectDevice Me
            
            ' Restart the timer
            DXUtil_Timer TIMER_start
            
        Case vbKeyReturn
        
            ' Check for Alt-Enter if not pressed exit
            If Shift <> 4 Then Exit Sub
            
            ' If we are windowed go fullscreen
            ' If we are fullscreen returned to windowed
            If g_d3dpp.Windowed Then
                 hr = D3DUtil_ResetFullscreen
            Else
                 hr = D3DUtil_ResetWindowed
            End If
                                                          
            If hr = D3DERR_DEVICELOST Then
                
                DeleteDeviceObjects
                
                m_bInit = D3DUtil_Init(Me.hwnd, True, 0, 0, D3DDEVTYPE_HAL, Me)
                If Not (m_bInit) Then End
                
                InitDeviceObjects
            End If
            
            ' Call Restore after ever mode change
            ' because calling reset looses state that needs to
            ' be reinitialized
            RestoreDeviceObjects
           
    End Select
End Sub


'-----------------------------------------------------------------------------
' Name: Form_Resize()
' Desc: hadle resizing of the D3D backbuffer
'-----------------------------------------------------------------------------
Private Sub Form_Resize()

    ' If D3D is not initialized then exit
    If Not m_bInit Then Exit Sub
    
    ' If we are in a minimized state stop the timer and exit
    If Me.WindowState = vbMinimized Then
        DXUtil_Timer TIMER_STOP
        m_bMinimized = True
        Exit Sub
        
    ' If we just went from a minimized state to maximized
    ' restart the timer
    Else
        If m_bMinimized = True Then
            DXUtil_Timer TIMER_start
            m_bMinimized = False
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

        
    'reset and resize our D3D backbuffer to the size of the window
    D3DUtil_ResizeWindowed Me.hwnd
    
    'All state get losts after a reset so we need to reinitialze it here
    RestoreDeviceObjects
    
End Sub


'-----------------------------------------------------------------------------
' Name: Picture1_KeyUp
' Desc:
'-----------------------------------------------------------------------------
Private Sub Form_KeyUp(KeyCode As Integer, Shift As Integer)
    m_bKey(KeyCode) = False
End Sub
