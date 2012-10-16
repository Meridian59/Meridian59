VERSION 5.00
Begin VB.Form Form1 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Matrices"
   ClientHeight    =   3195
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   4680
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3195
   ScaleWidth      =   4680
   StartUpPosition =   3  'Windows Default
   Begin VB.PictureBox Picture1 
      Height          =   3015
      Left            =   120
      ScaleHeight     =   2955
      ScaleWidth      =   4395
      TabIndex        =   0
      Top             =   120
      Width           =   4455
      Begin VB.Timer Timer1 
         Enabled         =   0   'False
         Interval        =   40
         Left            =   1920
         Top             =   1320
      End
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'-----------------------------------------------------------------------------
' File: Tut03_Matrices.frm
'
'
' Desc: Now that we know how to create a device and render some 2D vertices,
'       this tutorial goes the next step and renders 3D geometry. To deal with
'       3D geometry we need to introduce the use of 4x4 matrices to transform
'       the geometry with translations, rotations, scaling, and setting up our
'       camera.
'
'       Geometry is defined in model space. We can move it (translation),
'       rotate it (rotation), or stretch it (scaling) using a world transform.
'       The geometry is then said to be in world space. Next, we need to
'       position the camera, or eye point, somewhere to look at the geometry.
'       Another transform, via the view matrix, is used, to position and
'       rotate our view. With the geometry then in view space, our last
'       transform is the projection transform, which "projects" the 3D scene
'       into our 2D viewport.
'
'       Note that in this tutorial, we are introducing the use of D3DX, which
'       is a set up helper utilities for D3D. In this case, we are using some
'       of D3DX's useful matrix initialization functions.
'
' Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
'-----------------------------------------------------------------------------
Option Explicit

'-----------------------------------------------------------------------------
' variables
'-----------------------------------------------------------------------------
Dim g_DX As New DirectX8
Dim g_D3D As Direct3D8              'Used to create the D3DDevice
Dim g_D3DDevice As Direct3DDevice8  'Our rendering device
Dim g_VB As Direct3DVertexBuffer8


' A structure for our custom vertex type
' representing a point on the screen
Private Type CUSTOMVERTEX
    x As Single         'x in screen space
    y As Single         'y in screen space
    z  As Single        'normalized z
    color As Long       'vertex color
End Type

' Our custom FVF, which describes our custom vertex structure
Const D3DFVF_CUSTOMVERTEX = (D3DFVF_XYZ Or D3DFVF_DIFFUSE)

Const g_pi = 3.1415


'-----------------------------------------------------------------------------
' Name: Form_Load()
'-----------------------------------------------------------------------------
Private Sub Form_Load()
    Dim b As Boolean
    
    ' Allow the form to become visible
    Me.Show
    DoEvents
    
    ' Initialize D3D and D3DDevice
    b = InitD3D(Picture1.hWnd)
    If Not b Then
        MsgBox "Unable to CreateDevice (see InitD3D() source for comments)"
        End
    End If
    
    
    ' Initialize Vertex Buffer with Geometry
    b = InitGeometry()
    If Not b Then
        MsgBox "Unable to Create VertexBuffer"
        End
    End If
    
    
    ' Enable Timer to update
    Timer1.Enabled = True
    
End Sub

'-----------------------------------------------------------------------------
' Name: Timer1_Timer()
'-----------------------------------------------------------------------------
Private Sub Timer1_Timer()
    Render
End Sub

'-----------------------------------------------------------------------------
' Name: Form_Unload()
'-----------------------------------------------------------------------------
Private Sub Form_Unload(Cancel As Integer)
    End
End Sub


'-----------------------------------------------------------------------------
' Name: InitD3D()
' Desc: Initializes Direct3D
'-----------------------------------------------------------------------------
Function InitD3D(hWnd As Long) As Boolean
    On Local Error Resume Next
    
    ' Create the D3D object
    Set g_D3D = g_DX.Direct3DCreate()
    If g_D3D Is Nothing Then Exit Function
    
    ' Get the current display mode
    Dim mode As D3DDISPLAYMODE
    g_D3D.GetAdapterDisplayMode D3DADAPTER_DEFAULT, mode
        
    ' Fill in the type structure used to create the device
    Dim d3dpp As D3DPRESENT_PARAMETERS
    d3dpp.Windowed = 1
    d3dpp.SwapEffect = D3DSWAPEFFECT_COPY_VSYNC
    d3dpp.BackBufferFormat = mode.Format
    
    ' Create the D3DDevice
    ' If you do not have hardware 3d acceleration. Enable the reference rasterizer
    ' using the DirectX control panel and change D3DDEVTYPE_HAL to D3DDEVTYPE_REF
    
    Set g_D3DDevice = g_D3D.CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, _
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING, d3dpp)
    If g_D3DDevice Is Nothing Then Exit Function
    
    ' Device state would normally be set here
    ' Turn off culling, so we see the front and back of the triangle
    g_D3DDevice.SetRenderState D3DRS_CULLMODE, D3DCULL_NONE

    ' Turn off D3D lighting, since we are providing our own vertex colors
    g_D3DDevice.SetRenderState D3DRS_LIGHTING, 0

    InitD3D = True
