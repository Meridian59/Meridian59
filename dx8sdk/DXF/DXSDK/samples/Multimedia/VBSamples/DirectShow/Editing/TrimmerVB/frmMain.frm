VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "COMDLG32.OCX"
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmMain 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "TrimmerVB"
   ClientHeight    =   8775
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   10890
   Icon            =   "frmMain.frx":0000
   LinkTopic       =   "frmMain"
   MaxButton       =   0   'False
   ScaleHeight     =   8775
   ScaleWidth      =   10890
   StartUpPosition =   2  'CenterScreen
   Begin VB.TextBox txtInstruction 
      Appearance      =   0  'Flat
      BackColor       =   &H8000000F&
      BorderStyle     =   0  'None
      Height          =   1365
      HideSelection   =   0   'False
      Left            =   7425
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      TabIndex        =   40
      TabStop         =   0   'False
      Text            =   "frmMain.frx":030A
      Top             =   4875
      Width           =   3315
   End
   Begin VB.Frame fraPretty 
      Enabled         =   0   'False
      Height          =   4815
      Left            =   7305
      TabIndex        =   38
      Top             =   3525
      Width           =   3540
      Begin VB.Timer tmrTimer 
         Interval        =   1000
         Left            =   600
         Top             =   4330
      End
      Begin VB.TextBox txtCopyright 
         Appearance      =   0  'Flat
         BackColor       =   &H8000000F&
         BorderStyle     =   0  'None
         Height          =   840
         Left            =   975
         Locked          =   -1  'True
         MultiLine       =   -1  'True
         TabIndex        =   39
         TabStop         =   0   'False
         Text            =   "frmMain.frx":03D5
         Top             =   225
         Width           =   2490
      End
      Begin MSComDlg.CommonDialog ctrlCommonDialog 
         Left            =   75
         Top             =   4275
         _ExtentX        =   847
         _ExtentY        =   847
         _Version        =   393216
      End
      Begin VB.Line lnAboutSeptum 
         X1              =   150
         X2              =   3375
         Y1              =   1125
         Y2              =   1125
      End
      Begin VB.Image imgAbout 
         Height          =   765
         Left            =   150
         Picture         =   "frmMain.frx":0454
         Stretch         =   -1  'True
         Top             =   225
         Width           =   765
      End
   End
   Begin MSComctlLib.StatusBar ctrlStatusBar 
      Align           =   2  'Align Bottom
      Height          =   390
      Left            =   0
      TabIndex        =   30
      Top             =   8385
      Width           =   10890
      _ExtentX        =   19209
      _ExtentY        =   688
      Style           =   1
      _Version        =   393216
      BeginProperty Panels {8E3867A5-8586-11D1-B16A-00C0F0283628} 
      EndProperty
   End
   Begin VB.Frame fraPreviewControl 
      Caption         =   "Video Preview:"
      Height          =   2340
      Left            =   45
      TabIndex        =   25
      Top             =   6000
      Width           =   7190
      Begin VB.CommandButton cmdSetStop 
         Caption         =   "Set Trim Preview Stop Position"
         Height          =   375
         Left            =   150
         TabIndex        =   9
         ToolTipText     =   "Selects the ending point of the trim operation."
         Top             =   1755
         Width           =   3000
      End
      Begin VB.CommandButton cmdSelStart 
         Caption         =   "Set Trim Preview Start Position"
         Height          =   375
         Left            =   150
         TabIndex        =   8
         ToolTipText     =   "Selects the starting point of the trim operation."
         Top             =   1275
         Width           =   3000
      End
      Begin VB.Frame fraVideoPreview 
         Height          =   940
         Left            =   3300
         TabIndex        =   31
         Top             =   1200
         Width           =   3765
         Begin VB.Label lblFPS 
            Caption         =   "FPS:"
            Height          =   255
            Left            =   150
            TabIndex        =   37
            Top             =   300
            Width           =   975
         End
         Begin VB.Label lblFPSValue 
            Caption         =   "0"
            Height          =   255
            Left            =   1230
            TabIndex        =   36
            Top             =   300
            Width           =   1005
         End
         Begin VB.Label lblStreams 
            Caption         =   "Streams:"
            Height          =   255
            Left            =   150
            TabIndex        =   35
            Top             =   540
            Width           =   975
         End
         Begin VB.Label lblStreamsValue 
            Caption         =   "0"
            Height          =   255
            Left            =   1230
            TabIndex        =   34
            Top             =   540
            Width           =   1005
         End
         Begin VB.Label lblVideoStream 
            Caption         =   "Video Stream:"
            Height          =   255
            Left            =   2250
            TabIndex        =   33
            Top             =   300
            Width           =   1005
         End
         Begin VB.Label lblVideoStreamValue 
            Caption         =   "0"
            Height          =   255
            Left            =   3330
            TabIndex        =   32
            Top             =   300
            Width           =   255
         End
      End
      Begin VB.CommandButton cmdEnd 
         Caption         =   "&End"
         Height          =   375
         Left            =   2190
         TabIndex        =   6
         ToolTipText     =   "Move to the last frame."
         Top             =   300
         Width           =   975
      End
      Begin VB.CommandButton cmdHome 
         Caption         =   "&Home"
         Height          =   375
         Left            =   150
         TabIndex        =   3
         ToolTipText     =   "Move to the first frame."
         Top             =   300
         Width           =   975
      End
      Begin VB.CommandButton cmdFwdFrame 
         Caption         =   ">"
         Height          =   375
         Left            =   1710
         TabIndex        =   5
         ToolTipText     =   "Move Forward one frame."
         Top             =   300
         Width           =   375
      End
      Begin VB.CommandButton cmdBackFrame 
         Caption         =   "<"
         Height          =   375
         Left            =   1230
         TabIndex        =   4
         ToolTipText     =   "Move backward one frame."
         Top             =   300
         Width           =   375
      End
      Begin MSComctlLib.Slider ctrlSlider 
         Height          =   375
         Left            =   30
         TabIndex        =   7
         ToolTipText     =   "Highlighted portion of the timeline represents the selected video which will be 'Trimmed' from the source clip"
         Top             =   825
         Width           =   7130
         _ExtentX        =   12568
         _ExtentY        =   661
         _Version        =   393216
         Max             =   50
         SelectRange     =   -1  'True
         TextPosition    =   1
      End
      Begin VB.Label lblCurrentTimeValue 
         Caption         =   "0"
         Height          =   255
         Left            =   5070
         TabIndex        =   29
         Top             =   540
         Width           =   1680
      End
      Begin VB.Label lblCurrentTime 
         Caption         =   "Current Time:"
         Height          =   255
         Left            =   3750
         TabIndex        =   28
         Top             =   540
         Width           =   1125
      End
      Begin VB.Label lblCurrentFrameValue 
         Caption         =   "0"
         Height          =   255
         Left            =   5070
         TabIndex        =   27
         Top             =   300
         Width           =   1680
      End
      Begin VB.Label lblCurrentFrame 
         Caption         =   "Current Frame:"
         Height          =   255
         Left            =   3750
         TabIndex        =   26
         Top             =   300
         Width           =   1140
      End
   End
   Begin VB.Frame fraVideoControl 
      Caption         =   "Video Control:"
      Height          =   2415
      Left            =   45
      TabIndex        =   14
      Top             =   3525
      Width           =   7190
      Begin VB.CommandButton cmdPlayback 
         Caption         =   "&Playback"
         Height          =   375
         Left            =   150
         TabIndex        =   2
         ToolTipText     =   "Plays back the video using Media Player"
         Top             =   1875
         Width           =   975
      End
      Begin VB.CommandButton cmdBrowse 
         Caption         =   "&Browse..."
         Height          =   375
         Left            =   150
         TabIndex        =   0
         ToolTipText     =   "Browse for source media."
         Top             =   900
         Width           =   975
      End
      Begin VB.CommandButton cmdWrite 
         Caption         =   "&Write"
         Height          =   375
         Left            =   150
         TabIndex        =   1
         ToolTipText     =   "Exports the trimmed video to an avi file."
         Top             =   1380
         Width           =   975
      End
      Begin MSComctlLib.ProgressBar ctrlProgress 
         Height          =   405
         Left            =   1230
         TabIndex        =   42
         Top             =   1350
         Visible         =   0   'False
         Width           =   5805
         _ExtentX        =   10239
         _ExtentY        =   714
         _Version        =   393216
         Appearance      =   1
      End
      Begin VB.Label lblPlaybackFileName 
         BorderStyle     =   1  'Fixed Single
         Caption         =   "c:\smart.avi"
         Height          =   375
         Left            =   1230
         TabIndex        =   41
         Top             =   1875
         Width           =   5805
      End
      Begin VB.Label lblReadFileName 
         BorderStyle     =   1  'Fixed Single
         Height          =   375
         Left            =   1230
         TabIndex        =   24
         Top             =   900
         Width           =   5805
      End
      Begin VB.Label lblWriteFileName 
         BorderStyle     =   1  'Fixed Single
         Caption         =   "c:\smart.avi"
         Height          =   375
         Left            =   1230
         TabIndex        =   23
         Top             =   1380
         Width           =   5805
      End
      Begin VB.Label lblStartFrame 
         Caption         =   "Start Frame:"
         Height          =   255
         Left            =   150
         TabIndex        =   22
         Top             =   300
         Width           =   1095
      End
      Begin VB.Label lblStartFrameValue 
         Caption         =   "0"
         Height          =   255
         Left            =   1350
         TabIndex        =   21
         Top             =   300
         Width           =   1680
      End
      Begin VB.Label lblStopFrame 
         Caption         =   "Stop Frame:"
         Height          =   255
         Left            =   3270
         TabIndex        =   20
         Top             =   300
         Width           =   1095
      End
      Begin VB.Label lblStopFrameValue 
         Caption         =   "0"
         Height          =   255
         Left            =   4380
         TabIndex        =   19
         Top             =   300
         Width           =   1680
      End
      Begin VB.Label lblStartTime 
         Caption         =   "Start Time:"
         Height          =   255
         Left            =   150
         TabIndex        =   18
         Top             =   540
         Width           =   1095
      End
      Begin VB.Label lblStartTimeValue 
         Caption         =   "0"
         Height          =   255
         Left            =   1350
         TabIndex        =   17
         Top             =   540
         Width           =   1680
      End
      Begin VB.Label lblStopTime 
         Caption         =   "Stop Time:"
         Height          =   255
         Left            =   3270
         TabIndex        =   16
         Top             =   540
         Width           =   1095
      End
      Begin VB.Label lblStopTimeValue 
         Caption         =   "0"
         Height          =   255
         Left            =   4380
         TabIndex        =   15
         Top             =   540
         Width           =   1680
      End
   End
   Begin VB.PictureBox picPreview 
      Height          =   3225
      Left            =   45
      ScaleHeight     =   3165
      ScaleWidth      =   3480
      TabIndex        =   10
      Top             =   270
      Width           =   3540
   End
   Begin VB.Label lblVideoStopFrame 
      Caption         =   "Video Stop Frame:"
      Height          =   240
      Left            =   7305
      TabIndex        =   13
      Top             =   0
      Width           =   3480
   End
   Begin VB.Label lblVideoStartFrame 
      Caption         =   "Video Start Frame:"
      Height          =   240
      Left            =   3660
      TabIndex        =   12
      Top             =   0
      Width           =   3555
   End
   Begin VB.Label lblVideoCurrentFrame 
      Caption         =   "Current Video Frame:"
      Height          =   240
      Left            =   45
      TabIndex        =   11
      Top             =   0
      Width           =   1515
   End
   Begin VB.Image imgPreviewStop 
      BorderStyle     =   1  'Fixed Single
      Height          =   3225
      Left            =   7305
      Stretch         =   -1  'True
      Top             =   270
      Width           =   3540
   End
   Begin VB.Image imgPreviewStart 
      BorderStyle     =   1  'Fixed Single
      Height          =   3225
      Left            =   3675
      Stretch         =   -1  'True
      Top             =   270
      Width           =   3540
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'*******************************************************************************
'*       This is a part of the Microsoft DXSDK Code Samples.
'*       Copyright (C) 1999-2001 Microsoft Corporation.
'*       All rights reserved.
'*       This source code is only intended as a supplement to
'*       Microsoft Development Tools and/or SDK documentation.
'*       See these sources for detailed information regarding the
'*       Microsoft samples programs.
'*******************************************************************************
Option Explicit
Option Base 0
Option Compare Text

