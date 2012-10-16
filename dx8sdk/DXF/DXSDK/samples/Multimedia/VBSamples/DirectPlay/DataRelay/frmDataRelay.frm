VERSION 5.00
Begin VB.Form frmDataRelay 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "vbData Relay"
   ClientHeight    =   6255
   ClientLeft      =   645
   ClientTop       =   930
   ClientWidth     =   7755
   Icon            =   "frmDataRelay.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   6255
   ScaleWidth      =   7755
   StartUpPosition =   3  'Windows Default
   Begin VB.Frame Frame5 
      Caption         =   "Connection Information"
      Height          =   2715
      Left            =   3240
      TabIndex        =   23
      Top             =   960
      Width           =   4455
      Begin VB.TextBox txtInfo 
         BackColor       =   &H8000000F&
         Height          =   1935
         Left            =   120
         MultiLine       =   -1  'True
         ScrollBars      =   2  'Vertical
         TabIndex        =   26
         Top             =   660
         Width           =   4155
      End
      Begin VB.ComboBox cboInfoTarget 
         Height          =   315
         Left            =   1380
         Style           =   2  'Dropdown List
         TabIndex        =   25
         Top             =   240
         Width           =   2655
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Info Target: "
         Height          =   195
         Index           =   6
         Left            =   300
         TabIndex        =   24
         Top             =   300
         Width           =   1035
      End
   End
   Begin VB.Timer tmrReceivedData 
      Interval        =   1
      Left            =   180
      Top             =   3060
   End
   Begin VB.Timer tmrSendData 
      Interval        =   1
      Left            =   720
      Top             =   3060
   End
   Begin VB.Frame Frame4 
      Caption         =   "Statistics"
      Height          =   915
      Left            =   60
      TabIndex        =   18
      Top             =   2760
      Width           =   3135
      Begin VB.Label lblReceive 
         BackStyle       =   0  'Transparent
         Caption         =   "0.0"
         Height          =   195
         Left            =   2160
         TabIndex        =   22
         Top             =   480
         Width           =   855
      End
      Begin VB.Label lblSendRate 
         BackStyle       =   0  'Transparent
         Caption         =   "0.0"
         Height          =   195
         Left            =   2160
         TabIndex        =   21
         Top             =   240
         Width           =   795
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Received Rate (bytes/sec) :"
         Height          =   195
         Index           =   8
         Left            =   60
         TabIndex        =   20
         Top             =   480
         Width           =   2055
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Send Rate (bytes/sec) :"
         Height          =   195
         Index           =   7
         Left            =   60
         TabIndex        =   19
         Top             =   240
         Width           =   2055
      End
   End
   Begin VB.Frame Frame3 
      Caption         =   "Send"
      Height          =   1755
      Left            =   60
      TabIndex        =   9
      Top             =   960
      Width           =   3135
      Begin VB.ComboBox cboTimeout 
         Height          =   315
         Left            =   1200
         Style           =   2  'Dropdown List
         TabIndex        =   17
         Top             =   1320
         Width           =   1815
      End
      Begin VB.ComboBox cboTarget 
         Height          =   315
         Left            =   1200
         Style           =   2  'Dropdown List
         TabIndex        =   16
         Top             =   240
         Width           =   1815
      End
      Begin VB.ComboBox cboSize 
         Height          =   315
         Left            =   1200
         Style           =   2  'Dropdown List
         TabIndex        =   15
         Top             =   600
         Width           =   1815
      End
      Begin VB.ComboBox cboRate 
         Height          =   315
         Left            =   1200
         Style           =   2  'Dropdown List
         TabIndex        =   14
         Top             =   960
         Width           =   1815
      End
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   "Timeout (ms) :"
         Height          =   195
         Index           =   5
         Left            =   120
         TabIndex        =   13
         Top             =   1380
         Width           =   1035
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Target :"
         Height          =   195
         Index           =   4
         Left            =   120
         TabIndex        =   12
         Top             =   300
         Width           =   1035
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Size (bytes) :"
         Height          =   195
         Index           =   3
         Left            =   120
         TabIndex        =   11
         Top             =   660
         Width           =   1035
      End
      Begin VB.Label Label1 
         Alignment       =   1  'Right Justify
         BackStyle       =   0  'Transparent
         Caption         =   "Rate (ms) :"
         Height          =   195
         Index           =   2
         Left            =   120
         TabIndex        =   10
         Top             =   1020
         Width           =   1035
      End
   End
   Begin VB.Frame Frame2 
      Caption         =   "Log"
      Height          =   2415
      Left            =   60
      TabIndex        =   7
      Top             =   3720
      Width           =   7635
      Begin VB.TextBox txtLog 
         BackColor       =   &H8000000F&
         Height          =   2055
         Left            =   120
         MultiLine       =   -1  'True
         ScrollBars      =   3  'Both
         TabIndex        =   8
         Top             =   240
         Width           =   7395
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Game Status"
      Height          =   855
      Left            =   60
      TabIndex        =   0
      Top             =   60
      Width           =   7635
      Begin VB.CommandButton cmdExit 
         Cancel          =   -1  'True
         Caption         =   "Exit"
         Height          =   375
         Left            =   5880
         TabIndex        =   6
         Top             =   300
         Width           =   1575
      End
      Begin VB.CommandButton cmdSend 
         Caption         =   "Push to send"
         Enabled         =   0   'False
         Height          =   375
         Left            =   4200
         TabIndex        =   5
         Top             =   300
         Width           =   1575
      End
      Begin VB.Label lblPlayers 
         BackStyle       =   0  'Transparent
         Caption         =   "0"
         Height          =   255
         Left            =   2340
         TabIndex        =   4
         Top             =   480
         Width           =   195
      End
      Begin VB.Label lblPlayer 
         BackStyle       =   0  'Transparent
         Caption         =   "TestPlayer"
         Height          =   255
         Left            =   1560
         TabIndex        =   3
         Top             =   240
         Width           =   1635
      End
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   "Number of Players in session:"
         Height          =   195
         Index           =   1
         Left            =   120
         TabIndex        =   2
         Top             =   480
         Width           =   2175
      End
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   "Local Player Name:"
         Height          =   195
         Index           =   0
         Left            =   120
         TabIndex        =   1
         Top             =   240
         Width           =   1455
      End
   End
