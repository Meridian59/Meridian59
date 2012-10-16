VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmAir 
   BackColor       =   &H00000000&
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Air Hockey"
   ClientHeight    =   4500
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   6000
   BeginProperty Font 
      Name            =   "Comic Sans MS"
      Size            =   9.75
      Charset         =   0
      Weight          =   700
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   Icon            =   "frmAir.frx":0000
   KeyPreview      =   -1  'True
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   300
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   400
   StartUpPosition =   2  'CenterScreen
   Begin MSComctlLib.ProgressBar barProg 
      Height          =   540
      Left            =   225
      TabIndex        =   0
      Top             =   3390
      Visible         =   0   'False
      Width           =   5490
      _ExtentX        =   9684
      _ExtentY        =   953
      _Version        =   393216
      Appearance      =   1
      Scrolling       =   1
   End
   Begin VB.Label lblSplash 
      Alignment       =   2  'Center
      BackStyle       =   0  'Transparent
      Caption         =   "Visual Basic Air Hockey, loading...."
      ForeColor       =   &H00FFFFFF&
      Height          =   360
      Left            =   1095
      TabIndex        =   1
      Top             =   390
      Visible         =   0   'False
      Width           =   4110
   End
   Begin VB.Image imgSplash 
      Height          =   4395
      Left            =   30
      Picture         =   "frmAir.frx":030A
      Stretch         =   -1  'True
      Top             =   60
      Visible         =   0   'False
      Width           =   5925
   End
End
Attribute VB_Name = "frmAir"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'Sleep declare
Private Declare Sub Sleep Lib "kernel32" (ByVal dwMilliseconds As Long)
Private Enum SplashScreenMode
    SplashShow
    SplashHide
    SplashResize
End Enum

'We need to implement our event interfaces
Implements DirectPlay8Event
Private mlSendTime As Long
Private mlNumSend As Long
Private mfGotGameSettings As Boolean
Private mfGameStarted As Boolean

Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
    'We need to be able to handle F2 keys for resolution changes
     Select Case KeyCode
     Case vbKeyF2
        PauseSystem True
        goDev.SelectDevice Me
    Case vbKeyF1
        'Toggle the ability to draw the room
        goRoom.DrawRoom = Not goRoom.DrawRoom
    Case vbKeyF4
        'Toggle the transparency of the paddles
        goTable.Transparent = Not goTable.Transparent
    Case vbKeyF5
        'Toggle the ability to draw the room
        goTable.DrawTable = Not goTable.DrawTable
    Case vbKeyF6
        'Toggle the transparency of the paddles
        goPaddle(0).Transparent = Not goPaddle(0).Transparent
        goPaddle(1).Transparent = Not goPaddle(1).Transparent
    Case vbKeyF3
        'Restart the game if it's available
        If gfGameOver Then
            gPlayer(0).Score = 0: gPlayer(1).Score = 0
            goPuck.DefaultStartPosition
            gfGameOver = False
            NotifyGameRestart
        End If
    Case vbKeyReturn
    
        ' Check for Alt-Enter if not pressed exit
        If Shift <> 4 Then Exit Sub
        PauseSystem True
        ' If we are windowed go fullscreen
        ' If we are fullscreen returned to windowed
        SaveOrRestoreObjectSettings True
        InvalidateDeviceObjects
        Cleanup True, True
        If g_d3dpp.Windowed Then
             D3DUtil_ResetFullscreen
        Else
             D3DUtil_ResetWindowed
        End If
        
        ' Call Restore after ever mode change
        ' because calling reset looses state that needs to
        ' be reinitialized
        Me.RestoreDeviceObjects False
        SaveOrRestoreObjectSettings False
        PauseSystem False
    End Select
End Sub

