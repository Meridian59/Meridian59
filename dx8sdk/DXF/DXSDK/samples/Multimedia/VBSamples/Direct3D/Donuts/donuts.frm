VERSION 5.00
Begin VB.Form frmVBDonuts 
   Caption         =   "VBDonuts"
   ClientHeight    =   4440
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   5355
   Icon            =   "Donuts.frx":0000
   LinkTopic       =   "Form1"
   ScaleHeight     =   296
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   357
   StartUpPosition =   3  'Windows Default
End
Attribute VB_Name = "frmVBDonuts"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       Donuts.frm
'  Content:    This sample shows how 2d can be simulated with Direct3D using
'              transformed and lit vertices.
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

'Set the constant for full screen operation.
Const FULLSCREENWIDTH = 640
Const FULLSCREENHEIGHT = 480

'Set the number of sprites used in the sample.
Const NUM_SPRITES = 100

'Set the maximum velocity of the sprites.
Const MAX_VELOCITY = 1.5

'Flexible vertex format the describes transformed and lit vertices.
Const FVF = D3DFVF_XYZRHW Or D3DFVF_TEX1 Or D3DFVF_DIFFUSE Or D3DFVF_SPECULAR

'This structure describes a transformed and lit vertex.
Private Type TLVERTEX
    x As Single
    y As Single
    z As Single
    rhw As Single
    color As Long
    specular As Long
    tu As Single
    tv As Single
End Type

'A structure that defines all the needed properties
'of the Sprite.
Private Type typeSprite
    AnimDimensions As Single        'The dimensions of one frame of animation.
    AnimSpeed As Single             'The speed at which the animation occurs.
    AnimTheta As Single             'The current animation time count.
    FramesPerRow As Long            'How many animation frames are contained in one row in the texture.
    FramesTotal As Long             'Total number of frames for this animation.
    FrameCurrent As Single          'The current animation frame.
    RowOffset As Single             'Offset of the texture start for this sprite.
    SpriteDimensions As Single      'The dimensions of this sprite as will be displayed on the screen in pixels.
    SpriteNum As Long               'The index of this sprite.
    SpriteVerts(3) As TLVERTEX   'Vertex information for this sprite.
    Velocity As D3DVECTOR2          'The velocity of this sprite.
    Location As D3DVECTOR2          'The location of this sprite.
End Type

Dim m_Sprite(NUM_SPRITES) As typeSprite

'Dim the DirectX objects/structs the app uses.
Dim dx As DirectX8
Dim d3d As Direct3D8
Dim dev As Direct3DDevice8
Dim d3dx As D3DX8
Dim d3dtBackground As Direct3DTexture8
Dim d3dtSprite As Direct3DTexture8
Dim m_d3dpp As D3DPRESENT_PARAMETERS

'Dim the vertices for the background DirectX logo.
Dim m_MainVerts(3) As TLVERTEX

'Module level boolean that determines whether
'the app is in fullscreen or windowed.
Dim m_bWindowed As Boolean

'Module level variable to store the caps of the device.
Dim m_D3DCaps As D3DCAPS8

'Module level variable to store the display mode.
Dim m_d3ddm As D3DDISPLAYMODE

'Module level variables to store the window dimensions.
Dim m_lWindowWidth As Long
Dim m_lWindowHeight As Long

'Variables to store the render surface width and height.
Dim m_lClientWidth As Long
Dim m_lClientHeight As Long

'Module level variable to store app state.
Dim m_bRunning As Boolean

Private Sub Form_Load()

    'Show and size the form.
    With Me
        .Show
        .Height = .ScaleY(300, vbPixels, vbTwips)
        .Width = .ScaleX(400, vbPixels, vbTwips)
    End With
        
    'Seed the random number generator.
    Call Randomize
    
    'Call the sub to initialize the app.
    Call InitApp
        
    'Start the main loop of the sample.
    Call MainLoop

End Sub

