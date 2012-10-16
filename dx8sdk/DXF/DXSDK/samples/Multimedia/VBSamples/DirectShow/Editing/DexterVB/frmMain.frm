VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "COMDLG32.OCX"
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Object = "{38911DA0-E448-11D0-84A3-00DD01104159}#1.1#0"; "COMCT332.OCX"
Begin VB.Form frmMain 
   Caption         =   "DexterVB"
   ClientHeight    =   5565
   ClientLeft      =   60
   ClientTop       =   645
   ClientWidth     =   8610
   Icon            =   "frmMain.frx":0000
   LinkTopic       =   "Form1"
   LockControls    =   -1  'True
   ScaleHeight     =   5565
   ScaleWidth      =   8610
   WhatsThisButton =   -1  'True
   WhatsThisHelp   =   -1  'True
   Begin VB.Timer Timer1 
      Left            =   9300
      Top             =   1140
   End
   Begin MSComctlLib.ImageList ctrlMainToolbarImageListHot 
      Left            =   9300
      Top             =   480
      _ExtentX        =   1005
      _ExtentY        =   1005
      BackColor       =   -2147483643
      ImageWidth      =   16
      ImageHeight     =   15
      MaskColor       =   16711935
      _Version        =   393216
      BeginProperty Images {2C247F25-8591-11D1-B16A-00C0F0283628} 
         NumListImages   =   10
         BeginProperty ListImage1 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":030A
            Key             =   ""
         EndProperty
         BeginProperty ListImage2 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":062C
            Key             =   ""
         EndProperty
         BeginProperty ListImage3 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":094E
            Key             =   ""
         EndProperty
         BeginProperty ListImage4 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":0C70
            Key             =   ""
         EndProperty
         BeginProperty ListImage5 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":0F62
            Key             =   ""
         EndProperty
         BeginProperty ListImage6 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":1254
            Key             =   ""
         EndProperty
         BeginProperty ListImage7 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":1546
            Key             =   ""
         EndProperty
         BeginProperty ListImage8 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":1838
            Key             =   ""
         EndProperty
         BeginProperty ListImage9 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":1B2A
            Key             =   ""
         EndProperty
         BeginProperty ListImage10 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":1E1C
            Key             =   ""
         EndProperty
      EndProperty
   End
   Begin ComCtl3.CoolBar ctrlCoolBar 
      Height          =   405
      Left            =   0
      TabIndex        =   3
      Top             =   0
      Width           =   8580
      _ExtentX        =   15134
      _ExtentY        =   714
      BandCount       =   2
      FixedOrder      =   -1  'True
      BandBorders     =   0   'False
      OLEDropMode     =   1
      MousePointer    =   1
      _CBWidth        =   8580
      _CBHeight       =   405
      _Version        =   "6.7.8988"
      MinHeight1      =   345
      NewRow1         =   0   'False
      Child2          =   "tbMain"
      MinHeight2      =   315
      Width2          =   5730
      NewRow2         =   0   'False
      AllowVertical2  =   0   'False
      Begin MSComctlLib.Toolbar tbMain 
         Height          =   315
         Left            =   165
         TabIndex        =   4
         Top             =   45
         Width           =   8325
         _ExtentX        =   14684
         _ExtentY        =   556
         ButtonWidth     =   609
         ButtonHeight    =   556
         AllowCustomize  =   0   'False
         Wrappable       =   0   'False
         Style           =   1
         ImageList       =   "ctrlMainToolbarImageList"
         HotImageList    =   "ctrlMainToolbarImageListHot"
         _Version        =   393216
         BeginProperty Buttons {66833FE8-8583-11D1-B16A-00C0F0283628} 
            NumButtons      =   11
            BeginProperty Button1 {66833FEA-8583-11D1-B16A-00C0F0283628} 
               Key             =   "New"
               Description     =   "New TimeLine"
               Object.ToolTipText     =   "Create A New Timeline"
               ImageIndex      =   1
            EndProperty
            BeginProperty Button2 {66833FEA-8583-11D1-B16A-00C0F0283628} 
               Key             =   "Open"
               Description     =   "Open Timeline"
               Object.ToolTipText     =   "Open An Existing Timeline File"
               ImageIndex      =   2
            EndProperty
            BeginProperty Button3 {66833FEA-8583-11D1-B16A-00C0F0283628} 
               Key             =   "Save"
               Description     =   "Save Timeline"
               Object.ToolTipText     =   "Save The Timeline"
               ImageIndex      =   3
            EndProperty
            BeginProperty Button4 {66833FEA-8583-11D1-B16A-00C0F0283628} 
               Style           =   3
            EndProperty
            BeginProperty Button5 {66833FEA-8583-11D1-B16A-00C0F0283628} 
               Key             =   "Rewind"
               Description     =   "btnRewind"
               Object.ToolTipText     =   "Rewind To Beginning Of TimeLine"
               ImageIndex      =   4
            EndProperty
            BeginProperty Button6 {66833FEA-8583-11D1-B16A-00C0F0283628} 
               Key             =   "SeekBackward"
               Description     =   "btnSeekBack"
               Object.ToolTipText     =   "Seek Backwards One Second At A Time"
               ImageIndex      =   5
            EndProperty
            BeginProperty Button7 {66833FEA-8583-11D1-B16A-00C0F0283628} 
               Key             =   "Play"
               Description     =   "Play"
               Object.ToolTipText     =   "Play Currently Loaded Timeline"
               ImageIndex      =   6
               Style           =   2
            EndProperty
            BeginProperty Button8 {66833FEA-8583-11D1-B16A-00C0F0283628} 
               Key             =   "Pause"
               Description     =   "Pause"
               Object.ToolTipText     =   "Pauses Playback"
               ImageIndex      =   7
               Style           =   2
            EndProperty
            BeginProperty Button9 {66833FEA-8583-11D1-B16A-00C0F0283628} 
               Key             =   "Stop"
               Description     =   "Stop"
               Object.ToolTipText     =   "Stops Playback"
               ImageIndex      =   8
               Style           =   2
            EndProperty
            BeginProperty Button10 {66833FEA-8583-11D1-B16A-00C0F0283628} 
               Key             =   "SeekForward"
               Description     =   "btnSeekFwd"
               Object.ToolTipText     =   "Seek Forward One Second At A Time"
               ImageIndex      =   9
            EndProperty
            BeginProperty Button11 {66833FEA-8583-11D1-B16A-00C0F0283628} 
               Key             =   "FastForward"
               Description     =   "btnffwd"
               Object.ToolTipText     =   "Go To End Of TimeLine"
               ImageIndex      =   10
            EndProperty
         EndProperty
         OLEDropMode     =   1
      End
   End
   Begin MSComctlLib.ListView lstViewInfo 
      Height          =   4745
      Left            =   2475
      TabIndex        =   2
      Top             =   450
      Width           =   6120
      _ExtentX        =   10795
      _ExtentY        =   8361
      View            =   3
      LabelWrap       =   -1  'True
      HideSelection   =   -1  'True
      OLEDropMode     =   1
      FlatScrollBar   =   -1  'True
      FullRowSelect   =   -1  'True
      HotTracking     =   -1  'True
      HoverSelection  =   -1  'True
      _Version        =   393217
      ForeColor       =   -2147483640
      BackColor       =   -2147483643
      Appearance      =   1
      OLEDropMode     =   1
      NumItems        =   0
   End
   Begin MSComctlLib.TreeView tvwSimpleTree 
      Height          =   4745
      Left            =   0
      TabIndex        =   1
      Top             =   450
      Width           =   2415
      _ExtentX        =   4260
      _ExtentY        =   8361
      _Version        =   393217
      HideSelection   =   0   'False
      Style           =   7
      FullRowSelect   =   -1  'True
      HotTracking     =   -1  'True
      ImageList       =   "ctrlTreeViewImageList"
      Appearance      =   1
      OLEDropMode     =   1
   End
   Begin MSComctlLib.ImageList ctrlMainToolbarImageList 
      Left            =   8685
      Top             =   480
      _ExtentX        =   1005
      _ExtentY        =   1005
      BackColor       =   -2147483643
      ImageWidth      =   16
      ImageHeight     =   15
      MaskColor       =   16711935
      _Version        =   393216
      BeginProperty Images {2C247F25-8591-11D1-B16A-00C0F0283628} 
         NumListImages   =   10
         BeginProperty ListImage1 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":210E
            Key             =   "new"
         EndProperty
         BeginProperty ListImage2 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":2430
            Key             =   "open"
         EndProperty
         BeginProperty ListImage3 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":2752
            Key             =   "save"
         EndProperty
         BeginProperty ListImage4 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":2A74
            Key             =   "rewind"
         EndProperty
         BeginProperty ListImage5 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":2D66
            Key             =   "seekbackward"
         EndProperty
         BeginProperty ListImage6 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":3058
            Key             =   "play"
         EndProperty
         BeginProperty ListImage7 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":334A
            Key             =   "pause"
         EndProperty
         BeginProperty ListImage8 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":363C
            Key             =   "stop"
         EndProperty
         BeginProperty ListImage9 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":392E
            Key             =   "seekforward"
         EndProperty
         BeginProperty ListImage10 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":3C20
            Key             =   "fastforward"
         EndProperty
      EndProperty
   End
   Begin MSComctlLib.StatusBar sbStatus 
      Align           =   2  'Align Bottom
      Height          =   345
      Left            =   0
      TabIndex        =   0
      Top             =   5220
      Width           =   8610
      _ExtentX        =   15187
      _ExtentY        =   609
      Style           =   1
      _Version        =   393216
      BeginProperty Panels {8E3867A5-8586-11D1-B16A-00C0F0283628} 
      EndProperty
      OLEDropMode     =   1
   End
   Begin MSComDlg.CommonDialog ctrlCommonDialog 
      Left            =   8700
      Top             =   1080
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin MSComctlLib.ImageList ctrlTreeViewImageList 
      Left            =   9900
      Top             =   480
      _ExtentX        =   1005
      _ExtentY        =   1005
      BackColor       =   -2147483643
      ImageWidth      =   16
      ImageHeight     =   16
      MaskColor       =   12632256
      _Version        =   393216
      BeginProperty Images {2C247F25-8591-11D1-B16A-00C0F0283628} 
         NumListImages   =   3
         BeginProperty ListImage1 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":3F12
            Key             =   ""
         EndProperty
         BeginProperty ListImage2 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":422C
            Key             =   ""
         EndProperty
         BeginProperty ListImage3 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmMain.frx":4546
            Key             =   ""
         EndProperty
      EndProperty
   End
   Begin VB.Menu mnuFile 
      Caption         =   "&File"
      NegotiatePosition=   1  'Left
      Begin VB.Menu mnuFileNew 
         Caption         =   "&New"
         Begin VB.Menu mnuFileNewTimeline 
            Caption         =   "Timeline"
         End
      End
      Begin VB.Menu mnuFileOpen 
         Caption         =   "&Open"
         Shortcut        =   ^O
      End
      Begin VB.Menu mnuFileSeptum 
         Caption         =   "-"
      End
      Begin VB.Menu mnuFileSaveAs 
         Caption         =   "Save &As"
      End
      Begin VB.Menu mnuFileSeptum0 
         Caption         =   "-"
      End
      Begin VB.Menu mnuFileExit 
         Caption         =   "&Exit"
      End
   End
   Begin VB.Menu mnuTimeline 
      Caption         =   "TimeLine"
      Visible         =   0   'False
      Begin VB.Menu mnuTimeLinePlay 
         Caption         =   "Play"
      End
      Begin VB.Menu mnuTimeLineStop 
         Caption         =   "Stop"
      End
      Begin VB.Menu mnuTimeLinePause 
         Caption         =   "Pause"
      End
      Begin VB.Menu mnuTimelineSeptum 
         Caption         =   "-"
      End
      Begin VB.Menu mnuTimelineExpand 
         Caption         =   "Expand"
      End
      Begin VB.Menu mnuTimelineCollapse 
         Caption         =   "Collapse"
      End
      Begin VB.Menu mnuTimelineSeptum2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuTimeLineEdit 
         Caption         =   "Edit"
      End
      Begin VB.Menu mnuTimeLineInsertGroup 
         Caption         =   "Insert Group"
      End
      Begin VB.Menu mnuTimelineSeptum3 
         Caption         =   "-"
      End
      Begin VB.Menu mnuTimeLineRenderTimeLine 
         Caption         =   "Render Timeline"
      End
      Begin VB.Menu mnuTimeLineClearRenderEngine 
         Caption         =   "ClearRenderEngine"
      End
   End
   Begin VB.Menu mnuTrack 
      Caption         =   "Track"
      Visible         =   0   'False
      Begin VB.Menu mnuTrackEdit 
         Caption         =   "Edit"
      End
      Begin VB.Menu mnuTrackDelete 
         Caption         =   "Delete"
      End
      Begin VB.Menu mnuTrackSeptum 
         Caption         =   "-"
      End
      Begin VB.Menu mnuTrackExpand 
         Caption         =   "Expand"
      End
      Begin VB.Menu mnuTrackCollapse 
         Caption         =   "Collapse"
      End
      Begin VB.Menu mnuTrackSeptum2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuTrackAddClip 
         Caption         =   "Add Clip"
      End
      Begin VB.Menu mnuTrackAddEffect 
         Caption         =   "Add Effect"
      End
      Begin VB.Menu mnuTrackAddTransition 
         Caption         =   "Add Transition"
      End
   End
   Begin VB.Menu mnuGroup 
      Caption         =   "group"
      Visible         =   0   'False
      Begin VB.Menu mnuGroupEdit 
         Caption         =   "Edit"
      End
      Begin VB.Menu mnuGroupDelete 
         Caption         =   "Delete"
      End
      Begin VB.Menu mnuGroupSeptum 
         Caption         =   "-"
      End
      Begin VB.Menu mnuGroupExpand 
         Caption         =   "Expand"
      End
      Begin VB.Menu mnuGroupCollapse 
         Caption         =   "Collapse"
      End
      Begin VB.Menu mnuGroupSeptum2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuGroupAddComp 
         Caption         =   "Add Composition"
      End
      Begin VB.Menu mnuGroupAddTrack 
         Caption         =   "Add Track"
      End
      Begin VB.Menu mnuGroupAddTransition 
         Caption         =   "Add Transition"
      End
      Begin VB.Menu mnuGroupAddEffect 
         Caption         =   "Add Effect"
      End
   End
   Begin VB.Menu mnuComp 
      Caption         =   "Comp"
      Visible         =   0   'False
      Begin VB.Menu mnuCompEdit 
         Caption         =   "Edit"
      End
      Begin VB.Menu mnuCompDelete 
         Caption         =   "Delete"
      End
      Begin VB.Menu mnuCompSeptum 
         Caption         =   "-"
      End
      Begin VB.Menu mnuCompExpand 
         Caption         =   "Expand"
      End
      Begin VB.Menu mnuCompCollapse 
         Caption         =   "Collapse"
      End
      Begin VB.Menu mnuCompSeptum2 
         Caption         =   "-"
      End
      Begin VB.Menu mnuCompAddTrack 
         Caption         =   "Add Track"
      End
      Begin VB.Menu mnuCompAddEffect 
         Caption         =   "Add Effect"
      End
      Begin VB.Menu mnuCompAddTransition 
         Caption         =   "Add Transition"
      End
      Begin VB.Menu mnuCompAddComp 
         Caption         =   "Add Composition"
      End
   End
   Begin VB.Menu mnuClip 
      Caption         =   "Clip"
      Visible         =   0   'False
      Begin VB.Menu mnuClipEdit 
         Caption         =   "Edit"
      End
      Begin VB.Menu mnuClipDelete 
         Caption         =   "Delete"
      End
      Begin VB.Menu mnuClipSeptum 
         Caption         =   "-"
      End
      Begin VB.Menu mnuClipExpand 
         Caption         =   "Expand"
      End
      Begin VB.Menu mnuClipCollapse 
         Caption         =   "Collapse"
      End
   End
   Begin VB.Menu mnuTrans 
      Caption         =   "Trans"
      Visible         =   0   'False
      Begin VB.Menu mnuTransEdit 
         Caption         =   "Edit"
      End
      Begin VB.Menu mnuTransDelete 
         Caption         =   "Delete"
      End
      Begin VB.Menu mnuTransSeptum 
         Caption         =   "-"
      End
      Begin VB.Menu mnuTransExpand 
         Caption         =   "Expand"
      End
      Begin VB.Menu mnuTransCollapse 
         Caption         =   "Collapse"
      End
   End
   Begin VB.Menu mnuEffect 
      Caption         =   "Effect"
      Visible         =   0   'False
      Begin VB.Menu mnuEffectEdit 
         Caption         =   "Edit"
      End
      Begin VB.Menu mnuEffectDelete 
         Caption         =   "Delete"
      End
      Begin VB.Menu mnuEffectSeptum 
         Caption         =   "-"
      End
      Begin VB.Menu mnuEffectExpand 
         Caption         =   "Expand"
      End
      Begin VB.Menu mnuEffectCollapse 
         Caption         =   "Collapse"
      End
   End
   Begin VB.Menu mnuHelp 
      Caption         =   "&Help"
      NegotiatePosition=   3  'Right
      Begin VB.Menu mnuHelpAbout 
         Caption         =   "&About"
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
'*       Copyright (C) 1999-2000 Microsoft Corporation.
'*       All rights reserved.
'*       This source code is only intended as a supplement to
'*       Microsoft Development Tools and/or SDK documentation.
'*       See these sources for detailed information regarding the
'*       Microsoft samples programs.
'*******************************************************************************
Option Explicit
Option Base 0
Option Compare Text

