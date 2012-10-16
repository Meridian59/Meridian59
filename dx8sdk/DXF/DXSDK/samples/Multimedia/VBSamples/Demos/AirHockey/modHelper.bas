Attribute VB_Name = "modAirHockey"
Option Explicit

Public dx As New DirectX8

Public Type HockeyPlayerInfo
    Score As Long 'Current score of this player
    PlayerName As String ' The name of the player
    Latency As Long 'Average latency (ping time) of this player
End Type

'Declare for timeGetTime
Public Declare Function timeGetTime Lib "winmm.dll" () As Long

'Registry constants (for saving and retreiving information)
Public Const gsKeyName As String = "vbAirHockey"
Public Const gsSubKey As String = "Defaults"
Public Const gsSubKeyAudio As String = "Audio"
Public Const gsSubKeyInput As String = "Input"
Public Const gsSubKeyGraphics As String = "Graphics"

Public Const glMaxPuckSpeedConstant As Long = 10.23

'The wall locations, use these for easier collision detection
Public Const gnSideRightWallEdge As Single = -5
Public Const gnSideLeftWallEdge As Single = 5
Public Const gnNearWallEdge As Single = 9.92
Public Const gnFarWallEdge As Single = -9.92
'We also need the dimensions for the 'scoring area' so we can tell when we score
Public Const gnScoringEdgeLeft As Single = 1.35
Public Const gnScoringEdgeRight As Single = -1.35

Public Const gnPuckScored As Single = 1.15
'Radius constants for the puck and paddle
Public Const gnPuckRadius As Single = 0.46046
Public Const gnPaddleRadius As Single = 0.6
'ComputerAI Maximum velocity
Public Const gnComputerMaximumVelocity As Single = 0.43
'Winning score
Public Const glDefaultWinningScore As Long = 7

'We will ensure that we have at least a particular number of physics calculations per second
'We will lower frame rate to ensure we can calculate these physic calculations if necessary
'Number of physics calculations per second
Public Const glNumPhysicCalcPerSec As Long = 100
'Ticks between physic calcs
Public Const glNumTickForPhysicCalcs As Long = 1000 \ glNumPhysicCalcPerSec

'Minimum delay before allowing another paddle hit
Public Const glMinDelayPaddleHit = 100
'Delay time (ms) before 'helper' text appears
Public Const glDefaultDelayTime As Long = 3000
'Delay time (ms) before 'helper' text disappears
Public Const glDefaultDelayTimeGone As Long = 10000
Public Const gnVelocityBoost As Single = 1.1

'The objects that can appear in the scene
Public goCamera As cCamera 'Doesn't really appear in the scene, but it does control what we see in the scene
Public goPuck As cPuck 'The puck.  Pretty important
Public goPaddle(1) As cPaddle 'There are two paddles
Public goTable As cTable 'The table will never have a destination or a velocity, but we may need to move it's position
Public goRoom As cRoom 'The room information
Public goAudio As cAudio 'All of the audio information will be stored here
Public goInput As cInput 'All of the input (mouse,keyboard, joystick,etc) will be stored here
Public goFade As cFade 'The 'Fading' class
'Text variables
Public goTextLittle As cText
Public goTextBig As cText
'Main 'Select device' form
Public goDev As frmSelectDevice

'Which paddle am I controlling (Used mainly for multiplayer mode)
Public glMyPaddleID As Long
Public gfScored As Boolean 'Is the puck in the scored state
Public gfMultiplayer As Boolean 'Is this a multiplayer game
Public gfHost As Boolean 'Am I the host of this game?
Public gfGameCanBeStarted As Boolean 'Can the game be started
Public gPlayer(1) As HockeyPlayerInfo 'Current information of all the players
Public gfRecentlyHitPaddle As Boolean 'Have we recently hit a paddle?
'Current time for all objects
Public glTimeCompPaddle As Long
'Is the game over (ie, has someone won the game)
Public gfGameOver As Boolean
'The user defined winning score
Public glUserWinningScore As Long
Public glPaddleCollideTime As Long
'We'll maintain a slight dampening factor for realism as the puck bounces off
'the wall
Public gnVelocityDamp As Single
'Paddle mass
Public gnPaddleMass As Single
'Time the puck was last scored
Public glTimePuckScored As Long
'Time the game was over
Public glTimeGameOver As Long
'Time the F1 help was displayed
Public glTimeNoRoom As Long
'Is the system paused
Public gfSystemPause As Boolean

