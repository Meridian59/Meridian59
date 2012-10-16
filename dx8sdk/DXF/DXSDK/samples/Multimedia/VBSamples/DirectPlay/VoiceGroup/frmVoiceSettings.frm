VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmVoiceSettings 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Direct Play Voice Settings"
   ClientHeight    =   5010
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   5835
   Icon            =   "frmVoiceSettings.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   5010
   ScaleWidth      =   5835
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdOk 
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   4545
      TabIndex        =   36
      Top             =   4545
      Width           =   1215
   End
   Begin VB.CommandButton cmdCancel 
      Cancel          =   -1  'True
      Caption         =   "Cancel"
      Height          =   375
      Left            =   3255
      TabIndex        =   35
      Top             =   4545
      Width           =   1215
   End
   Begin VB.Frame fraServer 
      Caption         =   "Server Options (set only when creating a new session) "
      Height          =   1020
      Left            =   60
      TabIndex        =   1
      Top             =   3420
      Width           =   5700
      Begin VB.Frame fraCompression 
         Caption         =   " Compression Codec "
         Height          =   660
         Left            =   120
         TabIndex        =   33
         Top             =   240
         Width           =   5475
         Begin VB.ComboBox cboCompressionTypes 
            Height          =   315
            Left            =   135
            Style           =   2  'Dropdown List
            TabIndex        =   34
            Top             =   255
            Width           =   5220
         End
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Client Options (may be adjusted at any time) "
      Height          =   3195
      Left            =   60
      TabIndex        =   0
      Top             =   60
      Width           =   5715
      Begin VB.Frame Frame3 
         Caption         =   " Aggressiveness "
         Height          =   1335
         Index           =   4
         Left            =   3000
         TabIndex        =   27
         Top             =   1740
         Width           =   1755
         Begin VB.OptionButton optAggressivenessSet 
            Caption         =   "Set"
            Height          =   255
            Left            =   1020
            TabIndex        =   29
            Top             =   300
            Width           =   675
         End
         Begin VB.OptionButton optAgressivenessDefault 
            Caption         =   "Default"
            Height          =   195
            Left            =   60
            TabIndex        =   28
            Top             =   300
            Value           =   -1  'True
            Width           =   855
         End
         Begin MSComctlLib.Slider sldAggressiveness 
            Height          =   195
            Left            =   60
            TabIndex        =   30
            Top             =   780
            Width           =   1635
            _ExtentX        =   2884
            _ExtentY        =   344
            _Version        =   393216
            Min             =   1
            Max             =   100
            SelStart        =   1
            TickFrequency   =   10
            Value           =   1
         End
         Begin VB.Label Label1 
            BackStyle       =   0  'Transparent
            Caption         =   "Max"
            Height          =   195
            Index           =   9
            Left            =   1320
            TabIndex        =   32
            Top             =   1080
            Width           =   315
         End
         Begin VB.Label Label1 
            BackStyle       =   0  'Transparent
            Caption         =   "Min"
            Height          =   195
            Index           =   8
            Left            =   60
            TabIndex        =   31
            Top             =   1080
            Width           =   315
         End
      End
      Begin VB.Frame Frame3 
         Caption         =   " Quality "
         Height          =   1335
         Index           =   3
         Left            =   1020
         TabIndex        =   21
         Top             =   1740
         Width           =   1755
         Begin VB.OptionButton optQualityDefault 
            Caption         =   "Default"
            Height          =   195
            Left            =   60
            TabIndex        =   24
            Top             =   300
            Value           =   -1  'True
            Width           =   855
         End
         Begin VB.OptionButton OptQualitySet 
            Caption         =   "Set"
            Height          =   255
            Left            =   1020
            TabIndex        =   23
            Top             =   300
            Width           =   675
         End
         Begin MSComctlLib.Slider sldQuality 
            Height          =   195
            Left            =   60
            TabIndex        =   22
            Top             =   780
            Width           =   1635
            _ExtentX        =   2884
            _ExtentY        =   344
            _Version        =   393216
            Min             =   1
            Max             =   100
            SelStart        =   1
            TickFrequency   =   10
            Value           =   1
         End
         Begin VB.Label Label1 
            BackStyle       =   0  'Transparent
            Caption         =   "Min"
            Height          =   195
            Index           =   7
            Left            =   60
            TabIndex        =   26
            Top             =   1080
            Width           =   315
         End
         Begin VB.Label Label1 
            BackStyle       =   0  'Transparent
            Caption         =   "Max"
            Height          =   195
            Index           =   6
            Left            =   1320
            TabIndex        =   25
            Top             =   1080
            Width           =   315
         End
      End
      Begin VB.Frame Frame3 
         Caption         =   "Threshold"
         Height          =   1335
         Index           =   2
         Left            =   3840
         TabIndex        =   14
         Top             =   300
         Width           =   1755
         Begin VB.OptionButton optSensitivityDefault 
            Caption         =   "Default"
            Height          =   255
            Left            =   840
            TabIndex        =   17
            Top             =   300
            Width           =   855
         End
         Begin VB.OptionButton optnSensitivityAuto 
            Caption         =   "Auto"
            Height          =   195
            Left            =   60
            TabIndex        =   16
            Top             =   300
            Value           =   -1  'True
            Width           =   735
         End
         Begin VB.OptionButton optSensitivitySet 
            Caption         =   "Set"
            Height          =   255
            Left            =   420
            TabIndex        =   15
            Top             =   540
            Width           =   855
         End
         Begin MSComctlLib.Slider sldSensitivity 
            Height          =   195
            Left            =   60
            TabIndex        =   18
            Top             =   780
            Width           =   1635
            _ExtentX        =   2884
            _ExtentY        =   344
            _Version        =   393216
            Max             =   99
            TickFrequency   =   10
         End
         Begin VB.Label Label1 
            BackStyle       =   0  'Transparent
            Caption         =   "Max"
            Height          =   195
            Index           =   5
            Left            =   1320
            TabIndex        =   20
            Top             =   1080
            Width           =   315
         End
         Begin VB.Label Label1 
            BackStyle       =   0  'Transparent
            Caption         =   "Min"
            Height          =   195
            Index           =   4
            Left            =   60
            TabIndex        =   19
            Top             =   1080
            Width           =   315
         End
      End
      Begin VB.Frame Frame3 
         Caption         =   " Record Volume "
         Height          =   1335
         Index           =   1
         Left            =   1980
         TabIndex        =   7
         Top             =   300
         Width           =   1755
         Begin VB.OptionButton optRecordSet 
            Caption         =   "Set"
            Height          =   255
            Left            =   420
            TabIndex        =   13
            Top             =   540
            Width           =   855
         End
         Begin VB.OptionButton optRecordAuto 
            Caption         =   "Auto"
            Height          =   195
            Left            =   60
            TabIndex        =   10
            Top             =   300
            Value           =   -1  'True
            Width           =   735
         End
         Begin VB.OptionButton optRecordDefault 
            Caption         =   "Default"
            Height          =   255
            Left            =   840
            TabIndex        =   9
            Top             =   300
            Width           =   855
         End
         Begin MSComctlLib.Slider sldRecord 
            Height          =   195
            Left            =   60
            TabIndex        =   8
            Top             =   780
            Width           =   1635
            _ExtentX        =   2884
            _ExtentY        =   344
            _Version        =   393216
            LargeChange     =   500
            SmallChange     =   100
            Min             =   -10000
            Max             =   0
            TickFrequency   =   1000
         End
         Begin VB.Label Label1 
            BackStyle       =   0  'Transparent
            Caption         =   "Min"
            Height          =   195
            Index           =   3
            Left            =   60
            TabIndex        =   12
            Top             =   1080
            Width           =   315
         End
         Begin VB.Label Label1 
            BackStyle       =   0  'Transparent
            Caption         =   "Max"
            Height          =   195
            Index           =   2
            Left            =   1320
            TabIndex        =   11
            Top             =   1080
            Width           =   315
         End
      End
      Begin VB.Frame Frame3 
         Caption         =   " Playback Volume "
         Height          =   1335
         Index           =   0
         Left            =   120
         TabIndex        =   2
         Top             =   300
         Width           =   1755
         Begin VB.OptionButton optVolumeSet 
            Caption         =   "Set"
            Height          =   255
            Left            =   1020
            TabIndex        =   4
            Top             =   300
            Width           =   675
         End
         Begin VB.OptionButton optVolumeDefault 
            Caption         =   "Default"
            Height          =   195
            Left            =   60
            TabIndex        =   3
            Top             =   300
            Value           =   -1  'True
            Width           =   855
         End
         Begin MSComctlLib.Slider sldVolume 
            Height          =   195
            Left            =   60
            TabIndex        =   37
            Top             =   780
            Width           =   1635
            _ExtentX        =   2884
            _ExtentY        =   344
            _Version        =   393216
            LargeChange     =   500
            SmallChange     =   100
            Min             =   -10000
            Max             =   0
            TickFrequency   =   1000
         End
         Begin VB.Label Label1 
            BackStyle       =   0  'Transparent
            Caption         =   "Max"
            Height          =   195
            Index           =   1
            Left            =   1320
            TabIndex        =   6
            Top             =   1080
            Width           =   315
         End
         Begin VB.Label Label1 
            BackStyle       =   0  'Transparent
            Caption         =   "Min"
            Height          =   195
            Index           =   0
            Left            =   60
            TabIndex        =   5
            Top             =   1080
            Width           =   315
         End
      End
   End