Private Sub Form_KeyPress(KeyAscii As Integer)
    Dim nVel As Single
    Dim vNewVel As D3DVECTOR
    
    If KeyAscii = vbKeyEscape Then
        Unload Me
    ElseIf LCase(Chr$(KeyAscii)) = "v" Then
        'Scroll through the different 'default' views.  If there is currently a custom view on
        'turn on the default view.
        goCamera.NextCameraPosition glMyPaddleID
    ElseIf KeyAscii = vbKeySpace Then
        'We want to launch the puck.  We should only be able to do this if
        'we have recently scored, or if we haven't started the game yet.
        If gfGameCanBeStarted And gfScored And (Not gfGameOver) Then
            goPuck.LaunchPuck
            If gfMultiplayer Then
                SendPuck
            End If
            'Start the puck spinning
            goPuck.Spinning = True
            gfScored = False
            glTimeCompPaddle = 0
        End If
    ElseIf LCase(Chr$(KeyAscii)) = "w" Then
        gfWireFrame = Not gfWireFrame
    'These two cases should be removed in the final version
    ElseIf LCase(Chr$(KeyAscii)) = "+" Then
        If Not gfScored Then
            nVel = D3DXVec3Length(goPuck.Velocity) * 1.2
            D3DXVec3Normalize vNewVel, goPuck.Velocity
            D3DXVec3Scale vNewVel, vNewVel, nVel
            goPuck.Velocity = vNewVel
            SendPuck
        End If
    ElseIf LCase(Chr$(KeyAscii)) = "-" Then
        If Not gfScored Then
            nVel = D3DXVec3Length(goPuck.Velocity) * 0.8
            D3DXVec3Normalize vNewVel, goPuck.Velocity
            D3DXVec3Scale vNewVel, vNewVel, nVel
            goPuck.Velocity = vNewVel
            SendPuck
        End If
    End If
End Sub

Private Sub Form_Load()
    
    glMyPaddleID = 0
    mfGotGameSettings = False
    'We've got here now.  Go ahead and init our 3D device
    If gfMultiplayer Then
        'Oh good, we want to play a multiplayer game.
        'First lets get the dplay connection started
        
        'Here we will init our DPlay objects
        InitDPlay
        'Now we can create a new Connection Form (which will also be our message pump)
        Set DPlayEventsForm = New DPlayConnect
        'Start the connection form (it will either create or join a session)
        If Not DPlayEventsForm.StartConnectWizard(dx, dpp, AppGuid, 2, Me, False) Then
            CleanupDPlay
            End
        Else 'We did choose to play a game
            gsUserName = DPlayEventsForm.UserName
            If DPlayEventsForm.IsHost Then
                Me.Caption = Me.Caption & " (HOST)"
                mfGotGameSettings = True
            End If
            gfHost = DPlayEventsForm.IsHost
        End If
    End If
        
    'Do a quick switch to windowed mode just to initialize all the vars
    If Not D3DUtil_Init(frmAir.hwnd, True, 0, 0, D3DDEVTYPE_HAL, Me) Then
        MsgBox "Could not initialize Direct3D.  This sample will now exit", vbOKOnly Or vbInformation, "Exiting..."
        Unload Me
        Exit Sub
    End If
    'Now update to the 'correct' resolution (or windowed)
    goDev.UpdateNow Me
    glScreenHeight = Me.ScaleHeight: glScreenWidth = Me.ScaleWidth
    If g_d3dpp.Windowed = 0 Then
        Me.Move 0, 0, g_d3dpp.BackBufferWidth * Screen.TwipsPerPixelX, g_d3dpp.BackBufferHeight * Screen.TwipsPerPixelY
    End If
    Me.Show
    SplashScreenMode SplashShow
    DoEvents
    barProg.Min = 0: barProg.Max = 9
    InitDeviceObjects
    IncreaseProgressBar
    RestoreDeviceObjects
    IncreaseProgressBar
    'Start up our Input devices
    If Not goInput.InitDirectInput(Me) Then
        Cleanup 'This should restore our state so we can complain that we couldn't Init Dinput
        MsgBox "Unable to Initialize DirectInput, this sample will now exit.", vbOKOnly Or vbInformation, "No DirectInput"
        Unload Me
        Exit Sub
    End If
    IncreaseProgressBar
    'Start up our sounds
    If Not goAudio.InitAudio Then
        MsgBox "Unable to Initialize Audio, this sample will not have audio capablities.", vbOKOnly Or vbInformation, "No Audio"
        goAudio.PlayMusic = False
        goAudio.PlaySounds = False
    End If
    IncreaseProgressBar
    'Here we will load the initial positions for our objects
    LoadDefaultStartPositions
    'Get rid of the splash screen
    Unload frmSplash
    glTimePuckScored = timeGetTime
    SplashScreenMode SplashHide
    'Wait a brief period of time
    Sleep 100
    'Do the intro
    ShowStartup
    goAudio.StartBackgroundMusic
    glTimePuckScored = timeGetTime
    
    Me.Show
    
    'Start the puck spinning
    goPuck.Spinning = True
    'Now, if we're in a multiplayer game, and we're the client
    'let the host know that we are ready to play the game, and he can launch the puck at any time.
    If gfMultiplayer Then
        Do While Not mfGotGameSettings
            DPlayEventsForm.DoSleep 10 'Wait until we receive the game settings
        Loop
        NotifyClientReady
    End If
    glTimePuckScored = timeGetTime
    MainGameLoop
    