End Function


'-----------------------------------------------------------------------------
' Name: SetupMatrices()
' Desc: Sets up the world, view, and projection transform matrices.
'-----------------------------------------------------------------------------
Sub SetupMatrices()

    
    ' The transform Matrix is used to position and orient the objects
    ' you are drawing
    ' For our world matrix, we will just rotate the object about the y-axis.
    Dim matWorld As D3DMATRIX
    D3DXMatrixRotationY matWorld, Timer * 4
    g_D3DDevice.SetTransform D3DTS_WORLD, matWorld


    ' The view matrix defines the position and orientation of the camera
    ' Set up our view matrix. A view matrix can be defined given an eye point,
    ' a point to lookat, and a direction for which way is up. Here, we set the
    ' eye five units back along the z-axis and up three units, look at the
    ' origin, and define "up" to be in the y-direction.
    
    
    Dim matView As D3DMATRIX
    D3DXMatrixLookAtLH matView, vec3(0#, 3#, -5#), _
                                 vec3(0#, 0#, 0#), _
                                 vec3(0#, 1#, 0#)
                                 
    g_D3DDevice.SetTransform D3DTS_VIEW, matView

    ' The projection matrix describes the camera's lenses
    ' For the projection matrix, we set up a perspective transform (which
    ' transforms geometry from 3D view space to 2D viewport space, with
    ' a perspective divide making objects smaller in the distance). To build
    ' a perpsective transform, we need the field of view (1/4 pi is common),
    ' the aspect ratio, and the near and far clipping planes (which define at
    ' what distances geometry should be no longer be rendered).
    Dim matProj As D3DMATRIX
    D3DXMatrixPerspectiveFovLH matProj, g_pi / 4, 1, 1, 1000
    g_D3DDevice.SetTransform D3DTS_PROJECTION, matProj

End Sub



'-----------------------------------------------------------------------------
' Name: InitGeometry()
' Desc: Creates a vertex buffer and fills it with our vertices.
'-----------------------------------------------------------------------------
Function InitGeometry() As Boolean

    ' Initialize three vertices for rendering a triangle
    Dim Vertices(2) As CUSTOMVERTEX
    Dim VertexSizeInBytes As Long
    
    VertexSizeInBytes = Len(Vertices(0))
    
    With Vertices(0): .x = -1: .y = -1: .z = 0: .color = &HFFFF0000: End With
    With Vertices(1): .x = 1: .y = -1: .z = 0:  .color = &HFF00FF00: End With
    With Vertices(2): .x = 0: .y = 1: .z = 0:  .color = &HFF00FFFF: End With
        

    ' Create the vertex buffer.
    Set g_VB = g_D3DDevice.CreateVertexBuffer(VertexSizeInBytes * 3, _
                     0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT)
    If g_VB Is Nothing Then Exit Function

    ' fill the vertex buffer from our array
    D3DVertexBuffer8SetData g_VB, 0, VertexSizeInBytes * 3, 0, Vertices(0)

    InitGeometry = True
End Function



'-----------------------------------------------------------------------------
' Name: Cleanup()
' Desc: Releases all previously initialized objects
'-----------------------------------------------------------------------------
Sub Cleanup()
    Set g_VB = Nothing
    Set g_D3DDevice = Nothing
    Set g_D3D = Nothing
End Sub

'-----------------------------------------------------------------------------
' Name: Render()
' Desc: Draws the scene
'-----------------------------------------------------------------------------
Sub Render()

    Dim v As CUSTOMVERTEX
    Dim sizeOfVertex As Long
    
    
    If g_D3DDevice Is Nothing Then Exit Sub

    ' Clear the backbuffer to a blue color (ARGB = 000000ff)
    '
    ' To clear the entire back buffer we send down
    g_D3DDevice.Clear 0, ByVal 0, D3DCLEAR_TARGET, &HFF&, 1#, 0
    
     
    ' Begin the scene
    g_D3DDevice.BeginScene
    
    
    ' Setup the world, view, and projection matrices
    SetupMatrices

    'Draw the triangles in the vertex buffer
    sizeOfVertex = Len(v)
    g_D3DDevice.SetStreamSource 0, g_VB, sizeOfVertex
    g_D3DDevice.SetVertexShader D3DFVF_CUSTOMVERTEX
    g_D3DDevice.DrawPrimitive D3DPT_TRIANGLELIST, 0, 1

         
    ' End the scene
    g_D3DDevice.EndScene
    
     
    ' Present the backbuffer contents to the front buffer (screen)
    g_D3DDevice.Present ByVal 0, ByVal 0, 0, ByVal 0
    
End Sub


'-----------------------------------------------------------------------------
' Name: vec3()
' Desc: helper function
'-----------------------------------------------------------------------------
Function vec3(x As Single, y As Single, z As Single) As D3DVECTOR
    vec3.x = x
    vec3.y = y
    vec3.z = z
End Function






