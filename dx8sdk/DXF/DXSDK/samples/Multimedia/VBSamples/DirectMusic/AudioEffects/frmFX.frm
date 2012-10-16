VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form frmEffects 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Audio Effects using DirectMusic AudioPaths"
   ClientHeight    =   4845
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   4800
   Icon            =   "frmFX.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   4845
   ScaleWidth      =   4800
   StartUpPosition =   3  'Windows Default
   Begin VB.CheckBox chkLoop 
      Caption         =   "Loop Sound"
      Height          =   315
      Left            =   780
      TabIndex        =   15
      Top             =   4380
      Width           =   1455
   End
   Begin VB.CommandButton cmdStop 
      Caption         =   "&Stop"
      Height          =   375
      Left            =   3540
      TabIndex        =   14
      Top             =   4380
      Width           =   1095
   End
   Begin VB.CommandButton cmdPlay 
      Caption         =   "&Play"
      Height          =   375
      Left            =   2340
      TabIndex        =   13
      Top             =   4380
      Width           =   1095
   End
   Begin VB.Frame fraEffects 
      Caption         =   "Effects Information"
      Height          =   3495
      Left            =   120
      TabIndex        =   2
      Top             =   780
      Width           =   4515
      Begin VB.TextBox txtFile 
         Height          =   285
         Left            =   120
         Locked          =   -1  'True
         TabIndex        =   9
         Text            =   "No file loaded..."
         Top             =   480
         Width           =   3855
      End
      Begin VB.CommandButton cmdBrowse 
         Caption         =   "..."
         Height          =   285
         Left            =   3960
         TabIndex        =   8
         Top             =   480
         Width           =   315
      End
      Begin VB.ListBox lstAvail 
         Height          =   1815
         ItemData        =   "frmFX.frx":0442
         Left            =   120
         List            =   "frmFX.frx":045E
         TabIndex        =   7
         Top             =   1080
         Width           =   1875
      End
      Begin VB.ListBox lstUse 
         Height          =   1815
         Left            =   2400
         TabIndex        =   6
         Top             =   1080
         Width           =   1875
      End
      Begin VB.CommandButton cmdAdd 
         Caption         =   ">"
         Height          =   285
         Left            =   2040
         TabIndex        =   5
         Top             =   1500
         Width           =   315
      End
      Begin VB.CommandButton cmdRemove 
         Caption         =   "<"
         Height          =   285
         Left            =   2040
         TabIndex        =   4
         Top             =   2220
         Width           =   315
      End
      Begin VB.CommandButton cmdApply 
         Caption         =   "Apply Effects"
         Height          =   315
         Left            =   3120
         TabIndex        =   3
         Top             =   3000
         Width           =   1215
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Currently loaded sound file:"
         Height          =   195
         Index           =   0
         Left            =   120
         TabIndex        =   12
         Top             =   240
         Width           =   4515
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Available Effects"
         Height          =   195
         Index           =   1
         Left            =   120
         TabIndex        =   11
         Top             =   840
         Width           =   1215
      End
      Begin VB.Label lbl 
         BackStyle       =   0  'Transparent
         Caption         =   "Effects in use"
         Height          =   195
         Index           =   2
         Left            =   2700
         TabIndex        =   10
         Top             =   840
         Width           =   1215
      End
   End
   Begin MSComDlg.CommonDialog cdlOpen 
      Left            =   300
      Top             =   3720
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Audio Effects using DirectMusic Audiopaths"
      Height          =   255
      Index           =   4
      Left            =   660
      TabIndex        =   1
      Top             =   60
      Width           =   3195
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Copyright (C) 1999-2001 Microsoft Corporation, All Rights Reserved."
      Height          =   435
      Index           =   3
      Left            =   660
      TabIndex        =   0
      Top             =   300
      Width           =   3555
   End
   Begin VB.Image Image1 
      Height          =   480
      Left            =   120
      Picture         =   "frmFX.frx":04AF
      Top             =   180
      Width           =   480
   End
End
Attribute VB_Name = "frmEffects"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmFX.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'API declare for windows folder
Private Declare Function GetWindowsDirectory Lib "kernel32" Alias "GetWindowsDirectoryA" (ByVal lpBuffer As String, ByVal nSize As Long) As Long

Private Const mlMaxEffects As Long = 20
'Private declares for our DirectX objects
Private dx As DirectX8
Private dmp As DirectMusicPerformance8
Private dml As DirectMusicLoader8
Private dmSeg As DirectMusicSegment8
Private dmSegState As DirectMusicSegmentState8

