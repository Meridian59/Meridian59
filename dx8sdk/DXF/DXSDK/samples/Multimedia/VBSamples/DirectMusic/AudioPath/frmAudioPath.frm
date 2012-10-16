VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmAudioPath 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "vbAudioPath Sample"
   ClientHeight    =   5700
   ClientLeft      =   720
   ClientTop       =   780
   ClientWidth     =   7965
   Icon            =   "frmAudioPath.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   5700
   ScaleWidth      =   7965
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer tmrUpdate 
      Interval        =   50
      Left            =   8820
      Top             =   660
   End
   Begin MSComDlg.CommonDialog cdlOpen 
      Left            =   8820
      Top             =   3120
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.Frame fraPath 
      Caption         =   "Audio Path 1 (Primary)"
      Height          =   4095
      Index           =   0
      Left            =   0
      TabIndex        =   31
      Top             =   1500
      Width           =   2595
      Begin VB.TextBox txtPrimary 
         Height          =   285
         Left            =   60
         Locked          =   -1  'True
         TabIndex        =   16
         Top             =   1620
         Width           =   2175
      End
      Begin VB.CommandButton cmdAdd 
         Caption         =   "..."
         Height          =   300
         Index           =   0
         Left            =   2220
         TabIndex        =   15
         Top             =   1620
         Width           =   315
      End
      Begin VB.CheckBox chkLoop 
         Caption         =   "Loop"
         Height          =   195
         Index           =   0
         Left            =   120
         TabIndex        =   14
         Top             =   2040
         Width           =   735
      End
      Begin MSComctlLib.Slider sldX 
         Height          =   135
         Index           =   0
         Left            =   300
         TabIndex        =   18
         Top             =   3360
         Width           =   2235
         _ExtentX        =   3942
         _ExtentY        =   238
         _Version        =   393216
         Min             =   -20
         Max             =   20
         TickFrequency   =   5
      End
      Begin VB.CommandButton cmdStop 
         Caption         =   "Stop"
         Height          =   315
         Index           =   0
         Left            =   900
         TabIndex        =   13
         Top             =   1980
         Width           =   795
      End
      Begin VB.CommandButton cmdPlay 
         Caption         =   "Play"
         Height          =   315
         Index           =   0
         Left            =   1740
         TabIndex        =   12
         Top             =   1980
         Width           =   795
      End
      Begin MSComctlLib.Slider sldY 
         Height          =   135
         Index           =   0
         Left            =   300
         TabIndex        =   19
         Top             =   3600
         Width           =   2235
         _ExtentX        =   3942
         _ExtentY        =   238
         _Version        =   393216
         Min             =   -20
         Max             =   20
         TickFrequency   =   5
      End
      Begin MSComctlLib.Slider sldZ 
         Height          =   135
         Index           =   0
         Left            =   300
         TabIndex        =   20
         Top             =   3840
         Width           =   2235
         _ExtentX        =   3942
         _ExtentY        =   238
         _Version        =   393216
         Min             =   -20
         Max             =   20
         TickFrequency   =   5
      End
      Begin MSComctlLib.Slider sldVolume 
         Height          =   135
         Index           =   0
         Left            =   120
         TabIndex        =   17
         Top             =   2940
         Width           =   2415
         _ExtentX        =   4260
         _ExtentY        =   238
         _Version        =   393216
         LargeChange     =   500
         SmallChange     =   100
         Min             =   -6000
         Max             =   0
         TickFrequency   =   1000
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   $"frmAudioPath.frx":0442
         Height          =   1035
         Index           =   21
         Left            =   120
         TabIndex        =   53
         Top             =   240
         Width           =   2415
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Z"
         Height          =   255
         Index           =   10
         Left            =   120
         TabIndex        =   42
         Top             =   3780
         Width           =   195
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Y"
         Height          =   255
         Index           =   9
         Left            =   120
         TabIndex        =   41
         Top             =   3600
         Width           =   255
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "X"
         Height          =   255
         Index           =   8
         Left            =   120
         TabIndex        =   40
         Top             =   3360
         Width           =   195
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "3D Positioning"
         Height          =   255
         Index           =   7
         Left            =   120
         TabIndex        =   39
         Top             =   3120
         Width           =   2415
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Volume (hundredths of a decibel)"
         Height          =   255
         Index           =   6
         Left            =   120
         TabIndex        =   38
         Top             =   2700
         Width           =   2415
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Audio Path Settings"
         Height          =   255
         Index           =   3
         Left            =   120
         TabIndex        =   35
         Top             =   2460
         Width           =   2415
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "File in Audio Path"
         Height          =   255
         Index           =   0
         Left            =   120
         TabIndex        =   32
         Top             =   1380
         Width           =   2415
      End
   End
   Begin VB.Frame fraPath 
      Caption         =   "Audio Path 3 (Secondary)"
      Height          =   4095
      Index           =   2
      Left            =   5280
      TabIndex        =   30
      Top             =   1500
      Width           =   2595
      Begin VB.CheckBox chkLoop 
         Caption         =   "Loop"
         Height          =   195
         Index           =   2
         Left            =   1740
         TabIndex        =   2
         Top             =   1440
         Width           =   735
      End
      Begin VB.CommandButton cmdStop 
         Caption         =   "Stop"
         Height          =   315
         Index           =   2
         Left            =   1680
         TabIndex        =   0
         Top             =   2100
         Width           =   795
      End
      Begin VB.CommandButton cmdPlay 
         Caption         =   "Play"
         Height          =   315
         Index           =   2
         Left            =   1680
         TabIndex        =   1
         Top             =   1740
         Width           =   795
      End
      Begin VB.CommandButton cmdRemove 
         Caption         =   "Remove"
         Height          =   315
         Index           =   2
         Left            =   1680
         TabIndex        =   3
         Top             =   960
         Width           =   795
      End
      Begin VB.CommandButton cmdAdd 
         Caption         =   "Add"
         Height          =   315
         Index           =   2
         Left            =   1680
         TabIndex        =   4
         Top             =   600
         Width           =   795
      End
      Begin VB.ListBox lstPath 
         Height          =   1815
         Index           =   2
         Left            =   120
         TabIndex        =   5
         Top             =   600
         Width           =   1515
      End
      Begin MSComctlLib.Slider sldVolume 
         Height          =   135
         Index           =   2
         Left            =   60
         TabIndex        =   25
         Top             =   2940
         Width           =   2415
         _ExtentX        =   4260
         _ExtentY        =   238
         _Version        =   393216
         LargeChange     =   500
         SmallChange     =   100
         Min             =   -6000
         Max             =   0
         TickFrequency   =   1000
      End
      Begin MSComctlLib.Slider sldX 
         Height          =   135
         Index           =   2
         Left            =   240
         TabIndex        =   26
         Top             =   3360
         Width           =   2235
         _ExtentX        =   3942
         _ExtentY        =   238
         _Version        =   393216
         Min             =   -20
         Max             =   20
         TickFrequency   =   5
      End
      Begin MSComctlLib.Slider sldY 
         Height          =   135
         Index           =   2
         Left            =   240
         TabIndex        =   27
         Top             =   3600
         Width           =   2235
         _ExtentX        =   3942
         _ExtentY        =   238
         _Version        =   393216
         Min             =   -20
         Max             =   20
         TickFrequency   =   5
      End
      Begin MSComctlLib.Slider sldZ 
         Height          =   135
         Index           =   2
         Left            =   240
         TabIndex        =   28
         Top             =   3840
         Width           =   2235
         _ExtentX        =   3942
         _ExtentY        =   238
         _Version        =   393216
         Min             =   -20
         Max             =   20
         TickFrequency   =   5
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Z"
         Height          =   255
         Index           =   20
         Left            =   60
         TabIndex        =   52
         Top             =   3780
         Width           =   195
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Y"
         Height          =   255
         Index           =   19
         Left            =   60
         TabIndex        =   51
         Top             =   3600
         Width           =   255
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "X"
         Height          =   255
         Index           =   18
         Left            =   60
         TabIndex        =   50
         Top             =   3360
         Width           =   195
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "3D Positioning"
         Height          =   255
         Index           =   17
         Left            =   60
         TabIndex        =   49
         Top             =   3120
         Width           =   2415
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Volume (hundredths of a decibel)"
         Height          =   255
         Index           =   16
         Left            =   60
         TabIndex        =   48
         Top             =   2700
         Width           =   2415
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Audio Path Settings"
         Height          =   255
         Index           =   5
         Left            =   120
         TabIndex        =   37
         Top             =   2460
         Width           =   2415
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Files in Audio Path"
         Height          =   255
         Index           =   2
         Left            =   180
         TabIndex        =   34
         Top             =   240
         Width           =   2415
      End
   End
   Begin VB.Frame fraPath 
      Caption         =   "Audio Path 2 (Secondary)"
      Height          =   4095
      Index           =   1
      Left            =   2640
      TabIndex        =   29
      Top             =   1500
      Width           =   2595
      Begin VB.CheckBox chkLoop 
         Caption         =   "Loop"
         Height          =   195
         Index           =   1
         Left            =   1740
         TabIndex        =   8
         Top             =   1440
         Width           =   735
      End
      Begin MSComctlLib.Slider sldX 
         Height          =   135
         Index           =   1
         Left            =   240
         TabIndex        =   22
         Top             =   3360
         Width           =   2235
         _ExtentX        =   3942
         _ExtentY        =   238
         _Version        =   393216
         Min             =   -20
         Max             =   20
         TickFrequency   =   5
      End
      Begin MSComctlLib.Slider sldVolume 
         Height          =   135
         Index           =   1
         Left            =   60
         TabIndex        =   21
         Top             =   2940
         Width           =   2415
         _ExtentX        =   4260
         _ExtentY        =   238
         _Version        =   393216
         LargeChange     =   500
         SmallChange     =   100
         Min             =   -6000
         Max             =   0
         TickFrequency   =   1000
      End
      Begin VB.CommandButton cmdStop 
         Caption         =   "Stop"
         Height          =   315
         Index           =   1
         Left            =   1680
         TabIndex        =   6
         Top             =   2100
         Width           =   795
      End
      Begin VB.CommandButton cmdPlay 
         Caption         =   "Play"
         Height          =   315
         Index           =   1
         Left            =   1680
         TabIndex        =   7
         Top             =   1740
         Width           =   795
      End
      Begin VB.CommandButton cmdRemove 
         Caption         =   "Remove"
         Height          =   315
         Index           =   1
         Left            =   1680
         TabIndex        =   9
         Top             =   960
         Width           =   795
      End
      Begin VB.CommandButton cmdAdd 
         Caption         =   "Add"
         Height          =   315
         Index           =   1
         Left            =   1680
         TabIndex        =   10
         Top             =   600
         Width           =   795
      End
      Begin VB.ListBox lstPath 
         Height          =   1815
         Index           =   1
         Left            =   120
         TabIndex        =   11
         Top             =   600
         Width           =   1515
      End
      Begin MSComctlLib.Slider sldY 
         Height          =   135
         Index           =   1
         Left            =   240
         TabIndex        =   23
         Top             =   3600
         Width           =   2235
         _ExtentX        =   3942
         _ExtentY        =   238
         _Version        =   393216
         Min             =   -20
         Max             =   20
         TickFrequency   =   5
      End
      Begin MSComctlLib.Slider sldZ 
         Height          =   135
         Index           =   1
         Left            =   240
         TabIndex        =   24
         Top             =   3840
         Width           =   2235
         _ExtentX        =   3942
         _ExtentY        =   238
         _Version        =   393216
         Min             =   -20
         Max             =   20
         TickFrequency   =   5
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Z"
         Height          =   255
         Index           =   15
         Left            =   60
         TabIndex        =   47
         Top             =   3780
         Width           =   195
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Y"
         Height          =   255
         Index           =   14
         Left            =   60
         TabIndex        =   46
         Top             =   3600
         Width           =   255
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "X"
         Height          =   255
         Index           =   13
         Left            =   60
         TabIndex        =   45
         Top             =   3360
         Width           =   195
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "3D Positioning"
         Height          =   255
         Index           =   12
         Left            =   60
         TabIndex        =   44
         Top             =   3120
         Width           =   2415
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Volume (hundredths of a decibel)"
         Height          =   255
         Index           =   11
         Left            =   60
         TabIndex        =   43
         Top             =   2700
         Width           =   2415
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Audio Path Settings"
         Height          =   255
         Index           =   4
         Left            =   120
         TabIndex        =   36
         Top             =   2460
         Width           =   2415
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Files in Audio Path"
         Height          =   255
         Index           =   1
         Left            =   180
         TabIndex        =   33
         Top             =   240
         Width           =   2415
      End
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Audio Path Samples"
      Height          =   255
      Index           =   24
      Left            =   600
      TabIndex        =   56
      Top             =   120
      Width           =   2655
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Copyright (C) 1999-2001 Microsoft Corporation, All Rights Reserved."
      Height          =   195
      Index           =   23
      Left            =   600
      TabIndex        =   55
      Top             =   360
      Width           =   7335
   End
   Begin VB.Image Image1 
      Height          =   480
      Left            =   60
      Picture         =   "frmAudioPath.frx":04CC
      Top             =   120
      Width           =   480
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   $"frmAudioPath.frx":090E
      Height          =   795
      Index           =   22
      Left            =   60
      TabIndex        =   54
      Top             =   600
      Width           =   7875
   End
