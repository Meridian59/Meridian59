VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form frmPlayMotif 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "vb PlayMotif"
   ClientHeight    =   4365
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   5955
   Icon            =   "frmPlayMotif.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4365
   ScaleWidth      =   5955
   StartUpPosition =   3  'Windows Default
   Begin MSComDlg.CommonDialog cdlOpen 
      Left            =   5160
      Top             =   1080
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
      DialogTitle     =   "Open Segment File"
   End
   Begin VB.TextBox txtStatus 
      BackColor       =   &H8000000F&
      Height          =   315
      Left            =   1320
      Locked          =   -1  'True
      TabIndex        =   16
      Top             =   600
      Width           =   4515
   End
   Begin VB.TextBox txtSegment 
      BackColor       =   &H8000000F&
      Height          =   315
      Left            =   1320
      Locked          =   -1  'True
      TabIndex        =   14
      Top             =   180
      Width           =   4515
   End
   Begin VB.OptionButton optMeasure 
      Caption         =   "Measure"
      Height          =   255
      Left            =   4800
      TabIndex        =   13
      Top             =   3600
      Value           =   -1  'True
      Width           =   975
   End
   Begin VB.OptionButton optBeat 
      Caption         =   "Beat"
      Height          =   255
      Left            =   4020
      TabIndex        =   12
      Top             =   3600
      Width           =   675
   End
   Begin VB.OptionButton optGrid 
      Caption         =   "Grid"
      Height          =   255
      Left            =   3180
      TabIndex        =   11
      Top             =   3600
      Width           =   735
   End
   Begin VB.OptionButton optImmediate 
      Caption         =   "Immediate"
      Height          =   255
      Left            =   2040
      TabIndex        =   10
      Top             =   3600
      Width           =   1035
   End
   Begin VB.OptionButton optDefault 
      Caption         =   "Default"
      Height          =   255
      Left            =   1080
      TabIndex        =   9
      Top             =   3600
      Width           =   855
   End
   Begin VB.ListBox lstMotif 
      Height          =   1815
      Left            =   60
      TabIndex        =   7
      Top             =   1680
      Width           =   5775
   End
   Begin VB.CheckBox chkLoop 
      Caption         =   "Loop Segment"
      Height          =   195
      Left            =   120
      TabIndex        =   5
      Top             =   1140
      Width           =   1395
   End
   Begin VB.CommandButton cmdStop 
      Caption         =   "&Stop"
      Height          =   315
      Left            =   2700
      TabIndex        =   4
      Top             =   1080
      Width           =   1095
   End
   Begin VB.CommandButton cmdPlay 
      Caption         =   "&Play"
      Height          =   315
      Left            =   1560
      TabIndex        =   3
      Top             =   1080
      Width           =   1095
   End
   Begin VB.CommandButton cmdExit 
      Caption         =   "E&xit"
      Height          =   315
      Left            =   4740
      TabIndex        =   2
      Top             =   3960
      Width           =   1095
   End
   Begin VB.CommandButton cmdPlayMotif 
      Caption         =   "Play &Motif"
      Height          =   315
      Left            =   60
      TabIndex        =   1
      Top             =   3960
      Width           =   1095
   End
   Begin VB.CommandButton cmdSegment 
      Caption         =   "Segment &File"
      Default         =   -1  'True
      Height          =   315
      Left            =   120
      TabIndex        =   0
      Top             =   180
      Width           =   1095
   End
   Begin VB.Label Label1 
      Alignment       =   1  'Right Justify
      BackStyle       =   0  'Transparent
      Caption         =   "Status:"
      Height          =   195
      Index           =   2
      Left            =   120
      TabIndex        =   15
      Top             =   660
      Width           =   1035
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "Align Option:"
      Height          =   195
      Index           =   1
      Left            =   60
      TabIndex        =   8
      Top             =   3600
      Width           =   915
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "Select a Motif:"
      Height          =   195
      Index           =   0
      Left            =   60
      TabIndex        =   6
      Top             =   1440
      Width           =   4635
   End
End
Attribute VB_Name = "frmPlayMotif"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmPlayMotif.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Implements DirectXEvent8

Private Type Motif_Node
    Motif As DirectMusicSegment8
    Name As String
    ListIndex As Long
End Type

Private Declare Function GetWindowsDirectory Lib "kernel32" Alias "GetWindowsDirectoryA" (ByVal lpBuffer As String, ByVal nSize As Long) As Long

Private dx As New DirectX8
Private dmPerf As DirectMusicPerformance8
Private dmLoader As DirectMusicLoader8
Private dmSegment As DirectMusicSegment8
Private mlSeg As Long
Private moMotifs() As Motif_Node

Private Sub cmdExit_Click()
    Unload Me
End Sub

Private Sub cmdPlay_Click()
    If chkLoop.Value = vbChecked Then
        dmSegment.SetRepeats INFINITE
    Else
        dmSegment.SetRepeats 0
    End If
    dmPerf.PlaySegmentEx dmSegment, 0, 0
    EnablePlayUI False
