VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form frmAdjust 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Adjust Sound"
   ClientHeight    =   6420
   ClientLeft      =   240
   ClientTop       =   525
   ClientWidth     =   6900
   Icon            =   "frmAdjust.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   6420
   ScaleWidth      =   6900
   StartUpPosition =   2  'CenterScreen
   Begin VB.Timer tmrUpdate 
      Interval        =   250
      Left            =   7200
      Top             =   240
   End
   Begin MSComctlLib.Slider sldFreq 
      Height          =   270
      Left            =   2460
      TabIndex        =   17
      Top             =   960
      Width           =   3615
      _ExtentX        =   6376
      _ExtentY        =   476
      _Version        =   393216
      LargeChange     =   1000
      SmallChange     =   100
      Min             =   100
      Max             =   100000
      SelStart        =   100
      TickFrequency   =   10000
      Value           =   100
   End
   Begin MSComDlg.CommonDialog cdlFile 
      Left            =   8880
      Top             =   240
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.CommandButton cmdExit 
      Caption         =   "Exit"
      Height          =   315
      Left            =   5760
      TabIndex        =   12
      Top             =   6060
      Width           =   975
   End
   Begin VB.CommandButton cmdStop 
      Caption         =   "Stop"
      Enabled         =   0   'False
      Height          =   315
      Left            =   1080
      TabIndex        =   11
      Top             =   6060
      Width           =   975
   End
   Begin VB.CommandButton cmdPlay 
      Caption         =   "Play"
      Enabled         =   0   'False
      Height          =   315
      Left            =   60
      TabIndex        =   10
      Top             =   6060
      Width           =   975
   End
   Begin VB.CommandButton cmdSound 
      Caption         =   "Sound &File"
      Default         =   -1  'True
      Height          =   315
      Left            =   120
      TabIndex        =   9
      Top             =   120
      Width           =   975
   End
   Begin VB.CheckBox chkLoop 
      Caption         =   "Loop Sound"
      Height          =   315
      Left            =   120
      TabIndex        =   8
      Top             =   5580
      Width           =   3135
   End
   Begin VB.Frame Frame1 
      Caption         =   "Expected Behavior"
      Height          =   2475
      Index           =   1
      Left            =   120
      TabIndex        =   7
      Top             =   3060
      Width           =   6615
      Begin VB.Label lblBehavior 
         BackStyle       =   0  'Transparent
         Caption         =   "Expected Behavior Text"
         Height          =   2055
         Left            =   120
         TabIndex        =   13
         Top             =   300
         Width           =   6375
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Buffer Settings"
      Height          =   915
      Index           =   0
      Left            =   120
      TabIndex        =   6
      Top             =   2040
      Width           =   6615
      Begin VB.PictureBox Picture1 
         BorderStyle     =   0  'None
         Height          =   255
         Index           =   1
         Left            =   1260
         ScaleHeight     =   255
         ScaleWidth      =   5235
         TabIndex        =   32
         Top             =   540
         Width           =   5235
         Begin VB.OptionButton optDefault 
            Caption         =   "Default"
            Height          =   195
            Left            =   0
            TabIndex        =   35
            Top             =   0
            Value           =   -1  'True
            Width           =   1035
         End
         Begin VB.OptionButton optHardware 
            Caption         =   "Hardware"
            Height          =   195
            Left            =   1200
            TabIndex        =   34
            Top             =   0
            Width           =   1035
         End
         Begin VB.OptionButton optSoftware 
            Caption         =   "Software"
            Height          =   195
            Left            =   2400
            TabIndex        =   33
            Top             =   0
            Width           =   1035
         End
      End
      Begin VB.PictureBox Picture1 
         BorderStyle     =   0  'None
         Height          =   255
         Index           =   0
         Left            =   1260
         ScaleHeight     =   255
         ScaleWidth      =   5235
         TabIndex        =   28
         Top             =   180
         Width           =   5235
         Begin VB.OptionButton optGlobal 
            Caption         =   "Global"
            Height          =   195
            Left            =   2400
            TabIndex        =   31
            Top             =   0
            Width           =   1035
         End
         Begin VB.OptionButton optSticky 
            Caption         =   "Sticky"
            Height          =   195
            Left            =   1200
            TabIndex        =   30
            Top             =   0
            Width           =   1035
         End
         Begin VB.OptionButton optNormal 
            Caption         =   "Normal"
            Height          =   195
            Left            =   0
            TabIndex        =   29
            Top             =   0
            Value           =   -1  'True
            Width           =   1035
         End
      End
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   "Buffer Mixing"
         Height          =   195
         Index           =   11
         Left            =   120
         TabIndex        =   27
         Top             =   540
         Width           =   915
      End
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   "Focus"
         Height          =   195
         Index           =   10
         Left            =   120
         TabIndex        =   26
         Top             =   240
         Width           =   555
      End
   End
   Begin MSComctlLib.Slider sldPan 
      Height          =   270
      Left            =   2460
      TabIndex        =   24
      Top             =   1320
      Width           =   3615
      _ExtentX        =   6376
      _ExtentY        =   476
      _Version        =   393216
      LargeChange     =   1000
      SmallChange     =   100
      Min             =   -10000
      Max             =   10000
      TickFrequency   =   1000
   End
   Begin MSComctlLib.Slider sldVolume 
      Height          =   270
      Left            =   2460
      TabIndex        =   25
      Top             =   1680
      Width           =   3615
      _ExtentX        =   6376
      _ExtentY        =   476
      _Version        =   393216
      LargeChange     =   1000
      SmallChange     =   100
      Min             =   -2500
      Max             =   0
      TickFrequency   =   250
   End
   Begin VB.Label Label1 
      Alignment       =   2  'Center
      BackStyle       =   0  'Transparent
      Caption         =   "High"
      Height          =   195
      Index           =   9
      Left            =   6240
      TabIndex        =   23
      Top             =   1740
      Width           =   555
   End
   Begin VB.Label Label1 
      Alignment       =   2  'Center
      BackStyle       =   0  'Transparent
      Caption         =   "Low"
      Height          =   195
      Index           =   8
      Left            =   1860
      TabIndex        =   22
      Top             =   1740
      Width           =   555
   End
   Begin VB.Label Label1 
      Alignment       =   2  'Center
      BackStyle       =   0  'Transparent
      Caption         =   "Right"
      Height          =   195
      Index           =   7
      Left            =   6180
      TabIndex        =   21
      Top             =   1380
      Width           =   555
   End
   Begin VB.Label Label1 
      Alignment       =   2  'Center
      BackStyle       =   0  'Transparent
      Caption         =   "Left"
      Height          =   195
      Index           =   6
      Left            =   1860
      TabIndex        =   20
      Top             =   1380
      Width           =   555
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "100 KHz"
      Height          =   195
      Index           =   5
      Left            =   6120
      TabIndex        =   19
      Top             =   1020
      Width           =   615
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "100 Hz"
      Height          =   195
      Index           =   4
      Left            =   1860
      TabIndex        =   18
      Top             =   1020
      Width           =   555
   End
   Begin VB.Label lblVolume 
      BackStyle       =   0  'Transparent
      BorderStyle     =   1  'Fixed Single
      Height          =   315
      Left            =   1200
      TabIndex        =   16
      Top             =   1680
      Width           =   555
   End
   Begin VB.Label lblPan 
      BackStyle       =   0  'Transparent
      BorderStyle     =   1  'Fixed Single
      Height          =   315
      Left            =   1200
      TabIndex        =   15
      Top             =   1320
      Width           =   555
   End
   Begin VB.Label lblFrequency 
      BackStyle       =   0  'Transparent
      BorderStyle     =   1  'Fixed Single
      Height          =   315
      Left            =   1200
      TabIndex        =   14
      Top             =   960
      Width           =   555
   End
   Begin VB.Label lblStatus 
      BackStyle       =   0  'Transparent
      BorderStyle     =   1  'Fixed Single
      Height          =   315
      Left            =   1200
      TabIndex        =   5
      Top             =   540
      Width           =   5475
   End
   Begin VB.Label lblFile 
      BackStyle       =   0  'Transparent
      BorderStyle     =   1  'Fixed Single
      Height          =   315
      Left            =   1200
      TabIndex        =   4
      Top             =   120
      Width           =   5475
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "Volume"
      Height          =   195
      Index           =   3
      Left            =   180
      TabIndex        =   3
      Top             =   1740
      Width           =   795
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "Pan"
      Height          =   195
      Index           =   2
      Left            =   180
      TabIndex        =   2
      Top             =   1380
      Width           =   795
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "Frequency"
      Height          =   195
      Index           =   1
      Left            =   180
      TabIndex        =   1
      Top             =   1020
      Width           =   795
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "Status"
      Height          =   195
      Index           =   0
      Left            =   180
      TabIndex        =   0
      Top             =   600
      Width           =   795
   End
