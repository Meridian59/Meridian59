VERSION 5.00
Begin VB.Form Form1 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Vertices"
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
' File: Tut02_Vertices.frm
'
' Desc: In this tutorial, we are rendering some vertices. This introduces the
'       concept of the vertex buffer, a Direct3D object used to store
'       vertices. Vertices can be defined any way we want by defining a
'       custom structure and a custom FVF (flexible vertex format). In this
'       tutorial, we are using vertices that are transformed (meaning they
'       are already in 2D window coordinates) and lit (meaning we are not
'       using Direct3D lighting, but are supplying our own colors).
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
    rhw As Single       'normalized z rhw
    color As Long       'vertex color
End Type

' Our custom FVF, which describes our custom vertex structure
Const D3DFVF_CUSTOMVERTEX = (D3DFVF_XYZRHW Or D3DFVF_DIFFUSE)



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
    
    
    ' Initialize Vertex Buffer
    b = InitVB()
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
    
    ' Create the D3D object, which is needed to create the D3DDevice. It can
    ' also be used to enumerate devices types, modes, etc., which will be
    ' shown in a separate tutorial.
    Set g_D3D = g_DX.Direct3DCreate()
    If g_D3D Is Nothing Then Exit Function
    
    ' get the current display mode
    Dim mode As D3DDISPLAYMODE
    g_D3D.GetAdapterDisplayMode D3DADAPTER_DEFAULT, mode
    
    ' Fill in the type structure used to create the D3DDevice. Most parameters
    ' are left at zero. We set Windowed to 1 for TRUE, since we want to do D3D
    ' in a window, and the set the SwapEffect to flip the backbuffer to the
    ' frontbuffer only on vsync (which prevents "tearing" artifacts).
    ' Use the same format as the current display mode
    Dim d3dpp As D3DPRESENT_PARAMETERS
    d3dpp.Windowed = 1
    d3dpp.SwapEffect = D3DSWAPEFFECT_COPY_VSYNC
    d3dpp.BackBufferFormat = mode.Format
    
    ' Create the D3DDevice. Here we are using the default adapter (most
    ' systems only have one, unless they have multiple graphics hardware cards
    ' installed) and using the HAL (which is saying we prefer the hardware
    ' device or a software one). Software vertex processing is specified
    ' since we know it will work on all cards. On cards that support hardware
    ' vertex processing, though, we would see a big performance gain by using it.
    '
    ' If you do not have hardware 3d acceleration. Enable the reference rasterizer
    ' using the DirectX control panel and change D3DDEVTYPE_HAL to D3DDEVTYPE_REF
    
    Set g_D3DDevice = g_D3D.CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, _
                                      D3DCREATE_SOFTWARE_VERTEXPROCESSING, d3dpp)
    If g_D3DDevice Is Nothing Then Exit Function
    
    ' Device state would normally be set here
    
    InitD3D = True
End Function



'-----------------------------------------------------------------------------
' Name: InitVB()
' Desc: Creates a vertex buffer and fills it with our vertices. The vertex
'       buffer is basically just a chuck of memory that holds vertices. After
'       creating it, we must D3DVertexBuffer8SetData to fill it. For indices,
'       D3D also uses index buffers. The special thing about vertex and index
'       buffers is that they can be created in device memory, allowing some
'       cards to process them in hardware, resulting in a dramatic
'       performance gain.
'-----------------------------------------------------------------------------
Function InitVB() As Boolean

    ' Initialize three vertices for rendering a triangle
    Dim Vertices(2) As CUSTOMVERTEX
    Dim VertexSizeInBytes As Long
    
    VertexSizeInBytes = Len(Vertices(0))
    
    With Vertices(0): .x = 150: .y = 50: .z = 0.5: .rhw = 1: .color = &HFFFF0000: End With
    With Vertices(1): .x = 250: .y = 250: .z = 0.5: .rhw = 1: .color = &HFF00FF00: End With
    With Vertices(2): .x = 50: .y = 250: .z = 0.5: .rhw = 1: .color = &HFF00FFFF: End With
        

    ' Create the vertex buffer. Here we are allocating enough memory
    ' (from the default pool) to hold all our 3 custom vertices. We also
    ' specify the FVF, so the vertex buffer knows what data it contains.
    '   LengthInBytes= VertexSizeInBytes *3     (For total size of our buffer)
    '   fvf=D3DFVF_CUSTOMVERTEX                 (Describes whats in our vertex)
    '   flags= 0                                (default)
    '   pool=D3DPOOL_DEFAULT                    (Let d3d decide what kind of memory)
    Set g_VB = g_D3DDevice.CreateVertexBuffer(VertexSizeInBytes * 3, _
                     0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT)
    If g_VB Is Nothing Then Exit Function

    ' Now we fill the vertex buffer. To do this in Visual Basic we will use the
    ' D3DVertexBuffer8SetData helper function. It locks the vertex buffer
    ' copys data in and then unlocks the surface all with one call
    '   VBuffer=g_VB                        The vertex buffer we want to fill
    '   Offset=0                            We want to fill from the start of the buffer
    '   Size=VertSizeInBytes*3              Copy 3 CUSTOMVERTEX types into the buffer
    '   flags=0                             Send default flags to the lock
    '   data=Vertices(0)                    This param is as any
    '                                       To use it we send the first element
    '                                       in our array
    D3DVertexBuffer8SetData g_VB, 0, VertexSizeInBytes * 3, 0, Vertices(0)

    InitVB = True
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
    '   rect count = 0
    '   clearD3DRect = ByVal 0 (ByVal is necessary as param is of type as any)
    '   flags = D3DCLEAR_TARGET to specify the backbuffer
    '   color = &HFF& to specify BLUE (note final & indicates this is a long)
    '   zClear = 1  which is not used
    '   stencil = 0  which is not used
    g_D3DDevice.Clear 0, ByVal 0, D3DCLEAR_TARGET, &HFF&, 1#, 0
    
     
    ' Begin the scene
    g_D3DDevice.BeginScene
    
    'Draw the triangles in the vertex buffer. This is broken into a few
    ' steps. We are passing the vertices down a "stream", so first we need
    ' to specify the source of that stream, which is our vertex buffer. Then
    ' we need to let D3D know what vertex shader to use. Full, custom vertex
    ' shaders are an advanced topic, but in most cases the vertex shader is
    ' just the FVF, so that D3D knows what type of vertices we are dealing
    ' with. Finally, we call DrawPrimitive() which does the actual rendering
    ' of our geometry (in this case, just one triangle).
    sizeOfVertex = Len(v)
    g_D3DDevice.SetStreamSource 0, g_VB, sizeOfVertex
    g_D3DDevice.SetVertexShader D3DFVF_CUSTOMVERTEX
    g_D3DDevice.DrawPrimitive D3DPT_TRIANGLELIST, 0, 1

         
    ' End the scene
    g_D3DDevice.EndScene
    
     
    ' Present the backbuffer contents to the front buffer (screen)
    ' parameters are flexible to allow for only showing certain
    ' portions of the back buffer, we want to Present the entire buffer
    ' so we will pass down 0 to all parameters
    '   SourceRect = ByVal 0 (ByVal is necessary as param is of type as any)
    '   DestRect = ByVal 0 (ByVal is necessary as param is of type as any)
    '   hWndOverride = 0 (use same hWnd as passed to CreateDevice)
    '   DirtyRegion = Byval 0 (ByVal is necessary as param is of type as any)
    g_D3DDevice.Present ByVal 0, ByVal 0, 0, ByVal 0
    
End Sub








