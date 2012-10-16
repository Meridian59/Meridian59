Attribute VB_Name = "modDexter"
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

'user for async rendering procedures
Private m_objMediaEvent As IMediaEvent
Private m_objFilterGraph As IGraphBuilder
Private m_objRenderEngine As RenderEngine
Private m_objFilterGraphManager As New FilgraphManager


' **************************************************************************************************************************************
' * PUBLIC INTERFACE- ENUMERATIONS
' *
' *
            'supported export formats
            Public Enum DEXExportFormatEnum
            DEXExportXTL = 0
            DEXExportGRF = 1
            End Enum

            'supported import formats
            Public Enum DEXImportFormatEnum
            DEXImportXTL = 0
            End Enum
            
            'supported media groups
            Public Enum DEXMediaTypeEnum
            DEXMediaTypeAudio = 1
            DEXMediaTypeVideo = 0
            End Enum
            
            

' **************************************************************************************************************************************
' * PUBLIC INTERFACE- DEXTER PROCEDURES
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: ClearTimeline
            ' * procedure description: purges the given timeline of all groups
            ' *                                      NOTE: YOU MUST CALL THIS ON ANY AMTIMELINES YOU HAVE BEFORE RELEASING
            ' *                                      THEM (e.g. BEFORE YOUR APP SHUTS DOWN) OR SO AS TO FREE MEMORY RESOURCES
            ' ******************************************************************************************************************************
            Public Sub ClearTimeline(objTimeline As AMTimeline)
            On Local Error GoTo ErrLine
             
            If Not objTimeline Is Nothing Then
               Call objTimeline.ClearAllGroups
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: SaveTimeline
            ' * procedure description:  Persists a timeline to a file given the specified format
            ' *
            ' ******************************************************************************************************************************
            Public Sub SaveTimeline(objTimeline As AMTimeline, bstrFileName As String, Optional Format As DEXExportFormatEnum = DEXExportXTL)
            Dim objXml2Dex As Xml2Dex
            Dim objFilterGraph As IGraphBuilder
            Dim objRenderEngine As RenderEngine
            On Local Error GoTo ErrLine
            
            If Not objTimeline Is Nothing Then
               Select Case LCase(Format)
                    Case DEXExportFormatEnum.DEXExportXTL
                        'Persist the timeline using the dexter XTL File Format
                        Set objXml2Dex = New Xml2Dex
                        objXml2Dex.WriteXMLFile objTimeline, bstrFileName
                        
                    Case DEXExportFormatEnum.DEXExportGRF
                        'Persist the timeline to a DShow Filter Graph Format
                        Set objXml2Dex = New Xml2Dex
                        Set objRenderEngine = New RenderEngine
                        Call objRenderEngine.SetTimelineObject(objTimeline)
                        Call objRenderEngine.ConnectFrontEnd
                        Call objRenderEngine.RenderOutputPins
                        Call objRenderEngine.GetFilterGraph(objFilterGraph)
                        objXml2Dex.WriteGrfFile objFilterGraph, bstrFileName
               End Select
            End If
            
            'clean-up & dereference
            If Not objXml2Dex Is Nothing Then Set objXml2Dex = Nothing
            If Not objFilterGraph Is Nothing Then Set objFilterGraph = Nothing
            If Not objRenderEngine Is Nothing Then Set objRenderEngine = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub


            
            ' ******************************************************************************************************************************
            ' * procedure name: RestoreTimeline
            ' * procedure description:  Restores a timeline from a file given the specified format
            ' *
            ' ******************************************************************************************************************************
            Public Sub RestoreTimeline(objTimeline As AMTimeline, bstrFileName As String, Optional Format As DEXImportFormatEnum = DEXImportXTL)
            Dim objXml2Dex As Xml2Dex
            On Local Error GoTo ErrLine
            
            If Not objTimeline Is Nothing Then
               Select Case LCase(Format)
                    Case DEXImportFormatEnum.DEXImportXTL
                        'restore the timeline from a dexter XTL File Format
                        Set objXml2Dex = New Xml2Dex
                        Call objXml2Dex.ReadXMLFile(objTimeline, bstrFileName)
               End Select
            End If
            
            'clean-up & dereference
            If Not objXml2Dex Is Nothing Then Set objXml2Dex = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: CreateTimeline
            ' * procedure description:  creates a AMTimeline object
            ' *
            ' ******************************************************************************************************************************
            Public Function CreateTimeline() As AMTimeline
            On Local Error GoTo ErrLine
            'instantiate return value direct
            Set CreateTimeline = New AMTimeline
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: CreateGroup
            ' * procedure description:  creates a group object given the passed properties (group name & mediatype)  on the given timeline
            ' *                                       groups can only be inserted into a timeline; so you could use this function with 'InsertGroup' typically
            ' ******************************************************************************************************************************
            Public Function CreateGroup(objTimeline As AMTimeline, bstrGroupName As String, MediaType As DEXMediaTypeEnum, Optional OutputFPS As Double = 15, Optional PreviewMode As Long, Optional OutputBuffer As Long = 32) As AMTimelineGroup
            Dim objGroup As AMTimelineGroup
            Dim objTimelineObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
            'create an empty node on the timeline
            objTimeline.CreateEmptyNode objTimelineObject, TIMELINE_MAJOR_TYPE_GROUP
            'derive the group interface
            Set objGroup = objTimelineObject
            'set the name of the group
            Call objGroup.SetGroupName(bstrGroupName)
            'set the media type for the group
            Call objGroup.SetMediaTypeForVB(MediaType)
            'set the output buffer for the group
            Call objGroup.SetOutputBuffering(OutputBuffer)
            'set the preview mode for the group
            Call objGroup.SetPreviewMode(PreviewMode)
            'set the output fps for the group
            Call objGroup.SetOutputFPS(OutputFPS)
            'return the group to the client
            Set CreateGroup = objGroup
            'clean-up & dereference
            If Not objGroup Is Nothing Then Set objGroup = Nothing
            If Not objTimelineObject Is Nothing Then Set objTimelineObject = Nothing
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: CreateComposite
            ' * procedure description: Creates a Composite object on the given timeline
            ' *
            ' ******************************************************************************************************************************
            Public Function CreateComposite(objTimeline As AMTimeline) As AMTimelineComp
            Dim objComp As AMTimelineComp
            Dim objTimelineObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
            'create an empty node on the timeline
            objTimeline.CreateEmptyNode objTimelineObject, TIMELINE_MAJOR_TYPE_COMPOSITE
            'derive the composite interface
            Set objComp = objTimelineObject
            'return the group to the client
            Set CreateComposite = objComp
            'clean-up & dereference
            If Not objComp Is Nothing Then Set objComp = Nothing
            If Not objTimelineObject Is Nothing Then Set objTimelineObject = Nothing
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: CreateTrack
            ' * procedure description: Create a track object on the given timeline
            ' *
            ' ******************************************************************************************************************************
            Public Function CreateTrack(objTimeline As AMTimeline) As AMTimelineTrack
            Dim objTrack As AMTimelineTrack
            Dim objTimelineObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
            'create an empty node on the timeline
            objTimeline.CreateEmptyNode objTimelineObject, TIMELINE_MAJOR_TYPE_TRACK
            'derive the track interface
            Set objTrack = objTimelineObject
            'return the track to the client
            Set CreateTrack = objTrack
            'clean-up & dereference
            If Not objTrack Is Nothing Then Set objTrack = Nothing
            If Not objTimelineObject Is Nothing Then Set objTimelineObject = Nothing
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: CreateEffect
            ' * procedure description: creates an effect object on the given timeline
            ' *
            ' ******************************************************************************************************************************
            Public Function CreateEffect(objTimeline As AMTimeline) As AMTimelineEffect
            Dim objEffect As AMTimelineEffect
            Dim objTimelineObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
            'create an empty node on the timeline
            objTimeline.CreateEmptyNode objTimelineObject, TIMELINE_MAJOR_TYPE_EFFECT
            'derive the effect interface
            Set objEffect = objTimelineObject
            'return the group to the client
            Set CreateEffect = objEffect
            'clean-up & dereference
            If Not objEffect Is Nothing Then Set objEffect = Nothing
            If Not objTimelineObject Is Nothing Then Set objTimelineObject = Nothing
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: CreateTransition
            ' * procedure description: creates a transition object on the given timeline
            ' *
            ' ******************************************************************************************************************************
            Public Function CreateTransition(objTimeline As AMTimeline) As AMTimelineTrans
            Dim objTrans As AMTimelineTrans
            Dim objTimelineObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
            'create an empty node on the timeline
            objTimeline.CreateEmptyNode objTimelineObject, TIMELINE_MAJOR_TYPE_TRANSITION
            'derive the effect interface
            Set objTrans = objTimelineObject
            'return the group to the client
            Set CreateTransition = objTrans
            'clean-up & dereference
            If Not objTrans Is Nothing Then Set objTrans = Nothing
            If Not objTimelineObject Is Nothing Then Set objTimelineObject = Nothing
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: CreateSource
            ' * procedure description: creates a clip/source object on the given timeline
            ' *
            ' ******************************************************************************************************************************
            Public Function CreateSource(objTimeline As AMTimeline) As AMTimelineSrc
            Dim objSrc As AMTimelineSrc
            Dim objTimelineObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
            'create an empty node on the timeline
            objTimeline.CreateEmptyNode objTimelineObject, TIMELINE_MAJOR_TYPE_SOURCE
            'derive the source interface
            Set objSrc = objTimelineObject
            'return the source to the client
            Set CreateSource = objSrc
            'clean-up & dereference
            If Not objSrc Is Nothing Then Set objSrc = Nothing
            If Not objTimelineObject Is Nothing Then Set objTimelineObject = Nothing
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: GroupFromTimeline
            ' * procedure description:  Returns a group object given the passed arguments (timeline & group)
            ' *                                       Groups can only be inserted into a timeline; use this function with 'InsertGroup'
            ' ******************************************************************************************************************************
            Public Function GroupFromTimeline(objTimeline As AMTimeline, Optional Group As Long = 0) As AMTimelineGroup
            Dim objGroup As AMTimelineGroup
            Dim objTimelineObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
            'obtain a Timeline Object from the timeline
            Call objTimeline.GetGroup(objTimelineObject, Group)
            'derive the group interface from the timeline object
            Set objGroup = objTimelineObject
            'returnt the reference to the client
            Set GroupFromTimeline = objGroup
            'clean-up & dereference
            If Not objGroup Is Nothing Then Set objGroup = Nothing
            If Not objTimelineObject Is Nothing Then Set objTimelineObject = Nothing
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: InsertGroup
            ' * procedure description: appends a group to a timeline object; you can only append groups to a timeline
            ' *
            ' ******************************************************************************************************************************
            Public Sub InsertGroup(objDestTimeline As AMTimeline, objSourceGroup As AMTimelineGroup)
            Dim objTimelineObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
            If Not objSourceGroup Is Nothing Then
               If Not objDestTimeline Is Nothing Then
                  'query for the Timelineobj interface
                  Set objTimelineObject = objSourceGroup
                  'append the source group to the destination timeline
                  objDestTimeline.AddGroup objTimelineObject
                  'clean-up & dereference
                  If Not objTimelineObject Is Nothing Then Set objTimelineObject = Nothing
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: InsertComposite
            ' * procedure description: Inserts Composite into a group or into another composite,
            ' *                                      The second argument, objInsetDestination evaluates to either a group or a composite object
            ' ******************************************************************************************************************************
            Public Sub InsertComposite(objSourceComposite As AMTimelineComp, objInsetDestination As AMTimelineObj, Optional Priority As Long = -1)
            Dim objComp As AMTimelineComp
            Dim objTimelineObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
            If Not objSourceComposite Is Nothing Then
               If Not objInsetDestination Is Nothing Then
                  'query for the composite interface
                  Set objComp = objInsetDestination
                  'query for the timelineobj object
                  Set objTimelineObject = objSourceComposite
                  'insert the comp into the group; or comp & set the priority
                  Call objComp.VTrackInsBefore(objTimelineObject, Priority)
                  'clean-up & dereference
                  If Not objComp Is Nothing Then Set objComp = Nothing
                  If Not objTimelineObject Is Nothing Then Set objTimelineObject = Nothing
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: InsertTrack
            ' * procedure description: Inserts a track into a group or a composite,
            ' *                                      The second argument, objInsetDestination evaluates to either a group or a composite
            ' ******************************************************************************************************************************
            Public Sub InsertTrack(objTrack As AMTimelineTrack, objInsetDestination As AMTimelineObj, Optional Priority As Long = -1)
            Dim objComp As AMTimelineComp
            Dim objTimelineObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
            If Not objTrack Is Nothing Then
               If Not objInsetDestination Is Nothing Then
                  'query for the composite interface
                  Set objComp = objInsetDestination
                  'query for the timelineobj object
                  Set objTimelineObject = objTrack
                  'insert the comp into the group; or comp & set the priority
                  Call objComp.VTrackInsBefore(objTimelineObject, Priority)
                  'clean-up & dereference
                  If Not objComp Is Nothing Then Set objComp = Nothing
                  If Not objTimelineObject Is Nothing Then Set objTimelineObject = Nothing
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: InsertEffect
            ' * procedure description: appends an effect to a timeline object
            ' *
            ' ******************************************************************************************************************************
            Public Sub InsertEffect(objSourceEffect As AMTimelineEffect, objInsetDestination As AMTimelineObj, bstrEffectCLSID As String, dblTStart As Double, dblTStop As Double, Optional Priority As Long = -1)
            Dim objTimelineObject As AMTimelineObj
            Dim objTimelineEffectable As IAMTimelineEffectable
            On Local Error GoTo ErrLine
            
            If Not objSourceEffect Is Nothing Then
               If Not objInsetDestination Is Nothing Then
                  'query for the timelineobj object
                  Set objTimelineObject = objSourceEffect
                  Call objTimelineObject.SetSubObjectGUIDB(bstrEffectCLSID)
                  Call objTimelineObject.SetStartStop2(dblTStart, dblTStop)
                  'insert the effect into the destination
                  Set objTimelineEffectable = objInsetDestination
                  Call objTimelineEffectable.EffectInsBefore(objTimelineObject, Priority)
                  'clean-up & dereference
                  If Not objTimelineObject Is Nothing Then Set objTimelineObject = Nothing
                  If Not objTimelineEffectable Is Nothing Then Set objTimelineEffectable = Nothing
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
        
            
            ' ******************************************************************************************************************************
            ' * procedure name: InsertSource
            ' * procedure description: inserts a source clip to a timeline object; you can only append source to a track
            ' *
            ' ******************************************************************************************************************************
            Public Sub InsertSource(objDestTrack As AMTimelineTrack, objSourceClip As AMTimelineSrc, bstrMediaName As String, dblTStart As Double, dblTStop As Double, Optional dblMStart As Double, Optional dblMStop As Double)
            Dim objTimelineObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
            If Not objDestTrack Is Nothing Then
               If Not objSourceClip Is Nothing Then
                  'set the media name
                  Call objSourceClip.SetMediaName(bstrMediaName)
                  'query for the Timelineobj interface
                  Set objTimelineObject = objSourceClip
                  'set start/stop times
                  Call objTimelineObject.SetStartStop2(dblTStart, dblTStop)
                  If dblMStart >= 0 And dblMStop <> 0 Then
                     'set the media times
                     Call objSourceClip.SetMediaTimes2(dblMStart, dblMStop)
                  End If
                  'append the source clip to the destination track
                  objDestTrack.SrcAdd objTimelineObject
                  'clean-up & dereference
                  If Not objTimelineObject Is Nothing Then Set objTimelineObject = Nothing
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: InsertTransition
            ' * procedure description: appends a transition to a timeline object
            ' *
            ' ******************************************************************************************************************************
            Public Sub InsertTransition(objSourceTransition As AMTimelineTrans, objInsetDestination As AMTimelineObj, bstrEffectCLSID As String, dblTStart As Double, dblTStop As Double, Optional Priority As Long = -1)
            Dim objTimelineObject As AMTimelineObj
            Dim objTimelineTransable As IAMTimelineTransable
            On Local Error GoTo ErrLine
            
            If Not objSourceTransition Is Nothing Then
               If Not objInsetDestination Is Nothing Then
                  'query for the timelineobj object
                  Set objTimelineObject = objSourceTransition
                  Call objTimelineObject.SetSubObjectGUIDB(bstrEffectCLSID)
                  Call objTimelineObject.SetStartStop2(dblTStart, dblTStop)
                  'insert the transition into the destination
                  Set objTimelineTransable = objInsetDestination
                  Call objTimelineTransable.TransAdd(objTimelineObject)
                  'clean-up & dereference
                  If Not objTimelineObject Is Nothing Then Set objTimelineObject = Nothing
                  If Not objTimelineTransable Is Nothing Then Set objTimelineTransable = Nothing
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: EngineFromTimeline
            ' * procedure description: renders the timeline for the client
            ' *
            ' ******************************************************************************************************************************
            Public Function EngineFromTimeline(objTimeline As AMTimeline) As RenderEngine
            Dim objRenderEngine As RenderEngine
            On Local Error GoTo ErrLine
            
            'instantiate new render engine
            Set objRenderEngine = New RenderEngine
            
            'connect everything up..
            Call objRenderEngine.SetTimelineObject(objTimeline)
            objRenderEngine.ConnectFrontEnd
            objRenderEngine.RenderOutputPins
            
            'return the render engine to the client
            Set EngineFromTimeline = objRenderEngine
            
            'dereference & clean-up
            If Not objRenderEngine Is Nothing Then Set objRenderEngine = Nothing
            Exit Function
            