End
Attribute VB_Name = "frmAdjust"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmAdjust.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'API declare for windows folder
Private Declare Function GetWindowsDirectory Lib "kernel32" Alias "GetWindowsDirectoryA" (ByVal lpBuffer As String, ByVal nSize As Long) As Long

Private dx As New DirectX8
Private ds As DirectSound8
Private dsb As DirectSoundSecondaryBuffer8
Private msFile As String

Private Sub cmdExit_Click()
    Cleanup
    Unload Me
End Sub

Private Sub Cleanup()
    If Not (dsb Is Nothing) Then dsb.Stop
    Set dsb = Nothing
    Set ds = Nothing
    Set dx = Nothing
End Sub

Private Function InitDSound() As Boolean
    On Error GoTo FailedInit
    InitDSound = True
    Set ds = dx.DirectSoundCreate(vbNullString)
    ds.SetCooperativeLevel Me.hWnd, DSSCL_PRIORITY
    Exit Function

FailedInit:
    InitDSound = False
End Function

Private Sub cmdPlay_Click()
    Dim dsBuf As DSBUFFERDESC
    Dim bFocusSticky As Boolean, bFocusGlobal As Boolean
    Dim bMixHardware As Boolean, bMixSoftware As Boolean

    On Error GoTo ErrOut
    bFocusSticky = (optSticky.Value)
    bFocusGlobal = (optGlobal.Value)
    bMixHardware = (optHardware.Value)
    bMixSoftware = (optSoftware.Value)
        
    dsBuf.lFlags = DSBCAPS_CTRLFREQUENCY Or DSBCAPS_CTRLPAN Or DSBCAPS_CTRLVOLUME
    If bFocusGlobal Then
        dsBuf.lFlags = dsBuf.lFlags Or DSBCAPS_GLOBALFOCUS
    End If
    
    If bFocusSticky Then
        dsBuf.lFlags = dsBuf.lFlags Or DSBCAPS_STICKYFOCUS
    End If

    If bMixHardware Then
        dsBuf.lFlags = dsBuf.lFlags Or DSBCAPS_LOCHARDWARE
    End If

    If bMixSoftware Then
        dsBuf.lFlags = dsBuf.lFlags Or DSBCAPS_LOCSOFTWARE
    End If
            
    Set dsb = ds.CreateSoundBufferFromFile(msFile, dsBuf)
    'Update the buffer based on the current slider
    OnSliderChange
    If chkLoop.Value = vbChecked Then
        dsb.Play DSBPLAY_LOOPING
    Else
        dsb.Play 0
    End If
    lblStatus.Caption = "File playing."
    EnablePlayUI False
    Exit Sub
    