End
Attribute VB_Name = "frmDataRelay"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmDataRelay.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'Declare for timeGetTime
Private Declare Function timeGetTime Lib "winmm.dll" () As Long

Implements DirectPlay8Event
Private Const mlTextSize As Long = 32768
Private Type PacketInfo
    lPacketID As Long
    lDataSize As Long
End Type

Private mfSending As Boolean
Private mlRate As Long
Private mlToPlayerID As Long
Private mlTimeOut As Long
Private mlSize As Long
Private mlSending As Long
Private mlLastSendTime As Long
Private mlDataReceived As Long
Private mlDataSent As Long
Private mfInSend As Boolean
Private mfInReceive As Boolean
Private moByte() As Byte, moBuf() As Byte 'DirectPlayBuffer

Private moReceived As New Collection

Private Sub cmdExit_Click()
    'We're done, unload
    Unload Me
End Sub

Private Sub cmdSend_Click()
    
    If mfSending Then
        'Stop sending now
        cmdSend.Caption = "Push to send"
    Else
        'Start sending now
        cmdSend.Caption = "Push to stop"
        ReadCombos
    End If
    EnableComboUI mfSending
    mfSending = Not mfSending
End Sub

Private Sub Form_Load()
    
    'First lets populate our combo boxes
    PopulateBoxes
    'Here we will init our DPlay objects
    InitDPlay
    'Now we can create a new Connection Form (which will also be our message pump)
    Set DPlayEventsForm = New DPlayConnect
    'Start the connection form (it will either create or join a session)
    If Not DPlayEventsForm.StartConnectWizard(dx, dpp, AppGuid, 20, Me) Then
        Cleanup
        End
    Else 'We did choose to play a game
        gsUserName = DPlayEventsForm.UserName
        lblPlayer.Caption = gsUserName
        If DPlayEventsForm.IsHost Then Me.Caption = Me.Caption & " (HOST)"
    End If

End Sub

Private Sub Form_Unload(Cancel As Integer)
    Me.Hide
    'Here we need to turn off our timers
    If mfSending Then cmdSend_Click
    mfSending = False
    Do While moReceived.Count > 0
        DPlayEventsForm.DoSleep 50
    Loop
    tmrReceivedData.Enabled = False
    tmrSendData.Enabled = False
    Cleanup
End Sub