ErrLine:

            Err.Clear
            Exit Function
            End Function
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: GraphFromTimeline
            ' * procedure description: returns a graph from the given timeline
            ' *
            ' ******************************************************************************************************************************
            Public Function GraphFromTimeline(objTimeline As AMTimeline) As IGraphBuilder
            Dim objGraphBuilder As IGraphBuilder
            Dim objRenderEngine As RenderEngine
            On Local Error GoTo ErrLine
            
            'instantiate new render engine
            Set objRenderEngine = New RenderEngine
            
            'connect everything up..
            Call objRenderEngine.SetTimelineObject(objTimeline)
            objRenderEngine.ConnectFrontEnd
            objRenderEngine.RenderOutputPins
            
            'return the graph builder to the client
            Call objRenderEngine.GetFilterGraph(objGraphBuilder)
            If Not objGraphBuilder Is Nothing Then Set GraphFromTimeline = objGraphBuilder
            
            'dereference & clean-up
            If Not objGraphBuilder Is Nothing Then Set objGraphBuilder = Nothing
            If Not objRenderEngine Is Nothing Then Set objRenderEngine = Nothing
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: RunFilterGraphSync
            ' * procedure description: playsback the filtergraph for the client synchronously, and returns.
            ' *
            ' ******************************************************************************************************************************
            Public Sub RunFilterGraphSync(objGraph As IFilterGraph)
            Dim nExitCode As Long
            Dim objMediaEvent As IMediaEvent
            Dim objMediaControl As IMediaControl
            On Local Error GoTo ErrLine
            
            'obtain the media control, event
            Set objMediaEvent = objGraph
            Set objMediaControl = objGraph
            
            'render the graph
            objMediaControl.Run
            'wait for play to complete..
            objMediaEvent.WaitForCompletion -1, nExitCode
            
            'clean-up & dereference
            If Not objMediaEvent Is Nothing Then Set objMediaEvent = Nothing
            If Not objMediaControl Is Nothing Then Set objMediaControl = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
           
            
            ' ******************************************************************************************************************************
            ' * procedure name: TransitionCLSIDToFriendlyName
            ' * procedure description: returns the localized friendly name of a transition given it's CLSID
            ' *
            ' ******************************************************************************************************************************
             Public Function TransitionCLSIDToFriendlyName(bstrTransitionCLSID As String, Optional bstrLanguage As String = "EN-US") As String
             Dim bstrReturn As String
             On Local Error GoTo ErrLine
             
             If UCase(bstrLanguage) = "EN-US" Then
                         Select Case bstrTransitionCLSID
                            Case "{C3BDF740-0B58-11d2-A484-00C04F8EFB69}"
                                bstrReturn = "Barn"
                            Case "{00C429C0-0BA9-11d2-A484-00C04F8EFB69}"
                                bstrReturn = "Blinds"
                            Case "{107045D1-06E0-11D2-8D6D-00C04F8EF8E0}"
                                bstrReturn = "BurnFilm"
                            Case "{AA0D4D0C-06A3-11D2-8F98-00C04FB92EB7}"
                                bstrReturn = "CenterCurls"
                            Case "{2A54C908-07AA-11D2-8D6D-00C04F8EF8E0}"
                                bstrReturn = "ColorFade"
                            Case "{9A43A844-0831-11D1-817F-0000F87557DB}"
                                bstrReturn = "Compositor"
                            Case "{AA0D4D0E-06A3-11D2-8F98-00C04FB92EB7}"
                                bstrReturn = "Curls"
                            Case "{AA0D4D12-06A3-11D2-8F98-00C04FB92EB7}"
                                bstrReturn = "Curtains"
                            Case "{16B280C5-EE70-11D1-9066-00C04FD9189D}"
                                bstrReturn = "Fade"
                            Case "{107045CC-06E0-11D2-8D6D-00C04F8EF8E0}"
                                bstrReturn = "FadeWhite"
                            Case "{2A54C90B-07AA-11D2-8D6D-00C04F8EF8E0}"
                                bstrReturn = "FlowMotion"
                            Case "{2A54C913-07AA-11D2-8D6D-00C04F8EF8E0}"
                                bstrReturn = "GlassBlock"
                            Case "{2A54C911-07AA-11D2-8D6D-00C04F8EF8E0}"
                                bstrReturn = "Grid"
                            Case "{93073C40-0BA5-11d2-A484-00C04F8EFB69}"
                                bstrReturn = "Inset"
                            Case "{3F69F351-0379-11D2-A484-00C04F8EFB69}"
                                bstrReturn = "Iris"
                            Case "{2A54C904-07AA-11D2-8D6D-00C04F8EF8E0}"
                                bstrReturn = "Jaws"
                            Case "{107045CA-06E0-11D2-8D6D-00C04F8EF8E0}"
                                bstrReturn = "Lens"
                            Case "{107045C8-06E0-11D2-8D6D-00C04F8EF8E0}"
                                bstrReturn = "LightWipe"
                            Case "{AA0D4D0A-06A3-11D2-8F98-00C04FB92EB7}"
                                bstrReturn = "Liquid"
                            Case "{AA0D4D08-06A3-11D2-8F98-00C04FB92EB7}"
                                bstrReturn = "PageCurl"
                            Case "{AA0D4D10-06A3-11D2-8F98-00C04FB92EB7}"
                                bstrReturn = "PeelABCD"
                            Case "{4CCEA634-FBE0-11d1-906A-00C04FD9189D}"
                                bstrReturn = "Pixelate"
                            Case "{424B71AF-0695-11D2-A484-00C04F8EFB69}"
                                bstrReturn = "RadialWipe"
                            Case "{AA0D4D03-06A3-11D2-8F98-00C04FB92EB7}"
                                bstrReturn = "Ripple"
                            Case "{9C61F46E-0530-11D2-8F98-00C04FB92EB7}"
                                bstrReturn = "RollDown"
                            Case "{810E402F-056B-11D2-A484-00C04F8EFB69}"
                                bstrReturn = "Slide"
                            Case "{dE75D012-7A65-11D2-8CEA-00A0C9441E20}"
                                bstrReturn = "SMPTE Wipe"
                            Case "{ACA97E00-0C7D-11d2-A484-00C04F8EFB69}"
                                bstrReturn = "Spiral"
                            Case "{7658F2A2-0A83-11d2-A484-00C04F8EFB69}"
                                bstrReturn = "Stretch"
                            Case "{2A54C915-07AA-11D2-8D6D-00C04F8EF8E0}"
                                bstrReturn = "Threshold"
                            Case "{107045CF-06E0-11D2-8D6D-00C04F8EF8E0}"
                                bstrReturn = "Twister"
                            Case "{2A54C90D-07AA-11D2-8D6D-00C04F8EF8E0}"
                                bstrReturn = "Vacuum"
                            Case "{107045C5-06E0-11D2-8D6D-00C04F8EF8E0}"
                                bstrReturn = "Water"
                            Case "{5AE1DAE0-1461-11d2-A484-00C04F8EFB69}"
                                bstrReturn = "Wheel"
                            Case "{AF279B30-86EB-11D1-81BF-0000F87557DB}"
                                bstrReturn = "Wipe"
                            Case "{0E6AE022-0C83-11D2-8CD4-00104BC75D9A}"
                                bstrReturn = "WormHole"
                            Case "{E6E73D20-0C8A-11d2-A484-00C04F8EFB69}"
                                bstrReturn = "Zigzag"
                            Case Else: bstrReturn = vbNullString
                        End Select
            End If
            'return friendly name to the client
            TransitionCLSIDToFriendlyName = bstrReturn
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
             
             
             
            ' ******************************************************************************************************************************
            ' * procedure name: TransitionFriendlyNameToCLSID
            ' * procedure description: returns the CLSID of a transition given it's localized friendly name
            ' *
            ' ******************************************************************************************************************************
             Public Function TransitionFriendlyNameToCLSID(bstrFriendlyName As String, Optional bstrLanguage As String = "EN-US") As String
             Dim bstrReturn As String
             On Local Error GoTo ErrLine
             
             If UCase(bstrLanguage) = "EN-US" Then
                        Select Case bstrFriendlyName
                            Case "Barn"
                                      bstrReturn = "{C3BDF740-0B58-11d2-A484-00C04F8EFB69}"
                            Case "Blinds"
                                      bstrReturn = "{00C429C0-0BA9-11d2-A484-00C04F8EFB69}"
                            Case "BurnFilm"
                                      bstrReturn = "{107045D1-06E0-11D2-8D6D-00C04F8EF8E0}"
                            Case "CenterCurls"
                                      bstrReturn = "{AA0D4D0C-06A3-11D2-8F98-00C04FB92EB7}"
                            Case "ColorFade"
                                      bstrReturn = "{2A54C908-07AA-11D2-8D6D-00C04F8EF8E0}"
                            Case "Compositor"
                                      bstrReturn = "{9A43A844-0831-11D1-817F-0000F87557DB}"
                            Case "Curls"
                                      bstrReturn = "{AA0D4D0E-06A3-11D2-8F98-00C04FB92EB7}"
                            Case "Curtains"
                                      bstrReturn = "{AA0D4D12-06A3-11D2-8F98-00C04FB92EB7}"
                            Case "Fade"
                                      bstrReturn = "{16B280C5-EE70-11D1-9066-00C04FD9189D}"
                            Case "FadeWhite"
                                      bstrReturn = "{107045CC-06E0-11D2-8D6D-00C04F8EF8E0}"
                            Case "FlowMotion"
                                      bstrReturn = "{2A54C90B-07AA-11D2-8D6D-00C04F8EF8E0}"
                            Case "GlassBlock"
                                      bstrReturn = "{2A54C913-07AA-11D2-8D6D-00C04F8EF8E0}"
                            Case "Grid"
                                      bstrReturn = "{2A54C911-07AA-11D2-8D6D-00C04F8EF8E0}"
                            Case "Inset"
                                      bstrReturn = "{93073C40-0BA5-11d2-A484-00C04F8EFB69}"
                            Case "Iris"
                                      bstrReturn = "{3F69F351-0379-11D2-A484-00C04F8EFB69}"
                            Case "Jaws"
                                      bstrReturn = "{2A54C904-07AA-11D2-8D6D-00C04F8EF8E0}"
                            Case "Lens"
                                      bstrReturn = "{107045CA-06E0-11D2-8D6D-00C04F8EF8E0}"
                            Case "LightWipe"
                                      bstrReturn = "{107045C8-06E0-11D2-8D6D-00C04F8EF8E0}"
                            Case "Liquid"
                                      bstrReturn = "{AA0D4D0A-06A3-11D2-8F98-00C04FB92EB7}"
                            Case "PageCurl"
                                      bstrReturn = "{AA0D4D08-06A3-11D2-8F98-00C04FB92EB7}"
                            Case "PeelABCD"
                                      bstrReturn = "{AA0D4D10-06A3-11D2-8F98-00C04FB92EB7}"
                            Case "Pixelate"
                                      bstrReturn = "{4CCEA634-FBE0-11d1-906A-00C04FD9189D}"
                            Case "RadialWipe"
                                      bstrReturn = "{424B71AF-0695-11D2-A484-00C04F8EFB69}"
                            Case "Ripple"
                                      bstrReturn = "{AA0D4D03-06A3-11D2-8F98-00C04FB92EB7}"
                            Case "RollDown"
                                      bstrReturn = "{9C61F46E-0530-11D2-8F98-00C04FB92EB7}"
                            Case "Slide"
                                      bstrReturn = "{810E402F-056B-11D2-A484-00C04F8EFB69}"
                            Case "SMPTE Wipe"
                                      bstrReturn = "{dE75D012-7A65-11D2-8CEA-00A0C9441E20}"
                            Case "Spiral"
                                      bstrReturn = "{ACA97E00-0C7D-11d2-A484-00C04F8EFB69}"
                            Case "Stretch"
                                      bstrReturn = "{7658F2A2-0A83-11d2-A484-00C04F8EFB69}"
                            Case "Threshold"
                                      bstrReturn = "{2A54C915-07AA-11D2-8D6D-00C04F8EF8E0}"
                            Case "Twister"
                                      bstrReturn = "{107045CF-06E0-11D2-8D6D-00C04F8EF8E0}"
                            Case "Vacuum"
                                      bstrReturn = "{2A54C90D-07AA-11D2-8D6D-00C04F8EF8E0}"
                            Case "Water"
                                      bstrReturn = "{107045C5-06E0-11D2-8D6D-00C04F8EF8E0}"
                            Case "Wheel"
                                      bstrReturn = "{5AE1DAE0-1461-11d2-A484-00C04F8EFB69}"
                            Case "Wipe"
                                      bstrReturn = "{AF279B30-86EB-11D1-81BF-0000F87557DB}"
                            Case "WormHole"
                                      bstrReturn = "{0E6AE022-0C83-11D2-8CD4-00104BC75D9A}"
                            Case "Zigzag"
                                      bstrReturn = "{E6E73D20-0C8A-11d2-A484-00C04F8EFB69}"
                            Case Else: bstrReturn = vbNullString
                        End Select
            End If
            'return friendly name to the client
            TransitionFriendlyNameToCLSID = bstrReturn
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: EffectCLSIDToFriendlyName
            ' * procedure description: returns the localized friendly name of an effect given it's CLSID
            ' *
            ' ******************************************************************************************************************************
             Public Function EffectCLSIDToFriendlyName(bstrTransitionCLSID As String, Optional bstrLanguage As String = "EN-US") As String
             Dim bstrReturn As String
             On Local Error GoTo ErrLine
             
             If UCase(bstrLanguage) = "EN-US" Then
                         Select Case bstrTransitionCLSID
                            Case "{16B280C8-EE70-11D1-9066-00C04FD9189D}"
                                     bstrReturn = "BasicImage"
                            Case "{7312498D-E87A-11d1-81E0-0000F87557DB}"
                                     bstrReturn = "Blur"
                            Case "{421516C1-3CF8-11D2-952A-00C04FA34F05}"
                                     bstrReturn = "Chroma"
                            Case "{ADC6CB86-424C-11D2-952A-00C04FA34F05}"
                                     bstrReturn = "DropShadow"
                            Case "{F515306D-0156-11d2-81EA-0000F87557DB}"
                                     bstrReturn = "Emboss"
                            Case "{F515306E-0156-11d2-81EA-0000F87557DB}"
                                     bstrReturn = "Engrave"
                            Case "{16B280C5-EE70-11D1-9066-00C04FD9189D}"
                                     bstrReturn = "Fade"
                            Case "{4CCEA634-FBE0-11d1-906A-00C04FD9189D}"
                                     bstrReturn = "Pixelate"
                            Case Else: bstrReturn = vbNullString
                        End Select
            End If
            'return friendly name to the client
            EffectCLSIDToFriendlyName = bstrReturn
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
             
              
              
            ' ******************************************************************************************************************************
            ' * procedure name: EffectFriendlyNameToCLSID
            ' * procedure description: returns the CLSID of an effect given it's localized friendly name
            ' *
            ' ******************************************************************************************************************************
             Public Function EffectFriendlyNameToCLSID(bstrFriendlyName As String, Optional bstrLanguage As String = "EN-US") As String
             Dim bstrReturn As String
             On Local Error GoTo ErrLine
             
             If UCase(bstrLanguage) = "EN-US" Then
                        Select Case bstrFriendlyName
                            Case "BasicImage"
                                     bstrReturn = "{16B280C8-EE70-11D1-9066-00C04FD9189D}"
                            Case "Blur"
                                     bstrReturn = "{7312498D-E87A-11d1-81E0-0000F87557DB}"
                            Case "Chroma"
                                     bstrReturn = "{421516C1-3CF8-11D2-952A-00C04FA34F05}"
                            Case "DropShadow"
                                     bstrReturn = "{ADC6CB86-424C-11D2-952A-00C04FA34F05}"
                            Case "Emboss"
                                     bstrReturn = "{F515306D-0156-11d2-81EA-0000F87557DB}"
                            Case "Engrave"
                                     bstrReturn = "{F515306E-0156-11d2-81EA-0000F87557DB}"
                            Case "Fade"
                                     bstrReturn = "{16B280C5-EE70-11D1-9066-00C04FD9189D}"
                            Case "Pixelate"
                                     bstrReturn = "{4CCEA634-FBE0-11d1-906A-00C04FD9189D}"
                            Case Else: bstrReturn = vbNullString
                        End Select
            End If
            'return friendly name to the client
            EffectFriendlyNameToCLSID = bstrReturn
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: GetGroupCount
            ' * procedure description: returns the number of groups encapsulated within the given timeline
            ' *
            ' ******************************************************************************************************************************
            Public Function GetGroupCount(objTimeline As AMTimeline) As Long
            Dim nCount As Long
            On Local Error GoTo ErrLine
             
            'obtain the number of groups
            Call objTimeline.GetGroupCount(nCount)
            'return the group count
            GetGroupCount = nCount
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: HasGroups
            ' * procedure description: returns a boolean indicating wether or not the specified timeline has any any groups inserted
            ' *
            ' ******************************************************************************************************************************
            Public Function HasGroups(objTimeline As AMTimeline) As Boolean
            Dim nCount As Long
            On Local Error GoTo ErrLine
             
            'obtain the number of groups
            Call objTimeline.GetGroupCount(nCount)
            'return the group count
            If nCount > 0 Then HasGroups = True
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: HasStreams
            ' * procedure description:  Returns True if the given media file contains any valid streams at all
            ' *
            ' ******************************************************************************************************************************
            Public Function HasStreams(bstrFileName As String) As Boolean
            Dim nStreams As Long
            Dim objMediaDet As MediaDet
            On Local Error GoTo ErrLine
            
            'verify argument(s)
            If bstrFileName <> vbNullString Then
               'instantiate mediadet
               Set objMediaDet = New MediaDet
               'assign the filename to the MediaDet
               objMediaDet.FileName = bstrFileName
               'obtain the number of streams
               nStreams = objMediaDet.OutputStreams
               'verify there is at least one valid media stream in the assigned file
               If nStreams > 0 Then HasStreams = True
            End If
            
            'clean-up & dereference
            If Not objMediaDet Is Nothing Then Set objMediaDet = Nothing
            Exit Function
            
