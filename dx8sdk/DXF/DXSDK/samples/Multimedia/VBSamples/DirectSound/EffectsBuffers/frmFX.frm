VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form frmEffects 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Audio Effects using DirectSound Buffers"
   ClientHeight    =   5790
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   4770
   Icon            =   "frmFX.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   5790
   ScaleWidth      =   4770
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer tmrUpdate 
      Interval        =   50
      Left            =   5760
      Top             =   900
   End
   Begin VB.CheckBox chkLoop 
      Caption         =   "Loop Sound"
      Height          =   315
      Left            =   840
      TabIndex        =   16
      Top             =   5340
      Width           =   1455
   End
   Begin VB.CommandButton cmdStop 
      Caption         =   "&Stop"
      Height          =   375
      Left            =   3600
      TabIndex        =   14
      Top             =   5340
      Width           =   1095
   End
   Begin VB.CommandButton cmdPlay 
      Caption         =   "&Play"
      Height          =   375
      Left            =   2400
      TabIndex        =   13
      Top             =   5340
      Width           =   1095
   End
   Begin VB.Frame fraEffects 
      Caption         =   "Effects Information"
      Height          =   3675
      Left            =   120
      TabIndex        =   2
      Top             =   1560
      Width           =   4515
      Begin VB.CommandButton cmdModify 
         Caption         =   "Modify Selected Effects"
         Enabled         =   0   'False
         Height          =   315
         Left            =   120
         TabIndex        =   17
         Top             =   3240
         Width           =   2235
      End
      Begin VB.TextBox txtFile 
         Height          =   285
         Left            =   120
         Locked          =   -1  'True
         TabIndex        =   9
         Text            =   "No file loaded..."
         Top             =   480
         Width           =   3915
      End
      Begin VB.CommandButton cmdBrowse 
         Caption         =   "..."
         Height          =   285
         Left            =   4020
         TabIndex        =   8
         ToolTipText     =   "Open a new audio file..."
         Top             =   480
         Width           =   315
      End
      Begin VB.ListBox lstAvail 
         Height          =   840
         ItemData        =   "frmFX.frx":0442
         Left            =   120
         List            =   "frmFX.frx":045E
         TabIndex        =   7
         Top             =   1080
         Width           =   4275
      End
      Begin VB.ListBox lstUse 
         Height          =   840
         Left            =   120
         TabIndex        =   6
         Top             =   2280
         Width           =   4275
      End
      Begin VB.CommandButton cmdAdd 
         Height          =   285
         Left            =   2040
         MaskColor       =   &H000000FF&
         Picture         =   "frmFX.frx":04AF
         Style           =   1  'Graphical
         TabIndex        =   5
         Top             =   1980
         UseMaskColor    =   -1  'True
         Width           =   315
      End
      Begin VB.CommandButton cmdRemove 
         Height          =   285
         Left            =   2400
         MaskColor       =   &H000000FF&
         Picture         =   "frmFX.frx":09F1
         Style           =   1  'Graphical
         TabIndex        =   4
         Top             =   1980
         UseMaskColor    =   -1  'True
         Width           =   315
      End
      Begin VB.CommandButton cmdApply 
         Caption         =   "Apply Effects"
         Height          =   315
         Left            =   2460
         TabIndex        =   3
         Top             =   3240
         Width           =   1875
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
         Left            =   120
         TabIndex        =   10
         Top             =   2040
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
      Caption         =   $"frmFX.frx":0F33
      Height          =   675
      Index           =   5
      Left            =   120
      TabIndex        =   15
      Top             =   840
      Width           =   4575
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Audio Effects using DirectSoundBuffers"
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
      Picture         =   "frmFX.frx":0FCA
      Top             =   180
      Width           =   480
   End
   Begin VB.Menu mnuPop 
      Caption         =   "pop"
      Visible         =   0   'False
      Begin VB.Menu mnuRemove 
         Caption         =   "Remove"
      End
      Begin VB.Menu mnuChange 
         Caption         =   "Change Settings..."
      End
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
Private ds As DirectSound8
Private dsb As DirectSoundSecondaryBuffer8
Private mlEffectKey As Long

Private Sub cmdAdd_Click()
    If lstAvail.ListIndex = -1 Then 'Nothing is selected
        MsgBox "Please select an available effect before attempting to add it.", vbOKOnly Or vbInformation, "Nothing selected."
        Exit Sub
    End If
    If Not (dsb Is Nothing) Then
        'Yup, now is there a sound already playing?
        If (dsb.GetStatus And DSBSTATUS_PLAYING) = DSBSTATUS_PLAYING Then
            MsgBox "Stop the currently playing sound before adding any effects.", vbOKOnly Or vbInformation, "Sound is playing"
            Exit Sub
        End If
    End If
    
    If lstUse.ListCount >= mlMaxEffects Then
        MsgBox "You cannot add more than " & CStr(mlMaxEffects) & " effects in this sample.", vbOKOnly Or vbInformation, "No more effects."
        Exit Sub
    End If
    
    'Add this item to our list of effects
    lstUse.AddItem lstAvail.List(lstAvail.ListIndex) & " (Unallocated)"
    RemoveAllForms
