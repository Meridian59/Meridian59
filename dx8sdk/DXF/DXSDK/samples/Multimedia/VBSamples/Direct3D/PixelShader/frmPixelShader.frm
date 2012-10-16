VERSION 5.00
Begin VB.Form frmPixelShader 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "VB Pixel Shader"
   ClientHeight    =   3195
   ClientLeft      =   60
   ClientTop       =   330
   ClientWidth     =   4680
   Icon            =   "frmPixelShader.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3195
   ScaleWidth      =   4680
   StartUpPosition =   3  'Windows Default
End
Attribute VB_Name = "frmPixelShader"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       FrmPixelShader.frm
'  Content:    This sample shows how to use Pixel Shaders. It renders a few polys with
'              different pixel shader functions to manipulate the way the textures look.
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

' This sample will use 7 different shaders.
Private Const NUM_PIXELSHADERS = 7

' A structure to describe the type of vertices the app will use.
Private Type VERTEX2TC_
    x As Single
    y As Single
    z As Single
    rhw As Single
    color0 As Long
    color1 As Long
    tu0 As Single
    tv0 As Single
    tu1 As Single
    tv1 As Single
End Type
Dim VERTEX2TC(3) As VERTEX2TC_
Dim verts(3) As VERTEX2TC_

' Describe the vertex format that the vertices use.
Private Const FVFVERTEX2TC = (D3DFVF_XYZRHW Or D3DFVF_DIFFUSE Or D3DFVF_SPECULAR Or D3DFVF_TEX2)

' Allocate a few DirectX object variables that the app needs to use.
Dim dX As DirectX8
Dim d3d As Direct3D8
Dim dev As Direct3DDevice8
Dim d3dx As D3DX8
Dim d3dvb As Direct3DVertexBuffer8
Dim d3dt(1) As Direct3DTexture8

'Keep the present params around for resetting the device if needed
Dim g_d3dpp As D3DPRESENT_PARAMETERS

' This string array will store the shader functions
Dim sPixelShader(6) As String

' This array will store the pointers to the assembled pixel shaders
Dim hPixelShader(6) As Long

Private Sub Form_Load()
    
'************************************************************************
'
' Here the app will call functions to set up D3D, create a device,
' initialize the vertices, initialize the vertex buffers, create the
' textures, setup the shader string arrays, and assemble the pixel shaders.
' Finally, it calls Form_Paint to render everything.
'
'************************************************************************
        
    'Set the width and height of the window
    Me.Width = 125 * Screen.TwipsPerPixelX
    Me.Height = 225 * Screen.TwipsPerPixelY
    Me.Show
    DoEvents
    
    Call InitD3D
    Call InitTextures
    Call InitVerts
    Call SetupShaders
    Call InitDevice
    Call PaintMe
    'Call Form_Paint
    
End Sub

Private Sub InitVB()
    
'************************************************************************
'
' This sub creates the vertex buffer that the app will use.
'
' PARAMETERS:
'           None.
'************************************************************************
                            
    ' Create the vertex buffer, It will hold 4 vertices (two primitives).
    Set d3dvb = dev.CreateVertexBuffer(4 * Len(VERTEX2TC(0)), D3DUSAGE_WRITEONLY, FVFVERTEX2TC, D3DPOOL_MANAGED)

    Call MoveVBVerts(0, 0)

End Sub

Private Sub MoveVBVerts(dX As Single, dY As Single)

'************************************************************************
'
' This sub moves the vertices in the vertex buffer to a new location.
'
' PARAMETERS:
'           dx: A single that represents the new X coordinate for the upper left hand corner of the vertices.
'           dy: A single that represents the new Y coordinate for the upper left hand corner of the vertices.
'
'************************************************************************
    
    Dim pVBVerts(3) As VERTEX2TC_
    Dim pData As Long, i As Long, lSize As Long
    
    'Store the size of a vertex
    lSize = Len(VERTEX2TC(0))
    
    'Lock and retrieve the data in the vertex buffer
    Call D3DAUX.D3DVertexBuffer8GetData(d3dvb, 0, lSize * 4, 0, pVBVerts(0))
    
    For i = 0 To 3
        'Set this vertex to equal the global vertex
        pVBVerts(i) = verts(i)
        'Add the X component to this vertex
        pVBVerts(i).x = verts(i).x + dX
        'Add the Y component to this vertex
        pVBVerts(i).y = verts(i).y + dY
    Next
    
    'Set and unlock the data in the vertex buffer.
    Call D3DAUX.D3DVertexBuffer8SetData(d3dvb, 0, lSize * 4, 0, pVBVerts(0))
    
