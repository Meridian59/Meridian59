VERSION 5.00
Begin VB.Form Form1 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Textures"
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
' File: Tut05_textures.frm
'
' Desc: Better than just lights and materials, 3D objects look much more
'       convincing when texture-mapped. Textures can be thought of as a sort
'       of wallpaper, that is shrinkwrapped to fit a texture. Textures are
'       typically loaded from image files, and D3DX provides a utility to
'       function to do this for us. Like a vertex buffer, textures have
'       Lock() and Unlock() functions to access (read or write) the image
'       data. Textures have a width, height, miplevel, and pixel format. The
'       miplevel is for "mipmapped" textures, an advanced performance-
'       enhancing feature which uses lower resolutions of the texture for
'       objects in the distance where detail is less noticeable. The pixel
'       format determines how the colors are stored in a texel. The most
'       common formats are the 16-bit R5G6B5 format (5 bits of red, 6-bits of
'       green and 5 bits of blue) and the 32-bit A8R8G8B8 format (8 bits each
'       of alpha, red, green, and blue).
'
'       Textures are associated with geometry through texture coordinates.
'       Each vertex has one or more sets of texture coordinates, which are
'       named tu and tv and range from 0.0 to 1.0. Texture coordinates can be
'       supplied by the geometry, or can be automatically generated using
'       Direct3D texture coordinate generation (which is an advanced feature).
'
' Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
'-----------------------------------------------------------------------------
Option Explicit

'-----------------------------------------------------------------------------
' variables
'-----------------------------------------------------------------------------
Dim g_DX As New DirectX8
Dim g_D3DX As New D3DX8
Dim g_D3D As Direct3D8              ' Used to create the D3DDevice
Dim g_D3DDevice As Direct3DDevice8  ' Our rendering device
Dim g_VB As Direct3DVertexBuffer8   ' Holds our vertex data
Dim g_Texture As Direct3DTexture8   ' Our texture

' A structure for our custom vertex type
Private Type CUSTOMVERTEX
    postion As D3DVECTOR    '3d position for vertex
    color As Long           'color of the vertex
    tu As Single            'texture map coordinate
    tv As Single            'texture map coordinate
End Type