End
Attribute VB_Name = "frmAudioPath"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmAudioPath.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

'Note the actual max files will be 1+ this constant (0 to Max)
Private Const MAX_FILES_PER_AUDIO_PATH As Byte = 4

'First we need our directx object
Private dx As New DirectX8
'And we need the audio paths we'll be using
Private dmAudio(0 To 2) As DirectMusicAudioPath8
'Loader and Performance objects
Private dmLoader As DirectMusicLoader8
Private dmPerformance As DirectMusicPerformance8
'Each audio path can have up to 5 segments
Private dmSegment(0 To 2, 0 To MAX_FILES_PER_AUDIO_PATH) As DirectMusicSegment8
Private dmState(0 To 2, 0 To MAX_FILES_PER_AUDIO_PATH) As DirectMusicSegmentState8
'API declare for windows folder
Private Declare Function GetWindowsDirectory Lib "kernel32" Alias "GetWindowsDirectoryA" (ByVal lpBuffer As String, ByVal nSize As Long) As Long

Private Sub InitAudio()

    On Error GoTo FailedInit
    Dim dmAParams As DMUS_AUDIOPARAMS
    Dim lCount As Long
    
    'First we'll create out loader and performance
    Set dmPerformance = dx.DirectMusicPerformanceCreate
    Set dmLoader = dx.DirectMusicLoaderCreate
    
    'Now we'll init our audio, and create our audio paths
    dmPerformance.InitAudio Me.hWnd, DMUS_AUDIOF_ALL, dmAParams
    For lCount = 0 To 2
        Set dmAudio(lCount) = dmPerformance.CreateStandardAudioPath(DMUS_APATH_DYNAMIC_3D, 128, True)
        EnablePlayUI lCount, True
    Next
    Exit Sub