Private Sub InitApp()

    '***********************************************************************
    '
    ' This sub initializes the application.
    '
    ' Parameters:
    '
    '           None.
    '
    '***********************************************************************
    
    Dim lErrNum As Long
            
    'Store the current window dimensions
    m_lWindowWidth = Me.ScaleWidth
    m_lWindowHeight = Me.ScaleHeight
    
    'Call the function that initializes the DirectX8, Direct3D8, and Direct3DDevice8 objects.
    lErrNum = InitD3D(dx, d3d, dev, Me.hwnd)
    If lErrNum Then
        'There was an error. We'll need to exit out at this point.
        Unload Me
    End If
    
    'Set the d3dx variable to a new D3DX8 object
    Set d3dx = New D3DX8
    
    'Call the function to load any textures.
    Call InitTextures
    
End Sub

Private Sub MainLoop()
    
    '***********************************************************************
    '
    ' This sub is the main loop for the sample.
    '
    ' Parameters:
    '
    '           None.
    '
    '***********************************************************************
                
    m_bRunning = True
    
    Do While m_bRunning
        
        Call RenderScene
        DoEvents
        
    Loop
                
    'Exiting app now
    Unload Me
    
End Sub

Private Sub RenderScene()

    '***********************************************************************
    '
    ' This sub handles the rendering of the scene.
    '
    ' Parameters:
    '
    '   None.
    '
    '***********************************************************************
    
    On Local Error Resume Next
            
    Dim hr As Long
    
    'Call TestCooperativeLevel to see what state the device is in.
    hr = dev.TestCooperativeLevel
    
    If hr = D3DERR_DEVICELOST Then
        
        'If the device is lost, exit and wait for it to come back.
        Exit Sub
    
    ElseIf hr = D3DERR_DEVICENOTRESET Then
            
        'The device became lost for some reason (probably an alt-tab) and now
        'Reset() needs to be called to try and get the device back.
        hr = 0
        hr = ResetDevice()
        
        'If the device failed to be reset, exit the sub.
        If hr Then Exit Sub
    
    End If
    
    'Make sure the app isn't minimized.
    If Me.WindowState <> vbMinimized Then
        
        'The app is ready for rendering.
        With dev
                                    
            'Clear the back buffer
            Call .Clear(0, ByVal 0&, D3DCLEAR_TARGET, &HFF, 0, 0)
            
            'Begin the 3d scene
            Call .BeginScene
            
            'Set the background texture on the device
            Call .SetTexture(0, d3dtBackground)
            
            'Draw the 2 polygons that make up the background
            Call .DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, m_MainVerts(0), Len(m_MainVerts(0)))
            
            'Call the sub that renders the sprites
            Call RenderSprites
            
            'End the scene
            Call .EndScene
            
            'Draw the graphics to the front buffer.
            Call .Present(ByVal 0&, ByVal 0&, 0, ByVal 0&)
            
        End With
    
    End If
    
End Sub

Private Sub RenderSprites()

    '***********************************************************************
    '
    ' This sub handles the rendering and animation of the sprites.
    '
    ' Parameters:
    '
    '   None.
    '
    '***********************************************************************
    
    Dim i As Long
    Dim TexX As Single, TexY As Single
    
    With dev
        
        'Set the Sprite texture on the device
        Call .SetTexture(0, d3dtSprite)
        
        'Make sure the device supports alpha blending
        If (m_D3DCaps.TextureCaps And D3DPTEXTURECAPS_ALPHA) Then
            
            'It does, so turn alpha blending on
            Call .SetRenderState(D3DRS_ALPHABLENDENABLE, 1)
        
        End If
                                               
       For i = 0 To UBound(m_Sprite)

            'Call the sub that updates the Sprite
            Call UpdateSprite(i)

            'Draw the 2 polygons that make up the Sprite
            Call .DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, m_Sprite(i).SpriteVerts(0), Len(m_Sprite(i).SpriteVerts(0)))

        Next
        
        
        'If alpha blending was turned on
         If .GetRenderState(D3DRS_ALPHABLENDENABLE) Then
            
            'Turn it back off
            Call .SetRenderState(D3DRS_ALPHABLENDENABLE, 0)
            
        End If
        
    End With
    
End Sub

