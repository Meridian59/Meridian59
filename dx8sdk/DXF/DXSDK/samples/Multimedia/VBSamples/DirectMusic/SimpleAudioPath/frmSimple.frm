VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmSimple 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Simple Audio Path"
   ClientHeight    =   4380
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   7245
   Icon            =   "frmSimple.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4380
   ScaleWidth      =   7245
   StartUpPosition =   3  'Windows Default
   Begin VB.Frame Frame3 
      Caption         =   "Description"
      Height          =   1695
      Left            =   3660
      TabIndex        =   17
      Top             =   2220
      Width           =   3495
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   $"frmSimple.frx":0442
         Height          =   1335
         Index           =   4
         Left            =   60
         TabIndex        =   18
         Top             =   240
         Width           =   3375
      End
   End
   Begin VB.Frame Frame2 
      Caption         =   "3D Positioning of AudioPath"
      Height          =   1695
      Left            =   60
      TabIndex        =   6
      Top             =   2220
      Width           =   3555
      Begin MSComctlLib.Slider sldX 
         Height          =   195
         Left            =   300
         TabIndex        =   10
         Top             =   420
         Width           =   2595
         _ExtentX        =   4577
         _ExtentY        =   344
         _Version        =   393216
         LargeChange     =   2
         Min             =   -20
         Max             =   20
         TickFrequency   =   4
      End
      Begin MSComctlLib.Slider sldY 
         Height          =   195
         Left            =   300
         TabIndex        =   11
         Top             =   840
         Width           =   2595
         _ExtentX        =   4577
         _ExtentY        =   344
         _Version        =   393216
         LargeChange     =   2
         Min             =   -20
         Max             =   20
         TickFrequency   =   4
      End
      Begin MSComctlLib.Slider SldZ 
         Height          =   195
         Left            =   300
         TabIndex        =   12
         Top             =   1260
         Width           =   2535
         _ExtentX        =   4471
         _ExtentY        =   344
         _Version        =   393216
         LargeChange     =   2
         Min             =   -20
         Max             =   20
         TickFrequency   =   4
      End
      Begin VB.Label lblZ 
         Alignment       =   2  'Center
         BackStyle       =   0  'Transparent
         Height          =   255
         Left            =   2940
         TabIndex        =   21
         Top             =   1260
         Width           =   555
      End
      Begin VB.Label lblY 
         Alignment       =   2  'Center
         BackStyle       =   0  'Transparent
         Height          =   255
         Left            =   2940
         TabIndex        =   20
         Top             =   840
         Width           =   555
      End
      Begin VB.Label lblX 
         Alignment       =   2  'Center
         BackStyle       =   0  'Transparent
         Height          =   255
         Left            =   2940
         TabIndex        =   19
         Top             =   420
         Width           =   555
      End
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   "Z"
         Height          =   255
         Index           =   2
         Left            =   120
         TabIndex        =   9
         Top             =   1260
         Width           =   135
      End
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   "Y"
         Height          =   255
         Index           =   1
         Left            =   120
         TabIndex        =   8
         Top             =   840
         Width           =   135
      End
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   "X"
         Height          =   255
         Index           =   0
         Left            =   120
         TabIndex        =   7
         Top             =   420
         Width           =   135
      End
   End
   Begin VB.CommandButton cmdExit 
      Cancel          =   -1  'True
      Caption         =   "E&xit"
      Height          =   375
      Left            =   5760
      TabIndex        =   5
      Top             =   3960
      Width           =   1395
   End
   Begin VB.Frame Frame1 
      Caption         =   "DirectMusic Segments"
      Height          =   1995
      Left            =   60
      TabIndex        =   0
      Top             =   180
      Width           =   7095
      Begin VB.CommandButton cmdSeg 
         Caption         =   "Rude Awakening"
         Height          =   315
         Index           =   3
         Left            =   120
         TabIndex        =   4
         Top             =   1500
         Width           =   1395
      End
      Begin VB.CommandButton cmdSeg 
         Caption         =   "Mumble"
         Height          =   315
         Index           =   2
         Left            =   120
         TabIndex        =   3
         Top             =   1080
         Width           =   1395
      End
      Begin VB.CommandButton cmdSeg 
         Caption         =   "Snore"
         Height          =   315
         Index           =   1
         Left            =   120
         TabIndex        =   2
         Top             =   660
         Width           =   1395
      End
      Begin VB.CommandButton cmdSeg 
         Caption         =   "Lullaby"
         Height          =   315
         Index           =   0
         Left            =   120
         TabIndex        =   1
         Top             =   240
         Width           =   1395
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Segment file.  Stops all sound on audio path.  Plays as primary segment."
         Height          =   255
         Index           =   3
         Left            =   1620
         TabIndex        =   16
         Top             =   1560
         Width           =   5295
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Wave file.  Overlaps if pressed twice.  Plays as secondary segment."
         Height          =   255
         Index           =   2
         Left            =   1620
         TabIndex        =   15
         Top             =   1140
         Width           =   5295
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Segment file.  Overlaps if pressed twice.  Plays as secondary segment."
         Height          =   255
         Index           =   1
         Left            =   1620
         TabIndex        =   14
         Top             =   720
         Width           =   5295
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Segment file.  Starts over if pressed twice.  Plays as primary segment."
         Height          =   255
         Index           =   0
         Left            =   1620
         TabIndex        =   13
         Top             =   300
         Width           =   5295
      End
   End