ErrOut:
    lblStatus.Caption = "An error occured trying to play this file with these settings."

End Sub

Private Sub cmdSound_Click()

    Static sCurDir As String
    Static lFilter As Long
    Dim dsBuf As DSBUFFERDESC
    
    'Now we should load a wave file
    'Ask them for a file to load
    UpdateStatus "Loading file..."
    With cdlFile
        .flags = cdlOFNFileMustExist Or cdlOFNHideReadOnly
        .FilterIndex = lFilter
        .Filter = "Wave Files (*.wav)|*.wav"
        .FileName = vbNullString
        If sCurDir = vbNullString Then
            'Set the init folder to \windows\media if it exists.  If not, set it to the \windows folder
            Dim sWindir As String
            sWindir = Space$(255)
            If GetWindowsDirectory(sWindir, 255) = 0 Then
                'We couldn't get the windows folder for some reason, use the c:\
                .InitDir = "C:\"
            Else
                Dim sMedia As String
                sWindir = Left$(sWindir, InStr(sWindir, Chr$(0)) - 1)
                If Right$(sWindir, 1) = "\" Then
                    sMedia = sWindir & "Media"
                Else
                    sMedia = sWindir & "\Media"
                End If
                If Dir$(sMedia, vbDirectory) <> vbNullString Then
                    .InitDir = sMedia
                Else
                    .InitDir = sWindir
                End If
            End If
        Else
            .InitDir = sCurDir
        End If
        .ShowOpen   ' Display the Open dialog box
        If .FileName = vbNullString Then
            UpdateStatus "No file loaded."
            Exit Sub 'We didn't click anything exit
        End If
        'Save the current information
        sCurDir = GetFolder(.FileName)
        lFilter = .FilterIndex
        UpdateStatus "File loaded."
        
        'Save the filename for later use
        msFile = .FileName
        If Not (dsb Is Nothing) Then dsb.Stop
        Set dsb = Nothing
        dsBuf.lFlags = DSBCAPS_CTRLFREQUENCY Or DSBCAPS_CTRLPAN Or DSBCAPS_CTRLVOLUME
        On Error Resume Next
        Set dsb = ds.CreateSoundBufferFromFile(msFile, dsBuf)
        If Err Then
            UpdateStatus "Could not create buffer."
            Exit Sub
        End If
        sldFreq.Value = dsBuf.fxFormat.lSamplesPerSec
        lblFile.Caption = .FileName
        EnablePlayUI True
    End With
    
