VERSION 5.00
Begin VB.Form frmMain 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "XtlTest"
   ClientHeight    =   1140
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   4470
   Icon            =   "frmMain.frx":0000
   LinkTopic       =   "Form1"
   LockControls    =   -1  'True
   MaxButton       =   0   'False
   ScaleHeight     =   1140
   ScaleWidth      =   4470
   StartUpPosition =   2  'CenterScreen
   Begin VB.CheckBox ChkDynamic 
      Caption         =   "Dynamic Sources"
      Height          =   255
      Left            =   960
      TabIndex        =   4
      Top             =   720
      Value           =   1  'Checked
      Width           =   1935
   End
   Begin VB.CommandButton CmdReplay 
      Caption         =   "&Replay"
      Enabled         =   0   'False
      Height          =   375
      Left            =   120
      TabIndex        =   2
      Top             =   720
      Width           =   735
   End
   Begin VB.Timer tmrTimer 
      Interval        =   1000
      Left            =   3900
      Top             =   675
   End
   Begin VB.PictureBox picDropBox 
      BackColor       =   &H00FFFFFF&
      Height          =   495
      Left            =   120
      OLEDropMode     =   1  'Manual
      ScaleHeight     =   435
      ScaleWidth      =   4155
      TabIndex        =   0
      Top             =   120
      Width           =   4215
      Begin VB.Label lblDragAndDrop 
         BackStyle       =   0  'Transparent
         Caption         =   "Drag and Drop an XTL file in the box."
         Enabled         =   0   'False
         Height          =   255
         Left            =   720
         TabIndex        =   3
         Top             =   120
         Width           =   2775
      End
      Begin VB.Label lbPlaying 
         Alignment       =   2  'Center
         BackStyle       =   0  'Transparent
         Caption         =   "P L A Y I N G"
         Enabled         =   0   'False
         BeginProperty Font 
            Name            =   "Comic Sans MS"
            Size            =   9.75
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   255
         Left            =   720
         TabIndex        =   1
         Top             =   120
         Visible         =   0   'False
         Width           =   2655
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

Private m_objTimeline As AMTimeline
Private m_objMediaEvent As IMediaEvent
Private m_objRenderEngine As RenderEngine



