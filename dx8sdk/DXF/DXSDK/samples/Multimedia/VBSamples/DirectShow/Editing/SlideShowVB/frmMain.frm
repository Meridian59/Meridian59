VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form frmMain 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "SlideshowVB"
   ClientHeight    =   6480
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   9045
   Icon            =   "frmMain.frx":0000
   LinkTopic       =   "frmMain"
   MaxButton       =   0   'False
   ScaleHeight     =   6480
   ScaleWidth      =   9045
   Visible         =   0   'False
   Begin MSComDlg.CommonDialog ctrlCommonDialog 
      Left            =   60
      Top             =   6000
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin SlideshowVB.SourceClip ctrlSourceClip 
      DragMode        =   1  'Automatic
      Height          =   1965
      Index           =   0
      Left            =   60
      TabIndex        =   12
      TabStop         =   0   'False
      ToolTipText     =   "Source Clip"
      Top             =   75
      Width           =   2190
      _ExtentX        =   3863
      _ExtentY        =   3466
      BorderColor     =   4210752
      BorderSize      =   3
   End
   Begin VB.Frame fraOptions 
      Height          =   1890
      Left            =   60
      TabIndex        =   6
      Top             =   4050
      Width           =   4420
      Begin VB.TextBox txtMaxMediaLength 
         Height          =   375
         Left            =   140
         OLEDropMode     =   1  'Manual
         TabIndex        =   0
         ToolTipText     =   "Maximum playback time per source clip."
         Top             =   480
         Width           =   4150
      End
      Begin VB.ComboBox cmbTransitions 
         Height          =   315
         Left            =   140
         TabIndex        =   1
         ToolTipText     =   "Default Transition"
         Top             =   1440
         Width           =   4150
      End
      Begin VB.Label lblTransitionDescription 
         Caption         =   "Select a transition to use.  If the transition is not installed on your system, the default transition will be used."
         Height          =   375
         Index           =   0
         Left            =   140
         TabIndex        =   7
         Top             =   930
         Width           =   4155
      End
      Begin VB.Label lbltxtMaxMediaLength 
         Caption         =   "Set the maximum time for each clip in the slideshow:"
         Height          =   255
         Index           =   0
         Left            =   140
         TabIndex        =   8
         Top             =   225
         Width           =   4155
      End
   End
   Begin VB.Frame fraCommandFixture 
      Height          =   1890
      Left            =   4560
      TabIndex        =   9
      Top             =   4050
      Width           =   4420
      Begin MSComctlLib.ProgressBar ctrlProgress 
         Height          =   405
         Left            =   140
         TabIndex        =   10
         ToolTipText     =   "Current Progress"
         Top             =   1350
         Visible         =   0   'False
         Width           =   4140
         _ExtentX        =   7303
         _ExtentY        =   714
         _Version        =   393216
         Appearance      =   1
      End
      Begin VB.Label lblInstructions 
         Caption         =   "This interface supports drag-and-drop editing.  Drag your media files into the poster frames to preview, then select a transition."
         BeginProperty Font 
            Name            =   "Comic Sans MS"
            Size            =   9.75
            Charset         =   0
            Weight          =   400
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   990
         Left            =   150
         TabIndex        =   11
         ToolTipText     =   "This interface supports drag-and-drop editing.  Drag your media files into the poster frames to preview, then select a transition."
         Top             =   225
         Width           =   4140
      End
   End
   Begin VB.CommandButton cmdWriteXTL 
      Caption         =   "Write &XTL"
      Height          =   350
      Left            =   5220
      TabIndex        =   3
      ToolTipText     =   "Export using XTL Format"
      Top             =   6075
      Width           =   1215
   End
   Begin VB.CommandButton cmdWriteAVI 
      Caption         =   "&Write AVI"
      Height          =   350
      Left            =   6510
      TabIndex        =   4
      ToolTipText     =   "Export using AVI Format"
      Top             =   6075
      Width           =   1215
   End
   Begin VB.CommandButton cmdPlay 
      Caption         =   "&Play"
      Height          =   350
      Left            =   3960
      TabIndex        =   2
      ToolTipText     =   "Play"
      Top             =   6075
      Width           =   1215
   End
   Begin VB.CommandButton cmdExit 
      Caption         =   "&Exit"
      Default         =   -1  'True
      Height          =   350
      Left            =   7785
      TabIndex        =   5
      ToolTipText     =   "Exit"
      Top             =   6075
      Width           =   1215
   End
   Begin SlideshowVB.SourceClip ctrlSourceClip 
      DragMode        =   1  'Automatic
      Height          =   1965
      Index           =   1
      Left            =   2310
      TabIndex        =   13
      TabStop         =   0   'False
      ToolTipText     =   "Source Clip"
      Top             =   75
      Width           =   2190
      _ExtentX        =   3863
      _ExtentY        =   3466
      BorderColor     =   4210752
      BorderSize      =   3
   End
   Begin SlideshowVB.SourceClip ctrlSourceClip 
      DragMode        =   1  'Automatic
      Height          =   1965
      Index           =   2
      Left            =   4560
      TabIndex        =   14
      TabStop         =   0   'False
      ToolTipText     =   "Source Clip"
      Top             =   75
      Width           =   2190
      _ExtentX        =   3863
      _ExtentY        =   3466
      BorderColor     =   4210752
      BorderSize      =   3
   End
   Begin SlideshowVB.SourceClip ctrlSourceClip 
      DragMode        =   1  'Automatic
      Height          =   1965
      Index           =   3
      Left            =   6810
      TabIndex        =   15
      TabStop         =   0   'False
      ToolTipText     =   "Source Clip"
      Top             =   75
      Width           =   2190
      _ExtentX        =   3863
      _ExtentY        =   3466
      BorderColor     =   4210752
      BorderSize      =   3
   End
   Begin SlideshowVB.SourceClip ctrlSourceClip 
      DragMode        =   1  'Automatic
      Height          =   1965
      Index           =   4
      Left            =   60
      TabIndex        =   16
      TabStop         =   0   'False
      ToolTipText     =   "Source Clip"
      Top             =   2100
      Width           =   2190
      _ExtentX        =   3863
      _ExtentY        =   3466
      BorderColor     =   4210752
      BorderSize      =   3
   End
   Begin SlideshowVB.SourceClip ctrlSourceClip 
      DragMode        =   1  'Automatic
      Height          =   1965
      Index           =   5
      Left            =   2310
      TabIndex        =   17
      TabStop         =   0   'False
      ToolTipText     =   "Source Clip"
      Top             =   2100
      Width           =   2190
      _ExtentX        =   3863
      _ExtentY        =   3466
      BorderColor     =   4210752
      BorderSize      =   3
   End
   Begin SlideshowVB.SourceClip ctrlSourceClip 
      DragMode        =   1  'Automatic
      Height          =   1965
      Index           =   6
      Left            =   4560
      TabIndex        =   18
      TabStop         =   0   'False
      ToolTipText     =   "Source Clip"
      Top             =   2100
      Width           =   2190
      _ExtentX        =   3863
      _ExtentY        =   3466
      BorderColor     =   4210752
      BorderSize      =   3
   End
   Begin SlideshowVB.SourceClip ctrlSourceClip 
      DragMode        =   1  'Automatic
      Height          =   1965
      Index           =   7
      Left            =   6810
      TabIndex        =   19
      TabStop         =   0   'False
      ToolTipText     =   "Source Clip"
      Top             =   2100
      Width           =   2190
      _ExtentX        =   3863
      _ExtentY        =   3466
      BorderColor     =   4210752
      BorderSize      =   3
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

