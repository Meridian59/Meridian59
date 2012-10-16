VERSION 5.00
Begin VB.Form AudTut2 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Audio Tutorial 2"
   ClientHeight    =   3405
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   3390
   Icon            =   "audtut2.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3405
   ScaleWidth      =   3390
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdStop 
      Caption         =   "Stop"
      Height          =   375
      Left            =   1718
      TabIndex        =   6
      Top             =   2940
      Width           =   855
   End
   Begin VB.CommandButton cmdPlay 
      Caption         =   "Play"
      Height          =   375
      Left            =   818
      TabIndex        =   5
      Top             =   2940
      Width           =   855
   End
   Begin VB.Frame fraSound 
      Caption         =   "Sound Type"
      Height          =   1275
      Left            =   60
      TabIndex        =   4
      Top             =   1560
      Width           =   3255
      Begin VB.OptionButton optSeg 
         Caption         =   "DirectMusic Segment"
         Height          =   255
         Left            =   180
         TabIndex        =   9
         Top             =   900
         Width           =   2655
      End
      Begin VB.OptionButton optMid 
         Caption         =   "Midi File"
         Height          =   255
         Left            =   180
         TabIndex        =   8
         Top             =   600
         Width           =   1815
      End
      Begin VB.OptionButton optWave 
         Caption         =   "Wave File"
         Height          =   255
         Left            =   180
         TabIndex        =   7
         Top             =   300
         Value           =   -1  'True
         Width           =   1815
      End
   End
   Begin VB.HScrollBar scrlPan 
      Height          =   255
      LargeChange     =   2
      Left            =   1080
      Max             =   10
      Min             =   -10
      TabIndex        =   1
      Top             =   1200
      Width           =   2235
   End
   Begin VB.HScrollBar scrlVol 
      Height          =   255
      LargeChange     =   20
      Left            =   1080
      Max             =   0
      Min             =   -5000
      SmallChange     =   500
      TabIndex        =   0
      Top             =   840
      Width           =   2235
   End
   Begin VB.Image Image1 
      Height          =   480
      Left            =   120
      Picture         =   "audtut2.frx":0442
      Top             =   180
      Width           =   480
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Copyright (C) 1999-2001 Microsoft Corporation, All Rights Reserved."
      Height          =   435
      Index           =   2
      Left            =   660
      TabIndex        =   11
      Top             =   300
      Width           =   2655
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Audio Tutorial 2"
      Height          =   255
      Index           =   0
      Left            =   660
      TabIndex        =   10
      Top             =   60
      Width           =   2655
   End
   Begin VB.Label Label2 
      Caption         =   "Pan"
      Height          =   255
      Left            =   120
      TabIndex        =   3
      Top             =   1200
      Width           =   975
   End
   Begin VB.Label Label1 
      Caption         =   "Volume"
      Height          =   255
      Left            =   120
      TabIndex        =   2
      Top             =   840
      Width           =   1095
   End
End
Attribute VB_Name = "AudTut2"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       audTut2.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'===========================================================
'DirectX Audio Tutorial 2
'This tutorial will show you how to load a wave, midi, or
'segment file in to an audio path, and then play back the
'file.  It will also show how to get an object from an audio
'path and manipulate it
'===========================================================

Option Explicit

Private dx As DirectX8
Private dml As DirectMusicLoader8
Private dmp As DirectMusicPerformance8
Private dmSeg As DirectMusicSegment8
Private dmSegState As DirectMusicSegmentState8

Private dmPath As DirectMusicAudioPath8
Private mlOffset As Long

Private MediaPath As String

Private Sub Form_Load()
    
    Dim dmA As DMUS_AUDIOPARAMS
    MediaPath = FindMediaDir("tuta.wav")
    
    '===========================================
    '- Step 1 initialize the DirectX objects.
    '===========================================
    
    On Local Error Resume Next
    
    Set dx = New DirectX8
    Set dml = dx.DirectMusicLoaderCreate
    Set dmp = dx.DirectMusicPerformanceCreate
    dml.SetSearchDirectory MediaPath
    
    '========================================================
    '- Step 2
    '  Now we can init our audio environment, and check for any errors
    '  if a sound card is not present or DirectX is not
    '  installed. The 'On Local Error Resume Next'
    '  statement allows us to check error values immediately
    '  after execution. The error number 0 indicates no error.
    '========================================================

    dmp.InitAudio Me.hWnd, DMUS_AUDIOF_ALL, dmA
    
    If Err.Number <> 0 Then
        MsgBox "Could not initialize DirectMusic." & vbCrLf & "This sample will exit.", vbOKOnly Or vbInformation, "Exiting..."
        Unload Me
        Exit Sub
    End If

    '===========================================================
    '- Step 3 Now we can set up our Audio path which will play
    ' the sounds.
    '============================================================
    Set dmPath = dmp.CreateStandardAudioPath(DMUS_APATH_DYNAMIC_3D, 64, True)
    
