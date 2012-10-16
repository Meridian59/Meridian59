VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form DS3DPositionForm 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "DS 3D Positioning"
   ClientHeight    =   5565
   ClientLeft      =   930
   ClientTop       =   330
   ClientWidth     =   5055
   Icon            =   "Sound3D.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   5565
   ScaleWidth      =   5055
   Begin VB.Timer tmrUpdate 
      Interval        =   50
      Left            =   4260
      Top             =   2100
   End
   Begin MSComDlg.CommonDialog cdlFile 
      Left            =   3780
      Top             =   2040
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.PictureBox picDraw 
      BackColor       =   &H00FFFFFF&
      FillStyle       =   7  'Diagonal Cross
      Height          =   2775
      Left            =   120
      ScaleHeight     =   181
      ScaleMode       =   3  'Pixel
      ScaleWidth      =   317
      TabIndex        =   7
      TabStop         =   0   'False
      Top             =   2640
      Width           =   4815
   End
   Begin VB.PictureBox picContainer 
      Height          =   1755
      Index           =   0
      Left            =   120
      ScaleHeight     =   1695
      ScaleWidth      =   4755
      TabIndex        =   10
      TabStop         =   0   'False
      Top             =   120
      Width           =   4815
      Begin VB.TextBox txtSound 
         BackColor       =   &H8000000F&
         Height          =   315
         Left            =   960
         Locked          =   -1  'True
         TabIndex        =   13
         Top             =   120
         Width           =   3735
      End
      Begin VB.CommandButton cmdSound 
         Caption         =   "Sound..."
         Enabled         =   0   'False
         Height          =   315
         Left            =   60
         TabIndex        =   0
         Top             =   120
         Width           =   855
      End
      Begin VB.CommandButton cmdPlay 
         Caption         =   "Play"
         Height          =   375
         Left            =   120
         TabIndex        =   3
         Top             =   1200
         Width           =   855
      End
      Begin VB.CommandButton cmdPause 
         Caption         =   "Pause"
         Height          =   375
         Left            =   1020
         TabIndex        =   4
         Top             =   1200
         Width           =   855
      End
      Begin VB.CommandButton cmdStop 
         Caption         =   "Stop"
         Height          =   375
         Left            =   1920
         TabIndex        =   5
         Top             =   1200
         Width           =   735
      End
      Begin VB.CheckBox chLoop 
         Caption         =   "Loop Play"
         Height          =   315
         Left            =   2760
         TabIndex        =   6
         Top             =   1260
         Width           =   1455
      End
      Begin VB.HScrollBar scrlVol 
         Height          =   255
         LargeChange     =   20
         Left            =   840
         Max             =   0
         Min             =   -3000
         SmallChange     =   500
         TabIndex        =   1
         Top             =   540
         Width           =   3855
      End
      Begin VB.HScrollBar scrlAngle 
         Height          =   255
         LargeChange     =   20
         Left            =   840
         Max             =   360
         Min             =   -360
         SmallChange     =   10
         TabIndex        =   2
         Top             =   840
         Value           =   -90
         Width           =   3855
      End
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   "Volume"
         Height          =   255
         Index           =   0
         Left            =   120
         TabIndex        =   12
         Top             =   600
         Width           =   1095
      End
      Begin VB.Label Label2 
         BackStyle       =   0  'Transparent
         Caption         =   "Direction"
         Height          =   255
         Index           =   0
         Left            =   120
         TabIndex        =   11
         Top             =   900
         Width           =   975
      End
   End
   Begin VB.Label Label5 
      BackStyle       =   0  'Transparent
      Caption         =   "Click and drag the red triangle around with the left mouse button to change the sound position."
      Height          =   495
      Left            =   120
      TabIndex        =   9
      Top             =   2160
      Width           =   4755
   End
   Begin VB.Label Label4 
      BackStyle       =   0  'Transparent
      Caption         =   "Sound Positions"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   375
      Left            =   120
      TabIndex        =   8
      Top             =   1920
      Width           =   1575
   End
End
Attribute VB_Name = "DS3DPositionForm"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       Sound3d.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

'API declare for windows folder
Private Declare Function GetWindowsDirectory Lib "kernel32" Alias "GetWindowsDirectoryA" (ByVal lpBuffer As String, ByVal nSize As Long) As Long

Dim dx As New DirectX8 'Our DirectX object
Dim ds As DirectSound8 'Our DirectSound object
Dim dsBuffer As DirectSoundSecondaryBuffer8 'Our SoundBuffer
Dim ds3dBuffer As DirectSound3DBuffer8 'We need to get a 3DSoundBuffer
Dim oPos As D3DVECTOR 'Position
Dim fMouseDown As Boolean 'Is the mouse down?