End Sub

Private Sub cmdApply_Click()
    ApplySettings
End Sub

Private Sub cmdBrowse_Click()
    Static sCurDir As String
    Dim desc As DSBUFFERDESC
    
    'We want to open a file now
    cdlOpen.flags = cdlOFNHideReadOnly Or cdlOFNFileMustExist
    cdlOpen.Filter = "Wave Files (*.wav)|*.wav"
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
    If Not (dsb Is Nothing) Then If dsb.GetStatus = DSBSTATUS_PLAYING Then dsb.Stop
    'We need to set the CTRLFX flag so we can control the effects on this object
    desc.lFlags = DSBCAPS_CTRLFX
    'Now let's load the segment
    RemoveAllForms
    Set dsb = ds.CreateSoundBufferFromFile(cdlOpen.FileName, desc)
    mlEffectKey = 0
    txtFile.Text = cdlOpen.FileName
    
    Exit Sub
NoLoadSegment:
    If Err.Number = DSERR_BUFFERTOOSMALL Then 'This buffer isn't big enough to control effects on
        MsgBox "This file isn't long enough to control effects.  Please choose a longer audio file.", vbOKOnly Or vbCritical, "Couldn't load"
    Else 'Some other error
        MsgBox "Couldn't load this file", vbOKOnly Or vbCritical, "Couldn't load"
    End If
ClickedCancel:
End Sub

Private Sub cmdModify_Click()
    ChangeSettings
End Sub

Private Sub cmdPlay_Click()
    If dsb Is Nothing Then
        MsgBox "You must first load a wave file into a sound buffer before you can play it.", vbOKOnly Or vbInformation, "No buffer"
        Exit Sub
    End If
    'We need to pre-roll any effects parameter changes that occurred since the last
    'call to an API that does pre-rolling (ie, Stop or SetCurrentPosition)
    dsb.SetCurrentPosition 0
    dsb.Play chkLoop.Value
    EnablePlayUI False
End Sub

Private Sub cmdRemove_Click()
    Dim lLastIndex As Long
    If lstUse.ListIndex = -1 Then 'Nothing is selected
        MsgBox "Please select an effect that's being used before attempting to remove it.", vbOKOnly Or vbInformation, "Nothing selected."
        Exit Sub
    End If
    If Not (dsb Is Nothing) Then
        'Yup, now is there a sound already playing?
        If (dsb.GetStatus And DSBSTATUS_PLAYING) = DSBSTATUS_PLAYING Then
            MsgBox "Stop the currently playing sound before removing any effects.", vbOKOnly Or vbInformation, "Sound is playing"
            Exit Sub
        End If
    End If
    lLastIndex = lstUse.ListIndex
    'Add this item to our list of effects
    lstUse.RemoveItem lstUse.ListIndex
    If (lstUse.ListCount > 0) Then
        If lstUse.ListCount > lLastIndex Then
            lstUse.ListIndex = lLastIndex
        Else
            lstUse.ListIndex = 0
        End If
    End If
    'Enable the menus
    If lstUse.ListCount < 1 Or lstUse.ListIndex < 0 Then
        EnableMenus False
    Else
        EnableMenus True
    End If
    mlEffectKey = 0
    RemoveAllForms
End Sub

Private Sub cmdStop_Click()
    If dsb Is Nothing Then
        MsgBox "You must first load a wave file into a sound buffer before you can stop it.", vbOKOnly Or vbInformation, "No buffer"
        Exit Sub
    End If
    dsb.Stop
    'Stop doesn't reset the current position
    dsb.SetCurrentPosition 0
    EnablePlayUI True
End Sub

Private Sub Form_Load()
    EnablePlayUI True
    InitDSound
End Sub

Private Sub Form_Unload(Cancel As Integer)
    RemoveAllForms
    CleanupDSound
End Sub

Private Sub InitDSound()
    
    On Error GoTo FailedInit
    Set dx = New DirectX8
    'Create our default DirectSound object
    Set ds = dx.DirectSoundCreate(vbNullString)
    ds.SetCooperativeLevel Me.hWnd, DSSCL_NORMAL
    Exit Sub
    
FailedInit:
    MsgBox "Could not initialize DirectSound." & vbCrLf & "This sample will exit.", vbOKOnly Or vbInformation, "Exiting..."
    Unload Me
    
End Sub