End Sub

Private Sub Form_Resize()

    If Me.WindowState = vbMinimized Then
        PauseSystem True
    Else
        PauseSystem False
        glScreenHeight = Me.ScaleHeight: glScreenWidth = Me.ScaleWidth
        SplashScreenMode SplashResize
    End If
End Sub

Private Sub Form_Unload(Cancel As Integer)
    goFade.Fade -5
    Do While goFade.AmFading
        Render
        goFade.UpdateFade goPuck, goPaddle, goTable, goRoom
        DoEvents
    Loop
    SaveDrawingSettings
    CleanupDPlay
    Cleanup True
    End
End Sub

Private Sub SaveDrawingSettings()
    SaveSetting gsKeyName, gsSubKey, "DrawRoom", goRoom.DrawRoom
    SaveSetting gsKeyName, gsSubKey, "DrawTable", goTable.DrawTable
End Sub

Public Function VerifyDevice(flags As Long, format As CONST_D3DFORMAT) As Boolean
    'All the checks we care about are already done, always return true
    VerifyDevice = True
End Function

Public Sub InvalidateDeviceObjects()
    InitDeviceObjects False
End Sub

Public Sub RestoreDeviceObjects(Optional ByVal fSplash As Boolean = True)
    modAirHockey.RestoreDeviceObjects
    InitDeviceObjects (Not fSplash)
    glScreenHeight = Me.ScaleHeight: glScreenWidth = Me.ScaleWidth
    If fSplash Then SplashScreenMode SplashResize
End Sub

Public Sub DeleteDeviceObjects()
    Cleanup
End Sub

Public Sub InitDeviceObjects(Optional fLoadGeometry As Boolean = True)
    'Check caps for lights
    Dim d3dcaps As D3DCAPS8
    
    g_dev.GetDeviceCaps d3dcaps
    If (d3dcaps.VertexProcessingCaps And D3DVTXPCAPS_DIRECTIONALLIGHTS) <> 0 Then  'We can use directional lights
        InitDefaultLights d3dcaps.MaxActiveLights 'Set up the lights for the room
    Else
        'We could render the whole scene just using ambient light
        '(which we'll have too since we can't position our direction
        'lights), but the user will miss out on the shading of the table
        InitDefaultLights 0 'Set up a default ambiant only light
    End If
    'Make sure the device supports alpha blending
    If (d3dcaps.TextureCaps And D3DPTEXTURECAPS_ALPHA) Then
        If Not (goFade Is Nothing) Then goFade.CanFade = True
        g_dev.SetRenderState D3DRS_ALPHABLENDENABLE, 1
    Else
        If Not (goFade Is Nothing) Then goFade.CanFade = False
    End If
    
    'Load our objects now
    If fLoadGeometry Then InitGeometry 'Set up the room geometry
End Sub

Public Sub IncreaseProgressBar()
    On Error Resume Next
    barProg.Value = barProg.Value + 1
    DoEvents
End Sub