Private Sub UpdateSprite(ByVal index As Long)
    
    '***********************************************************************
    '
    ' This sub updates the sprites texture coordinates and position.
    ' Direc3DDevice8.
    '
    ' Parameters:
    '
    ' [IN]
    '       index:      The index of the Sprite to update.
    '
    '***********************************************************************
        
    Dim TexX As Single, TexY As Single
    
    With m_Sprite(index)
                
        'Update the theta for this sprite.
        .AnimTheta = .AnimTheta + .AnimSpeed
        
        'If the theta count is greater than one, advance the animation frame.
        If .AnimTheta > 1 Then
            
            'Reset theta
            .AnimTheta = 0
            
            'Advance the animation frame
            .FrameCurrent = .FrameCurrent + 1
            If .FrameCurrent >= .FramesTotal Then
                .FrameCurrent = 0
            End If
            
        End If
        
        'Calculate the updated texture coordinates for this Sprite
        TexY = ((.FrameCurrent \ .FramesPerRow) * .AnimDimensions) + .RowOffset
        TexX = (.FrameCurrent Mod .FramesPerRow) * .AnimDimensions
        
        'Update the position of the Sprite
        .Location.x = .Location.x + .Velocity.x
        .Location.y = .Location.y + .Velocity.y
        
        .SpriteVerts(0).x = .Location.x
        .SpriteVerts(0).y = .Location.y + .SpriteDimensions
        .SpriteVerts(0).tu = TexX
        .SpriteVerts(0).tv = TexY + .AnimDimensions
        
        .SpriteVerts(1).x = .Location.x
        .SpriteVerts(1).y = .Location.y
        .SpriteVerts(1).tu = TexX
        .SpriteVerts(1).tv = TexY
                
        .SpriteVerts(2).x = .Location.x + .SpriteDimensions
        .SpriteVerts(2).y = .Location.y + .SpriteDimensions
        .SpriteVerts(2).tu = TexX + .AnimDimensions
        .SpriteVerts(2).tv = TexY + .AnimDimensions
        
        .SpriteVerts(3).x = .Location.x + .SpriteDimensions
        .SpriteVerts(3).y = .Location.y
        .SpriteVerts(3).tu = TexX + .AnimDimensions
        .SpriteVerts(3).tv = TexY
        
        'Check to see if the Sprite hit a wall. If it did, reverse its velocity.
        If .Location.x <= 0 Then
            .Velocity.x = -1 * .Velocity.x
        ElseIf .Location.x + .SpriteDimensions >= m_lClientWidth Then
            .Velocity.x = -1 * .Velocity.x
        End If
        
        If .Location.y <= 0 Then
            .Velocity.y = -1 * .Velocity.y
        ElseIf .Location.y + .SpriteDimensions >= m_lClientHeight Then
            .Velocity.y = -1 * .Velocity.y
        End If

    End With

End Sub