End Sub

Private Sub cmdPlayMotif_Click()
    Dim lFlags As CONST_DMUS_SEGF_FLAGS
    lFlags = DMUS_SEGF_SECONDARY
    If optBeat.Value Then lFlags = lFlags Or DMUS_SEGF_BEAT
    If optDefault.Value Then lFlags = lFlags Or DMUS_SEGF_DEFAULT
    If optGrid.Value Then lFlags = lFlags Or DMUS_SEGF_GRID
    If optImmediate.Value Then lFlags = lFlags Or DMUS_SEGF_SECONDARY
    If optMeasure.Value Then lFlags = lFlags Or DMUS_SEGF_MEASURE
    dmPerf.PlaySegmentEx moMotifs(lstMotif.ListIndex).Motif, lFlags, 0
End Sub

Private Sub cmdSegment_Click()
    Static sCurDir As String
    Static lFilter As Long
    
    'We want to open a file now
    cdlOpen.flags = cdlOFNHideReadOnly Or cdlOFNFileMustExist
    cdlOpen.FilterIndex = lFilter
    cdlOpen.Filter = "Segment Files (*.sgt)|*.sgt"
    cdlOpen.FileName = vbNullString
    If sCurDir = vbNullString Then
        'Set the init folder to \windows\media if it exists.  If not, set it to the \windows folder
        Dim sWindir As String
        sWindir = Space$(255)
        If GetWindowsDirectory(sWindir, 255) = 0 Then
            'We couldn't get the windows folder for some reason, use the c:\
            cdlOpen.InitDir = "C:\"
        Else
            Dim sMedia As String
            sWindir = Left$(sWindir, InStr(sWindir, Chr$(0)) - 1)
            If Right$(sWindir, 1) = "\" Then
                sMedia = sWindir & "Media"
            Else
                sMedia = sWindir & "\Media"
            End If
            If Dir$(sMedia, vbDirectory) <> vbNullString Then
                cdlOpen.InitDir = sMedia
            Else
                cdlOpen.InitDir = sWindir
            End If
        End If
    Else
        cdlOpen.InitDir = sCurDir
    End If
    On Local Error GoTo ClickedCancel
    cdlOpen.CancelError = True
    cdlOpen.ShowOpen   ' Display the Open dialog box
    
    'Save the current information
    sCurDir = GetFolder(cdlOpen.FileName)
    'Set the search folder to this one so we can auto download anything we need
    dmLoader.SetSearchDirectory sCurDir
    lFilter = cdlOpen.FilterIndex
            
    On Local Error GoTo NoLoadSegment
    'Before we load the segment stop one if it's playing
    cmdStop_Click
    'Now let's load the segment
    LoadSegment cdlOpen.FileName
    Exit Sub
NoLoadSegment:
    UpdateStatus "Couldn't load this segment"
ClickedCancel:
End Sub

Private Sub cmdStop_Click()
    'Stop the segment
    dmPerf.StopEx dmSegment, 0, 0
    EnablePlayUI True
    UpdateStatus "User pressed stop."
End Sub

Private Sub DirectXEvent8_DXCallback(ByVal eventid As Long)
    'Here we will handle the DMusic callbacks
    Dim dmNotification As DMUS_NOTIFICATION_PMSG
    Dim oState As DirectMusicSegmentState8
    Dim oSeg As DirectMusicSegment8
    Dim lCount As Long
    
    On Error GoTo FailedOut
    'Process all events
    Do While dmPerf.GetNotificationPMSG(dmNotification)
        If dmNotification.lNotificationOption = DMUS_NOTIFICATION_SEGEND Then 'The segment has ended
            'First we need to figure out which segment
            Set oState = dmNotification.User 'The user field holds the segment state on segment notifications
            Set oSeg = oState.GetSegment 'Get the segment from the state
            'Is this the primary segment?
            If oSeg Is dmSegment Then 'Yup
                UpdateStatus "Primary Segment stopped playing."
                EnablePlayUI True
            Else
                'Go through all of the other segments
                For lCount = 0 To UBound(moMotifs)
                    If oSeg Is moMotifs(lCount).Motif Then
                        UpdateStatus moMotifs(lCount).Name & " motif stopped playing."
                        'Now update the listbox
                        lstMotif.List(moMotifs(lCount).ListIndex) = moMotifs(lCount).Name
                    End If
                Next
            End If
        End If
        If dmNotification.lNotificationOption = DMUS_NOTIFICATION_SEGSTART Then 'The segment has started
            'First we need to figure out which segment
            Set oState = dmNotification.User 'The user field holds the segment state on segment notifications
            Set oSeg = oState.GetSegment 'Get the segment from the state
            'Is this the primary segment?
            If oSeg Is dmSegment Then 'Yup
                UpdateStatus "Primary Segment started playing."
            Else
                'Go through all of the other segments
                For lCount = 0 To UBound(moMotifs)
                    If oSeg Is moMotifs(lCount).Motif Then
                        UpdateStatus moMotifs(lCount).Name & " motif started playing."
                        'Now update the listbox
                        lstMotif.List(moMotifs(lCount).ListIndex) = moMotifs(lCount).Name & " (Playing)"
                    End If
                Next
            End If
        End If
    Loop
    Exit Sub
    