End
Attribute VB_Name = "frmVoiceSettings"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Option Compare Text
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmVoiceSettings.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Private Sub cmdCancel_Click()
    fGotSettings = False
    Unload Me
End Sub

Private Sub cmdOk_Click()
    fGotSettings = True
    'Setup up the clients stuff
    If optAgressivenessDefault.Value Then
        oClient.lBufferAggressiveness = DVBUFFERAGGRESSIVENESS_DEFAULT
        oSession.lBufferAggressiveness = DVBUFFERAGGRESSIVENESS_DEFAULT
    Else
        oClient.lBufferAggressiveness = sldAggressiveness.Value
        oSession.lBufferAggressiveness = sldAggressiveness.Value
    End If
    If optQualityDefault Then
        oClient.lBufferQuality = DVBUFFERQUALITY_DEFAULT
        oSession.lBufferQuality = DVBUFFERQUALITY_DEFAULT
    Else
        oClient.lBufferQuality = sldQuality.Value
        oSession.lBufferQuality = sldQuality.Value
    End If
    If optnSensitivityAuto.Value Then
        oClient.lThreshold = DVTHRESHOLD_UNUSED
    ElseIf optSensitivitySet Then
        oClient.lThreshold = sldSensitivity.Value
        oClient.lFlags = oClient.lFlags Or DVCLIENTCONFIG_MANUALVOICEACTIVATED
    ElseIf optSensitivityDefault.Value Then
        oClient.lThreshold = DVTHRESHOLD_DEFAULT
        oClient.lFlags = oClient.lFlags Or DVCLIENTCONFIG_MANUALVOICEACTIVATED
    End If
    If optRecordAuto.Value Then
        oClient.lFlags = oClient.lFlags Or DVCLIENTCONFIG_AUTOVOICEACTIVATED
    ElseIf optRecordDefault.Value Then
        oClient.lRecordVolume = 0
    ElseIf optRecordSet.Value Then
        oClient.lRecordVolume = sldRecord.Value
    End If
    If optVolumeDefault Then
        oClient.lPlaybackVolume = DVPLAYBACKVOLUME_DEFAULT
    ElseIf optVolumeSet Then
        oClient.lPlaybackVolume = sldVolume.Value
    End If
    oClient.lNotifyPeriod = 0
    'Now set up the server stuff
    oSession.lSessionType = DVSESSIONTYPE_PEER
    
    Dim oData As DVCOMPRESSIONINFO
    dvServer.GetCompressionType cboCompressionTypes.ListIndex + 1, oData, 0
    oSession.guidCT = oData.guidType
    
    Unload Me