Private Function InitD3D(dx As DirectX8, d3d As Direct3D8, dev As Direct3DDevice8, ByVal hwnd As Long, Optional ByVal bWindowed As Boolean = True) As Long

    '***********************************************************************
    '
    ' This function creates the following objects: DirectX8, Direct3D8,
    ' Direc3DDevice8.
    '
    ' Parameters:
    '
    ' [IN]
    '       hwnd:       Handle to a window that will be used as the render target
    '       bWindowed:  Optional boolean argument that initializes either full screen
    '                   or windowed. Default is windowed.
    ' [OUT]
    '       dx:         Pass in an uninitialized DirectX8 object.
    '       d3d:        Pass in an uninitialized Direct3D8 object.
    '       dev:        Pass in an uninitialized Direct3DDevice8 object.
    '
    ' Return value:
    '
    '     If an error occurs, it returns the Direct3D error number. In the
    '     case that no fullscreen format was found, it returns D3DERR_INVALIDDEVICE.
    '
    '***********************************************************************
    
    Dim DevType As CONST_D3DDEVTYPE
    Dim i As Long, lCount As Long, lErrNum As Long, format As Long
    Dim bFoundMode As Boolean
        
    'Turn off error checking. The app will check for errors and handle them.
    On Local Error Resume Next
    
    'Store the window mode that was passed in
    m_bWindowed = bWindowed
        
    'Initiazlize the DirectX8 object
    Set dx = New DirectX8
        
    'Check to make sure that the dx object was created successfully.
    If Err.Number Then
    
        'There were problems creating the dx object. Return the error number.
        InitD3D = Err.Number
        Exit Function
        
    End If
    
    'Create the Direct3D object
    Set d3d = dx.Direct3DCreate
    
    'Check to make sure that the d3d object was created successfully.
    If Err.Number Then
    
        'There were problems creating the d3d object. Return the error number,
        InitD3D = Err.Number
        Exit Function
        
    End If
    
    'We'll start by attempting to create a HAL device. This variable
    'will hold the final type of device that we create after we check
    'some capabilities.
    DevType = D3DDEVTYPE_HAL
    
    'Get the capabilities of the Direct3D device that we specify. In this case,
    'we'll be using the adapter default (the primiary card on the system).
    Call d3d.GetDeviceCaps(D3DADAPTER_DEFAULT, DevType, m_D3DCaps)
    
    'Check for errors. If there is an error, the card more than likely doesn't support at least DX7,
    'so get the caps of the reference device instead.
    If Err.Number Then
        
        Err.Clear
        DevType = D3DDEVTYPE_REF
        Call d3d.GetDeviceCaps(D3DADAPTER_DEFAULT, DevType, m_D3DCaps)
        
        'If there is *still* an error, then the driver has problems. We'll
        'have to exit at this point, because there isn't anything else we can
        'do.
        If Err.Number Then
            InitD3D = D3DERR_NOTAVAILABLE
            Exit Function
        End If
        
    End If
    
    'Grab some information about the current display mode.
    Call d3d.GetAdapterDisplayMode(D3DADAPTER_DEFAULT, m_d3ddm)
    
    'Now we'll go ahead and fill the D3DPRESENT_PARAMETERS type.
    With m_d3dpp
        
        If bWindowed Then
                        
            'Make sure that the adapter is in a color bit-depth greater than 8 bits per pixel.
            If m_d3ddm.format = D3DFMT_P8 Or m_d3ddm.format = D3DFMT_A8P8 Then
                'Device is running in some variation of an 8 bit format
                MsgBox " For this sample to run, the primary display needs to be in 16 bit or higher color depth.", vbCritical
                InitD3D = D3DERR_INVALIDDEVICE
                Exit Function
            Else
                'Device is greater than 8 bit. Set the format variable to the current display format.
                format = m_d3ddm.format
            End If
            
            'For windowed mode, we just discard any information instead of flipping it.
            .SwapEffect = D3DSWAPEFFECT_DISCARD
                        
            'Set windowed mode to true.
            .Windowed = 1
                        
        Else
        
            'Call the sub to find the first suitable fullscreen format
            lErrNum = FindMode(FULLSCREENWIDTH, FULLSCREENHEIGHT, format)
            
            'If unable to find a suitable mode, the app will have to exit.
            If lErrNum Then
                MsgBox " Unable to find a compatible format to run the sample.", vbCritical
                InitD3D = D3DERR_INVALIDDEVICE
                Exit Function
            End If
        
            'We need the backbuffer to flip with the front for fullscreen. This
            'flag enables this.
            .SwapEffect = D3DSWAPEFFECT_FLIP
                                    
            'Set the width and height
            .BackBufferWidth = FULLSCREENWIDTH
            .BackBufferHeight = FULLSCREENHEIGHT
            
        End If
        
        'Set the backbuffer format
        .BackBufferFormat = format

    End With
    
    'Try to create the device now that we have everything set.
    Set dev = d3d.CreateDevice(D3DADAPTER_DEFAULT, DevType, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, m_d3dpp)
    
    'If the creation above failed, try to create a REF device instead.
    If Err.Number Then
        
        Err.Clear
        Set dev = d3d.CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, m_d3dpp)
        
        If Err.Number Then
                    
            'The app still hit an error. Both HAL and REF devices weren't created. The app will have to exit at this point.
            InitD3D = Err.Number
            Exit Function
            
        End If
    End If
    
    'Store the client dimensions
    If m_bWindowed Then
        m_lClientWidth = Me.ScaleWidth
        m_lClientHeight = Me.ScaleHeight
    Else
        m_lClientWidth = FULLSCREENWIDTH
        m_lClientHeight = FULLSCREENHEIGHT
    End If
    
    If InitDevice(dev, hwnd) Then
        
        MsgBox "Unable to initialize the device"
        Unload Me
        
    End If

End Function

