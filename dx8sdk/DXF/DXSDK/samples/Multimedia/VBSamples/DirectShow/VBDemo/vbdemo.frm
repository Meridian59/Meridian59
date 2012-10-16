VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "COMDLG32.OCX"
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmMain 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "DirectShow VB Sample"
   ClientHeight    =   8190
   ClientLeft      =   75
   ClientTop       =   600
   ClientWidth     =   5625
   DrawMode        =   1  'Blackness
   FillStyle       =   0  'Solid
   HasDC           =   0   'False
   Icon            =   "vbdemo.frx":0000
   LinkTopic       =   "frmMain"
   LockControls    =   -1  'True
   MaxButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   8190
   ScaleWidth      =   5625
   Begin MSComctlLib.Toolbar tbControlBar 
      Align           =   1  'Align Top
      Height          =   540
      Left            =   0
      TabIndex        =   20
      Top             =   0
      Width           =   5625
      _ExtentX        =   9922
      _ExtentY        =   953
      ButtonWidth     =   820
      ButtonHeight    =   794
      Appearance      =   1
      ImageList       =   "ctrlImageList"
      _Version        =   393216
      BeginProperty Buttons {66833FE8-8583-11D1-B16A-00C0F0283628} 
         NumButtons      =   3
         BeginProperty Button1 {66833FEA-8583-11D1-B16A-00C0F0283628} 
            Key             =   "play"
            Object.ToolTipText     =   "Play"
            ImageIndex      =   1
         EndProperty
         BeginProperty Button2 {66833FEA-8583-11D1-B16A-00C0F0283628} 
            Key             =   "pause"
            Object.ToolTipText     =   "Pause"
            ImageIndex      =   2
         EndProperty
         BeginProperty Button3 {66833FEA-8583-11D1-B16A-00C0F0283628} 
            Key             =   "stop"
            Object.ToolTipText     =   "Stop"
            ImageIndex      =   3
         EndProperty
      EndProperty
   End
   Begin VB.PictureBox picVideoWindow 
      Appearance      =   0  'Flat
      BackColor       =   &H00000000&
      BorderStyle     =   0  'None
      DrawMode        =   1  'Blackness
      FillStyle       =   0  'Solid
      ForeColor       =   &H80000008&
      HasDC           =   0   'False
      Height          =   4095
      Left            =   60
      MouseIcon       =   "vbdemo.frx":0442
      MousePointer    =   99  'Custom
      ScaleHeight     =   4095
      ScaleWidth      =   5475
      TabIndex        =   16
      Top             =   660
      Width           =   5475
   End
   Begin VB.Frame fraInfo 
      Caption         =   "Information:"
      Height          =   2055
      Left            =   60
      TabIndex        =   9
      Top             =   6060
      Width           =   5475
      Begin VB.OptionButton optPlaybackRate 
         Caption         =   "Double (200%)"
         Height          =   195
         Index           =   2
         Left            =   3900
         TabIndex        =   2
         ToolTipText     =   "Double Speed"
         Top             =   1680
         Width           =   1335
      End
      Begin VB.OptionButton optPlaybackRate 
         Caption         =   "Normal (100%)"
         Height          =   195
         Index           =   1
         Left            =   2460
         TabIndex        =   1
         ToolTipText     =   "Normal Speed"
         Top             =   1680
         Width           =   1515
      End
      Begin VB.OptionButton optPlaybackRate 
         Caption         =   "Half (50%)"
         Height          =   195
         Index           =   0
         Left            =   1320
         TabIndex        =   0
         ToolTipText     =   "Half Speed"
         Top             =   1680
         Width           =   1215
      End
      Begin VB.TextBox txtDuration 
         BackColor       =   &H8000000F&
         ForeColor       =   &H80000012&
         Height          =   270
         Left            =   1920
         Locked          =   -1  'True
         TabIndex        =   12
         TabStop         =   0   'False
         Top             =   360
         Width           =   3315
      End
      Begin VB.TextBox txtElapsed 
         BackColor       =   &H8000000F&
         ForeColor       =   &H80000012&
         Height          =   270
         Left            =   1920
         Locked          =   -1  'True
         TabIndex        =   11
         TabStop         =   0   'False
         Top             =   720
         Width           =   3315
      End
      Begin VB.TextBox txtRate 
         BackColor       =   &H8000000F&
         ForeColor       =   &H80000012&
         Height          =   270
         Left            =   1920
         Locked          =   -1  'True
         TabIndex        =   10
         TabStop         =   0   'False
         Top             =   1080
         Width           =   3315
      End
      Begin VB.Label lblResetSpeed 
         Caption         =   "Reset speed:"
         Height          =   255
         Left            =   240
         TabIndex        =   17
         Top             =   1680
         Width           =   1095
      End
      Begin VB.Line Line1 
         X1              =   240
         X2              =   5240
         Y1              =   1500
         Y2              =   1500
      End
      Begin VB.Label lblElapsed 
         Caption         =   "Elapsed Time:"
         Height          =   255
         Left            =   240
         TabIndex        =   15
         ToolTipText     =   "Elapsed Time (Seconds)"
         Top             =   720
         Width           =   1575
      End
      Begin VB.Label lblRate 
         Caption         =   "Playback speed:"
         Height          =   255
         Left            =   240
         TabIndex        =   14
         ToolTipText     =   "Playback Speed (Frames Per Second)"
         Top             =   1080
         Width           =   1335
      End
      Begin VB.Label lblDuration 
         Caption         =   "Length:"
         Height          =   255
         Left            =   240
         TabIndex        =   13
         ToolTipText     =   "Media Length (Seconds)"
         Top             =   360
         Width           =   1455
      End
   End
   Begin VB.Frame frameBalance 
      Caption         =   "Balance"
      Height          =   1215
      Left            =   2820
      TabIndex        =   6
      Top             =   4800
      Width           =   2715
      Begin MSComctlLib.Slider slBalance 
         Height          =   495
         Left            =   340
         TabIndex        =   19
         Top             =   300
         Width           =   2000
         _ExtentX        =   3519
         _ExtentY        =   873
         _Version        =   393216
         LargeChange     =   1000
         SmallChange     =   500
         Min             =   -5000
         Max             =   5000
         TickFrequency   =   1000
      End
      Begin VB.Label lblRight 
         Caption         =   "Right"
         Height          =   255
         Left            =   2160
         TabIndex        =   8
         Top             =   840
         Width           =   435
      End
      Begin VB.Label lblLeft 
         Caption         =   "Left"
         Height          =   255
         Left            =   120
         TabIndex        =   7
         Top             =   840
         Width           =   495
      End
   End
   Begin VB.Timer tmrTimer 
      Left            =   1080
      Top             =   8640
   End
   Begin VB.Frame frameVolume 
      Caption         =   "Volume"
      Height          =   1215
      Left            =   60
      TabIndex        =   3
      Top             =   4800
      Width           =   2595
      Begin MSComctlLib.Slider slVolume 
         Height          =   495
         Left            =   340
         TabIndex        =   18
         Top             =   300
         Width           =   2000
         _ExtentX        =   3519
         _ExtentY        =   873
         _Version        =   393216
         LargeChange     =   400
         SmallChange     =   100
         Min             =   -4000
         Max             =   0
         TickFrequency   =   400
      End
      Begin VB.Label lblMax 
         Caption         =   "Max"
         Height          =   255
         Left            =   2100
         TabIndex        =   5
         Top             =   840
         Width           =   375
      End
      Begin VB.Label lblMin 
         Caption         =   "Min"
         Height          =   255
         Left            =   120
         TabIndex        =   4
         Top             =   840
         Width           =   495
      End
   End
   Begin MSComDlg.CommonDialog ctrlCommonDialog 
      Left            =   600
      Top             =   8580
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin MSComctlLib.ImageList ctrlImageList 
      Left            =   0
      Top             =   8580
      _ExtentX        =   1005
      _ExtentY        =   1005
      BackColor       =   -2147483643
      ImageWidth      =   24
      ImageHeight     =   24
      MaskColor       =   12632256
      _Version        =   393216
      BeginProperty Images {2C247F25-8591-11D1-B16A-00C0F0283628} 
         NumListImages   =   3
         BeginProperty ListImage1 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "vbdemo.frx":0594
            Key             =   ""
         EndProperty
         BeginProperty ListImage2 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "vbdemo.frx":06A6
            Key             =   ""
         EndProperty
         BeginProperty ListImage3 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "vbdemo.frx":07B8
            Key             =   ""
         EndProperty
      EndProperty
   End
   Begin VB.Menu mnu_File 
      Caption         =   "&File"
      Begin VB.Menu mnu_FileOpen 
         Caption         =   "&Open"
         Shortcut        =   ^O
      End
      Begin VB.Menu mnuFileSeptum 
         Caption         =   "-"
      End
      Begin VB.Menu mnu_FileExit 
         Caption         =   "E&xit"
      End
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