Private Sub CleanupDSound()
    'Let's clean up now
    If Not dsb Is Nothing Then
        'If we are playing our file, stop it
        If dsb.GetStatus = DSBSTATUS_PLAYING Then dsb.Stop
        'Destroy our objects
        Set dsb = Nothing
    End If
    Set ds = Nothing
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

Private Sub lstUse_Click()
    'Enable the menu
    If lstUse.ListCount < 1 Or lstUse.ListIndex < 0 Then
        EnableMenus False
    Else
        EnableMenus True
    End If
End Sub

Private Sub lstUse_DblClick()
    'Double clicking should be the same as clicking the 'Remove' button
    cmdRemove_Click
End Sub

Private Sub lstUse_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = vbRightButton Then
        'Show the popup menu
        If lstUse.ListCount < 1 Or lstUse.ListIndex < 0 Then
            EnableMenus False
        Else
            EnableMenus True
        End If
        PopupMenu mnuPop, , X + lstUse.Left, Y + lstUse.Top + fraEffects.Top, mnuRemove
    End If
End Sub

Private Sub EnableMenus(ByVal fEnable As Boolean)
    mnuChange.Enabled = fEnable
    mnuRemove.Enabled = fEnable
    cmdModify.Enabled = fEnable
End Sub

Private Sub mnuChange_Click()
    ChangeSettings
End Sub

Private Sub mnuRemove_Click()
    cmdRemove_Click
End Sub

Private Sub ChangeSettings()
    Dim fGargle As frmGargle, fCompressor As frmCompressor
    Dim fEcho As frmEcho, fDistortion As frmDistortion
    Dim fChorus As frmChorus, fFlanger As frmFlanger
    Dim fParam As frmParamEQ, fWave As frmWaves
    Dim lIndex As Long, lCount As Long
    
    On Error GoTo LeaveSub
    'First we need to force the effects to be applied
    If Not ApplySettings(True) Then Exit Sub
    'No need to continue if the sound is playing or there is no sound buffer
    If dsb Is Nothing Then Exit Sub
    
    'Now we need to get the index of this effect
    lIndex = -1
    For lCount = 0 To lstUse.ListIndex
        If LCase(lstUse.List(lstUse.ListIndex)) = LCase(lstUse.List(lCount)) Then lIndex = lIndex + 1
    Next
    
    'Now show the correct screen based on the info
    Select Case Left$(LCase(lstUse.List(lstUse.ListIndex)), InStr(lstUse.List(lstUse.ListIndex), " ") - 1)
    Case "distortion"
        Set fDistortion = New frmDistortion
        fDistortion.SetBuffer dsb, lIndex
        fDistortion.Show vbModeless, Me
    Case "echo"
        Set fEcho = New frmEcho
        fEcho.SetBuffer dsb, lIndex
        fEcho.Show vbModeless, Me
    Case "chorus"
        Set fChorus = New frmChorus
        fChorus.SetBuffer dsb, lIndex
        fChorus.Show vbModeless, Me
    Case "flanger"
        Set fFlanger = New frmFlanger
        fFlanger.SetBuffer dsb, lIndex
        fFlanger.Show vbModeless, Me
    Case "compressor"
        Set fCompressor = New frmCompressor
        fCompressor.SetBuffer dsb, lIndex
        fCompressor.Show vbModeless, Me
    Case "gargle"
        Set fGargle = New frmGargle
        fGargle.SetBuffer dsb, lIndex
        fGargle.Show vbModeless, Me
    Case "parameq"
        Set fParam = New frmParamEQ
        fParam.SetBuffer dsb, lIndex
        fParam.Show vbModeless, Me
    Case "wavesreverb"
        Set fWave = New frmWaves
        fWave.SetBuffer dsb, lIndex
        fWave.Show vbModeless, Me
    End Select
LeaveSub:
End Sub

Private Sub EnablePlayUI(ByVal fEnable As Boolean)
    On Error Resume Next
    If fEnable Then
        chkLoop.Enabled = True
        cmdPlay.Enabled = True
        cmdStop.Enabled = False
        cmdBrowse.Enabled = True
        cmdPlay.SetFocus
    Else
        chkLoop.Enabled = False
        cmdPlay.Enabled = False
        cmdStop.Enabled = True
        cmdBrowse.Enabled = False
        cmdStop.SetFocus
    End If
End Sub

Private Sub tmrUpdate_Timer()
    If Not (dsb Is Nothing) Then
        If (dsb.GetStatus And DSBSTATUS_PLAYING) <> DSBSTATUS_PLAYING Then
            If cmdPlay.Enabled = False Then
                EnablePlayUI True
            End If
        End If
    End If
End Sub