Private m_dblFPS As Double                         'evaluates to the rate of the currently loaded clip (frames per second)
Private m_boolDirty As Boolean                    'evaluates to true if the UI needs repainted, and the poster frame needs regrabbed
Private m_nFrameCount As Long                   'evaluates to the number of frames in the current clip
Private m_bstrFileName As String                 'evaluates to the filename of the currently loaded clip
Private m_boolLoaded As Boolean                'evaluates to true if we have anything loaded
Private m_boolHasAudio As Boolean            'evaluates to true if the current clip has audio
Private m_objMediaDet As MediaDet            'evaluates to a media detector object which is used to work with stream information

Private Const VIDEO_CLSID As String = "{73646976-0000-0010-8000-00AA00389B71}"  'video clsid
Private Const AUDIO_CLSID As String = "{73647561-0000-0010-8000-00AA00389B71}"  'audio clsid
Private Const POSTER_FRAME_FILENAME As String = "bitmap.bmp"   ' filename to write out poster frames for loading into the UI
Private Const MPLAYER2_INSTALL_LOCATION As String = "c:\program files\windows media player\mplayer2.exe"  'mplayer2.exe


' **************************************************************************************************************************************
' * PRIVATE INTERFACE- FORM EVENTS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: Form_Load
            ' * procedure description:  Occurs when a form is loaded.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_Load()
            On Local Error GoTo ErrLine
            
            'disable ui
            ctrlSlider.Enabled = False
            cmdHome.Enabled = False
            cmdEnd.Enabled = False
            cmdBrowse.Enabled = True
            cmdWrite.Enabled = False
            cmdSelStart.Enabled = False
            cmdSetStop.Enabled = False
            cmdBackFrame.Enabled = False
            cmdFwdFrame.Enabled = False
            cmdPlayback.Enabled = False
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            

            ' ******************************************************************************************************************************
            ' * procedure name: Form_Load
            ' * procedure description:  Occurs when a form is loaded.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_Unload(Cancel As Integer)
            On Local Error GoTo ErrLine
            
            'ensure the temporary file has been deleted
            If File_Exists(GetTempDirectory & POSTER_FRAME_FILENAME) Then _
               Call File_Delete(GetTempDirectory & POSTER_FRAME_FILENAME, False, False, False)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Form_Initialize
            ' * procedure description:  Occurs when an application creates an instance of a Form, MDIForm, or class.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_Initialize()
            On Local Error GoTo ErrLine
            
            'initalize module-level variable(s)
            Set m_objMediaDet = New MediaDet
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Form_Terminate
            ' * procedure description:  Occurs when all references to an instance of a Form, MDIForm, or class are removed from memory.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_Terminate()
            On Local Error GoTo ErrLine
            
            'terminate module-level object(s0
            If Not m_objMediaDet Is Nothing Then Set m_objMediaDet = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