End Sub

Private Sub InitVerts()

'************************************************************************
'
' This sub initializes the vertices
'
' PARAMETERS:
'           None.
'
'************************************************************************

    With verts(0)
        .x = 10: .y = 10: .z = 0.5
        .rhw = 1
        .color0 = MakeRGB(&H0, &HFF, &HFF)
        .color1 = MakeRGB(&HFF, &HFF, &HFF)
        .tu0 = 0: .tv0 = 0
        .tu1 = 0: .tv1 = 0
    End With
    
    With verts(1)
        .x = 40: .y = 10: .z = 0.5
        .rhw = 1
        .color0 = MakeRGB(&HFF, &HFF, &H0)
        .color1 = MakeRGB(&HFF, &HFF, &HFF)
        .tu0 = 1: .tv0 = 0
        .tu1 = 1: .tv1 = 0
    End With
    
    With verts(2)
        .x = 40: .y = 40: .z = 0.5
        .rhw = 1
        .color0 = MakeRGB(&HFF, &H0, &H0)
        .color1 = MakeRGB(&H0, &H0, &H0)
        .tu0 = 1: .tv0 = 1
        .tu1 = 1: .tv1 = 1
    End With
    
    With verts(3)
        .x = 10: .y = 40: .z = 0.5
        .rhw = 1
        .color0 = MakeRGB(&H0, &H0, &HFF)
        .color1 = MakeRGB(&H0, &H0, &H0)
        .tu0 = 0: .tv0 = 1
        .tu1 = 0: .tv1 = 1
    End With
    
End Sub

Private Sub InitTextures()
        
'************************************************************************
'
' This sub initializes the textures that will be used.
'
' PARAMETERS:
'           None.
'
'************************************************************************

    Dim sFile As String
    
    sFile = FindMediaDir("lake.bmp") & "lake.bmp"
    Set d3dt(1) = d3dx.CreateTextureFromFile(dev, sFile)
    sFile = FindMediaDir("seafloor.bmp") & "seafloor.bmp"
    Set d3dt(0) = d3dx.CreateTextureFromFile(dev, sFile)
    
End Sub

Private Sub SetupShaders()
    
'************************************************************************
'
' This sub sets up the string arrays that contains each pixel shader.
'
' PARAMETERS:
'           None.
'
'************************************************************************

    ' 0: Display texture 0 (t0)
    sPixelShader(0) = _
    "ps.1.0 " & _
    "tex t0 " & _
    "mov r0,t0"
    
    ' 1: Display texture 1 (t1)
    sPixelShader(1) = _
    "ps.1.0 " & _
    "tex t1 " & _
    "mov r0,t1"
    
    ' 2: Blend between tex0 and tex1, using vertex 1 as the input (v1)
    sPixelShader(2) = _
    "ps.1.0 " & _
    "tex t0 " & _
    "tex t1 " & _
    "mov r1,t1 " & _
    "lrp r0,v1,r1,t0"

    ' 3: Scale texture 0 by vertex color 1 and add to texture 1
    sPixelShader(3) = _
    "ps.1.0 " & _
    "tex t0 " & _
    "tex t1 " & _
    "mov r1,t0 " & _
    "mad r0,t1,r1,v1"

    ' 4: Add all: texture 0, 1, and color 0, 1
    sPixelShader(4) = _
    "ps.1.0 " & _
    "tex t0 " & _
    "tex t1 " & _
    "add r1,t0,v1 " & _
    "add r1,r1,t1 " & _
    "add r1,r1,v0 " & _
    "mov r0,r1"
    
    ' 5: Modulate t0 by constant register c0
    sPixelShader(5) = _
    "ps.1.0 " & _
    "tex t0 " & _
    "mul r1,c0,t0 " & _
    "mov r0,r1"
    
    ' 6: Lerp by t0 and t1 by constant register c1
    sPixelShader(6) = _
    "ps.1.0 " & _
    "tex t0 " & _
    "tex t1 " & _
    "mov r1,t1 " & _
    "lrp r0,c1,t0,r1"
    
        
