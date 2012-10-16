VERSION 5.00
Begin VB.Form frmAudTut1 
   Caption         =   "Audio Tutorial 1"
   ClientHeight    =   1320
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   5460
   Icon            =   "audtut1.frx":0000
   LinkTopic       =   "Form1"
   ScaleHeight     =   1320
   ScaleWidth      =   5460
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdClose 
      Caption         =   "Close"
      Default         =   -1  'True
      Height          =   375
      Left            =   4260
      TabIndex        =   0
      Top             =   900
      Width           =   1035
   End
   Begin VB.Image Image1 
      Height          =   480
      Left            =   120
      Picture         =   "audtut1.frx":0442
      Top             =   120
      Width           =   480
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Copyright (C) 1999-2001 Microsoft Corporation, All Rights Reserved."
      Height          =   255
      Index           =   2
      Left            =   600
      TabIndex        =   3
      Top             =   300
      Width           =   4800
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "GM/GS® Sound Set Copyright ©1996, Roland Corporation U.S."
      Height          =   255
      Index           =   1
      Left            =   600
      TabIndex        =   2
      Top             =   540
      Width           =   4755
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "DirectMusic Segment Tutorial"
      Height          =   255
      Index           =   0
      Left            =   600
      TabIndex        =   1
      Top             =   60
      Width           =   2655
   End
End
Attribute VB_Name = "frmAudTut1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       audTut1.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Option Explicit
' Our DX variables
Private dx As New DirectX8
'We need a loader variable
Private dml As DirectMusicLoader8
'We need our performance object
Private dmp As DirectMusicPerformance8
'We also need our DMusic segment
Private seg As DirectMusicSegment8

Private Sub cmdClose_Click()
    Unload Me
End Sub

Private Sub Form_Load()
    Dim dmA As DMUS_AUDIOPARAMS
    'Get our loader and performance
    Set dml = dx.DirectMusicLoaderCreate
    Set dmp = dx.DirectMusicPerformanceCreate
    
    'We will put in error checking here in case we can't init DMusic
    'ie, if there is no sound card
    On Error GoTo FailedInit
    'Initialize our DMusic Audio with a default environment
    dmp.InitAudio Me.hWnd, DMUS_AUDIOF_ALL, dmA, Nothing, DMUS_APATH_SHARED_STEREOPLUSREVERB, 64
    
    'Here we will load our audio file.  We could load a wave file,
    'a midi file, and rmi file, or a DMusic segment.  For this
    'tutorial we will load a segment.
    
    'Before we load our segment, set our search directory
    dml.SetSearchDirectory FindMediaDir("sample.sgt")
    'Now we can load our segment
    Set seg = dml.LoadSegment("sample.sgt")
    'Download our segment to the default audio path (created during our call to InitAudio)
    seg.Download dmp.GetDefaultAudioPath
    'Play our segment from the beginning
    dmp.PlaySegmentEx seg, 0, 0
    Exit Sub
    
FailedInit:
    MsgBox "Could not initialize DirectMusic." & vbCrLf & "This sample will exit.", vbOKOnly Or vbInformation, "Exiting..."
    Unload Me
End Sub

Private Sub Form_Unload(Cancel As Integer)
    On Error Resume Next
    'Stops everything playing on the audio path
    dmp.StopEx dmp.GetDefaultAudioPath, 0, 0
    'Destroy all of our objects
    Set seg = Nothing
    'Closedown the performance object (we should always do this).
    dmp.CloseDown
    'Destroy the rest of our objects
    Set dmp = Nothing
    Set dml = Nothing
    Set dx = Nothing
End Sub