' **************************************************************************************************************************************
' * PRIVATE INTERFACE- CONTROL EVENTS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: cmdPlayback_Click
            ' * procedure description:  Occurs when the user presses and then releases a mouse button over an object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdPlayback_Click()
            Dim nResultant As Long
            Dim bstrFileName As String
            Dim bstrDirectoryName As String
            On Local Error GoTo ErrLine
            
            'verify that the export location is valid
            If File_Exists(lblPlaybackFileName.Caption) Then
               'obtain the filename & directory name from the label
               If InStr(1, lblPlaybackFileName.Caption, "\") > 0 Then
                  bstrFileName = Right(lblPlaybackFileName.Caption, Len(lblPlaybackFileName.Caption) - InStrRev(lblPlaybackFileName.Caption, "\"))
                  bstrDirectoryName = Replace(lblPlaybackFileName.Caption, bstrFileName, vbNullString)
                  If Right(bstrDirectoryName, 1) = "\" Then bstrDirectoryName = Left(bstrDirectoryName, Len(bstrDirectoryName) - 1)
               ElseIf InStr(1, lblPlaybackFileName.Caption, "/") > 0 Then
                  bstrFileName = Right(lblPlaybackFileName.Caption, Len(lblPlaybackFileName.Caption) - InStrRev(lblPlaybackFileName.Caption, "/"))
                  bstrDirectoryName = Replace(lblPlaybackFileName.Caption, bstrFileName, vbNullString)
                  If Right(bstrDirectoryName, 1) = "/" Then bstrDirectoryName = Left(bstrDirectoryName, Len(bstrDirectoryName) - 1)
               End If
               nResultant = File_Execute(bstrDirectoryName, bstrFileName)
            End If
            
            'verify the operation succeeded,
            'if it did not then dislay an error dialog
            If nResultant = 0 Then
               MsgBox "The file could not be found on the specified path: " & _
                             CStr(lblPlaybackFileName.Caption), vbExclamation + vbApplicationModal
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub



            ' ******************************************************************************************************************************
            ' * procedure name: cmdBackFrame_Click
            ' * procedure description:  Occurs when the user presses and then releases a mouse button over an object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdBackFrame_Click()
            Dim v As Long
            On Local Error GoTo ErrLine
            
            v = CLng(ctrlSlider.Value)
            v = (v - 1): If v < 0 Then v = 0
            ctrlSlider.Value = v: m_boolDirty = True 'reset to dirty
            lblCurrentFrameValue.Caption = CStr(Trim(Str(ctrlSlider.Value)))
            If m_dblFPS <> 0 Then lblCurrentTimeValue.Caption = CStr(Trim(Str(ctrlSlider.Value / m_dblFPS)))
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdFwdFrame_Click
            ' * procedure description:  Occurs when the user presses and then releases a mouse button over an object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdFwdFrame_Click()
            Dim v As Long
            On Local Error GoTo ErrLine
            
            v = CLng(ctrlSlider.Value): v = (v + 1)
            If v > m_nFrameCount Then v = m_nFrameCount
            ctrlSlider.Value = v: m_boolDirty = True 'reset to dirty
            lblCurrentFrameValue.Caption = CStr(Trim(Str(ctrlSlider.Value)))
            If m_dblFPS <> 0 Then lblCurrentTimeValue.Caption = CStr(Trim(Str(ctrlSlider.Value / m_dblFPS)))
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdEnd_Click
            ' * procedure description:  Occurs when the user presses and then releases a mouse button over an object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdEnd_Click()
            On Local Error GoTo ErrLine
            
            ctrlSlider.Value = m_nFrameCount: m_boolDirty = True 'reset to dirty
            lblCurrentFrameValue.Caption = CStr(Trim(Str(ctrlSlider.Value)))
            If m_dblFPS <> 0 Then lblCurrentTimeValue.Caption = CStr(Trim(Str(ctrlSlider.Value / m_dblFPS)))
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdHome_Click
            ' * procedure description:  Occurs when the user presses and then releases a mouse button over an object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdHome_Click()
            On Local Error GoTo ErrLine
            
            ctrlSlider.Value = 0: m_boolDirty = True 'reset to dirty
            lblCurrentFrameValue.Caption = CStr(Trim(Str(ctrlSlider.Value)))
            If m_dblFPS <> 0 Then lblCurrentTimeValue.Caption = CStr(Trim(Str(ctrlSlider.Value / m_dblFPS)))
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdBrowse_Click
            ' * procedure description:  Occurs when the user presses and then releases a mouse button over an object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdBrowse_Click()
            Dim nCount As Long
            Dim bstrWriteName As String
            Dim bstrStreamType As String
            Dim intVideoStream As Integer
            Dim objMediaDet As MediaDet
            On Local Error Resume Next
            
            'display the common 'open' dialog
            ctrlCommonDialog.CancelError = True
            ctrlCommonDialog.Filter = "Video Files (*.avi;*.mov)|*.avi;*.mov|"
            ctrlCommonDialog.ShowOpen
            
            If ctrlCommonDialog.FileName <> vbNullString Then
               'assign the filename to the MediaDet
               If File_Exists(ctrlCommonDialog.FileName) Then
                  Set objMediaDet = New MediaDet 'instantiate
                  objMediaDet.FileName = ctrlCommonDialog.FileName
               Else: Exit Sub
               End If
            Else: Exit Sub
            End If
                
            'fashion a new name to write out
            lblReadFileName.Caption = ctrlCommonDialog.FileName
            bstrWriteName = Left$(ctrlCommonDialog.FileName, Len(ctrlCommonDialog.FileName) - 4) + "_T.avi"
            lblWriteFileName.Caption = bstrWriteName: lblPlaybackFileName.Caption = bstrWriteName
            
            'see if there's any video and audio
            m_boolHasAudio = False
            
            intVideoStream = -1
            For nCount = 0 To objMediaDet.OutputStreams - 1
                'get the current stream
                objMediaDet.CurrentStream = nCount
                'obtain the type of stream (audio/video)
                bstrStreamType = objMediaDet.StreamTypeB
                'elect an action based on the stream type
                If bstrStreamType = VIDEO_CLSID Then
                    'video stream
                    intVideoStream = nCount
                    Call SetDuration(objMediaDet.StreamLength, objMediaDet.FrameRate)
                ElseIf bstrStreamType = AUDIO_CLSID Then
                    'audio stream
                    m_boolHasAudio = True
                End If
            Next
            
            'default error
            If intVideoStream = -1 Then
                MsgBox "The Selected File does not contain a video stream.", vbExclamation
                Exit Sub
            End If
            
            'assign the instance to module-level
            If Not objMediaDet Is Nothing Then Set m_objMediaDet = objMediaDet
            If ctrlCommonDialog.FileName <> vbNullString Then m_bstrFileName = ctrlCommonDialog.FileName
            
            'assign the stream info the the ui
            lblStreamsValue.Caption = Trim(CStr(objMediaDet.OutputStreams))
            lblVideoStreamValue.Caption = Trim(Str(intVideoStream))
                        
            ' get a poster frame to start out with
            objMediaDet.WriteBitmapBits 0, picPreview.Width / 15, picPreview.Height / 15, GetTempDirectory + POSTER_FRAME_FILENAME
            picPreview.Picture = LoadPicture(GetTempDirectory + POSTER_FRAME_FILENAME)
            
            'assign state
            m_boolLoaded = True
            m_boolDirty = False
            
            'reset scrollbar
            ctrlSlider.Value = 0
            Call ctrlSlider_Scroll
            
            'set  start/stop
            Call cmdSelStart_Click
            Call cmdSetStop_Click
            
            'enable ui
            ctrlSlider.Enabled = True
            cmdHome.Enabled = True
            cmdEnd.Enabled = True
            cmdBrowse.Enabled = True
            cmdWrite.Enabled = True
            cmdSelStart.Enabled = True
            cmdSetStop.Enabled = True
            cmdBackFrame.Enabled = True
            cmdFwdFrame.Enabled = True
            
            'clean-up & dereference
            If Not objMediaDet Is Nothing Then Set objMediaDet = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdSelStart_Click
            ' * procedure description:  Occurs when the user presses and then releases a mouse button over an object.
            ' *                                       Set the start frame and show a frame for it.
            ' ******************************************************************************************************************************
            Private Sub cmdSelStart_Click()
            On Local Error GoTo ErrLine
            
            If Not m_boolLoaded Then Exit Sub
            
            'setup the ui
            lblStartTimeValue.Caption = Trim(Str(GetCurrentPos))
            lblStartFrameValue.Caption = Trim(Str(ctrlSlider.Value))
            lblVideoStartFrame.Caption = "Video Start Frame:" & Space(2) & Trim(Str(Round(GetCurrentPos, 2)))
            
            'setup the slider
            If ctrlSlider.Value > ctrlSlider.SelStart Then
                ctrlSlider.SelStart = ctrlSlider.Value
                ctrlSlider.SelLength = 0
            Else: ctrlSlider.SelStart = ctrlSlider.Value
            End If
            
            'reset to dirty
            m_boolDirty = True
            'call the timer event proc
            Call tmrTimer_Timer
            'load the picture into the preview pane
            imgPreviewStart.Picture = LoadPicture(GetTempDirectory + POSTER_FRAME_FILENAME)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdSetStop_Click
            ' * procedure description:   Occurs when the user presses and then releases a mouse button over an object.
            ' *                                        Set the stop frame and show a frame for it
            ' ******************************************************************************************************************************
            Private Sub cmdSetStop_Click()
            On Local Error GoTo ErrLine
            
            If Not m_boolLoaded Then Exit Sub
            
            'setup the ui
            lblStopTimeValue.Caption = Trim(Str(GetCurrentPos))
            lblStopFrameValue.Caption = Trim(Str(ctrlSlider.Value))
            lblVideoStopFrame.Caption = "Video Stop Frame:" & Space(2) & Trim(Str(Round(GetCurrentPos, 2)))
            
            'setup the slider
            If ctrlSlider.Value < ctrlSlider.SelStart Then
                ctrlSlider.SelStart = ctrlSlider.Value
                ctrlSlider.SelLength = 0
            Else
                ctrlSlider.SelLength = ctrlSlider.Value - ctrlSlider.SelStart
            End If
            
            'reset to dirty
            m_boolDirty = True
            'call the timer event proc
            Call tmrTimer_Timer
            'load the picture into the preview pane
            imgPreviewStop.Picture = LoadPicture(GetTempDirectory + POSTER_FRAME_FILENAME)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdWrite_Click
            ' * procedure description:   Occurs when the user presses and then releases a mouse button over an object.
            ' *                                        Construct a timeline and write out the file using smart recompression.
            ' ******************************************************************************************************************************
            Private Sub cmdWrite_Click()
            Dim nState As Long
            Dim nReturnCode As Long
            Dim dblPosition As Double
            Dim dblDuration As Double
            Dim dblStartTime As Double
            Dim dblStopTime As Double
            
            Dim objMediaEvent As IMediaEvent
            Dim objMediaPosition As IMediaPosition
            Dim objFilterGraphManager As FilgraphManager
            
            Dim objTimeline As AMTimeline
            Dim objSourceObj As AMTimelineObj
            Dim objTrackObject As AMTimelineObj
            Dim objAudioGroupObj As AMTimelineObj
            Dim objVideoGroupObject As AMTimelineObj
            
            Dim objSource As AMTimelineSrc
            Dim objTrack As AMTimelineTrack
            Dim objAudioGroup As AMTimelineGroup
            Dim objVideoGroup As AMTimelineGroup
            Dim objAudioComposition As AMTimelineComp
            Dim objVideoComposition As AMTimelineComp
            Dim objSmartRenderEngine As New SmartRenderEngine
            On Local Error GoTo ErrLine
            
            
            'disable the form
            Call DisableEverything
            
            
            'instantiate a timeline
            Set objTimeline = New AMTimeline
            'create an empty node on the timeline for the video
            objTimeline.CreateEmptyNode objVideoGroupObject, TIMELINE_MAJOR_TYPE_GROUP
            'derive the video group object from the timeline object
            Set objVideoGroup = objVideoGroupObject
            'set the media type of the video group
            objVideoGroup.SetMediaTypeForVB 0
            'append the video group to the timeline
            objTimeline.AddGroup objVideoGroup
            
            
            
            'create an empty node on the timeline for the track
            objTimeline.CreateEmptyNode objTrackObject, TIMELINE_MAJOR_TYPE_TRACK
            'obtain a composition from the video group
            Set objVideoComposition = objVideoGroup
            'inset the track into the composition
            objVideoComposition.VTrackInsBefore objTrackObject, -1
            'derive the track object
            Set objTrack = objTrackObject
            
            
            
            'create an empty node on the timeline for the source clip
            objTimeline.CreateEmptyNode objSourceObj, TIMELINE_MAJOR_TYPE_SOURCE
            'derive the source clip from the timeline object
            Set objSource = objSourceObj
            'query the ui for duration times
            If m_dblFPS > 0 Then
               dblDuration = ctrlSlider.SelLength / m_dblFPS
               dblStartTime = ctrlSlider.SelStart / m_dblFPS
               dblStopTime = dblStartTime + dblDuration
            Else
               dblDuration = ctrlSlider.SelLength / 15
               dblStartTime = ctrlSlider.SelStart / 15
               dblStopTime = dblStartTime + dblDuration
            End If
            'verify start/stop times
            If dblStopTime = 0 Then
               dblStopTime = 1
            ElseIf dblStartTime = dblStopTime Then
               dblStopTime = dblStartTime + 1
            End If
            'set the start/stop times to the source clip
            objSourceObj.SetStartStop2 0, dblDuration
            objSource.SetMediaTimes2 dblStartTime, dblStopTime
            objSource.SetMediaName m_bstrFileName
            'append the source clip to the track
            objTrack.SrcAdd objSourceObj
            
            
            
            If m_boolHasAudio Then
               'create an empty node on the timeline for the audio group
               objTimeline.CreateEmptyNode objAudioGroupObj, TIMELINE_MAJOR_TYPE_GROUP
               'derive the audio group form the timeline object
               Set objAudioGroup = objAudioGroupObj
               'set the media type of the audio group
               objAudioGroup.SetMediaTypeForVB 1
               'append the group to the timeline
               objTimeline.AddGroup objAudioGroup
               
               'create an empty node on the timeline for the track
               objTimeline.CreateEmptyNode objTrackObject, TIMELINE_MAJOR_TYPE_TRACK
               'derive a composition from the audio group
               Set objAudioComposition = objAudioGroup
               'insetr the track into the composition
               objAudioComposition.VTrackInsBefore objTrackObject, -1
               'derive a track object from the timeline object
               Set objTrack = objTrackObject
               
               'create an empty node for the source clip
               objTimeline.CreateEmptyNode objSourceObj, TIMELINE_MAJOR_TYPE_SOURCE
               'derive a source object from the timeline object
               Set objSource = objSourceObj
               'set the start/stop times from the ui
               objSourceObj.SetStartStop2 0, dblDuration
               objSource.SetMediaTimes2 dblStartTime, dblStopTime
               objSource.SetMediaName m_bstrFileName
               'add the source to the track
               objTrack.SrcAdd objSourceObj
            End If
            
            
            
            ' set the recompression format of the video group
            objVideoGroup.SetRecompFormatFromSource objSource
            'set the timeline to the render engine
            objSmartRenderEngine.SetTimelineObject objTimeline
            'connect-up the render engine
            objSmartRenderEngine.ConnectFrontEnd
            'obtain a reference to the filter graph for the timeline
            objSmartRenderEngine.GetFilterGraph objFilterGraphManager
            'add a file writer and mux filter to the filtergraph
            AddFileWriterAndMux objFilterGraphManager, lblWriteFileName.Caption
            'render the output pins & prepare to proceed with smart render
            RenderGroupPins objSmartRenderEngine, objTimeline
            'run the graph, in turn creating the given file
            objFilterGraphManager.Run
            'obtain a media event from the filtergraph manager
            Set objMediaEvent = objFilterGraphManager
            'obtain the position within the graph
            Set objMediaPosition = objFilterGraphManager
            
            
            
            'display the progress during render
            ctrlProgress.Value = 0
            ctrlProgress.Visible = True: lblWriteFileName.Visible = False
            Do: DoEvents
                  'set the progress bar's current position
                  If dblDuration > 0 Then
                        If Round(ctrlProgress.Value, 0) = 100 Then
                           ctrlProgress.Value = 0
                        Else: ctrlProgress.Value = (ctrlProgress.Value + 1)
                        End If
                  End If
                  'wait until the file has been written, and exit
                  If Not objMediaEvent Is Nothing Then
                  Call objMediaEvent.WaitForCompletion(100, nReturnCode)
                  If nReturnCode = 1 Then Exit Do
                  Else: Exit Do
                  End If
            Loop


Cleanup:
            
            'clean-up code
            ctrlProgress.Value = 100
            ctrlProgress.Visible = False: lblWriteFileName.Visible = True
            cmdWrite.Enabled = True: Call EnableEverything
            
            'scrap the render engine
            If Not objSmartRenderEngine Is Nothing Then objSmartRenderEngine.ScrapIt
            'clean-up & dereference quartz object(s)
            If Not objMediaEvent Is Nothing Then Set objMediaEvent = Nothing
            If Not objMediaPosition Is Nothing Then Set objMediaPosition = Nothing
            If Not objFilterGraphManager Is Nothing Then Set objFilterGraphManager = Nothing
            'clean-up & dereference dexter timeline object(s)
            If Not objTimeline Is Nothing Then Set objTimeline = Nothing
            If Not objSourceObj Is Nothing Then Set objSourceObj = Nothing
            If Not objTrackObject Is Nothing Then Set objTrackObject = Nothing
            If Not objAudioGroupObj Is Nothing Then Set objAudioGroupObj = Nothing
            If Not objVideoGroupObject Is Nothing Then Set objVideoGroupObject = Nothing
            'clean-up & dereference dexter timeline object(s)
            If Not objTrack Is Nothing Then Set objTrack = Nothing
            If Not objSource Is Nothing Then Set objSource = Nothing
            If Not objAudioGroup Is Nothing Then Set objAudioGroup = Nothing
            If Not objVideoGroup Is Nothing Then Set objVideoGroup = Nothing
            If Not objAudioComposition Is Nothing Then Set objAudioComposition = Nothing
            If Not objVideoComposition Is Nothing Then Set objVideoComposition = Nothing
            If Not objSmartRenderEngine Is Nothing Then Set objSmartRenderEngine = Nothing
            Exit Sub
            
ErrLine:

            Select Case Err.Number
                Case 5 'Invalid procedure call or argument
                   Call MsgBox("Error creating file.  Verify that the start/stop times are valid before continuing.", vbExclamation + vbApplicationModal)
                   Err.Clear: GoTo Cleanup
                Case 287 'Application-defined or object-defined error
                   Err.Clear: Resume Next
                Case -2147024864 'The process cannot access the file because it is being used by another process.
                   Call MsgBox(Err.Description, vbExclamation + vbApplicationModal): Err.Clear: GoTo Cleanup
                Case Else 'unknown error
                   Call MsgBox(Err.Description, vbExclamation + vbApplicationModal): Err.Clear: GoTo Cleanup
            End Select
            Exit Sub
            End Sub
            

            ' ******************************************************************************************************************************
            ' * procedure name: ctrlSlider_Scroll
            ' * procedure description:  ctrlSlider scroll event.
            ' *
            ' ******************************************************************************************************************************
            Private Sub ctrlSlider_Scroll()
            On Local Error GoTo ErrLine
            
            If m_boolLoaded Then
               'reset the label caption's
               lblCurrentFrameValue.Caption = CStr(Trim(Str(ctrlSlider.Value)))
               If m_dblFPS <> 0 Then lblCurrentTimeValue.Caption = CStr(Trim(Str(ctrlSlider.Value / m_dblFPS)))
               'reset to dirty
               m_boolDirty = True
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            

            ' ******************************************************************************************************************************
            ' * procedure name: tmrTimer_Timer
            ' * procedure description:  Occurs when a preset interval for a Timer control has elapsed.
            ' *                                        If the UI is dirty, go grab a video frame and draw it.
            ' ******************************************************************************************************************************
            Private Sub tmrTimer_Timer()
            On Local Error GoTo ErrLine
            
            If m_boolDirty Then
               'reset to not dirty
               m_boolDirty = False
               'write out the current frame to the given bitmap file
               m_objMediaDet.WriteBitmapBits GetCurrentPos, picPreview.Width / 15, picPreview.Height / 15, GetTempDirectory + POSTER_FRAME_FILENAME
               'load the picture into the preview pane
               picPreview.Picture = LoadPicture(GetTempDirectory + POSTER_FRAME_FILENAME)
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
' **************************************************************************************************************************************
' * PRIVATE INTERFACE- PROCEDURES
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: EnableEverything
            ' * procedure description:  Enables most controls on the form.
            ' *
            ' ******************************************************************************************************************************
            Private Sub EnableEverything()
            On Local Error GoTo ErrLine
            
            'update ui
            ctrlSlider.Enabled = True
            cmdBrowse.Enabled = True
            cmdWrite.Enabled = True
            cmdSelStart.Enabled = True
            cmdSetStop.Enabled = True
            cmdBackFrame.Enabled = True
            cmdFwdFrame.Enabled = True
            cmdPlayback.Enabled = True
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: DisableEverything
            ' * procedure description:  Disables most controls on the form.
            ' *
            ' ******************************************************************************************************************************
            Private Sub DisableEverything()
            On Local Error GoTo ErrLine
            
            'update ui
            ctrlSlider.Enabled = False
            cmdBrowse.Enabled = False
            cmdWrite.Enabled = False
            cmdSelStart.Enabled = False
            cmdSetStop.Enabled = False
            cmdBackFrame.Enabled = False
            cmdFwdFrame.Enabled = False
            cmdPlayback.Enabled = False
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: SetDuration
            ' * procedure description:  Sets the status within the context of the ui given the duration and the rate.
            ' *
            ' ******************************************************************************************************************************
            Private Sub SetDuration(dblDuration As Double, dblFPS As Double)
            On Local Error GoTo ErrLine
            
            'set module-level data
            m_dblFPS = dblFPS
            m_nFrameCount = (dblDuration * dblFPS)
            
            'setup / update the UI
            ctrlSlider.SelStart = 0
            ctrlSlider.SelLength = 0
            ctrlSlider.Min = 0
            ctrlSlider.Max = m_nFrameCount
            ctrlSlider.LargeChange = (m_nFrameCount / 10)
            ctrlSlider.SmallChange = (m_nFrameCount / 100)
            ctrlSlider.TickFrequency = 100
            lblStartTimeValue.Caption = 0
            lblStopTimeValue.Caption = 0
            lblFPSValue.Caption = Trim(Str(Format(dblFPS, "##.##")))
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: GetDuration
            ' * procedure description:  Returns the duration of the loaded media given the frame count divided by the rate.
            ' *
            ' ******************************************************************************************************************************
            Private Function GetDuration() As Double
            On Local Error GoTo ErrLine
            
            If m_dblFPS = 0 Then Exit Function
            GetDuration = CDbl((m_nFrameCount / m_dblFPS))
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: GetCurrentPos
            ' * procedure description:  Returns the current position given the slider's value divided by the rate.
            ' *
            ' ******************************************************************************************************************************
            Private Function GetCurrentPos() As Double
            On Local Error GoTo ErrLine
            
            If m_dblFPS = 0 Then Exit Function
            If IsNumeric(ctrlSlider.Value) Then
               GetCurrentPos = (ctrlSlider.Value / m_dblFPS)
            End If
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
