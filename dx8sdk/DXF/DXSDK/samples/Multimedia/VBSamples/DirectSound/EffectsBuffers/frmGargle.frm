VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmGargle 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Gargle Effects Update"
   ClientHeight    =   1635
   ClientLeft      =   45
   ClientTop       =   285
   ClientWidth     =   2775
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   1635
   ScaleWidth      =   2775
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.OptionButton optTriangle 
      Caption         =   "Triangle"
      Height          =   255
      Left            =   1680
      TabIndex        =   6
      Top             =   540
      Width           =   915
   End
   Begin VB.OptionButton optSquare 
      Caption         =   "Square"
      Height          =   255
      Left            =   120
      TabIndex        =   5
      Top             =   540
      Width           =   915
   End
   Begin MSComctlLib.Slider sldRate 
      Height          =   195
      Left            =   780
      TabIndex        =   4
      Top             =   960
      Width           =   1935
      _ExtentX        =   3413
      _ExtentY        =   344
      _Version        =   393216
      LargeChange     =   100
      SmallChange     =   10
      Min             =   1
      Max             =   1000
      SelStart        =   1
      TickFrequency   =   100
      Value           =   1
   End
   Begin VB.CommandButton cmdOK 
      Caption         =   "OK"
      Height          =   315
      Left            =   1800
      TabIndex        =   3
      Top             =   1260
      Width           =   915
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Rate Hz"
      Height          =   255
      Index           =   1
      Left            =   60
      TabIndex        =   2
      Top             =   960
      Width           =   735
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Wave Type"
      Height          =   255
      Index           =   0
      Left            =   60
      TabIndex        =   1
      Top             =   300
      Width           =   915
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Here you can modify the gargle effect"
      Height          =   255
      Index           =   4
      Left            =   60
      TabIndex        =   0
      Top             =   60
      Width           =   2655
   End
End
Attribute VB_Name = "frmGargle"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmGargle.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Private oBuffer As DirectSoundSecondaryBuffer8
Private mlIndex As Long

Private oFX As DirectSoundFXGargle8

Private Sub SaveAllSettings()
    Dim fxNew As DSFXGARGLE
    
    'Ok, save these new settings
    'Set the new information up
    fxNew.lRateHz = CLng(sldRate.Value)
    If optSquare.Value Then
        fxNew.lWaveShape = DSFXGARGLE_WAVE_SQUARE
    ElseIf optTriangle.Value Then
        fxNew.lWaveShape = DSFXGARGLE_WAVE_TRIANGLE
    End If
    'Now update the effect
    oFX.SetAllParameters fxNew
End Sub

Private Sub cmdOK_Click()
    SaveAllSettings
    Unload Me
End Sub

Private Sub Form_Load()
    Dim fxCurrent As DSFXGARGLE
    
    'Get the gargle interface
    Set oFX = oBuffer.GetObjectinPath(DSFX_STANDARD_GARGLE, mlIndex, IID_DirectSoundFXGargle)
    'Get the current settings from it
    fxCurrent = oFX.GetAllParameters
    'Now put them out there
    sldRate.Value = fxCurrent.lRateHz
    If fxCurrent.lWaveShape = DSFXGARGLE_WAVE_SQUARE Then
        optSquare.Value = True
    ElseIf fxCurrent.lWaveShape = DSFXGARGLE_WAVE_TRIANGLE Then
        optTriangle.Value = True
    End If
End Sub

Public Sub SetBuffer(oBuf As DirectSoundSecondaryBuffer8, Index As Long)
    'Store the buffer and index
    Set oBuffer = oBuf
    mlIndex = Index
End Sub

Private Sub optSquare_Click()
    SaveAllSettings
End Sub

Private Sub optTriangle_Click()
    SaveAllSettings
End Sub

Private Sub sldRate_Change()
    SaveAllSettings
End Sub

Private Sub sldRate_Scroll()
    SaveAllSettings
End Sub