'module-level drag indicator
Private m_boolDrag As Boolean 'returns true if the user is moving the ide septum divided bar
' **************************************************************************************************************************************
' * PRIVATE INTERFACE- FORM EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: Form_Activate
            ' * procedure description:  Occurs when a form becomes the active window.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_Activate()
            On Local Error GoTo ErrLine
            Call AppActivate(App.Title, 0)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Form_GotFocus
            ' * procedure description:  Occurs when an object receives the focus.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_GotFocus()
            On Local Error GoTo ErrLine
            
            'set focus to the treeview
            tvwSimpleTree.SetFocus
            
            'if a node has not been selected set the selected node to root
            If tvwSimpleTree.Nodes.Count > 0 Then
               If Not tvwSimpleTree.SelectedItem Is Nothing Then _
                  Set tvwSimpleTree.SelectedItem = tvwSimpleTree.Nodes(1)
            End If
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
            Set gbl_colNormalEnum = New Collection
            Set gbl_objQuartzVB = New VBQuartzHelper
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
            
            'set the listview to report view
            'and append the column header(s)
            lstViewInfo.View = lvwReport
            lstViewInfo.ColumnHeaders.Add , "Parameter", _
                                                                "Parameter", (lstViewInfo.Width / 2) - 50
            lstViewInfo.ColumnHeaders.Add , "Value", "Value", (lstViewInfo.Width / 2) - 50
            
            'assign the width of the column header(s)
            If lstViewInfo.ColumnHeaders.Count > 0 Then
                  lstViewInfo.ColumnHeaders(1).Width = lstViewInfo.Width * 0.2
                  If lstViewInfo.ColumnHeaders(2).Width <> _
                     (lstViewInfo.Width - lstViewInfo.ColumnHeaders(1).Width) Then _
                     lstViewInfo.ColumnHeaders(2).Width = _
                     (lstViewInfo.Width - lstViewInfo.ColumnHeaders(1).Width)
            End If
            
            'ensure the listview has zero items
            If lstViewInfo.ListItems.Count <> 0 Then _
                lstViewInfo.ListItems.Clear
            
           'update the text on the statusbar
           sbStatus.SimpleText = "Ready"
           
           'update the button(s)
           With tbMain.Buttons
              .Item("New").Enabled = True
              .Item("Open").Enabled = True
              .Item("Save").Enabled = False
              .Item("Play").Enabled = False
              .Item("Pause").Enabled = False
              .Item("Stop").Enabled = False
              .Item("Rewind").Enabled = False
              .Item("FastForward").Enabled = False
              .Item("SeekForward").Enabled = False
              .Item("SeekBackward").Enabled = False
           End With
           
            'update the state on the popup context menu
            mnuFileSaveAs.Enabled = False
            mnuTimeLinePlay.Enabled = False
            mnuTimeLineStop.Enabled = False
            mnuTimeLinePause.Enabled = False
            mnuTimeLineRenderTimeLine.Enabled = False
            mnuTimeLineClearRenderEngine.Enabled = False
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Form_MouseDown
            ' * procedure description:  Occurs when the user presses the mouse button while an object has the focus.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            
            'check if we are in the septum
            If X > Me.Controls("tvwSimpleTree").Width And X < Me.Controls("lstViewInfo").Left Then
               If Y > Me.Controls("tbMain").Height And Y < Me.Controls("sbStatus").Top Then
                  'we are hovering over the septum bar..
                  If Button = 1 Then
                     m_boolDrag = True
                     If Me.BackColor <> vbBlack Then Me.BackColor = vbBlack
                  End If
                  Exit Sub
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Form_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            
            'check if we are in the septum; and proceed to set the mousepointer
            If X > Me.Controls("tvwSimpleTree").Width And X < Me.Controls("lstViewInfo").Left Then
               If Y > Me.Controls("tbMain").Height And Y < Me.Controls("sbStatus").Top Then
                  'we are hovering over the septum bar..
                  Me.MousePointer = 9
               Else
                  'otherwise reset
                  Me.MousePointer = vbDefault
                  Me.BackColor = &H8000000F
               End If
            End If
            
            'if we are in 'drag mode' then resize the control(s)
            If m_boolDrag = True And Button = 1 Then
               If X > 65 And X < Me.ScaleWidth - 65 Then Me.Controls("tvwSimpleTree").Width = X
               Call Form_Resize
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Form_MouseUp
            ' * procedure description:  Occurs when the user releases the mouse button while an object has the focus.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            
            'check if we are in the septum
            If X > Me.Controls("tvwSimpleTree").Width And X < Me.Controls("lstViewInfo").Left Then
               If Y > Me.Controls("tbMain").Height And Y < Me.Controls("sbStatus").Top Then
                  'we are hovering over the septum bar..
                  If Button = 1 Then
                     m_boolDrag = False
                     If Me.BackColor = vbBlack Then Me.BackColor = &H8000000F
                  End If
                  Exit Sub
               End If
            Else
               'reset the mousepointer & septum color
               If Me.BackColor = vbBlack Then Me.BackColor = &H8000000F
               If Me.MousePointer <> vbDefault Then Me.MousePointer = vbDefault
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Form_OLEDragDrop
            ' * procedure description:   Occurs when data is dropped onto the control via an OLE drag/drop operation, and OLEDropMode is set to manual.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_OLEDragDrop(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            
            'pass to the application drag drop handler
            Call AppOLEDragDrop(Data, Effect, Button, Shift, X, Y)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Form_OLEDragOver
            ' * procedure description:  Occurs when the mouse is moved over the control during an OLE drag/drop operation, if its OLEDropMode property is set to manual.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_OLEDragOver(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single, State As Integer)
            On Local Error GoTo ErrLine
            
            'pass to the application drag over handler
            Call AppOLEDragOver(Data, Effect, Button, Shift, X, Y, State)
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
            ' * procedure name: Form_Resize
            ' * procedure description:  Occurs when a form is first displayed or the size of an object changes.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_Resize()
            Dim Septum As Single
            Dim InternalTop As Single
            Dim InternalHeight As Single
            On Local Error GoTo ErrLine
            
            Septum = 65  'assign the septum a specified width
               
            If Me.Controls("ctrlCoolBar").Visible = True And Controls("sbStatus").Visible = True Then
                 'position the coolbar
                 If Me.Controls("ctrlCoolBar").Top <> Me.ScaleTop Then Me.Controls("ctrlCoolBar").Top = Me.ScaleTop
                 If Me.Controls("ctrlCoolBar").Left <> Me.ScaleLeft Then Me.Controls("ctrlCoolBar").Left = Me.ScaleLeft
                 If Me.Controls("ctrlCoolBar").Width <> Me.ScaleWidth Then Me.Controls("ctrlCoolBar").Width = Me.ScaleWidth
                 'get the internal measurement(s)
                 InternalTop = Me.ScaleTop + Me.Controls("ctrlCoolBar").Height
                 InternalHeight = Me.ScaleHeight - (Me.Controls("ctrlCoolBar").Height + Me.Controls("sbStatus").Height)
                 'verify that the measurement(s) are valid
                 If InternalTop < 0 Then InternalTop = 0
                 If InternalHeight < 0 Then InternalHeight = 0
                 
            ElseIf Me.Controls("ctrlCoolBar").Visible = True And Controls("sbStatus").Visible = False Then
                'position the coolbar
                 If Me.Controls("ctrlCoolBar").Top <> Me.ScaleTop Then Me.Controls("ctrlCoolBar").Top = Me.ScaleTop
                 If Me.Controls("ctrlCoolBar").Left <> Me.ScaleLeft Then Me.Controls("ctrlCoolBar").Left = Me.ScaleLeft
                 If Me.Controls("ctrlCoolBar").Width <> Me.ScaleWidth Then Me.Controls("ctrlCoolBar").Width = Me.ScaleWidth
                 'get the internal measurement(s)
                 InternalTop = Me.ScaleTop + Me.Controls("ctrlCoolBar").Height
                 InternalHeight = Me.ScaleHeight - (Me.Controls("ctrlCoolBar").Height)
                 'verify that the measurement(s) are valid
                 If InternalTop < 0 Then InternalTop = 0
            
            ElseIf Me.Controls("ctrlCoolBar").Visible = False And Me.Controls("sbStatus").Visible = True Then
                 'get the internal measurement(s)
                 InternalTop = Me.ScaleTop
                 InternalHeight = Me.ScaleHeight - (Me.Controls("sbStatus").Height)
                 'verify that the measurement(s) are valid
                 If InternalTop < 0 Then InternalTop = 0
                 If InternalHeight < 0 Then InternalHeight = 0
            
            ElseIf Me.Controls("ctrlCoolBar").Visible = False And Controls("sbStatus").Visible = False Then
                 'get the internal measurement(s)
                 InternalTop = Me.ScaleTop
                 InternalHeight = Me.ScaleHeight
                 'verify that the measurement(s) are valid
                 If InternalTop < 0 Then InternalTop = 0
                 If InternalHeight < 0 Then InternalHeight = 0
            End If
            
               
            If Me.Controls("tvwSimpleTree").Visible = True And Controls("lstViewInfo").Visible = False Then
                'position the treeview
                If Me.Controls("tvwSimpleTree").Top <> InternalTop Then Me.Controls("tvwSimpleTree").Top = InternalTop
                If Me.Controls("tvwSimpleTree").Left <> 0 Then Me.Controls("tvwSimpleTree").Left = 0
                If Me.Controls("tvwSimpleTree").Height <> InternalHeight Then Me.Controls("tvwSimpleTree").Height = InternalHeight
                'position the listview
                If Me.Controls("lstViewInfo").Top <> InternalTop Then Me.Controls("lstViewInfo").Top = InternalTop
                If (Me.Controls("lstViewInfo").Left + Me.Controls("tvwSimpleTree").Width) + Septum > 0 Then Me.Controls("lstViewInfo").Left = (Me.Controls("tvwSimpleTree").Left + Me.Controls("tvwSimpleTree").Width) + Septum
                If Me.Controls("lstViewInfo").Height <> InternalHeight Then Me.Controls("lstViewInfo").Height = InternalHeight
                If (Me.ScaleWidth - Me.Controls("lstViewInfo").Width) - Septum > 0 Then Me.Controls("lstViewInfo").Width = (Me.ScaleWidth - Me.Controls("tvwSimpleTree").Width) - Septum
            
            ElseIf Me.Controls("tvwSimpleTree").Visible = True And Controls("lstViewInfo").Visible = True Then
               'position the treeview
                If Me.Controls("tvwSimpleTree").Top <> InternalTop Then Me.Controls("tvwSimpleTree").Top = InternalTop
                If Me.Controls("tvwSimpleTree").Left <> 0 Then Me.Controls("tvwSimpleTree").Left = 0
                If Me.Controls("tvwSimpleTree").Height <> InternalHeight Then Me.Controls("tvwSimpleTree").Height = InternalHeight
               'position the listview
                If Me.Controls("lstViewInfo").Top <> InternalTop Then Me.Controls("lstViewInfo").Top = InternalTop
                If (Me.Controls("lstViewInfo").Left + Me.Controls("tvwSimpleTree").Width) + Septum > 0 Then Me.Controls("lstViewInfo").Left = (Me.Controls("tvwSimpleTree").Left + Me.Controls("tvwSimpleTree").Width) + Septum
                If Me.Controls("lstViewInfo").Height <> InternalHeight Then Me.Controls("lstViewInfo").Height = InternalHeight
                If (Me.ScaleWidth - Me.Controls("lstViewInfo").Width) - Septum > 0 Then Me.Controls("lstViewInfo").Width = (Me.ScaleWidth - Me.Controls("tvwSimpleTree").Width) - Septum
               
            ElseIf Me.Controls("tvwSimpleTree").Visible = False And Controls("lstViewInfo").Visible = True Then
                'position the treeview
                If Me.Controls("lstViewInfo").Top <> InternalTop Then Me.Controls("lstViewInfo").Top = InternalTop
                If Me.Controls("lstViewInfo").Left <> 0 Then Me.Controls("lstViewInfo").Left = 0
                If Me.Controls("lstViewInfo").Height <> InternalHeight Then Me.Controls("lstViewInfo").Height = InternalHeight
            
            ElseIf Me.Controls("tvwSimpleTree").Visible = False And Controls("lstViewInfo").Visible = False Then
                Exit Sub
            End If
            
            'ensure listview's column headers get resized as well..
            If lstViewInfo.Visible Then
               If lstViewInfo.ColumnHeaders.Count > 0 Then
                  If lstViewInfo.ColumnHeaders(2).Width <> (lstViewInfo.Width - lstViewInfo.ColumnHeaders(1).Width) Then _
                     lstViewInfo.ColumnHeaders(2).Width = (lstViewInfo.Width - lstViewInfo.ColumnHeaders(1).Width)
               End If
            End If
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
            
            'dereference & clean-up module-level data
            If Not gbl_objQuartzVB Is Nothing Then Set gbl_objQuartzVB = Nothing
            If Not gbl_colNormalEnum Is Nothing Then Set gbl_colNormalEnum = Nothing
            
            'dereference & clean-up application-level data
            If Not gbl_objTimeline Is Nothing Then Set gbl_objTimeline = Nothing
            If Not gbl_objFilterGraph Is Nothing Then Set gbl_objFilterGraph = Nothing
            If Not gbl_objDexterObject Is Nothing Then Set gbl_objDexterObject = Nothing
            If Not gbl_objRenderEngine Is Nothing Then Set gbl_objRenderEngine = Nothing
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
            With Me
               .Move 0 - (Screen.Width * 8), 0 - (Screen.Height * 8): .Visible = False
            End With
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub



' **************************************************************************************************************************************
' * PRIVATE INTERFACE- MENU EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: mnuClip_Click
            ' * procedure description:  Occurs when the clip popup context menu is invoked
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuClip_Click()
            On Local Error GoTo ErrLine
            
            'set menu state
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Children = 0 Then
                  If mnuClipExpand.Enabled = True Then mnuClipExpand.Enabled = False
                  If mnuClipCollapse.Enabled = True Then mnuClipCollapse.Enabled = False
               ElseIf tvwSimpleTree.SelectedItem.Expanded = True Then
                  If mnuClipExpand.Enabled = True Then mnuClipExpand.Enabled = False
                  If mnuClipCollapse.Enabled = False Then mnuClipCollapse.Enabled = True
               ElseIf tvwSimpleTree.SelectedItem.Expanded = False Then
                  If mnuClipExpand.Enabled = False Then mnuClipExpand.Enabled = True
                  If mnuClipCollapse.Enabled = True Then mnuClipCollapse.Enabled = False
               Else
                  If mnuClipExpand.Enabled = True Then mnuClipExpand.Enabled = False
                  If mnuClipCollapse.Enabled = True Then mnuClipCollapse.Enabled = False
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuClipCollapse_Click
            ' * procedure description:  Occurs when the clip popup context menu's 'Collapse' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuClipCollapse_Click()
            On Local Error GoTo ErrLine
            
            'collapse the treeview
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Expanded = True Then
                  tvwSimpleTree.SelectedItem.Expanded = False
                  If mnuClipExpand.Enabled = False Then mnuClipExpand.Enabled = True
                  If mnuClipCollapse.Enabled = True Then mnuClipCollapse.Enabled = False
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuClipExpand_Click
            ' * procedure description:  Occurs when the clip popup context menu's 'Expand' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuClipExpand_Click()
            On Local Error GoTo ErrLine
            
            'expand the treeview
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Expanded = False Then
                  tvwSimpleTree.SelectedItem.Expanded = True
                  If mnuClipExpand.Enabled = True Then mnuClipExpand.Enabled = False
                  If mnuClipCollapse.Enabled = False Then mnuClipCollapse.Enabled = True
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub


            ' ******************************************************************************************************************************
            ' * procedure name: mnuClipDelete_Click
            ' * procedure description:  Occurs when the clip popup context menu's 'Delete' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuClipDelete_Click()
            Dim objSource As AMTimelineSrc
            On Local Error GoTo ErrLine
            
            'obtain the source from the global timeline object
            'which is reset when the user clicks a node on the tree
            If Not gbl_objDexterObject Is Nothing Then
               Set objSource = gbl_objDexterObject
            Else: Exit Sub
            End If
            
            'remove the item
            Call gbl_objDexterObject.RemoveAll
            Call gbl_colNormalEnum.Remove(tvwSimpleTree.SelectedItem.Key)
            Call tvwSimpleTree.Nodes.Remove(tvwSimpleTree.SelectedItem.Index)
            If Not gbl_objDexterObject Is Nothing Then Set gbl_objDexterObject = Nothing
            
            'clean-up & dereference
            If Not objSource Is Nothing Then Set objSource = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuClipEdit_Click
            ' * procedure description:  Occurs when the clip popup context menu's 'Edit' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuClipEdit_Click()
            Dim nMuted As Long
            Dim dblTStart As Double
            Dim dblTStop As Double
            Dim dblMStart As Double
            Dim dblMStop As Double
            Dim nStretchMode As Long
            Dim nStreamNumber As Long
            Dim dblDefaultFPS As Double
            Dim dblMediaLength As Double
            Dim bstrClipMediaName As String
            Dim objSourceClip As AMTimelineSrc
            On Local Error GoTo ErrLine
            
            'obtain the source from the global timeline object
            'which is reset when the user clicks a node on the tree
            If Not gbl_objDexterObject Is Nothing Then
               Set objSourceClip = gbl_objDexterObject
            Else: Exit Sub
            End If
            
            'obtain existing group information
            If Not objSourceClip Is Nothing Then
               With objSourceClip
                  'get media name
                  bstrClipMediaName = .GetMediaName
                  'get dexter start & stop times
                  Call gbl_objDexterObject.GetStartStop2(dblTStart, dblTStop)
                  'get media start & stop times
                  Call .GetMediaTimes2(dblMStart, dblMStop)
                  'get medialength
                  dblMediaLength = (dblMStop - dblMStart)
                  'get stream number
                  Call .GetStreamNumber(nStreamNumber)
                  'get default fps
                  Call .GetDefaultFPS(dblDefaultFPS)
                  'get stretch mode
                  Call .GetStretchMode(nStretchMode)
                  'get muted
                  Call gbl_objDexterObject.GetMuted(nMuted)
               End With
            End If
            
            'hide the dialog
            Load frmClip
            frmClip.Visible = False
            
            'update the form with the existing information
            With frmClip
               .txtMediaName = CStr(bstrClipMediaName)
               .txtMStart = CStr(dblMStart)
               .txtMStop = CStr(dblMStop)
               .txtTStart = CStr(dblTStart)
               .txtTStop = CStr(dblTStop)
               .txtMediaLength = CStr(dblMediaLength)
               .txtStreamNumber = CStr(nStreamNumber)
               .txtFPS = CStr(dblDefaultFPS)
               .txtStretchMode = CStr(nStretchMode)
               .txtMuted = CStr(nMuted)
            End With
            
            'display the dialog
            frmClip.Caption = "Edit Source"
            frmClip.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do Until frmClip.Visible = False: DoEvents
            Loop
            'verify unload mode
            If frmClip.UnloadMode = 1 Then
               Unload frmClip: Set frmClip = Nothing: Exit Sub
            End If
            
            'update the timeline info given the new information
            With objSourceClip
               'set the media name
               If frmClip.txtMediaName.Text <> vbNullString Then
                  Call .SetMediaName(CStr(frmClip.txtMediaName.Text))
               End If
               'set the media times
               If IsNumeric(frmClip.txtMStart.Text) Then
                  Call .SetMediaTimes2(CDbl(frmClip.txtMStart.Text), CDbl(frmClip.txtMStop.Text))
               End If
               'set the media length
               If IsNumeric(frmClip.txtMStop.Text) Then
                  Call .SetMediaLength2(CDbl(frmClip.txtTStop.Text) - CDbl(frmClip.txtTStart.Text))
               End If
               'set stream number
               If IsNumeric(frmClip.txtStreamNumber.Text) Then
                  Call .SetStreamNumber(CLng(frmClip.txtStreamNumber.Text))
               End If
               'set default frames per second
               If IsNumeric(frmClip.txtFPS.Text) Then
                  Call .SetDefaultFPS(CDbl(frmClip.txtFPS.Text))
               End If
               'set stretch mode
               If IsNumeric(frmClip.txtStretchMode.Text) Then
                  Call .SetStretchMode(CLng(frmClip.txtStretchMode.Text))
               End If
               'set muted state
               If IsNumeric(frmClip.txtMuted.Text) Then
                  Call gbl_objDexterObject.SetMuted(CLng(frmClip.txtMuted.Text))
               End If
            End With
            
            'refresh the ide
            Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            
            'destroy the dialog
            Unload frmClip: Set frmClip = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            

            ' ******************************************************************************************************************************
            ' * procedure name: mnuComp_Click
            ' * procedure description:  Occurs when the composition popup context menu is invoked
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuComp_Click()
            On Local Error GoTo ErrLine
            
            'set menu state
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Children = 0 Then
                  If mnuCompExpand.Enabled = True Then mnuCompExpand.Enabled = False
                  If mnuCompCollapse.Enabled = True Then mnuCompCollapse.Enabled = False
               ElseIf tvwSimpleTree.SelectedItem.Expanded = True Then
                  If mnuCompExpand.Enabled = True Then mnuCompExpand.Enabled = False
                  If mnuCompCollapse.Enabled = False Then mnuCompCollapse.Enabled = True
               ElseIf tvwSimpleTree.SelectedItem.Expanded = False Then
                  If mnuCompExpand.Enabled = False Then mnuCompExpand.Enabled = True
                  If mnuCompCollapse.Enabled = True Then mnuCompCollapse.Enabled = False
               Else
                  If mnuCompExpand.Enabled = True Then mnuCompExpand.Enabled = False
                  If mnuCompCollapse.Enabled = True Then mnuCompCollapse.Enabled = False
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuCompCollapse_Click
            ' * procedure description:  Occurs when the composition popup context menu's 'Collapse' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuCompCollapse_Click()
            On Local Error GoTo ErrLine
            
            'collapse the treeview
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Expanded = True Then
                  tvwSimpleTree.SelectedItem.Expanded = False
                  If mnuCompExpand.Enabled = False Then mnuCompExpand.Enabled = True
                  If mnuCompCollapse.Enabled = True Then mnuCompCollapse.Enabled = False
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuCompExpand_Click
            ' * procedure description:  Occurs when the composition popup context menu's 'Expand' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuCompExpand_Click()
            On Local Error GoTo ErrLine
            
            'expand the treeview
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Expanded = False Then
                  tvwSimpleTree.SelectedItem.Expanded = True
                  If mnuCompExpand.Enabled = True Then mnuCompExpand.Enabled = False
                  If mnuCompCollapse.Enabled = False Then mnuCompCollapse.Enabled = True
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuCompAddComp_Click
            ' * procedure description:  Occurs when the composition popup context menu's 'Add Composition' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuCompAddComp_Click()
            Dim nPriority As Long
            Dim dblStartTime As Double
            Dim dblStopTime As Double
            Dim objNewComposite As AMTimelineComp
            On Local Error GoTo ErrLine
            
            'display the dialog
            frmComp.Caption = "Add Composition"
            frmComp.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do Until frmComp.Visible = False: DoEvents
            Loop
            'verify unload mode
            If frmComp.UnloadMode = 1 Then
               Unload frmComp: Set frmComp = Nothing: Exit Sub
            End If
            
            'query the dialog for user input
            With frmComp
               nPriority = CLng(.txtPriority.Text)
               dblStartTime = CDbl(.txtStartTime.Text)
               dblStopTime = CDbl(.txtStopTime.Text)
            End With
            
            'insert the composite into the timeline
            If Not gbl_objTimeline Is Nothing Then
               Set objNewComposite = CreateComposite(gbl_objTimeline)
               If Not objNewComposite Is Nothing Then Call InsertComposite(objNewComposite, gbl_objDexterObject)
               If Not gbl_objDexterObject Is Nothing Then Call gbl_objDexterObject.SetStartStop2(dblStartTime, dblStopTime)
               'refresh the ide
               Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            End If
            
            'unload the dialog
            Unload frmComp: Set frmComp = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuCompAddEffect_Click
            ' * procedure description:  Occurs when the composition popup context menu's 'Add Effect' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuCompAddEffect_Click()
            Dim nEffectPriority As Long
            Dim bstrEffectGUID As String
            Dim dblEffectStopTime As Double
            Dim dblEffectStartTime As Double
            Dim objNewEffect As AMTimelineEffect
            Dim objTimelineObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
            'display the insert effect dialog
            frmEffect.Caption = "Add Effect"
            frmEffect.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do: DoEvents
            If frmEffect.Visible = False Then Exit Do
            Loop
            'verify unload mode
            If frmEffect.UnloadMode = 1 Then
               Unload frmEffect: Set frmEffect = Nothing: Exit Sub
            End If
            
            'query the dialog information
            With frmEffect
               If IsNumeric(.txtStartTime.Text) Then
                  dblEffectStartTime = CDbl(.txtStartTime.Text)
               End If
               If IsNumeric(.txtStopTime.Text) Then
                  dblEffectStopTime = CDbl(.txtStopTime.Text)
               End If
               If IsNumeric(.txtPriority.Text) Then
                  nEffectPriority = CLng(.txtPriority.Text)
               End If
               If .cmbEffect.Text <> vbNullString Then
                  bstrEffectGUID = CStr(.cmbEffect.Text)
               End If
            End With
            
            'insert the effect into the timeline
            If Not gbl_objTimeline Is Nothing Then
               Set objNewEffect = CreateEffect(gbl_objTimeline)
               If Not objNewEffect Is Nothing Then Call InsertEffect(objNewEffect, gbl_objDexterObject, EffectFriendlyNameToCLSID(bstrEffectGUID), dblEffectStartTime, dblEffectStopTime)
               'refresh the ide
               Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            End If
            
            'destroy the dialog
            Unload frmEffect: Set frmEffect = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuCompAddTrack_Click
            ' * procedure description:  Occurs when the composition popup context menu's 'Add Track' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuCompAddTrack_Click()
            Dim nBlank As Long
            Dim nMuted As Long
            Dim nPriority As Long
            Dim dblStartTime As Double
            Dim dblStopTime As Double
            Dim objNewTrack As AMTimelineTrack
            On Local Error GoTo ErrLine
            
            'display the dialog
            frmTrack.Caption = "Add Track"
            frmTrack.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do Until frmTrack.Visible = False: DoEvents
            Loop
            'verify unload mode
            If frmTrack.UnloadMode = 1 Then
               Unload frmTrack: Set frmTrack = Nothing: Exit Sub
            End If
            
            'obtain the user input from the dialog
            With frmTrack
               If IsNumeric(.txtBlank.Text) Then
                  nBlank = CLng(.txtBlank.Text)
               End If
               If IsNumeric(.txtMuted.Text) Then
                  nMuted = CLng(.txtMuted.Text)
               End If
               If IsNumeric(.txtMuted.Text) Then
                  nPriority = CLng(.txtMuted.Text)
               End If
               If IsNumeric(.txtStartTime.Text) Then
                  dblStartTime = CDbl(.txtStartTime.Text)
               End If
               If IsNumeric(.txtStopTime.Text) Then
                  dblStopTime = CDbl(.txtStopTime.Text)
               End If
            End With
            
            'insert the track into the timeline
            If Not gbl_objTimeline Is Nothing Then
               Set objNewTrack = CreateTrack(gbl_objTimeline)
               If Not objNewTrack Is Nothing Then Call InsertTrack(objNewTrack, gbl_objDexterObject, nPriority)
               'refresh the ide
               Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            End If
            
            'destroy dialog
            Unload frmTrack: Set frmTrack = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuCompAddTransition_Click
            ' * procedure description:  Occurs when the composition popup context menu's 'Add Transition' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuCompAddTransition_Click()
            Dim nCutpoint As Long
            Dim nCutsOnly As Long
            Dim nSwapInputs As Long
            Dim bstrTransition As String
            Dim dblStartTime As Double
            Dim dblStopTime As Double
            Dim objNewTransition As AMTimelineTrans
            On Local Error GoTo ErrLine
            
            'display the dialog
            frmTransitions.Caption = "Add Transition"
            frmTransitions.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do Until frmTransitions.Visible = False: DoEvents
            Loop
            'verify unload mode
            If frmTransitions.UnloadMode = 1 Then
               Unload frmTransitions: Set frmTransitions = Nothing: Exit Sub
            End If
            
            'obtain user input
            With frmTransitions
               If IsNumeric(.txtCutpoint.Text) Then
                  nCutpoint = CLng(.txtCutpoint.Text)
               End If
               If IsNumeric(.txtCutsOnly.Text) Then
                  nCutsOnly = CLng(.txtCutsOnly.Text)
               End If
               If IsNumeric(.txtSwapInputs.Text) Then
                  nSwapInputs = CLng(.txtSwapInputs.Text)
               End If
               If IsNumeric(.txtStartTime.Text) Then
                  dblStartTime = CDbl(.txtStartTime.Text)
               End If
               If IsNumeric(.txtStopTime.Text) Then
                  dblStopTime = CDbl(.txtStopTime.Text)
               End If
               If .cmbTransition.Text <> vbNullString Then
                  bstrTransition = CStr(.cmbTransition.Text)
               End If
            End With
            
            'insert the transition into the timeline
            If Not gbl_objTimeline Is Nothing Then
               Set objNewTransition = CreateTransition(gbl_objTimeline)
               If Not objNewTransition Is Nothing Then Call InsertTransition(objNewTransition, gbl_objDexterObject, TransitionFriendlyNameToCLSID(bstrTransition), dblStartTime, dblStopTime)
               'refresh the ide
               Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            End If
            
            'destroy the dialog
            Unload frmTransitions: Set frmTransitions = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuCompDelete_Click
            ' * procedure description:  Occurs when the composition popup context menu's 'Delete' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuCompDelete_Click()
            Dim objComposition As AMTimelineComp
            On Local Error GoTo ErrLine
            
            'obtain the composition from the global timeline object
            'which is reset when the user clicks a node on the tree
            If Not gbl_objDexterObject Is Nothing Then
               Set objComposition = gbl_objDexterObject
            Else: Exit Sub
            End If
            
            'remove the item
            Call gbl_objDexterObject.RemoveAll
            Call gbl_colNormalEnum.Remove(tvwSimpleTree.SelectedItem.Key)
            Call tvwSimpleTree.Nodes.Remove(tvwSimpleTree.SelectedItem.Index)
            If Not gbl_objDexterObject Is Nothing Then Set gbl_objDexterObject = Nothing
            
            'clean-up & dereference
            If Not objComposition Is Nothing Then Set objComposition = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuCompEdit_Click
            ' * procedure description:  Occurs when the composition popup context menu's 'Edit' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuCompEdit_Click()
            Dim nPriority As Long
            Dim dblStopTime As Double
            Dim dblStartTime As Double
            Dim objComp As AMTimelineComp
            On Local Error GoTo ErrLine
            
            'obtain the composition from the global timeline object
            'which is reset when the user clicks a node on the tree
            If Not gbl_objDexterObject Is Nothing Then
               Set objComp = gbl_objDexterObject
            Else: Exit Sub
            End If
            
            'obtain existing group information
            If Not objComp Is Nothing Then
                  'get start /stoptime
                  Call gbl_objDexterObject.GetStartStop2(dblStartTime, dblStopTime)
            End If
            
            'hide the dialog
            Load frmComp
            frmComp.Visible = False
            
            'update the form with the existing information
            With frmComp
               .txtPriority = 0  'methodology not available
               .txtStartTime = CStr(dblStartTime)
               .txtStopTime = CStr(dblStopTime)
            End With
            
            'display the dialog
            frmComp.Caption = "Edit Composition"
            frmComp.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do Until frmComp.Visible = False: DoEvents
            Loop
            'verify unload mode
            If frmComp.UnloadMode = 1 Then
               Unload frmComp: Set frmComp = Nothing: Exit Sub
            End If
            
            'update the timeline info given the new information
            If Not gbl_objDexterObject Is Nothing Then
               'set the media times
               If IsNumeric(frmComp.txtStopTime.Text) And IsNumeric(frmComp.txtStartTime.Text) Then
                  Call gbl_objDexterObject.SetStartStop2(CDbl(frmComp.txtStopTime.Text), CDbl(frmComp.txtStartTime.Text))
               End If
            End If
            
            'refresh the ide
            Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            
            'destroy the dialog
            Unload frmClip: Set frmClip = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuEffect_Click
            ' * procedure description:  Occurs when the effect popup context menu is invoked
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuEffect_Click()
            On Local Error GoTo ErrLine
            
            'set menu state
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Children = 0 Then
                  If mnuEffectExpand.Enabled = True Then mnuEffectExpand.Enabled = False
                  If mnuEffectCollapse.Enabled = True Then mnuEffectCollapse.Enabled = False
               ElseIf tvwSimpleTree.SelectedItem.Expanded = True Then
                  If mnuEffectExpand.Enabled = True Then mnuEffectExpand.Enabled = False
                  If mnuEffectCollapse.Enabled = False Then mnuEffectCollapse.Enabled = True
               ElseIf tvwSimpleTree.SelectedItem.Expanded = False Then
                  If mnuEffectExpand.Enabled = False Then mnuEffectExpand.Enabled = True
                  If mnuEffectCollapse.Enabled = True Then mnuEffectCollapse.Enabled = False
               Else
                  If mnuEffectExpand.Enabled = True Then mnuEffectExpand.Enabled = False
                  If mnuEffectCollapse.Enabled = True Then mnuEffectCollapse.Enabled = False
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuEffectCollapse_Click
            ' * procedure description:  Occurs when the effect popup context menu's 'Collapse' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuEffectCollapse_Click()
            On Local Error GoTo ErrLine
            
            'collapse the treeview
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Expanded = True Then
                  tvwSimpleTree.SelectedItem.Expanded = False
                  If mnuEffectExpand.Enabled = False Then mnuEffectExpand.Enabled = True
                  If mnuEffectCollapse.Enabled = True Then mnuEffectCollapse.Enabled = False
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuEffectExpand_Click
            ' * procedure description:  Occurs when the effect popup context menu's 'Expand' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuEffectExpand_Click()
            On Local Error GoTo ErrLine
            
            'expand the treeview
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Expanded = False Then
                  tvwSimpleTree.SelectedItem.Expanded = True
                  If mnuEffectExpand.Enabled = True Then mnuEffectExpand.Enabled = False
                  If mnuEffectCollapse.Enabled = False Then mnuEffectCollapse.Enabled = True
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuEffectDelete_Click
            ' * procedure description:  Occurs when the effect popup context menu's 'Delete' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuEffectDelete_Click()
            Dim objEffect As AMTimelineEffect
            On Local Error GoTo ErrLine
            
            'obtain the effect from the global timeline object
            'which is reset when the user clicks a node on the tree
            If Not gbl_objDexterObject Is Nothing Then
               Set objEffect = gbl_objDexterObject
            Else: Exit Sub
            End If
            
            'remove the item
            Call gbl_objDexterObject.RemoveAll
            Call gbl_colNormalEnum.Remove(tvwSimpleTree.SelectedItem.Key)
            Call tvwSimpleTree.Nodes.Remove(tvwSimpleTree.SelectedItem.Index)
            If Not gbl_objDexterObject Is Nothing Then Set gbl_objDexterObject = Nothing
            
            'clean-up & dereference
            If Not objEffect Is Nothing Then Set objEffect = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuEffectEdit_Click
            ' * procedure description:  Occurs when the effect popup context menu's 'Edit' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuEffectEdit_Click()
            Dim nPriority As Long
            Dim dblStopTime As Double
            Dim dblStartTime As Double
            Dim bstrEffectCLSID As String
            Dim objEffect As AMTimelineEffect
            On Local Error GoTo ErrLine
            
            'obtain the effect from the global timeline object
            'which is reset when the user clicks a node on the tree
            If Not gbl_objDexterObject Is Nothing Then
               Set objEffect = gbl_objDexterObject
            Else: Exit Sub
            End If
            
            'obtain existing group information
            If Not objEffect Is Nothing Then
               With objEffect
                  'get priority
                  Call objEffect.EffectGetPriority(nPriority)
                  'get start /stoptime
                  Call gbl_objDexterObject.GetStartStop2(dblStartTime, dblStopTime)
                  'get effect guid
                  bstrEffectCLSID = gbl_objDexterObject.GetSubObjectGUIDB
               End With
            End If
            
            'hide the dialog
            Load frmEffect
            frmEffect.Visible = False
            
            'update the form with the existing information
            With frmEffect
               .txtPriority = 0  'methodology not available
               .txtStartTime = CStr(dblStartTime)
               .txtStopTime = CStr(dblStopTime)
               .cmbEffect.Text = CStr(EffectCLSIDToFriendlyName(bstrEffectCLSID))
            End With
            
            'display the dialog
            frmEffect.Caption = "Edit Composition"
            frmEffect.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do Until frmEffect.Visible = False: DoEvents
            Loop
            'verify unload mode
            If frmEffect.UnloadMode = 1 Then
               Unload frmEffect: Set frmEffect = Nothing: Exit Sub
            End If
            
            'query the dialog information
            With frmEffect
               If IsNumeric(.txtStartTime.Text) Then
                  dblStartTime = CDbl(.txtStartTime.Text)
               End If
               If IsNumeric(.txtStopTime.Text) Then
                  dblStopTime = CDbl(.txtStopTime.Text)
               End If
               If IsNumeric(.txtPriority.Text) Then
                  nPriority = CLng(.txtPriority.Text)
               End If
               If .cmbEffect.Text <> vbNullString Then
                  bstrEffectCLSID = CStr(.cmbEffect.Text)
               End If
            End With
            
            'update the timeline info given the new information
            If Not gbl_objDexterObject Is Nothing Then
               'set the media times
               Call gbl_objDexterObject.SetStartStop2(dblStartTime, dblStopTime)
               'set the effect
               If EffectFriendlyNameToCLSID(bstrEffectCLSID) <> vbNullString Then
                  Call gbl_objDexterObject.SetSubObjectGUIDB(EffectFriendlyNameToCLSID(bstrEffectCLSID))
               End If
               'refresh the ide
               Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            End If
            
            'destroy the dialog
            Unload frmClip: Set frmClip = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuFile_Click
            ' * procedure description:  Occurs when the file popup context menu is invoked
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuFile_Click()
            On Local Error GoTo ErrLine
            
            'do not allow new timeline or open file operations when rendering..
            If Not gbl_objQuartzVB Is Nothing Then
               If gbl_objQuartzVB.State = QTZStatusPlaying Then
                  If mnuFileNew.Enabled = True Then mnuFileNew.Enabled = False
                  If mnuFileOpen.Enabled = True Then mnuFileOpen.Enabled = False
               Else
                  If mnuFileNew.Enabled = False Then mnuFileNew.Enabled = True
                  If mnuFileOpen.Enabled = False Then mnuFileOpen.Enabled = True
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuFileExit_Click
            ' * procedure description:  Occurs when the file popup context menu's 'Exit' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuFileExit_Click()
            Dim frm As Form
            On Local Error GoTo ErrLine
            
            'proceed to end the application
            For Each frm In Forms
                  frm.Move Screen.Width * -8, Screen.Height * -8
                  frm.Visible = False: Unload frm
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuFileNewTimeline_Click
            ' * procedure description:  Occurs when the file popup context menu's 'New' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuFileNewTimeline_Click()
            On Local Error GoTo ErrLine
            
            'display the timeline dialog
            frmTimeline.Show vbModal, Me
            frmTimeline.Caption = "Insert Timeline"
            
            'wait until the user closes the dialog
            Do Until frmTimeline.Visible = False: DoEvents
            Loop
            'determine if they canceled out
            If frmTimeline.UnloadMode = 1 Then
               Unload frmTimeline: Set frmTimeline = Nothing: Exit Sub
            End If

            'clear the existing timeline
            If Not gbl_objTimeline Is Nothing Then Call ClearTimeline(gbl_objTimeline)
            'dereference & clean-up application-level data
            If Not gbl_objTimeline Is Nothing Then Set gbl_objTimeline = Nothing
            If Not gbl_objFilterGraph Is Nothing Then Set gbl_objFilterGraph = Nothing
            'reinitalize application-level data
            Set gbl_objTimeline = New AMTimeline
            Set gbl_objFilterGraph = New FilgraphManager
            'reinitalize the treeview/listview
            Call lstViewInfo.ListItems.Clear
            Call tvwSimpleTree.Nodes.Clear
            
            'set default(s)
            With gbl_objTimeline
               If IsNumeric(frmTimeline.txtTransitionsEnabled.Text) Then
                  .EnableTransitions CLng(frmTimeline.txtTransitionsEnabled.Text)
               End If
               If IsNumeric(frmTimeline.txtEffectsEnabled.Text) Then
                  .EnableEffects CLng(frmTimeline.txtEffectsEnabled.Text)
               End If
               If IsNumeric(frmTimeline.txtDefaultFPS.Text) Then
                  .SetDefaultFPS CDbl(frmTimeline.txtDefaultFPS.Text)
               End If
               If frmTimeline.cmbDefaultTransition.Text <> vbNullString Then
                  .SetDefaultTransitionB CStr(TransitionFriendlyNameToCLSID(frmTimeline.cmbDefaultTransition.Text))
               End If
               If frmTimeline.cmbDefaultEffect.Text <> vbNullString Then
                  .SetDefaultEffectB CStr(EffectFriendlyNameToCLSID(frmTimeline.cmbDefaultEffect.Text))
               End If
            End With
            
            'unload dialog
            Unload frmTimeline: Set frmTimeline = Nothing
            
            'update ide
            Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            
            'update the button(s)
           With tbMain.Buttons
              .Item("Play").Enabled = True
              .Item("Pause").Enabled = False
              .Item("Stop").Enabled = False
              .Item("Rewind").Enabled = False
              .Item("FastForward").Enabled = False
              .Item("SeekForward").Enabled = False
              .Item("SeekBackward").Enabled = False
           End With
           
            'update the state on the popup context menu
            mnuTimeLinePlay.Enabled = True
            mnuTimeLineStop.Enabled = False
            mnuTimeLinePause.Enabled = False
            mnuTimeLineRenderTimeLine.Enabled = True
            mnuTimeLineClearRenderEngine.Enabled = True
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuFileOpen_Click
            ' * procedure description:  Occurs when the file popup context menu's 'Open' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuFileOpen_Click()
            Dim nCount As Long
            Dim bstrFileName As String
            Dim objLocalTimeline As AMTimeline
            On Local Error GoTo ErrLine

            'display a common dialog
            'for the user and obtain a filename
            With ctrlCommonDialog
               .CancelError = True
               .DefaultExt = "XTL"
               .Filter = "XTL Files (*.xtl)|*.xtl|"
               .ShowOpen: bstrFileName = .FileName
            End With
            
            'verify the file extension is valid
            If InStr(1, LCase(bstrFileName), ".xtl") > 0 Then
               'at least it's been named an xtl file, proceed to attempt an import..
               Set objLocalTimeline = New AMTimeline
               Call RestoreTimeline(objLocalTimeline, bstrFileName, DEXImportXTL)
               'verify restoration
               If Not objLocalTimeline Is Nothing Then
                  'import succeeded; clean-up application-level scope
                  If Not gbl_objTimeline Is Nothing Then
                    'dereference & clean-up timeline
                     Call ClearTimeline(gbl_objTimeline)
                     Set gbl_objTimeline = Nothing
                     'dereference & clean-up rendering
                     If Not gbl_objQuartzVB Is Nothing Then Call gbl_objQuartzVB.StopGraph
                     If Not gbl_objFilterGraph Is Nothing Then Set gbl_objFilterGraph = Nothing
                     If Not gbl_objRenderEngine Is Nothing Then Call gbl_objRenderEngine.ScrapIt
                     If Not gbl_objRenderEngine Is Nothing Then Set gbl_objRenderEngine = Nothing
                  End If
                  'assign the local timeline to global scope
                  Set gbl_objTimeline = objLocalTimeline
                  'render the timeline and derive a filter graph manager
                  Set gbl_objFilterGraph = RenderTimeline(gbl_objTimeline)
                  'map the timeline to the userinterface
                  Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
                  mnuTimeLineClearRenderEngine.Enabled = False
                  'update the button(s)
                  With tbMain.Buttons
                     .Item("Play").Enabled = True
                     .Item("Pause").Enabled = False
                     .Item("Stop").Enabled = False
                     .Item("Rewind").Enabled = False
                     .Item("FastForward").Enabled = False
                     .Item("SeekForward").Enabled = False
                     .Item("SeekBackward").Enabled = False
                  End With
                  
                  'enable/disable the state of the popup context menu's
                  If mnuTimeLinePlay.Enabled = False Then mnuTimeLinePlay.Enabled = True
                  If mnuTimeLineStop.Enabled = True Then mnuTimeLineStop.Enabled = False
                  If mnuTimeLinePause.Enabled = False Then mnuTimeLinePause.Enabled = True
                  If mnuTimeLineRenderTimeLine.Enabled = True Then mnuTimeLineRenderTimeLine.Enabled = False
                  If mnuTimeLineClearRenderEngine.Enabled = False Then mnuTimeLineClearRenderEngine.Enabled = True
                  'reset module-level filename
                  gbl_bstrLoadFile = ctrlCommonDialog.FileName
                  'reset the caption on the application's main form
                  bstrFileName = Mid(bstrFileName, InStrRev(bstrFileName, "\") + 1)
                  Me.Caption = "DexterVB - " & bstrFileName
               End If
            End If
            'clean-up & dereference
            If Not objLocalTimeline Is Nothing Then Set objLocalTimeline = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuFileSaveAs_Click
            ' * procedure description:  Occurs when the file popup context menu's 'SaveAs' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuFileSaveAs_Click()
            Dim bstrFileName As String
            On Local Error GoTo ErrLine
            
            'display a common dialog
            'for the user and obtain a filename
            With ctrlCommonDialog
               .CancelError = True
               .DefaultExt = "XTL"
               .Filter = "XTL Files (*.xtl)|*.xtl|Graph Files (*.grf)|*.grf|"
               .ShowSave: bstrFileName = .FileName
            End With
            
            'verify the file extension is valid
            If InStr(1, LCase(bstrFileName), ".xtl") > 0 Then
               'user would like to export as an xtl file, proceed to attempt an export..
               If Not gbl_objTimeline Is Nothing Then
                  Call SaveTimeline(gbl_objTimeline, bstrFileName, DEXExportXTL)
               End If
            ElseIf InStr(1, LCase(bstrFileName), ".grf") > 0 Then
               'user would like to export as a graph file, proceed to attempt an export..
               If Not gbl_objTimeline Is Nothing Then
                  Call SaveTimeline(gbl_objTimeline, bstrFileName, DEXExportGRF)
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuGroup_Click
            ' * procedure description:  Occurs when the group popup context menu is invoked
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuGroup_Click()
            On Local Error GoTo ErrLine
            
            'set menu state
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Children = 0 Then
                  If mnuGroupExpand.Enabled = True Then mnuGroupExpand.Enabled = False
                  If mnuGroupCollapse.Enabled = True Then mnuGroupCollapse.Enabled = False
               ElseIf tvwSimpleTree.SelectedItem.Expanded = True Then
                  If mnuGroupExpand.Enabled = True Then mnuGroupExpand.Enabled = False
                  If mnuGroupCollapse.Enabled = False Then mnuGroupCollapse.Enabled = True
               ElseIf tvwSimpleTree.SelectedItem.Expanded = False Then
                  If mnuGroupExpand.Enabled = False Then mnuGroupExpand.Enabled = True
                  If mnuGroupCollapse.Enabled = True Then mnuGroupCollapse.Enabled = False
               Else
                  If mnuGroupExpand.Enabled = True Then mnuGroupExpand.Enabled = False
                  If mnuGroupCollapse.Enabled = True Then mnuGroupCollapse.Enabled = False
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuGroupCollapse_Click
            ' * procedure description:  Occurs when the group popup context menu's 'Collapse' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuGroupCollapse_Click()
            On Local Error GoTo ErrLine
            
            'collapse the treeview
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Expanded = True Then
                  tvwSimpleTree.SelectedItem.Expanded = False
                  If mnuGroupExpand.Enabled = False Then mnuGroupExpand.Enabled = True
                  If mnuGroupCollapse.Enabled = True Then mnuGroupCollapse.Enabled = False
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuGroupExpand_Click
            ' * procedure description:  Occurs when the group popup context menu's 'Expand' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuGroupExpand_Click()
            On Local Error GoTo ErrLine
            
            'expand the treeview
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Expanded = False Then
                  tvwSimpleTree.SelectedItem.Expanded = True
                  If mnuGroupExpand.Enabled = True Then mnuGroupExpand.Enabled = False
                  If mnuGroupCollapse.Enabled = False Then mnuGroupCollapse.Enabled = True
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuGroupAddComp_Click
            ' * procedure description:  Occurs when the group popup context menu's 'Add Composition' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuGroupAddComp_Click()
            Dim nPriority As Long
            Dim dblStartTime As Double
            Dim dblStopTime As Double
            Dim objNewComposite As AMTimelineComp
            On Local Error GoTo ErrLine
            
            'display the dialog
            frmComp.Caption = "Add Composition"
            frmComp.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do Until frmComp.Visible = False: DoEvents
            Loop
            'verify unload mode
            If frmComp.UnloadMode = 1 Then
               Unload frmComp: Set frmComp = Nothing: Exit Sub
            End If
            
            'query the dialog for user input
            With frmComp
               If IsNumeric(.txtPriority.Text) Then
                  nPriority = CLng(.txtPriority.Text)
               End If
               If IsNumeric(.txtStartTime.Text) Then
                  dblStartTime = CDbl(.txtStartTime.Text)
               End If
               If IsNumeric(.txtStopTime.Text) Then
                  dblStopTime = CDbl(.txtStopTime.Text)
               End If
            End With
            
            'insert the composite into the timeline
            If Not gbl_objTimeline Is Nothing Then
               Set objNewComposite = CreateComposite(gbl_objTimeline)
               If Not objNewComposite Is Nothing Then Call InsertComposite(objNewComposite, gbl_objDexterObject)
               'refresh the ide
               Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            End If
            
            'unload the dialog
            Unload frmComp: Set frmComp = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuGroupAddEffect_Click
            ' * procedure description:  Occurs when the group popup context menu's 'Add Effect' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuGroupAddEffect_Click()
            Dim nEffectPriority As Long
            Dim bstrEffectGUID As String
            Dim dblEffectStopTime As Double
            Dim dblEffectStartTime As Double
            Dim objNewEffect As AMTimelineEffect
            Dim objTimelineObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
            'display the insert effect dialog
            frmEffect.Caption = "Add Effect"
            frmEffect.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do: DoEvents
            If frmEffect.Visible = False Then Exit Do
            Loop
            'verify unload mode
            If frmEffect.UnloadMode = 1 Then
               Unload frmEffect: Set frmEffect = Nothing: Exit Sub
            End If
            
            'query the dialog information
            With frmEffect
               If IsNumeric(.txtStartTime.Text) Then
                  dblEffectStartTime = CDbl(.txtStartTime.Text)
               End If
               If IsNumeric(.txtStopTime.Text) Then
                  dblEffectStopTime = CDbl(.txtStopTime.Text)
               End If
               If IsNumeric(.txtPriority.Text) Then
                  nEffectPriority = CLng(.txtPriority.Text)
               End If
               If .cmbEffect.Text <> vbNullString Then
                  bstrEffectGUID = CStr(.cmbEffect.Text)
               End If
            End With
            
            'insert the effect into the timeline
            If Not gbl_objTimeline Is Nothing Then
               Set objNewEffect = CreateEffect(gbl_objTimeline)
               If Not objNewEffect Is Nothing Then Call InsertEffect(objNewEffect, gbl_objDexterObject, EffectFriendlyNameToCLSID(bstrEffectGUID), dblEffectStartTime, dblEffectStopTime)
               'refresh the ide
               Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            End If
            
            'destroy the dialog
            Unload frmEffect: Set frmEffect = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuGroupAddTrack_Click
            ' * procedure description:  Occurs when the group popup context menu's 'Add Track' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuGroupAddTrack_Click()
            Dim nBlank As Long
            Dim nMuted As Long
            Dim nPriority As Long
            Dim dblStartTime As Double
            Dim dblStopTime As Double
            Dim objNewTrack As AMTimelineTrack
            On Local Error GoTo ErrLine
            
            'display the dialog
            frmTrack.Caption = "Add Track"
            frmTrack.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do Until frmTrack.Visible = False: DoEvents
            Loop
            'verify unload mode
            If frmTrack.UnloadMode = 1 Then
               Unload frmTrack: Set frmTrack = Nothing: Exit Sub
            End If
            
            'obtain the user input from the dialog
            With frmTrack
               If IsNumeric(.txtBlank.Text) Then
                  nBlank = CLng(.txtBlank.Text)
               End If
               If IsNumeric(.txtMuted.Text) Then
                  nMuted = CLng(.txtMuted.Text)
               End If
               If IsNumeric(.txtMuted.Text) Then
                  nPriority = CLng(.txtMuted.Text)
               End If
               If IsNumeric(.txtStartTime.Text) Then
                  dblStartTime = CDbl(.txtStartTime.Text)
               End If
               If IsNumeric(.txtStopTime.Text) Then
                  dblStopTime = CDbl(.txtStopTime.Text)
               End If
            End With
            
            'insert the track into the timeline
            If Not gbl_objTimeline Is Nothing Then
               Set objNewTrack = CreateTrack(gbl_objTimeline)
               If Not objNewTrack Is Nothing Then Call InsertTrack(objNewTrack, gbl_objDexterObject, nPriority)
               'refresh the ide
               Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            End If
            
            'destroy dialog
            Unload frmTrack: Set frmTrack = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuGroupAddTransition_Click
            ' * procedure description:  Occurs when the group popup context menu's 'Add Transition' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuGroupAddTransition_Click()
            Dim nCutpoint As Long
            Dim nCutsOnly As Long
            Dim nSwapInputs As Long
            Dim bstrTransition As String
            Dim dblStartTime As Double
            Dim dblStopTime As Double
            Dim objNewTransition As AMTimelineTrans
            On Local Error GoTo ErrLine
            
            'display the dialog
            frmTransitions.Caption = "Add Transition"
            frmTransitions.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do Until frmTransitions.Visible = False: DoEvents
            Loop
            'verify unload mode
            If frmTransitions.UnloadMode = 1 Then
               Unload frmTransitions: Set frmTransitions = Nothing: Exit Sub
            End If
            
            'obtain user input
            With frmTransitions
               If IsNumeric(.txtCutpoint.Text) Then
                  nCutpoint = CLng(.txtCutpoint.Text)
               End If
               If IsNumeric(.txtCutsOnly.Text) Then
                  nCutsOnly = CLng(.txtCutsOnly.Text)
               End If
               If IsNumeric(.txtSwapInputs.Text) Then
                  nSwapInputs = CLng(.txtSwapInputs.Text)
               End If
               If IsNumeric(.txtStartTime.Text) Then
                  dblStartTime = CDbl(.txtStartTime.Text)
               End If
               If IsNumeric(.txtStopTime.Text) Then
                  dblStopTime = CDbl(.txtStopTime.Text)
               End If
               If .cmbTransition.Text <> vbNullString Then
                  bstrTransition = CStr(.cmbTransition.Text)
               End If
            End With
            
            'insert the transition into the timeline
            If Not gbl_objTimeline Is Nothing Then
               Set objNewTransition = CreateTransition(gbl_objTimeline)
               If Not objNewTransition Is Nothing Then Call InsertTransition(objNewTransition, gbl_objDexterObject, TransitionFriendlyNameToCLSID(bstrTransition), dblStartTime, dblStopTime)
               'refresh the ide
               Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            End If
            
            'destroy the dialog
            Unload frmTransitions: Set frmTransitions = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuGroupDelete_Click
            ' * procedure description:  Occurs when the group popup context menu's 'Delete' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuGroupDelete_Click()
            Dim objGroup As AMTimelineGroup
            On Local Error GoTo ErrLine
            
            'obtain the group from the global timeline object
            'which is reset when the user clicks a node on the tree
            If Not gbl_objDexterObject Is Nothing Then
               Set objGroup = gbl_objDexterObject
            Else: Exit Sub
            End If
            
            'remove the item
            Call gbl_objDexterObject.RemoveAll
            Call gbl_colNormalEnum.Remove(tvwSimpleTree.SelectedItem.Key)
            Call tvwSimpleTree.Nodes.Remove(tvwSimpleTree.SelectedItem.Index)
            If Not gbl_objDexterObject Is Nothing Then Set gbl_objDexterObject = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuGroupEdit_Click
            ' * procedure description:  Occurs when the group popup context menu's 'Edit' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuGroupEdit_Click()
            Dim nPriority As Long
            Dim nOutputBuffer As Long
            Dim nPreviewMode As Long
            Dim dblOutputFPS As Double
            Dim bstrGroupName As String
            Dim nSmartRecompDirty As Long
            Dim nSmartRecompFormat As Long
            Dim objGroup As AMTimelineGroup
            On Local Error GoTo ErrLine
            
            'obtain the group from the global timeline object
            'which is reset when the user clicks a node on the tree
            If Not gbl_objDexterObject Is Nothing Then
               Set objGroup = gbl_objDexterObject
            Else: Exit Sub
            End If
            
            'obtain existing group information
            If Not objGroup Is Nothing Then
               With objGroup
                   bstrGroupName = .GetGroupName
                  Call .GetOutputBuffering(nOutputBuffer)
                  Call .GetOutputFPS(dblOutputFPS)
                  Call .GetPreviewMode(nPreviewMode)
                  Call .GetPriority(nPriority)
                  Call .IsRecompressFormatDirty(nSmartRecompDirty)
                  Call .IsSmartRecompressFormatSet(nSmartRecompFormat)
               End With
            End If
            
            'hide the dialog
            Load frmGroup
            frmTimeline.Visible = False
            
            'update the form with the existing information
            With frmGroup
               .txtGroupName = bstrGroupName
               .txtMediaType = "?"  'feature unavailable, simply set to nullstring
               .txtPriority = nPriority
               .txtOutputFPS = dblOutputFPS
               .txtPreviewMode = nPreviewMode
               .txtBuffering = nOutputBuffer
            End With
            
            'display the dialog
            frmGroup.Caption = "Edit Group"
            frmGroup.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do Until frmGroup.Visible = False: DoEvents
            Loop
            'verify unload mode
            If frmGroup.UnloadMode = 1 Then
               Unload frmGroup: Set frmGroup = Nothing: Exit Sub
            End If
            
            'update the timeline info given the new information
            With objGroup
               .SetGroupName CStr(frmGroup.txtGroupName)
               If IsNumeric(frmGroup.txtMediaType) Then
                  .SetMediaTypeForVB CLng(frmGroup.txtMediaType)
               End If
               If IsNumeric(frmGroup.txtOutputFPS) Then
                  If CDbl(frmGroup.txtOutputFPS) > 0 Then
                     .SetOutputFPS CDbl(frmGroup.txtOutputFPS)
                  End If
               End If
               If IsNumeric(frmGroup.txtPreviewMode) Then
                  .SetPreviewMode CLng(frmGroup.txtPreviewMode)
               End If
               If IsNumeric(frmGroup.txtBuffering) Then
                  If CLng(frmGroup.txtBuffering) > 0 Then
                      .SetOutputBuffering CLng(frmGroup.txtBuffering)
                  End If
               End If
            End With
            
            'refresh the ide
            Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            
            'destroy the dialog
            Unload frmGroup: Set frmGroup = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuHelpAbout_Click
            ' * procedure description:  Occurs when the help menu's 'About' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuHelpAbout_Click()
            On Local Error GoTo ErrLine
            
            'display the about dialog for the user
            frmAbout.Show 1, Me
            frmAbout.SetFocus
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTimeLine_Click
            ' * procedure description:  Occurs when the timeline popup context menu is invoked
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTimeLine_Click()
            On Local Error GoTo ErrLine
            
            'set menu state
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Children = 0 Then
                  If mnuTimelineExpand.Enabled = True Then mnuTimelineExpand.Enabled = False
                  If mnuTimelineCollapse.Enabled = True Then mnuTimelineCollapse.Enabled = False
               ElseIf tvwSimpleTree.SelectedItem.Expanded = True Then
                  If mnuTimelineExpand.Enabled = True Then mnuTimelineExpand.Enabled = False
                  If mnuTimelineCollapse.Enabled = False Then mnuTimelineCollapse.Enabled = True
               ElseIf tvwSimpleTree.SelectedItem.Expanded = False Then
                  If mnuTimelineExpand.Enabled = False Then mnuTimelineExpand.Enabled = True
                  If mnuTimelineCollapse.Enabled = True Then mnuTimelineCollapse.Enabled = False
               Else
                  If mnuTimelineExpand.Enabled = True Then mnuTimelineExpand.Enabled = False
                  If mnuTimelineCollapse.Enabled = True Then mnuTimelineCollapse.Enabled = False
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTimelineCollapse_Click
            ' * procedure description:  Occurs when the timeline popup context menu's 'Collapse' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTimelineCollapse_Click()
            On Local Error GoTo ErrLine
            
            'collapse the treeview
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Expanded = True Then
                  tvwSimpleTree.SelectedItem.Expanded = False
                  If mnuTimelineExpand.Enabled = False Then mnuTimelineExpand.Enabled = True
                  If mnuTimelineCollapse.Enabled = True Then mnuTimelineCollapse.Enabled = False
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTimelineExpand_Click
            ' * procedure description:  Occurs when the timeline popup context menu's 'Expand' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTimelineExpand_Click()
            On Local Error GoTo ErrLine
            
            'expand the treeview
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Expanded = False Then
                  tvwSimpleTree.SelectedItem.Expanded = True
                  If mnuTimelineExpand.Enabled = True Then mnuTimelineExpand.Enabled = False
                  If mnuTimelineCollapse.Enabled = False Then mnuTimelineCollapse.Enabled = True
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTimeLineClearRenderEngine_Click
            ' * procedure description:  Occurs when the timeline popup context menu's 'ClearRenderEngine' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTimeLineClearRenderEngine_Click()
            On Local Error GoTo ErrLine
            
            'dereference & clean-up
            If Not gbl_objFilterGraph Is Nothing Then Set gbl_objFilterGraph = Nothing
            If Not gbl_objRenderEngine Is Nothing Then Call gbl_objRenderEngine.ScrapIt
            If Not gbl_objRenderEngine Is Nothing Then Set gbl_objRenderEngine = Nothing
            
            'disable popup context menu(s)
            If mnuTimeLineRenderTimeLine.Enabled = False Then mnuTimeLineRenderTimeLine.Enabled = True
            If mnuTimeLineClearRenderEngine.Enabled = True Then mnuTimeLineClearRenderEngine.Enabled = False
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTimeLineEdit_Click
            ' * procedure description:  Occurs when the timeline popup context menu's 'Edit' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTimeLineEdit_Click()
            Dim nDirty As Long
            Dim nInsertMode As Long
            Dim dblDuration As Double
            Dim nEffectsEnabled As Long
            Dim dblDefaultFPS As Double
            Dim bstrDefaultEffect As String
            Dim nTransitionsEnabled As Long
            Dim bstrDefaultTransition As String
            On Local Error GoTo ErrLine
            
            'obtain existing timeline information
            If Not gbl_objTimeline Is Nothing Then
               With gbl_objTimeline
                  .GetInsertMode nInsertMode
                  .TransitionsEnabled nTransitionsEnabled
                  .EffectsEnabled nEffectsEnabled
                  .GetDefaultFPS dblDefaultFPS
                  .IsDirty nDirty
                  bstrDefaultTransition = .GetDefaultTransitionB
                  bstrDefaultEffect = .GetDefaultEffectB
               End With
            End If
            
            'hide the dialog
            Load frmTimeline
            frmTimeline.Visible = False
            
            'update the form with the existing information
            With frmTimeline
               .txtInsertMode.Text = CStr(nInsertMode)
               .txtTransitionsEnabled.Text = CStr(nTransitionsEnabled)
               .txtEffectsEnabled.Text = CStr(nEffectsEnabled)
               .txtDuration.Text = CStr(dblDuration)
               .txtDefaultFPS.Text = CStr(dblDefaultFPS)
               .txtDirty.Text = CStr(nDirty)
               .cmbDefaultTransition.Text = CStr(TransitionCLSIDToFriendlyName(bstrDefaultTransition))
               .cmbDefaultEffect.Text = CStr(EffectCLSIDToFriendlyName(bstrDefaultEffect))
            End With
            
            'display the dialog
            frmTimeline.Caption = "Edit Timeline"
            frmTimeline.txtDuration.Enabled = False
            frmTimeline.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do Until frmTimeline.Visible = False: DoEvents
            Loop
            'determine if they canceled out
            If frmTimeline.UnloadMode = 1 Then
               Unload frmTimeline: Set frmTimeline = Nothing: Exit Sub
            End If
            
            'update the timeline info given the new information
            With gbl_objTimeline
               If IsNumeric(frmTimeline.txtTransitionsEnabled.Text) Then
                  .EnableTransitions CLng(frmTimeline.txtTransitionsEnabled.Text)
               End If
               If IsNumeric(frmTimeline.txtEffectsEnabled.Text) Then
                  .EnableEffects CLng(frmTimeline.txtEffectsEnabled.Text)
               End If
               If IsNumeric(frmTimeline.txtDefaultFPS.Text) Then
                  .SetDefaultFPS CDbl(frmTimeline.txtDefaultFPS.Text)
               End If
               If TransitionFriendlyNameToCLSID(frmTimeline.cmbDefaultTransition.Text) <> vbNullString Then
               .SetDefaultTransitionB CStr(TransitionFriendlyNameToCLSID(frmTimeline.cmbDefaultTransition.Text))
               End If
               If EffectFriendlyNameToCLSID(frmTimeline.cmbDefaultEffect.Text) <> vbNullString Then
                  .SetDefaultEffectB CStr(EffectFriendlyNameToCLSID(frmTimeline.cmbDefaultEffect.Text))
               End If
            End With
            
            'refresh the ide
            Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            
            'destroy the dialog
            Unload frmTimeline: Set frmTimeline = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTimeLineInsertGroup_Click
            ' * procedure description:  Occurs when the timeline popup context menu's 'Insert Group' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTimeLineInsertGroup_Click()
            Dim nPriority As Long
            Dim nBuffering As Long
            Dim nMediaType As Long
            Dim dblOutputFPS As Double
            Dim nPreviewMode As Long
            Dim bstrGroupName As String
            Dim objNewGroup As AMTimelineGroup
            On Local Error GoTo ErrLine
            
            'display the dialog
            frmGroup.Caption = "Insert Group"
            Call frmGroup.Show(vbModal, Me)
            
            'wait until the user closes the dialog
            Do Until frmGroup.Visible = False: DoEvents
            Loop
            'verify unload mode
            If frmGroup.UnloadMode = 1 Then
               Unload frmGroup: Set frmGroup = Nothing: Exit Sub
            End If
            
            'obtain the values the user entered into the dialog
            With frmGroup
               If IsNumeric(.txtPriority.Text) Then
                  nPriority = CLng(.txtPriority.Text)
               End If
               If IsNumeric(.txtBuffering.Text) Then
                  nBuffering = CLng(.txtBuffering.Text)
               End If
               If IsNumeric(.txtMediaType.Text) Then
                  nMediaType = CLng(.txtMediaType.Text)
               End If
               If IsNumeric(.txtOutputFPS.Text) Then
                  dblOutputFPS = CDbl(.txtOutputFPS.Text)
               End If
               If IsNumeric(.txtPreviewMode.Text) Then
                  nPreviewMode = CLng(.txtPreviewMode.Text)
               End If
               If .txtGroupName.Text <> vbNullString Then
                  bstrGroupName = CStr(.txtGroupName.Text)
               End If
            End With
            
            'insert the new group into the application timeline
            Set objNewGroup = _
            CreateGroup(gbl_objTimeline, bstrGroupName, nMediaType, dblOutputFPS, nPreviewMode, nBuffering)
            If Not objNewGroup Is Nothing Then Call InsertGroup(gbl_objTimeline, objNewGroup)
            
            'refresh ide
            Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            
            'update the button(s)
           With tbMain.Buttons
              .Item("New").Enabled = True
              .Item("Open").Enabled = True
              .Item("Save").Enabled = True
              .Item("Play").Enabled = True
              .Item("Pause").Enabled = False
              .Item("Stop").Enabled = False
              .Item("Rewind").Enabled = False
              .Item("FastForward").Enabled = False
              .Item("SeekForward").Enabled = False
              .Item("SeekBackward").Enabled = False
           End With
           
            'update the state on the popup context menu
            mnuFileSaveAs.Enabled = True
            mnuTimeLinePlay.Enabled = True
            mnuTimeLineStop.Enabled = False
            mnuTimeLinePause.Enabled = False
            mnuTimeLineRenderTimeLine.Enabled = True
            mnuTimeLineClearRenderEngine.Enabled = True
            
            'unload the dialog
            Unload frmGroup: Set frmGroup = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTimeLinePause_Click
            ' * procedure description:  Occurs when the timeline popup context menu's 'Pause' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTimelinePause_Click()
            On Local Error GoTo ErrLine
            
            'dupe the funtionality of a 'pause' button click
            Call tbMain_ButtonClick(tbMain.Buttons("Pause"))
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTimeLinePlay_Click
            ' * procedure description:  Occurs when the timeline popup context menu's 'Play' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTimelinePlay_Click()
            On Local Error GoTo ErrLine
            
            'dupe the funtionality of a 'play' button click
            Call tbMain_ButtonClick(tbMain.Buttons("Play"))
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTimeLineRenderTimeLine_Click
            ' * procedure description:  Occurs when the timeline popup context menu's 'RenderTimeline' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTimeLineRenderTimeLine_Click()
            On Local Error GoTo ErrLine
            
            'refresh ide
            Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            
            'connect front end
            Call gbl_objRenderEngine.ConnectFrontEnd
            
            'enable/disable popup context menu's
            mnuTimeLineRenderTimeLine.Enabled = False
            mnuTimeLineClearRenderEngine.Enabled = True
            Exit Sub
            
ErrLine:

            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTimeLineStop_Click
            ' * procedure description:  Occurs when the timeline popup context menu's 'Stop' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTimelineStop_Click()
            On Local Error GoTo ErrLine
            
            'dupe the funtionality of a 'stop' button click
            Call tbMain_ButtonClick(tbMain.Buttons("Stop"))
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTrack_Click
            ' * procedure description:  Occurs when the track popup context menu is invoked
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTrack_Click()
            On Local Error GoTo ErrLine
            
            'set menu state
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Children = 0 Then
                  If mnuTrackExpand.Enabled = True Then mnuTrackExpand.Enabled = False
                  If mnuTrackCollapse.Enabled = True Then mnuTrackCollapse.Enabled = False
               ElseIf tvwSimpleTree.SelectedItem.Expanded = True Then
                  If mnuTrackExpand.Enabled = True Then mnuTrackExpand.Enabled = False
                  If mnuTrackCollapse.Enabled = False Then mnuTrackCollapse.Enabled = True
               ElseIf tvwSimpleTree.SelectedItem.Expanded = False Then
                  If mnuTrackExpand.Enabled = False Then mnuTrackExpand.Enabled = True
                  If mnuTrackCollapse.Enabled = True Then mnuTrackCollapse.Enabled = False
               Else
                  If mnuTrackExpand.Enabled = True Then mnuTrackExpand.Enabled = False
                  If mnuTrackCollapse.Enabled = True Then mnuTrackCollapse.Enabled = False
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTrackCollapse_Click
            ' * procedure description:  Occurs when the track popup context menu's 'Collapse' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTrackCollapse_Click()
            On Local Error GoTo ErrLine
            
            'collapse the treeview
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Expanded = True Then
                  tvwSimpleTree.SelectedItem.Expanded = False
                  If mnuTrackExpand.Enabled = False Then mnuTrackExpand.Enabled = True
                  If mnuTrackCollapse.Enabled = True Then mnuTrackCollapse.Enabled = False
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTrackExpand_Click
            ' * procedure description:  Occurs when the track popup context menu's 'Expand' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTrackExpand_Click()
            On Local Error GoTo ErrLine
            
            'expand the treeview
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Expanded = False Then
                  tvwSimpleTree.SelectedItem.Expanded = True
                  If mnuTrackExpand.Enabled = True Then mnuTrackExpand.Enabled = False
                  If mnuTrackCollapse.Enabled = False Then mnuTrackCollapse.Enabled = True
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTrackAddClip_Click
            ' * procedure description:  Occurs when the track popup context menu's 'Add Clip' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTrackAddClip_Click()
            Dim nMuted As Long
            Dim dblFPS As Double
            Dim dblTStop As Double
            Dim dblTStart As Double
            Dim dblMStart As Double
            Dim dblMStop As Double
            Dim nStretchMode As Long
            Dim nStreamNumber As Long
            Dim dblMediaLength As Double
            Dim bstrMediaSourceName As String
            Dim objNewSourceClip As AMTimelineSrc
            On Local Error GoTo ErrLine
            
            'display the dialog
            frmClip.Caption = "Add Clip"
            frmClip.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do Until frmClip.Visible = False: DoEvents
            Loop
            'verify unload mode
            If frmClip.UnloadMode = 1 Then
               Unload frmClip: Set frmClip = Nothing: Exit Sub
            End If
            
            'obtain the user input from the dialog
            With frmClip
               If IsNumeric(.txtMStart.Text) Then
                  dblMStart = CDbl(.txtMStart.Text)
               End If
               If IsNumeric(.txtTStop.Text) Then
                  dblMStop = CDbl(.txtTStop.Text)
               End If
               If IsNumeric(.txtTStart.Text) Then
                  dblTStart = CDbl(.txtTStart.Text)
               End If
               If IsNumeric(.txtTStop.Text) Then
                  dblTStop = CDbl(.txtTStop.Text)
               End If
               If IsNumeric(.txtMediaLength.Text) Then
                  dblMediaLength = CDbl(.txtMediaLength.Text)
               End If
               If IsNumeric(.txtStreamNumber.Text) Then
                  nStreamNumber = CLng(.txtStreamNumber.Text)
               End If
               If IsNumeric(.txtFPS.Text) Then
                  dblFPS = CDbl(.txtFPS.Text)
               End If
               If IsNumeric(.txtStretchMode.Text) Then
                  nStretchMode = CLng(.txtStretchMode.Text)
               End If
               If IsNumeric(.txtMuted.Text) Then
                  nMuted = CLng(.txtMuted.Text)
               End If
               If .txtMediaName <> vbNullString Then
                  bstrMediaSourceName = CStr(.txtMediaName)
               End If
            End With
            
            'insert the new clip into the timeline
            If Not gbl_objTimeline Is Nothing Then
               Set objNewSourceClip = CreateSource(gbl_objTimeline)
               If Not objNewSourceClip Is Nothing Then Call InsertSource(gbl_objDexterObject, objNewSourceClip, bstrMediaSourceName, dblTStart, dblTStop, dblMStart, dblMStop)
               If Not objNewSourceClip Is Nothing Then
                  'refresh the ide
                  Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
                  
                  'update the button(s)
                  With tbMain.Buttons
                     .Item("Play").Enabled = True
                     .Item("Pause").Enabled = False
                     .Item("Stop").Enabled = False
                     .Item("Rewind").Enabled = False
                     .Item("FastForward").Enabled = False
                     .Item("SeekForward").Enabled = False
                     .Item("SeekBackward").Enabled = False
                  End With
           
                  'update the state on the popup context menu
                  mnuTimeLinePlay.Enabled = True
                  mnuTimeLineStop.Enabled = False
                  mnuTimeLinePause.Enabled = False
                  mnuTimeLineRenderTimeLine.Enabled = True
                  mnuTimeLineClearRenderEngine.Enabled = True
               End If
            End If
            
            'destroy dialog
            Unload frmClip: Set frmClip = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTrackAddEffect_Click
            ' * procedure description:  Occurs when the track popup context menu's 'Add Effect' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTrackAddEffect_Click()
            Dim nEffectPriority As Long
            Dim bstrEffectGUID As String
            Dim dblEffectStopTime As Double
            Dim dblEffectStartTime As Double
            Dim objNewEffect As AMTimelineEffect
            Dim objTimelineObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
            'display the insert effect dialog
            frmEffect.Caption = "Add Effect"
            frmEffect.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do: DoEvents
            If frmEffect.Visible = False Then Exit Do
            Loop
            'verify unload mode
            If frmEffect.UnloadMode = 1 Then
               Unload frmEffect: Set frmEffect = Nothing: Exit Sub
            End If
            
            'query the dialog information
            With frmEffect
               If IsNumeric(.txtStartTime.Text) Then
                  dblEffectStartTime = CDbl(.txtStartTime.Text)
               End If
               If IsNumeric(.txtStopTime.Text) Then
                  dblEffectStopTime = CDbl(.txtStopTime.Text)
               End If
               If IsNumeric(.txtPriority.Text) Then
                  nEffectPriority = CLng(.txtPriority.Text)
               End If
               If .cmbEffect.Text <> vbNullString Then
                  bstrEffectGUID = CStr(.cmbEffect.Text)
               End If
            End With
            
            'insert the effect into the timeline
            If Not gbl_objTimeline Is Nothing Then
               Set objNewEffect = CreateEffect(gbl_objTimeline)
               If Not objNewEffect Is Nothing Then Call InsertEffect(objNewEffect, gbl_objDexterObject, EffectFriendlyNameToCLSID(bstrEffectGUID), dblEffectStartTime, dblEffectStopTime)
               'refresh the ide
               Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            End If
            
            'destroy the dialog
            Unload frmEffect: Set frmEffect = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTrackAddTransition_Click
            ' * procedure description:  Occurs when the track popup context menu's 'Add Transition' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTrackAddTransition_Click()
            Dim nCutpoint As Long
            Dim nCutsOnly As Long
            Dim nSwapInputs As Long
            Dim bstrTransition As String
            Dim dblStartTime As Double
            Dim dblStopTime As Double
            Dim objNewTransition As AMTimelineTrans
            On Local Error GoTo ErrLine
            
            'display the dialog
            frmTransitions.Caption = "Add Transition"
            frmTransitions.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do Until frmTransitions.Visible = False: DoEvents
            Loop
            'verify unload mode
            If frmTransitions.UnloadMode = 1 Then
               Unload frmTransitions: Set frmTransitions = Nothing: Exit Sub
            End If
            
            'obtain user input
            With frmTransitions
               If IsNumeric(.txtCutpoint.Text) Then
                  nCutpoint = CLng(.txtCutpoint.Text)
               End If
               If IsNumeric(.txtCutsOnly.Text) Then
                  nCutsOnly = CLng(.txtCutsOnly.Text)
               End If
               If IsNumeric(.txtSwapInputs.Text) Then
                  nSwapInputs = CLng(.txtSwapInputs.Text)
               End If
               If IsNumeric(.txtStartTime.Text) Then
                  dblStartTime = CDbl(.txtStartTime.Text)
               End If
               If IsNumeric(.txtStopTime.Text) Then
                  dblStopTime = CDbl(.txtStopTime.Text)
               End If
               If .cmbTransition.Text <> vbNullString Then
                  bstrTransition = CStr(.cmbTransition.Text)
               End If
            End With
            
            'insert the transition into the timeline
            If Not gbl_objTimeline Is Nothing Then
               Set objNewTransition = CreateTransition(gbl_objTimeline)
               If Not objNewTransition Is Nothing Then Call InsertTransition(objNewTransition, gbl_objDexterObject, TransitionFriendlyNameToCLSID(bstrTransition), dblStartTime, dblStopTime)
               'refresh the ide
               Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            End If
            
            'destroy the dialog
            Unload frmTransitions: Set frmTransitions = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTrackDelete_Click
            ' * procedure description:  Occurs when the track popup context menu's 'Delete' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTrackDelete_Click()
            Dim objTrack As AMTimelineTrack
            On Local Error GoTo ErrLine
            
            'obtain the track from the global timeline object
            'which is reset when the user clicks a node on the tree
            If Not gbl_objDexterObject Is Nothing Then
               Set objTrack = gbl_objDexterObject
            Else: Exit Sub
            End If
            
            'remove the item
            Call gbl_objDexterObject.RemoveAll
            Call gbl_colNormalEnum.Remove(tvwSimpleTree.SelectedItem.Key)
            Call tvwSimpleTree.Nodes.Remove(tvwSimpleTree.SelectedItem.Index)
            If Not gbl_objDexterObject Is Nothing Then Set gbl_objDexterObject = Nothing
            
            'clean-up & dereference
            If Not objTrack Is Nothing Then Set objTrack = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTrackEdit_Click
            ' * procedure description:  Occurs when the track popup context menu's 'Edit' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTrackEdit_Click()
            Dim nBlank As Long
            Dim nMuted As Long
            Dim nPriority As Long
            Dim dblStopTime As Double
            Dim dblStartTime As Double
            Dim objTrack As AMTimelineTrack
            On Local Error GoTo ErrLine
            
            'obtain the track from the global timeline object
            'which is reset when the user clicks a node on the tree
            If Not gbl_objDexterObject Is Nothing Then
               Set objTrack = gbl_objDexterObject
            Else: Exit Sub
            End If
            
            'obtain existing group information
            If Not objTrack Is Nothing Then
               With objTrack
                  'get blank
                  Call objTrack.AreYouBlank(nBlank)
                  'get muted
                  Call gbl_objDexterObject.GetMuted(nMuted)
                  'get start /stoptime
                  Call gbl_objDexterObject.GetStartStop2(dblStartTime, dblStopTime)
               End With
            End If
            
            'hide the dialog
            Load frmTrack
            frmTrack.Visible = False
            
            'update the form with the existing information
            With frmTrack
               .txtPriority = 0  'methodology not available
               .txtStartTime = CStr(dblStartTime)
               .txtStopTime = CStr(dblStopTime)
               .txtBlank.Text = CStr(nBlank)
               .txtMuted.Text = CStr(nMuted)
            End With
            
            'display the dialog
            frmTrack.Caption = "Edit Track"
            frmTrack.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do Until frmTrack.Visible = False: DoEvents
            Loop
            'verify unload mode
            If frmTrack.UnloadMode = 1 Then
               Unload frmTrack: Set frmTrack = Nothing: Exit Sub
            End If
            
            'query the dialog information
            With frmTrack
               If IsNumeric(.txtStartTime.Text) Then
                  dblStartTime = CDbl(.txtStartTime.Text)
               End If
               If IsNumeric(.txtStopTime.Text) Then
                  dblStopTime = CDbl(.txtStopTime.Text)
               End If
               If IsNumeric(.txtPriority.Text) Then
                  nPriority = CLng(.txtPriority.Text)
               End If
               If .txtBlank.Text <> vbNullString Then
                  nBlank = CStr(.txtBlank.Text)
               End If
               If .txtMuted.Text <> vbNullString Then
                  nMuted = CStr(.txtMuted.Text)
               End If
            End With
            
            'update the timeline info given the new information
            If Not gbl_objDexterObject Is Nothing Then
               'set the muted state
               Call gbl_objDexterObject.SetMuted(nMuted)
               'set the media times
               Call gbl_objDexterObject.SetStartStop2(dblStartTime, dblStopTime)
            End If
            
            'refresh the ide
            Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            
            'destroy the dialog
            Unload frmTrack: Set frmTrack = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTrans_Click
            ' * procedure description:  Occurs when the transition popup context menu is invoked
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTrans_Click()
            On Local Error GoTo ErrLine
            
            'set menu state
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Children = 0 Then
                  If mnuTransExpand.Enabled = True Then mnuTransExpand.Enabled = False
                  If mnuTransCollapse.Enabled = True Then mnuTransCollapse.Enabled = False
               ElseIf tvwSimpleTree.SelectedItem.Expanded = True Then
                  If mnuTransExpand.Enabled = True Then mnuTransExpand.Enabled = False
                  If mnuTransCollapse.Enabled = False Then mnuTransCollapse.Enabled = True
               ElseIf tvwSimpleTree.SelectedItem.Expanded = False Then
                  If mnuTransExpand.Enabled = False Then mnuTransExpand.Enabled = True
                  If mnuTransCollapse.Enabled = True Then mnuTransCollapse.Enabled = False
               Else
                  If mnuTransExpand.Enabled = True Then mnuTransExpand.Enabled = False
                  If mnuTransCollapse.Enabled = True Then mnuTransCollapse.Enabled = False
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTransCollapse_Click
            ' * procedure description:  Occurs when the transition popup context menu's 'Collapse' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTransCollapse_Click()
            On Local Error GoTo ErrLine
            
            'collapse the treeview
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Expanded = True Then
                  tvwSimpleTree.SelectedItem.Expanded = False
                  If mnuTransExpand.Enabled = False Then mnuTransExpand.Enabled = True
                  If mnuTransCollapse.Enabled = True Then mnuTransCollapse.Enabled = False
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTransExpand_Click
            ' * procedure description:  Occurs when the transition popup context menu's 'Expand' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTransExpand_Click()
            On Local Error GoTo ErrLine
            
            'expand the treeview
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.SelectedItem.Expanded = False Then
                  tvwSimpleTree.SelectedItem.Expanded = True
                  If mnuTransExpand.Enabled = True Then mnuTransExpand.Enabled = False
                  If mnuTransCollapse.Enabled = False Then mnuTransCollapse.Enabled = True
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTransDelete_Click
            ' * procedure description:  Occurs when the transition popup context menu's 'Delete' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTransDelete_Click()
            Dim objTransition As AMTimelineTrans
            On Local Error GoTo ErrLine
            
            'obtain the transition from the global timeline object
            'which is reset when the user clicks a node on the tree
            If Not gbl_objDexterObject Is Nothing Then
               Set objTransition = gbl_objDexterObject
            Else: Exit Sub
            End If

            'remove the item
            Call gbl_objDexterObject.RemoveAll
            Call gbl_colNormalEnum.Remove(tvwSimpleTree.SelectedItem.Key)
            Call tvwSimpleTree.Nodes.Remove(tvwSimpleTree.SelectedItem.Index)
            If Not gbl_objDexterObject Is Nothing Then Set gbl_objDexterObject = Nothing
            
            'clean-up & dereference
            If Not objTransition Is Nothing Then Set objTransition = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnuTransEdit_Click
            ' * procedure description:  Occurs when the transition popup context menu's 'Edit' option is elected
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnuTransEdit_Click()
            Dim nCutsOnly As Long
            Dim nSwapInputs As Long
            Dim dblCutpoint As Double
            Dim dblStopTime As Double
            Dim dblStartTime As Double
            Dim bstrTransitionCLSID As String
            Dim objTransition As AMTimelineTrans
            On Local Error GoTo ErrLine
            
            'obtain the transition from the global timeline object
            'which is reset when the user clicks a node on the tree
            If Not gbl_objDexterObject Is Nothing Then
               Set objTransition = gbl_objDexterObject
            Else: Exit Sub
            End If
            
            'obtain existing transition information
            If Not objTransition Is Nothing Then
               With objTransition
                  'get cutpoint
                  Call .GetCutPoint2(dblCutpoint)
                  'get cutsonly
                  Call .GetCutsOnly(nCutsOnly)
                  'get swap inputs
                  Call .GetSwapInputs(nSwapInputs)
                  'get start /stoptime
                  Call gbl_objDexterObject.GetStartStop2(dblStartTime, dblStopTime)
                  'get transition clsid
                  bstrTransitionCLSID = gbl_objDexterObject.GetSubObjectGUIDB
               End With
            End If
            
            'hide the dialog
            Load frmTransitions
            frmTransitions.Visible = False
            
            'update the form with the existing information
            With frmTransitions
               .txtStartTime = CStr(dblStartTime)
               .txtStopTime = CStr(dblStopTime)
               .txtCutsOnly = CStr(nCutsOnly)
               .txtCutpoint = CStr(dblCutpoint)
               .txtSwapInputs = CStr(nSwapInputs)
               .cmbTransition = CStr(TransitionCLSIDToFriendlyName(bstrTransitionCLSID))
            End With
            
            'display the dialog
            frmTransitions.Caption = "Edit Transition"
            frmTransitions.Show vbModal, Me
            
            'wait until the user closes the dialog
            Do Until frmTransitions.Visible = False: DoEvents
            Loop
            'verify unload mode
            If frmTransitions.UnloadMode = 1 Then
               Unload frmTransitions: Set frmTransitions = Nothing: Exit Sub
            End If
            
            'query the dialog information
            With frmTransitions
               If IsNumeric(.txtStartTime.Text) Then
                  dblStartTime = CDbl(.txtStartTime.Text)
               End If
               If IsNumeric(.txtStopTime.Text) Then
                  dblStopTime = CDbl(.txtStopTime.Text)
               End If
               If IsNumeric(.txtCutpoint.Text) Then
                  dblCutpoint = CDbl(.txtCutpoint.Text)
               End If
               If IsNumeric(.txtCutsOnly.Text) Then
                  nCutsOnly = CLng(.txtCutsOnly.Text)
               End If
               If IsNumeric(.txtSwapInputs.Text) Then
                  nSwapInputs = CLng(.txtSwapInputs.Text)
               End If
               If .cmbTransition <> vbNullString Then
                 bstrTransitionCLSID = TransitionFriendlyNameToCLSID(CStr(.cmbTransition.Text))
               End If
            End With
            
            'update the transition info given the new information
            If Not gbl_objDexterObject Is Nothing Then
               'set the transition clsid
               Call gbl_objDexterObject.SetSubObjectGUIDB(bstrTransitionCLSID)
               'set the cutpoint
               Call objTransition.SetCutPoint2(dblCutpoint)
               'set the cutsonly
               Call objTransition.SetCutsOnly(nCutsOnly)
               'set the swap inputs
               Call objTransition.SetSwapInputs(nSwapInputs)
               'set the media times
               Call gbl_objDexterObject.SetStartStop2(dblStartTime, dblStopTime)
            End If
            
            'refresh the ide
            Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            
            'destroy the dialog
            Unload frmTransitions: Set frmTransitions = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub






' **************************************************************************************************************************************
' * PRIVATE INTERFACE- STATUSBAR EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: sbStatus_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *
            ' ******************************************************************************************************************************
            Private Sub sbStatus_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            If Me.MousePointer = 9 Then Me.MousePointer = vbDefault
            If Me.BackColor = vbBlack Then Me.BackColor = &H8000000F
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: sbStatus_OLEDragDrop
            ' * procedure description:  Occurs when data is dropped onto the control via an OLE drag/drop operation, and OLEDropMode is set to manual.
            ' *
            ' ******************************************************************************************************************************
            Private Sub sbStatus_OLEDragDrop(Data As MSComctlLib.DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            
            'pass to the application drag drop handler
            Call AppOLEDragDrop(Data, Effect, Button, Shift, X, Y)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: sbStatus_OLEDragOver
            ' * procedure description:  Occurs when the mouse is moved over the control during an OLE drag/drop operation, if its OLEDropMode property is set to manual.
            ' *
            ' ******************************************************************************************************************************
            Private Sub sbStatus_OLEDragOver(Data As MSComctlLib.DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single, State As Integer)
            On Local Error GoTo ErrLine
            
            'pass to the application drag over handler
            Call AppOLEDragOver(Data, Effect, Button, Shift, X, Y, State)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub




' **************************************************************************************************************************************
' * PRIVATE INTERFACE- LISTVIEW EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: lstViewInfo_AfterLabelEdit
            ' * procedure description:   Occurs after a user edits the label of the currently selected Node or ListItem object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub lstViewInfo_AfterLabelEdit(Cancel As Integer, NewString As String)
            On Local Error GoTo ErrLine
            Cancel = 1
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: lstViewInfo_BeforeLabelEdit
            ' * procedure description:  Occurs when a user attempts to edit the label of the currently selected ListItem or Node object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub lstViewInfo_BeforeLabelEdit(Cancel As Integer)
            On Local Error GoTo ErrLine
            Cancel = 1
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: lstViewInfo_Click
            ' * procedure description:  Occurs when the user presses and then releases a mouse button over an object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub lstViewInfo_Click()
            On Local Error GoTo ErrLine
            lstViewInfo.SetFocus
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: lstViewInfo_ColumnClick
            ' * procedure description:  Occurs when a ColumnHeader object in a ListView control is clicked.
            ' *
            ' ******************************************************************************************************************************
            Private Sub lstViewInfo_ColumnClick(ByVal ColumnHeader As MSComctlLib.ColumnHeader)
            On Local Error GoTo ErrLine
            
            'sort listview contents by given column
            If ColumnHeader.Key = "Parameter" Then
               If lstViewInfo.Sorted = False Then lstViewInfo.Sorted = True
               If lstViewInfo.SortKey <> 0 Then lstViewInfo.SortKey = 0
               If lstViewInfo.SortOrder <> lvwAscending Then lstViewInfo.SortOrder = lvwAscending
            ElseIf ColumnHeader.Key = "Value" Then
               If lstViewInfo.Sorted = False Then lstViewInfo.Sorted = True
               If lstViewInfo.SortKey <> 1 Then lstViewInfo.SortKey = 1
               If lstViewInfo.SortOrder <> lvwAscending Then lstViewInfo.SortOrder = lvwAscending
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: lstViewInfo_GotFocus
            ' * procedure description:  Occurs when an object receives the focus.
            ' *
            ' ******************************************************************************************************************************
            Private Sub lstViewInfo_GotFocus()
            On Local Error GoTo ErrLine
            'reset the tooltip text
            lstViewInfo.ToolTipText = vbNullString
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: lstViewInfo_ItemClick
            ' * procedure description:  Occurs when a ListItem object is clicked or selected
            ' *
            ' ******************************************************************************************************************************
            Private Sub lstViewInfo_ItemClick(ByVal Item As MSComctlLib.ListItem)
            On Local Error GoTo ErrLine
            
            'display a tooltip for the item
            If Not Item Is Nothing Then
               If Item.ListSubItems.Count > 0 Then
                  If Trim(Item.SubItems(1)) <> vbNullString Then
                  lstViewInfo.ToolTipText = CStr(Trim(Item.Text) & " = " & Chr(34) & Trim(Item.SubItems(1)) & Chr(34))
                  Else: lstViewInfo.ToolTipText = vbNullString
                  End If
               Else: lstViewInfo.ToolTipText = CStr(Item.Text)
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: lstViewInfo_LostFocus
            ' * procedure description:  Occurs when an object loses the focus.
            ' *
            ' ******************************************************************************************************************************
            Private Sub lstViewInfo_LostFocus()
            On Local Error GoTo ErrLine
            'reset the tooltip text
            lstViewInfo.ToolTipText = vbNullString
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: lstViewInfo_MouseDown
            ' * procedure description:  Occurs when the user presses the mouse button while an object has the focus.
            ' *
            ' ******************************************************************************************************************************
            Private Sub lstViewInfo_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            'reset the tooltip text
            lstViewInfo.ToolTipText = vbNullString
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: lstViewInfo_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *
            ' ******************************************************************************************************************************
            Private Sub lstViewInfo_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            
            If Me.MousePointer = 9 Then Me.MousePointer = vbDefault
            If Me.BackColor = vbBlack Then Me.BackColor = &H8000000F
            
            'disable tooltip if a hittest on the current position fails
            If lstViewInfo.HitTest(X, Y) Is Nothing Then lstViewInfo.ToolTipText = vbNullString
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: lstViewInfo_OLEDragDrop
            ' * procedure description:  Occurs when data is dropped onto the control via an OLE drag/drop operation, and OLEDropMode is set to manual.
            ' *
            ' ******************************************************************************************************************************
            Private Sub lstViewInfo_OLEDragDrop(Data As MSComctlLib.DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            
            'pass to the application drag drop handler
            Call AppOLEDragDrop(Data, Effect, Button, Shift, X, Y)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: lstViewInfo_OLEDragOver
            ' * procedure description:  Occurs when the mouse is moved over the control during an OLE drag/drop operation, if its OLEDropMode property is set to manual.
            ' *
            ' ******************************************************************************************************************************
            Private Sub lstViewInfo_OLEDragOver(Data As MSComctlLib.DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single, State As Integer)
            On Local Error GoTo ErrLine
            
            'pass to the application drag over handler
            Call AppOLEDragOver(Data, Effect, Button, Shift, X, Y, State)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub


' **************************************************************************************************************************************
' * PRIVATE INTERFACE- TREEVIEW EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: tvwSimpleTree_AfterLabelEdit
            ' * procedure description:  Occurs after a user edits the label of the currently selected Node or ListItem object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tvwSimpleTree_AfterLabelEdit(Cancel As Integer, NewString As String)
            On Local Error GoTo ErrLine
            Cancel = 1
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tvwSimpleTree_BeforeLabelEdit
            ' * procedure description:  Occurs when a user attempts to edit the label of the currently selected ListItem or Node object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tvwSimpleTree_BeforeLabelEdit(Cancel As Integer)
            On Local Error GoTo ErrLine
            Cancel = 1
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tvwSimpleTree_Click
            ' * procedure description:  Occurs when the user presses and then releases a mouse button over an object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tvwSimpleTree_Click()
            On Local Error GoTo ErrLine
            
            'set the root node selected if nothing is selected
            If tvwSimpleTree.SelectedItem Is Nothing Then
               If tvwSimpleTree.Nodes.Count > 0 Then
                  If Not tvwSimpleTree.Nodes(1).Root Is Nothing Then
                     Set tvwSimpleTree.SelectedItem = tvwSimpleTree.Nodes(1).Root
                  End If
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tvwSimpleTree_Collapse
            ' * procedure description:  Generated when any Node object in a TreeView control is collapsed.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tvwSimpleTree_Collapse(ByVal node As MSComctlLib.node)
            On Local Error GoTo ErrLine
            
            'ensure selected
            If Not node Is Nothing Then Set tvwSimpleTree.SelectedItem = node
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tvwSimpleTree_DblClick
            ' * procedure description:  Occurs when you press and release a mouse button and then press and release it again over an object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tvwSimpleTree_DblClick()
            On Local Error GoTo ErrLine
            
            'set focus
            tvwSimpleTree.SetFocus
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tvwSimpleTree_Expand
            ' * procedure description:  Occurs when a Node object in a TreeView control is expanded; that is, when its child nodes become visible.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tvwSimpleTree_Expand(ByVal node As MSComctlLib.node)
            On Local Error GoTo ErrLine
            
            'ensure selected
            If Not node Is Nothing Then Set tvwSimpleTree.SelectedItem = node
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tvwSimpleTree_GotFocus
            ' * procedure description:  Occurs when an object receives the focus.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tvwSimpleTree_GotFocus()
            On Local Error GoTo ErrLine
            'reset the tooltip text
            tvwSimpleTree.ToolTipText = vbNullString
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tvwSimpleTree_KeyDown
            ' * procedure description:  Occurs when the user presses a key while an object has the focus.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tvwSimpleTree_KeyDown(KeyCode As Integer, Shift As Integer)
            Dim objNode As node
            On Local Error GoTo ErrLine
            
            'obtain the selected node on the treeview
            If Not tvwSimpleTree.SelectedItem Is Nothing Then
               Set objNode = tvwSimpleTree.SelectedItem
            Else: Exit Sub
            End If
            
            'cross reference the object's guid with the module-level collection
            'and attempt to obtain a match based on the unique identifier of the node
            If Not gbl_colNormalEnum(objNode.Key) Is Nothing Then
               Select Case objNode.Tag
                  Case "AMTimelineGroup"
                      Set gbl_objDexterObject = gbl_colNormalEnum(objNode.Key)
                  Case "AMTimelineComp"
                      Set gbl_objDexterObject = gbl_colNormalEnum(objNode.Key)
                  Case "AMTimelineTrack"
                      Set gbl_objDexterObject = gbl_colNormalEnum(objNode.Key)
                  Case "AMTimelineSrc"
                      Set gbl_objDexterObject = gbl_colNormalEnum(objNode.Key)
                  Case "AMTimelineTrans"
                      Set gbl_objDexterObject = gbl_colNormalEnum(objNode.Key)
                  Case "AMTimelineEffect"
                      Set gbl_objDexterObject = gbl_colNormalEnum(objNode.Key)
              End Select
            Else: Exit Sub
            End If
            
            
            If KeyCode = vbKeyDelete Then
               'delete the item from the treeview
               If Not gbl_colNormalEnum(objNode.Key) Is Nothing Then
               
                  Select Case objNode.Tag
                  
                     Case "AMTimeline"
                        'dereference & clean-up the existing timeline
                        If Not gbl_objTimeline Is Nothing Then Call ClearTimeline(gbl_objTimeline)
                        'dereference & clean-up application-level data
                        If Not gbl_objTimeline Is Nothing Then Set gbl_objTimeline = Nothing
                        If Not gbl_objFilterGraph Is Nothing Then Set gbl_objFilterGraph = Nothing
                        'reinitalize application-level data
                        Set gbl_objTimeline = New AMTimeline
                        Set gbl_objFilterGraph = New FilgraphManager
                        'reinitalize the treeview/listview
                        Call lstViewInfo.ListItems.Clear
                        Call tvwSimpleTree.Nodes.Clear
                        
                     Case "AMTimelineGroup"
                         'dupe the functionality of a manual group delete
                         Call mnuGroupDelete_Click
                         
                     Case "AMTimelineComp"
                         'dupe the functionality of a manual composition delete
                         Call mnuCompDelete_Click
                         
                     Case "AMTimelineTrack"
                         'dupe the functionality of a manual track delete
                         Call mnuTrackDelete_Click
                         
                     Case "AMTimelineSrc"
                         'dupe the functionality of a manual clip source delete
                         Call mnuClipDelete_Click
                         
                     Case "AMTimelineTrans"
                         'dupe the functionality of a manual transition delete
                         Call mnuTransDelete_Click
                         
                     Case "AMTimelineEffect"
                         'dupe the functionality of a manual effect delete
                         Call mnuEffectDelete_Click
                 End Select
               Else: Exit Sub
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tvwSimpleTree_LostFocus
            ' * procedure description:  Occurs when an object loses the focus.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tvwSimpleTree_LostFocus()
            On Local Error GoTo ErrLine
            'reset the tooltip text
            tvwSimpleTree.ToolTipText = vbNullString
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tvwSimpleTree_MouseDown
            ' * procedure description:   Occurs when the user presses the mouse button while an object has the focus.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tvwSimpleTree_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
            Dim objNode As node
            On Local Error GoTo ErrLine
            
            'get the current node
            If Not tvwSimpleTree.HitTest(X, Y) Is Nothing Then
               Set objNode = tvwSimpleTree.HitTest(X, Y)
               Set tvwSimpleTree.SelectedItem = objNode
            ElseIf Not tvwSimpleTree.SelectedItem Is Nothing Then
               Set objNode = tvwSimpleTree.SelectedItem
            Else: Exit Sub
            End If
            
            'cross reference the object's guid with the module-level collection
            'and attempt to obtain a match based on the unique identifier of the node
            If Not gbl_colNormalEnum(objNode.Key) Is Nothing Then
               Select Case objNode.Tag
                  Case "AMTimelineGroup"
                      Set gbl_objDexterObject = gbl_colNormalEnum(objNode.Key)
                  Case "AMTimelineComp"
                      Set gbl_objDexterObject = gbl_colNormalEnum(objNode.Key)
                  Case "AMTimelineTrack"
                      Set gbl_objDexterObject = gbl_colNormalEnum(objNode.Key)
                  Case "AMTimelineSrc"
                      Set gbl_objDexterObject = gbl_colNormalEnum(objNode.Key)
                  Case "AMTimelineTrans"
                      Set gbl_objDexterObject = gbl_colNormalEnum(objNode.Key)
                  Case "AMTimelineEffect"
                      Set gbl_objDexterObject = gbl_colNormalEnum(objNode.Key)
              End Select
            Else: Exit Sub
            End If
            
            'display a tooltip for the item
            If Not objNode Is Nothing Then
               If Trim(objNode.Text) <> vbNullString Then
                  tvwSimpleTree.ToolTipText = objNode.Text
               End If
            End If
            
            'update the listview with the item's information
            If Not gbl_colNormalEnum(objNode.Key) Is Nothing Then
               Select Case objNode.Tag
                  Case "AMTimeline"
                     Call RefreshListView(lstViewInfo, "AMTimeline", objNode.Key)
                     Call ViewTimelineInfo(lstViewInfo, gbl_objTimeline)
                     
                  Case "AMTimelineGroup"
                      Call RefreshListView(lstViewInfo, "AMTimelineGroup", objNode.Key)
                      Call ViewGroupInfo(lstViewInfo, gbl_objDexterObject)
                      
                  Case "AMTimelineComp"
                      Call RefreshListView(lstViewInfo, "AMTimelineComp", objNode.Key)
                      Call ViewCompositeInfo(lstViewInfo, gbl_objDexterObject)
                      
                  Case "AMTimelineTrack"
                      Call RefreshListView(lstViewInfo, "AMTimelineTrack", objNode.Key)
                      Call ViewTrackInfo(lstViewInfo, gbl_objDexterObject)
                      
                  Case "AMTimelineSrc"
                      Call RefreshListView(lstViewInfo, "AMTimelineSrc", objNode.Key)
                      Call ViewSourceInfo(lstViewInfo, gbl_objDexterObject)
                      
                  Case "AMTimelineTrans"
                      Call RefreshListView(lstViewInfo, "AMTimelineTrans", objNode.Key)
                      Call ViewTransitionInfo(lstViewInfo, gbl_objDexterObject)
                      
                  Case "AMTimelineEffect"
                      Call RefreshListView(lstViewInfo, "AMTimelineEffect", objNode.Key)
                      Call ViewEffectInfo(lstViewInfo, gbl_objDexterObject)
                      
              End Select
            Else: Exit Sub
            End If
            
            'derive the type of object given the tag of the node
            'and popup the menu for the given object type..
            If Button = 2 Then
               Select Case objNode.Tag
                   Case "AMTimeline"
                       PopupMenu mnuTimeline
                   Case "AMTimelineGroup"
                       PopupMenu mnuGroup
                   Case "AMTimelineComp"
                       PopupMenu mnuComp
                   Case "AMTimelineTrack"
                       PopupMenu mnuTrack
                   Case "AMTimelineSrc"
                       PopupMenu mnuClip
                   Case "AMTimelineTrans"
                       PopupMenu mnuTrans
                   Case "AMTimelineEffect"
                       PopupMenu mnuEffect
               End Select
            End If
            
            'clean-up & dereference
            If Not objNode Is Nothing Then Set objNode = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tvwSimpleTree_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tvwSimpleTree_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            If Me.MousePointer = 9 Then Me.MousePointer = vbDefault
            If Me.BackColor = vbBlack Then Me.BackColor = &H8000000F
            
            'disable tooltip if a hittest on the current position fails
            If tvwSimpleTree.HitTest(X, Y) Is Nothing Then tvwSimpleTree.ToolTipText = vbNullString
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tvwSimpleTree_NodeClick
            ' * procedure description:  Occurs when a Node object is clicked.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tvwSimpleTree_NodeClick(ByVal node As MSComctlLib.node)
            On Local Error GoTo ErrLine
            
            'display a tooltip for the item
            If Not node Is Nothing Then
               If Trim(node.Text) <> vbNullString Then
                  tvwSimpleTree.ToolTipText = node.Text
               End If
            End If
            
            'update the listview with the item's information
            If Not gbl_colNormalEnum(node.Key) Is Nothing Then
               Select Case node.Tag
                  Case "AMTimeline"
                     Call RefreshListView(lstViewInfo, "AMTimeline", node.Key)
                     Call ViewTimelineInfo(lstViewInfo, gbl_objTimeline)
                     
                  Case "AMTimelineGroup"
                      Call RefreshListView(lstViewInfo, "AMTimelineGroup", node.Key)
                      Call ViewGroupInfo(lstViewInfo, gbl_objDexterObject)
                      
                  Case "AMTimelineComp"
                      Call RefreshListView(lstViewInfo, "AMTimelineComp", node.Key)
                      Call ViewCompositeInfo(lstViewInfo, gbl_objDexterObject)
                      
                  Case "AMTimelineTrack"
                      Call RefreshListView(lstViewInfo, "AMTimelineTrack", node.Key)
                      Call ViewTrackInfo(lstViewInfo, gbl_objDexterObject)
                      
                  Case "AMTimelineSrc"
                      Call RefreshListView(lstViewInfo, "AMTimelineSrc", node.Key)
                      Call ViewSourceInfo(lstViewInfo, gbl_objDexterObject)
                      
                  Case "AMTimelineTrans"
                      Call RefreshListView(lstViewInfo, "AMTimelineTrans", node.Key)
                      Call ViewTransitionInfo(lstViewInfo, gbl_objDexterObject)
                      
                  Case "AMTimelineEffect"
                      Call RefreshListView(lstViewInfo, "AMTimelineEffect", node.Key)
                      Call ViewEffectInfo(lstViewInfo, gbl_objDexterObject)
                      
              End Select
            Else: Exit Sub
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tvwSimpleTree_OLEDragDrop
            ' * procedure description:  Occurs when data is dropped onto the control via an OLE drag/drop operation, and OLEDropMode is set to manual.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tvwSimpleTree_OLEDragDrop(Data As MSComctlLib.DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            
            'pass to the application drag drop handler
            Call AppOLEDragDrop(Data, Effect, Button, Shift, X, Y)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tvwSimpleTree_OLEDragOver
            ' * procedure description:  Occurs when the mouse is moved over the control during an OLE drag/drop operation, if its OLEDropMode property is set to manual.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tvwSimpleTree_OLEDragOver(Data As MSComctlLib.DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single, State As Integer)
            On Local Error GoTo ErrLine
            
            'pass to the application drag over handler
            Call AppOLEDragOver(Data, Effect, Button, Shift, X, Y, State)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub


' **************************************************************************************************************************************
' * PRIVATE INTERFACE- TOOLBAR EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: tbMain_ButtonClick
            ' * procedure description:  Occurs when the user clicks on a Button object in a Toolbar control.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tbMain_ButtonClick(ByVal Button As MSComctlLib.Button)
            On Local Error GoTo ErrLine
            
                 Select Case LCase(Button.Key)

                     Case "new"
                              'the functionality is identical to the file 'new' menu option
                              Call mnuFileNewTimeline_Click

                     Case "open"
                              'the functionality is identical to the file 'open' menu option
                              Call mnuFileOpen_Click

                     Case "save"
                              'the functionality is identical to the file 'saveas' menu option
                              Call mnuFileSaveAs_Click

                     Case "rewind"
                              'seek to the beggining of the media
                              Select Case gbl_objQuartzVB.State
                                   Case QTZStatusConstants.QTZStatusPlaying
                                            Call gbl_objQuartzVB.StopGraph
                                            gbl_objQuartzVB.Position = 0
                                            
                                   Case QTZStatusConstants.QTZStatusPaused
                                            gbl_objQuartzVB.Position = 0
                                            
                                   Case QTZStatusConstants.QTZStatusStopped
                                            gbl_objQuartzVB.Position = 0
                               End Select
                              
                              
                     Case "seekbackward"
                              'scrub backward by one second
                              Select Case gbl_objQuartzVB.State
                                   Case QTZStatusConstants.QTZStatusPlaying
                                            Call gbl_objQuartzVB.StopGraph
                                            gbl_objQuartzVB.Position = gbl_objQuartzVB.Position - 1
                                            
                                   Case QTZStatusConstants.QTZStatusPaused
                                            gbl_objQuartzVB.Position = gbl_objQuartzVB.Position - 1
                                            
                                   Case QTZStatusConstants.QTZStatusStopped
                                            gbl_objQuartzVB.Position = gbl_objQuartzVB.Position - 1
                               End Select
                              
                              
                     Case "play"
                               
                               'update the button(s)
                               With tbMain.Buttons
                                  .Item("New").Enabled = False
                                  .Item("Open").Enabled = False
                                  .Item("Save").Enabled = False
                                  .Item("Play").Enabled = True
                                  .Item("Pause").Enabled = True
                                  .Item("Stop").Enabled = True
                                  .Item("Rewind").Enabled = False
                                  .Item("FastForward").Enabled = False
                                  .Item("SeekForward").Enabled = False
                                  .Item("SeekBackward").Enabled = False
                               End With
                               
                               'update the state on the popup context menu
                               mnuTimeLinePlay.Enabled = False
                               mnuTimeLineStop.Enabled = True
                               mnuTimeLinePause.Enabled = True
                               mnuTimeLineRenderTimeLine.Enabled = False
                               mnuTimeLineClearRenderEngine.Enabled = False
                               
                               'play the timeline for the client
                               Select Case gbl_objQuartzVB.State
                                   Case QTZStatusConstants.QTZStatusPlaying
                                            Call gbl_objQuartzVB.StopGraph
                                            gbl_objQuartzVB.Position = 0
                                            Call gbl_objQuartzVB.RunGraph
                                            
                                   Case QTZStatusConstants.QTZStatusPaused
                                            Call gbl_objQuartzVB.RunGraph
                                            
                                   Case QTZStatusConstants.QTZStatusStopped
                                            'to prevent rendering out the timeline again (consequently resetting the media's
                                            'position to zero before restarting audio/video playback) comment the line below..
                                            Set gbl_objQuartzVB.FilterGraph = RenderTimeline(gbl_objTimeline)
                                            Call gbl_objQuartzVB.RunGraph
                               End Select
                               

                     Case "pause"
                               'update the button(s)
                               With tbMain.Buttons
                                  .Item("New").Enabled = True
                                  .Item("Open").Enabled = True
                                  .Item("Save").Enabled = True
                                  .Item("Play").Enabled = True
                                  .Item("Pause").Enabled = True
                                  .Item("Stop").Enabled = True
                                  .Item("Rewind").Enabled = True
                                  .Item("FastForward").Enabled = True
                                  .Item("SeekForward").Enabled = True
                                  .Item("SeekBackward").Enabled = True
                               End With
                               
                               'update the state on the popup context menu
                               mnuTimeLinePlay.Enabled = True
                               mnuTimeLineStop.Enabled = True
                               mnuTimeLinePause.Enabled = False
                               mnuTimeLineRenderTimeLine.Enabled = False
                               mnuTimeLineClearRenderEngine.Enabled = False
                               
                               'pause the timeline
                              Select Case gbl_objQuartzVB.State
                                   Case QTZStatusConstants.QTZStatusPlaying
                                            Call gbl_objQuartzVB.PauseGraph
                                            
                                   Case QTZStatusConstants.QTZStatusPaused
                                            Call gbl_objQuartzVB.PauseGraph
                                            
                                   Case QTZStatusConstants.QTZStatusStopped
                                            Call gbl_objQuartzVB.PauseGraph
                               End Select
                              
                              
                               
                               
                     Case "stop"
                              'update the button(s)
                               With tbMain.Buttons
                                  .Item("New").Enabled = True
                                  .Item("Open").Enabled = True
                                  .Item("Save").Enabled = True
                                  .Item("Play").Enabled = True
                                  .Item("Pause").Enabled = False
                                  .Item("Stop").Enabled = True
                                  .Item("Rewind").Enabled = False
                                  .Item("FastForward").Enabled = False
                                  .Item("SeekForward").Enabled = False
                                  .Item("SeekBackward").Enabled = False
                               End With
                               
                               'update the state on the popup context menu
                               mnuTimeLinePlay.Enabled = True
                               mnuTimeLineStop.Enabled = False
                               mnuTimeLinePause.Enabled = False
                               mnuTimeLineRenderTimeLine.Enabled = True
                               mnuTimeLineClearRenderEngine.Enabled = True
                               
                              'stop the timeline
                              Select Case gbl_objQuartzVB.State
                                   Case QTZStatusConstants.QTZStatusPlaying
                                            Call gbl_objQuartzVB.StopGraph
                                            
                                   Case QTZStatusConstants.QTZStatusPaused
                                            Call gbl_objQuartzVB.StopGraph
                                            
                                   Case QTZStatusConstants.QTZStatusStopped
                                            Call gbl_objQuartzVB.StopGraph
                               End Select
                              
                              
                     Case "seekforward"
                              'scrub forward by one second
                              Select Case gbl_objQuartzVB.State
                                   Case QTZStatusConstants.QTZStatusPlaying
                                            Call gbl_objQuartzVB.PauseGraph
                                            gbl_objQuartzVB.Position = gbl_objQuartzVB.Position + 1
                                            
                                   Case QTZStatusConstants.QTZStatusPaused
                                            Call gbl_objQuartzVB.PauseGraph
                                            gbl_objQuartzVB.Position = gbl_objQuartzVB.Position + 1
                                            
                                   Case QTZStatusConstants.QTZStatusStopped
                                            Call gbl_objQuartzVB.PauseGraph
                                            gbl_objQuartzVB.Position = gbl_objQuartzVB.Position + 1
                               End Select
                              
                              
                     Case "fastforward"
                              'seek to the end of the media
                              Select Case gbl_objQuartzVB.State
                                   Case QTZStatusConstants.QTZStatusPlaying
                                            Call gbl_objQuartzVB.PauseGraph
                                            gbl_objQuartzVB.Position = (gbl_objQuartzVB.StopTime - 0.01)
                                            
                                   Case QTZStatusConstants.QTZStatusPaused
                                            Call gbl_objQuartzVB.PauseGraph
                                            gbl_objQuartzVB.Position = (gbl_objQuartzVB.StopTime - 0.01)
                                            
                                   Case QTZStatusConstants.QTZStatusStopped
                                            Call gbl_objQuartzVB.PauseGraph
                                            gbl_objQuartzVB.Position = (gbl_objQuartzVB.StopTime - 0.01)
                               End Select
                 End Select
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tbMain_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tbMain_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            If Me.MousePointer = 9 Then Me.MousePointer = vbDefault
            If Me.BackColor = vbBlack Then Me.BackColor = &H8000000F
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tbMain_OLEDragDrop
            ' * procedure description:  Occurs when data is dropped onto the control via an OLE drag/drop operation, and OLEDropMode is set to manual.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tbMain_OLEDragDrop(Data As MSComctlLib.DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            
            'pass to the application drag drop handler
            Call AppOLEDragDrop(Data, Effect, Button, Shift, X, Y)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: tbMain_OLEDragOver
            ' * procedure description:  Occurs when the mouse is moved over the control during an OLE drag/drop operation, if its OLEDropMode property is set to manual.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tbMain_OLEDragOver(Data As MSComctlLib.DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single, State As Integer)
            On Local Error GoTo ErrLine
            
            'pass to the application drag over handler
            Call AppOLEDragOver(Data, Effect, Button, Shift, X, Y, State)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            

' **************************************************************************************************************************************
' * PRIVATE INTERFACE- COOLBAR EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: ctrlCoolBar_HeightChanged
            ' * procedure description:  Occurrs when the Coolbar control's Height changes, if its Orientation is horizontal.  Occurrs when the Coolbar control's Width changes, if its Orientation is vertical.
            ' *
            ' ******************************************************************************************************************************
            Private Sub ctrlCoolBar_HeightChanged(ByVal NewHeight As Single)
            On Local Error GoTo ErrLine
            'resize the container
            Call Form_Resize
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: ctrlCoolBar_MouseMove
            ' * procedure description:  Occurs when the user moves the mouse.
            ' *
            ' ******************************************************************************************************************************
            Private Sub ctrlCoolBar_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            If Me.MousePointer = 9 Then Me.MousePointer = vbDefault
            If Me.BackColor = vbBlack Then Me.BackColor = &H8000000F
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
                     
            
            ' ******************************************************************************************************************************
            ' * procedure name: ctrlCoolBar_OLEDragDrop
            ' * procedure description:  Occurs when data is dropped onto the control via an OLE drag/drop operation, and OLEDropMode is set to manual.
            ' *
            ' ******************************************************************************************************************************
            Private Sub ctrlCoolBar_OLEDragDrop(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            
            'pass to the application drag drop handler
            Call AppOLEDragDrop(Data, Effect, Button, Shift, X, Y)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
          
            ' ******************************************************************************************************************************
            ' * procedure name: ctrlCoolBar_OLEDragOver
            ' * procedure description:  Occurs when the mouse is moved over the control during an OLE drag/drop operation, if its OLEDropMode property is set to manual.
            ' *
            ' ******************************************************************************************************************************
            Private Sub ctrlCoolBar_OLEDragOver(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single, State As Integer)
            On Local Error GoTo ErrLine
            
            'pass to the application drag over handler
            Call AppOLEDragOver(Data, Effect, Button, Shift, X, Y, State)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
' **************************************************************************************************************************************
' * PRIVATE INTERFACE- APPLICATION METHODS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: AppOLEDragDrop
            ' * procedure description:  Occurs when data is dropped onto the control via an OLE drag/drop operation, and OLEDropMode is set to manual.
            ' *
            ' ******************************************************************************************************************************
            Private Sub AppOLEDragDrop(Data As MSComctlLib.DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single)
            Dim nCount As Long
            Dim bstrFileName As String
            Dim objLocalTimeline As AMTimeline
            On Local Error GoTo ErrLine
            
            'assign mouse state
            Screen.MousePointer = vbHourglass
            
            For nCount = 1 To Data.Files.Count
                  bstrFileName = Data.Files(nCount)
                  If Len(bstrFileName) > 4 Then
                        'verify the file extension is valid
                        If InStr(1, LCase(bstrFileName), ".xtl") > 0 Then
                           'at least it's been named an xtl file, proceed to attempt an import..
                           Set objLocalTimeline = New AMTimeline
                           Call RestoreTimeline(objLocalTimeline, bstrFileName, DEXImportXTL)
                           'verify restoration
                           If Not objLocalTimeline Is Nothing Then
                              'import succeeded; clean-up application scope
                              If Not gbl_objTimeline Is Nothing Then
                                 'dereference & clean-up timeline
                                 Call ClearTimeline(gbl_objTimeline)
                                 Set gbl_objTimeline = Nothing
                                 'dereference & clean-up rendering
                                If Not gbl_objQuartzVB Is Nothing Then Call gbl_objQuartzVB.StopGraph
                                If Not gbl_objFilterGraph Is Nothing Then Set gbl_objFilterGraph = Nothing
                                If Not gbl_objRenderEngine Is Nothing Then Call gbl_objRenderEngine.ScrapIt
                                If Not gbl_objRenderEngine Is Nothing Then Set gbl_objRenderEngine = Nothing
                              End If
                              'assign the local timeline to global scope
                              Set gbl_objTimeline = objLocalTimeline
                              'render the timeline and derive a filter graph manager
                              Set gbl_objFilterGraph = RenderTimeline(gbl_objTimeline)
                              'map the timeline to the userinterface
                              Call GetTimelineDirect(tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
                              mnuTimeLineClearRenderEngine.Enabled = False
                              'update the button(s)
                              With tbMain.Buttons
                                 .Item("New").Enabled = True
                                 .Item("Open").Enabled = True
                                 .Item("Save").Enabled = True
                                 .Item("Play").Enabled = True
                                 .Item("Pause").Enabled = False
                                 .Item("Stop").Enabled = False
                                 .Item("Rewind").Enabled = False
                                 .Item("FastForward").Enabled = False
                                 .Item("SeekForward").Enabled = False
                                 .Item("SeekBackward").Enabled = False
                              End With
                              'update the state on the popup context menu
                              mnuTimeLinePlay.Enabled = True
                              mnuTimeLineStop.Enabled = False
                              mnuTimeLinePause.Enabled = True
                              mnuTimeLineRenderTimeLine.Enabled = True
                              mnuTimeLineClearRenderEngine.Enabled = True
                              'reset module-level filename
                              gbl_bstrLoadFile = Data.Files(nCount)
                              'reset the caption on the application's main form
                              bstrFileName = Mid(bstrFileName, InStrRev(bstrFileName, "\") + 1)
                              Me.Caption = "DexterVB - " & bstrFileName
                           End If
                        End If
                        'clean-up & dereference
                        If Not objLocalTimeline Is Nothing Then
                           Set objLocalTimeline = Nothing
                           Exit For
                        End If
                  End If
            Next
            
            'reassign mouse state
            Screen.MousePointer = vbDefault
            Exit Sub
            
ErrLine:
            Err.Clear
            'reassign mouse state
            Screen.MousePointer = vbDefault
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: AppOLEDragOver
            ' * procedure description:  Occurs when the mouse is moved over the control during an OLE drag/drop operation, if its OLEDropMode property is set to manual.
            ' *
            ' ******************************************************************************************************************************
            Private Sub AppOLEDragOver(Data As MSComctlLib.DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single, State As Integer)
            Dim nCount As Long
            Dim bstrFileName As String
            On Local Error GoTo ErrLine
            
            'do not allow drag n' drop operations when rendering..
            If Not gbl_objQuartzVB Is Nothing Then
               If gbl_objQuartzVB.State = QTZStatusPlaying Then
                  Effect = vbDropEffectNone
                  Exit Sub
               End If
            End If
            
            For nCount = 1 To Data.Files.Count
                  bstrFileName = Data.Files(nCount)
                  If Len(bstrFileName) > 4 Then
                        'verify the file extension is valid
                        If InStr(1, LCase(bstrFileName), ".xtl") > 0 Then
                           'at least it's been named an xtl file, proceed to attempt an import..
                           Effect = vbDropEffectCopy
                        Else: If Effect <> vbDropEffectNone Then Effect = vbDropEffectNone
                        End If
                  End If
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
