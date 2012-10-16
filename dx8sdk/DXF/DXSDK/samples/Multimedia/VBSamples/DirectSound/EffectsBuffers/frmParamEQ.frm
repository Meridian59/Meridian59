VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmParamEQ 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "ParamEQ Effects Update"
   ClientHeight    =   2220
   ClientLeft      =   45
   ClientTop       =   285
   ClientWidth     =   2775
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   2220
   ScaleWidth      =   2775
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.CommandButton cmdOK 
      Caption         =   "OK"
      Height          =   315
      Left            =   1800
      TabIndex        =   6
      Top             =   1800
      Width           =   915
   End
   Begin MSComctlLib.Slider sldCenter 
      Height          =   195
      Left            =   60
      TabIndex        =   0
      Top             =   360
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   500
      SmallChange     =   100
      Min             =   80
      Max             =   16000
      SelStart        =   80
      TickFrequency   =   1000
      Value           =   80
   End
   Begin MSComctlLib.Slider sldBand 
      Height          =   195
      Left            =   60
      TabIndex        =   1
      Top             =   900
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   4
      Min             =   1
      Max             =   36
      SelStart        =   1
      TickFrequency   =   4
      Value           =   1
   End
   Begin MSComctlLib.Slider sldGain 
      Height          =   195
      Left            =   60
      TabIndex        =   2
      Top             =   1440
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      Min             =   -15
      Max             =   15
      TickFrequency   =   2
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Center"
      Height          =   255
      Index           =   1
      Left            =   60
      TabIndex        =   5
      Top             =   120
      Width           =   735
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Bandwith"
      Height          =   255
      Index           =   0
      Left            =   60
      TabIndex        =   4
      Top             =   660
      Width           =   735
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Gain"
      Height          =   255
      Index           =   3
      Left            =   60
      TabIndex        =   3
      Top             =   1200
      Width           =   1035
   End
End
Attribute VB_Name = "frmParamEQ"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmParamEQ.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Private oBuffer As DirectSoundSecondaryBuffer8
Private mlIndex As Long

Private oFX As DirectSoundFXParamEq8

Private Sub SaveAllSettings()
    Dim fxNew As DSFXPARAMEQ
    
    'Ok, save these new settings
    'Set the new information up
    With fxNew
        .fBandwidth = CSng(sldBand.Value)
        .fCenter = CSng(sldCenter.Value)
        .fGain = CSng(sldGain.Value)
    End With
    'Now update the effect
    oFX.SetAllParameters fxNew
End Sub

Private Sub cmdOK_Click()
    SaveAllSettings
    Unload Me
End Sub

Private Sub Form_Load()
    Dim fxCurrent As DSFXPARAMEQ
    
    'Get the echo interface
    Set oFX = oBuffer.GetObjectinPath(DSFX_STANDARD_PARAMEQ, mlIndex, IID_DirectSoundFXParamEq)
    'Get the current settings from it
    fxCurrent = oFX.GetAllParameters
    'Now put them out there
    With fxCurrent
        sldBand.Value = CLng(.fBandwidth)
        sldCenter.Value = CLng(.fCenter)
        sldGain.Value = CLng(.fGain)
    End With
End Sub

Public Sub SetBuffer(oBuf As DirectSoundSecondaryBuffer8, Index As Long)
    'Store the buffer and index
    Set oBuffer = oBuf
    mlIndex = Index
End Sub

Private Sub sldBand_Change()
    SaveAllSettings
End Sub

Private Sub sldBand_Scroll()
    SaveAllSettings
End Sub

Private Sub sldCenter_Change()
    SaveAllSettings
End Sub

Private Sub sldCenter_Scroll()
    SaveAllSettings
End Sub

Private Sub sldGain_Change()
    SaveAllSettings
End Sub

Private Sub sldGain_Scroll()
    SaveAllSettings
End Sub