'enable/disable clipsource dragdrop operations
Private m_boolEnableDragDrop As Boolean
'default/highlight border color on clip controls
Private Const HIGHLIGHT_CLIPBORDERCOLOR As Long = vbBlue
Private Const DEFAULT_CLIPBORDERCOLOR As Long = &H404040
'temporary filename for writing out poster frames
Private Const TEMPORARY_XTLFILENAME As String = "SlideshowVB.xtl"
'maximum preview per clip in the slideshow presentation, in seconds
Private m_nMaximumClipLength As Long


' **************************************************************************************************************************************
' * PRIVATE INTERFACE- FORM EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: Form_Initialize
            ' * procedure description:  Occurs when an application creates an instance of a Form, MDIForm, or class.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_Initialize()
            On Local Error GoTo ErrLine
            
            'instantiate global data
            Set gbl_objTimeline = New AMTimeline
            Set gbl_objRenderEngine = New RenderEngine
            Set gbl_objMediaControl = New FilgraphManager
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
            Private Sub Form_Load()
            On Local Error GoTo ErrLine
            
            'enable/disable application
            Call AppEnable(False, True, True)
            
            'assign default value(s)
            m_nMaximumClipLength = 8
            
            'setup default control(s)
            txtMaxMediaLength.Text = 8
            cmbTransitions.Text = vbNullString
            Call ViewTransitionFriendlyNamesDirect(cmbTransitions)

            'assign the default transition
            If TransitionCLSIDToFriendlyName(gbl_objTimeline.GetDefaultTransitionB) <> vbNullString Then _
               cmbTransitions.Text = TransitionCLSIDToFriendlyName(gbl_objTimeline.GetDefaultTransitionB)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Form_QueryUnload
            ' * procedure description:  Occurs before a form or application closes.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_QueryUnload(Cancel As Integer, UnloadMode As Integer)
            Dim frm As Form
            On Local Error GoTo ErrLine
            
            Call RenderTimelineQuasiAsync(Nothing)
            
            Select Case UnloadMode
                Case vbFormControlMenu
                         '0 The user chose the Close command from the Control menu on the form.
                         For Each frm In Forms
                               frm.Move Screen.Width * -8, Screen.Height * -8
                               frm.Visible = False: Unload frm
                         Next
            
                Case vbFormCode
                         '1 The Unload statement is invoked from code.
                         Exit Sub
                         
                Case vbAppWindows
                         '2 The current Microsoft Windows operating environment session is ending.
                         For Each frm In Forms
                               frm.Move Screen.Width * -8, Screen.Height * -8
                               frm.Visible = False: Unload frm
                         Next
                         
                Case vbAppTaskManager
                         '3 The Microsoft Windows Task Manager is closing the application.
                         For Each frm In Forms
                               frm.Move Screen.Width * -8, Screen.Height * -8
                               frm.Visible = False: Unload frm
                         Next
                         End
                         
                Case vbFormMDIForm
                         '4 An MDI child form is closing because the MDI form is closing.
                         Exit Sub
                         
                Case vbFormOwner
                         '5 A form is closing because its owner is closing
                         For Each frm In Forms
                               frm.Move Screen.Width * -8, Screen.Height * -8
                               frm.Visible = False: Unload frm
                         Next
            End Select
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
            
            'clean-up & dereference global data
            If Not gbl_objTimeline Is Nothing Then Set gbl_objTimeline = Nothing
            If Not gbl_objMediaControl Is Nothing Then Set gbl_objMediaControl = Nothing
            If Not gbl_objVideoWindow Is Nothing Then Set gbl_objVideoWindow = Nothing
            If Not gbl_objRenderEngine Is Nothing Then Set gbl_objRenderEngine = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub



' **************************************************************************************************************************************
' * PRIVATE INTERFACE- CONTROL EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: cmdExit_Click
            ' * procedure description:  Occurs when the user presses and then releases a mouse button over an object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdExit_Click()
            Dim frm As Form
            On Local Error GoTo ErrLine
            
            'Invoke the Unload statement on each loaded form
            For Each frm In Forms
               frm.Move Screen.Width * 8, Screen.Height * 8
               Unload frm: Set frm = Nothing
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdPlay_Click
            ' * procedure description:  Occurs when the user presses and then releases a mouse button over an object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdPlay_Click()
            On Local Error GoTo ErrLine
            
            'assign the maximum media length per clip
            If IsNumeric(txtMaxMediaLength.Text) Then _
               m_nMaximumClipLength = CLng(txtMaxMediaLength.Text)
               
            'splice the video clip(s)
            Set gbl_objTimeline = _
                  SpliceVideo(TransitionFriendlyNameToCLSID _
                  ( _
                  cmbTransitions.Text), _
                  ctrlSourceClip(0).MediaFile, _
                  ctrlSourceClip(1).MediaFile, _
                  ctrlSourceClip(2).MediaFile, _
                  ctrlSourceClip(3).MediaFile, _
                  ctrlSourceClip(4).MediaFile, _
                  ctrlSourceClip(5).MediaFile, _
                  ctrlSourceClip(6).MediaFile, _
                  ctrlSourceClip(7).MediaFile _
                  )
            
            'disable the ui
            Call AppEnable(False, False)
            
            'obtain a reference to the filtergraph manager
            If Not gbl_objTimeline Is Nothing Then
               If Not gbl_objRenderEngine Is Nothing Then
                  'set the timeline object
                  Call gbl_objRenderEngine.SetTimelineObject(gbl_objTimeline)
                  'playback the timeline
                  Call RenderTimelineQuasiAsync(gbl_objTimeline)
               End If
            End If
            
            'enable the ui
            If Not gbl_objTimeline Is Nothing Then
               Call AppEnable(True, True)
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Resume Next
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdWriteAVI_Click
            ' * procedure description:  Occurs when the user presses and then releases a mouse button over an object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdWriteAVI_Click()
            Dim nState As Long
            Dim nReturnCode As Long
            Dim dblPosition As Double
            Dim dblDuration As Double
            Dim bstrFileName As String
            Dim objMediaEvent As IMediaEvent
            Dim objMediaPosition As IMediaPosition
            Dim objFilterGraphManager As FilgraphManager
            Dim objSmartRenderEngine As SmartRenderEngine
            On Error GoTo ErrLine
            
            'assign the maximum media length per clip
            If IsNumeric(txtMaxMediaLength.Text) Then _
               m_nMaximumClipLength = CLng(txtMaxMediaLength.Text)
            
            'splice the video clip(s)
            Set gbl_objTimeline = _
                  SpliceVideo(TransitionFriendlyNameToCLSID _
                  ( _
                  cmbTransitions.Text), _
                  ctrlSourceClip(0).MediaFile, _
                  ctrlSourceClip(1).MediaFile, _
                  ctrlSourceClip(2).MediaFile, _
                  ctrlSourceClip(3).MediaFile, _
                  ctrlSourceClip(4).MediaFile, _
                  ctrlSourceClip(5).MediaFile, _
                  ctrlSourceClip(6).MediaFile, _
                  ctrlSourceClip(7).MediaFile _
                  )
            
                  
            'disable the ui
            Call AppEnable(False, False, False)
            
            'query the user for a media file
            ctrlCommonDialog.DefaultExt = "AVI"
            ctrlCommonDialog.InitDir = vbNullString
            ctrlCommonDialog.Filter = "*.avi|*.avi"
            Call ctrlCommonDialog.ShowSave
            bstrFileName = ctrlCommonDialog.FileName

            If bstrFileName = vbNullString Then
               'enable the ui / user cancel
               Call AppEnable(True, True, True)
               Exit Sub
            End If
            
            
            'instantiate a smart render engine
            Set objSmartRenderEngine = New SmartRenderEngine
            'set the timeline object to the render engine
            objSmartRenderEngine.SetTimelineObject gbl_objTimeline
            'connect-up the render engine's frontend
            objSmartRenderEngine.ConnectFrontEnd
            'obtain an instance of the filtergraph manager
            objSmartRenderEngine.GetFilterGraph objFilterGraphManager
            'append a filewriter and av mux filter to the graph
            AddFileWriterAndMux objFilterGraphManager, bstrFileName
            'render the output pins on the smart render engine
            RenderGroupPins objSmartRenderEngine, gbl_objTimeline
            'render the filtergraph
            objFilterGraphManager.Run
            'derive the media event interface from the filtergraph manager
            Set objMediaEvent = objFilterGraphManager
            Set objMediaPosition = objMediaEvent
            
            'display the progress during render
            ctrlProgress.Value = 0
            ctrlProgress.Visible = True
            dblDuration = objMediaPosition.Duration
            If dblDuration > 0 Then
               Do Until ctrlProgress.Value = 100: DoEvents
                     'query current position
                     dblPosition = objMediaPosition.CurrentPosition
                     'set the progress bar's current position
                     If dblPosition <> 0 Then
                        If dblDuration > 0 Then
                           ctrlProgress.Value = dblPosition * 100 / dblDuration
                        Else: ctrlProgress.Value = 100: Exit Do
                        End If
                     Else: ctrlProgress.Value = 100: Exit Do
                     End If
               Loop
            End If
            ctrlProgress.Value = 100
            ctrlProgress.Visible = False
            
            'enable the ui
            Call AppEnable(True, True, True)
            
            'clean-up & dereference
            If Not objMediaEvent Is Nothing Then Set objMediaEvent = Nothing
            If Not objMediaPosition Is Nothing Then Set objMediaPosition = Nothing
            If Not objFilterGraphManager Is Nothing Then Set objFilterGraphManager = Nothing
            If Not objSmartRenderEngine Is Nothing Then Set objSmartRenderEngine = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Resume Next
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdWriteXTL_Click
            ' * procedure description:  Occurs when the user presses and then releases a mouse button over an object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdWriteXTL_Click()
            Dim bstrFileName As String
            Dim objXml2Dex As Xml2Dex
            On Local Error GoTo ErrLine
            
            'assign the maximum media length per clip
            If IsNumeric(txtMaxMediaLength.Text) Then _
               m_nMaximumClipLength = CLng(txtMaxMediaLength.Text)
               
            'splice the video clip(s)
            Set gbl_objTimeline = _
                  SpliceVideo(TransitionFriendlyNameToCLSID _
                  ( _
                  cmbTransitions.Text), _
                  ctrlSourceClip(0).MediaFile, _
                  ctrlSourceClip(1).MediaFile, _
                  ctrlSourceClip(2).MediaFile, _
                  ctrlSourceClip(3).MediaFile, _
                  ctrlSourceClip(4).MediaFile, _
                  ctrlSourceClip(5).MediaFile, _
                  ctrlSourceClip(6).MediaFile, _
                  ctrlSourceClip(7).MediaFile _
                  )
            
            'disable the ui
            Call AppEnable(False, False, False)
            
            'query the user for a media file
            ctrlCommonDialog.DefaultExt = "XTL"
            ctrlCommonDialog.InitDir = vbNullString
            ctrlCommonDialog.Filter = "*.xtl|*.xtl"
            Call ctrlCommonDialog.ShowSave
            bstrFileName = ctrlCommonDialog.FileName

            If bstrFileName = vbNullString Then
               'enable the ui
               Call AppEnable(True, True, True)
               Exit Sub
            Else
               'if the file already exists, then delete it
               If File_Exists(bstrFileName) Then _
                  Call File_Delete(bstrFileName, False, False, False)
            End If
            
            'obtain a reference to the filtergraph manager
            If Not gbl_objTimeline Is Nothing Then
               If Not gbl_objRenderEngine Is Nothing Then
                  'set the timeline object
                  Call gbl_objRenderEngine.SetTimelineObject(gbl_objTimeline)
                  'render the timeline
                  Call SaveTimeline(gbl_objTimeline, bstrFileName, DEXExportXTL)
               End If
            End If
            
            'enable the ui
            Call AppEnable(True, True, True)
            Exit Sub
            