Private Sub PopulateBoxes()
    With cboTarget
        .AddItem "Everyone"
        .ListIndex = 0
    End With
    With cboRate
        .AddItem "1000"
        .AddItem "500"
        .AddItem "250"
        .AddItem "100"
        .AddItem "50"
        .ListIndex = 0
    End With
    With cboSize
        .AddItem "512"
        .AddItem "256"
        .AddItem "128"
        .AddItem "64"
        .AddItem "32"
        .AddItem "16"
        .ListIndex = 0
    End With
    With cboTimeout
        .AddItem "5"
        .AddItem "10"
        .AddItem "20"
        .AddItem "50"
        .AddItem "100"
        .AddItem "250"
        .AddItem "500"
        .ListIndex = 0
    End With
    With cboInfoTarget
        .AddItem "None"
        .ListIndex = 0
    End With
    
End Sub

Private Sub EnableComboUI(ByVal fEnable As Boolean)
    cboRate.Enabled = fEnable
    cboTarget.Enabled = fEnable
    cboTimeout.Enabled = fEnable
    cboSize.Enabled = fEnable
End Sub

Private Sub ReadCombos()
    mlRate = CLng(cboRate.List(cboRate.ListIndex))
    mlSize = CLng(cboSize.List(cboSize.ListIndex))
    mlTimeOut = CLng(cboTimeout.List(cboTimeout.ListIndex))
    mlToPlayerID = cboTarget.ItemData(cboTarget.ListIndex) 'The ItemData for everyone is 0
End Sub

Private Sub AppendText(ByVal sString As String)
    'Update the chat window first
    txtLog.Text = txtLog.Text & sString & vbCrLf
    'Now limit the text in the window to be 16k
    If Len(txtLog.Text) > mlTextSize Then
        txtLog.Text = Right$(txtLog.Text, mlTextSize)
    End If
    'Autoscroll the text
    txtLog.SelStart = Len(txtLog.Text)
End Sub

Private Function GetName(ByVal lID As Long) As String
    Dim lCount As Long
    
    'Here we will get the name of the player sending us info from the combo box
    GetName = vbNullString
    For lCount = 0 To cboTarget.ListCount - 1
        If cboTarget.ItemData(lCount) = lID Then 'This is the player
            GetName = cboTarget.List(lCount)
            Exit For
        End If
    Next
End Function

Private Sub tmrReceivedData_Timer()
    Dim oBuf() As Byte, lNewMsg As Long, lNewOffset As Long
    Dim sItems() As String, oPacket As PacketInfo
    
    'If mfInReceive Then Exit Sub
    'We use a timer control here because we don't want to ever
    'block DirectPlay.
    Do While moReceived.Count > 0
        mfInReceive = True
        sItems = Split(moReceived.Item(1), ";")
        AppendText "Received packet #" & sItems(1) & " from " & GetName(CLng(sItems(0))) & " - Size:" & sItems(2)
        'now let this user know we received the packet
        lNewMsg = MSG_PacketReceive
        lNewOffset = NewBuffer(oBuf)
        AddDataToBuffer oBuf, lNewMsg, LenB(lNewMsg), lNewOffset
        oPacket.lDataSize = CLng(sItems(2))
        oPacket.lPacketID = CLng(sItems(1))
        mlDataReceived = mlDataReceived + oPacket.lDataSize
        AddDataToBuffer oBuf, oPacket, LenB(oPacket), lNewOffset
        'We don't care to see the receive callback.
        dpp.SendTo CLng(sItems(0)), oBuf, mlTimeOut, DPNSEND_NOLOOPBACK
        Erase oBuf
        moReceived.Remove 1
    Loop
    mfInReceive = False
End Sub

Private Sub tmrSendData_Timer()
    Dim lMsg As Long, lOffset As Long
    Dim oPacket As PacketInfo

    'We use a timer control here because we don't want to ever
    'block DirectPlay.
    'If mfInSend Then Exit Sub
    If mfSending Then 'We are sending
        If Abs(timeGetTime - mlLastSendTime) > mlRate Then 'We should send another packet now
            mfInSend = True
            lMsg = MSG_GamePacket
            lOffset = NewBuffer(moBuf)
            AddDataToBuffer moBuf, lMsg, LenB(lMsg), lOffset
            mlSending = mlSending + 1
            oPacket.lPacketID = mlSending
            oPacket.lDataSize = mlSize
            mlDataSent = mlDataSent + mlSize
            AddDataToBuffer moBuf, oPacket, LenB(oPacket), lOffset
            ReDim moByte(mlSize)
            AddDataToBuffer moBuf, moByte(0), mlSize, lOffset
            'We will send the NOLOOPBACK flag so we do not get a 'Receive' event for
            'this message.
            'The NOCOPY flag tells DPlay not to copy our buffer.  We will erase the buffer in the
            'SendComplete event
            dpp.SendTo mlToPlayerID, moBuf, mlTimeOut, DPNSEND_NOLOOPBACK Or DPNSEND_NOCOPY
            mlLastSendTime = timeGetTime
        End If
    End If
    'Regardless of what's going on, we should update our ui
    UpdateStats