End Sub

Private Sub InitPixelShaders()

'************************************************************************
'
' This sub creates the pixel shaders, and stores the pointer (handle) to them.
'
' PARAMETERS:
'           None.
'
'************************************************************************

    Dim pCode As D3DXBuffer
    Dim i As Long, lArray() As Long, lSize As Long

    'Loop through each pixel shader string
    For i = 0 To UBound(sPixelShader)
        
        'Assemble the pixel shader
        Set pCode = d3dx.AssembleShader(sPixelShader(i), 0, Nothing)
        
        'Get the size of the assembled pixel shader
        lSize = pCode.GetBufferSize() / 4
        
        'Resize the array
        ReDim lArray(lSize - 1)
        
        'Retrieve the contents of the buffer
        Call d3dx.BufferGetData(pCode, 0, 4, lSize, lArray(0))
        
        'Create the pixel shader.
        hPixelShader(i) = dev.CreatePixelShader(lArray(0))
        
        Set pCode = Nothing
        
    Next

End Sub

Private Sub InitDevice()

'************************************************************************
'
' This sub initializes the device to states that won't change, and sets
' the constant values that some of the pixel shaders use.
'
' PARAMETERS:
'           None.
'
'************************************************************************

    ' Constant registers store values that the pixel shaders can use. Each
    ' constant is an array of 4 singles that contain information about color
    ' and alpha components. This 2d array represents two pixel shader constants.
    Dim fPSConst(3, 1) As Single
    
    'Used to set the constant values for c0 (used in pixel shader 5)
    'Red
    fPSConst(0, 0) = 0.15
    'Green
    fPSConst(1, 0) = 0.75
    'Blue
    fPSConst(2, 0) = 0.25
    'Alpha
    fPSConst(3, 0) = 0
    
    'Used to set the constant values for c1 (used in pixel shader 6)
    'Red
    fPSConst(0, 1) = 0.15
    'Green
    fPSConst(1, 1) = 1
    'Blue
    fPSConst(2, 1) = 0.5
    'Alpha
    fPSConst(3, 1) = 0

    'Create the vertex buffer
    Call InitVB
    
    'Create the pixel shaders
    Call InitPixelShaders

    With dev
        
        'Lighting isn't needed, since the vertices are prelit
        Call .SetRenderState(D3DRS_LIGHTING, False)
        
        'Point the stream source to the vertex buffer that contains the vertices for rendering.
        Call .SetStreamSource(0, d3dvb, Len(VERTEX2TC(0)))
        
        'Set the vertex shader to the flexible vertex format the app describes.
        Call .SetVertexShader(FVFVERTEX2TC)
        
        'Set the pixel shader constans to the values that were set above.
        Call .SetPixelShaderConstant(0, fPSConst(0, 0), 2)
        
    End With

End Sub

Private Sub PaintMe()
    
