VERSION 5.00
Begin VB.Form Form1 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Meshes"
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
' File: Tut06_meshes.frm
'
'
' Desc: For advanced geometry, most apps will prefer to load pre-authored
'       meshes from a file. Fortunately, when using meshes, D3DX does most of
'       the work for this, parsing a geometry file and creating vertx buffers
'       (and index buffers) for us. This tutorial shows how to use a D3DXMESH
'       object, including loading it from a file and rendering it. One thing
'       D3DX does not handle for us is the materials and textures for a mesh,
'       so note that we have to handle those manually.
'
'       Note: one advanced (but nice) feature that we don't show here, is that
'       when cloning a mesh we can specify the FVF. So, regardless of how the
'       mesh was authored, we can add/remove normals, add more texture
'       coordinate sets (for multi-texturing), etc..
'
'
' Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
'-----------------------------------------------------------------------------
Option Explicit

'-----------------------------------------------------------------------------
' variables
'-----------------------------------------------------------------------------
Dim g_DX As New DirectX8
Dim g_D3DX As New D3DX8
Dim g_D3D As Direct3D8                  ' Used to create the D3DDevice
Dim g_D3DDevice As Direct3DDevice8      ' Our rendering device
Dim g_Mesh As D3DXMesh                  ' Our Mesh
Dim g_MeshMaterials() As D3DMATERIAL8   ' Mesh Material data
Dim g_MeshTextures() As Direct3DTexture8 ' Mesh Textures
Dim g_NumMaterials As Long

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
    'g_D3DDevice.SetRenderState D3DRS_CULLMODE, D3DCULL_NONE

    ' Turn on the zbuffer
    g_D3DDevice.SetRenderState D3DRS_ZENABLE, 1
    
    ' Turn on lighting
    g_D3DDevice.SetRenderState D3DRS_LIGHTING, 0
    
    ' Turn on full ambient light to white
    g_D3DDevice.SetRenderState D3DRS_AMBIENT, &HFFFFFFFF

    InitD3D = True
End Function


'-----------------------------------------------------------------------------
' Name: SetupMatrices()
' Desc: Sets up the world, view, and projection transform matrices.
'-----------------------------------------------------------------------------
Sub SetupMatrices()

    
    ' The transform Matrix is used to position and orient the objects
    ' you are drawing
    ' For our world matrix, we will just rotate the object about the y axis.
    Dim matWorld As D3DMATRIX
    D3DXMatrixRotationAxis matWorld, vec3(0, 1, 0), Timer / 4
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
' Desc: Load Mesh and textures
'-----------------------------------------------------------------------------
Function InitGeometry() As Boolean
    Dim MtrlBuffer As D3DXBuffer    'a d3dxbuffer is a generic chunk of memory
    Dim i As Long
    
    ' Load the mesh from the specified file
    '   filename = x file to load
    '   flags = D3DXMESH_MANAGED  allow d3dx manage the memory usage of this geometry
    '   device = g_d3ddevice (if we destroy the device we have to reload the geomerty)
    '   adjacenyBuffer = nothing (we dont need it)
    '   materialBuffer = MtrlBuffer (this variable will be filled in with a new object)
    Set g_Mesh = g_D3DX.LoadMeshFromX(App.Path + "\Tiger.x", D3DXMESH_MANAGED, _
                                   g_D3DDevice, Nothing, MtrlBuffer, g_NumMaterials)
                                   
    If g_Mesh Is Nothing Then Exit Function


     
    'allocate space for our materials and textures
    ReDim g_MeshMaterials(g_NumMaterials - 1)
    ReDim g_MeshTextures(g_NumMaterials - 1)
    
    Dim strTexName As String
    
    ' We need to extract the material properties and texture names
    ' from the MtrlBuffer
    
    For i = 0 To g_NumMaterials - 1
    
        ' Copy the material using the d3dx helper function
        g_D3DX.BufferGetMaterial MtrlBuffer, i, g_MeshMaterials(i)

        ' Set the ambient color for the material (D3DX does not do this)
        g_MeshMaterials(i).Ambient = g_MeshMaterials(i).diffuse
     
        ' Create the texture
        strTexName = g_D3DX.BufferGetTextureName(MtrlBuffer, i)
        If strTexName <> "" Then
            Set g_MeshTextures(i) = g_D3DX.CreateTextureFromFile(g_D3DDevice, App.Path + "\" + strTexName)
        End If
        
    Next
    
    Set MtrlBuffer = Nothing

    InitGeometry = True
End Function



'-----------------------------------------------------------------------------
' Name: Cleanup()
' Desc: Releases all previously initialized objects
'-----------------------------------------------------------------------------
Sub Cleanup()
    Erase g_MeshTextures
    Erase g_MeshMaterials
    
    Set g_Mesh = Nothing
    Set g_D3DDevice = Nothing
    Set g_D3D = Nothing
End Sub

'-----------------------------------------------------------------------------
' Name: Render()
' Desc: Draws the scene
'-----------------------------------------------------------------------------
Sub Render()

    Dim i As Long
    
    If g_D3DDevice Is Nothing Then Exit Sub

    ' Clear the backbuffer to a blue color (ARGB = 000000ff)
    ' Clear the z buffer to 1
    g_D3DDevice.Clear 0, ByVal 0, D3DCLEAR_TARGET Or D3DCLEAR_ZBUFFER, &HFF&, 1#, 0
    
     
    ' Setup the world, view, and projection matrices
    SetupMatrices
 
    
    ' Begin the scene
    g_D3DDevice.BeginScene
    
    ' Meshes are divided into subsets, one for each material.
    ' Render them in a loop
    For i = 0 To g_NumMaterials - 1
    
        ' Set the material and texture for this subset
        g_D3DDevice.SetMaterial g_MeshMaterials(i)
        g_D3DDevice.SetTexture 0, g_MeshTextures(i)
        
        ' Draw the mesh subset
        g_Mesh.DrawSubset i
    Next
            
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






