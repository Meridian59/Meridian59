VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Vertex Blend"
   ClientHeight    =   4485
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   5640
   Icon            =   "vertexblend.frx":0000
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
'-----------------------------------------------------------------------------
' File: VertexBlend.frm
'
' Desc: Example code showing how to do a skinning effect, using the vertex
'       blending feature of Direct3D. Normally, Direct3D transforms each
'       vertex through the world matrix. The vertex blending feature,
'       however, uses mulitple world matrices and a per-vertex blend factor
'       to transform each vertex.
'
' Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
'-----------------------------------------------------------------------------


Option Explicit

Const D3DVBF_DISABLE = 0    '     Disable vertex blending
Const D3DVBF_1WEIGHTS = 1   '     2 matrix blending
Const D3DVBF_2WEIGHTS = 2   '     3 matrix blending
Const D3DVBF_3WEIGHTS = 3   '     4 matrix blending
Const D3DVBF_0WEIGHTS = 256 '     one matrix is used with weight 1.0

'-----------------------------------------------------------------------------
' Name: struct D3DBLENDVERTEX
' Desc: Custom vertex which includes a blending factor
'-----------------------------------------------------------------------------
Private Type D3DBLENDVERTEX

    v As D3DVECTOR
    blend As Single
    n As D3DVECTOR
    tu As Single
    tv As Single
    
End Type

Const D3DFVF_BLENDVERTEX = (D3DFVF_XYZB1 Or D3DFVF_NORMAL Or D3DFVF_TEX1)

Dim m_Object As CD3DMesh
Dim m_matUpperArm As D3DMATRIX
Dim m_matLowerArm As D3DMATRIX
Dim m_mediadir As String
Dim g_ftime As Single

Dim m_bInit As Boolean                  ' Indicates that d3d has been initialized
Dim m_bMinimized As Boolean             ' Indicates that display window is minimized


'-----------------------------------------------------------------------------
' Name: Form_Load()
' Desc:
'-----------------------------------------------------------------------------
Private Sub Form_Load()
    
    
    ' Show the form
    Me.Show
    DoEvents

    
    Me.Caption = "VertexBlend: Surface Skinning Example"
        

    
    ' Initialize D3D
    ' Note: D3DUtil_Init will attempt to use D3D Hardware acceleartion.
    ' If it is not available it attempt to use the Software Reference Rasterizer.
    ' If all fail it will display a message box indicating so.
    '
    m_bInit = D3DUtil_Init(Me.hwnd, True, 0, 0, D3DDEVTYPE_HAL, Nothing)
    If Not (m_bInit) Then End
            
    
    ' Find and set the path to our media
    m_mediadir = FindMediaDir("mslogo.x")
    D3DUtil_SetMediaPath m_mediadir
    
    ' Create new D3D mesh objects and loads content from disk
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
' Name: Form_KeyDown()
' Desc: Process key messages for exit and change device
'-----------------------------------------------------------------------------
Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
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
                 D3DUtil_ResetFullscreen
            Else
                 D3DUtil_ResetWindowed
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
    g_lWindowWidth = Me.ScaleWidth
    g_lWindowHeight = Me.ScaleHeight
    RestoreDeviceObjects
    
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
' Name: FrameMove()
' Desc: Called once per frame, the call is the entry point for animating
'       the scene.
'-----------------------------------------------------------------------------
Sub FrameMove()
    g_ftime = DXUtil_Timer(TIMER_GETAPPTIME)

    ' Set the vertex blending matrices for this frame
    D3DXMatrixIdentity m_matUpperArm
    Dim vAxis As D3DVECTOR
    vAxis = vec3(2 + Sin(g_ftime * 3.1), 2 + Sin(g_ftime * 3.3), Sin(g_ftime * 3.5))
    
    D3DXMatrixRotationAxis m_matLowerArm, vAxis, Sin(3 * g_ftime)

End Sub



'-----------------------------------------------------------------------------
' Name: Render()
' Desc: Called once per frame, the call is the entry point for 3d
'       rendering. This function sets up render states, clears the
'       viewport, and renders the scene.
'-----------------------------------------------------------------------------
Function Render() As Boolean
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

    ' Clear the backbuffer
    D3DUtil_ClearAll &HFF&
    
    With g_dev
        .BeginScene
        
    
        ' Enable vertex blending
        .SetRenderState D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS
        .SetTransform D3DTS_WORLD, m_matUpperArm
        .SetTransform D3DTS_WORLD1, m_matLowerArm
        
        ' Display the object
        m_Object.Render g_dev

        ' End the scene.
        .EndScene
    End With