Private Sub cmdAdd_Click()
    If lstAvail.ListIndex = -1 Then 'Nothing is selected
        MsgBox "Please select an available effect before attempting to add it.", vbOKOnly Or vbInformation, "Nothing selected."
        Exit Sub
    End If
    If lstUse.ListCount >= mlMaxEffects Then
        MsgBox "You cannot add more than " & CStr(mlMaxEffects) & " effects in this sample.", vbOKOnly Or vbInformation, "No more effects."
        Exit Sub
    End If
    'Add this item to our list of effects
    lstUse.AddItem lstAvail.List(lstAvail.ListIndex)
End Sub

Private Sub cmdApply_Click()
    On Local Error GoTo NoFX
    Dim DSEffects() As DSEFFECTDESC
    Dim lResults() As Long
    Dim lCount As Long
    Dim dsb As DirectSoundSecondaryBuffer8
    
    'Do we have a sound buffer
    If dmSeg Is Nothing Then
        MsgBox "You must first load an audio file before you can apply effects to it.", vbOKOnly Or vbInformation, "No buffer"
        Exit Sub
    End If
    'Yup, stop a sound already playing
    If dmp.IsPlaying(dmSeg, dmSegState) = True Then
        MsgBox "Stop the currently playing sound before adding effects.", vbOKOnly Or vbInformation, "Sound is playing"
        Exit Sub
    End If
    'Yes we do, do we have effects selected?
    If lstUse.ListCount = 0 Then
        If MsgBox("Do you want to turn off effects for this buffer?", vbYesNo Or vbQuestion, "No effects") = vbYes Then
            'We need to get a DirectSoundSecondaryBuffer from the audio path
            Set dsb = dmp.GetDefaultAudioPath.GetObjectinPath(DMUS_PCHANNEL_ALL, DMUS_PATH_BUFFER, 0, GUID_ALL, 0, IID_DirectSoundSecondaryBuffer)
            'Before we can call SetFX on our Audio Path, we need to deactivate it first
            dmp.GetDefaultAudioPath.Activate False
            'Go ahead and apply our effects
            dsb.SetFX 0, DSEffects, lResults
            'Now we can reactivate our audio path
            dmp.GetDefaultAudioPath.Activate True
            Exit Sub
        Else
            MsgBox "You must first select some effects to use.", vbOKOnly Or vbInformation, "No effects"
            Exit Sub
        End If
    End If
    'Ok, let's apply our effects info here
    
    'First get an array of effects structs the right size
    ReDim DSEffects(lstUse.ListCount - 1)
    ReDim lResults(lstUse.ListCount - 1)
    
    For lCount = 0 To lstUse.ListCount - 1
        Select Case LCase(lstUse.List(lCount))
        Case "distortion"
            DSEffects(lCount).guidDSFXClass = DSFX_STANDARD_DISTORTION
        Case "echo"
            DSEffects(lCount).guidDSFXClass = DSFX_STANDARD_ECHO
        Case "chorus"
            DSEffects(lCount).guidDSFXClass = DSFX_STANDARD_CHORUS
        Case "flanger"
            DSEffects(lCount).guidDSFXClass = DSFX_STANDARD_FLANGER
        Case "compressor"
            DSEffects(lCount).guidDSFXClass = DSFX_STANDARD_COMPRESSOR
        Case "gargle"
            DSEffects(lCount).guidDSFXClass = DSFX_STANDARD_GARGLE
        Case "parameq"
            DSEffects(lCount).guidDSFXClass = DSFX_STANDARD_PARAMEQ
        Case "wavesreverb"
            DSEffects(lCount).guidDSFXClass = DSFX_STANDARD_WAVES_REVERB
        End Select
    Next
    'We need to get a DirectSoundSecondaryBuffer from the audio path
    Set dsb = dmp.GetDefaultAudioPath.GetObjectinPath(DMUS_PCHANNEL_ALL, DMUS_PATH_BUFFER, 0, GUID_ALL, 0, IID_DirectSoundSecondaryBuffer)
    'Before we can call SetFX on our Audio Path, we need to deactivate it first
    dmp.GetDefaultAudioPath.Activate False
    'Go ahead and apply our effects
    dsb.SetFX lstUse.ListCount, DSEffects, lResults
    'Now we can reactivate our audio path
    dmp.GetDefaultAudioPath.Activate True
    Exit Sub
    
NoFX:
    MsgBox "This set of effects could not be set on this audio file.", vbOKOnly Or vbInformation, "Cannot set"
End Sub