End Sub

Private Sub UpdateStats()
    Dim lNumMsgs As Long, lNumBytes As Long
    Dim lCurTime As Long
    Dim sText As String, dpnInfo As DPN_CONNECTION_INFO
    Dim lNumMsgHigh As Long, lNumByteHigh As Long
    Dim lNumMsgNormal As Long, lNumByteNormal As Long
    Dim lNumMsgLow As Long, lNumByteLow As Long
    Dim lDrops As Long, lSends As Long
    Dim lPlayerID As Long
    
    On Error Resume Next
    Static lLastTime As Long
    
    If lLastTime = 0 Then lLastTime = timeGetTime
    lCurTime = timeGetTime

    If (lCurTime - lLastTime) < 1000 Then Exit Sub 'We don't need to update more than once a second
        
    Dim nSecondsPassed As Single, nDataIn As Single
    Dim nDataOut As Single
    
    nSecondsPassed = (lCurTime - lLastTime) / 1000
    nDataIn = mlDataReceived / nSecondsPassed
    nDataOut = mlDataSent / nSecondsPassed
    lLastTime = lCurTime
    mlDataReceived = 0
    mlDataSent = 0

    lblSendRate.Caption = Format$(CStr(nDataOut), "0.0#")
    lblReceive.Caption = Format$(CStr(nDataIn), "0.0#")
    
    If cboInfoTarget.ListIndex >= 0 Then
        lPlayerID = cboInfoTarget.ItemData(cboInfoTarget.ListIndex)
        If lPlayerID <> 0 Then
            'Update the connection info
            dpnInfo = dpp.GetConnectionInfo(lPlayerID, 0)
            dpp.GetSendQueueInfo lPlayerID, lNumMsgHigh, lNumByteHigh, DPNGETSENDQUEUEINFO_PRIORITY_HIGH
            dpp.GetSendQueueInfo lPlayerID, lNumMsgLow, lNumByteLow, DPNGETSENDQUEUEINFO_PRIORITY_LOW
            dpp.GetSendQueueInfo lPlayerID, lNumMsgNormal, lNumByteNormal, DPNGETSENDQUEUEINFO_PRIORITY_NORMAL
            lDrops = dpnInfo.lPacketsDropped + dpnInfo.lPacketsRetried
            lDrops = lDrops * 10000
            lSends = dpnInfo.lPacketsSentGuaranteed + dpnInfo.lPacketsSentNonGuaranteed
    
            If lSends > 0 Then lDrops = lDrops \ lSends
            
            sText = "Send Queue Messages High Priority=" & CStr(lNumMsgHigh) & vbCrLf
            sText = sText & "Send Queue Bytes High Priority=" & CStr(lNumByteHigh) & vbCrLf
            sText = sText & "Send Queue Messages Normal Priority=" & CStr(lNumMsgNormal) & vbCrLf
            sText = sText & "Send Queue Bytes Normal Priority=" & CStr(lNumByteNormal) & vbCrLf
    
            sText = sText & "Send Queue Messages Low Priority=" & CStr(lNumMsgLow) & vbCrLf
            sText = sText & "Send Queue Bytes Low Priority=" & CStr(lNumByteLow) & vbCrLf
    
            sText = sText & "Round Trip Latency MS=" & CStr(dpnInfo.lRoundTripLatencyMS) & " ms" & vbCrLf
            sText = sText & "Throughput BPS=" & CStr(dpnInfo.lThroughputBPS) & vbCrLf
            sText = sText & "Peak Throughput BPS=" & CStr(dpnInfo.lPeakThroughputBPS) & vbCrLf
                                                                            
            sText = sText & "Bytes Sent Guaranteed=" & CStr(dpnInfo.lBytesSentGuaranteed) & vbCrLf
            sText = sText & "Packets Sent Guaranteed=" & CStr(dpnInfo.lPacketsSentGuaranteed) & vbCrLf
            sText = sText & "Bytes Sent Non-Guaranteed=" & CStr(dpnInfo.lBytesSentNonGuaranteed) & vbCrLf
            sText = sText & "Packets Sent Non-Guaranteed=" & CStr(dpnInfo.lPacketsSentNonGuaranteed) & vbCrLf
                                                                            
            sText = sText & "Bytes Retried Guaranteed=" & CStr(dpnInfo.lBytesRetried) & vbCrLf
            sText = sText & "Packets Retried Guaranteed=" & CStr(dpnInfo.lPacketsRetried) & vbCrLf
            sText = sText & "Bytes Dropped Non-Guaranteed=" & CStr(dpnInfo.lBytesDropped) & vbCrLf
            sText = sText & "Packets Dropped Non-Guaranteed=" & CStr(dpnInfo.lPacketsDropped) & vbCrLf
                                                                            
            sText = sText & "Messages Transmitted High Priority=" & CStr(dpnInfo.lMessagesTransmittedHighPriority) & vbCrLf
            sText = sText & "Messages Timed Out High Priority=" & CStr(dpnInfo.lMessagesTimedOutHighPriority) & vbCrLf
            sText = sText & "Messages Transmitted Normal Priority=" & CStr(dpnInfo.lMessagesTransmittedNormalPriority) & vbCrLf
            sText = sText & "Messages Timed Out Normal Priority=" & CStr(dpnInfo.lMessagesTimedOutNormalPriority) & vbCrLf
            sText = sText & "Messages Transmitted Low Priority=" & CStr(dpnInfo.lMessagesTransmittedLowPriority) & vbCrLf
            sText = sText & "Messages Timed Out Low Priority=" & CStr(dpnInfo.lMessagesTimedOutLowPriority) & vbCrLf
                                                                            
            sText = sText & "Bytes Received Guaranteed=" & CStr(dpnInfo.lBytesReceivedGuaranteed) & vbCrLf
            sText = sText & "Packets Received Guaranteed=" & CStr(dpnInfo.lPacketsReceivedGuaranteed) & vbCrLf
            sText = sText & "Bytes Received Non-Guaranteed=" & CStr(dpnInfo.lBytesReceivedNonGuaranteed) & vbCrLf
            sText = sText & "Packets Received Non-Guaranteed=" & CStr(dpnInfo.lPacketsReceivedNonGuaranteed) & vbCrLf
            sText = sText & "Messages Received=" & CStr(dpnInfo.lMessagesReceived) & vbCrLf
                                                                            
            sText = sText & "Loss Rate=" & CStr(lDrops \ 100) & "." & CStr(lDrops Mod 100) & vbCrLf
            txtInfo.Text = sText
        Else
            txtInfo.Text = vbNullString
        End If
    End If