ErrLine:

            Err.Clear
            Exit Function
            End Function
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: HasVideoStream
            ' * procedure description:  Returns True if the given media file contains a valid video stream
            ' *
            ' ******************************************************************************************************************************
            Public Function HasVideoStream(bstrFileName As String) As Boolean
            Dim nCount As Long
            Dim nStreams As Long
            Dim objMediaDet As MediaDet
            Dim bstrMediaCLSID As String
            On Local Error GoTo ErrLine
            
            'verify argument(s)
            If bstrFileName <> vbNullString Then
               'instantiate mediadet
               Set objMediaDet = New MediaDet
               'assign the filename to the MediaDet
               objMediaDet.FileName = bstrFileName
               'obtain the number of streams
               nStreams = objMediaDet.OutputStreams
               'verify there is at least one valid media stream in the assigned file
               If nStreams > 0 Then
                  For nCount = 0 To objMediaDet.OutputStreams - 1
                       objMediaDet.CurrentStream = nCount
                       bstrMediaCLSID = objMediaDet.StreamTypeB
                       If bstrMediaCLSID = SLIDESHOWVB_VIDEOTYPE Then
                          HasVideoStream = True: Exit For
                       End If
                  Next
               End If
            End If
            
            'clean-up & dereference
            If Not objMediaDet Is Nothing Then Set objMediaDet = Nothing
            Exit Function
            