Private Sub SplashScreenMode(ByVal Mode As SplashScreenMode)
    Select Case Mode
    Case SplashHide
        imgSplash.Visible = False
        barProg.Visible = False
        lblSplash.Visible = False
    Case SplashResize
        'Move the splash screen to cover the entire client area
        imgSplash.Move 0, 0, Me.ScaleWidth, Me.ScaleHeight
        'Move the progress bar
        barProg.Move 15, Me.ScaleHeight - ((Me.ScaleHeight / 10) + 20), Me.ScaleWidth - 30, Me.ScaleHeight / 10
        lblSplash.Move 15, ((Me.ScaleHeight / 10) + 20), Me.ScaleWidth - 30, Me.ScaleHeight / 10
    Case SplashShow
        imgSplash.Visible = True
        barProg.Visible = True
        lblSplash.Visible = True
        lblSplash.ZOrder
    End Select
End Sub

Private Sub DirectPlay8Event_AddRemovePlayerGroup(ByVal lMsgID As Long, ByVal lPlayerID As Long, ByVal lGroupID As Long, fRejectMsg As Boolean)
    'VB requires that we implement *all* members of an interface
End Sub

Private Sub DirectPlay8Event_AppDesc(fRejectMsg As Boolean)
    'VB requires that we implement *all* members of an interface
End Sub

Private Sub DirectPlay8Event_AsyncOpComplete(dpnotify As DxVBLibA.DPNMSG_ASYNC_OP_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we implement *all* members of an interface
End Sub

Private Sub DirectPlay8Event_ConnectComplete(dpnotify As DxVBLibA.DPNMSG_CONNECT_COMPLETE, fRejectMsg As Boolean)
    If dpnotify.hResultCode <> 0 Then 'There was a problem
        MsgBox "Failed to connect to host." & vbCrLf & "Error:" & CStr(dpnotify.hResultCode), vbOKOnly Or vbInformation, "Exiting..."
        Unload Me
        Exit Sub
    End If
    'If we are receiving this event we must know that we are the client, since the server never receives this message.
    'Make sure we are assigned paddle ID #1
    glMyPaddleID = 1 'We are the second paddle
End Sub

Private Sub DirectPlay8Event_CreateGroup(ByVal lGroupID As Long, ByVal lOwnerID As Long, fRejectMsg As Boolean)
    'VB requires that we implement *all* members of an interface
End Sub

Private Sub DirectPlay8Event_CreatePlayer(ByVal lPlayerID As Long, fRejectMsg As Boolean)
    'We've got the create player message, so someone has just joined.  Send them the
    'Setup message (if it's not us)
    Dim dpPlayer As DPN_PLAYER_INFO
    
    dpPlayer = dpp.GetPeerInfo(lPlayerID)
    If (dpPlayer.lPlayerFlags And DPNPLAYER_HOST) = 0 Then 'This isn't the host, let them know
        SendGameSettings
    End If
    If (dpPlayer.lPlayerFlags And DPNPLAYER_LOCAL) = 0 Then 'This isn't the local player, save this id
        glOtherPlayerID = lPlayerID
    End If
End Sub

