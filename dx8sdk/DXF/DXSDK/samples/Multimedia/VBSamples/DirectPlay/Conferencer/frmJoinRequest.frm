VERSION 5.00
Begin VB.Form frmJoinRequest 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Receiving a call...."
   ClientHeight    =   975
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   4680
   ControlBox      =   0   'False
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   975
   ScaleWidth      =   4680
   ShowInTaskbar   =   0   'False
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdReject 
      Cancel          =   -1  'True
      Caption         =   "Reject"
      Height          =   315
      Left            =   3420
      TabIndex        =   3
      Top             =   120
      Width           =   1155
   End
   Begin VB.CommandButton cmdAccept 
      Caption         =   "Accept"
      Default         =   -1  'True
      Height          =   315
      Left            =   3420
      TabIndex        =   2
      Top             =   540
      Width           =   1155
   End
   Begin VB.Label lblFriend 
      BackStyle       =   0  'Transparent
      Height          =   195
      Left            =   720
      TabIndex        =   1
      Top             =   420
      Width           =   2115
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "You are receiving a call from"
      Height          =   195
      Left            =   720
      TabIndex        =   0
      Top             =   180
      Width           =   2115
   End
   Begin VB.Image Image1 
      Height          =   480
      Left            =   120
      Picture         =   "frmJoinRequest.frx":0000
      Top             =   180
      Width           =   480
   End
End
Attribute VB_Name = "frmJoinRequest"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmJoinRequest.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Private mlPlayerID As Long
Private msPlayername As String
Private moForm As frmNetwork

Public Sub SetupRequest(oForm As frmNetwork, ByVal lPlayerID As Long, ByVal sPlayerName As String)
    Set moForm = oForm
    mlPlayerID = lPlayerID
    msPlayername = sPlayerName
    lblFriend.Caption = sPlayerName
End Sub

Private Sub cmdAccept_Click()
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    
    'Accept this connection
    lMsg = MsgAcceptJoin
    lOffset = NewBuffer(oBuf)
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    dpp.SendTo mlPlayerID, oBuf, 0, DPNSEND_NOLOOPBACK
    moForm.UpdatePlayerList
    'Notify everyone that this player has joined
    lMsg = MsgNewPlayerJoined
    lOffset = NewBuffer(oBuf)
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_NOLOOPBACK Or DPNSEND_GUARANTEED
    Unload Me
End Sub

Private Sub cmdReject_Click()
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    
    'Reject this connection
    lMsg = MsgRejectJoin
    lOffset = NewBuffer(oBuf)
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    dpp.SendTo mlPlayerID, oBuf, 0, DPNSEND_NOLOOPBACK
    Unload Me
End Sub