ErrLine:

            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: HasAudioStream
            ' * procedure description:  Returns True if the given media file contains a valid audio stream
            ' *
            ' ******************************************************************************************************************************
            Public Function HasAudioStream(bstrFileName As String) As Boolean
            Dim nCount As Long
            Dim nStreams As Long
            Dim objMediaDet As MediaDet
            Dim bstrMediaCLSID As String
            On Local Error GoTo ErrLine
            
            'verify argument(s)
            If bstrFileName <> vbNullString Then
               'instantiate mediadet
               Set objMediaDet = New MediaDet
               'assign the filename to the MediaDet
               objMediaDet.FileName = bstrFileName
               'obtain the number of streams
               nStreams = objMediaDet.OutputStreams
               'verify there is at least one valid media stream in the assigned file
               If nStreams > 0 Then
                  For nCount = 0 To objMediaDet.OutputStreams - 1
                       objMediaDet.CurrentStream = nCount
                       bstrMediaCLSID = objMediaDet.StreamTypeB
                       If bstrMediaCLSID = SLIDESHOWVB_AUDIOTYPE Then
                          HasAudioStream = True: Exit For
                       End If
                  Next
               End If
            End If
            
            'clean-up & dereference
            If Not objMediaDet Is Nothing Then Set objMediaDet = Nothing
            Exit Function
            
