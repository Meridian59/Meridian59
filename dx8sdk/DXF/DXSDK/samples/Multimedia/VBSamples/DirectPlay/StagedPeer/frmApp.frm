VERSION 5.00
Begin VB.Form frmApp 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Make a face..."
   ClientHeight    =   3135
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   4530
   Icon            =   "frmApp.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3135
   ScaleWidth      =   4530
   StartUpPosition =   3  'Windows Default
   Begin VB.TextBox txtFaces 
      BackColor       =   &H8000000F&
      Height          =   1815
      Left            =   60
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      ScrollBars      =   3  'Both
      TabIndex        =   4
      Top             =   840
      Width           =   4395
   End
   Begin VB.Frame Frame1 
      Caption         =   "Session Status"
      Height          =   735
      Left            =   60
      TabIndex        =   1
      Top             =   60
      Width           =   4395
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   "Current number of players:"
         Height          =   195
         Left            =   120
         TabIndex        =   3
         Top             =   300
         Width           =   1935
      End
      Begin VB.Label lblPlayer 
         BorderStyle     =   1  'Fixed Single
         Height          =   315
         Left            =   3780
         TabIndex        =   2
         Top             =   240
         Width           =   495
      End
   End
   Begin VB.CommandButton cmdMakeFace 
      Caption         =   "Make Face"
      Height          =   315
      Left            =   3420
      TabIndex        =   0
      Top             =   2760
      Width           =   1035
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
Private mlNumPlayers As Long

Public Sub LoadGame(ByVal lNumPlayers As Long)
    mlNumPlayers = lNumPlayers
    lblPlayer.Caption = CStr(lNumPlayers)
    DPlayEventsForm.RegisterCallback Me
    Me.Show vbModeless
End Sub

Private Sub cmdMakeFace_Click()
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    
    'It's time to start the game
    lMsg = MsgMakeFace
    lOffset = NewBuffer(oBuf)
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    'Send this message to everyone
    dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_NOLOOPBACK
End Sub

Private Sub Form_Load()
    If DPlayEventsForm.IsHost Then Me.Caption = Me.Caption & " (HOST)"
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Me.Hide
    DPlayEventsForm.DoSleep 50
    Cleanup
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
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_CreateGroup(ByVal lGroupID As Long, ByVal lOwnerID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_CreatePlayer(ByVal lPlayerID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_DestroyGroup(ByVal lGroupID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_DestroyPlayer(ByVal lPlayerID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'No one can join, but people can leave
    mlNumPlayers = mlNumPlayers - 1
    lblPlayer.Caption = CStr(mlNumPlayers)
End Sub

Private Sub DirectPlay8Event_EnumHostsQuery(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_QUERY, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_EnumHostsResponse(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_RESPONSE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_HostMigrate(ByVal lNewHostID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_IndicateConnect(dpnotify As DxVBLibA.DPNMSG_INDICATE_CONNECT, fRejectMsg As Boolean)
    'No one can connect now, we've already started.
    fRejectMsg = True
End Sub

Private Sub DirectPlay8Event_IndicatedConnectAborted(fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_InfoNotify(ByVal lMsgID As Long, ByVal lNotifyID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_Receive(dpnotify As DxVBLibA.DPNMSG_RECEIVE, fRejectMsg As Boolean)
    'There is only one msg that can be sent in this demo
    Dim sPeer As String
    Dim lMsg As Long, lOffset As Long
    
    With dpnotify
    GetDataFromBuffer .ReceivedData, lMsg, LenB(lMsg), lOffset
    Select Case lMsg
    Case MsgMakeFace
        sPeer = dpp.GetPeerInfo(dpnotify.idSender).Name
        UpdateChat sPeer & " is making funny faces at you.", txtFaces
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