Public gfDrawRoomText As Boolean
Public glScreenWidth As Long
Public glScreenHeight As Long

Public gfObjectsLoaded As Boolean

'Extra misc vars
Public gfWireFrame As Boolean

Public Sub MainGameLoop()
    Dim lTime As Long
    Dim lLastPhysicsTime As Long
    
    'Start the render loop
    lTime = timeGetTime
    Do While True
        Do While Not gfSystemPause
            'In each frame we need to do a few things
            If (timeGetTime - lTime > 100) And (Not gfDrawRoomText) And (goRoom.DrawRoom) Then
                'We want to maintain a reasonable frame rate (10fps is on
                'the low end), so if we start taking too long between updates,
                'tell them they can get rid of the room
                gfDrawRoomText = True
                glTimeNoRoom = timeGetTime
            End If
            lTime = timeGetTime
            'Check to see if the game is over
            CheckGameOver
            'We need to update any objects that are in the scene
            UpdateObjects
            'Get and handle any input
            goInput.GetAndHandleInput goPaddle(glMyPaddleID), goPuck
            If (Not gfScored) And (Not gfGameOver) Then
                'Next we need to check for any collisions that may have happened
                goPuck.CheckCollisions goPaddle, goAudio
                If Not gfMultiplayer Then 'Only on single player mode
                    'Let the Computer AI do it's thing
                    goPaddle(Abs(glMyPaddleID - 1)).DoComputerAI goPuck
                End If
            End If
            'We need to update the game state on the other machine
            If gfMultiplayer Then
                UpdateNetworkSettings
            Else
                If gfScored Then goPaddle(Abs(glMyPaddleID - 1)).UpdateTime
            End If
            'Only redraw the world if we're keeping up with our physic calculations
            If timeGetTime - lLastPhysicsTime < glNumTickForPhysicCalcs Then
                'We should fade if necessary
                If goFade.AmFading Then goFade.UpdateFade goPuck, goPaddle, goTable, goRoom
                'Now we need to render the frame
                Render
            End If
            lLastPhysicsTime = timeGetTime
            DoEvents
        Loop
        'Now give the CPU a chance
        DoEvents
    Loop
End Sub

Public Sub LoadDefaultStartPositions()
    'Our camera will start away from the table, and zoom in on it
    With goCamera
        .Position = vec3(0, 35, -40)
        .LastPosition = .Position
    End With
    goCamera.SetCameraPosition 0, glMyPaddleID
    'The puck's initial position should be on top of the table
    With goPaddle(0)
        .Position = vec3(0, 2.5, -6.8)
        .LastPosition = .Position
    End With
    With goPaddle(1)
        .Position = vec3(0, 2.5, 6.8)
        .LastPosition = .Position
    End With
    With goTable
        .Position = vec3(0, -5, 0)
    End With
    goPuck.DefaultStartPosition
    
End Sub

Public Sub UpdateObjects()
    'We need a timer for each of the objects we're updating
    Dim lCount As Long
    
    If gfMultiplayer And gfNoSendData Then 'Uh oh!  We've been disconnected sometime, no need to process anything
        Exit Sub
    End If
    'Update the camera's position based on it's velocity
    goCamera.UpdatePosition
    'Update the puck's position
    goPuck.UpdatePosition
End Sub

Public Sub CheckGameOver()
    Dim lCount As Long
    
    If gfGameOver Then Exit Sub
    For lCount = 0 To 1
        If gPlayer(lCount).Score >= glUserWinningScore Then
            'Make sure we're leading the other player by 2 or more
            If gPlayer(lCount).Score > gPlayer(Abs(lCount - 1)).Score + 1 Then
                gfGameOver = True
                glTimeGameOver = timeGetTime
            End If
        End If
    Next
End Sub

Public Sub ShowStartup()

    'Now 'zoom' in with our camera
    Do While ((goCamera.Dest.Y <> goCamera.Position.Y) Or (goCamera.Dest.z <> goCamera.Position.z))
        goCamera.UpdatePosition
        Render
        DoEvents
    Loop
    
End Sub