Private Sub DirectPlay8Event_DestroyGroup(ByVal lGroupID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'VB requires that we implement *all* members of an interface
End Sub

Private Sub DirectPlay8Event_DestroyPlayer(ByVal lPlayerID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'If we receive a DestroyPlayer msg, then the other player must have quit.
    'We have been disconnected, stop sending data
    gfNoSendData = True
End Sub

Private Sub DirectPlay8Event_EnumHostsQuery(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_QUERY, fRejectMsg As Boolean)
    'If the game has started don't even bother answering the enum query.
    If mfGameStarted Then fRejectMsg = True
End Sub

Private Sub DirectPlay8Event_EnumHostsResponse(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_RESPONSE, fRejectMsg As Boolean)
    'VB requires that we implement *all* members of an interface
End Sub

Private Sub DirectPlay8Event_HostMigrate(ByVal lNewHostID As Long, fRejectMsg As Boolean)
    'VB requires that we implement *all* members of an interface
End Sub

Private Sub DirectPlay8Event_IndicateConnect(dpnotify As DxVBLibA.DPNMSG_INDICATE_CONNECT, fRejectMsg As Boolean)
    If Not mfGameStarted Then
        'We haven't started the game yet, go ahead and allow this
        mfGameStarted = True
    Else
        fRejectMsg = True
    End If
End Sub

Private Sub DirectPlay8Event_IndicatedConnectAborted(fRejectMsg As Boolean)
    'Uh oh, the person who indicated connect has now aborted, reset our flag
    fRejectMsg = False
End Sub

Private Sub DirectPlay8Event_InfoNotify(ByVal lMsgID As Long, ByVal lNotifyID As Long, fRejectMsg As Boolean)
    'VB requires that we implement *all* members of an interface
End Sub

Private Sub DirectPlay8Event_Receive(dpnotify As DxVBLibA.DPNMSG_RECEIVE, fRejectMsg As Boolean)
    'process what msgs we receive.
    Dim lMsg As Byte, lOffset As Long
    Dim lPaddleID As Byte
    Dim vTemp As D3DVECTOR
    
    With dpnotify
    GetDataFromBuffer .ReceivedData, lMsg, LenB(lMsg), lOffset
    Select Case lMsg
    Case MsgPaddleLocation
        GetDataFromBuffer .ReceivedData, lPaddleID, LenB(lPaddleID), lOffset
        GetDataFromBuffer .ReceivedData, vTemp, LenB(vTemp), lOffset
        goPaddle(lPaddleID).Position = vTemp
    Case MsgPuckLocation
        GetDataFromBuffer .ReceivedData, vTemp, LenB(vTemp), lOffset
        goPuck.Position = vTemp
        GetDataFromBuffer .ReceivedData, vTemp, LenB(vTemp), lOffset
        goPuck.Velocity = vTemp
        'Start the puck spinning
        goPuck.Spinning = True
        gfScored = False
    Case MsgClientConnectedAndReadyToPlay
        gfGameCanBeStarted = True
    Case MsgPlayerScored
        goPuck.DropPuckIntoScoringPosition goAudio, True
    Case MsgRestartGame
        If gfGameOver Then
            gPlayer(0).Score = 0: gPlayer(1).Score = 0
            goPuck.DefaultStartPosition
            gfGameOver = False
        End If
    Case MsgSendGameSettings
        'Get the data that holds the game settings
        GetDataFromBuffer .ReceivedData, gnVelocityDamp, LenB(gnVelocityDamp), lOffset
        goPuck.MaximumPuckVelocity = gnVelocityDamp * 6.23
        GetDataFromBuffer .ReceivedData, glUserWinningScore, LenB(glUserWinningScore), lOffset
        GetDataFromBuffer .ReceivedData, gnPaddleMass, LenB(gnPaddleMass), lOffset
        mfGotGameSettings = True
    Case MsgCollidePaddle
        'Notify the user that the puck hit the paddle by playing a sound
        goAudio.PlayHitSound
    End Select
    End With
End Sub

Private Sub DirectPlay8Event_SendComplete(dpnotify As DxVBLibA.DPNMSG_SEND_COMPLETE, fRejectMsg As Boolean)
    
    'Here we can update our send frequency based on how quickly the messages are arriving
    mlSendTime = mlSendTime + dpnotify.lSendTime
    mlNumSend = mlNumSend + 1
    If dpnotify.hResultCode = DPNERR_TIMEDOUT Then
        'Add a little more delay, packets are timing out
        mlSendTime = mlSendTime + dpnotify.lSendTime + (glMinimumSendFrequency \ 2)
    End If
    'Send them as fast as they can receive them, but not overly fast (20 times/second max)
    'We will calculate this based on the average amount of time it takes to send the data
    glSendFrequency = ((mlSendTime \ mlNumSend) + glSendFrequency) \ 2
    
    Debug.Print "Send Freq:"; glSendFrequency; mlSendTime; mlNumSend
    glOneWaySendLatency = (mlSendTime \ mlNumSend) \ 2
    If glSendFrequency < glMinimumSendFrequency Then glSendFrequency = glMinimumSendFrequency
    'Check for the max value for long (just in case)
    If (mlNumSend > 2147483647) Or (mlSendTime > 2147483647) Then 'You would have to run the app for like 3 years to reach this level, but just in case...
        'If it does though, reset the average
        mlNumSend = 0
        mlSendTime = 0
    End If
End Sub

Private Sub DirectPlay8Event_TerminateSession(dpnotify As DxVBLibA.DPNMSG_TERMINATE_SESSION, fRejectMsg As Boolean)
    'We have been disconnected, stop sending data
    gfNoSendData = True
End Sub
