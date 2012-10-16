Attribute VB_Name = "modDplay"
Option Explicit
Private Declare Sub Sleep Lib "kernel32" (ByVal dwMilliseconds As Long)

'We want to keep the amount of data we send down to a bare minimum.  Use the lowest
'data type we can.  For example, even though Enums are by default Long's
'We will never have more than 255 messages for this application so we will convert
'them all to bytes when we send them
Public Enum vbDplayHockeyMsgType
    MsgSendGameSettings 'The settings for the application to run under
    MsgPaddleLocation 'The location of a particular paddle
    MsgPuckLocation 'The location of the puck
    MsgPlayerScored 'Someone just scored
    MsgClientConnectedAndReadyToPlay 'The client is connected, has received the game settings and is ready to play
    MsgRestartGame 'Time to restart the game
    MsgCollidePaddle 'Used only for sound effects...
End Enum

'Constants
Public Const AppGuid = "{AC35AAB4-32D3-465d-96C3-4F4137FBF9A1}"
'Minimum frequency to allow sending data (in ms)
'Regardless of network latency, we never want to send more than 20 msgs/second
'which equates to a minimum send frequency of 50
Public Const glMinimumSendFrequency As Long = 1000 \ 20
'Main Peer object
Public dpp As DirectPlay8Peer
'PlayerID of the user who is connected
Public glOtherPlayerID As Long

'App specific variables
Public gsUserName As String
'Our connection form and message pump
Public DPlayEventsForm As DPlayConnect
'How often we should send our paddles information
Public glSendFrequency As Long
'The amount of latency between two systems
'(calculated as Avg(RoundTripLatency)/2)
Public glOneWaySendLatency As Long
'We have disconnected from the session.  Stop sending data
Public gfNoSendData As Boolean

Public Sub InitDPlay()
    'Create our DX/DirectPlay objects
    If dx Is Nothing Then Set dx = New DirectX8
    Set dpp = dx.DirectPlayPeerCreate
    glSendFrequency = glMinimumSendFrequency
End Sub

Public Sub CleanupDPlay()
    On Error Resume Next
    If Not (DPlayEventsForm Is Nothing) Then
        If Not (dpp Is Nothing) Then dpp.UnRegisterMessageHandler
        DPlayEventsForm.DoSleep 50
        'Get rid of our message pump
        DPlayEventsForm.GoUnload
        'Close down our session
        If Not (dpp Is Nothing) Then dpp.Close
        'Lose references to peer and dx objects
        Set dpp = Nothing
        Set dx = Nothing
    End If
End Sub

Public Sub UpdateNetworkSettings()

    Dim lMsg As Long, lNumMsg As Long, lNumByte As Long
    Dim lOffset As Long, oBuf() As Byte
    Static lLastSendTime As Long
    Static lLastSendCount As Long
    
    On Error Resume Next 'in case we are already in this sub when we receive our connection terminated message
    If gfGameOver Then Exit Sub
    If gfNoSendData Then Exit Sub
    If Not gfGameCanBeStarted Then Exit Sub
    'First lets check the current send queue information.  IF the queue is building up,
    'then we need to bump up the frequency so we don't oversaturate our line.
    dpp.GetSendQueueInfo glOtherPlayerID, lNumMsg, lNumByte
    If lNumMsg > 3 Or lNumByte > 256 Then
        'We are sending data to fast, slow down
        glSendFrequency = glSendFrequency + glMinimumSendFrequency
    End If
    'Here we will send the current game state (puck, and paddle information), and we will send this information
    'not faster than the glSendFrequency (which will be throttled according to latency)
    If timeGetTime - lLastSendTime > glSendFrequency Then
        If gfHost Then
            lLastSendCount = lLastSendCount + 1
            'We will not send the puck every time
            If lLastSendCount > 3 Then
                'Update puck
                'SendPuck 0
                lLastSendCount = 0
            End If
        End If
        
        'Now send our paddle
        lMsg = MsgPaddleLocation
        AddDataToBuffer oBuf, CByte(lMsg), SIZE_BYTE, lOffset 'Msg
        AddDataToBuffer oBuf, CByte(glMyPaddleID), SIZE_BYTE, lOffset 'Paddle ID
        AddDataToBuffer oBuf, goPaddle(glMyPaddleID).Position, LenB(goPaddle(glMyPaddleID).Position), lOffset 'Paddle information
        'We will send this information to the other player only
        dpp.SendTo glOtherPlayerID, oBuf, glSendFrequency, 0
        lLastSendTime = timeGetTime
    End If

End Sub

