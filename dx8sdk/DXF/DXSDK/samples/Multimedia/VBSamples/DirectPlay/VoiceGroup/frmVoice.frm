VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmVoice 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "DirectPlay Voice Sample"
   ClientHeight    =   3285
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   5985
   Icon            =   "frmVoice.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3285
   ScaleWidth      =   5985
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdClient 
      Caption         =   "Settings"
      Default         =   -1  'True
      Height          =   375
      Left            =   4680
      TabIndex        =   7
      Top             =   300
      Width           =   1215
   End
   Begin VB.CommandButton cmdCancel 
      Cancel          =   -1  'True
      Caption         =   "Exit"
      Height          =   375
      Left            =   4680
      TabIndex        =   6
      Top             =   780
      Width           =   1215
   End
   Begin VB.ComboBox cboTalkingGroup 
      Height          =   315
      ItemData        =   "frmVoice.frx":0442
      Left            =   1980
      List            =   "frmVoice.frx":0458
      Style           =   2  'Dropdown List
      TabIndex        =   5
      Top             =   2880
      Width           =   2595
   End
   Begin VB.ComboBox cboMyGroup 
      Height          =   315
      ItemData        =   "frmVoice.frx":0498
      Left            =   1980
      List            =   "frmVoice.frx":04AE
      Style           =   2  'Dropdown List
      TabIndex        =   3
      Top             =   2520
      Width           =   2595
   End
   Begin MSComctlLib.ListView lvMembers 
      Height          =   2175
      Left            =   60
      TabIndex        =   1
      Top             =   300
      Width           =   4515
      _ExtentX        =   7964
      _ExtentY        =   3836
      View            =   3
      LabelEdit       =   1
      LabelWrap       =   -1  'True
      HideSelection   =   -1  'True
      _Version        =   393217
      ForeColor       =   -2147483640
      BackColor       =   -2147483643
      BorderStyle     =   1
      Appearance      =   1
      NumItems        =   4
      BeginProperty ColumnHeader(1) {BDD1F052-858B-11D1-B16A-00C0F0283628} 
         Text            =   "Name"
         Object.Width           =   2540
      EndProperty
      BeginProperty ColumnHeader(2) {BDD1F052-858B-11D1-B16A-00C0F0283628} 
         SubItemIndex    =   1
         Text            =   "Status"
         Object.Width           =   1235
      EndProperty
      BeginProperty ColumnHeader(3) {BDD1F052-858B-11D1-B16A-00C0F0283628} 
         SubItemIndex    =   2
         Text            =   "Group"
         Object.Width           =   2117
      EndProperty
      BeginProperty ColumnHeader(4) {BDD1F052-858B-11D1-B16A-00C0F0283628} 
         SubItemIndex    =   3
         Text            =   "Target"
         Object.Width           =   1764
      EndProperty
   End
   Begin VB.Label lblInfo 
      BackStyle       =   0  'Transparent
      Caption         =   "I'm talking to this group:"
      Height          =   255
      Index           =   2
      Left            =   60
      TabIndex        =   4
      Top             =   2940
      Width           =   1815
   End
   Begin VB.Label lblInfo 
      BackStyle       =   0  'Transparent
      Caption         =   "I'm currently in the group:"
      Height          =   255
      Index           =   1
      Left            =   60
      TabIndex        =   2
      Top             =   2580
      Width           =   1815
   End
   Begin VB.Label lblInfo 
      BackStyle       =   0  'Transparent
      Caption         =   "Members of this conversation:"
      Height          =   255
      Index           =   0
      Left            =   120
      TabIndex        =   0
      Top             =   60
      Width           =   3855
   End
End
Attribute VB_Name = "frmVoice"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmVoice.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Implements DirectPlayVoiceEvent8
Implements DirectPlay8Event

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

Private Sub cboMyGroup_Click()
    On Error Resume Next
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    
    'Ok, I don't want to be in this group anymore.. let's change..
    UpdateGroup glMyPlayerID, cboMyGroup.ListIndex
    'Now send a message to everyone telling them
    If DPlayEventsForm.IsHost Then
        If cboMyGroup.ListIndex = 0 Then
            RemovePlayerFromAllGroups glMyPlayerID
        Else
            RemovePlayerFromAllGroups glMyPlayerID
            'Add myself to the new group
            dpp.AddPlayerToGroup glGroupID(cboMyGroup.ListIndex), glMyPlayerID, 0
        End If
    Else
        lMsg = MSG_SERVERCHANGEGROUP
        lOffset = NewBuffer(oBuf)
        AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
        AddDataToBuffer oBuf, CLng(cboMyGroup.ListIndex), SIZE_LONG, lOffset
        dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_GUARANTEED Or DPNSEND_NOLOOPBACK
    End If
    
    lMsg = MSG_CHANGEGROUP
    lOffset = NewBuffer(oBuf)
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    AddDataToBuffer oBuf, CLng(cboMyGroup.ListIndex), SIZE_LONG, lOffset
    dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_GUARANTEED Or DPNSEND_NOLOOPBACK