ErrLine:

            Err.Clear
            Exit Function
            End Function
            
            
            
            
' **************************************************************************************************************************************
' * PUBLIC INTERFACE- QUARTZ PROCEDURES
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: RenderTimeline
            ' * procedure description: renders the timeline for the client and returns an instance of the filter graph manager
            ' *                                      NOTE:  THIS PROCEDURE USES MODULE-LEVEL VARIABLES BECAUSE
            ' *                                                   IT WORKS ASYNCRONOUSLY.  IF YOU MOVE THEM OVER LOCALLY
            ' *                                                   YOUR APPLICATION WILL TAKE A READ FAULT BECAUSE QEDIT WILL
            ' *                                                   NOT BE ABLE TO READ YOUR FILTERGRAPH WHEN THE PROCEDURE EXITS.
            ' ******************************************************************************************************************************
            Public Function RenderTimeline(objTimeline As AMTimeline, Optional UseDynamicConnections As Boolean, Optional UseSmartRecompression As Boolean) As FilgraphManager
            On Local Error GoTo ErrLine
            
            'instantiate new render engine
            Set m_objRenderEngine = New RenderEngine
            
            'setup dynamic connections
            If UseDynamicConnections = True Then
               Call m_objRenderEngine.SetDynamicReconnectLevel(1)
            Else: Call m_objRenderEngine.SetDynamicReconnectLevel(0)
            End If
            
            'setup smart recompression
            If UseSmartRecompression = True Then
               'smart recompression is not currently supported in vb
            End If
            
            'connect everything up..
            Call m_objRenderEngine.SetTimelineObject(objTimeline)
            m_objRenderEngine.ConnectFrontEnd
            m_objRenderEngine.RenderOutputPins
            
            'render the audio/video
            Call m_objRenderEngine.GetFilterGraph(m_objFilterGraph)
            Set m_objFilterGraphManager = New FilgraphManager
            Set m_objFilterGraphManager = m_objFilterGraph
            
            'return an instance of the filgraph manager to the client
            Set RenderTimeline = m_objFilterGraphManager
            Exit Function
            