Public Sub NotifyClientReady()
    Dim lMsg As Long
    Dim lOffset As Long, oBuf() As Byte
    
    If gfNoSendData Then Exit Sub
    If Not gfMultiplayer Then Exit Sub
    If gfHost Then Exit Sub 'Only the client needs to tell the host
    'Here we will tell the host we are ready to play
    lMsg = MsgClientConnectedAndReadyToPlay
    AddDataToBuffer oBuf, CByte(lMsg), SIZE_BYTE, lOffset
    'We will send this information to the other player only
    dpp.SendTo glOtherPlayerID, oBuf, glSendFrequency, DPNSEND_GUARANTEED Or DPNSEND_PRIORITY_HIGH
    gfGameCanBeStarted = True
End Sub

Public Sub NotifyPlayersWeScored()
    Dim lMsg As Long
    Dim lOffset As Long, oBuf() As Byte
    
    If gfNoSendData Then Exit Sub
    If Not gfMultiplayer Then Exit Sub
    If Not gfHost Then Exit Sub
    'Here we will tell the host we are ready to play
    lMsg = MsgPlayerScored
    AddDataToBuffer oBuf, CByte(lMsg), SIZE_BYTE, lOffset
    'We will send this information to the other player only
    dpp.SendTo glOtherPlayerID, oBuf, glSendFrequency, DPNSEND_GUARANTEED Or DPNSEND_PRIORITY_HIGH
End Sub

Public Sub NotifyGameRestart()
    Dim lMsg As Long
    Dim lOffset As Long, oBuf() As Byte
    
    If gfNoSendData Then Exit Sub
    If Not gfMultiplayer Then Exit Sub
    'Here we will tell the host we are ready to play
    lMsg = MsgRestartGame
    AddDataToBuffer oBuf, CByte(lMsg), SIZE_BYTE, lOffset
    'We will send this information to the other player only
    dpp.SendTo glOtherPlayerID, oBuf, glSendFrequency, DPNSEND_GUARANTEED Or DPNSEND_PRIORITY_HIGH
End Sub

Public Sub SendGameSettings()
    Dim lMsg As Long
    Dim lOffset As Long, oBuf() As Byte
    
    If gfNoSendData Then Exit Sub
    If Not gfMultiplayer Then Exit Sub
    If Not gfHost Then Exit Sub
    'Here we will tell the host we are ready to play
    lMsg = MsgSendGameSettings
    AddDataToBuffer oBuf, CByte(lMsg), SIZE_BYTE, lOffset
    AddDataToBuffer oBuf, gnVelocityDamp, LenB(gnVelocityDamp), lOffset
    AddDataToBuffer oBuf, glUserWinningScore, LenB(glUserWinningScore), lOffset
    AddDataToBuffer oBuf, gnPaddleMass, LenB(gnPaddleMass), lOffset
    'We will send this information to the other player only
    dpp.SendTo glOtherPlayerID, oBuf, glSendFrequency, DPNSEND_GUARANTEED Or DPNSEND_PRIORITY_HIGH
End Sub

Public Sub SendPuck(Optional ByVal lFlags As Long = (DPNSEND_GUARANTEED Or DPNSEND_PRIORITY_HIGH))
    Dim lMsg As Long
    Dim lOffset As Long, oBuf() As Byte
    
    If gfNoSendData Then Exit Sub
    If Not gfMultiplayer Then Exit Sub
    'Here we will tell the host we are ready to play
    lMsg = MsgPuckLocation
    AddDataToBuffer oBuf, CByte(lMsg), SIZE_BYTE, lOffset
    AddDataToBuffer oBuf, goPuck.Position, LenB(goPuck.Position), lOffset
    AddDataToBuffer oBuf, goPuck.Velocity, LenB(goPuck.Velocity), lOffset
    'We will send this information to the other player only
    dpp.SendTo glOtherPlayerID, oBuf, glSendFrequency, lFlags
End Sub

Public Sub SendCollidePaddle()
    Dim lMsg As Long
    Dim lOffset As Long, oBuf() As Byte
    
    If gfNoSendData Then Exit Sub
    If Not gfMultiplayer Then Exit Sub
    'Here we will tell the host we are ready to play
    lMsg = MsgCollidePaddle
    AddDataToBuffer oBuf, CByte(lMsg), SIZE_BYTE, lOffset
    'We will send this information to the other player only
    dpp.SendTo glOtherPlayerID, oBuf, glSendFrequency, DPNSEND_GUARANTEED Or DPNSEND_PRIORITY_HIGH
End Sub