Private Function InitDevice(dev As Direct3DDevice8, hwnd As Long) As Long

    '***********************************************************************
    '
    ' This function initializes the device with some renderstates, and also
    ' sets up the viewport, camera, and world.
    '
    ' Parameters:
    '
    ' [IN]
    '       dev:    An existing Direct3DDevice8 object
    '       m_d3dpp:  A filled D3DPRESENT_PARAMETERS type
    '       hwnd:   Handle to the target window
    '
    '
    ' Return value:
    '     If an error occurs, it returns D3DERR_INVALIDCALL.
    '
    '***********************************************************************

    'On Local Error Resume Next
    Call InitGeometry
    
    With dev
                
        'Set the vertex shader to an FVF that contains texture coords,
        'and transformed and lit vertex coords.
        Call .SetVertexShader(FVF)
        
        'Turn off lighting
        Call .SetRenderState(D3DRS_LIGHTING, 0)
                                
        'Set the render state that uses the alpha component as the source for blending.
        Call .SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA)
        
        'Set the render state that uses the inverse alpha component as the destination blend.
        Call .SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA)
        
    End With
        
    If Err.Number Then InitDevice = D3DERR_INVALIDCALL
    
End Function

Private Sub InitGeometry()

    '***********************************************************************
    '
    ' This sub initializes the vertices for all the needed polygons.
    '
    ' Parameters:
    '           None.
    '
    '***********************************************************************
        
    Dim sDimensions As Single
    Dim i As Long
    Static bInit As Boolean
    
    ' All the polygons that this sample use are made of two triangles that create a rectangle.
    ' The textures are painted on these two polygons to create the look of a 2d sprite.
    ' All of the polygons are transformed and lit, meaning that Direct3D will perform no
    ' lighting calculations, and no coordinate transformation. The application is responsible
    ' for doing all of these calculations. Since this is just a 2d simulation, it's very easy
    ' to set up the polygons and transform them manually.
    ' This illustration shows the placement of each vertex (vn) to draw the rectangle. Notice the
    ' order that the vertices are placed. This follows the clockwise winding order rule for culling
    ' polygons. If the order was reversed, the polygon wouldn't be rendered by Direct3D. See the "3-D
    ' Coordinate Systems and Geometry" section in the docs for more info.
    
    ' * v1      * v3
    ' |\        |
    ' |  \      |
    ' |    \    |
    ' |      \  |
    ' |        \|
    ' * v0      * v2
    
    'Initialize the 2 polygons that will display the DirectX logo
    With m_MainVerts(0)
        
        'X and Y are the familiar XY values in screen space that this vertex will be placed.
        'This one is going in the bottom left corner of the screen.
        .x = 0: .y = m_lClientHeight
        
        'This sets up the texture coordinates for this vertex in the polygon.
        'tu is the X of the texture, tv is the Y of the texture. Texture coordinates
        'are from 0 to 1, 0 being all the way to the left or top, and 1 being all the
        'way to the right or bottom, depending on whether it is the tu or tv element.
        .tu = 0: .tv = 1
        
        'rhw is the value that D3D uses to produce scaling. Since this app
        'won't be doing any scaling, this value needs to be 1.
        .rhw = 1
        
        'Since the app will handle all lighting, the color value will be used
        'to light the polygon. For this app, the polygon will be fully lit.
        .color = &HFFFFFF
    End With
    
    'The rest of the vertices follow the same format, but are placed in different XY coordinates.
    
    With m_MainVerts(1)
        .x = 0: .y = 0
        .tu = 0: .tv = 0
        .rhw = 1
        .color = &HFFFFFF
    End With
    With m_MainVerts(2)
        .x = m_lClientWidth: .y = m_lClientHeight
        .tu = 1: .tv = 1
        .rhw = 1
        .color = &HFFFFFF
    End With
    With m_MainVerts(3)
        .x = m_lClientWidth: .y = 0
        .tu = 1: .tv = 0
        .rhw = 1
        .color = &HFFFFFF
    End With

    For i = 0 To UBound(m_Sprite)
                
        With m_Sprite(i)
                    
            'If this is the first time the sub is called.
            If Not bInit Then
            
                'Choose a random Sprite
                .SpriteNum = Int((3) * Rnd)
                                        
                'Set the sprites properties accordingly
                If .SpriteNum = 0 Then
                    .FramesPerRow = 8
                    .FramesTotal = 29
                    .RowOffset = 0
                    .AnimDimensions = 0.125
                    .SpriteDimensions = 40
                ElseIf .SpriteNum = 1 Then
                    .FramesPerRow = 16
                    .FramesTotal = 39
                    .RowOffset = 0.5
                    .AnimDimensions = 0.0625
                    .SpriteDimensions = 15
                ElseIf .SpriteNum = 2 Then
                    .FramesPerRow = 16
                    .FramesTotal = 39
                    .RowOffset = 0.6875
                    .AnimDimensions = 0.0625
                    .SpriteDimensions = 15
                End If
            
                'Choose a random starting location, velocity, and animation frame
                .Location.x = (m_lClientWidth - .SpriteDimensions) * Rnd
                .Location.y = (m_lClientHeight - .SpriteDimensions) * Rnd
                .Velocity.x = (((MAX_VELOCITY - -MAX_VELOCITY) * Rnd) + -MAX_VELOCITY)
                .Velocity.y = (((MAX_VELOCITY - -MAX_VELOCITY) * Rnd) + -MAX_VELOCITY)
                .FrameCurrent = Int(.FramesTotal * Rnd)
                
                'Calculate the speed at which the animation should occurr. Based on the velocity of the sprite.
                'The higher the velocity, the faster the animation.
                .AnimSpeed = ((Abs(.Velocity.x) + Abs(.Velocity.y)) / 4)
                                
            Else
                
                'The window was resized. Make sure sprites are still in view, move them so they are if neccessary.
                If .Location.x + .SpriteDimensions > m_lClientWidth Then
                    .Location.x = m_lClientWidth - .SpriteDimensions - 1
                End If
                
                If .Location.y + .SpriteDimensions > m_lClientHeight Then
                    .Location.y = m_lClientHeight - .SpriteDimensions - 1
                End If
                                
            End If
            
            'Create the vertices for the Sprite
            With .SpriteVerts(0)
                .x = 0: .y = 0
                .tu = 0: .tv = m_Sprite(i).AnimDimensions
                .rhw = 1
                .color = &HFFFFFF
            End With
            With .SpriteVerts(1)
                .x = 0: .y = m_Sprite(i).SpriteDimensions
                .tu = 0: .tv = 0
                .rhw = 1
                .color = &HFFFFFF
            End With
            With .SpriteVerts(2)
                .x = m_Sprite(i).SpriteDimensions: .y = 0
                .tu = m_Sprite(i).AnimDimensions: .tv = m_Sprite(i).AnimDimensions
                .rhw = 1
                .color = &HFFFFFF
            End With
            With .SpriteVerts(3)
                .x = m_Sprite(i).SpriteDimensions: .y = m_Sprite(i).SpriteDimensions
                .tu = m_Sprite(i).AnimDimensions: .tv = 0
                .rhw = 1
                .color = &HFFFFFF
            End With
            
        End With
        
    Next
    
    'The geometry is initialized. No need to randomize again.
    bInit = True
    
