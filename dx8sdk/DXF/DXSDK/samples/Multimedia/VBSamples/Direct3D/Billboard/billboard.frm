VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   4290
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   5580
   Icon            =   "billboard.frx":0000
   LinkTopic       =   "Form1"
   ScaleHeight     =   286
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   372
   StartUpPosition =   3  'Windows Default
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'-----------------------------------------------------------------------------
' File: Billboard.frm
'
' Desc: Example code showing how to do billboarding. The sample uses
'       billboarding to draw some trees.
'
'       Note: this implementation is for billboards that are fixed to rotate
'       about the Y-axis, which is good for things like trees. For
'       unconstrained billboards, like explosions in a flight sim, the
'       technique is the same, but the the billboards are positioned slightly
'       different. Try using the inverse of the view matrix, TL-vertices, or
'       some other technique.
'
' Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
'-----------------------------------------------------------------------------

Option Explicit

'-----------------------------------------------------------------------------
' Defines, constants, and global variables
'-----------------------------------------------------------------------------
Const NUM_TREES = 200
Const D3DFVF_TREEVERTEX = (D3DFVF_XYZ Or D3DFVF_DIFFUSE Or D3DFVF_TEX1)
Const NUMTREETEXTURES = 3

' Custom vertex type for the trees
Private Type TREEVERTEX
    p As D3DVECTOR
    color As Long
    tu As Single
    tv As Single
End Type

Dim m_bInit As Boolean                  ' Indicates that d3d has been initialized
Dim m_bMinimized As Boolean             ' Indicates that display window is minimized


'-----------------------------------------------------------------------------
' Name: Tree
' Desc: Simple structure to hold data for rendering a tree
'-----------------------------------------------------------------------------
Private Type TREE
    v(3) As TREEVERTEX
    vPos As D3DVECTOR
    iTreeTexture As Long
    iNext As Long
    dist As Single
End Type

Private Type HILLVERTEX
    x As Single
    y As Single
    z As Single
    tu As Single
    tv As Single
End Type
    

Dim m_vEyePt As D3DVECTOR
Dim m_strTreeTextures(3) As String
Dim m_media As String

Dim m_Terrain As CD3DMesh
Dim m_SkyBox  As CD3DMesh              ' Skybox background object
Dim m_TreeVB As Direct3DVertexBuffer8  ' Vertex buffer for rendering a tree
Dim m_TreeTextures(NUMTREETEXTURES)    ' Tree images
Dim m_matBillboardMatrix As D3DMATRIX   ' Used for billboard orientation
Dim m_Trees(NUM_TREES)  As TREE                ' Array of tree info
Dim m_fTime As Single
Dim m_iTreeHead As Long
Dim m_iSortHead As Long

'-----------------------------------------------------------------------------
' Name: Form_Load()
' Desc:
'-----------------------------------------------------------------------------
Private Sub Form_Load()
    Me.Show
    DoEvents
    
    'Setup defaults
    Init
    
     ' Initialize D3D
    ' Note: D3DUtil_Init will attempt to use D3D Hardware acceleartion.
    ' If it is not available it attempt to use the Software Reference Rasterizer.
    ' If all fail it will display a message box indicating so.
    '
    m_bInit = D3DUtil_Init(Me.hwnd, True, 0, 0, D3DDEVTYPE_HAL, Nothing)
    If Not (m_bInit) Then End

    ' Find media and set media directory
    m_media = FindMediaDir("Tree02S.tga")
    D3DUtil_SetMediaPath m_media
    
    ' Initialize Application Data
    OneTimeSceneInit
    
    ' Create and load mesh objects
    InitDeviceObjects
    
    ' Sets the state for those objects and the current D3D device
    ' (setup camera and lights etc)
    RestoreDeviceObjects
    
    ' Start application timer
    DXUtil_Timer TIMER_start
    
    ' Run the simulation forever
    ' See Form_Keydown for exit processing
    Do While True
    
        ' Increment the simulation
        FrameMove
        
        ' Render one image of the simulation
        If Render Then 'Success
            
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
     
     Dim hr As Long
     
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
                             
            ' Call Restore after ever mode change
            ' because calling reset looses state that needs to
            ' be reinitialized
            If (hr = D3D_OK) Then
                RestoreDeviceObjects
            End If
           
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
' Name: Form_Unload()
' Desc:
'-----------------------------------------------------------------------------
Private Sub Form_Unload(Cancel As Integer)
    DeleteDeviceObjects
    End
