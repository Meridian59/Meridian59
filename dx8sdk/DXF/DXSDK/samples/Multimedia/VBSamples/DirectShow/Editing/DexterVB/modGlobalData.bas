Attribute VB_Name = "modGlobalData"
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


' **************************************************************************************************************************************
' * GLOBAL INTERFACE- DATA
' *
' *
            'global timeline reference in application
            Global gbl_bstrLoadFile As String                        'the last file name/path opened by the user
            Global gbl_colNormalEnum As Collection            'collection for maintaining node/object relational data
            Global gbl_objQuartzVB As VBQuartzHelper        'helper object for rendering filtergraph's using quartz.dll
            Global gbl_objTimeline As AMTimeline               'global application timeline
            Global gbl_objRenderEngine As RenderEngine  'global application render engine
            Global gbl_objFilterGraph As FilgraphManager  'global application filtergraph manager
            Global gbl_objDexterObject As AMTimelineObj  'global application dexter object (current object in treeview)




' **************************************************************************************************************************************
' * PUBLIC INTERFACE- PROCEDURES
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: Main
            ' * procedure description:   Application Entry Point
            ' *
            ' ******************************************************************************************************************************
            Public Sub Main()
            Dim nStart As Long
            Dim nLength As Long
            Dim bstrFileName As String
            Dim boolDynamic As Boolean
            Dim boolPlayback As Boolean
            Dim boolSmartRecomp As Boolean
            Dim boolCloseOnComplete As Boolean
            Dim objLocalTimeline As AMTimeline
            On Local Error GoTo ErrLine
            
            
            'inhibit dupe instances of this application
            If App.PrevInstance = False Then
              'initalize global data
              Set gbl_objTimeline = New AMTimeline
              'display the main form for the application
              Load frmMain: frmMain.Move 0, 0: frmMain.Show: frmMain.Refresh
            Else: Exit Sub
            End If
            
            
            'handle command line
            If Command <> vbNullString Then
                  'check the command line argument(s) for a valid xtl filename with double quotes
                  If InStr(1, UCase(Command), Chr(34)) > 0 Then
                     nStart = InStr(1, UCase(Command), Chr(34))
                     nLength = InStr(nStart + 1, Command, Chr(34))
                     If nLength - nStart > Len(Command) Then
                        bstrFileName = Mid(Command, nStart, nLength)
                     End If
                     bstrFileName = Trim(LCase(Command))
                         bstrFileName = Replace(bstrFileName, "/r", vbNullString)
                         bstrFileName = Replace(bstrFileName, "/d", vbNullString)
                         bstrFileName = Replace(bstrFileName, Chr(34), vbNullString)
                         bstrFileName = Trim(bstrFileName)
                         If InStr(1, bstrFileName, ".xtl") > 0 Then
                            If InStr(1, bstrFileName, ".xtl") + 4 <> Len(bstrFileName) Then
                               bstrFileName = Left(bstrFileName, InStr(1, bstrFileName, ".xtl") + 4)
                            End If
                         End If
                  'check the command line argument(s) for a valid xtl filename with single quotes
                  ElseIf InStr(1, UCase(Command), Chr(39)) > 0 Then
                     nStart = InStr(1, UCase(Command), Chr(39))
                     nLength = InStr(nStart + 1, Command, Chr(39))
                     If nLength - nStart > Len(Command) Then
                        bstrFileName = Mid(Command, nStart, nLength)
                     End If
                     bstrFileName = Trim(LCase(Command))
                         bstrFileName = Replace(bstrFileName, "/r", vbNullString)
                         bstrFileName = Replace(bstrFileName, "/d", vbNullString)
                         bstrFileName = Replace(bstrFileName, Chr(39), vbNullString)
                         bstrFileName = Trim(bstrFileName)
                         If InStr(1, bstrFileName, ".xtl") > 0 Then
                            If InStr(1, bstrFileName, ".xtl") + 4 <> Len(bstrFileName) Then
                               bstrFileName = Left(bstrFileName, InStr(1, bstrFileName, ".xtl") + 4)
                            End If
                         End If
                  'check the command line argument(s) for a valid xtl filename with no quotes
                  Else
                         bstrFileName = Trim(LCase(Command))
                         bstrFileName = Replace(bstrFileName, "/r", vbNullString)
                         bstrFileName = Replace(bstrFileName, "/d", vbNullString)
                         bstrFileName = Trim(bstrFileName)
                         If InStr(1, bstrFileName, ".xtl") > 0 Then
                            If InStr(1, bstrFileName, ".xtl") + 4 <> Len(bstrFileName) Then
                               bstrFileName = Left(bstrFileName, InStr(1, bstrFileName, ".xtl") + 4)
                            End If
                         End If
                  End If
                  
                  'check the command line argument(s) for smart recomp optional
                  If InStr(1, UCase(Command), Space(1) & "/R") > 0 Then
                     boolSmartRecomp = True
                  End If
                  
                  'check the command line argument(s) for dynamic connections optional
                  If InStr(1, UCase(Command), Space(1) & "/D") > 0 Then
                     boolDynamic = True
                  End If
                  
                  'check the command line argument(s) for playback optional
                  If InStr(1, UCase(Command), Space(1) & "/P") > 0 Then
                     boolPlayback = True
                  End If
                  
                  'check the command line argument(s) for close when finished optional
                  If InStr(1, UCase(Command), Space(1) & "/C") > 0 Then
                     boolCloseOnComplete = True
                  End If
            End If
            
                  
               
            'proceed to load the xtl file into the application ide and render it
            If InStr(1, LCase(bstrFileName), ".xtl") > 0 Then
               'at least it's been named an xtl file, proceed to attempt an import..
               Set objLocalTimeline = New AMTimeline
               Call RestoreTimeline(objLocalTimeline, bstrFileName, DEXImportXTL)
               'verify restoration
               If Not objLocalTimeline Is Nothing Then
                  'import succeeded; clean-up global scope
                  If Not gbl_objTimeline Is Nothing Then
                     Call ClearTimeline(gbl_objTimeline)
                     Set gbl_objTimeline = Nothing
                  End If
                  'assign the local timeline to global scope
                  Set gbl_objTimeline = objLocalTimeline
                  'reset application-level filename
                  gbl_bstrLoadFile = bstrFileName
                  'reset the caption on the application's main form
                  bstrFileName = Mid(bstrFileName, InStrRev(bstrFileName, "\") + 1)
                  frmMain.Caption = "DexterVB - " & bstrFileName
               Else: Exit Sub
               End If
            End If
               
               
            'render the timeline and derive a filter graph manager
            Set gbl_objFilterGraph = RenderTimeline(gbl_objTimeline, boolDynamic, boolSmartRecomp)
            Set gbl_objQuartzVB.FilterGraph = gbl_objFilterGraph
            'map the timeline to the userinterface
            Call GetTimelineDirect(frmMain.tvwSimpleTree, gbl_objTimeline, gbl_colNormalEnum)
            frmMain.mnuTimeLineClearRenderEngine.Enabled = False
            'update the button(s)
            With frmMain.tbMain.Buttons
               .Item("Play").Image = 6
               .Item("Pause").Image = 7
               .Item("Stop").Image = 22
               .Item("Rewind").Image = 18
               .Item("FastForward").Image = 24
               .Item("SeekForward").Image = 23
               .Item("SeekBackward").Image = 19
               .Item("Play").Enabled = True
               .Item("Pause").Enabled = True
               .Item("Stop").Enabled = False
               .Item("Rewind").Enabled = False
               .Item("FastForward").Enabled = False
               .Item("SeekForward").Enabled = False
               .Item("SeekBackward").Enabled = False
            End With
            'update the state on the popup context menu
            frmMain.mnuTimeLinePlay.Enabled = True
            frmMain.mnuTimeLineStop.Enabled = False
            frmMain.mnuTimeLinePause.Enabled = True
            frmMain.mnuTimeLineRenderTimeLine.Enabled = False
            frmMain.mnuTimeLineClearRenderEngine.Enabled = False
            
            If boolPlayback Then
               'run the graph
               Call gbl_objFilterGraph.Run
               
               'if optional close on complete, unload when the timeline is finished rendering..
               If boolCloseOnComplete = True Then
                  Do Until gbl_objQuartzVB.Position = gbl_objQuartzVB.StopTime: DoEvents: DoEvents
                  If frmMain.Visible = False Then
                     Exit Do
                  End If
                  Loop
                  Unload frmMain: Set frmMain = Nothing: Exit Sub
               End If
            End If
            
            'clean-up & dereference
            If Not objLocalTimeline Is Nothing Then Set objLocalTimeline = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
