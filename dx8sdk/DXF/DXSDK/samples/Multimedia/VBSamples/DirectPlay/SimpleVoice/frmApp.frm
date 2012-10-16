VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmApp 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Simple Voice"
   ClientHeight    =   3465
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   4755
   Icon            =   "frmApp.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3465
   ScaleWidth      =   4755
   StartUpPosition =   3  'Windows Default
   Begin MSComctlLib.ListView lvMembers 
      Height          =   3075
      Left            =   120
      TabIndex        =   0
      Top             =   300
      Width           =   4575
      _ExtentX        =   8070
      _ExtentY        =   5424
      View            =   3
      LabelEdit       =   1
      LabelWrap       =   -1  'True
      HideSelection   =   -1  'True
      _Version        =   393217
      ForeColor       =   -2147483640
      BackColor       =   -2147483643
      BorderStyle     =   1
      Appearance      =   1
      NumItems        =   2
      BeginProperty ColumnHeader(1) {BDD1F052-858B-11D1-B16A-00C0F0283628} 
         Text            =   "Name"
         Object.Width           =   2540
      EndProperty
      BeginProperty ColumnHeader(2) {BDD1F052-858B-11D1-B16A-00C0F0283628} 
         SubItemIndex    =   1
         Text            =   "Status"
         Object.Width           =   2469
      EndProperty
   End
   Begin VB.Label lblInfo 
      BackStyle       =   0  'Transparent
      Caption         =   "Members of this conversation:"
      Height          =   255
      Left            =   180
      TabIndex        =   1
      Top             =   60
      Width           =   3855
   End
End
Attribute VB_Name = "frmApp"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmApp.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Implements DirectPlay8Event
Implements DirectPlayVoiceEvent8

Private Sub Form_Load()
    'Init our vars
    InitDPlay
    
    'Now we can create a new Connection Form (which will also be our message pump)
    Set DPlayEventsForm = New DPlayConnect
    'First lets get the dplay connection started
    If Not DPlayEventsForm.StartConnectWizard(dx, dpp, AppGuid, 10, Me) Then
        Unload Me
    End If
    
    'Am I the host?
    fAmHost = DPlayEventsForm.IsHost
    
    'First let's set up the DirectPlayVoice stuff since that's the point of this demo
    If fAmHost Then
        'After we've created the session and let's start
        'the DplayVoice server
        Dim oSession As DVSESSIONDESC
        
        'Create our DPlayVoice Server
        Set dvServer = dx.DirectPlayVoiceServerCreate
            
        'Set up the Session
        oSession.lBufferAggressiveness = DVBUFFERAGGRESSIVENESS_DEFAULT
        oSession.lBufferQuality = DVBUFFERQUALITY_DEFAULT
        oSession.lSessionType = DVSESSIONTYPE_PEER
        oSession.guidCT = vbNullString
        
        'Init and start the session
        dvServer.Initialize dpp, 0
        dvServer.StartSession oSession, 0
    
        Dim oSound As DVSOUNDDEVICECONFIG
        Dim oClient As DVCLIENTCONFIG
        'Now create a client as well (so we can both talk and listen)
        Set dvClient = dx.DirectPlayVoiceClientCreate
        'Now let's create a client event..
        dvClient.StartClientNotification Me
        dvClient.Initialize dpp, 0
        'Set up our client and sound structs
        oClient.lFlags = DVCLIENTCONFIG_AUTOVOICEACTIVATED Or DVCLIENTCONFIG_AUTORECORDVOLUME
        oClient.lBufferAggressiveness = DVBUFFERAGGRESSIVENESS_DEFAULT
        oClient.lBufferQuality = DVBUFFERQUALITY_DEFAULT
        oClient.lNotifyPeriod = 0
        oClient.lThreshold = DVTHRESHOLD_UNUSED
        oClient.lPlaybackVolume = DVPLAYBACKVOLUME_DEFAULT
        oSound.hwndAppWindow = Me.hwnd
        
        On Error Resume Next
        'Connect the client
        dvClient.Connect oSound, oClient, 0
        If Err.Number = DVERR_RUN_SETUP Then    'The audio tests have not been run on this
                                                'machine.  Run them now.
            'we need to run setup first
            Dim dvSetup As DirectPlayVoiceTest8
            
            Set dvSetup = dx.DirectPlayVoiceTestCreate
            dvSetup.CheckAudioSetup vbNullString, vbNullString, Me.hwnd, 0 'Check the default devices since that's what we'll be using
            If Err.Number = DVERR_COMMANDALREADYPENDING Then
                MsgBox "Could not start DirectPlayVoice.  The Voice Networking wizard is already open.  This sample must exit.", vbOKOnly Or vbInformation, "No Voice"
                Cleanup
                Unload Me
                End
            End If
            If Err.Number = DVERR_USERCANCEL Then
                MsgBox "Could not start DirectPlayVoice.  The Voice Networking wizard has been cancelled.  This sample must exit.", vbOKOnly Or vbInformation, "No Voice"
                Cleanup
                Unload Me
                End
            End If
            Set dvSetup = Nothing
            dvClient.Connect oSound, oClient, 0
        ElseIf Err.Number <> 0 And Err.Number <> DVERR_PENDING Then
            MsgBox "Could not start DirectPlayVoice.  This sample must exit." & vbCrLf & "Error:" & CStr(Err.Number), vbOKOnly Or vbCritical, "Exiting"
            Cleanup
            Unload Me
            End
        End If
    End If
