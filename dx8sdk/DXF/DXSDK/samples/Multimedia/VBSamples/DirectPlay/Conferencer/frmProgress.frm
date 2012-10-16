VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "mscomctl.ocx"
Begin VB.Form frmProgress 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "File Copy Progress"
   ClientHeight    =   1350
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   3870
   Icon            =   "frmProgress.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   1350
   ScaleWidth      =   3870
   StartUpPosition =   3  'Windows Default
   Begin MSComctlLib.ProgressBar bar 
      Height          =   375
      Left            =   60
      TabIndex        =   0
      Top             =   900
      Width           =   3675
      _ExtentX        =   6482
      _ExtentY        =   661
      _Version        =   393216
      Appearance      =   1
      Scrolling       =   1
   End
   Begin VB.Label lblFile 
      BackStyle       =   0  'Transparent
      Height          =   735
      Left            =   60
      TabIndex        =   1
      Top             =   60
      Width           =   3675
   End
End
Attribute VB_Name = "frmProgress"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmProgress.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Option Explicit

'This form will act both as the UI for the progress of file transfers as well
'as holding the information for the transfer

Public sFileName As String
Public lFileSize As Long
Public lDPlayID As Long
Public FileUniqueID As Long
Public lCurrentPos As Long
Public filNumber As Long
Public RequestForm As frmTransferRequest

Public Sub SetFile(ByVal sFile As String, Optional ByVal fReceive As Boolean = False)
    If fReceive Then
        lblFile.Caption = "Receiving: " & sFile
    Else
        lblFile.Caption = "Transfering: " & sFile
    End If
End Sub

Public Sub SetMax(ByVal lMax As Long)
    bar.Max = lMax
End Sub

Public Sub SetValue(ByVal lValue As Long)
    bar.Value = lValue
End Sub

