VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Create Device"
   ClientHeight    =   3195
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   4680
   LinkTopic       =   "Form1"
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
' File: Tut01_CreateDevice.frm
'
' Desc: This is the first tutorial for using Direct3D. In this tutorial, all
'       we are doing is create a Direct3D device and using it to clear the
'       screen.
' Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
'-----------------------------------------------------------------------------


'-----------------------------------------------------------------------------
' variables
'-----------------------------------------------------------------------------
Dim g_DX As New DirectX8
Dim g_D3D As Direct3D8              'Used to create the D3DDevice
Dim g_D3DDevice As Direct3DDevice8  'Our rendering device


'-----------------------------------------------------------------------------
' Name: Form_Load()
'-----------------------------------------------------------------------------
Private Sub Form_Load()
    
    ' Allow the form to become visible
    Me.Show
    DoEvents
    
    ' Initialize D3D and D3DDevice
    b = InitD3D(Picture1.hWnd)
    If Not b Then
        MsgBox "Unable to CreateDevice (see InitD3D() source for comments)"
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
    
    ' Get The current Display Mode format
    Dim mode As D3DDISPLAYMODE
    g_D3D.GetAdapterDisplayMode D3DADAPTER_DEFAULT, mode
    
    ' Fill in the type structure used to create the D3DDevice. Most parameters
    ' are left at zero. We set Windowed to 1 for TRUE, since we want to do D3D
    ' in a window, and the set the SwapEffect to flip the backbuffer to the
    ' frontbuffer only on vsync (which prevents "tearing" artifacts).
    ' we set the back buffer format from the current display mode
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
' Name: Cleanup()
' Desc: Releases all previously initialized objects
'-----------------------------------------------------------------------------
Sub Cleanup()
    Set g_D3DDevice = Nothing
    Set g_D3D = Nothing
End Sub



'-----------------------------------------------------------------------------
' Name: Render()
' Desc: Draws the scene
'-----------------------------------------------------------------------------
Sub Render()

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
    
    ' Rendering of scene objects happens here
         
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