Private Sub cmdSound_Click()

    Static sCurDir As String
    Static lFilter As Long
    Dim dsBuf As DSBUFFERDESC
    
    'Now we should load a wave file
    'Ask them for a file to load
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
            Exit Sub 'We didn't click anything exit
        End If
        'Save the current information
        sCurDir = GetFolder(.FileName)
        lFilter = .FilterIndex
        
        'Save the filename for later use
        If Not (dsBuffer Is Nothing) Then dsBuffer.Stop
        Set dsBuffer = Nothing
        txtSound.Text = vbNullString
        dsBuf.lFlags = DSBCAPS_CTRL3D Or DSBCAPS_CTRLVOLUME
        'Before we load the 3D dialog check to see if this is a mono file
        On Error Resume Next
        Set dsBuffer = ds.CreateSoundBufferFromFile(.FileName, dsBuf)
        If Err Then
            'First check to see if this is a stereo wav file
            If (dsBuf.fxFormat.nChannels > 1) And (Err.Number = 5) Then 'Yup
                MsgBox "You must load a mono wave file to control 3D sound.  Stereo wave files are not supported.", vbOKOnly Or vbInformation, "Couldn't load"
            Else
                MsgBox "Could not load this wave file." & vbCrLf & "Format is not supported.", vbOKOnly Or vbInformation, "Couldn't load"
            End If
            Exit Sub
        End If
        
        'Now we need to get the 3D virtualization params
        Dim f3DParams As New frm3DAlg
        
        f3DParams.Show vbModal, Me
        If f3DParams.OKHit Then
            If f3DParams.optFull Then dsBuf.guid3DAlgorithm = GUID_DS3DALG_HRTF_FULL
            If f3DParams.optHalf Then dsBuf.guid3DAlgorithm = GUID_DS3DALG_HRTF_LIGHT
            If f3DParams.optNone Then dsBuf.guid3DAlgorithm = GUID_DS3DALG_NO_VIRTUALIZATION
        Else
            Set dsBuffer = Nothing
            Exit Sub
        End If
        On Error Resume Next
        Set dsBuffer = ds.CreateSoundBufferFromFile(.FileName, dsBuf)
        If Err Then
            MsgBox "Could not create the sound buffer.", vbOKOnly Or vbInformation, "Couldn't load"
            Exit Sub
        End If
        txtSound.Text = .FileName
        EnablePlayUI True
        Set ds3dBuffer = dsBuffer.GetDirectSound3DBuffer
        ds3dBuffer.SetConeAngles DS3D_MINCONEANGLE, 100, DS3D_IMMEDIATE
        ds3dBuffer.SetConeOutsideVolume -400, DS3D_IMMEDIATE
        ' position our sound
        ds3dBuffer.SetPosition oPos.x / 50, 0, oPos.z / 50, DS3D_IMMEDIATE
        'Update the volume
        scrlVol_Change
    End With
    
End Sub

Private Sub Form_Load()
    
    
    On Local Error Resume Next
    Set ds = dx.DirectSoundCreate(vbNullString) 'Create a default DirectSound object
    'We couldn't create the DSound object.  End the app now
    If Err.Number <> 0 Then
        MsgBox "Could not initialize DirectSound." & vbCrLf & "This sample will exit.", vbOKOnly Or vbInformation, "Exiting..."
        Unload Me
        End
    End If
    'Set the coop level
    ds.SetCooperativeLevel Me.hWnd, DSSCL_PRIORITY
    
    'Show the form
    Me.Show
    oPos.x = 0: oPos.z = 5
    '- Make sure we pickup the correct volume and orientation
    scrlAngle_Change
    scrlVol_Change
    
    DrawPositions
    EnablePlayUI True
    cmdPlay.Enabled = False
    cmdSound.SetFocus
End Sub

Private Sub cmdPlay_Click()
    If dsBuffer Is Nothing Then Exit Sub
           
    'Play plays the sound from the current position
    'if the sound was paused using the stop command
    'then play will begin where it last left off
    dsBuffer.Play chLoop.Value 'Checked = 1 (looping), Unchecked = 0 (Default)
    EnablePlayUI False
End Sub

Private Sub cmdStop_Click()
    If dsBuffer Is Nothing Then Exit Sub
    
    dsBuffer.Stop
    dsBuffer.SetCurrentPosition 0 'Reset the position since Stop doesn't
    EnablePlayUI True
End Sub

Private Sub cmdPause_Click()
    If dsBuffer Is Nothing Then Exit Sub
    dsBuffer.Stop 'Stop doesn't reset the position
End Sub

'They've changed the volume.  Update it
Private Sub scrlVol_Change()
    If dsBuffer Is Nothing Then Exit Sub
    dsBuffer.SetVolume scrlVol.Value
