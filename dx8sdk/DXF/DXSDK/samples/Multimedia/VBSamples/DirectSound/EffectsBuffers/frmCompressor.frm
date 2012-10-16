VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmCompressor 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Compressor Effects Update"
   ClientHeight    =   2145
   ClientLeft      =   45
   ClientTop       =   285
   ClientWidth     =   5550
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   2145
   ScaleWidth      =   5550
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.CommandButton cmdOK 
      Caption         =   "OK"
      Height          =   315
      Left            =   4560
      TabIndex        =   12
      Top             =   1680
      Width           =   915
   End
   Begin MSComctlLib.Slider sldAttack 
      Height          =   195
      Left            =   60
      TabIndex        =   0
      Top             =   300
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   10
      SmallChange     =   5
      Min             =   1
      Max             =   500
      SelStart        =   1
      TickFrequency   =   33
      Value           =   1
   End
   Begin MSComctlLib.Slider sldOutputGain 
      Height          =   195
      Left            =   60
      TabIndex        =   1
      Top             =   840
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   10
      SmallChange     =   2
      Min             =   -60
      Max             =   60
      TickFrequency   =   10
   End
   Begin MSComctlLib.Slider sldDelay 
      Height          =   195
      Left            =   60
      TabIndex        =   2
      Top             =   1380
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   1
      Max             =   4
   End
   Begin MSComctlLib.Slider sldRatio 
      Height          =   195
      Left            =   2820
      TabIndex        =   3
      Top             =   840
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   10
      Min             =   1
      Max             =   100
      SelStart        =   1
      TickFrequency   =   10
      Value           =   1
   End
   Begin MSComctlLib.Slider sldRelease 
      Height          =   195
      Left            =   2835
      TabIndex        =   4
      Top             =   1365
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   10
      SmallChange     =   5
      Min             =   50
      Max             =   3000
      SelStart        =   50
      TickFrequency   =   100
      Value           =   50
   End
   Begin MSComctlLib.Slider sldThreshold 
      Height          =   195
      Left            =   2820
      TabIndex        =   10
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
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Threshold"
      Height          =   255
      Index           =   8
      Left            =   2820
      TabIndex        =   11
      Top             =   60
      Width           =   735
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Ratio"
      Height          =   255
      Index           =   4
      Left            =   2820
      TabIndex        =   9
      Top             =   600
      Width           =   1035
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Pre-Delay"
      Height          =   255
      Index           =   3
      Left            =   60
      TabIndex        =   8
      Top             =   1140
      Width           =   1035
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Gain"
      Height          =   255
      Index           =   0
      Left            =   60
      TabIndex        =   7
      Top             =   600
      Width           =   1215
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Attack"
      Height          =   255
      Index           =   1
      Left            =   60
      TabIndex        =   6
      Top             =   60
      Width           =   735
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Release"
      Height          =   255
      Index           =   2
      Left            =   2835
      TabIndex        =   5
      Top             =   1125
      Width           =   1035
   End
End
Attribute VB_Name = "frmCompressor"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmCompressor.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Private oBuffer As DirectSoundSecondaryBuffer8
Private mlIndex As Long

Private oFX As DirectSoundFXCompressor8
Private Sub SaveAllSettings()
    Dim fxNew As DSFXCOMPRESSOR
    
    'Ok, save these new settings
    'Set the new information up
    With fxNew
        .fAttack = CSng(sldAttack.Value)
        .fGain = CSng(sldOutputGain.Value)
        .fPredelay = CSng(sldDelay.Value)
        .fRatio = CSng(sldRatio.Value)
        .fRelease = CSng(sldRelease.Value)
        .fThreshold = CSng(sldThreshold.Value)
    End With
    'Now update the effect
    oFX.SetAllParameters fxNew
End Sub

Private Sub cmdOK_Click()
    SaveAllSettings
    Unload Me
End Sub

Private Sub Form_Load()
    Dim fxCurrent As DSFXCOMPRESSOR
    
    'Get the echo interface
    Set oFX = oBuffer.GetObjectinPath(DSFX_STANDARD_COMPRESSOR, mlIndex, IID_DirectSoundFXCompressor)
    'Get the current settings from it
    fxCurrent = oFX.GetAllParameters
    'Now put them out there
    With fxCurrent
        sldAttack.Value = CLng(.fAttack)
        sldOutputGain.Value = CLng(.fGain)
        sldDelay.Value = CLng(.fPredelay)
        sldRatio.Value = CLng(.fRatio)
        sldRelease.Value = CLng(.fRelease)
        sldThreshold.Value = CLng(.fThreshold)
    End With
End Sub

Public Sub SetBuffer(oBuf As DirectSoundSecondaryBuffer8, Index As Long)
    'Store the buffer and index
    Set oBuffer = oBuf
    mlIndex = Index
End Sub

Private Sub sldAttack_Change()
    SaveAllSettings
End Sub

Private Sub sldAttack_Scroll()
    SaveAllSettings
End Sub

Private Sub sldDelay_Change()
    SaveAllSettings
End Sub

Private Sub sldDelay_Scroll()
    SaveAllSettings
End Sub

Private Sub sldOutputGain_Change()
    SaveAllSettings
End Sub

Private Sub sldOutputGain_Scroll()
    SaveAllSettings
End Sub

Private Sub sldRatio_Change()
    SaveAllSettings
End Sub

Private Sub sldRatio_Scroll()
    SaveAllSettings
End Sub

Private Sub sldRelease_Change()
    SaveAllSettings
End Sub

Private Sub sldRelease_Scroll()
    SaveAllSettings
End Sub

Private Sub sldThreshold_Change()
    SaveAllSettings
End Sub

Private Sub sldThreshold_Scroll()
    SaveAllSettings
End Sub