ErrLine:

            Err.Clear
            Exit Function
            End Function
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: RenderTimelineSync
            ' * procedure description: Renders the timeline for the client, and waits for completion
            ' *                                      until the media completes or until the specified timeout is reached..
            ' *
            ' ******************************************************************************************************************************
            Public Sub RenderTimelineSync(objTimeline As AMTimeline, Optional Timeout As Long = -1)
            Dim nExitCode As Long
            Dim objMediaEvent As IMediaEvent
            Dim objMediaControl As IMediaControl
            Dim objFilterGraph As IGraphBuilder
            Dim objRenderEngine As RenderEngine
            On Local Error GoTo ErrLine
            
            'instantiate new render engine
            Set objRenderEngine = New RenderEngine
            
            'connect everything up..
            Call objRenderEngine.SetTimelineObject(objTimeline)
            objRenderEngine.ConnectFrontEnd
            objRenderEngine.RenderOutputPins
            
            'obtain the filtergraph
            Call objRenderEngine.GetFilterGraph(objFilterGraph)
            Set objMediaEvent = objFilterGraph
            Set objMediaControl = objFilterGraph
            
            'render the graph
            objMediaControl.Run
            'wait for the graph to complete..
            objMediaEvent.WaitForCompletion Timeout, nExitCode
            
            'clean-up & dereference
            If Not objFilterGraph Is Nothing Then Set objFilterGraph = Nothing
            If Not objMediaEvent Is Nothing Then Set objMediaEvent = Nothing
            If Not objMediaControl Is Nothing Then Set objMediaControl = Nothing
            If Not objRenderEngine Is Nothing Then Set objRenderEngine = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: RenderTimelineAsync
            ' * procedure description: renders the timeline for the client in true async fashion
            ' *                                      NOTE:  THIS PROCEDURE USES MODULE-LEVEL VARIABLES BECAUSE
            ' *                                                   IT WORKS ASYNCRONOUSLY.  IF YOU MOVE THEM OVER LOCALLY
            ' *                                                   YOUR APPLICATION WILL TAKE A READ FAULT BECAUSE QEDIT WILL
            ' *                                                   NOT BE ABLE TO READ YOUR FILTERGRAPH WHEN THE PROCEDURE EXITS.
            ' ******************************************************************************************************************************
            Public Sub RenderTimelineAsync(objTimeline As AMTimeline)
            On Local Error GoTo ErrLine
            
            'instantiate new render engine
            Set gbl_objRenderEngine = New RenderEngine
            
            'connect everything up..
            Call gbl_objRenderEngine.SetTimelineObject(objTimeline)
            gbl_objRenderEngine.ConnectFrontEnd
            gbl_objRenderEngine.RenderOutputPins
            
            'render the audio/video
            Call gbl_objRenderEngine.GetFilterGraph(m_objFilterGraph)
            Set m_objFilterGraphManager = New FilgraphManager
            Set m_objFilterGraphManager = m_objFilterGraph
            m_objFilterGraphManager.Run
            Exit Sub
            