End Sub

Private Sub InitTextures()
    
    '***********************************************************************
    '
    ' This sub loads any textures needed. If for some reason this sub doesn't
    ' succeed, we'll just exit the app, because it won't run without the
    ' textures being loaded.
    '
    ' Parameters:
    '           None.
    '
    '***********************************************************************
    
    On Local Error Resume Next
        
    Dim sFile As String
    
    'Locate the path to the media
    sFile = FindMediaDir("dx5_logo.bmp")
    If sFile = "" Then
        sFile = App.Path & "\" & "dx5_logo.bmp"
    Else
        sFile = sFile & "dx5_logo.bmp"
    End If
    
    'Check to make sure the media was found
    If Dir(sFile) = vbNullString Then
        MsgBox "Unable to locate sample media."
        Unload Me
    End If
    
    'Load the background texture
    Set d3dtBackground = d3dx.CreateTextureFromFile(dev, sFile)
        
    'Locate the path to the next media file.
    sFile = FindMediaDir("donuts1.bmp")
    If sFile = "" Then
        sFile = App.Path & "\" & "donuts1.bmp"
    Else
        sFile = sFile & "donuts1.bmp"
    End If
    
    'Check to make sure the media was found
    If Dir(sFile) = vbNullString Then
        MsgBox "Unable to locate sample media."
        Unload Me
    End If

    
    'Load the Sprite texture. We need to get alpha information embedded into this
    'surface, so we'll call the more complex CreateTextureFromFileEx() method instead.
    'The main thing we need to do is just let it know we want to use black as the
    'alpha channel. We do this by passing &HFF000000 to the method, and it fills in
    'the high order byte of any pixel that contains black with full alpha so that it
    'becomes transparent when rendered with alpha blending enabled.
    
    Set d3dtSprite = d3dx.CreateTextureFromFileEx( _
                                                    dev, _
                                                    sFile, _
                                                    D3DX_DEFAULT, _
                                                    D3DX_DEFAULT, _
                                                    D3DX_DEFAULT, _
                                                    0, _
                                                    D3DFMT_UNKNOWN, _
                                                    D3DPOOL_MANAGED, _
                                                    D3DX_FILTER_POINT, _
                                                    D3DX_FILTER_POINT, _
                                                    &HFF000000, _
                                                    ByVal 0, _
                                                    ByVal 0 _
                                                    )
            
    If Err.Number Then
        
        'Something happened while loading the texture.
        MsgBox "Error loading texture. Error number: " & Err.Number
        Unload Me
        
    End If
    