End
Attribute VB_Name = "frmSimple"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmSimple.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Private dx As New DirectX8
Private dmPerf As DirectMusicPerformance8
Private dmLoad As DirectMusicLoader8
Private dmPath As DirectMusicAudioPath8
Private dmSeg(0 To 3) As DirectMusicSegment8

Private sMediaFolder As String

Private Sub cmdExit_Click()
    Unload Me
End Sub

Private Sub cmdSeg_Click(Index As Integer)
    If Not (dmSeg(Index) Is Nothing) Then
        If Index = 0 Then
            'Lullaby theme. This should play as a primary segment.
            dmPerf.PlaySegmentEx dmSeg(Index), DMUS_SEGF_DEFAULT, 0, Nothing, dmPath
        ElseIf Index = 3 Then
            'Rude awakening. Notice that this also passes the audio path in pFrom, indicating that
            'all segments currently playing on the audio path should be stopped at the exact time
            'this starts.
            dmPerf.PlaySegmentEx dmSeg(Index), 0, 0, dmPath, dmPath
        ElseIf Index < 3 Then
            'Sound effects. These play as secondary segments so they can be triggered multiple
            'times and will layer on top.
            dmPerf.PlaySegmentEx dmSeg(Index), DMUS_SEGF_SECONDARY Or DMUS_SEGF_DEFAULT, 0, , dmPath
        End If
    End If
End Sub

Private Sub Form_Load()
    SetPosition
    InitAudio
End Sub

Private Sub InitAudio()
    On Error GoTo FailedInit
    Dim dma As DMUS_AUDIOPARAMS
    'Create our objects
    Set dmPerf = dx.DirectMusicPerformanceCreate
    Set dmLoad = dx.DirectMusicLoaderCreate
    'Initialize our audio
    dmPerf.InitAudio Me.hWnd, DMUS_AUDIOF_ALL, dma
    
    Set dmPath = dmPerf.CreateStandardAudioPath(DMUS_APATH_DYNAMIC_3D, 64, True)
    sMediaFolder = FindMediaDir("audiopath1.sgt")
    If sMediaFolder = vbNullString Then sMediaFolder = AddDirSep(CurDir)
    dmLoad.SetSearchDirectory sMediaFolder
    'Now load the segments
    Set dmSeg(0) = dmLoad.LoadSegment(sMediaFolder & "audiopath1.sgt")
    Set dmSeg(1) = dmLoad.LoadSegment(sMediaFolder & "audiopath2.sgt")
    Set dmSeg(2) = dmLoad.LoadSegment(sMediaFolder & "audiopath3.wav")
    Set dmSeg(3) = dmLoad.LoadSegment(sMediaFolder & "audiopath4.sgt")
    
    'Download our segments onto the audio path
    Dim lCount As Long
    For lCount = 0 To 3
        dmSeg(lCount).Download dmPath
    Next
    Exit Sub
    
FailedInit:
    MsgBox "Could not initialize DirectMusic." & vbCrLf & "This sample will exit.", vbOKOnly Or vbInformation, "Exiting..."
    Unload Me
    
End Sub

Private Sub Cleanup()
    Dim lCount As Long
        
    'Unload all of our segments
    For lCount = 0 To 3
        If Not (dmSeg(lCount) Is Nothing) Then
            dmSeg(lCount).Unload dmPath
        End If
        Set dmSeg(lCount) = Nothing
    Next
    'Destroy everything
    Set dmPath = Nothing
    dmPerf.CloseDown
    Set dmPerf = Nothing
    Set dmLoad = Nothing
End Sub

Private Sub SetPosition()
    Dim dsb As DirectSound3DBuffer8
    
    If Not (dmPath Is Nothing) Then
        'First, get the 3D interface from the buffer by using GetObjectInPath.
        Set dsb = dmPath.GetObjectinPath(DMUS_PCHANNEL_ALL, DMUS_PATH_BUFFER, 0, GUID_ALL, 0, IID_DirectSound3DBuffer)
        If Not (dsb Is Nothing) Then
            'Then, set the coordinates
            dsb.SetPosition sldX.Value, sldY.Value, SldZ.Value, DS3D_IMMEDIATE
        End If
    End If
    'Update the text boxes as well
    lblX.Caption = CStr(sldX.Value)
    lblY.Caption = CStr(sldY.Value)
    lblZ.Caption = CStr(SldZ.Value)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Cleanup
End Sub

'Update the 3D Positions when any of the scroll bars move
Private Sub sldX_Click()
    SetPosition
End Sub

Private Sub sldX_Scroll()
    SetPosition
End Sub

Private Sub sldY_Click()
    SetPosition
End Sub

Private Sub sldY_Scroll()
    SetPosition
End Sub

Private Sub SldZ_Click()
    SetPosition
End Sub

Private Sub SldZ_Scroll()
    SetPosition
End Sub