FailedInit:
    MsgBox "Could not initialize DirectMusic." & vbCrLf & "This sample will exit.", vbOKOnly Or vbInformation, "Exiting..."
    Unload Me

End Sub

Private Sub Cleanup()
    Dim lCount As Long, lCountInner As Long
    'We need to clean up everything now
    'First unload all of the segments
    For lCount = 0 To 2
        For lCountInner = 0 To MAX_FILES_PER_AUDIO_PATH
            If Not dmSegment(lCount, lCountInner) Is Nothing Then
                'Stop the segment if it's playing
                dmPerformance.StopEx dmSegment(lCount, lCountInner), 0, 0
                'Unload the segment from the audio path
                dmSegment(lCount, lCountInner).Unload dmAudio(lCount)
                'Destroy the segment
                Set dmSegment(lCount, lCountInner) = Nothing
                'Destroy the state
                Set dmState(lCount, lCountInner) = Nothing
            End If
        Next
    Next
    For lCount = 0 To 2
        'Destroy the audio path
        Set dmAudio(lCount) = Nothing
    Next
    'Destroy the rest of our objects
    Set dmLoader = Nothing
    If Not (dmPerformance Is Nothing) Then dmPerformance.CloseDown
    Set dmPerformance = Nothing
    Set dx = Nothing