End Sub

Private Sub cmdStop_Click()
    If Not (dsb Is Nothing) Then
        dsb.Stop 'Stop the buffer and reset it's position
        dsb.SetCurrentPosition 0
        lblStatus.Caption = "File stopped."
        EnablePlayUI True
    End If
End Sub

Private Sub Form_Load()
    'First we should set up our DirectSound object
    If Not InitDSound Then
        MsgBox "Could not initialize DirectSound.  This sample is exiting.", vbOKOnly Or vbInformation, "Failed."
        Cleanup
        Unload Me
        End
    End If
    UpdateBehaviorText
    OnSliderChange
    UpdateStatus "No file loaded."
End Sub

Private Sub UpdateBehaviorText()
    Dim sText As String
    Dim bFocusSticky As Boolean, bFocusGlobal As Boolean
    Dim bMixHardware As Boolean, bMixSoftware As Boolean

    bFocusSticky = (optSticky.Value)
    bFocusGlobal = (optGlobal.Value)
    bMixHardware = (optHardware.Value)
    bMixSoftware = (optSoftware.Value)
    'Figure what the user should expect based on the dialog choice
    If bFocusSticky Then
        sText = "With sticky focus an application using DirectSound " & _
                             "can continue to play its sticky focus buffers if the " & _
                             "user switches to another application not using " & _
                             "DirectSound.  However, if the user switches to another " & _
                             "DirectSound application, all sound buffers, both normal " & _
                             "and sticky focus, in the previous application are muted."

    ElseIf bFocusGlobal Then
        sText = "With global focus, an application using DirectSound " & _
                             "can continue to play its buffers if the user switches " & _
                             "focus to another application, even if the new application " & _
                             "uses DirectSound. The one exception is if you switch " & _
                             "focus to a DirectSound application that uses the " & _
                             "DSSCL_WRITEPRIMARY flag for its " & _
                             "cooperative level. In this case, the global sounds from " & _
                             "other applications will not be audible."
    Else
        'Normal focus
        sText = "With normal focus, an application using DirectSound " & _
                             "will mute its buffers if the user switches focus to " & _
                             "another application"
    End If


    If bMixHardware Then
        sText = sText & vbCrLf & vbCrLf & "With hardware mixing, the new buffer will be " & _
                             "forced to use hardware mixing. If the device does " & _
                             "not support hardware mixing or if the required " & _
                             "hardware memory is not available, the call to the " & _
                             "IDirectSound::CreateSoundBuffer method will fail."
    ElseIf bMixSoftware Then
        sText = sText & vbCrLf & vbCrLf & "With software mixing, the new buffer will be " & _
                             "stored in software memory and use software mixing, " & _
                             "even if hardware resources are available."
    Else
        'Default mixing
        sText = sText & vbCrLf & vbCrLf & "With default mixing, the new buffer will use " & _
                             "hardware mixing if availible, otherwise software " & _
                             "memory and mixing will be used."
    End If

    'Tell the user what to expect
    lblBehavior.Caption = sText