End Sub

Private Sub UpdateList(ByVal lPlayerID As Long, fTalking As Boolean)
    Dim lCount As Long
    For lCount = lvMembers.ListItems.Count To 1 Step -1
        If lvMembers.ListItems.Item(lCount).Key = "K" & CStr(lPlayerID) Then
            'Change this guys status
            If fTalking Then
                lvMembers.ListItems.Item(lCount).SubItems(1) = "Talking"
            Else
                lvMembers.ListItems.Item(lCount).SubItems(1) = "Silent"
            End If
        End If
    Next
End Sub


Private Sub Form_Unload(Cancel As Integer)
    Me.Hide
    DPlayEventsForm.DoSleep 50
    Cleanup
End Sub

Public Sub UpdatePlayerList()
    'Get everyone who is currently in the session and add them if we don't have them currently.
    Dim lCount As Long
    Dim Player As DPN_PLAYER_INFO
    
    ' Enumerate players
    For lCount = 1 To dpp.GetCountPlayersAndGroups(DPNENUM_PLAYERS)
        If Not (AmIInList(dpp.GetPlayerOrGroup(lCount))) Then 'Add this player
            Dim lItem As ListItem, sName As String
            Player = dpp.GetPeerInfo(dpp.GetPlayerOrGroup(lCount))
            sName = Player.Name
            If sName = vbNullString Then sName = "Unknown"
            If (Player.lPlayerFlags And DPNPLAYER_LOCAL = DPNPLAYER_LOCAL) Then glMyID = dpp.GetPlayerOrGroup(lCount)
            Set lItem = lvMembers.ListItems.Add(, "K" & CStr(dpp.GetPlayerOrGroup(lCount)), sName)
            lItem.SubItems(1) = "Silent"
        End If
    Next lCount
End Sub

Private Function AmIInList(ByVal lPlayerID As Long) As Boolean
    Dim lCount As Long, fInThis As Boolean
    
    For lCount = lvMembers.ListItems.Count To 1 Step -1
        If lvMembers.ListItems.Item(lCount).Key = "K" & CStr(lPlayerID) Then
            fInThis = True
        End If
    Next
    AmIInList = fInThis
End Function

Private Sub RemovePlayer(ByVal lPlayerID As Long)
    Dim lCount As Long
    
    For lCount = lvMembers.ListItems.Count To 1 Step -1
        If lvMembers.ListItems.Item(lCount).Key = "K" & CStr(lPlayerID) Then
            lvMembers.ListItems.Remove lCount
        End If
    Next
End Sub