End Sub

Private Sub cmdAdd_Click(Index As Integer)

    Static sCurDir As String
    Static lFilter As Long
    Dim lCount As Long
    
    'Now we should load a segment
    'First check to see if we already have enough segments
    If Index > 0 Then
        If lstPath(Index).ListCount > MAX_FILES_PER_AUDIO_PATH Then
            MsgBox "You already have " & CStr(MAX_FILES_PER_AUDIO_PATH + 1) & " segments loaded on this audio path.  Please remove one before attempting to load another on this audio path.", vbOKOnly Or vbInformation, "Can't load."
            Exit Sub
        End If
    End If
        
    'Ask them for a file to load
    With cdlOpen
        .flags = cdlOFNFileMustExist Or cdlOFNHideReadOnly
        .FilterIndex = lFilter
        .Filter = "Wave Files (*.wav)|*.wav|Music Files (*.mid;*.rmi)|*.mid;*.rmi|Segment Files (*.sgt)|*.sgt|All Audio Files|*.wav;*.mid;*.rmi;*.sgt|All Files (*.*)|*.*"
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
        If .FileName = vbNullString Then Exit Sub 'We didn't click anything exit
        'Save the current information
        sCurDir = GetFolder(.FileName)
        dmLoader.SetSearchDirectory sCurDir
        lFilter = .FilterIndex

        On Local Error GoTo ErrOut
        'Now lets try to create a segment
        If Index > 0 Then
            For lCount = 0 To MAX_FILES_PER_AUDIO_PATH
                If (dmSegment(Index, lCount) Is Nothing) Then
                    Set dmSegment(Index, lCount) = dmLoader.LoadSegment(.FileName)
                    If (Right$(.FileName, 4) = ".mid") Or (Right$(.FileName, 4) = ".rmi") Or (Right$(.FileName, 5) = ".midi") Then
                        dmSegment(Index, lCount).SetStandardMidiFile
                    End If
                    dmSegment(Index, lCount).Download dmAudio(Index)
                    Exit For
                End If
            Next
            'Now we can add this file
            lstPath(Index).AddItem .FileName
            lstPath(Index).ItemData(lstPath(Index).ListCount - 1) = ObjPtr(dmSegment(Index, lCount))
        Else
            Set dmSegment(Index, 0) = dmLoader.LoadSegment(.FileName)
            If (Right$(.FileName, 4) = ".mid") Or (Right$(.FileName, 4) = ".rmi") Or (Right$(.FileName, 5) = ".midi") Then
                dmSegment(Index, 0).SetStandardMidiFile
            End If
            dmSegment(Index, 0).Download dmAudio(Index)
            txtPrimary.Text = .FileName
        End If
        
    End With
    
    Exit Sub
