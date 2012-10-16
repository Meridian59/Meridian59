VERSION 5.00
Begin VB.Form frm3DAlg 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Select 3D Algorithm"
   ClientHeight    =   1755
   ClientLeft      =   45
   ClientTop       =   285
   ClientWidth     =   5700
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   1755
   ScaleWidth      =   5700
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.CommandButton cmdOK 
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   315
      Left            =   3660
      TabIndex        =   4
      Top             =   1320
      Width           =   915
   End
   Begin VB.CommandButton cmdCancel 
      Cancel          =   -1  'True
      Caption         =   "Cancel"
      Height          =   315
      Left            =   4620
      TabIndex        =   3
      Top             =   1320
      Width           =   915
   End
   Begin VB.OptionButton optHalf 
      Caption         =   "&Light Quality (WDM Only.  Good quality 3D effect, uses less CPU)"
      Height          =   255
      Left            =   60
      TabIndex        =   2
      Top             =   840
      Width           =   5415
   End
   Begin VB.OptionButton optFull 
      Caption         =   "&High Quality (WDM Only.  Highest quality 3D effect, but uses more CPU)"
      Height          =   255
      Left            =   60
      TabIndex        =   1
      Top             =   480
      Width           =   5415
   End
   Begin VB.OptionButton optNone 
      Caption         =   "&No Virtualization (WDM or VxD - CPU efficient, but basic 3D effect)"
      Height          =   255
      Left            =   60
      TabIndex        =   0
      Top             =   120
      Value           =   -1  'True
      Width           =   5415
   End
End
Attribute VB_Name = "frm3DAlg"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frm3DAlg.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Private mfOkHit As Boolean

Private Sub cmdCancel_Click()
    mfOkHit = False
    Unload Me
End Sub

Private Sub cmdOK_Click()
    mfOkHit = True
    Unload Me
End Sub

Public Property Get OKHit() As Boolean
    OKHit = mfOkHit
End Property