'************************************************************************
'
' This sub is where all rendering happens. The vertices get moved to
' a new position, and then rendered.
'
' PARAMETERS:
'              None.
'
'************************************************************************
            
    Dim hr As Long
    Static bNotReady As Boolean
    
    If Not dev Is Nothing And Me.ScaleHeight > 0 And Not d3dvb Is Nothing Then
    
        'Call TestCooperativeLevel to see what state the device is in.
        hr = dev.TestCooperativeLevel
        
        If hr = D3DERR_DEVICELOST Then
            
            'If the device is lost, exit and wait for it to come back.
            bNotReady = True
            Exit Sub
        
        ElseIf hr = D3DERR_DEVICENOTRESET Then
            
            'The device is back, now it needs to be reset.
            hr = 0
            hr = ResetDevice
            If hr Then Exit Sub
            
            bNotReady = False
            
        End If
        
        'Make sure the app is ready and that the form's height is greater than 0
        If bNotReady Or Me.ScaleHeight < 1 Then Exit Sub
                
        With dev
                                    
            Call .BeginScene
            Call .Clear(0, ByVal 0, D3DCLEAR_TARGET, MakeRGB(0, 0, 255), 0, 0)

            'To just show the interpolation of each vertex color, remove all of the textures.
            Call .SetTexture(0, Nothing)
            Call .SetTexture(1, Nothing)
            
            'Move the vertices.
            Call MoveVBVerts(0, 0)
            'No pixel shader will be used for this one.
            Call .SetPixelShader(0)
            'Draw the two primitives.
            Call .DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)
                                    
            'Now set the two textures on the device.
            Call .SetTexture(0, d3dt(0))
            Call .SetTexture(1, d3dt(1))
            
            'Move the vertices
            Call MoveVBVerts(50, 0)
            'Use pixel shader 0
            Call .SetPixelShader(hPixelShader(0))
            Call .DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)
            
            'The rest of the calls just move the vertices to a new position, set
            'the next pixel shader, and render the two primitives.
            Call MoveVBVerts(0, 50)
            Call .SetPixelShader(hPixelShader(1))
            Call .DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)

            Call MoveVBVerts(50, 50)
            Call .SetPixelShader(hPixelShader(2))
            Call .DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)
        
            Call MoveVBVerts(0, 100)
            Call .SetPixelShader(hPixelShader(3))
            Call .DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)

            Call MoveVBVerts(50, 100)
            Call .SetPixelShader(hPixelShader(4))
            Call .DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)

            Call MoveVBVerts(0, 150)
            Call .SetPixelShader(hPixelShader(5))
            Call .DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)
    
            Call MoveVBVerts(50, 150)
            Call .SetPixelShader(hPixelShader(6))
            Call .DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2)
    
            Call .EndScene
            Call .Present(ByVal 0, ByVal 0, 0, ByVal 0)
        
        End With
        
    End If


End Sub

Private Function MakeRGB(r As Long, g As Long, b As Long) As Long

'************************************************************************
'
' This function takes three longs and packs them into a single long to
' create an RGB color. Each parameter has to be in the range of 0-255.
'
' PARAMETERS:
'           r   Long that represents the red component
'           g   Long that represents the green component
'           b   Long that represents the blue component
'
' RETURNS:
'           A long that.
'
'************************************************************************

    MakeRGB = b
    MakeRGB = MakeRGB Or (g * (2 ^ 8))
    MakeRGB = MakeRGB Or (r * (2 ^ 16))

End Function

Private Sub InitD3D()
    
'************************************************************************
'
' This sub initializes all the object variables, and creates the 3d device.
'
' PARAMETERS:
'            None.
'
'************************************************************************

    Dim d3ddm As D3DDISPLAYMODE
    
    'Turn off error handling, the app will handle any errors that occur.
    On Local Error Resume Next
        
    'Get a new D3DX object
    Set d3dx = New D3DX8
    'Get a new DirectX object
    Set dX = New DirectX8
    'Create a Direct3D object
    Set d3d = dX.Direct3DCreate()
    
    'Grab some information about the current display mode to see if the display
    'was switched to something that isn't supported.
    Call d3d.GetAdapterDisplayMode(D3DADAPTER_DEFAULT, d3ddm)
    
    'Make sure that the adapter is in a color bit depth greater than 8 bits per pixel.
    If d3ddm.Format = D3DFMT_P8 Or d3ddm.Format = D3DFMT_A8P8 Then
        
        'Device is running in some variation of an 8 bit format. Sample will have to exit at this point.
        MsgBox " For this sample to run, the primary display needs to be in 16 bit or higher color depth.", vbCritical
        Unload Me
        End
        
    End If
    
    With g_d3dpp
        
        'This app will run windowed.
        .Windowed = 1
        
        'The backbuffer format is unknown. Since this is windowed mode,
        'the app can just use whatever mode the device is in now.
        .BackBufferFormat = d3ddm.Format
        
        'When running windowed, the information contained in the
        'backbuffer is copied to the front buffer when Direct3DDevice.Present is called.
        .SwapEffect = D3DSWAPEFFECT_COPY
        
    End With
    
    'Create the device using the default adapter on the system using software vertex processing.
    Set dev = d3d.CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, Me.hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, g_d3dpp)
        
    'Check to make sure the device was created successfully. If not, exit.
    If dev Is Nothing Then
        MsgBox "Unable to initialize Direct3D. App will now exit."
        Unload Me
        End
    End If
    