Private Sub cmdBrowse_Click()
    Static sCurDir As String
    
    'We want to open a file now
    cdlOpen.flags = cdlOFNHideReadOnly Or cdlOFNFileMustExist
    cdlOpen.Filter = "Wave Files (*.wav)|*.wav|Music Files (*.mid;*.rmi)|*.mid;*.rmi|Segment Files (*.sgt)|*.sgt|All Audio Files|*.wav;*.mid;*.rmi;*.sgt|All Files (*.*)|*.*"
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
            'We are trying to find the windows\media directory.  If it
            'doesn't exist, then use the windows folder as a default
            If Dir$(sMedia, vbDirectory) <> vbNullString Then
                cdlOpen.InitDir = sMedia
            Else
                cdlOpen.InitDir = sWindir
            End If
        End If
    Else
        'No need to move folders.  Stay where they picked the last file
        cdlOpen.InitDir = sCurDir
    End If
    On Local Error GoTo ClickedCancel
    cdlOpen.CancelError = True
    cdlOpen.ShowOpen   ' Display the Open dialog box
    
    'Save the current information
    sCurDir = GetFolder(cdlOpen.FileName)
            
    On Local Error GoTo NoLoadSegment
    'Before we load the buffer stop one if it's playing
    If Not (dmSeg Is Nothing) Then
        dmp.StopEx dmSeg, 0, 0
        dmSeg.Unload dmp.GetDefaultAudioPath
        Set dmSeg = Nothing
    End If
    'Now let's load the segment
    dml.SetSearchDirectory sCurDir
    Set dmSeg = dml.LoadSegment(cdlOpen.FileName)
    If (Right$(cdlOpen.FileName, 4) = ".mid") Or (Right$(cdlOpen.FileName, 4) = ".rmi") Or (Right$(cdlOpen.FileName, 5) = ".midi") Then
        dmSeg.SetStandardMidiFile
    End If
    dmSeg.Download dmp.GetDefaultAudioPath
    txtFile.Text = cdlOpen.FileName
    
    Exit Sub
NoLoadSegment:
    If Err.Number = DSERR_BUFFERTOOSMALL Then 'This buffer isn't big enough to control effects on
        MsgBox "This file isn't long enough to control effects.  Please choose a longer audio file.", vbOKOnly Or vbCritical, "Couldn't load"
    Else 'Some other error
        MsgBox "Couldn't load this file", vbOKOnly Or vbCritical, "Couldn't load"
    End If
    txtFile.Text = "No file loaded..."
ClickedCancel:
End Sub

Private Sub cmdPlay_Click()
    If dmSeg Is Nothing Then
        MsgBox "You must first load a audio file before you can play it.", vbOKOnly Or vbInformation, "No buffer"
        Exit Sub
    End If
    If chkLoop.Value = vbChecked Then
        dmSeg.SetRepeats -1
    Else
        dmSeg.SetRepeats 0
    End If
    Set dmSegState = dmp.PlaySegmentEx(dmSeg, DMUS_SEGF_DEFAULT, 0, , dmp.GetDefaultAudioPath)
End Sub

Private Sub cmdRemove_Click()
    If lstUse.ListIndex = -1 Then 'Nothing is selected
        MsgBox "Please select an effect that's being used before attempting to remove it.", vbOKOnly Or vbInformation, "Nothing selected."
        Exit Sub
    End If
    'Add this item to our list of effects
    lstUse.RemoveItem lstUse.ListIndex
End Sub

Private Sub cmdStop_Click()
    If dmSeg Is Nothing Then
        MsgBox "You must first load an audio file before you can stop it.", vbOKOnly Or vbInformation, "No buffer"
        Exit Sub
    End If
    dmp.StopEx dmSeg, 0, 0
End Sub

Private Sub Form_Load()
    InitAudio
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Cleanup
End Sub

Private Sub InitAudio()
    
    On Local Error Resume Next
    Dim dma As DMUS_AUDIOPARAMS
    
    Set dx = New DirectX8
    'Create our default Performance and Loader objects
    Set dmp = dx.DirectMusicPerformanceCreate
    Set dml = dx.DirectMusicLoaderCreate
    'We want to be able to get a buffer, and control effects.
    dmp.InitAudio Me.hWnd, DMUS_AUDIOF_EAX Or DMUS_AUDIOF_BUFFERS, dma, , DMUS_APATH_DYNAMIC_3D, 128
    'Make sure we did init the audio
    If Err <> 0 Then 'Nope we didn't
        MsgBox "Could not initialize DirectMusic." & vbCrLf & "This sample will exit.", vbOKOnly Or vbInformation, "Exiting..."
        Unload Me
    End If
    
End Sub

Private Sub Cleanup()
    'Let's clean up now
    If Not dmSeg Is Nothing Then
        'If we are playing our file, stop it
        dmp.StopEx dmSeg, 0, 0
        dmSeg.Unload dmp.GetDefaultAudioPath
    End If
    'Destroy our objects
    Set dmSeg = Nothing
    If Not (dmp Is Nothing) Then dmp.CloseDown
    Set dmp = Nothing
    Set dml = Nothing
    Set dx = Nothing
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

Private Sub lstAvail_DblClick()
    'Double clicking should be the same as clicking the 'Add' button
    cmdAdd_Click
End Sub

Private Sub lstUse_DblClick()
    'Double clicking should be the same as clicking the 'Remove' button
    cmdRemove_Click
End Sub

