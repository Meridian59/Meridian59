VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "VBQuartzHelper"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
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

'IMediaControl is used for async rendering & control;
'It is a derivitive interface of the FilterGraph Manager.
Private m_objFilterGraphManager As IMediaControl


' **************************************************************************************************************************************
' * PUBLIC INTERFACE- ENUMERATIONS
' *
' *
            Public Enum QTZSynchronicityConstants
            QTZSynchronous = 0   'Synchronous execution
            QTZAsynchronous = 1 'Asynchronous execution
            End Enum
            
            Public Enum QTZStatusConstants
            QTZStatusStopped = 0 'State Stopped
            QTZStatusPaused = 1  'State Paused
            QTZStatusPlaying = 2  'State Playing
            End Enum


' **************************************************************************************************************************************
' * PUBLIC INTERFACE- PROPERTIES
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: FilterGraph
            ' * procedure description: Allows the client to get the encapsulated graph as a FilterGraph object
            ' *
            ' ******************************************************************************************************************************
            Public Property Get FilterGraph() As FilgraphManager
Attribute FilterGraph.VB_UserMemId = 0
Attribute FilterGraph.VB_MemberFlags = "200"
            On Local Error GoTo ErrLine
            If Not m_objFilterGraphManager Is Nothing Then
               Set FilterGraph = m_objFilterGraphManager
            End If
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: FilterGraph
            ' * procedure description: Allows the client to set a constructed FilterGraph object for which to render and control.
            ' *
            ' ******************************************************************************************************************************
            Public Property Set FilterGraph(RHS As FilgraphManager)
            On Local Error GoTo ErrLine
            If Not RHS Is Nothing Then
               Set m_objFilterGraphManager = RHS
            End If
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Position
            ' * procedure description: Allows the client to get the current position within the context of the media.
            ' *
            ' ******************************************************************************************************************************
            Public Property Get Position() As Double
            Dim objPosition As IMediaPosition
            On Local Error GoTo ErrLine
            If Not m_objFilterGraphManager Is Nothing Then
               'derive the position control interface
               Set objPosition = m_objFilterGraphManager
               'set the current position using this interface
               Position = CDbl(objPosition.CurrentPosition)
            End If
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Position
            ' * procedure description: Allows the client to set the current position within the context of the media.
            ' *
            ' ******************************************************************************************************************************
            Public Property Let Position(RHS As Double)
            Dim objPosition As IMediaPosition
            On Local Error GoTo ErrLine
            If Not m_objFilterGraphManager Is Nothing Then
               'derive the position control interface
               Set objPosition = m_objFilterGraphManager
               'set the current position using this interface
               objPosition.CurrentPosition = RHS
            End If
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Position
            ' * procedure description: Allows the client to get the current state of the playback.
            ' *
            ' ******************************************************************************************************************************
            Public Property Get State(Optional Timeout As Long = 1000) As QTZStatusConstants
            Dim nResultant As Long
            Dim objControl As IMediaControl
            On Local Error GoTo ErrLine
            If Not m_objFilterGraphManager Is Nothing Then
               'derive the position control interface
               Set objControl = m_objFilterGraphManager
               'get the current state using this interface
               Call objControl.GetState(Timeout, nResultant)
               'return to client
               State = nResultant
            End If
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
           
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Duration
            ' * procedure description: Allows the client to get the media's duration.
            ' *
            ' ******************************************************************************************************************************
            Public Property Get Duration() As Double
            Dim objPosition As IMediaPosition
            On Local Error GoTo ErrLine
            If Not m_objFilterGraphManager Is Nothing Then
               Set objPosition = m_objFilterGraphManager
               Duration = CDbl(objPosition.Duration)
            End If
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: FPS
            ' * procedure description: Allows the client to get the media's playback rate, in frames per second.
            ' *
            ' ******************************************************************************************************************************
            Public Property Get FPS() As Double
            Dim objPosition As IMediaPosition
            On Local Error GoTo ErrLine
            If Not m_objFilterGraphManager Is Nothing Then
               Set objPosition = m_objFilterGraphManager
               FPS = CDbl(objPosition.Rate)
            End If
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: StopTime
            ' * procedure description: Allows the client to get the media's StopTime (when the media will hault playback)
            ' *
            ' ******************************************************************************************************************************
            Public Property Get StopTime() As Double
            Dim objPosition As IMediaPosition
            On Local Error GoTo ErrLine
            If Not m_objFilterGraphManager Is Nothing Then
               Set objPosition = m_objFilterGraphManager
               StopTime = CDbl(objPosition.StopTime)
            End If
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property


' **************************************************************************************************************************************
' * PUBLIC INTERFACE- METHODS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: StopGraph
            ' * procedure description: Stop the rendering/playback of the graph.
            ' *
            ' ******************************************************************************************************************************
            Public Sub StopGraph()
            On Local Error GoTo ErrLine
            If Not m_objFilterGraphManager Is Nothing Then
              Call m_objFilterGraphManager.Stop
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: PauseGraph
            ' * procedure description: Pauses the rendering/playback of the graph.
            ' *
            ' ******************************************************************************************************************************
            Public Sub PauseGraph()
            On Local Error GoTo ErrLine
            If Not m_objFilterGraphManager Is Nothing Then
               Call m_objFilterGraphManager.Pause
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: RunGraph
            ' * procedure description:   Renders the graph.
            ' *
            ' ******************************************************************************************************************************
            Public Sub RunGraph(Optional Synchronicity As QTZSynchronicityConstants = QTZAsynchronous)
            Dim objPosition As IMediaPosition
            Dim objControl As IMediaControl
            On Local Error GoTo ErrLine
            
            If Not m_objFilterGraphManager Is Nothing Then
               Select Case Synchronicity
                       Case QTZSynchronicityConstants.QTZSynchronous
                                'run the graph
                                Call m_objFilterGraphManager.Run
                                'obtain the position of audio/video
                                Set objPosition = m_objFilterGraphManager
                                'loop with events
                                Do: DoEvents
                                        If objPosition.CurrentPosition = objPosition.StopTime Then
                                           Call m_objFilterGraphManager.Stop
                                           Exit Do
                                        End If
                                Loop
                    
                        Case QTZSynchronicityConstants.QTZAsynchronous
                                'the client desires to run the graph asynchronously
                                Call m_objFilterGraphManager.Run
                End Select
            End If
            'clean-up & dereference
            If Not objPosition Is Nothing Then Set objPosition = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub



' **************************************************************************************************************************************
' * PUBLIC INTERFACE- METHODS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: Class_Initialize
            ' * procedure description: fired intrinsically by visual basic, occurs when this class initalizes
            ' *
            ' ******************************************************************************************************************************
            Private Sub Class_Initialize()
            On Local Error GoTo ErrLine
            Set m_objFilterGraphManager = New FilgraphManager
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Class_Terminate
            ' * procedure description: fired intrinsically by visual basic, occurs when this class terminates
            ' *
            ' ******************************************************************************************************************************
            Private Sub Class_Terminate()
            On Local Error GoTo ErrLine
            If Not m_objFilterGraphManager Is Nothing Then Set m_objFilterGraphManager = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