End Sub

' Simple function to define "hilliness" for terrain
Function HeightField(x As Single, y As Single) As Single
    HeightField = 9 * (Cos(x / 20 + 0.2) * Cos(y / 15 - 0.2) + 1#)
End Function

Sub Init()

    m_strTreeTextures(0) = "Tree02S.tga"
    m_strTreeTextures(1) = "Tree35S.tga"
    m_strTreeTextures(2) = "Tree01S.tga"

    Me.Caption = ("Billboard: D3D Billboarding Example")
    
    Set m_SkyBox = New CD3DMesh
    Set m_Terrain = New CD3DMesh
    Set m_TreeVB = Nothing

End Sub



'-----------------------------------------------------------------------------
' Name: OneTimeSceneInit()
' Desc: Called during initial app startup, this function performs all the
'       permanent initialization.
'-----------------------------------------------------------------------------
Sub OneTimeSceneInit()
    Dim i As Long
    Dim fTheta As Single, fRadius As Single, fWidth As Single, fHeight As Single
    Dim r As Long, g As Long, b As Long, treecolor As Long
    Rnd (1)
    
    ' Initialize the tree data
    For i = 0 To NUM_TREES - 1

        ' Position the trees randomly
        fTheta = 2 * g_pi * Rnd()
        fRadius = 25 + 55 * Rnd()
        m_Trees(i).vPos.x = fRadius * Sin(fTheta)
        m_Trees(i).vPos.z = fRadius * Cos(fTheta)
        m_Trees(i).vPos.y = HeightField(m_Trees(i).vPos.x, m_Trees(i).vPos.z)

        ' Size the trees randomly
        fWidth = 1 + 0.2 * (Rnd() - Rnd())
        fHeight = 1.4 + 0.4 * (Rnd() - Rnd())

        ' Each tree is a random color between red and green
         r = (255 - 190) + CLng(190 * Rnd())
         g = (255 - 190) + CLng(190 * Rnd())
         b = 0
         treecolor = &HFF000000 + r * 2 ^ 16 + g * 2 ^ 8 + b

        m_Trees(i).v(0).p = vec3(-fWidth, 0 * fHeight, 0)
        m_Trees(i).v(0).color = treecolor
        m_Trees(i).v(0).tu = 0: m_Trees(i).v(0).tv = 1
        m_Trees(i).v(1).p = vec3(-fWidth, 2 * fHeight, 0)
        m_Trees(i).v(1).color = treecolor
        m_Trees(i).v(1).tu = 0: m_Trees(i).v(1).tv = 0
        m_Trees(i).v(2).p = vec3(fWidth, 0 * fHeight, 0)
        m_Trees(i).v(2).color = treecolor
        m_Trees(i).v(2).tu = 1:      m_Trees(i).v(2).tv = 1
        m_Trees(i).v(3).p = vec3(fWidth, 2 * fHeight, 0)
        m_Trees(i).v(3).color = treecolor
        m_Trees(i).v(3).tu = 1:      m_Trees(i).v(3).tv = 0

        ' Size the trees randomly
        m_Trees(i).iTreeTexture = CLng((NUMTREETEXTURES - 1) * Rnd())
        m_Trees(i).iNext = i + 1
    Next
    
    m_Trees(NUM_TREES - 1).iNext = -1  'use -1 to indicate end of the list
    
End Sub


'-----------------------------------------------------------------------------
' Name: Sort
' Desc: Callback function for sorting trees in back-to-front order
'-----------------------------------------------------------------------------
Sub DoSort()
    Dim i As Long
    Dim dx As Single, dz As Single, dist As Single
    
    'calculate the square of the distance to the eyept
    'to best approximate sort order
    'CONSIDER transforming the position into screen space and sorting on z/w
    For i = 0 To NUM_TREES - 1
        dx = m_Trees(i).vPos.x - m_vEyePt.x
        dz = m_Trees(i).vPos.z - m_vEyePt.z
        m_Trees(i).dist = dx * dx + dz * dz
    Next
    
    Dim iAtU As Long
    Dim iPrevU As Long
    Dim iNextU As Long
    
    iAtU = m_iTreeHead
    iPrevU = -1
    iNextU = -1
    m_iSortHead = -1
    
    Dim z As Long
    Dim q As Long
        
    Do While iAtU <> -1
        dist = m_Trees(iAtU).dist
        
        iNextU = m_Trees(iAtU).iNext
        InsertIntoList iAtU, dist
                
        
        
        'advance to next item in Unsorted list
        iPrevU = iAtU
        iAtU = iNextU
        
    Loop
     
    m_iTreeHead = m_iSortHead


End Sub


Sub InsertIntoList(iNode As Long, dist2 As Single)
    
    
    
    Dim iAtS As Long
    Dim iPrevS As Long
            
    iAtS = m_iSortHead
    iPrevS = -1
    
    'If Sorted list is empty add first node
    If iAtS = -1 Then
        m_iSortHead = iNode
        m_Trees(iNode).iNext = -1
        Exit Sub
    End If
    
    
    'see if we need to add at begining
    If m_Trees(m_iSortHead).dist < dist2 Then
        m_Trees(iNode).iNext = m_iSortHead
        m_iSortHead = iNode
        Exit Sub
    End If
    
    'we dont have an empty list
    'we dont need to add to front of list
    Do While iAtS <> -1
        
        If m_Trees(iAtS).dist < dist2 Then
        
            'add to sorted list
            m_Trees(iNode).iNext = m_Trees(iPrevS).iNext
            m_Trees(iPrevS).iNext = iNode
            Exit Sub
        End If
                
        'advance to next item in  sorted list
        iPrevS = iAtS
        iAtS = m_Trees(iAtS).iNext
        
    Loop
    
    'must go at end of list
    m_Trees(iPrevS).iNext = iNode
    m_Trees(iNode).iNext = -1
    
    
End Sub




'-----------------------------------------------------------------------------
' Name: FrameMove()
' Desc: Called once per frame, the call is the entry point for animating
'       the scene.
'-----------------------------------------------------------------------------
Sub FrameMove()
    m_fTime = DXUtil_Timer(TIMER_GETAPPTIME)

    ' Get the eye and lookat points from the camera's path
    Dim vUpVec As D3DVECTOR, vEyePt As D3DVECTOR, vLookAtpt As D3DVECTOR
    vUpVec = vec3(0, 1, 0)
    
    vEyePt.x = 30 * Cos(0.8 * (m_fTime + 1))
    vEyePt.z = 30 * Sin(0.8 * (m_fTime + 1))
    vEyePt.y = 4 + HeightField(vEyePt.x, vEyePt.z)

    vLookAtpt.x = 30 * Cos(0.8 * (m_fTime + 1.5))
    vLookAtpt.z = 30 * Sin(0.8 * (m_fTime + 1.5))
    vLookAtpt.y = vEyePt.y - 1

    ' Set the app view matrix for normal viewing
    Dim matView As D3DMATRIX
    D3DXMatrixLookAtLH matView, vEyePt, vLookAtpt, vUpVec
    g_dev.SetTransform D3DTS_VIEW, matView

    ' Set up a rotation matrix to orient the billboard towards the camera.
    Dim vDir As D3DVECTOR
    D3DXVec3Subtract vDir, vLookAtpt, vEyePt
    
    If (vDir.x > 0) Then
        D3DXMatrixRotationY m_matBillboardMatrix, -Atn(vDir.z / vDir.x) + (g_pi / 2)
    Else
        D3DXMatrixRotationY m_matBillboardMatrix, -Atn(vDir.z / vDir.x) - (g_pi / 2)
    End If
    
    ' Sort trees in back-to-front order
    m_vEyePt = vEyePt
    
    
    DoSort

End Sub



'-----------------------------------------------------------------------------
' Name: DrawTrees()
' Desc:
'-----------------------------------------------------------------------------
Sub DrawTrees()
    Dim i As Long

    ' Set diffuse blending for alpha set in vertices.
    g_dev.SetRenderState D3DRS_ALPHABLENDENABLE, 1  'TRUE
    g_dev.SetRenderState D3DRS_SRCBLEND, D3DBLEND_SRCALPHA
    g_dev.SetRenderState D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA

    ' Enable alpha testing (skips pixels with less than a certain alpha.)
    If ((g_d3dCaps.AlphaCmpCaps And D3DPCMPCAPS_GREATEREQUAL) = D3DPCMPCAPS_GREATEREQUAL) Then
        g_dev.SetRenderState D3DRS_ALPHATESTENABLE, 1 'TRUE
        g_dev.SetRenderState D3DRS_ALPHAREF, &H8&
        g_dev.SetRenderState D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL
    End If

    ' Loop through and render all trees
    'For i = 0 To NUM_TREES
    i = m_iTreeHead
    Do While i <> -1
        
        ' Set the tree texture
        g_dev.SetTexture 0, m_TreeTextures(m_Trees(i).iTreeTexture)

        ' Translate the billboard into place
        m_matBillboardMatrix.m41 = m_Trees(i).vPos.x
        m_matBillboardMatrix.m42 = m_Trees(i).vPos.y
        m_matBillboardMatrix.m43 = m_Trees(i).vPos.z
        g_dev.SetTransform D3DTS_WORLD, m_matBillboardMatrix

        ' Copy tree mesh into vertexbuffer
        Dim v As TREEVERTEX
        D3DVertexBuffer8SetData m_TreeVB, 0, Len(v) * 4, 0, m_Trees(i).v(0)
        

        ' Render the billboards one at a time
        ' CONSIDER: putting this in larger vertex buffers sorted by texture
        g_dev.SetStreamSource 0, m_TreeVB, Len(v)
        g_dev.SetVertexShader D3DFVF_TREEVERTEX
        g_dev.DrawPrimitive D3DPT_TRIANGLESTRIP, 0, 2
    
        i = m_Trees(i).iNext
    Loop
    'Next

    ' Restore state
    Dim matWorld As D3DMATRIX
    
    D3DXMatrixIdentity matWorld
    g_dev.SetTransform D3DTS_WORLD, matWorld
    g_dev.SetRenderState D3DRS_ALPHATESTENABLE, 0 '   FALSE
    g_dev.SetRenderState D3DRS_ALPHABLENDENABLE, 0 '  FALSE

End Sub



'-----------------------------------------------------------------------------
' Name: Render()
' Desc: Called once per frame, the call is the entry point for 3d
'       rendering. This function sets up render states, clears the
'       viewport, and renders the scene.
'-----------------------------------------------------------------------------
Function Render() As Boolean

    Dim matView As D3DMATRIX, matViewSave As D3DMATRIX, hr As Long
    
    Render = False
    'See what state the device is in.
    hr = g_dev.TestCooperativeLevel
    If hr = D3DERR_DEVICENOTRESET Then
        On Error Resume Next
        g_dev.Reset g_d3dpp
        If (Err.Number = D3D_OK) Then
            RestoreDeviceObjects
        End If
        On Error GoTo 0
    ElseIf hr <> 0 Then
        Exit Function 'dont bother rendering if we are not ready yet
    End If
    Render = True
    ' Clear the viewport
    g_dev.Clear ByVal 0, ByVal 0, D3DCLEAR_ZBUFFER, &H0, 1, 0

    ' Begin the scene
    g_dev.BeginScene

    ' Render the Skybox
    
    ' Center view matrix for skybox and disable zbuffer
        
    g_dev.GetTransform D3DTS_VIEW, matViewSave
    matView = matViewSave
    matView.m41 = 0: matView.m42 = -0.3: matView.m43 = 0
    g_dev.SetTransform D3DTS_VIEW, matView
    g_dev.SetRenderState D3DRS_ZENABLE, 0 ' FALSE

    ' Render the skybox
    m_SkyBox.Render g_dev

    ' Restore the render states
    g_dev.SetTransform D3DTS_VIEW, matViewSave
    g_dev.SetRenderState D3DRS_ZENABLE, 1 'TRUE

    ' Draw the terrain
    m_Terrain.Render g_dev

    ' Draw the trees
    DrawTrees


        ' End the scene.
    g_dev.EndScene


End Function


'-----------------------------------------------------------------------------
' Name: InitDeviceObjects()
' Desc: This creates all device-dependant managed objects, such as managed
'       textures and managed vertex buffers.
'-----------------------------------------------------------------------------
Sub InitDeviceObjects()
    Dim i As Long
    Dim v As TREEVERTEX

    ' Create the tree textures
    For i = 0 To NUMTREETEXTURES - 1
        Set m_TreeTextures(i) = g_d3dx.CreateTextureFromFileEx(g_dev, m_media + m_strTreeTextures(i), 256, 256, D3DX_DEFAULT, 0, D3DFMT_A1R5G5B5, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, &HFF000000, ByVal 0, ByVal 0)
    Next
    
    ' Create a quad for rendering each tree
    Set m_TreeVB = g_dev.CreateVertexBuffer(4 * Len(v), 0, D3DFVF_TREEVERTEX, D3DPOOL_MANAGED)

    ' Load the skybox
    m_SkyBox.InitFromFile g_dev, m_media + "SkyBox2.x"

    ' Load the terrain
    m_Terrain.InitFromFile g_dev, m_media + "SeaFloor.x"
    
    
    ' Add some "hilliness" to the terrain
    Dim HillVB As Direct3DVertexBuffer8, NumHillVerts As Long
    Dim HillVerts() As HILLVERTEX
    Set HillVB = m_Terrain.mesh.GetVertexBuffer()
    
    NumHillVerts = m_Terrain.mesh.GetNumVertices
    ReDim HillVerts(NumHillVerts)

    D3DVertexBuffer8GetData HillVB, 0, NumHillVerts * Len(HillVerts(0)), 0, HillVerts(0)
    For i = 0 To NumHillVerts - 1
        HillVerts(i).y = HeightField(HillVerts(i).x, HillVerts(i).z)
    Next
    D3DVertexBuffer8SetData HillVB, 0, NumHillVerts * Len(HillVerts(0)), 0, HillVerts(0)

End Sub



'-----------------------------------------------------------------------------
' Name: RestoreDeviceObjects()
' Desc: Restore device-memory objects and state after a device is created or
'       resized.
'-----------------------------------------------------------------------------
Sub RestoreDeviceObjects()

    ' Restore the device objects for the meshes and fonts
    m_Terrain.RestoreDeviceObjects g_dev
    m_SkyBox.RestoreDeviceObjects g_dev
    
    ' Set the transform matrices (view and world are updated per frame)
    Dim matProj As D3DMATRIX
    D3DXMatrixPerspectiveFovLH matProj, g_pi / 4, Me.ScaleHeight / Me.ScaleWidth, 1, 100
    g_dev.SetTransform D3DTS_PROJECTION, matProj

    ' Set up the default texture states
    g_dev.SetTextureStageState 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1
    g_dev.SetTextureStageState 0, D3DTSS_COLORARG1, D3DTA_TEXTURE
    g_dev.SetTextureStageState 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1
    g_dev.SetTextureStageState 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE
    g_dev.SetTextureStageState 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR
    g_dev.SetTextureStageState 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR
    g_dev.SetTextureStageState 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP
    g_dev.SetTextureStageState 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP

    g_dev.SetRenderState D3DRS_DITHERENABLE, 1 'TRUE
    g_dev.SetRenderState D3DRS_ZENABLE, 1      'TRUE )
    g_dev.SetRenderState D3DRS_LIGHTING, 0     'FALSE )
    
End Sub



'-----------------------------------------------------------------------------
' Name: InvalidateDeviceObjects()
' Desc: Called when the device-dependant objects are about to be lost.
'-----------------------------------------------------------------------------
Sub InvalidateDeviceObjects()

    m_Terrain.InvalidateDeviceObjects
    m_SkyBox.InvalidateDeviceObjects
    
End Sub



'-----------------------------------------------------------------------------
' Name: DeleteDeviceObjects()
' Desc: Called when the app is exitting, or the device is being changed,
'       this function deletes any device dependant objects.
'-----------------------------------------------------------------------------
Sub DeleteDeviceObjects()
    
    Dim i As Long
    
    m_Terrain.Destroy
    m_SkyBox.Destroy

    For i = 0 To NUMTREETEXTURES - 1
        Set m_TreeTextures(i) = Nothing
    Next

    m_bInit = False

End Sub




'-----------------------------------------------------------------------------
' Name: FinalCleanup()
' Desc: Called before the app exits, this function gives the app the chance
'       to cleanup after itself.
'-----------------------------------------------------------------------------
Sub FinalCleanup()
    Set m_Terrain = Nothing
    Set m_SkyBox = Nothing
End Sub


'-----------------------------------------------------------------------------
' Name: VerifyDevice()
' Desc: Called during device intialization, this code checks the device
'       for some minimum set of capabilities
'-----------------------------------------------------------------------------
Public Function VerifyDevice(usageflags As Long, format As CONST_D3DFORMAT) As Boolean
    
    ' This sample uses alpha textures and/or straight alpha. Make sure the
    ' device supports them
    
    If ((g_d3dCaps.TextureCaps And D3DPTEXTURECAPS_ALPHAPALETTE) = D3DPTEXTURECAPS_ALPHAPALETTE) Then VerifyDevice = True
    If ((g_d3dCaps.TextureCaps And D3DPTEXTURECAPS_ALPHA) = D3DPTEXTURECAPS_ALPHA) Then VerifyDevice = True
    
End Function