Public Sub LoadObjects()
        
    If gfObjectsLoaded Then Exit Sub
    'Initialize the objects
    Set goPuck = New cPuck
    Set goPaddle(0) = New cPaddle
    goPaddle(0).PaddleID = 0
    Set goPaddle(1) = New cPaddle
    goPaddle(1).PaddleID = 1
    If goCamera Is Nothing Then Set goCamera = New cCamera
    Set goTable = New cTable
    Set goRoom = New cRoom
    
    If goInput Is Nothing Then Set goInput = New cInput
    If goAudio Is Nothing Then Set goAudio = New cAudio
    If goFade Is Nothing Then Set goFade = New cFade
    If goDev Is Nothing Then Set goDev = New frmSelectDevice
    
    D3DEnum_BuildAdapterList frmAir
    
    'Get any defaults from the registry we might need
    goTable.DrawTable = GetSetting(gsKeyName, gsSubKey, "DrawTable", True)
    goRoom.DrawRoom = GetSetting(gsKeyName, gsSubKey, "DrawRoom", True)
    goRoom.barRoom = GetSetting(gsKeyName, gsSubKey, "RoomIsBarRoom", True)
    
    'Audio options
    goAudio.PlayMusic = GetSetting(gsKeyName, gsSubKeyAudio, "UseBackgroundMusic", False)
    goAudio.PlaySounds = GetSetting(gsKeyName, gsSubKeyAudio, "UseSound", True)
    goAudio.MusicVolume = GetSetting(gsKeyName, gsSubKeyAudio, "MusicVolume", 0)
    goAudio.SoundVolume = GetSetting(gsKeyName, gsSubKeyAudio, "SoundVolume", 0)
    'Input options
    goInput.UseMouse = GetSetting(gsKeyName, gsSubKeyInput, "UseMouse", True)
    goInput.UseKeyboard = GetSetting(gsKeyName, gsSubKeyInput, "UseKeyboard", True)
    goInput.UseJoystick = GetSetting(gsKeyName, gsSubKeyInput, "UseJoystick", False)
    goInput.JoystickGuid = GetSetting(gsKeyName, gsSubKeyInput, "JoystickGuid", vbNullString)
    goInput.JoystickSensitivity = GetSetting(gsKeyName, gsSubKeyInput, "JoystickSensitivity", 0.00025)
    goInput.MouseSensitivity = GetSetting(gsKeyName, gsSubKeyInput, "MouseSensitivity", 0.02)
    goInput.KeyboardSensitivity = GetSetting(gsKeyName, gsSubKeyInput, "KeyboardSensitivity", 0.002)
    'D3D options
    goDev.Windowed = GetSetting(gsKeyName, gsSubKeyGraphics, "Windowed", True)
    goDev.Adapter = GetSetting(gsKeyName, gsSubKeyGraphics, "AdapterID", 0)
    goDev.Mode = GetSetting(gsKeyName, gsSubKeyGraphics, "Mode", 0)
    
    gfObjectsLoaded = True
    
End Sub

Public Sub PauseSystem(ByVal fPause As Boolean)
    gfSystemPause = fPause
    If Not fPause Then
        glTimeCompPaddle = timeGetTime
    End If
    If Not (goPuck Is Nothing) Then
        goPuck.PauseSystem fPause
    End If
End Sub

Public Sub Cleanup(Optional fFinalCleanup As Boolean = False, Optional fOnlyD3D As Boolean = False)
    
    'Getting rid of the objects will clean up the internal objects
    If fFinalCleanup Then
        Set goPuck = Nothing
        Set goPaddle(0) = Nothing
        Set goPaddle(1) = Nothing
        Set goTable = Nothing
        Set goRoom = Nothing
        Set goTextBig = Nothing
        Set goTextLittle = Nothing
        If Not fOnlyD3D Then
            Set goInput = Nothing
            Set goAudio = Nothing
            Set goFade = Nothing
            Set goDev = Nothing
        End If
        gfObjectsLoaded = False
    Else
        goPuck.CleanupFrame
        goPaddle(0).CleanupFrame
        goPaddle(1).CleanupFrame
        goTable.CleanupFrame
        goRoom.CleanupFrame
    End If
End Sub

Public Sub InitGeometry()
        
    LoadObjects
    'First set up the media
    D3DUtil_SetMediaPath AddDirSep(App.path) & "models\"
    goRoom.Init g_mediaPath, "room.x", "lobby_skybox.x"
    frmAir.IncreaseProgressBar
    goPaddle(1).Init g_mediaPath, "paddle.x"
    frmAir.IncreaseProgressBar
    goPaddle(0).Init g_mediaPath, "paddle.x"
    frmAir.IncreaseProgressBar
    goPuck.Init g_mediaPath, "puck.x"
    frmAir.IncreaseProgressBar
    goTable.Init g_mediaPath, "table.x"
    frmAir.IncreaseProgressBar
    