ErrLine:
            Err.Clear
            Resume Next
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: ctrlSourceClip_Import
            ' * procedure description:  Occurs whenever an import of a media file into the clipsource control is attempted.
            ' *                                       Set the second argument, 'Cancel' to true to cancel the operation and prevent the import.
            ' ******************************************************************************************************************************
            Private Sub ctrlSourceClip_Import(Index As Integer, bstrFileName As String, Cancel As Boolean)
            On Local Error GoTo ErrLine
            
            'enable/disable drag/drop
            If m_boolEnableDragDrop = False Then
               Cancel = True
               Exit Sub
            End If
            
            'otherwise enable everything
            Call AppEnable(True, True, True)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: ctrlSourceClip_DragDrop
            ' * procedure description:  Occurs when a drag-and-drop operation is completed.
            ' *
            ' ******************************************************************************************************************************
            Private Sub ctrlSourceClip_DragDrop(Index As Integer, Source As Control, X As Single, Y As Single)
            Dim nCount As Long
            On Local Error GoTo ErrLine
            
            For nCount = ctrlSourceClip.LBound To ctrlSourceClip.UBound
               If nCount <> Index Then
                  If ctrlSourceClip(nCount).BorderColor <> DEFAULT_CLIPBORDERCOLOR Then _
                     ctrlSourceClip(nCount).BorderColor = DEFAULT_CLIPBORDERCOLOR
               Else
                  If ctrlSourceClip(nCount).BorderColor <> HIGHLIGHT_CLIPBORDERCOLOR Then _
                     ctrlSourceClip(nCount).BorderColor = HIGHLIGHT_CLIPBORDERCOLOR
               End If
            Next
            'reset default media file
            ctrlSourceClip(Index).MediaFile = Source.MediaFile
            'reset the default color to the clip control
            For nCount = ctrlSourceClip.LBound To ctrlSourceClip.UBound
               If ctrlSourceClip(nCount).BorderColor <> DEFAULT_CLIPBORDERCOLOR Then _
                  ctrlSourceClip(nCount).BorderColor = DEFAULT_CLIPBORDERCOLOR
               Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: ctrlSourceClip_DragOver
            ' * procedure description:  Occurs when a drag-and-drop operation is in progress.
            ' *
            ' ******************************************************************************************************************************
            Private Sub ctrlSourceClip_DragOver(Index As Integer, Source As Control, X As Single, Y As Single, State As Integer)
            Dim nCount As Long
            On Local Error GoTo ErrLine
            
            For nCount = ctrlSourceClip.LBound To ctrlSourceClip.UBound
               If nCount <> Index Then
                  If ctrlSourceClip(nCount).BorderColor <> DEFAULT_CLIPBORDERCOLOR Then _
                     ctrlSourceClip(nCount).BorderColor = DEFAULT_CLIPBORDERCOLOR
               Else
                  If ctrlSourceClip(nCount).BorderColor <> HIGHLIGHT_CLIPBORDERCOLOR Then _
                     ctrlSourceClip(nCount).BorderColor = HIGHLIGHT_CLIPBORDERCOLOR
               End If
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub

            