' **************************************************************************************************************************************
' * PRIVATE INTERFACE- FORM EVENTS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: Form_Terminate
            ' * procedure description:  Occurs when all references to an instance of a Form, MDIForm, or class are removed from memory.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_Terminate()
            On Local Error GoTo ErrLine
            
            'ensure timer disabled
            tmrTimer.Enabled = False
            
            'clean-up & dereference
            Call ClearTimeline(m_objTimeline)
            If Not m_objMediaEvent Is Nothing Then Set m_objMediaEvent = Nothing
            If Not m_objRenderEngine Is Nothing Then Set m_objRenderEngine = Nothing
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
            ' * procedure name: ChkDynamic_Click
            ' * procedure description: Occurs when the 'Dynamic' checkbox is elected by the user.
            ' *
            ' ******************************************************************************************************************************
            Private Sub ChkDynamic_Click()
            On Local Error GoTo ErrLine
            Call SetDynamicLevel(m_objRenderEngine)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: CmdReplay_Click
            ' * procedure description: Occurs when the 'Replay' command button is clicked by the user.
            ' *
            ' ******************************************************************************************************************************
            Private Sub CmdReplay_Click()
            Dim objVideoWindow As IVideoWindow
            Dim objMediaPosition As IMediaPosition
            Dim objFilterGraphManager As FilgraphManager
            On Local Error GoTo ErrLine
            
            ' if there's no render engine, there's nothing to replay
            If m_objRenderEngine Is Nothing Then Exit Sub
            
            ' ask for the graph, so we can control it
            Call m_objRenderEngine.GetFilterGraph(objFilterGraphManager)
            
            'if we have a valid instance of a filtergraph, run the graph
            If Not objFilterGraphManager Is Nothing Then
               Call objFilterGraphManager.Stop
               Set objMediaPosition = objFilterGraphManager
               If Not objMediaPosition Is Nothing Then objMediaPosition.CurrentPosition = 0
               Call objFilterGraphManager.Run
               Set m_objMediaEvent = objFilterGraphManager
            End If
            
            'set the UI state
            lbPlaying.Visible = True
            tmrTimer.Enabled = True
            lblDragAndDrop.Visible = False
            picDropBox.BackColor = &HFF
            
            If Not objFilterGraphManager Is Nothing Then
               'derive an interface for the video window
               Set objVideoWindow = objFilterGraphManager
                     If Not objVideoWindow Is Nothing Then
                        objVideoWindow.Visible = True
                        objVideoWindow.Left = 0
                        objVideoWindow.Top = 0
                     End If
            End If
            
            'clean-up & dereference
            If Not objVideoWindow Is Nothing Then Set objVideoWindow = Nothing
            If Not objMediaPosition Is Nothing Then Set objMediaPosition = Nothing
            If Not objFilterGraphManager Is Nothing Then Set objFilterGraphManager = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub


            ' ******************************************************************************************************************************
            ' * procedure name: picDropBox_OLEDragDrop
            ' * procedure description:  Occurs when data is dropped onto the control via an OLE drag/drop operation,
            ' *                                       and OLEDropMode is set to manual.
            ' *                                       Here we dropped an XTL file on the timeline, so create a timeline, a render engine,
            ' *                                       an XML parser, and load them all up
            ' ******************************************************************************************************************************
            Private Sub picDropBox_OLEDragDrop(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single)
            Dim nCount As Long
            Dim bstrFileName As String
            Dim objXMLParser As New Xml2Dex
            On Local Error GoTo ErrLine
            
            'ensure that among the files being dragged is an xtl file..
            For nCount = 1 To Data.Files.Count
                  If Len(Data.Files.Item(nCount)) > 4 Then
                     If LCase(Right(Data.Files.Item(nCount), 4)) = ".xtl" Then
                        Effect = vbDropEffectCopy
                        bstrFileName = Data.Files(nCount)
                        Exit For
                     End If
                  End If
            Next
            'otherwise do not allow the drag operation to continue
            If bstrFileName = vbNullString Then
               Effect = vbDropEffectNone: Exit Sub
            End If
            
            'ensure timer is disabled
            tmrTimer.Enabled = False
            
            'clean-up & dereference
            Call ClearTimeline(m_objTimeline)
            If Not m_objMediaEvent Is Nothing Then Set m_objMediaEvent = Nothing
            If Not m_objRenderEngine Is Nothing Then Set m_objRenderEngine = Nothing
            
            
            'reinstantiate the timeline & render engine
            Set m_objTimeline = New AMTimeline
            Set m_objRenderEngine = New RenderEngine
            
            'Set the dynamic level on or off
            Call SetDynamicLevel(m_objRenderEngine)
            
            'read in the file
            Call objXMLParser.ReadXMLFile(m_objTimeline, bstrFileName)
            
            ' make sure all the sources exist where they should
            ' the 27 is a combination of flags from qedit.idl (c/c++ stuff)
            m_objTimeline.ValidateSourceNames 27, Nothing, vbNull
            
            'set the timeline
            m_objRenderEngine.SetTimelineObject m_objTimeline
            
            'connect the front
            m_objRenderEngine.ConnectFrontEnd
            
            'render the output pins (e.g. 'backend')
            m_objRenderEngine.RenderOutputPins
            
            'set the caption on the form & enable replay there after
            frmMain.CmdReplay.Enabled = True
            frmMain.Caption = "XtlTest -" + bstrFileName
            
            'replay the timeline
            Call CmdReplay_Click
            
            'clean-up & dereference
            If Not objXMLParser Is Nothing Then Set objXMLParser = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub


            ' ******************************************************************************************************************************
            ' * procedure name: picDropBox_OLEDragOver
            ' * procedure description:  Occurs when the mouse is moved over the control during an OLE drag/drop operation, if its OLEDropMode property is set to manual.
            ' *
            ' ******************************************************************************************************************************
            Private Sub picDropBox_OLEDragOver(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single, State As Integer)
            Dim nCount As Long
            On Local Error GoTo ErrLine
            
            'ensure that among the files being dragged is an xtl file..
            For nCount = 1 To Data.Files.Count
                  If Len(Data.Files.Item(nCount)) > 4 Then
                     If LCase(Right(Data.Files.Item(nCount), 4)) = ".xtl" Then
                        Effect = vbDropEffectCopy
                        Exit Sub
                     End If
                  End If
            Next
            
            'otherwise do not allow the drag operation to continue
            Effect = vbDropEffectNone
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub


            ' ******************************************************************************************************************************
            ' * procedure name: picDropBox_OLEGiveFeedback
            ' * procedure description:  Occurs at the source control of an OLE drag/drop operation when the mouse cursor needs to be changed.
            ' *
            ' ******************************************************************************************************************************
            Private Sub picDropBox_OLEGiveFeedback(Effect As Long, DefaultCursors As Boolean)
            On Local Error GoTo ErrLine
            
            'setup the ole drop effect
            Effect = vbDropEffectCopy
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub

            
            ' ******************************************************************************************************************************
            ' * procedure name: tmrTimer_Timer
            ' * procedure description:  Occurs when a preset interval for a Timer control has elapsed.
            ' *
            ' ******************************************************************************************************************************
            Private Sub tmrTimer_Timer()
            Dim nResultant As Long
            Dim objVideoWindow As IVideoWindow
            Dim objMediaPosition As IMediaPosition
            Dim objFilterGraphManager As FilgraphManager
            On Local Error GoTo ErrLine
            
            
            If Not m_objMediaEvent Is Nothing Then
               'wait 10 ms to see if it's running or not
               Call m_objMediaEvent.WaitForCompletion(10, nResultant)
              
               'derive an instance of the video window
               Set objVideoWindow = m_objMediaEvent
               
               If objVideoWindow.Visible = False Then
                   'the user closed the video window, hault playback
                    If Not m_objRenderEngine Is Nothing Then
                       Call m_objRenderEngine.GetFilterGraph(objFilterGraphManager)
                    End If
                    'the end of the media has been reached
                    lbPlaying.Visible = False
                    lblDragAndDrop.Visible = True
                    objVideoWindow.Visible = False
                    picDropBox.BackColor = &HFFFFFF
                    
                    'if we have a valid instance of a filtergraph, run the graph
                    If Not objFilterGraphManager Is Nothing Then
                       Call objFilterGraphManager.Stop
                       Set objMediaPosition = objFilterGraphManager
                       If Not objMediaPosition Is Nothing Then objMediaPosition.CurrentPosition = 0
                       Set m_objMediaEvent = objFilterGraphManager
                    End If
            
               ElseIf nResultant <> 1 Then ' 1 = EC_COMPLETE
                  'the end of the media has not been reached, exit
                  Exit Sub
               Else
                  'the end of the media has been reached
                  lbPlaying.Visible = False
                  lblDragAndDrop.Visible = True
                  objVideoWindow.Visible = False
                  picDropBox.BackColor = &HFFFFFF
               End If
            End If
            
            'clean-up & dereference
            If Not objVideoWindow Is Nothing Then Set objVideoWindow = Nothing
            If Not objMediaPosition Is Nothing Then Set objMediaPosition = Nothing
            If Not objFilterGraphManager Is Nothing Then Set objFilterGraphManager = Nothing
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
            ' * procedure name: ClearTimeline
            ' * procedure description:  Clear everything out so we can start over or exit
            ' *
            ' ******************************************************************************************************************************
            Private Sub ClearTimeline(objTimeline As AMTimeline)
            On Local Error GoTo ErrLine
            
            ' we need to call this manually, since groups
            ' themselves can have a circular reference back to the timeline
            If Not objTimeline Is Nothing Then
               Call objTimeline.ClearAllGroups: Set objTimeline = Nothing
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: SetDynamicLevel
            ' * procedure description:  we can either make the sources load before the project runs, or let them load when needed.
            ' *
            ' ******************************************************************************************************************************
            Private Sub SetDynamicLevel(objRenderEngine As RenderEngine)
            On Local Error GoTo ErrLine
            
            'proceed to set the dynamic reconnection level on the given render engine
            If Not objRenderEngine Is Nothing Then
               If ChkDynamic.Value Then
                   objRenderEngine.SetDynamicReconnectLevel 1
               Else: objRenderEngine.SetDynamicReconnectLevel 0
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
