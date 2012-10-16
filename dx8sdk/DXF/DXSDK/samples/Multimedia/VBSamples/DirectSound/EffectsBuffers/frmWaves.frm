VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmWaves 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "WavesReverb Effects Update"
   ClientHeight    =   2775
   ClientLeft      =   45
   ClientTop       =   285
   ClientWidth     =   2775
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   2775
   ScaleWidth      =   2775
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.CommandButton cmdOK 
      Caption         =   "OK"
      Height          =   315
      Left            =   1800
      TabIndex        =   8
      Top             =   2340
      Width           =   915
   End
   Begin MSComctlLib.Slider sldInGain 
      Height          =   195
      Left            =   60
      TabIndex        =   0
      Top             =   360
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   10
      Min             =   -96
      Max             =   0
      TickFrequency   =   10
   End
   Begin MSComctlLib.Slider sldReverbMix 
      Height          =   195
      Left            =   60
      TabIndex        =   2
      Top             =   900
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   10
      SmallChange     =   2
      Min             =   -96
      Max             =   0
      TickFrequency   =   10
   End
   Begin MSComctlLib.Slider sldReverbTime 
      Height          =   195
      Left            =   60
      TabIndex        =   4
      Top             =   1440
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   100
      SmallChange     =   10
      Min             =   1
      Max             =   3000
      SelStart        =   1
      TickFrequency   =   200
      Value           =   1
   End
   Begin MSComctlLib.Slider sldHighFreq 
      Height          =   195
      Left            =   60
      TabIndex        =   6
      Top             =   1980
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   100
      SmallChange     =   10
      Min             =   1
      Max             =   999
      SelStart        =   1
      TickFrequency   =   50
      Value           =   1
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "High Freq RT Ratio"
      Height          =   255
      Index           =   4
      Left            =   60
      TabIndex        =   7
      Top             =   1740
      Width           =   1695
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Reverb Time"
      Height          =   255
      Index           =   3
      Left            =   60
      TabIndex        =   5
      Top             =   1200
      Width           =   1035
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Reverb Mix"
      Height          =   255
      Index           =   0
      Left            =   60
      TabIndex        =   3
      Top             =   660
      Width           =   1275
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "In Gain"
      Height          =   255
      Index           =   1
      Left            =   60
      TabIndex        =   1
      Top             =   120
      Width           =   735
   End
End
Attribute VB_Name = "frmWaves"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmWaves.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Private oBuffer As DirectSoundSecondaryBuffer8
Private mlIndex As Long

Private oFX As DirectSoundFXWavesReverb8

Private Sub SaveAllSettings()
    Dim fxNew As DSFXWAVESREVERB
    
    'Ok, save these new settings
    'Set the new information up
    With fxNew
        .fInGain = CSng(sldInGain.Value)
        .fReverbMix = CSng(sldReverbMix.Value)
        .fReverbTime = CSng(sldReverbTime.Value)
        .fHighFreqRTRatio = CSng(sldHighFreq.Value / 1000) 'Range is 0.001 - 0.999
    End With
    'Now update the effect
    oFX.SetAllParameters fxNew
End Sub

Private Sub cmdOK_Click()
    SaveAllSettings
    Unload Me
End Sub

Private Sub Form_Load()
    Dim fxCurrent As DSFXWAVESREVERB
    
    'Get the echo interface
    Set oFX = oBuffer.GetObjectinPath(DSFX_STANDARD_WAVES_REVERB, mlIndex, IID_DirectSoundFXWavesReverb)
    'Get the current settings from it
    fxCurrent = oFX.GetAllParameters
    'Now put them out there
    With fxCurrent
        sldInGain.Value = CLng(.fInGain)
        sldReverbMix.Value = CLng(.fReverbMix)
        sldReverbTime.Value = CLng(.fReverbTime)
        sldHighFreq.Value = CLng(.fHighFreqRTRatio * 1000) 'Range is 0.001 - 0.999
    End With
End Sub

Public Sub SetBuffer(oBuf As DirectSoundSecondaryBuffer8, Index As Long)
    'Store the buffer and index
    Set oBuffer = oBuf
    mlIndex = Index
End Sub

Private Sub sldHighFreq_Change()
    SaveAllSettings
End Sub

Private Sub sldHighFreq_Scroll()
    SaveAllSettings
End Sub

Private Sub sldInGain_Change()
    SaveAllSettings
End Sub

Private Sub sldInGain_Scroll()
    SaveAllSettings
End Sub

Private Sub sldReverbMix_Change()
    SaveAllSettings
End Sub

Private Sub sldReverbMix_Scroll()
    SaveAllSettings
End Sub

Private Sub sldReverbTime_Change()
    SaveAllSettings
End Sub

Private Sub sldReverbTime_Scroll()
    SaveAllSettings
End Sub