Private Function ApplySettings(Optional ByVal fIgnoreSoundPlaying As Boolean = False) As Boolean
    On Local Error GoTo NoFX
    Dim DSEffects() As DSEFFECTDESC
    Dim lResults() As Long
    Dim lCount As Long
    Dim lTempEffect As Long
    
    'Do we have a sound buffer
    If dsb Is Nothing Then
        MsgBox "You must first load a wave file into a sound buffer before you can apply effects to it.", vbOKOnly Or vbInformation, "No buffer"
        Exit Function
    End If
    If Not fIgnoreSoundPlaying Then
        'Yup, now is there a sound already playing?
        If (dsb.GetStatus And DSBSTATUS_PLAYING) = DSBSTATUS_PLAYING Then
            MsgBox "Stop the currently playing sound before adding effects.", vbOKOnly Or vbInformation, "Sound is playing"
            Exit Function
        End If
    End If
    'Yes we do, do we have effects selected?
    If lstUse.ListCount = 0 Then
        If MsgBox("Do you want to turn off effects for this buffer?", vbYesNo Or vbQuestion, "No effects") = vbYes Then
            dsb.SetFX 0, DSEffects, lResults
            Exit Function
        Else
            MsgBox "You must first select some effects to use.", vbOKOnly Or vbInformation, "No effects"
            Exit Function
        End If
    End If
    'Ok, let's apply our effects info here
    
    'First get an array of effects structs the right size
    ReDim DSEffects(lstUse.ListCount - 1)
    ReDim lResults(lstUse.ListCount - 1)
    
    'Now we don't want to apply the effects if they've already been applied.  So,
    'through our list, and create a 'unique' number to describe this set of effects
    'and only apply them if the number is different form our stored one.
    For lCount = 0 To lstUse.ListCount - 1
        Select Case Left$(LCase(lstUse.List(lCount)), InStr(lstUse.List(lCount), " ") - 1)
        Case "distortion"
            lTempEffect = lTempEffect + (lCount + &H10)
            DSEffects(lCount).guidDSFXClass = DSFX_STANDARD_DISTORTION
        Case "echo"
            lTempEffect = lTempEffect + (lCount + &H20)
            DSEffects(lCount).guidDSFXClass = DSFX_STANDARD_ECHO
        Case "chorus"
            lTempEffect = lTempEffect + (lCount + &H40)
            DSEffects(lCount).guidDSFXClass = DSFX_STANDARD_CHORUS
        Case "flanger"
            lTempEffect = lTempEffect + (lCount + &H80)
            DSEffects(lCount).guidDSFXClass = DSFX_STANDARD_FLANGER
        Case "compressor"
            lTempEffect = lTempEffect + (lCount + &H100)
            DSEffects(lCount).guidDSFXClass = DSFX_STANDARD_COMPRESSOR
        Case "gargle"
            lTempEffect = lTempEffect + (lCount + &H200)
            DSEffects(lCount).guidDSFXClass = DSFX_STANDARD_GARGLE
        Case "parameq"
            lTempEffect = lTempEffect + (lCount + &H400)
            DSEffects(lCount).guidDSFXClass = DSFX_STANDARD_PARAMEQ
        Case "wavesreverb"
            lTempEffect = lTempEffect + (lCount + &H800)
            DSEffects(lCount).guidDSFXClass = DSFX_STANDARD_WAVES_REVERB
        End Select
    Next
    If mlEffectKey <> lTempEffect Then 'They don't match, set the fx.
        dsb.SetFX lstUse.ListCount, DSEffects, lResults
        Dim sNewItem As String
        For lCount = 0 To lstUse.ListCount - 1
            sNewItem = Left$(lstUse.List(lCount), InStr(lstUse.List(lCount), " ") - 1)
            Select Case lResults(lCount)
            Case DSFXR_FAILED
                lstUse.List(lCount) = sNewItem & " - Failed"
            Case DSFXR_LOCHARDWARE
                lstUse.List(lCount) = sNewItem & " - Hardware"
            Case DSFXR_LOCSOFTWARE
                lstUse.List(lCount) = sNewItem & " - Software"
            Case DSFXR_UNALLOCATED
                lstUse.List(lCount) = sNewItem & " - Unallocated"
            Case DSFXR_UNKNOWN
                lstUse.List(lCount) = sNewItem & " - Unknown"
            Case DSFXR_PRESENT
                lstUse.List(lCount) = sNewItem & " - Present"
            End Select
        Next
    End If
    mlEffectKey = lTempEffect
    ApplySettings = True
    Exit Function
    
NoFX:
    MsgBox "This set of effects could not be set on this audio file.", vbOKOnly Or vbInformation, "Cannot set"
    ApplySettings = False
End Function

Private Sub RemoveAllForms()
    'Get rid of all forms
    Dim f As Form
    For Each f In Forms
        If Not (f Is Me) Then Unload f
    Next
End Sub
