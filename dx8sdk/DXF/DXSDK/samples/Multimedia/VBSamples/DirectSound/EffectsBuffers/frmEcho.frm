VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmEcho 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Echo Effects Update"
   ClientHeight    =   3090
   ClientLeft      =   45
   ClientTop       =   285
   ClientWidth     =   2775
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3090
   ScaleWidth      =   2775
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.CheckBox chkPan 
      Caption         =   "Pan Delay Max"
      Height          =   195
      Left            =   120
      TabIndex        =   9
      Top             =   2340
      Width           =   2535
   End
   Begin VB.CommandButton cmdOK 
      Caption         =   "OK"
      Height          =   315
      Left            =   1800
      TabIndex        =   8
      Top             =   2700
      Width           =   915
   End
   Begin MSComctlLib.Slider sldFeedback 
      Height          =   195
      Left            =   60
      TabIndex        =   0
      Top             =   360
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   10
      Max             =   100
      SelStart        =   1
      TickFrequency   =   10
      Value           =   1
   End
   Begin MSComctlLib.Slider sldLeft 
      Height          =   195
      Left            =   60
      TabIndex        =   2
      Top             =   900
      Width           =   2655
      _ExtentX        =   4683
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   100
      SmallChange     =   10
      Min             =   1
      Max             =   2000
      SelStart        =   1
      TickFrequency   =   100
      Value           =   1
   End
   Begin MSComctlLib.Slider sldRight 
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
      Max             =   2000
      SelStart        =   1
      TickFrequency   =   100
      Value           =   1
   End
   Begin MSComctlLib.Slider sldWetDry 
      Height          =   195
      Left            =   60
      TabIndex        =   6
      Top             =   1980
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
      Caption         =   "Wet Dry Mix"
      Height          =   255
      Index           =   4
      Left            =   60
      TabIndex        =   7
      Top             =   1740
      Width           =   1035
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Right Delay"
      Height          =   255
      Index           =   3
      Left            =   60
      TabIndex        =   5
      Top             =   1200
      Width           =   1035
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Left Delay"
      Height          =   255
      Index           =   0
      Left            =   60
      TabIndex        =   3
      Top             =   660
      Width           =   735
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Feedback"
      Height          =   255
      Index           =   1
      Left            =   60
      TabIndex        =   1
      Top             =   120
      Width           =   735
   End
End
Attribute VB_Name = "frmEcho"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmEcho.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Private oBuffer As DirectSoundSecondaryBuffer8
Private mlIndex As Long

Private oFX As DirectSoundFXEcho8

Private Sub SaveAllSettings()
    Dim fxNew As DSFXECHO
    
    'Ok, save these new settings
    'Set the new information up
    With fxNew
        .fFeedback = CSng(sldFeedback.Value)
        .fLeftDelay = CSng(sldLeft.Value)
        .fRightDelay = CSng(sldRight.Value)
        .fWetDryMix = CSng(sldWetDry.Value)
        .lPanDelay = chkPan.Value
    End With
    'Now update the effect
    oFX.SetAllParameters fxNew
End Sub

Private Sub chkPan_Click()
    SaveAllSettings
End Sub

Private Sub cmdOK_Click()
    SaveAllSettings
    Unload Me
End Sub

Private Sub Form_Load()
    Dim fxCurrent As DSFXECHO
    
    'Get the echo interface
    Set oFX = oBuffer.GetObjectinPath(DSFX_STANDARD_ECHO, mlIndex, IID_DirectSoundFXEcho)
    'Get the current settings from it
    fxCurrent = oFX.GetAllParameters
    'Now put them out there
    With fxCurrent
        sldFeedback.Value = CLng(.fFeedback)
        sldLeft.Value = CLng(.fLeftDelay)
        sldRight.Value = CLng(.fRightDelay)
        sldWetDry.Value = CLng(.fWetDryMix)
        chkPan.Value = .lPanDelay
    End With
End Sub

Public Sub SetBuffer(oBuf As DirectSoundSecondaryBuffer8, Index As Long)
    'Store the buffer and index
    Set oBuffer = oBuf
    mlIndex = Index
End Sub

Private Sub sldFeedback_Change()
    SaveAllSettings
End Sub

Private Sub sldFeedback_Scroll()
    SaveAllSettings
End Sub

Private Sub sldLeft_Change()
    SaveAllSettings
End Sub

Private Sub sldLeft_Scroll()
    SaveAllSettings
End Sub

Private Sub sldRight_Change()
    SaveAllSettings
End Sub

Private Sub sldRight_Scroll()
    SaveAllSettings
End Sub

Private Sub sldWetDry_Change()
    SaveAllSettings
End Sub

Private Sub sldWetDry_Scroll()
    SaveAllSettings
End Sub