ErrOut:
    MsgBox "An error occured trying to load this segment.", vbOKOnly Or vbInformation, "Not loaded."
End Sub

Private Sub cmdPlay_Click(Index As Integer)
    Dim lCount As Long
    If Index > 0 Then
        If lstPath(Index).ListCount < 0 Then Exit Sub 'There is nothing to if nothing is select
    ElseIf txtPrimary.Text = vbNullString Then
        Exit Sub
    End If
        
    'We can play our segments
    For lCount = 0 To MAX_FILES_PER_AUDIO_PATH 'Max num of sounds
        If Not dmSegment(Index, lCount) Is Nothing Then 'Nothing to do if there is no segment loaded
            If chkLoop(Index).Value = vbChecked Then
                dmSegment(Index, lCount).SetRepeats INFINITE
            Else
                dmSegment(Index, lCount).SetRepeats 0
            End If
            If Index = 0 Then
                'We must play the segment as default for the primary path
                Set dmState(Index, lCount) = dmPerformance.PlaySegmentEx(dmSegment(Index, lCount), DMUS_SEGF_DEFAULT, 0, , dmAudio(Index))
            Else
                'We must play the segment as secondary if we want multiple segments to repeat in the same
                'audio path.
                Set dmState(Index, lCount) = dmPerformance.PlaySegmentEx(dmSegment(Index, lCount), DMUS_SEGF_SECONDARY, 0, , dmAudio(Index))
            End If
        End If
    Next
    UpdateVolume Index
    EnablePlayUI Index, False
    cmdStop(Index).SetFocus
End Sub

Private Sub cmdRemove_Click(Index As Integer)
    Dim lCount As Long
    Dim oTempSeg As DirectMusicSegment8
    Dim lPtr As Long
    
    If lstPath(Index).ListIndex < 0 Then Exit Sub 'There is nothing to if nothing is select
    
    'Remove this item
    lPtr = lstPath(Index).ItemData(lstPath(Index).ListIndex)
    For lCount = 0 To MAX_FILES_PER_AUDIO_PATH
        If lPtr = ObjPtr(dmSegment(Index, lCount)) Then
            dmPerformance.StopEx dmSegment(Index, lCount), 0, 0
            dmSegment(Index, lCount).Unload dmAudio(Index)
            Set dmSegment(Index, lCount) = Nothing
        End If
    Next
    For lCount = lstPath(Index).ListCount - 1 To 0 Step -1
        If lstPath(Index).ItemData(lCount) = lPtr Then
            lstPath(Index).RemoveItem lCount
        End If
    Next
    
End Sub