End Sub

Private Sub DirectPlay8Event_AddRemovePlayerGroup(ByVal lMsgID As Long, ByVal lPlayerID As Long, ByVal lGroupID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_AppDesc(fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_AsyncOpComplete(dpnotify As DxVBLibA.DPNMSG_ASYNC_OP_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_ConnectComplete(dpnotify As DxVBLibA.DPNMSG_CONNECT_COMPLETE, fRejectMsg As Boolean)
    If dpnotify.hResultCode <> 0 Then
        'For some reason we could not connect.  All available slots must be closed.
        MsgBox "Connect Failed.  Error: 0x" & CStr(Hex$(dpnotify.hResultCode)) & "  - This sample will now close.", vbOKOnly Or vbCritical, "Closing"
        DPlayEventsForm.CloseForm Me
    End If
End Sub

Private Sub DirectPlay8Event_CreateGroup(ByVal lGroupID As Long, ByVal lOwnerID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_CreatePlayer(ByVal lPlayerID As Long, fRejectMsg As Boolean)
    Dim lCount As Long
    Dim dpPeer As DPN_PLAYER_INFO
    
    'When someone joins add them to the 'Target' combo box
    'and update the number of players list
    dpPeer = dpp.GetPeerInfo(lPlayerID)
    If (dpPeer.lPlayerFlags And DPNPLAYER_LOCAL) = 0 Then 'This isn't me add this user
        cboTarget.AddItem dpPeer.Name
        cboTarget.ItemData(cboTarget.ListCount - 1) = lPlayerID
        cboInfoTarget.AddItem dpPeer.Name
        cboInfoTarget.ItemData(cboInfoTarget.ListCount - 1) = lPlayerID
    End If
    'Update our player count,and enable the send button (if need be)
    lblPlayers.Caption = CStr(cboTarget.ListCount)
    cmdSend.Enabled = (cboTarget.ListCount > 1)
End Sub

Private Sub DirectPlay8Event_DestroyGroup(ByVal lGroupID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_DestroyPlayer(ByVal lPlayerID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    Dim lCount As Long
    Dim dpPeer As DPN_PLAYER_INFO
    
    'Remove this player from our list
    For lCount = 0 To cboTarget.ListCount - 1
        If cboTarget.ItemData(lCount) = lPlayerID Then 'This is the player
            cboTarget.RemoveItem lCount
            Exit For
        End If
    Next
    For lCount = 0 To cboInfoTarget.ListCount - 1
        If cboInfoTarget.ItemData(lCount) = lPlayerID Then 'This is the player
            cboInfoTarget.RemoveItem lCount
            Exit For
        End If
    Next
    'Update our player count,and enable the send button (if need be)
    lblPlayers.Caption = CStr(cboTarget.ListCount)
    cmdSend.Enabled = (cboTarget.ListCount > 1)
    'If we are sending, and there is no one left to send to, or the person we were sending too left, stop sending
    If (mfSending) And ((cboTarget.ListCount = 0) Or (mlToPlayerID = lPlayerID)) Then cmdSend_Click
    If cboInfoTarget.ListIndex < 0 Then cboInfoTarget.ListIndex = 0
    If cboTarget.ListIndex < 0 Then cboTarget.ListIndex = 0
End Sub

Private Sub DirectPlay8Event_EnumHostsQuery(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_QUERY, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_EnumHostsResponse(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_RESPONSE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_HostMigrate(ByVal lNewHostID As Long, fRejectMsg As Boolean)
    Dim dpPeer As DPN_PLAYER_INFO
    dpPeer = dpp.GetPeerInfo(lNewHostID)
    If (dpPeer.lPlayerFlags And DPNPLAYER_LOCAL) = DPNPLAYER_LOCAL Then 'I am the new host
        Me.Caption = Me.Caption & " (HOST)"
    End If
End Sub

Private Sub DirectPlay8Event_IndicateConnect(dpnotify As DxVBLibA.DPNMSG_INDICATE_CONNECT, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_IndicatedConnectAborted(fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_InfoNotify(ByVal lMsgID As Long, ByVal lNotifyID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_Receive(dpnotify As DxVBLibA.DPNMSG_RECEIVE, fRejectMsg As Boolean)
    'All we care about in this demo is what msgs we receive.
    Dim lMsg As Long, lOffset As Long
    Dim oPacket As PacketInfo
    
    With dpnotify
    GetDataFromBuffer .ReceivedData, lMsg, LenB(lMsg), lOffset
    Select Case lMsg
    Case MSG_GamePacket 'We received a packet
        'Update the UI showing we received the packet
        GetDataFromBuffer .ReceivedData, oPacket, LenB(oPacket), lOffset
        moReceived.Add CStr(dpnotify.idSender) & ";" & CStr(oPacket.lPacketID) & ";" & CStr(oPacket.lDataSize)
    Case MSG_PacketReceive 'They received a packet we sent
        'Update the UI showing we received the packet
        GetDataFromBuffer .ReceivedData, oPacket, LenB(oPacket), lOffset
        AppendText "Sent packet #" & CStr(oPacket.lPacketID) & " to " & GetName(dpnotify.idSender) & " - Size:" & CStr(oPacket.lDataSize)
    End Select
    End With
End Sub

Private Sub DirectPlay8Event_SendComplete(dpnotify As DxVBLibA.DPNMSG_SEND_COMPLETE, fRejectMsg As Boolean)
    If dpnotify.hResultCode = DPNERR_TIMEDOUT Then 'our packet timed out
        AppendText "Packet Timed Out... "
    End If
    'The send has completed, so DPlay no longer has a need for our
    'buffer, so we can get rid of it now.
    Erase moByte
    Erase moBuf
    'Allow the next send to happen
    mfInSend = False
End Sub

Private Sub DirectPlay8Event_TerminateSession(dpnotify As DxVBLibA.DPNMSG_TERMINATE_SESSION, fRejectMsg As Boolean)
    'This connection has been terminated.
    If dpnotify.hResultCode = DPNERR_HOSTTERMINATEDSESSION Then
        MsgBox "The host has terminated this session.  This sample will now exit.", vbOKOnly Or vbInformation, "Exiting"
    Else
        MsgBox "This session has been lost.  This sample will now exit.", vbOKOnly Or vbInformation, "Exiting"
    End If
    DPlayEventsForm.CloseForm Me
End Sub