' **************************************************************************************************************************************
' * PRIVATE INTERFACE- CONTROL EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: cmdPlay_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *                                        Reset control parameter(s) to default setting(s)
            ' ******************************************************************************************************************************
            Private Sub cmdPlay_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            Dim nCount As Long
            On Local Error GoTo ErrLine
            
            For nCount = ctrlSourceClip.LBound To ctrlSourceClip.UBound
            If ctrlSourceClip(nCount).BorderColor <> DEFAULT_CLIPBORDERCOLOR Then _
               ctrlSourceClip(nCount).BorderColor = DEFAULT_CLIPBORDERCOLOR
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdWriteAVI_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *                                        Reset control parameter(s) to default setting(s)
            ' ******************************************************************************************************************************
            Private Sub cmdWriteAVI_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            Dim nCount As Long
            On Local Error GoTo ErrLine
            
            For nCount = ctrlSourceClip.LBound To ctrlSourceClip.UBound
            If ctrlSourceClip(nCount).BorderColor <> DEFAULT_CLIPBORDERCOLOR Then _
               ctrlSourceClip(nCount).BorderColor = DEFAULT_CLIPBORDERCOLOR
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdWriteXTL_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *                                        Reset control parameter(s) to default setting(s)
            ' ******************************************************************************************************************************
            Private Sub cmdWriteXTL_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            Dim nCount As Long
            On Local Error GoTo ErrLine
            
            For nCount = ctrlSourceClip.LBound To ctrlSourceClip.UBound
            If ctrlSourceClip(nCount).BorderColor <> DEFAULT_CLIPBORDERCOLOR Then _
               ctrlSourceClip(nCount).BorderColor = DEFAULT_CLIPBORDERCOLOR
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdExit_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *                                        Reset control parameter(s) to default setting(s)
            ' ******************************************************************************************************************************
            Private Sub cmdExit_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            Dim nCount As Long
            On Local Error GoTo ErrLine
            
            For nCount = ctrlSourceClip.LBound To ctrlSourceClip.UBound
            If ctrlSourceClip(nCount).BorderColor <> DEFAULT_CLIPBORDERCOLOR Then _
               ctrlSourceClip(nCount).BorderColor = DEFAULT_CLIPBORDERCOLOR
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: ctrlProgress_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *                                        Reset control parameter(s) to default setting(s)
            ' ******************************************************************************************************************************
            Private Sub ctrlProgress_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            Dim nCount As Long
            On Local Error GoTo ErrLine
            
            For nCount = ctrlSourceClip.LBound To ctrlSourceClip.UBound
            If ctrlSourceClip(nCount).BorderColor <> DEFAULT_CLIPBORDERCOLOR Then _
               ctrlSourceClip(nCount).BorderColor = DEFAULT_CLIPBORDERCOLOR
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Form_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *                                        Reset control parameter(s) to default setting(s)
            ' ******************************************************************************************************************************
            Private Sub Form_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            Dim nCount As Long
            On Local Error GoTo ErrLine
            
            For nCount = ctrlSourceClip.LBound To ctrlSourceClip.UBound
            If ctrlSourceClip(nCount).BorderColor <> DEFAULT_CLIPBORDERCOLOR Then _
               ctrlSourceClip(nCount).BorderColor = DEFAULT_CLIPBORDERCOLOR
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: fraCommandFixture_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *                                        Reset control parameter(s) to default setting(s)
            ' ******************************************************************************************************************************
            Private Sub fraCommandFixture_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            Dim nCount As Long
            On Local Error GoTo ErrLine
            
            For nCount = ctrlSourceClip.LBound To ctrlSourceClip.UBound
            If ctrlSourceClip(nCount).BorderColor <> DEFAULT_CLIPBORDERCOLOR Then _
               ctrlSourceClip(nCount).BorderColor = DEFAULT_CLIPBORDERCOLOR
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: fraOptions_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *                                        Reset control parameter(s) to default setting(s)
            ' ******************************************************************************************************************************
            Private Sub fraOptions_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            Dim nCount As Long
            On Local Error GoTo ErrLine
            
            For nCount = ctrlSourceClip.LBound To ctrlSourceClip.UBound
            If ctrlSourceClip(nCount).BorderColor <> DEFAULT_CLIPBORDERCOLOR Then _
               ctrlSourceClip(nCount).BorderColor = DEFAULT_CLIPBORDERCOLOR
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: lblInstructions_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *                                        Reset control parameter(s) to default setting(s)
            ' ******************************************************************************************************************************
            Private Sub lblInstructions_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            Dim nCount As Long
            On Local Error GoTo ErrLine
            
            For nCount = ctrlSourceClip.LBound To ctrlSourceClip.UBound
            If ctrlSourceClip(nCount).BorderColor <> DEFAULT_CLIPBORDERCOLOR Then _
               ctrlSourceClip(nCount).BorderColor = DEFAULT_CLIPBORDERCOLOR
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: lblTransitionDescription_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *                                        Reset control parameter(s) to default setting(s)
            ' ******************************************************************************************************************************
            Private Sub lblTransitionDescription_MouseMove(Index As Integer, Button As Integer, Shift As Integer, X As Single, Y As Single)
            Dim nCount As Long
            On Local Error GoTo ErrLine
            
            For nCount = ctrlSourceClip.LBound To ctrlSourceClip.UBound
            If ctrlSourceClip(nCount).BorderColor <> DEFAULT_CLIPBORDERCOLOR Then _
               ctrlSourceClip(nCount).BorderColor = DEFAULT_CLIPBORDERCOLOR
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: lbltxtMaxMediaLength_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *                                        Reset control parameter(s) to default setting(s)
            ' ******************************************************************************************************************************
            Private Sub lbltxtMaxMediaLength_MouseMove(Index As Integer, Button As Integer, Shift As Integer, X As Single, Y As Single)
            Dim nCount As Long
            On Local Error GoTo ErrLine
            
            For nCount = ctrlSourceClip.LBound To ctrlSourceClip.UBound
            If ctrlSourceClip(nCount).BorderColor <> DEFAULT_CLIPBORDERCOLOR Then _
               ctrlSourceClip(nCount).BorderColor = DEFAULT_CLIPBORDERCOLOR
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: txtMaxMediaLength_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *                                        Reset control parameter(s) to default setting(s)
            ' ******************************************************************************************************************************
            Private Sub txtMaxMediaLength_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            Dim nCount As Long
            On Local Error GoTo ErrLine
            
            For nCount = ctrlSourceClip.LBound To ctrlSourceClip.UBound
            If ctrlSourceClip(nCount).BorderColor <> DEFAULT_CLIPBORDERCOLOR Then _
               ctrlSourceClip(nCount).BorderColor = DEFAULT_CLIPBORDERCOLOR
            Next
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
            ' * procedure name: SpliceVideo
            ' * procedure description:  Splices a variable number of video files together using the given transition.
            ' *                                       DefaultTransitionCLSID evaluates to the CLSID of the desired transition to use.
            ' *                                       Files evaluates to a variable number of BSTR String arguments containing the filename(s)
            ' ******************************************************************************************************************************
            Private Function SpliceVideo(DefaultTransitionCLSID As String, ParamArray Files()) As AMTimeline
            Dim nCount As Long
            Dim nCount2 As Long
            Dim bstrCurrentFile As String
            Dim boolAudioGroup As Boolean
            Dim boolVideoGroup As Boolean
            Dim dblAudioStartTime As Double
            Dim dblAudioStopTime As Double
            Dim dblVideoStartTime As Double
            Dim dblVideoStopTime As Double
            Dim objTimeline As AMTimeline
            Dim objNewSource As AMTimelineSrc
            Dim objNewTrack As AMTimelineTrack
            Dim objTransition As AMTimelineTrans
            Dim objAudioGroup As AMTimelineGroup
            Dim objVideoGroup As AMTimelineGroup
            Dim objTimelineTrackObject As AMTimelineObj
            Dim objTimelineSourceObject As AMTimelineObj
            Dim objTimelineAudioGroupObject As AMTimelineObj
            Dim objTimelineVideoGroupObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
            'instantiate new timeline
            Set objTimeline = CreateTimeline
            
            'enable transitions on the timeline
            Call objTimeline.EnableTransitions(1)
            
            'enumerate the files and place the group(s) on the timeline
            For nCount = LBound(Files) To UBound(Files)
                  If TypeName(Files(nCount)) = "String" Then
                     If Files(nCount) <> vbNullString Then
                        bstrCurrentFile = Files(nCount)
                        If HasStreams(bstrCurrentFile) Then
                           If HasAudioStream(bstrCurrentFile) Then
                              'enumerate all the groups in the timeline to ensure audio has not yet been added
                              If GetGroupCount(objTimeline) > 0 Then
                                 For nCount2 = 0 To GetGroupCount(objTimeline) - 1
                                    If Not GroupFromTimeline(objTimeline, nCount2) Is Nothing Then
                                       If GroupFromTimeline(objTimeline, nCount2).GetGroupName = "AUDIO" Then
                                           boolAudioGroup = True
                                       End If
                                    End If
                                 Next
                                 If boolAudioGroup = False Then
                                    'insert an audio group into the timeline
                                    Set objAudioGroup = CreateGroup(objTimeline, "AUDIO", DEXMediaTypeAudio)
                                    Call InsertGroup(objTimeline, objAudioGroup)
                                    Set objTimelineAudioGroupObject = objAudioGroup
                                    boolAudioGroup = True
                                 End If
                              Else:
                                       'insert an audio group into the timeline
                                       Set objAudioGroup = CreateGroup(objTimeline, "AUDIO", DEXMediaTypeAudio)
                                       Call InsertGroup(objTimeline, objAudioGroup)
                                       Set objTimelineAudioGroupObject = objAudioGroup
                                       boolAudioGroup = True
                              End If
                           End If
                           
                           If HasVideoStream(bstrCurrentFile) Then
                              'enumerate all the groups in the timeline to ensure audio has not yet been added
                              If GetGroupCount(objTimeline) > 0 Then
                                 For nCount2 = 0 To GetGroupCount(objTimeline) - 1
                                    If Not GroupFromTimeline(objTimeline, nCount2) Is Nothing Then
                                       If GroupFromTimeline(objTimeline, nCount2).GetGroupName = "VIDEO" Then
                                          boolVideoGroup = True
                                       End If
                                    End If
                                 Next
                                 If boolVideoGroup = False Then
                                    'insert a video group into the timeline
                                    Set objVideoGroup = CreateGroup(objTimeline, "VIDEO", DEXMediaTypeVideo)
                                    Call InsertGroup(objTimeline, objVideoGroup)
                                    Set objTimelineVideoGroupObject = objVideoGroup
                                    boolVideoGroup = True
                                 End If
                              Else:
                                       'insert a video group into the timeline
                                       Set objVideoGroup = CreateGroup(objTimeline, "VIDEO", DEXMediaTypeVideo)
                                       Call InsertGroup(objTimeline, objVideoGroup)
                                       Set objTimelineVideoGroupObject = objVideoGroup
                                       boolVideoGroup = True
                              End If
                           End If
                        End If
                     End If
                  End If
            Next
            
            'enumerate the files and place the tracks/source(s) on the timeline
            For nCount = LBound(Files) To UBound(Files)
                  If TypeName(Files(nCount)) = "String" Then
                     If Files(nCount) <> vbNullString Then
                        bstrCurrentFile = Files(nCount)
                        If HasVideoStream(bstrCurrentFile) Then
                           'insert a new video track for the clip in the timeline
                           Set objNewTrack = CreateTrack(objTimeline)
                           Set objTimelineTrackObject = objNewTrack
                           Call InsertTrack(objNewTrack, objTimelineVideoGroupObject)
                           'inset a new sourceclip into the timeline
                           Set objNewSource = CreateSource(objTimeline)
                           'insert the new source clip into the new track
                           If dblVideoStopTime = 0 Then
                              dblVideoStartTime = m_nMaximumClipLength * (nCount): dblVideoStopTime = (m_nMaximumClipLength * (nCount + 1)) + 1
                           Else: dblVideoStartTime = (m_nMaximumClipLength * (nCount)) - 1: dblVideoStopTime = (m_nMaximumClipLength * (nCount + 1)) + 1
                           End If
                           Call InsertSource(objNewTrack, objNewSource, bstrCurrentFile, dblVideoStartTime, dblVideoStopTime)
                           'insert a new transition into each track on the timeline
                           If DefaultTransitionCLSID <> vbNullString Then
                              Set objTransition = CreateTransition(objTimeline)
                              dblVideoStartTime = ((m_nMaximumClipLength * (nCount))) - 1: dblVideoStopTime = (m_nMaximumClipLength * nCount + 1)
                              If dblVideoStartTime < 0 Then dblVideoStartTime = 0
                              Call InsertTransition(objTransition, objTimelineTrackObject, DefaultTransitionCLSID, dblVideoStartTime, dblVideoStopTime)
                           End If
                        End If
                        
                        If HasAudioStream(bstrCurrentFile) Then
                           'insert a new audio track for the clip in the timeline
                           Set objNewTrack = CreateTrack(objTimeline)
                           Set objTimelineTrackObject = objNewTrack
                           Call InsertTrack(objNewTrack, objTimelineAudioGroupObject)
                           'inset a new sourceclip into the timeline
                           Set objNewSource = CreateSource(objTimeline)
                           'insert the new source clip into the new track
                           If dblAudioStopTime = 0 Then
                              dblAudioStartTime = m_nMaximumClipLength * (nCount): dblAudioStopTime = (m_nMaximumClipLength * (nCount + 1)) + 1
                           Else: dblAudioStartTime = (m_nMaximumClipLength * (nCount)) - 1: dblAudioStopTime = (m_nMaximumClipLength * (nCount + 1)) + 1
                           End If
                           Call InsertSource(objNewTrack, objNewSource, bstrCurrentFile, dblAudioStartTime, dblAudioStopTime)
                        End If
                     End If
                  End If
            Next
            
            'return the timeline
            If Not objTimeline Is Nothing Then Set SpliceVideo = objTimeline
            
            'clean-up & dereference
            If Not objTimeline Is Nothing Then Set objTimeline = Nothing  ' AMTimeline
            If Not objNewSource Is Nothing Then Set objNewSource = Nothing  ' AMTimelineSrc
            If Not objNewTrack Is Nothing Then Set objNewTrack = Nothing  ' AMTimelineTrack
            If Not objTransition Is Nothing Then Set objTransition = Nothing ' AMTimelineTrans
            If Not objAudioGroup Is Nothing Then Set objAudioGroup = Nothing  ' AMTimelineGroup
            If Not objVideoGroup Is Nothing Then Set objVideoGroup = Nothing  ' AMTimelineGroup
            If Not objTimelineTrackObject Is Nothing Then Set objTimelineTrackObject = Nothing  ' AMTimelineObj
            If Not objTimelineSourceObject Is Nothing Then Set objTimelineSourceObject = Nothing  ' AMTimelineObj
            If Not objTimelineAudioGroupObject Is Nothing Then Set objTimelineAudioGroupObject = Nothing  ' AMTimelineObj
            If Not objTimelineVideoGroupObject Is Nothing Then Set objTimelineVideoGroupObject = Nothing  ' AMTimelineObj
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function



            ' ******************************************************************************************************************************
            ' * procedure name: ViewTransitionFriendlyNamesDirect
            ' * procedure description:  Maps transition friendly names to a combobox for easy viewing.
            ' *
            ' ******************************************************************************************************************************
            Private Sub ViewTransitionFriendlyNamesDirect(cmbComboBox As Control)
            On Local Error GoTo ErrLine
            
            If Not cmbComboBox Is Nothing Then
               If TypeName(cmbComboBox) = "ComboBox" Then
                  With cmbComboBox
                     .AddItem "Barn"
                     .AddItem "Blinds"
                     .AddItem "BurnFilm"
                     .AddItem "CenterCurls"
                     .AddItem "ColorFade"
                     .AddItem "Compositor"
                     .AddItem "Curls"
                     .AddItem "Curtains"
                     .AddItem "Fade"
                     .AddItem "FadeWhite"
                     .AddItem "FlowMotion"
                     .AddItem "GlassBlock"
                     .AddItem "Grid"
                     .AddItem "Inset"
                     .AddItem "Iris"
                     .AddItem "Jaws"
                     .AddItem "Lens"
                     .AddItem "LightWipe"
                     .AddItem "Liquid"
                     .AddItem "PageCurl"
                     .AddItem "PeelABCD"
                     .AddItem "Pixelate"
                     .AddItem "RadialWipe"
                     .AddItem "Ripple"
                     .AddItem "RollDown"
                     .AddItem "Slide"
                     .AddItem "SMPTE Wipe"
                     .AddItem "Spiral"
                     .AddItem "Stretch"
                     .AddItem "Threshold"
                     .AddItem "Twister"
                     .AddItem "Vacuum"
                     .AddItem "Water"
                     .AddItem "Wheel"
                     .AddItem "Wipe"
                     .AddItem "WormHole"
                     .AddItem "Zigzag"
                  End With
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: AppEnable
            ' * procedure description:  Enabled/Disables the application's controls.
            ' *
            ' ******************************************************************************************************************************
            Private Sub AppEnable(EnableControls As Boolean, Optional EnableDragDrop As Boolean = True, Optional EnableExit As Boolean = True)
            On Local Error GoTo ErrLine
            
            'enable/disable controls
            If EnableControls Then
               cmdPlay.Enabled = True
               cmdWriteAVI.Enabled = True
               cmdWriteXTL.Enabled = True
            Else
               cmdPlay.Enabled = False
               cmdWriteAVI.Enabled = False
               cmdWriteXTL.Enabled = False
            End If
            
            'enable/disable drag/drop
            If EnableDragDrop Then
               m_boolEnableDragDrop = True
            Else: m_boolEnableDragDrop = False
            End If
            
            'enable/disable exit
            If EnableExit Then
               cmdExit.Enabled = True
            Else: cmdExit.Enabled = False
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