Private m_dblRate As Double                          'Rate in Frames Per Second
Private m_bstrFileName As String                   'Loaded Filename
Private m_dblRunLength As Double                'Duration in seconds
Private m_dblStartPosition As Double             'Start position in seconds
Private m_boolVideoRunning As Boolean       'Flag used to trigger clock

Private m_objBasicAudio  As IBasicAudio         'Basic Audio Object
Private m_objBasicVideo As IBasicVideo          'Basic Video Object
Private m_objMediaEvent As IMediaEvent        'MediaEvent Object
Private m_objVideoWindow As IVideoWindow   'VideoWindow Object
Private m_objMediaControl As IMediaControl    'MediaControl Object
Private m_objMediaPosition As IMediaPosition 'MediaPosition Object





' **************************************************************************************************************************************
' * PRIVATE INTERFACE- FORM EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: Form_Load
            ' * procedure description:  Occurs when a form is loaded.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_Load()
            On Local Error GoTo ErrLine
            
            'reset the rate to 1 (normal)
            optPlaybackRate(1).Value = True
            
            'Alter the coordinate system so that we work
            'in pixels (instead of the default twips)
            frmMain.ScaleMode = 3   ' pixels
            
            'Set the granularity for the timer control
            'so that we can display the duration for
            'given video sequence.
            tmrTimer.Interval = 250   '1/4 second intervals
            
            'disable all the control buttons by default
            tbControlBar.Buttons("play").Enabled = False
            tbControlBar.Buttons("stop").Enabled = False
            tbControlBar.Buttons("pause").Enabled = False
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Form_Unload
            ' * procedure description:  Occurs when a form is about to be removed from the screen.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_Unload(Cancel As Integer)
            On Local Error GoTo ErrLine
            
            'stop playback
            m_boolVideoRunning = False
            DoEvents
            'cleanup media control
            If Not m_objMediaControl Is Nothing Then
               m_objMediaControl.Stop
            End If
            'clean-up video window
            If Not m_objVideoWindow Is Nothing Then
               m_objVideoWindow.Left = Screen.Width * 8
               m_objVideoWindow.Height = Screen.Height * 8
               m_objVideoWindow.Owner = 0          'sets the Owner to NULL
            End If
            
            'clean-up & dereference
            If Not m_objBasicAudio Is Nothing Then Set m_objBasicAudio = Nothing
            If Not m_objBasicVideo Is Nothing Then Set m_objBasicVideo = Nothing
            If Not m_objMediaControl Is Nothing Then Set m_objMediaControl = Nothing
            If Not m_objVideoWindow Is Nothing Then Set m_objVideoWindow = Nothing
            If Not m_objMediaPosition Is Nothing Then Set m_objMediaPosition = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuFileExit_Click
            ' * procedure description:  Occurs when the "Exit" option is invoked from the "File" option on the main menubar.
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuFileExit_Click()
            Dim frm As Form
            On Local Error GoTo ErrLine
            
            'unload each loaded form
            For Each frm In Forms
                frm.Move Screen.Width * 8, Screen.Height * 8
                Unload frm
                Set frm = Nothing
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnu_FileExit_Click
            ' * procedure description:   Occurs when the user elects the 'Exit' option via the main 'File' menu.
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnu_FileExit_Click()
            Dim frm As Form
            On Local Error GoTo ErrLine
            
            For Each frm In Forms
               frm.Move Screen.Width * 8, Screen.Height * 8
               frm.Visible = False: Unload frm
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub


            ' ******************************************************************************************************************************
            ' * procedure name: mnu_FileOpen_Click
            ' * procedure description:   Occurs when the user elects the 'Open' option via the main 'File' menu.
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnu_FileOpen_Click()
            Dim nCount As Long
            On Local Error GoTo ErrLine
            
            ' Use the common file dialog to select a media file
            ' (has the extension .AVI or .MPG.)
            ' Initialize global variables based on the
            ' contents of the file:
            '   m_bstrFileName - name of file name selected by the user
            '   m_dblRunLength = length of the file; duration
            '   m_dblStartPosition - point at which to start playing clip
            '   m_objMediaControl, m_objMediaEvent, m_objMediaPosition,
            '   m_objBasicAudio, m_objVideoWindow - programmable objects
            
            'clean up memory (in case a file is already open)
            Call Form_Unload(True)
            
            'Retrieve the name of an .avi or an .mpg
            'file that the user wishes to view.
            ctrlCommonDialog.Filter = "Media Files (*.mpg;*.avi;*.mov;*.wav;*.mp2;*.mp3)|*.mpg;*.avi;*.mov;*.wav;*.mp2;*.mp3"
            ctrlCommonDialog.ShowOpen
            m_bstrFileName = ctrlCommonDialog.FileName
            
            'Instantiate a filter graph for the requested
            'file format.
            Set m_objMediaControl = New FilgraphManager
            Call m_objMediaControl.RenderFile(m_bstrFileName)
            
            'Setup the IBasicAudio object (this
            'is equivalent to calling QueryInterface()
            'on IFilterGraphManager). Initialize the volume
            'to the maximum value.
            
            ' Some filter graphs don't render audio
            ' In this sample, skip setting volume property
            Set m_objBasicAudio = m_objMediaControl
            m_objBasicAudio.Volume = slVolume.Value
            m_objBasicAudio.Balance = slBalance.Value
            
            'Setup the IVideoWindow object. Remove the
            'caption, border, dialog frame, and scrollbars
            'from the default window. Position the window.
            'Set the parent to the app's form.
            Set m_objVideoWindow = m_objMediaControl
            m_objVideoWindow.WindowStyle = CLng(&H6000000)
            m_objVideoWindow.Top = 0
            m_objVideoWindow.Left = 0
            m_objVideoWindow.Width = picVideoWindow.Width
            m_objVideoWindow.Height = picVideoWindow.Height
            'reset the video window owner
            m_objVideoWindow.Owner = picVideoWindow.hWnd
            
            'Setup the IMediaEvent object for the
            'sample toolbar (run, pause, play).
            Set m_objMediaEvent = m_objMediaControl
            
            'Setup the IMediaPosition object so that we
            'can display the duration of the selected
            'video as well as the elapsed time.
            Set m_objMediaPosition = m_objMediaControl
            
            'set the playback rate given the desired optional
            For nCount = optPlaybackRate.LBound To optPlaybackRate.UBound
               If optPlaybackRate(nCount).Value = True Then
                  Select Case nCount
                  Case 0
                         If Not m_objMediaPosition Is Nothing Then _
                            m_objMediaPosition.Rate = 0.5
                  Case 1
                         If Not m_objMediaPosition Is Nothing Then _
                            m_objMediaPosition.Rate = 1
                  Case 2
                         If Not m_objMediaPosition Is Nothing Then _
                            m_objMediaPosition.Rate = 2
                  End Select
                  Exit For
               End If
            Next
            
            m_dblRunLength = Round(m_objMediaPosition.Duration, 2)
            txtDuration.Text = CStr(m_dblRunLength)
            
            ' Reset start position to 0
            m_dblStartPosition = 0
            
            ' Use user-established playback rate
            m_dblRate = m_objMediaPosition.Rate
            txtRate.Text = CStr(m_dblRate)
            
            'enable run buttons by default
            tbControlBar.Buttons("play").Enabled = True
            tbControlBar.Buttons("stop").Enabled = False
            tbControlBar.Buttons("pause").Enabled = False
            
            'run the media file
            Call tbControlBar_ButtonClick(tbControlBar.Buttons(1))
            Exit Sub
            