FailedOut:
    MsgBox "Error processing this Notification", vbOKOnly Or vbInformation, "Cannot Process."
    
End Sub

Private Sub Form_Load()
    Me.Show
    InitAudio
End Sub

Private Sub InitAudio()
    On Error GoTo FailedInit
    Dim dma As DMUS_AUDIOPARAMS
    Dim sMedia As String
    'Create our objects
    Set dmPerf = dx.DirectMusicPerformanceCreate
    Set dmLoader = dx.DirectMusicLoaderCreate
    
    'Set up a default audio path
    dmPerf.InitAudio Me.hWnd, DMUS_AUDIOF_ALL, dma, , DMUS_APATH_SHARED_STEREOPLUSREVERB, 128
    
    'Create an event handle
    mlSeg = dx.CreateEvent(Me)
    dmPerf.AddNotificationType DMUS_NOTIFY_ON_SEGMENT
    dmPerf.SetNotificationHandle mlSeg
    'Don't let them play a motif yet
    cmdPlayMotif.Enabled = False
    'Now let's load our default segment
    sMedia = FindMediaDir("sample.sgt")
    dmLoader.SetSearchDirectory sMedia
    If sMedia = vbNullString Then sMedia = AddDirSep(CurDir)
    LoadSegment sMedia & "sample.sgt"
    EnablePlayMotif False
    Exit Sub
    
FailedInit:
    MsgBox "Could not initialize DirectMusic." & vbCrLf & "This sample will exit.", vbOKOnly Or vbInformation, "Exiting..."
    Unload Me

End Sub

Private Sub Cleanup()
    On Error Resume Next
    'Get rid of our event
    dmPerf.RemoveNotificationType DMUS_NOTIFY_ON_SEGMENT
    dx.DestroyEvent mlSeg
    'Unload our segment
    If Not (dmSegment Is Nothing) Then dmSegment.Unload dmPerf.GetDefaultAudioPath
    Set dmSegment = Nothing
    'Get rid of our motifs
    ReDim moMotifs(0)
    
    'Cleanup
    dmPerf.CloseDown
    Set dmPerf = Nothing
    Set dmLoader = Nothing
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

Public Sub EnablePlayUI(fEnable As Boolean)
    'Enable/Disable the buttons
    If fEnable Then
        chkLoop.Enabled = True
        cmdStop.Enabled = False
        cmdPlay.Enabled = True
        cmdSegment.Enabled = True
        cmdPlay.SetFocus
    Else
        chkLoop.Enabled = False
        cmdStop.Enabled = True
        cmdPlay.Enabled = False
        cmdSegment.Enabled = False
        cmdStop.SetFocus
    End If
    If lstMotif.ListCount > 0 And lstMotif.ListIndex <> -1 Then
        EnablePlayMotif Not fEnable
    Else
        EnablePlayMotif False
    End If
End Sub

Public Sub EnablePlayMotif(ByVal fEnable As Boolean)
    cmdPlayMotif.Enabled = fEnable
End Sub

Private Sub LoadSegment(ByVal sFile As String)
    Dim lTrack As Long, lCount As Long
    Dim oStyle As DirectMusicStyle8
    Dim lTotalStyle As Long, lTempTotalStyle As Long
    
    On Error GoTo LeaveProc
    ReDim moMotifs(0)
    lstMotif.Clear
    Set dmSegment = dmLoader.LoadSegment(sFile)
    dmSegment.Download dmPerf.GetDefaultAudioPath
    txtSegment.Text = sFile
    EnablePlayUI True
    'Now let's get the motifs in this segment
    Do While True
        Set oStyle = dmSegment.GetStyle(lTrack)
        lTotalStyle = lTotalStyle + oStyle.GetMotifCount - 1
        ReDim Preserve moMotifs(lTotalStyle)
        For lCount = 0 To oStyle.GetMotifCount - 1
            lstMotif.AddItem oStyle.GetMotifName(lCount)
            Set moMotifs(lTempTotalStyle + lCount).Motif = oStyle.GetMotif(oStyle.GetMotifName(lCount))
            moMotifs(lTempTotalStyle + lCount).Name = oStyle.GetMotifName(lCount)
            moMotifs(lTempTotalStyle + lCount).ListIndex = lstMotif.ListCount - 1
        Next
        lTrack = lTrack + 1
        lTempTotalStyle = lTotalStyle
    Loop
LeaveProc:
    If lstMotif.ListCount > 0 Then lstMotif.ListIndex = 0
    UpdateStatus "File loaded."
End Sub

Private Sub UpdateStatus(sStat As String)
    txtStatus.Text = sStat
End Sub