End Sub

Private Sub Form_Paint()
    
    If d3dvb Is Nothing Then Exit Sub
    
    'Anytime the window receives a paint message, repaint the scene.
    Call PaintMe
    
End Sub

Private Sub Form_Resize()
    
    If d3dvb Is Nothing Then Exit Sub
    
    'Anytime the form is resized, redraw the scene.
    Call PaintMe
    
End Sub
        
Private Function ResetDevice() As Long

'***********************************************************************
'
' This subroutine is called whenever the app needs to be resized, or the
' device has been lost.
'
' Parameters:
'
'   None.
'
'***********************************************************************
        
    Dim d3ddm As D3DDISPLAYMODE
    
    On Local Error Resume Next
    
    'Call the sub that destroys the vertex buffer and shaders.
    Call DestroyAll
    
    'Set the width and height of the window
    Me.Width = 110 * Screen.TwipsPerPixelX
    Me.Height = 225 * Screen.TwipsPerPixelY
    
     'Grab some information about the current adapters display mode.
    'This may have changed since startup or the last D3DDevice8.Reset().
    Call d3d.GetAdapterDisplayMode(D3DADAPTER_DEFAULT, d3ddm)
        
    'Refresh the backbuffer format using the retrieved format.
     g_d3dpp.BackBufferFormat = d3ddm.Format
    
    'Now reset the device.
    Call dev.Reset(g_d3dpp)
    
    'If something happens during the reset, trap any possible errors. This probably failed
    'because the app doesn't have focus yet, but could fail is the user switched to an incompatible
    'display mode.
    
    If Err.Number Then
                
        'Make sure that the adapter is in a color bit-depth greater than 8 bits per pixel.
        If d3ddm.Format = D3DFMT_P8 Or d3ddm.Format = D3DFMT_A8P8 Then
            
            'Device is running in some variation of an 8 bit format. Sample will have to exit at this point.
            MsgBox " For this sample to run, the primary display needs to be in 16 bit or higher color depth.", vbCritical
            Unload Me
            End
            
        Else
            
            'More than likely the app just lost the display adapter. Keep spinning until the adapter becomes available.
            ResetDevice = Err.Number
            Exit Function
            
        End If
    End If
        
    'Now get the device ready again
    Call InitDevice
    
    'Redraw the scene
    PaintMe
    
End Function

Private Sub Form_Unload(Cancel As Integer)

    ' When the app is exiting, call the DestroyAll() function to clean up.
    Call DestroyAll
    
End Sub

Private Sub DestroyAll()

'***********************************************************************
'
' This sub releases all the objects and pixel shader handles.
'
' PARAMETERS:
'           None.
'
'***********************************************************************
    
    Dim i As Long
        
    On Error Resume Next
    
    'Loop through and delete all pixel shaders.
    For i = 0 To UBound(hPixelShader)
        If hPixelShader(i) Then
            Call dev.DeletePixelShader(hPixelShader(i))
            hPixelShader(i) = 0
        End If
    Next
    
    'Destroy the vertex buffer if it exists.
    If Not d3dvb Is Nothing Then Set d3dvb = Nothing
    
End Sub