ErrLine:
            Err.Clear
            Resume Next
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: optPlaybackRate_Click
            ' * procedure description:   Indicates that the contents of a control have changed.
            ' *
            ' ******************************************************************************************************************************
            Private Sub optPlaybackRate_Click(Index As Integer)
            On Local Error GoTo ErrLine
            
            'reset textbox
            Select Case Index
            Case 0
                   If Not m_objMediaPosition Is Nothing Then _
                      txtRate.Text = 0.5
            Case 1
                   If Not m_objMediaPosition Is Nothing Then _
                      txtRate.Text = 1
            Case 2
                   If Not m_objMediaPosition Is Nothing Then _
                      txtRate.Text = 2
            End Select
            
            'reset media playback rate
            If Not m_objMediaPosition Is Nothing Then
               Select Case Index
               Case 0
                      If Not m_objMediaPosition Is Nothing Then _
                         m_objMediaPosition.Rate = 0.5
               Case 1
                      If Not m_objMediaPosition Is Nothing Then _
                         m_objMediaPosition.Rate = 1
               Case 2
                      If Not m_objMediaPosition Is Nothing Then _
                         m_objMediaPosition.Rate = 2
               End Select
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub


            ' ******************************************************************************************************************************
            ' * procedure name: slBalance_Change
            ' * procedure description:   Indicates that the contents of a control have changed.
            ' *
            ' ******************************************************************************************************************************
            Private Sub slBalance_Change()
            On Local Error GoTo ErrLine
            
            'Set the balance using the slider
            If Not m_objMediaControl Is Nothing Then _
               m_objBasicAudio.Balance = slBalance.Value
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: slVolume_Change
            ' * procedure description:   Indicates that the contents of a control have changed.
            ' *
            ' ******************************************************************************************************************************
            Private Sub slVolume_Change()
            On Local Error GoTo ErrLine
            
            'Set the volume using the slider
            If Not m_objMediaControl Is Nothing Then _
               m_objBasicAudio.Volume = slVolume.Value
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: slBalance_MouseMove
            ' * procedure description:    Occurs when the user moves the mouse.
            ' *
            ' ******************************************************************************************************************************
            Private Sub slBalance_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            
            'Set the balance using the slider
            If Not m_objMediaControl Is Nothing Then _
               m_objBasicAudio.Balance = slBalance.Value
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: slVolume_MouseMove
            ' * procedure description:    Occurs when the user moves the mouse.
            ' *
            ' ******************************************************************************************************************************
            Private Sub slVolume_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            
            'Set the volume using the slider
            If Not m_objMediaControl Is Nothing Then _
               m_objBasicAudio.Volume = slVolume.Value
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tbControlBar_ButtonClick
            ' * procedure description:    Occurs when the user clicks on a Button object in a Toolbar control.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tbControlBar_ButtonClick(ByVal Button As Button)
            On Local Error GoTo ErrLine
            
            ' handle buttons on the toolbar
            ' buttons 1, 3 and 5 are defined; 2 and 4 are separators
            ' all DirectShow objects are defined only if the user
            ' has already selected a filename and initialized the objects
            
            ' if the objects aren't defined, avoid errors
            If Not m_objMediaControl Is Nothing Then
               If Button.Key = "play" Then 'PLAY
                  'Invoke the MediaControl Run() method
                  'and pause the video that is being
                  'displayed through the predefined
                  'filter graph.
                  
                  'Assign specified starting position dependent on state
                  If CLng(m_objMediaPosition.CurrentPosition) < CLng(m_dblStartPosition) Then
                      m_objMediaPosition.CurrentPosition = m_dblStartPosition
                  ElseIf CLng(m_objMediaPosition.CurrentPosition) = CLng(m_dblRunLength) Then
                      m_objMediaPosition.CurrentPosition = m_dblStartPosition
                  End If
                  Call m_objMediaControl.Run
                  m_boolVideoRunning = True
                  'enable/disable control buttons
                  tbControlBar.Buttons("play").Enabled = False
                  tbControlBar.Buttons("stop").Enabled = True
                  tbControlBar.Buttons("pause").Enabled = True
                   
               ElseIf Button.Key = "pause" Then  'PAUSE
                  'Invoke the MediaControl Pause() method
                  'and pause the video that is being
                  'displayed through the predefined
                  'filter graph.
                  Call m_objMediaControl.Pause
                  m_boolVideoRunning = False
                  'enable/disable control buttons
                  tbControlBar.Buttons("play").Enabled = True
                  tbControlBar.Buttons("stop").Enabled = True
                  tbControlBar.Buttons("pause").Enabled = False
                  
               ElseIf Button.Key = "stop" Then  'STOP
                  'Invoke the MediaControl Stop() method
                  'and stop the video that is being
                  'displayed through the predefined
                  'filter graph.
                  
                  Call m_objMediaControl.Stop
                  m_boolVideoRunning = False
                  ' reset to the beginning of the video
                  m_objMediaPosition.CurrentPosition = 0
                  txtElapsed.Text = "0.0"
                  'enable/disable control buttons
                  tbControlBar.Buttons("play").Enabled = True
                  tbControlBar.Buttons("stop").Enabled = False
                  tbControlBar.Buttons("pause").Enabled = False
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tmrTimer_Timer
            ' * procedure description:    Occurs when a preset interval for a Timer control has elapsed.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tmrTimer_Timer()
            Dim nReturnCode As Long
            Dim dblPosition As Double
            On Local Error GoTo ErrLine

            'Retrieve the Elapsed Time and
            'display it in the corresponding
            'textbox.
            
            If m_boolVideoRunning = True Then
            
            'obtain return code
               Call m_objMediaEvent.WaitForCompletion(100, nReturnCode)
               
               
               If nReturnCode = 0 Then
                   'get the current position for display
                   dblPosition = m_objMediaPosition.CurrentPosition
                   txtElapsed.Text = CStr(Round(dblPosition, 2))
               Else
                   txtElapsed.Text = CStr(Round(m_dblRunLength, 2))
                   'enable/disable control buttons
                   tbControlBar.Buttons("play").Enabled = True
                   tbControlBar.Buttons("stop").Enabled = False
                   tbControlBar.Buttons("pause").Enabled = False
                   m_boolVideoRunning = False
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Resume Next
            Exit Sub
            End Sub