End Sub


Sub LoadSound(sFile As String)

    '========================================================================
    '- Step 4 We can now load our audio file.
    ' But first we'll make sure the file exists
    '=========================================================================
    On Error Resume Next 'We'll check our error later in the app
    If Dir$(sFile) = vbNullString Then
        MsgBox "Unable to find " & sFile, vbOKOnly Or vbCritical, "Not found."
        Unload Me
    End If
    
    'Cleanup if there is already a segment loaded
    If Not (dmSeg Is Nothing) Then
        dmSeg.Unload dmPath
        Set dmSeg = Nothing
    End If
    Set dmSeg = dml.LoadSegment(sFile)
    If Right$(sFile, 4) = ".mid" Then dmSeg.SetStandardMidiFile
    
    '========================================
    '- Step 5 Download the segment, and make sure we have no errors
    '========================================
    
    dmSeg.Download dmPath
    If Err.Number <> 0 Then
        MsgBox "Unable to download segment.", vbOKOnly Or vbCritical, "No download."
        Unload Me
    End If

    scrlPan_Change
    scrlVol_Change
    
End Sub

'===============================
' Step 6 - PLAYING THE SOUNDS
'===============================

Private Sub cmdPlay_Click()

    '=========================================================
    ' Make sure we've loaded our sound
    '=========================================================
    If dmSeg Is Nothing Then
        If optWave.Value Then LoadSound MediaPath & "\tuta.wav"
        If optMid.Value Then LoadSound MediaPath & "\tut.mid"
        If optSeg.Value Then LoadSound MediaPath & "\sample.sgt"
    End If
            
    
    '================================================
    'Plays the sound
    '================================================

    Set dmSegState = dmp.PlaySegmentEx(dmSeg, 0, 0, Nothing, dmPath)
    
End Sub

'==================
'- Step 7 Add Stop
'==================

Private Sub cmdStop_Click()
    If dmSeg Is Nothing Then Exit Sub
    dmp.StopEx dmSeg, 0, 0
    mlOffset = 0
End Sub

'======================================================================
'- Step 8 Add Handler for setting the volume
'
'  volume is set in db and ranges from -10000 to 0
'  (direct sound doesn't amplify sounds just decreases their volume)
'  because db is a log scale -6000 is almost the same as
'  off and changes near zero have more effect on the volume
'  than those at -6000. we use a -5000 to 0
'======================================================================

Private Sub scrlVol_Change()
    'We can just set our volume
    dmPath.SetVolume scrlVol.Value, 0
End Sub
Private Sub scrlVol_Scroll()
    scrlVol_Change
End Sub

'===============================================================
'- Step 9 Add Handler for Pan
'===============================================================

Private Sub scrlPan_Change()
    If dmSeg Is Nothing Then Exit Sub
    'Now we need to get the corresponding Sound buffer, and make the call
    Dim dsBuf As DirectSound3DBuffer8
    
    Set dsBuf = dmPath.GetObjectinPath(DMUS_PCHANNEL_ALL, DMUS_PATH_BUFFER, 0, vbNullString, 0, "IID_IDirectSound3DBuffer")
    dsBuf.SetPosition scrlPan.Value / 5, 0, 0, DS3D_IMMEDIATE
    Set dsBuf = Nothing
End Sub
Private Sub scrlPan_Scroll()
    scrlPan_Change
End Sub

Private Sub Form_Unload(Cancel As Integer)
    'Here we will cleanup any of our code
    'First we should stop any currently playing sounds
    If Not (dmSeg Is Nothing) Then dmp.StopEx dmSeg, 0, 0
    Set dml = Nothing
    Set dmSeg = Nothing
    dmp.CloseDown
    Set dmPath = Nothing
    Set dmp = Nothing
    Set dx = Nothing
End Sub

'If we click on any of the option buttons
'we should load the new a new segment
Private Sub optMid_Click()
    cmdStop_Click
    LoadSound MediaPath & "\tut.mid"
End Sub

Private Sub optSeg_Click()
    cmdStop_Click
    LoadSound MediaPath & "\sample.sgt"
End Sub

Private Sub optWave_Click()
    cmdStop_Click
    LoadSound MediaPath & "\tuta.wav"
End Sub