End Sub

Private Sub cboTalkingGroup_Click()
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    Dim lTargets(0) As Long
    
    'Who do I want to talk to?
    If cboTalkingGroup.ListIndex = 0 Then 'Talk to everyone
        
        lTargets(0) = DVID_ALLPLAYERS
        dvClient.SetTransmitTargets lTargets, 0
    Else
        If DPlayEventsForm.IsHost Then
            lTargets(0) = glGroupID(cboTalkingGroup.ListIndex)
        Else
            lTargets(0) = GetGroupID(cboTalkingGroup.ListIndex)
        End If
        dvClient.SetTransmitTargets lTargets, 0
    End If
    
    UpdateTarget glMyPlayerID, cboTalkingGroup.ListIndex
    'Now send a message to everyone telling them
    lMsg = MSG_CHANGETALK
    lOffset = NewBuffer(oBuf)
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    AddDataToBuffer oBuf, CLng(cboTalkingGroup.ListIndex), SIZE_LONG, lOffset
    dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_GUARANTEED Or DPNSEND_NOLOOPBACK
    
End Sub

Private Sub cmdCancel_Click()
    Unload Me
End Sub

Private Sub cmdClient_Click()
    'Show the settings screen, and re-adjust them
    frmVoiceSettings.Show vbModal
    dvClient.SetClientConfig oClient
End Sub

Private Sub Form_Load()
  
    Dim dpGroupInfo As DPN_GROUP_INFO
    Dim lCount As Long
    
    DPlayEventsForm.RegisterCallback Me
    'First let's set up the DirectPlayVoice stuff since that's the point of this demo
    If DPlayEventsForm.IsHost Then
        'After we've created (and opened) the session and got the first player, let's start
        'the DplayVoice server
        
        If (dvServer Is Nothing) Then Set dvServer = dx.DirectPlayVoiceServerCreate
            
        dvServer.Initialize dpp, 0
        dvServer.StartSession oSession, 0
    
    End If
    
    'Now create a client as well (so we can both talk and listen)
    Set dvClient = dx.DirectPlayVoiceClientCreate
    'Now let's create a client event..
    dvClient.StartClientNotification Me
    dvClient.Initialize dpp, 0
    oSound.hwndAppWindow = Me.hwnd
    
    On Error Resume Next
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
    
    'We need to create our 5 groups
    For lCount = 1 To 5
        With dpGroupInfo
            .lInfoFlags = DPNINFO_NAME
            .Name = "Group" & CStr(lCount)
        End With
        dpp.CreateGroup dpGroupInfo, 0
    Next
    
    cboMyGroup.ListIndex = 0
    cboTalkingGroup.ListIndex = 0
    UpdatePlayerList
    

End Sub

Public Sub AddPlayer(ByVal lPlayerID As Long, ByVal sName As String)
    Dim lItem As ListItem
  
    Set lItem = lvMembers.ListItems.Add(, "K" & CStr(lPlayerID), sName)
    lItem.SubItems(1) = "Silent"
    lItem.SubItems(2) = cboMyGroup.List(0)
    lItem.SubItems(3) = cboTalkingGroup.List(0)
    
End Sub

Public Sub RemovePlayer(ByVal lPlayerID As Long)
    Dim lCount As Long
    For lCount = lvMembers.ListItems.Count To 1 Step -1
        If lvMembers.ListItems.Item(lCount).Key = "K" & CStr(lPlayerID) Then
            'Remove this one
            lvMembers.ListItems.Remove lCount
        End If
    Next
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Me.Hide
    DPlayEventsForm.DoSleep 50
    Cleanup
End Sub

Public Sub UpdateTarget(ByVal lPlayerID As Long, ByVal lIndex As Long)
    Dim lCount As Long
    For lCount = lvMembers.ListItems.Count To 1 Step -1
        If lvMembers.ListItems.Item(lCount).Key = "K" & CStr(lPlayerID) Then
            'Change my group
            lvMembers.ListItems.Item(lCount).SubItems(3) = cboTalkingGroup.List(lIndex)
        End If
    Next
End Sub

Public Sub UpdateGroup(ByVal lPlayerID As Long, ByVal lIndex As Long)
    Dim lCount As Long
    For lCount = lvMembers.ListItems.Count To 1 Step -1
        If lvMembers.ListItems.Item(lCount).Key = "K" & CStr(lPlayerID) Then
            'Change my group
            lvMembers.ListItems.Item(lCount).SubItems(2) = cboMyGroup.List(lIndex)
        End If
    Next
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

Private Sub UpdatePlayerList()

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
            If (Player.lPlayerFlags And DPNPLAYER_LOCAL = DPNPLAYER_LOCAL) Then glMyPlayerID = dpp.GetPlayerOrGroup(lCount)
            Set lItem = lvMembers.ListItems.Add(, "K" & CStr(dpp.GetPlayerOrGroup(lCount)), sName)
            lItem.SubItems(1) = "Silent"
            lItem.SubItems(2) = cboMyGroup.List(0)
            lItem.SubItems(3) = cboTalkingGroup.List(0)
        End If
    Next lCount