' Our custom FVF, which describes our custom vertex structure
Const D3DFVF_CUSTOMVERTEX = (D3DFVF_XYZ Or D3DFVF_DIFFUSE Or D3DFVF_TEX1)

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
    
    
    ' Initialize vertex buffer with geometry and load our texture
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
    Cleanup
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
    
    ' Get The current Display Mode format
    Dim mode As D3DDISPLAYMODE
    g_D3D.GetAdapterDisplayMode D3DADAPTER_DEFAULT, mode
         
    ' Set up the structure used to create the D3DDevice. Since we are now
    ' using more complex geometry, we will create a device with a zbuffer.
    ' the D3DFMT_D16 indicates we want a 16 bit z buffer but
    Dim d3dpp As D3DPRESENT_PARAMETERS
    d3dpp.Windowed = 1
    d3dpp.SwapEffect = D3DSWAPEFFECT_COPY_VSYNC
    d3dpp.BackBufferFormat = mode.Format
    d3dpp.BackBufferCount = 1
    d3dpp.EnableAutoDepthStencil = 1
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16

    ' Create the D3DDevice
    ' If you do not have hardware 3d acceleration. Enable the reference rasterizer
    ' using the DirectX control panel and change D3DDEVTYPE_HAL to D3DDEVTYPE_REF
    
    Set g_D3DDevice = g_D3D.CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, _
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING, d3dpp)
    If g_D3DDevice Is Nothing Then Exit Function
    
    ' Device state would normally be set here
    ' Turn off culling, so we see the front and back of the triangle
    g_D3DDevice.SetRenderState D3DRS_CULLMODE, D3DCULL_NONE

    ' Turn on the zbuffer
    g_D3DDevice.SetRenderState D3DRS_ZENABLE, 1
    
    ' Turn off lighting we are going to use colored vertices
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
    ' For our world matrix, we will just rotate the object about the 1 1 1 axis.
    Dim matWorld As D3DMATRIX
    D3DXMatrixRotationAxis matWorld, vec3(1, 1, 1), Timer / 4
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
' Name: SetupLights()
' Desc: Sets up the lights and materials for the scene.
'-----------------------------------------------------------------------------
Sub SetupLights()
     
    Dim col As D3DCOLORVALUE
    
    
    ' Set up a material. The material here just has the diffuse and ambient
    ' colors set to yellow. Note that only one material can be used at a time.
    Dim mtrl As D3DMATERIAL8
    With col:    .r = 1: .g = 1: .b = 0: .a = 1:   End With
    mtrl.diffuse = col
    mtrl.Ambient = col
    g_D3DDevice.SetMaterial mtrl
    
    ' Set up a white, directional light, with an oscillating direction.
    ' Note that many lights may be active at a time (but each one slows down
    ' the rendering of our scene). However, here we are just using one. Also,
    ' we need to set the D3DRS_LIGHTING renderstate to enable lighting
    
    Dim light As D3DLIGHT8
    light.Type = D3DLIGHT_DIRECTIONAL
    light.diffuse.r = 1#
    light.diffuse.g = 1#
    light.diffuse.b = 1#
    light.Direction.x = Cos(Timer * 2)
    light.Direction.y = 1#
    light.Direction.z = Sin(Timer * 2)
    light.Range = 1000#
    
    g_D3DDevice.SetLight 0, light                   'let d3d know about the light
    g_D3DDevice.LightEnable 0, 1                    'turn it on
    g_D3DDevice.SetRenderState D3DRS_LIGHTING, 1    'make sure lighting is enabled

    ' Finally, turn on some ambient light.
    ' Ambient light is light that scatters and lights all objects evenly
    g_D3DDevice.SetRenderState D3DRS_AMBIENT, &H202020
    
End Sub


'-----------------------------------------------------------------------------
' Name: InitGeometry()
' Desc: Creates a vertex buffer and fills it with our vertices.
'-----------------------------------------------------------------------------
Function InitGeometry() As Boolean
    Dim i As Long
    
    
    
    'Use D3DX to create a texture from a file based image
    Set g_Texture = g_D3DX.CreateTextureFromFile(g_D3DDevice, App.Path + "\banana.bmp")
    If g_Texture Is Nothing Then Exit Function

    
    ' Initialize three vertices for rendering a triangle
    Dim Vertices(99) As CUSTOMVERTEX
    Dim VertexSizeInBytes As Long
    Dim theta As Single
    
    VertexSizeInBytes = Len(Vertices(0))
    
    
    ' We are algorithmically generating a cylinder
    ' here, including the normals, which are used for lighting.
    ' normals are vectors that are of length 1 and point in a direction
    ' perpendicular to the plane of the triangle the normal belongs to
    ' In later tutorials we will use d3dx to generate them
    
    For i = 0 To 49
        theta = (2 * g_pi * i) / (50 - 1)
        
        Vertices(2 * i + 0).postion = vec3(Sin(theta), -1, Cos(theta))
        Vertices(2 * i + 0).color = &HFFFFFFFF  'white
        Vertices(2 * i + 0).tu = i / (50 - 1)
        Vertices(2 * i + 0).tv = 1
        
        Vertices(2 * i + 1).postion = vec3(Sin(theta), 1, Cos(theta))
        Vertices(2 * i + 1).color = &HFF808080  'grey
        Vertices(2 * i + 1).tu = i / (50 - 1)
        Vertices(2 * i + 1).tv = 0

    Next

    ' Create the vertex buffer.
    Set g_VB = g_D3DDevice.CreateVertexBuffer(VertexSizeInBytes * 50 * 2, _
         0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT)
    If g_VB Is Nothing Then Exit Function

    ' fill the vertex buffer from our array
    D3DVertexBuffer8SetData g_VB, 0, VertexSizeInBytes * 100, 0, Vertices(0)

    InitGeometry = True
