VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmFlanger 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Flanger Effects Update"
   ClientHeight    =   4440
   ClientLeft      =   45
   ClientTop       =   285
   ClientWidth     =   2775
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4440
   ScaleWidth      =   2775
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.OptionButton optTriangle 
      Caption         =   "Triangle"
      Height          =   255
      Left            =   1680
      TabIndex        =   2
      Top             =   3540
      Width           =   915
   End
   Begin VB.OptionButton optSin 
      Caption         =   "Sine"
      Height          =   255
      Left            =   120
      TabIndex        =   1
      Top             =   3540
      Width           =   915
   End
   Begin VB.CommandButton cmdOK 
      Caption         =   "OK"
      Height          =   315
      Left            =   1800
      TabIndex        =   0
      Top             =   3960
      Width           =   915
   End
   Begin MSComctlLib.Slider sldFeedback 
      Height          =   195
      Left            =   60
      TabIndex        =   3
      Top             =   300
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   10
      Min             =   -99
      Max             =   99
      SelStart        =   1
      TickFrequency   =   10
      Value           =   1
   End
   Begin MSComctlLib.Slider sldDelay 
      Height          =   195
      Left            =   60
      TabIndex        =   4
      Top             =   840
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   2
      Max             =   4
   End
   Begin MSComctlLib.Slider sldDepth 
      Height          =   195
      Left            =   60
      TabIndex        =   5
      Top             =   1380
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      Max             =   100
      TickFrequency   =   10
   End
   Begin MSComctlLib.Slider sldFreq 
      Height          =   195
      Left            =   60
      TabIndex        =   6
      Top             =   1920
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      TickFrequency   =   2
   End
   Begin MSComctlLib.Slider sldPhase 
      Height          =   195
      Left            =   60
      TabIndex        =   7
      Top             =   2460
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   2
      Max             =   4
   End
   Begin MSComctlLib.Slider sldWetDry 
      Height          =   195
      Left            =   60
      TabIndex        =   8
      Top             =   3000
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   10
      SmallChange     =   5
      Max             =   100
      SelStart        =   1
      TickFrequency   =   10
      Value           =   1
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Frequency"
      Height          =   255
      Index           =   4
      Left            =   60
      TabIndex        =   15
      Top             =   1680
      Width           =   1035
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Depth"
      Height          =   255
      Index           =   3
      Left            =   60
      TabIndex        =   14
      Top             =   1140
      Width           =   1035
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Delay"
      Height          =   255
      Index           =   0
      Left            =   60
      TabIndex        =   13
      Top             =   600
      Width           =   735
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Feedback"
      Height          =   255
      Index           =   1
      Left            =   60
      TabIndex        =   12
      Top             =   60
      Width           =   735
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Phase"
      Height          =   255
      Index           =   2
      Left            =   60
      TabIndex        =   11
      Top             =   2220
      Width           =   1035
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Wave Form"
      Height          =   255
      Index           =   5
      Left            =   60
      TabIndex        =   10
      Top             =   3300
      Width           =   915
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Wet Dry Mix"
      Height          =   255
      Index           =   6
      Left            =   60
      TabIndex        =   9
      Top             =   2760
      Width           =   1035
   End
End
Attribute VB_Name = "frmFlanger"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmFlanger.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Private oBuffer As DirectSoundSecondaryBuffer8
Private mlIndex As Long

Private oFX As DirectSoundFXFlanger8

Private Sub SaveAllSettings()
    Dim fxNew As DSFXFLANGER
    
    'Ok, save these new settings
    'Set the new information up
    With fxNew
        .fFeedback = CSng(sldFeedback.Value)
        .fDelay = CSng(sldDelay.Value)
        .fDepth = CSng(sldDepth.Value)
        .fWetDryMix = CSng(sldWetDry.Value)
        .fFrequency = CSng(sldFreq.Value)
        .lPhase = sldPhase.Value
        If optSin.Value Then
            .lWaveform = DSFX_WAVE_SIN
        ElseIf optTriangle.Value Then
            .lWaveform = DSFX_WAVE_TRIANGLE
        End If
    End With
    'Now update the effect
    oFX.SetAllParameters fxNew
End Sub

Private Sub cmdOK_Click()
    SaveAllSettings
    Unload Me
End Sub

Private Sub Form_Load()
    Dim fxCurrent As DSFXFLANGER
    
    'Get the echo interface
    Set oFX = oBuffer.GetObjectinPath(DSFX_STANDARD_FLANGER, mlIndex, IID_DirectSoundFXFlanger)
    'Get the current settings from it
    fxCurrent = oFX.GetAllParameters
    'Now put them out there
    With fxCurrent
        sldFeedback.Value = CLng(.fFeedback)
        sldDelay.Value = CLng(.fDelay)
        sldDepth.Value = CLng(.fDepth)
        sldWetDry.Value = CLng(.fWetDryMix)
        sldFreq.Value = CLng(.fFrequency)
        sldPhase.Value = .lPhase
        If .lWaveform = DSFX_WAVE_SIN Then
            optSin.Value = True
        ElseIf .lWaveform = DSFX_WAVE_TRIANGLE Then
            optTriangle.Value = True
        End If
    End With
End Sub

Public Sub SetBuffer(oBuf As DirectSoundSecondaryBuffer8, Index As Long)
    'Store the buffer and index
    Set oBuffer = oBuf
    mlIndex = Index
End Sub

Private Sub optSin_Click()
    SaveAllSettings
End Sub

Private Sub optTriangle_Click()
    SaveAllSettings
End Sub

Private Sub sldDelay_Change()
    SaveAllSettings
End Sub

Private Sub sldDelay_Scroll()
    SaveAllSettings
End Sub

Private Sub sldDepth_Change()
    SaveAllSettings
End Sub

Private Sub sldDepth_Scroll()
    SaveAllSettings
End Sub

Private Sub sldFeedback_Change()
    SaveAllSettings
End Sub

Private Sub sldFeedback_Scroll()
    SaveAllSettings
End Sub

Private Sub sldFreq_Change()
    SaveAllSettings
End Sub

Private Sub sldFreq_Scroll()
    SaveAllSettings
End Sub

Private Sub sldPhase_Change()
    SaveAllSettings
End Sub

Private Sub sldPhase_Scroll()
    SaveAllSettings
End Sub

Private Sub sldWetDry_Change()
    SaveAllSettings
End Sub

Private Sub sldWetDry_Scroll()
    SaveAllSettings
End Sub