End Sub

Private Sub SwitchWindowMode()

    '***********************************************************************
    '
    ' This sub switches the current display mode between windowed/fullscreen.
    ' If it runs into an error, it just exits, leaving the display mode in
    ' its current state.
    '
    ' Parameters:
    '           None.
    '
    '***********************************************************************
    
    Dim d3dppEmpty As D3DPRESENT_PARAMETERS
    Dim format As Long
    Dim lErrNum As Long
    
    On Local Error Resume Next
    
    If m_bWindowed Then
                                    
        'Grab a valid format for this device. If a format
        'for the requested resolution wasn't found, exit the sub.
        If FindMode(FULLSCREENWIDTH, FULLSCREENHEIGHT, format) <> 0 Then Exit Sub
                
        'Store the current window mode format
        Call d3d.GetAdapterDisplayMode(D3DADAPTER_DEFAULT, m_d3ddm)
        
        'The app is running windowed currently, switch to fullscreen.
        m_bWindowed = False
                
        'Set the present parameters for running full screen
        m_d3dpp = d3dppEmpty
        
        With m_d3dpp
            .SwapEffect = D3DSWAPEFFECT_FLIP
            .BackBufferFormat = format
            .BackBufferWidth = FULLSCREENWIDTH
            .BackBufferHeight = FULLSCREENHEIGHT
            .Windowed = 0
        End With
        
        'Store the client dimensions
        m_lClientWidth = FULLSCREENWIDTH
        m_lClientHeight = FULLSCREENHEIGHT
        
        'Reset the device to the new mode
        lErrNum = ResetDevice
        
        'If there is an error resetting the device,
        'just exit the sub.
        If lErrNum Then
            'Store the client dimensions
            m_lClientWidth = Me.ScaleWidth
            m_lClientHeight = Me.ScaleHeight
            m_bWindowed = True
            Exit Sub
        End If
                
    Else
                                                                                                    
        'Set the present params to reflect windowed operation.
        m_d3dpp = d3dppEmpty
        
        With m_d3dpp
            .SwapEffect = D3DSWAPEFFECT_DISCARD
            .BackBufferFormat = m_d3ddm.format
            .Windowed = 1
        End With
        
        'Reset the device to the new mode
        lErrNum = ResetDevice
        
        'If there is an error, just exit the sub
        If lErrNum Then
            m_bWindowed = False
            Exit Sub
        End If
          
        'Now get the device ready again
        Call InitDevice(dev, Me.hwnd)
              
        'Resize the form to the size it was previous to going fullscreen.
        Me.Width = m_lWindowWidth * Screen.TwipsPerPixelX
        Me.Height = m_lWindowHeight * Screen.TwipsPerPixelY
        
        'The app is now running windowed
        m_bWindowed = True
        
        'Store the client dimensions
        m_lClientWidth = Me.ScaleWidth
        m_lClientHeight = Me.ScaleHeight
        
        'Resize the window to the old size now.
        Call Form_Resize
        
    End If
    
End Sub