Private Sub DirectPlay8Event_AddRemovePlayerGroup(ByVal lMsgID As Long, ByVal lPlayerID As Long, ByVal lGroupID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_AppDesc(fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_AsyncOpComplete(dpnotify As DxVBLibA.DPNMSG_ASYNC_OP_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_ConnectComplete(dpnotify As DxVBLibA.DPNMSG_CONNECT_COMPLETE, fRejectMsg As Boolean)
    'Now we're connected, start our voice session
    Dim oSound As DVSOUNDDEVICECONFIG
    Dim oClient As DVCLIENTCONFIG
    
    If dpnotify.hResultCode <> 0 Then
        'For some reason we could not connect.  All available slots must be closed.
        MsgBox "Connect Failed.  Error: 0x" & CStr(Hex$(dpnotify.hResultCode)) & "  - This sample will now close.", vbOKOnly Or vbCritical, "Closing"
        DPlayEventsForm.CloseForm Me
    Else
        'Now create a client as well (so we can both talk and listen)
        Set dvClient = dx.DirectPlayVoiceClientCreate
        
        'Now let's create a client event..
        dvClient.StartClientNotification Me
        
        dvClient.Initialize dpp, 0
        'Set up our client and sound structs
        oClient.lFlags = DVCLIENTCONFIG_AUTOVOICEACTIVATED Or DVCLIENTCONFIG_AUTORECORDVOLUME
        oClient.lBufferAggressiveness = DVBUFFERAGGRESSIVENESS_DEFAULT
        oClient.lBufferQuality = DVBUFFERQUALITY_DEFAULT
        oClient.lNotifyPeriod = 0
        oClient.lThreshold = DVTHRESHOLD_UNUSED
        oClient.lPlaybackVolume = DVPLAYBACKVOLUME_DEFAULT
        oSound.hwndAppWindow = Me.hwnd
        
        On Error Resume Next
        'Connect the client
        dvClient.Connect oSound, oClient, 0
        If Err.Number = DVERR_RUN_SETUP Then    'The audio tests have not been run on this
                                                'machine.  Run them now.
            'we need to run setup first
            Dim dvSetup As DirectPlayVoiceTest8
            
            Set dvSetup = dx.DirectPlayVoiceTestCreate
            dvSetup.CheckAudioSetup vbNullString, vbNullString, Me.hwnd, 0 'Check the default devices since that's what we'll be using
            If Err.Number = DVERR_COMMANDALREADYPENDING Then
                MsgBox "Could not start DirectPlayVoice.  The Voice Networking wizard is already open.  This sample must exit.", vbOKOnly Or vbInformation, "No Voice"
                DPlayEventsForm.CloseForm Me
            End If
            If Err.Number = DVERR_USERCANCEL Then
                MsgBox "Could not start DirectPlayVoice.  The Voice Networking wizard has been cancelled.  This sample must exit.", vbOKOnly Or vbInformation, "No Voice"
                DPlayEventsForm.CloseForm Me
            End If
            Set dvSetup = Nothing
            dvClient.Connect oSound, oClient, 0
        ElseIf Err.Number <> 0 And Err.Number <> DVERR_PENDING Then
            MsgBox "Could not start DirectPlayVoice.  This sample must exit." & vbCrLf & "Error:" & CStr(Err.Number), vbOKOnly Or vbCritical, "Exiting"
            DPlayEventsForm.CloseForm Me
            Exit Sub
        End If
    End If
End Sub

Private Sub DirectPlay8Event_CreateGroup(ByVal lGroupID As Long, ByVal lOwnerID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_CreatePlayer(ByVal lPlayerID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_DestroyGroup(ByVal lGroupID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_DestroyPlayer(ByVal lPlayerID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_EnumHostsQuery(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_QUERY, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_EnumHostsResponse(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_RESPONSE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_HostMigrate(ByVal lNewHostID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_IndicateConnect(dpnotify As DxVBLibA.DPNMSG_INDICATE_CONNECT, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_IndicatedConnectAborted(fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_InfoNotify(ByVal lMsgID As Long, ByVal lNotifyID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_Receive(dpnotify As DxVBLibA.DPNMSG_RECEIVE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_SendComplete(dpnotify As DxVBLibA.DPNMSG_SEND_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_TerminateSession(dpnotify As DxVBLibA.DPNMSG_TERMINATE_SESSION, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlayVoiceEvent8_ConnectResult(ByVal ResultCode As Long)
    Dim lTargets(0) As Long
    
    If ResultCode = 0 Then
        lTargets(0) = DVID_ALLPLAYERS
        dvClient.SetTransmitTargets lTargets, 0
        
        'Update the list
        UpdatePlayerList
    Else
        MsgBox "Could not start DirectPlayVoice.  This sample must exit." & vbCrLf & "Error:" & CStr(Err.Number), vbOKOnly Or vbCritical, "Exiting"
        DPlayEventsForm.CloseForm Me
    End If
End Sub

Private Sub DirectPlayVoiceEvent8_CreateVoicePlayer(ByVal playerID As Long, ByVal flags As Long)
    'Someone joined, update the player list
    UpdatePlayerList
End Sub

Private Sub DirectPlayVoiceEvent8_DeleteVoicePlayer(ByVal playerID As Long)
    'Someone quit, remove them from the session
    RemovePlayer playerID
End Sub

Private Sub DirectPlayVoiceEvent8_DisconnectResult(ByVal ResultCode As Long)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlayVoiceEvent8_HostMigrated(ByVal NewHostID As Long, ByVal NewServer As DxVBLibA.DirectPlayVoiceServer8)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlayVoiceEvent8_InputLevel(ByVal PeakLevel As Long, ByVal RecordVolume As Long)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlayVoiceEvent8_OutputLevel(ByVal PeakLevel As Long, ByVal OutputVolume As Long)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlayVoiceEvent8_PlayerOutputLevel(ByVal playerID As Long, ByVal PeakLevel As Long)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlayVoiceEvent8_PlayerVoiceStart(ByVal playerID As Long)
    'Someone is talking, update the list
    UpdateList playerID, True
End Sub

Private Sub DirectPlayVoiceEvent8_PlayerVoiceStop(ByVal playerID As Long)
    'Someone stopped talking, update the list
    UpdateList playerID, False
End Sub

Private Sub DirectPlayVoiceEvent8_RecordStart(ByVal PeakVolume As Long)
    'I am talking, update the list
    UpdateList glMyID, True
End Sub

Private Sub DirectPlayVoiceEvent8_RecordStop(ByVal PeakVolume As Long)
    'I have quit talking, update the list
    UpdateList glMyID, False
End Sub

Private Sub DirectPlayVoiceEvent8_SessionLost(ByVal ResultCode As Long)
    'The voice session has exited, let's quit
    MsgBox "The DirectPlayVoice session was lost.  This sample is exiting.", vbOKOnly Or vbInformation, "Session lost."
    DPlayEventsForm.CloseForm Me
End Sub