End Function



'-----------------------------------------------------------------------------
' Name: InitDeviceObjects()
' Desc: Initialize scene objects.
'-----------------------------------------------------------------------------
Function InitDeviceObjects() As Boolean
    
    Dim b As Boolean
    
    Set m_Object = New CD3DMesh
    
    b = m_Object.InitFromFile(g_dev, m_mediadir + "mslogo.x")
    
    If Not b Then
        MsgBox "media not found"
        End
    End If
    
    
    ' Set a custom FVF for the mesh
    m_Object.SetFVF g_dev, D3DFVF_BLENDVERTEX
    
    Dim VertB As Direct3DVertexBuffer8
    Dim Vertices() As D3DBLENDVERTEX
    Dim NumVertices As Long
    Dim MinX As Single
    Dim MaxX As Single
    Dim a As Single
    Dim i As Long
    
    NumVertices = m_Object.mesh.GetNumVertices()
    Set VertB = m_Object.mesh.GetVertexBuffer()
    
    
    MinX = 10000000000#
    MaxX = -10000000000#
    
    ReDim Vertices(NumVertices)
    
    'copy data into our own array
    D3DVertexBuffer8GetData VertB, 0, NumVertices * Len(Vertices(0)), 0, Vertices(0)
    
    
    ' Calculate the min/max z values for all the vertices
    For i = 0 To NumVertices - 1
        If Vertices(i).v.x < MinX Then MinX = Vertices(i).v.x
        If Vertices(i).v.x > MaxX Then MaxX = Vertices(i).v.x
    Next
    
    ' Set the blend factors for the vertices
    
    For i = 0 To NumVertices - 1
        a = (Vertices(i).v.x - MinX) / (MaxX - MinX)
        Vertices(i).blend = 1 - Sin(a * g_pi * 1)
    Next
    

    D3DVertexBuffer8SetData VertB, 0, NumVertices * Len(Vertices(0)), 0, Vertices(0)
    
    Set VertB = Nothing
    
    InitDeviceObjects = True
    
    
End Function

'-----------------------------------------------------------------------------
' Name: RestoreDeviceObjects()
' Desc: Restore device-memory objects and state after a device is created or
'       resized.
'-----------------------------------------------------------------------------
Sub RestoreDeviceObjects()

    ' Restore mesh's local memory objects
    m_Object.RestoreDeviceObjects g_dev

    ' Set miscellaneous render states
    With g_dev
        .SetRenderState D3DRS_ZENABLE, 1  'TRUE
        .SetRenderState D3DRS_AMBIENT, &H444444
        

        ' Set the projection matrix
        Dim matProj As D3DMATRIX
        D3DXMatrixPerspectiveFovLH matProj, g_pi / 4, Me.ScaleHeight / Me.ScaleWidth, 1#, 10000#
        .SetTransform D3DTS_PROJECTION, matProj

        ' Set the app view matrix for normal viewing
        Dim vEyePt As D3DVECTOR, vLookatPt As D3DVECTOR, vUpVec As D3DVECTOR
        Dim matView As D3DMATRIX
        vEyePt = vec3(0#, -5#, -10#)
        vLookatPt = vec3(0#, 0#, 0#)
        vUpVec = vec3(0#, 1#, 0#)
        D3DXMatrixLookAtLH matView, vEyePt, vLookatPt, vUpVec
        .SetTransform D3DTS_VIEW, matView

        ' Create a directional light
        Dim light As D3DLIGHT8
        D3DUtil_InitLight light, D3DLIGHT_DIRECTIONAL, 1, -1, 1
        .SetLight 1, light
        .LightEnable 1, 1  'True
        .SetRenderState D3DRS_LIGHTING, 1 'TRUE
        
        
    End With
    
End Sub


'-----------------------------------------------------------------------------
' Name: InvalidateDeviceObjects()
' Desc: Called when the device-dependant objects are about to be lost.
'-----------------------------------------------------------------------------
Sub InvalidateDeviceObjects()
    m_Object.InvalidateDeviceObjects
End Sub


'-----------------------------------------------------------------------------
' Name: DeleteDeviceObjects()
' Desc: Called when the app is exitting, or the device is being changed,
'       this function deletes any device dependant objects.
'-----------------------------------------------------------------------------
Sub DeleteDeviceObjects()
    m_Object.Destroy
    Set m_Object = Nothing
    m_bInit = False
End Sub