End Sub

Private Sub scrlVol_Scroll()
    scrlVol_Change
End Sub

'They've changed the angle.  Update it
Private Sub scrlAngle_Change()
    
    'We need to calculate a vector of what direction the sound is traveling in.
    Dim x As Single
    Dim z As Single
    'we take the current angle in degrees convert to radians
    'and get the cos or sin to find the direction from an angle
    x = 5 * Cos(3.141 * scrlAngle.Value / 180)
    z = 5 * Sin(3.141 * scrlAngle.Value / 180)
    
    'Update the UI
    DrawPositions
    If dsBuffer Is Nothing Then Exit Sub
    ds3dBuffer.SetConeOrientation x, 0, z, DS3D_IMMEDIATE
    
End Sub

Private Sub scrlAngle_Scroll()
    scrlAngle_Change
End Sub

Sub UpdatePosition(x As Single, z As Single)
    On Error Resume Next
    oPos.x = x - picDraw.ScaleWidth / 2
    oPos.z = z - picDraw.ScaleHeight / 2
    
    DrawPositions
    
    'the zero at the end indicates we want the postion updated immediately
    If ds3dBuffer Is Nothing Then Exit Sub
    
    ds3dBuffer.SetPosition oPos.x / 50, 0, oPos.z / 50, DS3D_IMMEDIATE
    
End Sub

Private Sub picDraw_MouseDown(Button As Integer, Shift As Integer, x As Single, z As Single)
    On Error Resume Next
    If Button = vbLeftButton Then
        UpdatePosition x, z
        fMouseDown = True
    End If
End Sub

Private Sub picDraw_MouseMove(Button As Integer, Shift As Integer, x As Single, z As Single)
    On Error Resume Next
    If Not fMouseDown Then Exit Sub
    If Button = vbLeftButton Then
        'Only update the position if it is outside of the box
        If x < 0 Or z < 0 Or x > picDraw.ScaleWidth Or z > picDraw.ScaleHeight Then Exit Sub
        UpdatePosition x, z
    End If
End Sub

Private Sub picDraw_MouseUp(Button As Integer, Shift As Integer, x As Single, Y As Single)
    On Error Resume Next
    fMouseDown = False
End Sub

Private Sub picDraw_Paint()
    DrawPositions
End Sub

Sub DrawPositions()
    Dim x As Integer
    Dim z As Integer
    
    picDraw.Cls
    
    'listener is in center and is black
    DrawTriangle 0, picDraw.ScaleWidth / 2, picDraw.ScaleHeight / 2, 90
    
    'draw sound as RED
    x = CInt(oPos.x) + picDraw.ScaleWidth / 2
    z = CInt(oPos.z) + picDraw.ScaleHeight / 2
    DrawTriangle RGB(256, 0, 0), x, z, scrlAngle.Value
    
End Sub

'Draw a triangle representing where we are
Sub DrawTriangle(col As Long, x As Integer, z As Integer, ByVal a As Single)
    
    Dim x1 As Integer
    Dim z1 As Integer
    Dim x2 As Integer
    Dim z2 As Integer
    Dim x3 As Integer
    Dim z3 As Integer
    
    a = 3.141 * (a - 90) / 180
    Dim q As Integer
    q = 10
    
    x1 = q * Sin(a) + x
    z1 = q * Cos(a) + z
    
    x2 = q * Sin(a + 3.141 / 1.3) + x
    z2 = q * Cos(a + 3.141 / 1.3) + z
    
    x3 = q * Sin(a - 3.141 / 1.3) + x
    z3 = q * Cos(a - 3.141 / 1.3) + z
    
    picDraw.Line (x1, z1)-(x2, z2), col
    picDraw.Line (x1, z1)-(x3, z3), col
    picDraw.Line (x2, z2)-(x3, z3), col
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

Private Sub EnablePlayUI(ByVal fEnable As Boolean)
    On Error Resume Next
    If fEnable Then
        chLoop.Enabled = True
        cmdPlay.Enabled = True
        cmdPause.Enabled = False
        cmdStop.Enabled = False
        cmdSound.Enabled = True
        cmdPlay.SetFocus
    Else
        chLoop.Enabled = False
        cmdPlay.Enabled = False
        cmdStop.Enabled = True
        cmdPause.Enabled = True
        cmdSound.Enabled = False
        cmdStop.SetFocus
    End If
End Sub

Private Sub tmrUpdate_Timer()
    If Not (dsBuffer Is Nothing) Then
        If (dsBuffer.GetStatus And DSBSTATUS_PLAYING) <> DSBSTATUS_PLAYING Then
            If cmdPlay.Enabled = False Then
                EnablePlayUI True
            End If
        End If
    End If
End Sub