End Sub

Public Sub ClientOnly()
    fraServer.Enabled = False
    fraCompression.Enabled = False
End Sub

Private Sub Form_Load()
    Dim lIndex As Long

    'Set up the defaults
    sldAggressiveness.Min = DVBUFFERAGGRESSIVENESS_MIN
    sldAggressiveness.Max = DVBUFFERAGGRESSIVENESS_MAX
    sldAggressiveness.LargeChange = (DVBUFFERAGGRESSIVENESS_MAX - DVBUFFERAGGRESSIVENESS_MIN) \ 10
    sldAggressiveness.TickFrequency = sldAggressiveness.LargeChange
    
    sldQuality.Min = DVBUFFERQUALITY_MIN
    sldQuality.Max = DVBUFFERQUALITY_MAX
    sldQuality.LargeChange = (DVBUFFERQUALITY_MAX - DVBUFFERQUALITY_MIN) \ 10
    sldQuality.TickFrequency = sldQuality.LargeChange
    
    sldSensitivity.Min = DVTHRESHOLD_MIN
    sldSensitivity.Max = DVTHRESHOLD_MAX
    sldSensitivity.LargeChange = (DVTHRESHOLD_MAX - DVTHRESHOLD_MIN) \ 10
    sldSensitivity.TickFrequency = sldSensitivity.LargeChange
    
    Dim lCount As Long, oData As DVCOMPRESSIONINFO
    
    If (dvServer Is Nothing) Then Set dvServer = dx.DirectPlayVoiceServerCreate
    For lCount = 1 To dvServer.GetCompressionTypeCount
        dvServer.GetCompressionType lCount, oData, 0
        cboCompressionTypes.AddItem oData.strName
        If InStr(oData.strName, "sc03") Then
            lIndex = lCount - 1
        End If
    Next
    cboCompressionTypes.ListIndex = lIndex
    
End Sub

Private Sub optRecordAuto_Click()
    If optRecordAuto.Value Then
        If optSensitivityDefault.Value Then optnSensitivityAuto.Value = True
    End If
End Sub

Private Sub optSensitivityDefault_Click()
    If optSensitivityDefault.Value Then
        If optRecordAuto.Value Then optRecordSet.Value = True
    End If
End Sub

Private Sub optSensitivitySet_Click()
    If optSensitivitySet.Value = True Then
        optRecordDefault.Value = True
    End If
End Sub