End Sub

Private Function GetGroupID(ByVal lIndex As Long) As Long
    Dim lCount As Long
    Dim dpGroup As DPN_GROUP_INFO
    
    For lCount = 1 To dpp.GetCountPlayersAndGroups(DPNENUM_GROUPS)
        dpGroup = dpp.GetGroupInfo(dpp.GetPlayerOrGroup(lCount))
        If dpGroup.Name = "Group" & CStr(lIndex) Then
            GetGroupID = dpp.GetPlayerOrGroup(lCount)
        End If
    Next lCount
End Function

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
    If dpnotify.hResultCode <> 0 Then
        'For some reason we could not connect.  All available slots must be closed.
        MsgBox "Connect Failed.  Error: 0x" & CStr(Hex$(dpnotify.hResultCode)) & "  - This sample will now close.", vbOKOnly Or vbCritical, "Closing"
        DPlayEventsForm.CloseForm Me
    End If
End Sub

Private Sub DirectPlay8Event_CreateGroup(ByVal lGroupID As Long, ByVal lOwnerID As Long, fRejectMsg As Boolean)
    Dim lGroupNum As Long
    Dim dpGroup As DPN_GROUP_INFO
    
    dpGroup = dpp.GetGroupInfo(lGroupID)
    lGroupNum = CLng(Right$(dpGroup.Name, 1))
    glGroupID(lGroupNum) = lGroupID
End Sub

Private Sub DirectPlay8Event_CreatePlayer(ByVal lPlayerID As Long, fRejectMsg As Boolean)
    Dim dpPeer As DPN_PLAYER_INFO
    
    dpPeer = dpp.GetPeerInfo(lPlayerID)
    AddPlayer lPlayerID, dpPeer.Name
    If (dpPeer.lPlayerFlags And DPNPLAYER_LOCAL) = DPNPLAYER_LOCAL Then
        glMyPlayerID = lPlayerID
    End If
End Sub

Private Sub DirectPlay8Event_DestroyGroup(ByVal lGroupID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_DestroyPlayer(ByVal lPlayerID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    RemovePlayer lPlayerID
    If lPlayerID = glMyPlayerID Then
        glMyPlayerID = 0
    End If
End Sub

Private Sub DirectPlay8Event_EnumHostsQuery(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_QUERY, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_EnumHostsResponse(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_RESPONSE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_HostMigrate(ByVal lNewHostID As Long, fRejectMsg As Boolean)
    If lNewHostID = glMyPlayerID Then
        frmVoice.Caption = frmVoice.Caption & " (HOST)"
    End If
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
    Dim lCount As Long, lOffset As Long
    Dim lMsg As Long
    Dim lIndex As Long
    
    'Here we will go through the messages
    'The first item in our byte array is the MSGID we passed in
    With dpnotify
    GetDataFromBuffer .ReceivedData, lMsg, LenB(lMsg), lOffset
    Select Case lMsg
    Case MSG_CHANGEGROUP
        GetDataFromBuffer .ReceivedData, lIndex, LenB(lIndex), lOffset
        frmVoice.UpdateGroup dpnotify.idSender, lIndex
    Case MSG_CHANGETALK
        GetDataFromBuffer .ReceivedData, lIndex, LenB(lIndex), lOffset
        frmVoice.UpdateTarget dpnotify.idSender, lIndex
    Case MSG_SERVERCHANGEGROUP
        If DPlayEventsForm.IsHost Then
            RemovePlayerFromAllGroups dpnotify.idSender
            GetDataFromBuffer .ReceivedData, lIndex, LenB(lIndex), lOffset
            If lIndex > 0 Then dpp.AddPlayerToGroup glGroupID(lIndex), dpnotify.idSender, 0
            frmVoice.UpdateGroup dpnotify.idSender, lIndex
        End If
    End Select
    End With
End Sub

Private Sub DirectPlay8Event_SendComplete(dpnotify As DxVBLibA.DPNMSG_SEND_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_TerminateSession(dpnotify As DxVBLibA.DPNMSG_TERMINATE_SESSION, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlayVoiceEvent8_ConnectResult(ByVal ResultCode As Long)
    If ResultCode <> 0 Then
        'For some reason we could not connect.  All available slots must be closed.
        MsgBox "Connect Failed.  Error: 0x" & CStr(Hex$(ResultCode)) & "  - This sample will now close.", vbOKOnly Or vbCritical, "Closing"
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
    UpdateList glMyPlayerID, True
End Sub

Private Sub DirectPlayVoiceEvent8_RecordStop(ByVal PeakVolume As Long)
    'I have quit talking, update the list
    UpdateList glMyPlayerID, False
End Sub

Private Sub DirectPlayVoiceEvent8_SessionLost(ByVal ResultCode As Long)
    'The voice session has exited, let's quit
    MsgBox "The DirectPlayVoice session was lost.  This sample is exiting.", vbOKOnly Or vbInformation, "Session lost."
    DPlayEventsForm.CloseForm Me
End Sub