Private Sub ResizeWindow()

    '***********************************************************************
    '
    ' This subroutine is called whenever the form is resized. It resets the
    ' device to the new size, and re-inits the device.
    '
    ' Parameters:
    '
    '   None.
    '
    '***********************************************************************
    
    Dim d3dppEmpty As D3DPRESENT_PARAMETERS
            
    m_lWindowWidth = Me.ScaleWidth
    m_lWindowHeight = Me.ScaleHeight
    m_lClientWidth = m_lWindowWidth
    m_lClientHeight = m_lWindowHeight
    
    'Reset the device to the new mode
    Call ResetDevice
    
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
        
    On Local Error Resume Next
    
    Call dev.Reset(m_d3dpp)
    
    If Err.Number Then
        ResetDevice = Err.Number
        Exit Function
    End If
    
    'Now get the device ready again
    Call InitDevice(dev, Me.hwnd)

End Function
Private Function FindMode(ByVal w As Long, ByVal h As Long, fmt As Long) As Long
    
    '***********************************************************************
    '
    ' This function returns a valid back buffer format for the width and height passed in.
    '
    ' Parameters:
    '
    ' [IN]
    '      w is the width of the mode being sought
    '      h is the height of the mode being sought
    '
    ' [OUT]
    '     fmt will be filled in with a valid CONST_D3DFORMAT
    '
    ' Return value:
    '     If a valid format was not found, D3DERR_INVALIDDEVICE is returned.
    '     If an error occurs, it returns D3DERR_INVALIDCALL.
    '***********************************************************************
    
    
    Dim i  As Long, lCount As Long
    Dim d3ddm As D3DDISPLAYMODE
    Dim bFoundMode As Boolean
    
    i = 0
    
    'Get the number of adapter modes this adapter supports.
    lCount = d3d.GetAdapterModeCount(D3DADAPTER_DEFAULT) - 1
    
    'If we encounter an error, return an error code and exit the function.
    If Err.Number Then
        FindMode = D3DERR_INVALIDCALL
        Exit Function
    End If
    
    'Next, loop through all the display modes until we find one
    'that matches the parameters passed in.
    For i = 0 To lCount
        
        Call d3d.EnumAdapterModes(D3DADAPTER_DEFAULT, i, d3ddm)
        
        'Again, catch any unexpected errors.
        If Err.Number Then
            FindMode = Err.Number
            Exit Function
        End If
        
        'Check to see if this mode matches what is being sought.
        If d3ddm.Width = w And d3ddm.Height = h Then
            
            'Now see if this mode is either a 32bpp or 16bpp mode
            If d3ddm.format = D3DFMT_R8G8B8 Or _
                d3ddm.format = D3DFMT_R5G6B5 Then
                
                'We've found a suitable display. Set the flag
                'to reflect this, and exit. No need to look further.
                bFoundMode = True
                
                'Set the fmt to the format that was found.
                fmt = d3ddm.format
                
                Exit For
            End If
        End If
    Next
        
    If bFoundMode Then
        
        'Everything checked out OK
        Exit Function
        
    Else
        
        'Return an error
        FindMode = D3DERR_INVALIDDEVICE
        
    End If
    
End Function

Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
    
    If (Shift And vbAltMask) And KeyCode = vbKeyReturn Then
        
        'User wants to switch from fullscreen/windowed mode
        Call SwitchWindowMode
            
    ElseIf KeyCode = vbKeyEscape Then
        
        'User wants to exit the app
        m_bRunning = False
        
    End If
                        
End Sub

Private Sub Form_Resize()

    'Call the subroutine that resizes the backbuffer on the device.
    'Make sure the device exists, and the app is windowed.
    If Not dev Is Nothing And m_bWindowed Then
        
    'Make sure the app isn't minimized.
    If Me.WindowState <> vbMinimized Then
        
        'Make sure the app isn't resized to the point where the sprites could get stuck.
        If Me.ScaleHeight < 100 Or Me.ScaleWidth < 100 Then
            Me.Width = Screen.TwipsPerPixelX * 100
            Me.Height = Screen.TwipsPerPixelY * 100
        End If
        
            Call ResizeWindow
        End If
        
    End If
    
End Sub

Private Sub Form_Unload(Cancel As Integer)
    
    'We need to terminate the app using the End statement,
    'otherwise the form will reload since the app is running
    'in a loop with DoEvents.
    End
    
End Sub