End Function



'-----------------------------------------------------------------------------
' Name: Cleanup()
' Desc: Releases all previously initialized objects
'-----------------------------------------------------------------------------
Sub Cleanup()
    Set g_Texture = Nothing
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
    ' Clear the z buffer to 1
    g_D3DDevice.Clear 0, ByVal 0, D3DCLEAR_TARGET Or D3DCLEAR_ZBUFFER, &HFF&, 1#, 0
    
     
    ' Begin the scene
    g_D3DDevice.BeginScene
    
    
    ' Setup our texture. Using textures introduces the texture stage states,
    ' which govern how textures get blended together (in the case of multiple
    ' textures) and lighting information. In this case, we are modulating
    ' (blending) our texture with the diffuse color of the vertices.
    g_D3DDevice.SetTexture 0, g_Texture
    g_D3DDevice.SetTextureStageState 0, D3DTSS_COLOROP, D3DTOP_MODULATE
    g_D3DDevice.SetTextureStageState 0, D3DTSS_COLORARG1, D3DTA_TEXTURE
    g_D3DDevice.SetTextureStageState 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE
    g_D3DDevice.SetTextureStageState 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE


    'Uncomment to learn about texture coordinate matrices
    'AnimateTextureCoordinates


    ' Setup the world, view, and projection matrices
    SetupMatrices

    ' Draw the triangles in the vertex buffer
    ' Note we are now using a triangle strip of vertices
    ' instead of a triangle list
    sizeOfVertex = Len(v)
    g_D3DDevice.SetStreamSource 0, g_VB, sizeOfVertex
    g_D3DDevice.SetVertexShader D3DFVF_CUSTOMVERTEX
    g_D3DDevice.DrawPrimitive D3DPT_TRIANGLESTRIP, 0, (4 * 25) - 2

         
    ' End the scene
    g_D3DDevice.EndScene
    
     
    ' Present the backbuffer contents to the front buffer (screen)
    g_D3DDevice.Present ByVal 0, ByVal 0, 0, ByVal 0
    
End Sub


'-----------------------------------------------------------------------------
' Name: AnimateTextureCoordinates()
' Desc: Advanced technique for generating texture coordinates
'-----------------------------------------------------------------------------
Sub AnimateTextureCoordinates()

    ' Note: to use D3D texture coordinate generation, use the stage state
    ' D3DTSS_TEXCOORDINDEX, as shown below. In this example, we are using
    ' the position of the vertex in camera space to generate texture
    ' coordinates. The tex coord index (TCI) parameters are passed into a
    ' texture transform, which is a 4x4 matrix which transforms the x,y,z
    ' TCI coordinates into tu, tv texture coordinates.

    ' In this example, the texture matrix is setup to
    ' transform the texture from (-1,+1) position coordinates to (0,1)
    ' texture coordinate space:
    '    tu =  0.25*x + 0.5
    '    tv = -0.25*y + 0.5
    Dim mat As D3DMATRIX
    
    mat.m11 = 0.25: mat.m12 = 0#:     mat.m13 = 0#: mat.m14 = 0#
    mat.m21 = 0#:   mat.m22 = -0.25:  mat.m23 = 0#: mat.m24 = 0#
    mat.m31 = 0#:   mat.m32 = 0#:     mat.m33 = 1#: mat.m34 = 0#
    mat.m41 = 0.5:  mat.m42 = 0.5:    mat.m43 = 0#: mat.m44 = 1#

    g_D3DDevice.SetTransform D3DTS_TEXTURE0, mat
    g_D3DDevice.SetTextureStageState 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2
    g_D3DDevice.SetTextureStageState 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION


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






