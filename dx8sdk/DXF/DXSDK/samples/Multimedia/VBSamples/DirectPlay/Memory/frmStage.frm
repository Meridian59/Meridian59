VERSION 5.00
Begin VB.Form frmStage 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "VB Memory Staging Area..."
   ClientHeight    =   1575
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   3060
   Icon            =   "frmStage.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   1575
   ScaleWidth      =   3060
   StartUpPosition =   3  'Windows Default
   Begin VB.Frame Frame1 
      Caption         =   "Session Status"
      Height          =   735
      Left            =   120
      TabIndex        =   2
      Top             =   120
      Width           =   2775
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   "Current number of players:"
         Height          =   195
         Left            =   120
         TabIndex        =   4
         Top             =   300
         Width           =   1935
      End
      Begin VB.Label lblPlayer 
         BorderStyle     =   1  'Fixed Single
         Height          =   315
         Left            =   2160
         TabIndex        =   3
         Top             =   240
         Width           =   495
      End
   End
   Begin VB.CommandButton cmdLeave 
      Cancel          =   -1  'True
      Caption         =   "Leave"
      Height          =   435
      Left            =   600
      TabIndex        =   1
      Top             =   960
      Width           =   1095
   End
   Begin VB.CommandButton cmdStart 
      Caption         =   "Start"
      Default         =   -1  'True
      Enabled         =   0   'False
      Height          =   435
      Left            =   1800
      TabIndex        =   0
      Top             =   960
      Width           =   1095
   End
End
Attribute VB_Name = "frmStage"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmStage.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Implements DirectPlay8Event
Private fStartGame As Boolean

Private Sub cmdLeave_Click()
    'We don't want to play.  Exit
    frmIntro.Visible = True
    frmIntro.EnableButtons True
    Unload Me
End Sub

Private Sub cmdStart_Click()

    Dim oSendBuffer() As Byte
    Dim lCount As Long, lOffset As Long
    Dim lClient As Long
    'Dim oPBuf() As Byte

    'Ok we're ready to start the game now.
    'First how many players do we have?
    gbNumPlayers = dpp.GetCountPlayersAndGroups(DPNENUM_PLAYERS)
    
    ' Initialize game state
    SetupBoard
    
    lOffset = NewBuffer(oSendBuffer)
    AddDataToBuffer oSendBuffer, CByte(MSG_SETUPBOARD), SIZE_BYTE, lOffset
    AddDataToBuffer oSendBuffer, CByte(gbNumPlayers), SIZE_BYTE, lOffset
    'Add each player's id now (this will determine the order of play)
    For lCount = 1 To gbNumPlayers
        lClient = dpp.GetPlayerOrGroup(lCount)
        AddDataToBuffer oSendBuffer, lClient, LenB(lClient), lOffset
        'Keep track of the order ourselves
        glPlayerIDs(lCount - 1) = lClient
    Next
        
    'Keep track of the offset in our byte array of where the data for the board starts
    For lCount = 1 To NumCells
        AddDataToBuffer oSendBuffer, gbPicArray(lCount - 1), LenB(gbPicArray(lCount - 1)), lOffset
    Next lCount
        
    'Now that we've created our byte array of data to start the game
    'We can actually send that msg, and start the game
    SendMessage oSendBuffer
    fStartGame = True
    Unload Me
    frmGameBoard.Show vbModeless
    
End Sub

Private Sub Form_Load()
    Dim lCount As Long
    Dim lFlags As Long
    
    If dpp Is Nothing Then Exit Sub
    
    'Register me as the message handler
    DPlayEventsForm.RegisterCallback Me
        
End Sub

Private Sub Form_Unload(Cancel As Integer)
    
    If Not fStartGame Then
        'make sure the intro form is visible again
        frmIntro.Visible = True
        frmIntro.EnableButtons True
        If Not (DPlayEventsForm Is Nothing) Then DPlayEventsForm.DoSleep 50
        Cleanup
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
    If dpnotify.hResultCode = DPNERR_HOSTREJECTEDCONNECTION Then
        MsgBox "The host would not allow you to join the game.  The game has already started.", vbOKOnly Or vbInformation, "Started"
        DPlayEventsForm.CloseForm Me
    ElseIf dpnotify.hResultCode <> 0 Then
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
    
    gbNumPlayers = gbNumPlayers + 1
    lblPlayer.Caption = CStr(gbNumPlayers)
    If gbNumPlayers > 1 Then
        cmdStart.Enabled = gfHost
    Else
        cmdStart.Enabled = False
    End If
End Sub

Private Sub DirectPlay8Event_DestroyGroup(ByVal lGroupID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_DestroyPlayer(ByVal lPlayerID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    gbNumPlayers = gbNumPlayers - 1
    lblPlayer.Caption = CStr(gbNumPlayers)
    If gbNumPlayers > 1 Then
        cmdStart.Enabled = gfHost
    Else
        cmdStart.Enabled = False
    End If
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
        gfHost = True
        MsgBox "Congratulations, You are the new host.", vbOKOnly Or vbInformation, "New host."
    End If
    If gbNumPlayers > 1 Then
        cmdStart.Enabled = gfHost
    Else
        cmdStart.Enabled = False
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
    Dim lCount As Long, lOffset As Long
    Dim bMsg As Byte, lNumPlayers As Byte
    
    'Here we will go through the messages
    'The first item in our byte array is the MSGID we passed in
    With dpnotify
    GetDataFromBuffer .ReceivedData, bMsg, LenB(bMsg), lOffset
    Select Case bMsg
        Case MSG_SETUPBOARD
        
        ' Number of players
        GetDataFromBuffer .ReceivedData, gbNumPlayers, LenB(gbNumPlayers), lOffset
        ' Play IDs, in play order. Unused players have ID of 0.
        For lCount = 0 To gbNumPlayers - 1
            GetDataFromBuffer .ReceivedData, glPlayerIDs(lCount), LenB(glPlayerIDs(lCount)), lOffset
            'If glPlayerIDs(lCount) = gMyPlayerID Then gMyTurn = lCount
        Next lCount
        ' Tile arrangment
        For lCount = 0 To NumCells - 1
            GetDataFromBuffer .ReceivedData, gbPicArray(lCount), LenB(gbPicArray(lCount)), lOffset
        Next lCount
        ' Show the game board. The scoreboard is initialized in the Load method.
        fStartGame = True
        Unload Me
        frmGameBoard.Show vbModeless
    End Select
    End With
End Sub

Private Sub DirectPlay8Event_SendComplete(dpnotify As DxVBLibA.DPNMSG_SEND_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_TerminateSession(dpnotify As DxVBLibA.DPNMSG_TERMINATE_SESSION, fRejectMsg As Boolean)
    If dpnotify.hResultCode = DPNERR_HOSTTERMINATEDSESSION Then
        MsgBox "The host has terminated this session.  This sample will now exit.", vbOKOnly Or vbInformation, "Exiting"
    Else
        MsgBox "This session has been lost.  This sample will now exit.", vbOKOnly Or vbInformation, "Exiting"
    End If
    DPlayEventsForm.CloseForm Me
End Sub