ErrLine:

            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: RenderTimelineQuasiAsync
            ' * procedure description: Renders the timeline for the client, and waits for completion by using
            ' *                                      a structured loop which constantly checks for EC_COMPLETE to occur.
            ' *                                      By using the VB 'WithEvents' Keyword normal form events are uninhibited.
            ' *                                      as VB will basically handle the multi-threading for you.
            ' *                                      To Stop playback use recursion with syntax: Call RenderTimelineQuasiAsync(Nothing)
            ' ******************************************************************************************************************************
            Public Sub RenderTimelineQuasiAsync(objTimeline As AMTimeline)
            Static nResultant As Long
            Static objPosition As IMediaPosition
            Static objMediaEvent As IMediaEvent
            Static objFilterGraph As IGraphBuilder
            Static objVideoWindow As IVideoWindow
            Static objRenderEngine As RenderEngine
            Static objFilterGraphManager As New FilgraphManager
            On Local Error GoTo ErrLine
            
            If Not objTimeline Is Nothing Then
               'instantiate new render engine
               Set objRenderEngine = New RenderEngine
               
               'connect everything up..
               Call objRenderEngine.SetTimelineObject(objTimeline)
               objRenderEngine.ConnectFrontEnd
               objRenderEngine.RenderOutputPins
               
               'render the audio/video
               Call objRenderEngine.GetFilterGraph(objFilterGraph)
               Set objFilterGraphManager = New FilgraphManager
               Set objFilterGraphManager = objFilterGraph
               objFilterGraphManager.Run
               'obtain the position of audio/video
               Set objPosition = objFilterGraphManager
               'obtain the video window
               Set objMediaEvent = objFilterGraphManager
               Set objVideoWindow = objMediaEvent
               
               'loop with events until the media has finished or the video
               'window has been manually closed (if the timeline has video)
               Do: DoEvents
                     'check state
                     If Not objMediaEvent Is Nothing Then _
                        Call objMediaEvent.WaitForCompletion(10, nResultant)
                     'evaluate resultant
                     If nResultant = 1 Then  'EC_COMPLETE
                        If Not objVideoWindow Is Nothing Then
                           objVideoWindow.Left = Screen.Width * 8
                           objVideoWindow.Top = Screen.Height * 8
                           objVideoWindow.Visible = False
                        End If
                        If Not objFilterGraphManager Is Nothing Then _
                           Call objFilterGraphManager.Stop
                        Exit Do
                     ElseIf objVideoWindow.Visible = False Then
                        If Not objFilterGraphManager Is Nothing Then _
                           Call objFilterGraphManager.Stop
                        Exit Do
                     ElseIf objTimeline Is Nothing Then
                        Exit Do
                     ElseIf objFilterGraphManager Is Nothing Then
                        Exit Do
                     End If
               Loop
            Else: nResultant = 1
            End If
            
            'clean-up & dereference
            If Not objPosition Is Nothing Then Set objPosition = Nothing
            If Not objFilterGraph Is Nothing Then Set objFilterGraph = Nothing
            If Not objMediaEvent Is Nothing Then Set objMediaEvent = Nothing
            If Not objVideoWindow Is Nothing Then Set objVideoWindow = Nothing
            If Not objRenderEngine Is Nothing Then Set objRenderEngine = Nothing
            If Not objFilterGraphManager Is Nothing Then Set objFilterGraphManager = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Resume Next
            Exit Sub
            End Sub
