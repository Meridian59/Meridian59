VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form frmCapture 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Audio Capture Tutorial"
   ClientHeight    =   1395
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   4305
   Icon            =   "frmCapture.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   1395
   ScaleWidth      =   4305
   StartUpPosition =   3  'Windows Default
   Begin MSComDlg.CommonDialog cdlSave 
      Left            =   4140
      Top             =   0
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
      CancelError     =   -1  'True
   End
   Begin VB.CommandButton cmdSave 
      Caption         =   "Sa&ve"
      Enabled         =   0   'False
      Height          =   375
      Left            =   3038
      TabIndex        =   2
      Top             =   840
      Width           =   975
   End
   Begin VB.CommandButton cmdStop 
      Caption         =   "&Stop"
      Enabled         =   0   'False
      Height          =   375
      Left            =   1898
      TabIndex        =   1
      Top             =   840
      Width           =   975
   End
   Begin VB.CommandButton cmdStart 
      Caption         =   "&Record"
      Height          =   375
      Left            =   638
      TabIndex        =   0
      Top             =   840
      Width           =   975
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Audio Capture Tutorial"
      Height          =   255
      Index           =   0
      Left            =   660
      TabIndex        =   4
      Top             =   120
      Width           =   2655
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Copyright (C) 1999-2001 Microsoft Corporation, All Rights Reserved."
      Height          =   435
      Index           =   2
      Left            =   660
      TabIndex        =   3
      Top             =   360
      Width           =   3510
   End
   Begin VB.Image Image1 
      Height          =   480
      Left            =   120
      Picture         =   "frmCapture.frx":0442
      Top             =   120
      Width           =   480
   End
End
Attribute VB_Name = "frmCapture"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmCapture.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

'This tutorial will show basic functionality.  You will capture a buffer to memory,
'and then write it out to a file.

'Variable declarations for our app
Private dx As New DirectX8
Private dsc As DirectSoundCapture8
Private dscb As DirectSoundCaptureBuffer8
Private dscd As DSCBUFFERDESC
Private capFormat As WAVEFORMATEX
Private ds As DirectSound8

Private Sub InitCapture()

    Dim cCaps As DSCCAPS
    On Local Error Resume Next
    'We need to create a direct sound object before the capture object
    If ds Is Nothing Then Set ds = dx.DirectSoundCreate(vbNullString)
    If Err Then
        MsgBox "Unable to create a DirectSound object", vbOKOnly Or vbCritical, "Cannot continue"
        Cleanup
        End
    End If
    'First we need to create our capture buffer on the default object
    Set dsc = dx.DirectSoundCaptureCreate(vbNullString)
    If Err Then
        MsgBox "Unable to create a Capture object", vbOKOnly Or vbCritical, "Cannot continue"
        Cleanup
        End
    End If
    
    'Lets get the caps for our object
    dsc.GetCaps cCaps
    
    'Check for a capture format we will support in the sample
    If cCaps.lFormats And WAVE_FORMAT_4M08 Then
        capFormat = CreateWaveFormatEx(44100, 1, 8)
    ElseIf cCaps.lFormats And WAVE_FORMAT_2M08 Then
        capFormat = CreateWaveFormatEx(22050, 1, 8)
    ElseIf cCaps.lFormats And WAVE_FORMAT_1M08 Then
        capFormat = CreateWaveFormatEx(11025, 1, 8)
    Else
        MsgBox "Could not get the caps we need on this card.", vbOKOnly Or vbCritical, "Exiting."
        Cleanup
        End
    End If
    
End Sub

Private Sub CreateCaptureBuffer()
    dscd.fxFormat = capFormat
    dscd.lBufferBytes = capFormat.lAvgBytesPerSec * 20
    dscd.lFlags = DSCBCAPS_WAVEMAPPED
    
    Set dscb = dsc.CreateCaptureBuffer(dscd)
End Sub
Private Sub Cleanup()
    Set ds = Nothing
    Set dscb = Nothing
    Set dsc = Nothing
    Set dx = Nothing
End Sub
Private Function CreateWaveFormatEx(Hz As Long, Channels As Integer, BITS As Integer) As WAVEFORMATEX

    'Create a WaveFormatEX structure using the vars we provide
    With CreateWaveFormatEx
        .nFormatTag = WAVE_FORMAT_PCM
        .nChannels = Channels
        .lSamplesPerSec = Hz
        .nBitsPerSample = BITS
        .nBlockAlign = Channels * BITS / 8
        .lAvgBytesPerSec = .lSamplesPerSec * .nBlockAlign
        .nSize = 0
    End With
End Function

Private Sub cmdSave_Click()
    On Local Error Resume Next
    With cdlSave
        'Set our initial properties
        .FileName = vbNullString
        .flags = cdlOFNHideReadOnly
        .Filter = "Wave files(*.WAV)|*.wav"
        .ShowOpen
        If Err Then Exit Sub 'We clicked cancel
        If .FileName = vbNullString Then Exit Sub 'No file
        'Save the file to disk
        GetSoundBufferFromCapture(dscb).SaveToFile .FileName
    End With
End Sub

Private Sub cmdStart_Click()
    'We want to record sound now.
    
    'First we need to get rid of any sound we may have
    Set dscb = Nothing
    'Now get our capture buffer once more
    CreateCaptureBuffer
    
    'Now start recording
    dscb.Start DSCBSTART_DEFAULT
    'Disable/Enable our buttons accordingly
    cmdStop.Enabled = True
    cmdStart.Enabled = False
    cmdSave.Enabled = False
End Sub

Private Sub cmdStop_Click()
    Dim lbufferStatus As Long
    
    'Stop the buffer
    dscb.Stop
    
    'Disable/Enable our buttons accordingly
    cmdStop.Enabled = False
    cmdStart.Enabled = True
    cmdSave.Enabled = True
End Sub

Private Sub Form_Load()

    'Lets init our capture device
    InitCapture
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Cleanup
End Sub

Private Function GetSoundBufferFromCapture(ByVal oCaptureBuffer As DirectSoundCaptureBuffer8) As DirectSoundSecondaryBuffer8
    Dim lbufferStatus As Long
    Dim capCURS As DSCURSORS
    Dim dsd As DSBUFFERDESC
    Dim ByteBuffer() As Integer 'Our digital data from our capture buffer
    
    'Are we still capturing? If so, stop
    oCaptureBuffer.Stop
    
    'Get the capture info
    oCaptureBuffer.GetCurrentPosition capCURS
    dsd.lBufferBytes = capCURS.lWrite + 1
    dsd.fxFormat = dscd.fxFormat
    'If there is nothing to write, then exit
    If capCURS.lWrite = 0 Then Exit Function
    
    Set GetSoundBufferFromCapture = ds.CreateSoundBuffer(dsd)
    'Set the size for our new Data
    ReDim ByteBuffer(capCURS.lWrite)
    'Read the data from our capture buffer
    oCaptureBuffer.ReadBuffer 0, capCURS.lWrite, ByteBuffer(0), DSCBLOCK_DEFAULT
    'Write the data to our sound buffer
    GetSoundBufferFromCapture.WriteBuffer 0, capCURS.lWrite, ByteBuffer(0), DSBLOCK_DEFAULT

End Function
