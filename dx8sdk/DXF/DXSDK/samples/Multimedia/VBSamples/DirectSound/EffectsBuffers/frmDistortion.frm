VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmDistortion 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Distortion Effects Update"
   ClientHeight    =   3240
   ClientLeft      =   45
   ClientTop       =   285
   ClientWidth     =   2775
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3240
   ScaleWidth      =   2775
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.CommandButton cmdOK 
      Caption         =   "OK"
      Height          =   315
      Left            =   1800
      TabIndex        =   10
      Top             =   2820
      Width           =   915
   End
   Begin MSComctlLib.Slider sldGain 
      Height          =   195
      Left            =   60
      TabIndex        =   0
      Top             =   300
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   10
      Min             =   -60
      Max             =   0
      TickFrequency   =   5
   End
   Begin MSComctlLib.Slider sldEdge 
      Height          =   195
      Left            =   60
      TabIndex        =   1
      Top             =   840
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   10
      Max             =   100
      TickFrequency   =   5
   End
   Begin MSComctlLib.Slider sldPostEQCenter 
      Height          =   195
      Left            =   60
      TabIndex        =   2
      Top             =   1380
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   500
      SmallChange     =   100
      Min             =   100
      Max             =   8000
      SelStart        =   100
      TickFrequency   =   500
      Value           =   100
   End
   Begin MSComctlLib.Slider sldPostEQBand 
      Height          =   195
      Left            =   60
      TabIndex        =   3
      Top             =   1920
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   500
      SmallChange     =   100
      Min             =   100
      Max             =   8000
      SelStart        =   100
      TickFrequency   =   500
      Value           =   100
   End
   Begin MSComctlLib.Slider sldPreLow 
      Height          =   195
      Left            =   60
      TabIndex        =   4
      Top             =   2460
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   500
      SmallChange     =   100
      Min             =   100
      Max             =   8000
      SelStart        =   100
      TickFrequency   =   500
      Value           =   100
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Post EQ Bandwith"
      Height          =   255
      Index           =   4
      Left            =   60
      TabIndex        =   9
      Top             =   1680
      Width           =   2055
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Post EQ Center Frequency"
      Height          =   255
      Index           =   3
      Left            =   60
      TabIndex        =   8
      Top             =   1140
      Width           =   2475
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Edge"
      Height          =   255
      Index           =   0
      Left            =   60
      TabIndex        =   7
      Top             =   600
      Width           =   735
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Gain"
      Height          =   255
      Index           =   1
      Left            =   60
      TabIndex        =   6
      Top             =   60
      Width           =   735
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Prelow Pass Cutoff"
      Height          =   255
      Index           =   2
      Left            =   60
      TabIndex        =   5
      Top             =   2220
      Width           =   2535
   End
End
Attribute VB_Name = "frmDistortion"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmDistortion.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Private oBuffer As DirectSoundSecondaryBuffer8
Private mlIndex As Long

Private oFX As DirectSoundFXDistortion8

Private Sub SaveAllSettings()
    Dim fxNew As DSFXDISTORTION
    
    'Ok, save these new settings
    'Set the new information up
    With fxNew
        .fEdge = CSng(sldEdge.Value)
        .fGain = CSng(sldGain.Value)
        .fPostEQBandwidth = CSng(sldPostEQBand.Value)
        .fPostEQCenterFrequency = CSng(sldPostEQCenter.Value)
        .fPreLowpassCutoff = CSng(sldPreLow.Value)
    End With
    'Now update the effect
    oFX.SetAllParameters fxNew
End Sub

Private Sub cmdOK_Click()
    SaveAllSettings
    Unload Me
End Sub

Private Sub Form_Load()
    Dim fxCurrent As DSFXDISTORTION
    
    'Get the echo interface
    Set oFX = oBuffer.GetObjectinPath(DSFX_STANDARD_DISTORTION, mlIndex, IID_DirectSoundFXDistortion)
    'Get the current settings from it
    fxCurrent = oFX.GetAllParameters
    'Now put them out there
    With fxCurrent
        sldEdge.Value = CLng(.fEdge)
        sldGain.Value = CLng(.fGain)
        sldPostEQBand.Value = CLng(.fPostEQBandwidth)
        sldPostEQCenter.Value = CLng(.fPostEQCenterFrequency)
        sldPreLow.Value = CLng(.fPreLowpassCutoff)
    End With
End Sub

Public Sub SetBuffer(oBuf As DirectSoundSecondaryBuffer8, Index As Long)
    'Store the buffer and index
    Set oBuffer = oBuf
    mlIndex = Index
End Sub

Private Sub sldEdge_Change()
    SaveAllSettings
End Sub

Private Sub sldEdge_Scroll()
    SaveAllSettings
End Sub

Private Sub sldGain_Change()
    SaveAllSettings
End Sub

Private Sub sldGain_Scroll()
    SaveAllSettings
End Sub

Private Sub sldPostEQBand_Change()
    SaveAllSettings
End Sub

Private Sub sldPostEQBand_Scroll()
    SaveAllSettings
End Sub

Private Sub sldPostEQCenter_Change()
    SaveAllSettings
End Sub

Private Sub sldPostEQCenter_Scroll()
    SaveAllSettings
End Sub

Private Sub sldPreLow_Change()
    SaveAllSettings
End Sub

Private Sub sldPreLow_Scroll()
    SaveAllSettings
End Sub