End Sub

Private Sub optDefault_Click()
    UpdateBehaviorText
End Sub

Private Sub optGlobal_Click()
    UpdateBehaviorText
End Sub

Private Sub optHardware_Click()
    UpdateBehaviorText
End Sub

Private Sub optNormal_Click()
    UpdateBehaviorText
End Sub

Private Sub optSoftware_Click()
    UpdateBehaviorText
End Sub

Private Sub optSticky_Click()
    UpdateBehaviorText
End Sub

Private Function GetFolder(ByVal sFile As String) As String
    Dim lCount As Long
    
    For lCount = Len(sFile) To 1 Step -1
        If Mid$(sFile, lCount, 1) = "\" Then
            GetFolder = Left$(sFile, lCount)
            Exit Function
        End If
    Next
    GetFolder = vbNullString
End Function

Private Sub OnSliderChange()
    Dim lFrequency As Long, lPan As Long, lVolume As Long
    'Get the position of the sliders
    lFrequency = sldFreq.Value
    lPan = sldPan.Value
    lVolume = sldVolume.Value

    'Set the static labels
    lblFrequency.Caption = CStr(lFrequency)
    lblPan.Caption = CStr(lPan)
    lblVolume.Caption = CStr(lVolume)

    'Set the options in the DirectSound buffer
    If Not (dsb Is Nothing) Then
        dsb.SetFrequency lFrequency
        dsb.SetPan lPan
        dsb.SetVolume lVolume
    End If
End Sub

Private Sub sldFreq_Change()
    OnSliderChange
End Sub

Private Sub sldFreq_Scroll()
    OnSliderChange
End Sub

Private Sub sldPan_Change()
    OnSliderChange
End Sub

Private Sub sldPan_Scroll()
    OnSliderChange
End Sub

Private Sub sldVolume_Change()
    OnSliderChange
End Sub

Private Sub UpdateStatus(ByVal sStatus As String)
    lblStatus.Caption = sStatus
End Sub

Private Sub sldVolume_Scroll()
    OnSliderChange
End Sub

Private Sub tmrUpdate_Timer()
    If Not (dsb Is Nothing) Then
        If (dsb.GetStatus And DSBSTATUS_PLAYING) <> DSBSTATUS_PLAYING Then
            If cmdPlay.Enabled = False Then
                EnablePlayUI True
                lblStatus.Caption = "File stopped."
            End If
        End If
    End If
End Sub

Private Sub EnablePlayUI(ByVal fEnable As Boolean)
    On Error Resume Next
    If fEnable Then
        chkLoop.Enabled = True
        cmdPlay.Enabled = True
        cmdStop.Enabled = False
        optNormal.Enabled = True
        optSticky.Enabled = True
        optGlobal.Enabled = True
        optDefault.Enabled = True
        optHardware.Enabled = True
        optSoftware.Enabled = True
        cmdSound.Enabled = True
        cmdPlay.SetFocus
    Else
        chkLoop.Enabled = False
        cmdPlay.Enabled = False
        cmdStop.Enabled = True
        optNormal.Enabled = False
        optSticky.Enabled = False
        optGlobal.Enabled = False
        optDefault.Enabled = False
        optHardware.Enabled = False
        optSoftware.Enabled = False
        cmdSound.Enabled = False
        cmdStop.SetFocus
    End If
End Sub
