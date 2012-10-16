VERSION 5.00
Begin VB.Form frmApp 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Session"
   ClientHeight    =   4470
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   5400
   Icon            =   "frmApp.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4470
   ScaleWidth      =   5400
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdExit 
      Cancel          =   -1  'True
      Caption         =   "Exit"
      Height          =   315
      Left            =   3383
      TabIndex        =   9
      Top             =   4020
      Width           =   1035
   End
   Begin VB.Frame Rules 
      Caption         =   "Rules"
      Height          =   735
      Left            =   60
      TabIndex        =   5
      Top             =   60
      Width           =   5295
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   $"frmApp.frx":0442
         Height          =   435
         Index           =   1
         Left            =   60
         TabIndex        =   6
         Top             =   180
         Width           =   5175
      End
   End
   Begin VB.TextBox txtFace 
      BackColor       =   &H8000000F&
      Height          =   2295
      Left            =   120
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      ScrollBars      =   2  'Vertical
      TabIndex        =   4
      Top             =   1620
      Width           =   5235
   End
   Begin VB.CommandButton cmdMakeFace 
      Caption         =   "Make Face"
      Default         =   -1  'True
      Height          =   315
      Left            =   983
      TabIndex        =   1
      Top             =   4020
      Width           =   1035
   End
   Begin VB.Frame Frame1 
      Caption         =   "Game Status"
      Height          =   735
      Left            =   60
      TabIndex        =   0
      Top             =   840
      Width           =   5295
      Begin VB.Label lblPlayerName 
         BackStyle       =   0  'Transparent
         Height          =   255
         Left            =   1980
         TabIndex        =   8
         Top             =   180
         Width           =   3135
      End
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   "Local Player Name:"
         Height          =   195
         Index           =   2
         Left            =   120
         TabIndex        =   7
         Top             =   180
         Width           =   1935
      End
      Begin VB.Label lblPlayer 
         BackStyle       =   0  'Transparent
         Height          =   255
         Left            =   2040
         TabIndex        =   3
         Top             =   420
         Width           =   3075
      End
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   "Current number of players:"
         Height          =   195
         Index           =   0
         Left            =   120
         TabIndex        =   2
         Top             =   420
         Width           =   1935
      End
   End
End
Attribute VB_Name = "frmApp"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmApp.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Implements DirectPlay8Event
Private Const gbMSGFACE As Byte = 1
Private msName As String

Private Sub cmdExit_Click()
    Unload Me
End Sub

Private Sub cmdMakeFace_Click()
    Dim Buf() As Byte, lOffSet As Long
    'For the purpose of this sample we don't care what the contents of the buffer
    'will be.  Since there is only one application defined msg in this sample
    'sending anything will suffice.
    
    If glNumPlayers > 1 Then 'Go ahead and send this to someone
        lOffSet = NewBuffer(Buf)
        AddDataToBuffer Buf, gbMSGFACE, SIZE_BYTE, lOffSet
        dpp.SendTo DPNID_ALL_PLAYERS_GROUP, Buf, 0, DPNSEND_NOLOOPBACK
    Else
        UpdateText "There is no one to make faces at!!!"
    End If
End Sub

Private Sub Form_Load()
    'Init our vars
    InitDPlay
    
    Set DPlayEventsForm = New DPlayConnect
    'First lets get the dplay connection started
    If Not DPlayEventsForm.StartConnectWizard(dx, dpp, AppGuid, 10, Me) Then
        Cleanup
        End
    End If
    
    gfHost = DPlayEventsForm.IsHost
    msName = DPlayEventsForm.UserName
    lblPlayerName.Caption = msName
    If gfHost Then
        Me.Caption = DPlayEventsForm.SessionName & " (HOST)"
    End If
    lblPlayer.Caption = CStr(glNumPlayers)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Cleanup
End Sub

Private Sub UpdateText(ByVal sString As String)
    'Update the chat window first
    txtFace.Text = txtFace.Text & sString & vbCrLf
    'Now limit the text in the window to be 16k
    If Len(txtFace.Text) > 16384 Then
        txtFace.Text = Right$(txtFace.Text, 16384)
    End If
    'Autoscroll the text
    txtFace.SelStart = Len(txtFace.Text)
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
    Dim AppDesc As DPN_APPLICATION_DESC
    
    'Go ahead and put the session name in the title bar
    AppDesc = dpp.GetApplicationDesc
    Me.Caption = AppDesc.SessionName
End Sub

Private Sub DirectPlay8Event_CreateGroup(ByVal lGroupID As Long, ByVal lOwnerID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_CreatePlayer(ByVal lPlayerID As Long, fRejectMsg As Boolean)
    'Someone joined, increment the count
    glNumPlayers = glNumPlayers + 1
    lblPlayer.Caption = CStr(glNumPlayers)
End Sub

Private Sub DirectPlay8Event_DestroyGroup(ByVal lGroupID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_DestroyPlayer(ByVal lPlayerID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'Someone left, decrement the count
    glNumPlayers = glNumPlayers - 1
    lblPlayer.Caption = CStr(glNumPlayers)
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
    'There is only one msg that can be sent in this sample
    Dim sPeer As String
    
    sPeer = dpp.GetPeerInfo(dpnotify.idSender).Name
    UpdateText sPeer & " is making funny faces at you, " & msName
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