End Sub

Public Sub Render()

    Dim lCount As Long
    
    On Error Resume Next
    If gfSystemPause Then Exit Sub
    If g_dev Is Nothing Then Exit Sub

        ' Clear the backbuffer to a black color
    If gfMultiplayer And gfNoSendData Then 'Uh oh!  We've been disconnected sometime, notify the user
        D3DUtil_ClearAll &HFF0000FF 'Clear with a blue background
    Else
        D3DUtil_ClearAll 0
        
        ' Setup the view and projection matrices
        SetupMatrices
        
        ' Begin the scene
        g_dev.BeginScene
        
        'Draw everything in either a solid fillmode, or wireframe
        If gfWireFrame Then
           g_dev.SetRenderState D3DRS_FILLMODE, D3DFILL_WIREFRAME
        Else
           g_dev.SetRenderState D3DRS_FILLMODE, D3DFILL_SOLID
        End If
       
        If goFade.AmFading Then
            g_dev.SetRenderState D3DRS_ALPHABLENDENABLE, 1  'TRUE
            g_dev.SetRenderState D3DRS_SRCBLEND, D3DBLEND_SRCALPHA
            g_dev.SetRenderState D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA
        End If
       
        'Render the room
        goRoom.Render g_dev
       
        'Render the table
        goTable.Render g_dev
        
        'Now Paddle (0)
        goPaddle(0).Render g_dev
        
        'Now Paddle (1)
        goPaddle(1).Render g_dev
        
        'And finally the puck
        goPuck.Render g_dev
        'Now lets draw whatever text we need
    End If
    
    'We can draw text (don't draw text if we're currently fading)
    If Not goFade.AmFading Then
        goTextLittle.BeginText
        If gfGameCanBeStarted Then
            'If the game can be started, then draw the scores at the top of the screen
            If gfMultiplayer Then
                If glMyPaddleID = 0 Then
                    goTextLittle.DrawText gsUserName & ":" & gPlayer(0).Score, 10, 5, &HFFFFFF00
                Else
                    goTextLittle.DrawText "Opponent:" & gPlayer(0).Score, 10, 5, &HFFFFFFFF
                End If
                If glMyPaddleID = 1 Then
                    goTextLittle.DrawText gsUserName & ":" & gPlayer(1).Score, glScreenWidth - 75, 5, &HFFFFFF00
                Else
                    goTextLittle.DrawText "Opponent:" & gPlayer(1).Score, glScreenWidth - 75, 5, &HFFFFFFFF
                End If
            Else
                goTextLittle.DrawText "Player:" & gPlayer(0).Score, 10, 5, &HFFFFFF00
                goTextLittle.DrawText "Computer:" & gPlayer(1).Score, glScreenWidth - 75, 5, &HFFFFFFFF
            End If
        Else
            'The game can't be started yet (only in multiplayer)  Let the host know
            goTextLittle.DrawText "Waiting for the game to be started...", (glScreenWidth / 2) - 50, 5, &HFFFFFFFF
        End If
        'Here is a little helper text letting the user know to press Space
        'to launch the puck (will show up after 3 seconds, and stay on for 10 seconds)
        If (timeGetTime - glTimePuckScored > glDefaultDelayTime) And gfScored And Not gfGameOver And ((timeGetTime - glTimePuckScored < glDefaultDelayTimeGone + glDefaultDelayTime)) Then
            goPuck.DefaultStartPosition
            goPuck.Spinning = True
            goTextLittle.DrawText "Press <Space> to launch puck...", (glScreenWidth / 2) - 50, 25, &HFF0000FF
        End If
        
        'Here is a little helper text letting the user know to press F1
        'to turn of the room (will show up after 3 seconds, and stay on for 10 seconds)
        If (gfDrawRoomText And goRoom.DrawRoom) And (timeGetTime - glTimeNoRoom < glDefaultDelayTimeGone) Then
            goTextLittle.DrawText "You can press F1 to turn off the drawing " & vbCrLf & " of the room, which will increase performance.", -15, glScreenHeight - 50, &HFFFF00FF
        End If
        If gfGameOver And ((timeGetTime - glTimeGameOver) > glDefaultDelayTime) And ((timeGetTime - glTimeGameOver < glDefaultDelayTimeGone + glDefaultDelayTime)) Then
            goTextLittle.DrawText "Press F3 to restart...", (glScreenWidth / 2) - 50, 25, &HFF0000FF
        End If
        goTextLittle.EndText
        goTextBig.BeginText
        If gfGameOver Then
            If gfMultiplayer Then
                If gPlayer(glMyPaddleID).Score > gPlayer(Abs(glMyPaddleID - 1)).Score Then
                    goTextBig.DrawText "Game over!!" & vbCrLf & "You win!!", (glScreenWidth / 2) - (glScreenWidth / 4), (glScreenHeight / 2) - (glScreenHeight / 4), &HFFDD11AA
                Else
                    goTextBig.DrawText "Game over!!" & vbCrLf & "You lose!!", (glScreenWidth / 2) - (glScreenWidth / 4), (glScreenHeight / 2) - (glScreenHeight / 4), &HFFDD11AA
                End If
            Else
                If gPlayer(0).Score > gPlayer(1).Score Then
                    goTextBig.DrawText "Game over!!" & vbCrLf & "You win!!", (glScreenWidth / 2) - (glScreenWidth / 4), (glScreenHeight / 2) - (glScreenHeight / 4), &HFFDD11AA
                Else
                    goTextBig.DrawText "Game over!!" & vbCrLf & "You lose!!", (glScreenWidth / 2) - (glScreenWidth / 4), (glScreenHeight / 2) - (glScreenHeight / 4), &HFFDD11AA
                End If
            End If
        End If
        If gfMultiplayer And gfNoSendData Then 'Uh oh!  We've been disconnected sometime, notify the user
            goTextBig.DrawText "The connection with the other" & vbCrLf & "system was lost.", 5, (glScreenHeight / 2) - (glScreenHeight / 4), &HFFFFFF00
            'This message isn't on a timer to go away
        End If
        goTextBig.EndText
    End If
    ' End the scene
    g_dev.EndScene
    ' Present the backbuffer contents to the front buffer (screen)
    D3DUtil_PresentAll 0
    
