Attribute VB_Name = "modDisposable"
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
' * PUBLIC INTERFACE- GENERAL PROCEDURES
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: GetTimelineDirect
            ' * procedure description:   Populate the treeview control by walking the Timeline via Dexter API's
            ' *
            ' ******************************************************************************************************************************
            Public Sub GetTimelineDirect(ctrlTreeView As TreeView, objTimeline As AMTimeline, Optional colDataStore As Collection)
            Dim nCount As Long
            Dim objNode As node
            Dim nGroupCount As Long
            Dim bstrRootGUID As String
            Dim bstrGroupGUID As String
            Dim objGroup As AMTimelineGroup
            Dim objTimelineObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
           'Ensure the treeview control's nodes are cleared
            If Not ctrlTreeView Is Nothing Then
               If ctrlTreeView.Nodes.Count <> 0 Then
                  ctrlTreeView.Nodes.Clear
               End If
            End If
            
            'clear the collection
            If Not colDataStore Is Nothing Then
               Do Until colDataStore.Count = 0
               colDataStore.Remove 1
               Loop
            End If
            
           'get a guid for a key
           bstrRootGUID = GetGUID
           'Insert the root timeline node
           Set objNode = ctrlTreeView.Nodes.Add(, , bstrRootGUID, "TimeLine 1", 1)
           objNode.Tag = "AMTimeline"
           'append to datastore (optional)
           If Not colDataStore Is Nothing Then colDataStore.Add objTimeline, bstrRootGUID
           
           
           'Obtain the number of groups to populate with
           nGroupCount = GetGroupCount(objTimeline)
            While nCount < nGroupCount: DoEvents
                'get the group
                objTimeline.GetGroup objTimelineObject, nCount
                'instantiate the local copy
                Set objGroup = objTimelineObject
                'get a guid for a key
                bstrGroupGUID = GetGUID
                'Insert the group timeline node
                Set objNode = ctrlTreeView.Nodes.Add(1, 4, bstrGroupGUID, "Group" + CStr(nCount) & Chr(32), 2)
                objNode.Tag = "AMTimelineGroup"
                'append to datastore (optional)
                If Not colDataStore Is Nothing Then colDataStore.Add objGroup, bstrGroupGUID
                'append all tracks
                AddTracks ctrlTreeView, bstrGroupGUID, objTimelineObject, colDataStore
                'append all effects
                AddEffects ctrlTreeView, bstrGroupGUID, objTimelineObject, colDataStore
                'append all transitions
                AddTransitions ctrlTreeView, bstrGroupGUID, objTimelineObject, colDataStore
                'append compositions
                AddComposites ctrlTreeView, bstrGroupGUID, objTimelineObject, colDataStore
                'dereference & clean-up
                If Not objGroup Is Nothing Then Set objGroup = Nothing
                If Not objTimelineObject Is Nothing Then Set objTimelineObject = Nothing
                'increment the group count by a factor of one
                nCount = nCount + 1
            Wend
            
            'expand all items in the treeview control
            For nCount = 1 To ctrlTreeView.Nodes.Count
                    ctrlTreeView.Nodes(nCount).Expanded = True
            Next
            
            'clean-up & dereference
            If Not objGroup Is Nothing Then Set objGroup = Nothing
            If Not objTimelineObject Is Nothing Then Set objTimelineObject = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: RefreshListView
            ' * procedure description:  Updates the listview
            ' *
            ' ******************************************************************************************************************************
            Public Sub RefreshListView(ctrlListView As ListView, bstrType As String, bstrKey As String)
            Dim objListItem As ListItem
            On Local Error GoTo ErrLine
            
            'setup listview
            If ctrlListView.View <> lvwReport Then ctrlListView.View = lvwReport
            If ctrlListView.ListItems.Count <> 0 Then Call ctrlListView.ListItems.Clear
            If ctrlListView.ColumnHeaders.Count = 0 Then
                ctrlListView.ColumnHeaders.Add , "Parameter", "Parameter", (ctrlListView.Width / 2) - 50
                ctrlListView.ColumnHeaders.Add , "Value", "Value", (ctrlListView.Width / 2) - 50
            End If
            'append items to the listview
            Set objListItem = ctrlListView.ListItems.Add(1, "Type", "Type")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = bstrType
            Set objListItem = ctrlListView.ListItems.Add(2, "Key Name", "Key Name")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = bstrKey
            Set objListItem = ctrlListView.ListItems.Add(3, Space(1), Space(1))
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = Space(1)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
' **************************************************************************************************************************************
' * PUBLIC INTERFACE- ADD PROCEDURES  (PROCEDURES ADD GIVEN ITEMS TO THE TREEVIEW)
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: AddComposites
            ' * procedure description: appends all composites on to the treeview
            ' *
            ' ******************************************************************************************************************************
            Public Sub AddComposites(ctrlTreeView As TreeView, bstrParent As String, objTimelineObject As AMTimelineObj, Optional colDataStore As Collection)
            Dim nType As Long
            Dim objNode As node
            Dim nPriority As Long
            Dim bstrGUID As String
            Dim nTrackCount As Long
            Dim nTotalVirtualTracks As Long
            Dim objComp As AMTimelineComp
            Dim objLocalTimelineObject As AMTimelineObj
            Dim objCompVirtualTrack As IAMTimelineVirtualTrack
            On Local Error GoTo ErrLine
            
            'derive composition
            Set objComp = objTimelineObject
            'obtain track count
            objComp.VTrackGetCount nTotalVirtualTracks
          
          
            'loop through the tracks
            While nTrackCount < nTotalVirtualTracks: DoEvents
                'get the object
                objComp.GetVTrack objLocalTimelineObject, nTrackCount
                'obtain the timeline type
                objLocalTimelineObject.GetTimelineType nType
                
                If nType = TIMELINE_MAJOR_TYPE_COMPOSITE Then
                    'obtain the comp
                    Set objCompVirtualTrack = objLocalTimelineObject
                    'obtain the Priority
                    objCompVirtualTrack.TrackGetPriority nPriority
                    'get a guid for a key
                    bstrGUID = GetGUID
                    'Insert the root timeline node
                    Set objNode = ctrlTreeView.Nodes.Add(bstrParent, 4, bstrGUID, "Comp" & CStr(nPriority) & Chr(32), 2)
                    objNode.Tag = "AMTimelineComp"
                    'append to datastore (optional)
                    If Not colDataStore Is Nothing Then colDataStore.Add objCompVirtualTrack, bstrGUID
                    'recursive call of dependent children
                    Call AddEffects(ctrlTreeView, bstrGUID, objLocalTimelineObject, colDataStore)
                    Call AddTransitions(ctrlTreeView, bstrGUID, objLocalTimelineObject, colDataStore)
                    Call AddComposites(ctrlTreeView, bstrGUID, objLocalTimelineObject, colDataStore)
                    
                ElseIf nType = TIMELINE_MAJOR_TYPE_TRACK Then
                    'recursive call of dependent children
                    AddTracks ctrlTreeView, bstrParent, objLocalTimelineObject, colDataStore
                End If
                
                'clean-up & dereference
                If Not objCompVirtualTrack Is Nothing Then Set objCompVirtualTrack = Nothing
                If Not objLocalTimelineObject Is Nothing Then Set objLocalTimelineObject = Nothing
                'increment track counter
                nTrackCount = nTrackCount + 1
            Wend
            
            'clean-up & dereference
            If Not objNode Is Nothing Then Set objNode = Nothing
            If Not objComp Is Nothing Then Set objComp = Nothing
            If Not objCompVirtualTrack Is Nothing Then Set objCompVirtualTrack = Nothing
            If Not objLocalTimelineObject Is Nothing Then Set objLocalTimelineObject = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
     
            
            ' ******************************************************************************************************************************
            ' * procedure name: AddEffects
            ' * procedure description: appends all effects on to the treeview
            ' *
            ' ******************************************************************************************************************************
            Public Sub AddEffects(ctrlTreeView As TreeView, bstrParent As String, objTimelineObject As AMTimelineObj, Optional colDataStore As Collection)
            Dim objNode As node
            Dim bstrGUID As String
            Dim nEffectCount As Long
            Dim nTotalEffects As Long
            Dim objEffect As AMTimelineEffect
            Dim objLocalTimelineObject As AMTimelineObj
            Dim objEffectable As IAMTimelineEffectable
            On Local Error GoTo ErrLine
            
            Set objEffectable = objTimelineObject
            objEffectable.EffectGetCount nTotalEffects
            
            While nEffectCount < nTotalEffects: DoEvents
                'get the timeline object
                objEffectable.GetEffect objLocalTimelineObject, nEffectCount
                'get the effect from the timeline object
                Set objEffect = objLocalTimelineObject
                'get a guid for a key
                bstrGUID = GetGUID
                'Insert the effect timeline node
                Set objNode = ctrlTreeView.Nodes.Add(bstrParent, 4, bstrGUID, "Effect" & CStr(nEffectCount) & Chr(32), 2)
                objNode.Tag = "AMTimelineEffect"
                'append to datastore (optional)
                If Not colDataStore Is Nothing Then colDataStore.Add objEffect, bstrGUID
                'clean-up & dereference
                If Not objEffect Is Nothing Then Set objEffect = Nothing
                If Not objLocalTimelineObject Is Nothing Then Set objLocalTimelineObject = Nothing
                'increment the count
                nEffectCount = nEffectCount + 1
            Wend
            
            'clean-up & dereference
            If Not objNode Is Nothing Then Set objNode = Nothing
            If Not objEffect Is Nothing Then Set objEffect = Nothing
            If Not objEffectable Is Nothing Then Set objEffectable = Nothing
            If Not objLocalTimelineObject Is Nothing Then Set objLocalTimelineObject = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: AddTracks
            ' * procedure description: appends all tracks on to the treeview; objTimelineObject evaluates to a virtual track
            ' *
            ' ******************************************************************************************************************************
            Public Sub AddTracks(ctrlTreeView As TreeView, bstrParent As String, objTimelineObject As AMTimelineObj, Optional colDataStore As Collection)
            Dim objNode As node
            Dim nPriority As Long
            Dim bstrGUID As String
            Dim objTrack As AMTimelineTrack
            Dim objVirtualTrack As IAMTimelineVirtualTrack
            On Local Error GoTo ErrLine
            
            'get the track object
            Set objTrack = objTimelineObject
            'get the virtual track object
            Set objVirtualTrack = objTrack
            'get the virtual track priority
            objVirtualTrack.TrackGetPriority nPriority
            
            'get a guid for a key
            bstrGUID = GetGUID
            'Insert the root timeline node
            Set objNode = ctrlTreeView.Nodes.Add(bstrParent, 4, bstrGUID, "Track" & CStr(nPriority) & Chr(32), 2)
            objNode.Tag = "AMTimelineTrack"
            'append to datastore (optional)
            If Not colDataStore Is Nothing Then colDataStore.Add objTrack, bstrGUID
            'recursive call of dependent children
            Call AddSources(ctrlTreeView, bstrGUID, objTimelineObject, colDataStore)
            Call AddEffects(ctrlTreeView, bstrGUID, objTimelineObject, colDataStore)
            Call AddTransitions(ctrlTreeView, bstrGUID, objTimelineObject, colDataStore)
            'clean-up & dereference
            If Not objNode Is Nothing Then Set objNode = Nothing
            If Not objTrack Is Nothing Then Set objTrack = Nothing
            If Not objVirtualTrack Is Nothing Then Set objVirtualTrack = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
           
            ' ******************************************************************************************************************************
            ' * procedure name: AddSources
            ' * procedure description: appends all sources on to the treeview
            ' *
            ' ******************************************************************************************************************************
            Public Sub AddSources(ctrlTreeView As TreeView, bstrParent As String, objTimelineObject As AMTimelineObj, Optional colDataStore As Collection)
            Dim objNode As node
            Dim dblStart As Double
            Dim bstrGUID As String
            Dim nSourceCount As Long
            Dim nTotalSources As Long
            Dim objSource As AMTimelineSrc
            Dim objTrack As AMTimelineTrack
            Dim objLocalTimelineObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
            'get a track
            Set objTrack = objTimelineObject
            'get the number of sources
            objTrack.GetSourcesCount nTotalSources
            
            
            While nSourceCount < nTotalSources: DoEvents
                'get the timeline object
                objTrack.GetNextSrc2 objLocalTimelineObject, dblStart
                'derive the source object from the timeline object
                Set objSource = objLocalTimelineObject
                'get a guid for a key
                bstrGUID = GetGUID
                'Insert the root timeline node
                Set objNode = ctrlTreeView.Nodes.Add(bstrParent, 4, bstrGUID, "Clip" & CStr(nSourceCount) & Chr(32), 2)
                objNode.Tag = "AMTimelineSrc"
                'append to datastore (optional)
                If Not colDataStore Is Nothing Then colDataStore.Add objSource, bstrGUID
                'dereference & clean-up
                If Not objSource Is Nothing Then Set objSource = Nothing
                If Not objLocalTimelineObject Is Nothing Then Set objLocalTimelineObject = Nothing
                'increment the source counter
                nSourceCount = nSourceCount + 1
            Wend
            'dereference & clean-up
            If Not objNode Is Nothing Then Set objNode = Nothing
            If Not objTrack Is Nothing Then Set objTrack = Nothing
            If Not objSource Is Nothing Then Set objSource = Nothing
            If Not objLocalTimelineObject Is Nothing Then Set objLocalTimelineObject = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: AddTransitions
            ' * procedure description: appends an all the transitions on to the treeview
            ' *
            ' ******************************************************************************************************************************
            Public Sub AddTransitions(ctrlTreeView As TreeView, bstrParent As String, objTimelineObject As AMTimelineObj, Optional colDataStore As Collection)
            Dim objNode As node
            Dim bstrGUID As String
            Dim nTransitionCount As Long
            Dim nTotalTransitions As Long
            Dim dblReferenceTime As Double
            Dim objTransition As AMTimelineTrans
            Dim objTransable As IAMTimelineTransable
            Dim objLocalTimelineObject As AMTimelineObj
            On Local Error GoTo ErrLine
            
            'get the transable
            Set objTransable = objTimelineObject
            'get the transition count
            objTransable.TransGetCount nTotalTransitions
            
            While nTransitionCount < nTotalTransitions: DoEvents
                'get the next transition into a timeline object
                objTransable.GetNextTrans2 objLocalTimelineObject, dblReferenceTime
                'get the transition object from the timeline object
                Set objTransition = objLocalTimelineObject
                'get a guid for a key
                bstrGUID = GetGUID
                'Insert the root timeline node
                Set objNode = ctrlTreeView.Nodes.Add(bstrParent, 4, bstrGUID, "Transition" & CStr(nTransitionCount) & Chr(32), 2)
                objNode.Tag = "AMTimelineTrans"
                'append to datastore (optional)
                If Not colDataStore Is Nothing Then colDataStore.Add objTransition, bstrGUID
                'clean-up & dereference
                If Not objTransition Is Nothing Then Set objTransition = Nothing
                If Not objLocalTimelineObject Is Nothing Then Set objLocalTimelineObject = Nothing
                'increment the transition count
                nTransitionCount = nTransitionCount + 1
            Wend
            
            'clean-up & dereference
            If Not objNode Is Nothing Then Set objNode = Nothing
            If Not objTransable Is Nothing Then Set objTransable = Nothing
            If Not objTransition Is Nothing Then Set objTransition = Nothing
            If Not objLocalTimelineObject Is Nothing Then Set objLocalTimelineObject = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub



' **************************************************************************************************************************************
' * PUBLIC INTERFACE- VIEW PROCEDURES  (PROCEDURES GET INFO FROM A TIMELINE OBJECT & DISPLAY IN LISTVIEW)
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: ViewTimelineInfo
            ' * procedure description: updates the ide/listview to display the given objTimeline information
            ' *
            ' ******************************************************************************************************************************
            Public Sub ViewTimelineInfo(ctrlListView As ListView, objTimeline As AMTimeline)
            Dim nDirty As Long
            Dim nInsertMode As Long
            Dim dblDuration As Double
            Dim objListItem As ListItem
            Dim nEffectsEnabled As Long
            Dim dblDefaultFPS As Double
            Dim bstrDefaultEffect As String
            Dim nTransitionsEnabled As Long
            Dim bstrDefaultTransition As String
            On Local Error GoTo ErrLine
            
            'obtain property value(s)
            objTimeline.GetInsertMode nInsertMode
            objTimeline.TransitionsEnabled nTransitionsEnabled
            objTimeline.EffectsEnabled nEffectsEnabled
            objTimeline.GetDefaultFPS dblDefaultFPS
            objTimeline.IsDirty nDirty
            bstrDefaultTransition = objTimeline.GetDefaultTransitionB
            bstrDefaultEffect = objTimeline.GetDefaultEffectB
            
            'append timeline's information to the listview
            Set objListItem = ctrlListView.ListItems.Add(4, "InsertMode", "InsertMode")
            If nInsertMode = 2 Then
                objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = "INSERT"
            Else: objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = "OVERLAY"
            End If
            Set objListItem = ctrlListView.ListItems.Add(5, "TransitionsEnabled", "TransitionsEnabled")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(nTransitionsEnabled)
            Set objListItem = ctrlListView.ListItems.Add(6, "EffectsEnabled", "EffectsEnabled")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(nEffectsEnabled)
            Set objListItem = ctrlListView.ListItems.Add(7, "Duration", "Duration")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblDuration)
            Set objListItem = ctrlListView.ListItems.Add(8, "DefaultFPS", "DefaultFPS")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblDefaultFPS)
            Set objListItem = ctrlListView.ListItems.Add(9, "Dirty", "Dirty")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(nDirty)
            Set objListItem = ctrlListView.ListItems.Add(10, "DefaultTransition", "DefaultTransition")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = bstrDefaultTransition
            Set objListItem = ctrlListView.ListItems.Add(11, "DefaultEffect", "DefaultEffect")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = bstrDefaultEffect
            
            'clean-up & dereference
            If Not objListItem Is Nothing Then Set objListItem = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: ViewGroupInfo
            ' * procedure description: updates the ide/listview to display the given group information
            ' *
            ' ******************************************************************************************************************************
            Public Sub ViewGroupInfo(ctrlListView As ListView, objTimelineObject As AMTimelineObj)
            Dim nPriority As Long
            Dim nMediaType As Long
            Dim objListItem As ListItem
            Dim nPreviewMode As Long
            Dim dblOutputFPS As Double
            Dim bstrGroupName As String
            Dim nOutputBuffering As Long
            Dim objGroup As AMTimelineGroup
            On Local Error GoTo ErrLine
            
            'obtain group from timeline object
            Set objGroup = objTimelineObject
            
            'obtain property value(s)
            objGroup.GetPriority nPriority
            objGroup.GetOutputFPS dblOutputFPS
            objGroup.GetPreviewMode nPreviewMode
            objGroup.GetOutputBuffering nOutputBuffering
            
            'append the timeline's information to the listview control
            Set objListItem = ctrlListView.ListItems.Add(4, "GroupName", "GroupName")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(bstrGroupName)
            Set objListItem = ctrlListView.ListItems.Add(5, "MediaType", "MediaType")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(nMediaType)
            Set objListItem = ctrlListView.ListItems.Add(6, "Priority", "Priority")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(nPriority)
            Set objListItem = ctrlListView.ListItems.Add(7, "OutputFPS", "OutputFPS")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblOutputFPS)
            Set objListItem = ctrlListView.ListItems.Add(8, "PreviewMode", "PreviewMode")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(nPreviewMode)
            Set objListItem = ctrlListView.ListItems.Add(9, "Buffering", "Buffering")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(nOutputBuffering)
            
            'clean-up & dereference
            If Not objGroup Is Nothing Then Set objGroup = Nothing
            If Not objListItem Is Nothing Then Set objListItem = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: ViewCompositeInfo
            ' * procedure description: updates the ide/listview to display the given composition information
            ' *
            ' ******************************************************************************************************************************
            Public Sub ViewCompositeInfo(ctrlListView As ListView, objTimelineObject As AMTimelineObj)
            Dim nPriority As Long
            Dim objListItem As ListItem
            Dim dblStartTime As Double
            Dim dblStopTime As Double
            Dim objVirtualTrack As IAMTimelineVirtualTrack
            On Local Error GoTo ErrLine
            
            'obtain the virtual track from the timeline object
            Set objVirtualTrack = objTimelineObject
            
            'obtain property value(s)
            objVirtualTrack.TrackGetPriority nPriority
            objTimelineObject.GetStartStop2 dblStartTime, dblStopTime
            
            'append the composite's information to the listview control
            Set objListItem = ctrlListView.ListItems.Add(4, "Priority", "Priority")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(nPriority)
            Set objListItem = ctrlListView.ListItems.Add(5, "Start", "Start")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblStartTime)
            Set objListItem = ctrlListView.ListItems.Add(6, "Stop", "Stop")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblStopTime)
            
            'clean-up & dereference
            If Not objListItem Is Nothing Then Set objListItem = Nothing
            If Not objVirtualTrack Is Nothing Then Set objVirtualTrack = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: ViewTransitionInfo
            ' * procedure description: updates the ide/listview to display the given transition information
            ' *
            ' ******************************************************************************************************************************
            Public Sub ViewTransitionInfo(ctrlListView As ListView, objTimelineObject As AMTimelineObj)
            Dim nCutsOnly As Long
            Dim nSwapInputs As Long
            Dim dblCutpoint As Double
            Dim objListItem As ListItem
            Dim dblStartTime As Double
            Dim dblStopTime As Double
            Dim objTransition As AMTimelineTrans
            On Local Error GoTo ErrLine
            
            'obtain transition from given timeline object
            Set objTransition = objTimelineObject
            
            'obtain property values
            objTimelineObject.GetStartStop2 dblStartTime, dblStopTime
            objTransition.GetCutPoint2 dblCutpoint
            objTransition.GetCutsOnly nCutsOnly
            objTransition.GetSwapInputs nSwapInputs
            
            'append transition's information to the listview control
            Set objListItem = ctrlListView.ListItems.Add(4, "CLSID", "CLSID")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(objTimelineObject.GetSubObjectGUIDB)
            Set objListItem = ctrlListView.ListItems.Add(5, "Start", "Start")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblStartTime)
            Set objListItem = ctrlListView.ListItems.Add(6, "Stop", "Stop")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblStopTime)
            Set objListItem = ctrlListView.ListItems.Add(7, "CutPoint", "CutPoint")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblCutpoint)
            Set objListItem = ctrlListView.ListItems.Add(8, "CutsOnly", "CutsOnly")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(nCutsOnly)
            Set objListItem = ctrlListView.ListItems.Add(9, "SwapIputs", "SwapIputs")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(nSwapInputs)
            
            'clean-up & dereference
            If Not objTransition Is Nothing Then Set objTransition = Nothing
            If Not objListItem Is Nothing Then Set objListItem = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: ViewEffectInfo
            ' * procedure description: updates the ide/listview to display the given effect information
            ' *
            ' ******************************************************************************************************************************
            Public Sub ViewEffectInfo(ctrlListView As ListView, objTimelineObject As AMTimelineObj)
            Dim nPriority As Long
            Dim objListItem As ListItem
            Dim dblStartTime As Double
            Dim dblStopTime As Double
            Dim objEffect As AMTimelineEffect
            On Local Error GoTo ErrLine
            
            'obtain effect object from timeline object
            Set objEffect = objTimelineObject
            
            'obtain property values
            objEffect.EffectGetPriority nPriority
            objTimelineObject.GetStartStop2 dblStartTime, dblStopTime
            
            'append the effect's information to the listview control
            Set objListItem = ctrlListView.ListItems.Add(4, "CLSID", "CLSID")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(objTimelineObject.GetSubObjectGUIDB)
            Set objListItem = ctrlListView.ListItems.Add(5, "Start", "Start")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblStartTime)
            Set objListItem = ctrlListView.ListItems.Add(6, "Stop", "Stop")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblStopTime)
            
            'clean-up & dereference
            If Not objEffect Is Nothing Then Set objEffect = Nothing
            If Not objListItem Is Nothing Then Set objListItem = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: ViewTrackInfo
            ' * procedure description: updates the ide/listview to display the given track information
            ' *
            ' ******************************************************************************************************************************
            Public Sub ViewTrackInfo(ctrlListView As ListView, objTimelineObject As AMTimelineObj)
            Dim nBlank As Long
            Dim nPriority As Long
            Dim nMuted As Long
            Dim dblStartTime As Double
            Dim dblStopTime As Double
            Dim objListItem As ListItem
            Dim objTimelineTrack As AMTimelineTrack
            Dim objTimelineVirtualTrack As IAMTimelineVirtualTrack
            On Local Error GoTo ErrLine
            
            'obtain timeline track from given timeline object
            Set objTimelineTrack = objTimelineObject
            'obtain virtual timeline track from given timeline object
            Set objTimelineVirtualTrack = objTimelineTrack
            
            'get track property values
            objTimelineTrack.AreYouBlank nBlank
            objTimelineObject.GetMuted nMuted
            objTimelineObject.GetStartStop2 dblStartTime, dblStopTime
            objTimelineVirtualTrack.TrackGetPriority nPriority
            
            'append the track's information to the listview control
            Set objListItem = ctrlListView.ListItems.Add(4, "Priority", "Priority")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(nPriority)
            Set objListItem = ctrlListView.ListItems.Add(5, "Blank", "Blank")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(nBlank)
            Set objListItem = ctrlListView.ListItems.Add(6, "Muted", "Muted")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(nMuted)
            Set objListItem = ctrlListView.ListItems.Add(7, "Start", "Start")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblStartTime)
            Set objListItem = ctrlListView.ListItems.Add(8, "Stop", "Stop")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblStopTime)
            
            'dereference & clean-up
            If Not objListItem Is Nothing Then Set objListItem = Nothing
            If Not objTimelineTrack Is Nothing Then Set objTimelineTrack = Nothing
            If Not objTimelineVirtualTrack Is Nothing Then Set objTimelineVirtualTrack = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: ViewSourceInfo
            ' * procedure description: updates the ide/listview to display the given source information
            ' *
            ' ******************************************************************************************************************************
            Public Sub ViewSourceInfo(ctrlListView As ListView, objTimelineObject As AMTimelineObj)
            Dim nMuted As Long
            Dim dblTStart As Double
            Dim dblTStop As Double
            Dim nStretchMode As Long
            Dim objListItem As ListItem
            Dim dblStartTime As Double
            Dim nStreamNumber As Long
            Dim dblDefaultFPS As Double
            Dim bstrMediaName As String
            Dim dblMediaLength As Double
            Dim dblMediaStartTime As Double
            Dim dblMediaStopTime As Double
            Dim objTimelineSource As AMTimelineSrc
            On Local Error GoTo ErrLine
            
            'obtain source from given timeline object
            Set objTimelineSource = objTimelineObject
            
            'obtain property values given the source object
            bstrMediaName = objTimelineSource.GetMediaName
            objTimelineSource.GetMediaTimes2 dblMediaStartTime, dblMediaStopTime
            objTimelineObject.GetStartStop2 dblTStart, dblTStop
            objTimelineSource.GetDefaultFPS dblDefaultFPS
            objTimelineSource.GetStreamNumber nStreamNumber
            objTimelineSource.GetStretchMode nStretchMode
            objTimelineObject.GetMuted nMuted
            
            'append source information on to the listview control
            Set objListItem = ctrlListView.ListItems.Add(4, "MediaName", "MediaName")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(bstrMediaName)
            Set objListItem = ctrlListView.ListItems.Add(5, "MStart", "MStart")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblMediaStartTime)
            Set objListItem = ctrlListView.ListItems.Add(6, "MStop", "MStop")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblMediaStopTime)
            Set objListItem = ctrlListView.ListItems.Add(7, "TStart", "TStart")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblTStart)
            Set objListItem = ctrlListView.ListItems.Add(8, "TStop", "TStop")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblTStop)
            Set objListItem = ctrlListView.ListItems.Add(9, "MediaLength", "MediaLength")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblMediaLength)
            Set objListItem = ctrlListView.ListItems.Add(10, "StreamNumber", "StreamNumber")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(nStreamNumber)
            Set objListItem = ctrlListView.ListItems.Add(11, "FPS", "FPS")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(dblDefaultFPS)
            Set objListItem = ctrlListView.ListItems.Add(12, "StretchMode", "StretchMode")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(nStretchMode)
            Set objListItem = ctrlListView.ListItems.Add(13, "Muted", "Muted")
            objListItem.SubItems(ctrlListView.ColumnHeaders("Value").SubItemIndex) = CStr(nMuted)
            
            'clean-up & dereference
            If Not objListItem Is Nothing Then Set objListItem = Nothing
            If Not objTimelineSource Is Nothing Then Set objTimelineSource = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
