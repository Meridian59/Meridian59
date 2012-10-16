VERSION 5.00
Begin VB.Form frmIntro 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "VB Memory - A DirectPlay Sample"
   ClientHeight    =   1515
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   2745
   Icon            =   "frmIntro.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   1515
   ScaleWidth      =   2745
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdSingle 
      Caption         =   "Solitaire"
      Height          =   375
      Left            =   315
      TabIndex        =   2
      Top             =   1020
      Width           =   975
   End
   Begin VB.CommandButton cmdMulti 
      Caption         =   "Multiplayer"
      Default         =   -1  'True
      Height          =   375
      Left            =   1455
      TabIndex        =   0
      Top             =   1020
      Width           =   975
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "This sample will show a developer a simplistic game (Memory).  Please choose how you would like to play this game."
      Height          =   1035
      Left            =   60
      TabIndex        =   1
      Top             =   60
      Width           =   2475
   End
End
Attribute VB_Name = "frmIntro"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmIntro.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Private Sub cmdMulti_Click()

    Dim StagingArea As New frmStage
    'Oh good, we want to play a multiplayer game.
    'First lets get the dplay connection started
    
    'Here we will init our DPlay objects
    InitDPlay
    Set DPlayEventsForm = New DPlayConnect
    Load StagingArea
    EnableButtons False
    'We only want to have a maximum of 4 players
    If Not DPlayEventsForm.StartConnectWizard(dx, dpp, AppGuid, 4, StagingArea) Then
        Cleanup
        EnableButtons True
    Else 'We did choose to play a game
        gsUserName = DPlayEventsForm.UserName
        Me.Hide
        StagingArea.Show vbModeless
        gfHost = DPlayEventsForm.IsHost
    End If
    
End Sub

Private Sub cmdSingle_Click()
    'We don't want to use DPlay, close down our objects
    Cleanup
    gbNumPlayers = 1
    EnableButtons False
    Me.Hide
    frmGameBoard.Show
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Cleanup
    End
End Sub

Public Sub EnableButtons(ByVal fEnable As Boolean)
    cmdMulti.Enabled = fEnable
    cmdSingle.Enabled = fEnable
End Sub