End Sub

Public Sub SetupMatrices()
    
    Dim matView As D3DMATRIX
    Dim matProj As D3DMATRIX
        
    D3DXMatrixLookAtLH matView, goCamera.Position, vec3(0#, 0#, 0#), vec3(0#, 1#, 0#)
    g_dev.SetTransform D3DTS_VIEW, matView
    
    D3DXMatrixPerspectiveFovLH matProj, g_pi / 4, 1, 1, 110
    g_dev.SetTransform D3DTS_PROJECTION, matProj

End Sub

Public Sub RestoreDeviceObjects()

    ' Set miscellaneous render states
    With g_dev
        ' Set world transform
        Dim matWorld As D3DMATRIX
        D3DXMatrixIdentity matWorld
        .SetTransform D3DTS_WORLD, matWorld
        
        .SetTextureStageState 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE
        .SetTextureStageState 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1
        .SetTextureStageState 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE
        .SetTextureStageState 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR
        .SetTextureStageState 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR

        ' Set default render states
        .SetRenderState D3DRS_ZENABLE, 1 'True

    End With
    
    Set goTextBig = Nothing
    Set goTextLittle = Nothing
    
    'Now create a new text object
    Set goTextLittle = New cText
    goTextLittle.InitText g_d3dx, g_dev, "Times New Roman", 8, True
    Set goTextBig = New cText
    goTextBig.InitText g_d3dx, g_dev, "Times New Roman", 18, True
    
End Sub

Public Sub InitDefaultLights(ByVal lNumLights As Long)
    With g_dev
        ' Set ambient light
        .SetRenderState D3DRS_AMBIENT, &HFFFFFFFF
        If lNumLights < 3 Then Exit Sub 'Nothing to do
        
        ' Set ambient light
        'We will slowly lower the ambient light as each new light gets added
        .SetRenderState D3DRS_AMBIENT, &HFFBBBBBB
        ' Turn on lighting
        .SetRenderState D3DRS_LIGHTING, 1
        
        'Turn on two lights one on each end of the table
        Dim light As D3DLIGHT8
        
        If lNumLights > 0 Then
            With light
                .Type = D3DLIGHT_DIRECTIONAL
                .diffuse.r = 1
                .diffuse.g = 1
                .diffuse.b = 1
                .Direction.X = 0
                .Direction.Y = -10
                .Direction.z = 0
                .Range = 1.84467435229094E+19 'User defined.
                .Position.X = 0
                .Position.Y = 3
                .Position.z = 0
            End With
    
            .SetLight 0, light                   'let d3d know about the light
            .LightEnable 0, 1                    'turn it on
        End If
        
        
        If lNumLights > 1 Then
            .SetRenderState D3DRS_AMBIENT, &HFFAAAAAA
            'Now turn on the second light if we can
            With light
                .Type = D3DLIGHT_DIRECTIONAL
                .Direction.X = 5
                .Direction.Y = -3
                .Direction.z = -5
                .Position.X = -5
                .Position.Y = 3
                .Position.z = 5
            End With
    
            .SetLight 1, light                   'let d3d know about the light
            .LightEnable 1, 1                    'turn it on
        End If

        
        If lNumLights > 3 Then
            .SetRenderState D3DRS_AMBIENT, 0
            'Now turn on the third light if we can
            With light
                .Type = D3DLIGHT_DIRECTIONAL
                .Direction.X = -5
                .Direction.Y = 3
                .Direction.z = 5
                .Position.X = 5
                .Position.Y = -3
                .Position.z = -5
            End With
    
            .SetLight 2, light                   'let d3d know about the light
            .LightEnable 2, 1                    'turn it on
        End If
    End With
End Sub

Public Sub SaveOrRestoreObjectSettings(ByVal fSave As Boolean)
    'Puck
    Static LastPuckPosition As D3DVECTOR
    Static PuckPosition As D3DVECTOR
    Static MaxPuckVel As Single
    Static PuckSpinning As Boolean
    Static PuckVelocity As D3DVECTOR
    
    If fSave Then
        LastPuckPosition = goPuck.LastPosition
        MaxPuckVel = goPuck.MaximumPuckVelocity
        PuckPosition = goPuck.Position
        PuckSpinning = goPuck.Spinning
        PuckVelocity = goPuck.Velocity
    Else
        goPuck.LastPosition = LastPuckPosition
        goPuck.MaximumPuckVelocity = MaxPuckVel
        goPuck.Position = PuckPosition
        goPuck.Spinning = PuckSpinning
        goPuck.Velocity = PuckVelocity
    End If
    
    'paddles
    Static LastPaddlePosition(1) As D3DVECTOR
    Static LastPaddleVelTick(1) As Long
    Static PaddleID(1) As Long
    Static PaddlePosition(1) As D3DVECTOR
    Static PaddleTrans(1) As Boolean
    Static PaddleVelocity(1) As D3DVECTOR
    
    Dim i As Integer
    If fSave Then
        For i = 0 To 1
            LastPaddlePosition(i) = goPaddle(i).LastPosition
            LastPaddleVelTick(i) = goPaddle(i).LastVelocityTick
            PaddleID(i) = goPaddle(i).PaddleID
            PaddlePosition(i) = goPaddle(i).Position
            PaddleTrans(i) = goPaddle(i).Transparent
            PaddleVelocity(i) = goPaddle(i).Velocity
        Next
    Else
        For i = 0 To 1
            goPaddle(i).LastPosition = LastPaddlePosition(i)
            goPaddle(i).LastVelocityTick = LastPaddleVelTick(i)
            goPaddle(i).PaddleID = PaddleID(i)
            goPaddle(i).Position = PaddlePosition(i)
            goPaddle(i).Transparent = PaddleTrans(i)
            goPaddle(i).Velocity = PaddleVelocity(i)
        Next
    End If
    
    'Room
    Static barRoom As Boolean
    Static DrawRoom As Boolean
    If fSave Then
        barRoom = goRoom.barRoom
        DrawRoom = goRoom.DrawRoom
    Else
        goRoom.barRoom = barRoom
        goRoom.DrawRoom = DrawRoom
    End If
    
    'Table
    Static DrawTable As Boolean
    Static TablePosition As D3DVECTOR
    Static TableTrans As Boolean
    
    If fSave Then
        DrawTable = goTable.DrawTable
        TablePosition = goTable.Position
        TableTrans = goTable.Transparent
    Else
        goTable.DrawTable = DrawTable
        goTable.Position = TablePosition
        goTable.Transparent = TableTrans
    End If
    
End Sub