Private Sub cmdStop_Click(Index As Integer)
    Dim lCount As Long
    If Index > 0 Then
        If lstPath(Index).ListCount < 0 Then Exit Sub 'There is nothing to if nothing is select
    ElseIf txtPrimary.Text = vbNullString Then
        Exit Sub
    End If
    
    'We can stop our segments
    For lCount = 0 To MAX_FILES_PER_AUDIO_PATH
        If Not dmSegment(Index, lCount) Is Nothing Then 'Nothing to do if there is no segment loaded
            dmPerformance.StopEx dmSegment(Index, lCount), 0, 0
            Set dmState(Index, lCount) = Nothing
        End If
    Next
    EnablePlayUI Index, True
    cmdPlay(Index).SetFocus
End Sub

Private Sub Form_Load()
    If App.PrevInstance Then 'We are already running an instance of this, exit
        Dim sSave As String
        'Activate the currently running instance
        sSave = App.Title
        App.Title = App.Title & "... duplicate instance."
        Me.Caption = App.Title & "... duplicate instance."
        AppActivate sSave
        'Get rid of ourselves
        Unload Me
        End
    End If
    InitAudio
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Cleanup
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

Private Sub lstPath_DblClick(Index As Integer)
    cmdRemove_Click Index
End Sub

Private Sub sldVolume_Click(Index As Integer)
    UpdateVolume Index
End Sub

Private Sub sldVolume_Scroll(Index As Integer)
    UpdateVolume Index
End Sub

Private Sub UpdateVolume(ByVal Index As Integer)
    'Ok , we 're changing, let's get the buffer for this path, and set the 3D position
    Dim dsb As DirectSoundSecondaryBuffer8

    'Get a 3D Buffer
    Set dsb = dmAudio(Index).GetObjectinPath(DMUS_PCHANNEL_ALL, DMUS_PATH_BUFFER, 0, GUID_ALL, 0, IID_DirectSoundSecondaryBuffer)
    'Set the new position of the 3D Buffer (immediately)
    dsb.SetVolume sldVolume(Index).Value
    Set dsb = Nothing
End Sub

Private Sub Update3D(ByVal Index As Integer)
    'Ok, we're changing, let's get the buffer for this path, and set the 3D position
    Dim dsb As DirectSound3DBuffer8
    
    'Get a 3D Buffer
    Set dsb = dmAudio(Index).GetObjectinPath(DMUS_PCHANNEL_ALL, DMUS_PATH_BUFFER, 0, GUID_ALL, 0, IID_DirectSound3DBuffer)
    'Set the new position of the 3D Buffer (immediately)
    dsb.SetPosition sldX(Index).Value / 5, sldY(Index).Value / 5, sldZ(Index).Value / 5, DS3D_IMMEDIATE
    Set dsb = Nothing
    
End Sub

Private Sub sldX_Click(Index As Integer)
    Update3D Index
End Sub

Private Sub sldX_Scroll(Index As Integer)
    Update3D Index
End Sub

Private Sub sldY_Click(Index As Integer)
    Update3D Index
End Sub

Private Sub sldY_Scroll(Index As Integer)
    Update3D Index
End Sub

Private Sub sldZ_Click(Index As Integer)
    Update3D Index
End Sub

Private Sub sldZ_Scroll(Index As Integer)
    Update3D Index
End Sub

Public Sub EnablePlayUI(ByVal lIndex As Long, fEnable As Boolean)
    'Enable/Disable the buttons
    If fEnable Then
        chkLoop(lIndex).Enabled = True
        cmdStop(lIndex).Enabled = False
        cmdPlay(lIndex).Enabled = True
        cmdAdd(lIndex).Enabled = True
        If lIndex > 0 Then cmdRemove(lIndex).Enabled = True
    Else
        chkLoop(lIndex).Enabled = False
        cmdStop(lIndex).Enabled = True
        cmdPlay(lIndex).Enabled = False
        cmdAdd(lIndex).Enabled = False
        If lIndex > 0 Then cmdRemove(lIndex).Enabled = False
    End If
End Sub

Private Sub tmrUpdate_Timer()
    Dim lCount As Long
    Dim lIndex As Long
    Dim fEnableIndex As Boolean
    
    For lIndex = 0 To 2
        fEnableIndex = True
        For lCount = 0 To MAX_FILES_PER_AUDIO_PATH
            If (Not (dmSegment(lIndex, lCount) Is Nothing)) And (Not (dmState(lIndex, lCount) Is Nothing)) Then
                If dmPerformance.IsPlaying(dmSegment(lIndex, lCount), dmState(lIndex, lCount)) Then
                    fEnableIndex = False
                End If
            End If
        Next
        EnablePlayUI lIndex, fEnableIndex
    Next
End Sub
