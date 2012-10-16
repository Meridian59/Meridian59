VERSION 5.00
Begin VB.Form frmStagePeer 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Staging Area"
   ClientHeight    =   4545
   ClientLeft      =   465
   ClientTop       =   435
   ClientWidth     =   8850
   Icon            =   "frmStagePeer.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4545
   ScaleWidth      =   8850
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer tmrUpdate 
      Interval        =   50
      Left            =   90
      Top             =   4125
   End
   Begin VB.CommandButton cmdCancel 
      Caption         =   "Cancel"
      Height          =   315
      Left            =   7560
      TabIndex        =   26
      Top             =   4140
      Width           =   1215
   End
   Begin VB.CommandButton cmdStartGame 
      Caption         =   "Start Game"
      Enabled         =   0   'False
      Height          =   315
      Left            =   6240
      TabIndex        =   25
      Top             =   4140
      Width           =   1275
   End
   Begin VB.Frame Frame1 
      Caption         =   "Chat"
      Height          =   3975
      Left            =   2520
      TabIndex        =   22
      Top             =   120
      Width           =   6255
      Begin VB.TextBox txtSend 
         Height          =   315
         Left            =   60
         TabIndex        =   0
         Top             =   3420
         Width           =   6135
      End
      Begin VB.TextBox txtChat 
         Height          =   3135
         Left            =   60
         Locked          =   -1  'True
         MultiLine       =   -1  'True
         ScrollBars      =   3  'Both
         TabIndex        =   23
         TabStop         =   0   'False
         Top             =   240
         Width           =   6075
      End
   End
   Begin VB.Frame fra 
      Caption         =   "Players"
      Height          =   3975
      Left            =   60
      TabIndex        =   1
      Top             =   120
      Width           =   2415
      Begin VB.CheckBox chkReady 
         Enabled         =   0   'False
         Height          =   255
         Index           =   9
         Left            =   2040
         TabIndex        =   21
         Top             =   3540
         Visible         =   0   'False
         Width           =   255
      End
      Begin VB.CheckBox chkReady 
         Enabled         =   0   'False
         Height          =   255
         Index           =   8
         Left            =   2040
         TabIndex        =   20
         Top             =   3180
         Visible         =   0   'False
         Width           =   255
      End
      Begin VB.CheckBox chkReady 
         Enabled         =   0   'False
         Height          =   255
         Index           =   7
         Left            =   2040
         TabIndex        =   19
         Top             =   2820
         Visible         =   0   'False
         Width           =   255
      End
      Begin VB.CheckBox chkReady 
         Enabled         =   0   'False
         Height          =   255
         Index           =   6
         Left            =   2040
         TabIndex        =   18
         Top             =   2460
         Visible         =   0   'False
         Width           =   255
      End
      Begin VB.CheckBox chkReady 
         Enabled         =   0   'False
         Height          =   255
         Index           =   5
         Left            =   2040
         TabIndex        =   17
         Top             =   2100
         Visible         =   0   'False
         Width           =   255
      End
      Begin VB.CheckBox chkReady 
         Enabled         =   0   'False
         Height          =   255
         Index           =   4
         Left            =   2040
         TabIndex        =   16
         Top             =   1740
         Visible         =   0   'False
         Width           =   255
      End
      Begin VB.CheckBox chkReady 
         Enabled         =   0   'False
         Height          =   255
         Index           =   3
         Left            =   2040
         TabIndex        =   15
         Top             =   1380
         Visible         =   0   'False
         Width           =   255
      End
      Begin VB.CheckBox chkReady 
         Enabled         =   0   'False
         Height          =   255
         Index           =   2
         Left            =   2040
         TabIndex        =   14
         Top             =   1020
         Visible         =   0   'False
         Width           =   255
      End
      Begin VB.CheckBox chkReady 
         Enabled         =   0   'False
         Height          =   255
         Index           =   1
         Left            =   2040
         TabIndex        =   13
         Top             =   660
         Visible         =   0   'False
         Width           =   255
      End
      Begin VB.CheckBox chkReady 
         Enabled         =   0   'False
         Height          =   255
         Index           =   0
         Left            =   2040
         TabIndex        =   12
         Top             =   300
         Visible         =   0   'False
         Width           =   255
      End
      Begin VB.ComboBox cboUser 
         Height          =   315
         Index           =   0
         Left            =   120
         Style           =   2  'Dropdown List
         TabIndex        =   11
         Top             =   300
         Visible         =   0   'False
         Width           =   1815
      End
      Begin VB.ComboBox cboUser 
         Height          =   315
         Index           =   9
         Left            =   120
         Style           =   2  'Dropdown List
         TabIndex        =   10
         Top             =   3540
         Visible         =   0   'False
         Width           =   1815
      End
      Begin VB.ComboBox cboUser 
         Height          =   315
         Index           =   8
         Left            =   120
         Style           =   2  'Dropdown List
         TabIndex        =   9
         Top             =   3180
         Visible         =   0   'False
         Width           =   1815
      End
      Begin VB.ComboBox cboUser 
         Height          =   315
         Index           =   7
         Left            =   120
         Style           =   2  'Dropdown List
         TabIndex        =   8
         Top             =   2820
         Visible         =   0   'False
         Width           =   1815
      End
      Begin VB.ComboBox cboUser 
         Height          =   315
         Index           =   6
         Left            =   120
         Style           =   2  'Dropdown List
         TabIndex        =   7
         Top             =   2460
         Visible         =   0   'False
         Width           =   1815
      End
      Begin VB.ComboBox cboUser 
         Height          =   315
         Index           =   5
         Left            =   120
         Style           =   2  'Dropdown List
         TabIndex        =   6
         Top             =   2100
         Visible         =   0   'False
         Width           =   1815
      End
      Begin VB.ComboBox cboUser 
         Height          =   315
         Index           =   4
         Left            =   120
         Style           =   2  'Dropdown List
         TabIndex        =   5
         Top             =   1740
         Visible         =   0   'False
         Width           =   1815
      End
      Begin VB.ComboBox cboUser 
         Height          =   315
         Index           =   3
         Left            =   120
         Style           =   2  'Dropdown List
         TabIndex        =   4
         Top             =   1380
         Visible         =   0   'False
         Width           =   1815
      End
      Begin VB.ComboBox cboUser 
         Height          =   315
         Index           =   2
         Left            =   120
         Style           =   2  'Dropdown List
         TabIndex        =   3
         Top             =   1020
         Visible         =   0   'False
         Width           =   1815
      End
      Begin VB.ComboBox cboUser 
         Height          =   315
         Index           =   1
         Left            =   120
         Style           =   2  'Dropdown List
         TabIndex        =   2
         Top             =   660
         Visible         =   0   'False
         Width           =   1815
      End
   End
   Begin VB.Label lblInfo 
      BackStyle       =   0  'Transparent
      Caption         =   "Click the check box when you're ready"
      Height          =   435
      Left            =   2580
      TabIndex        =   24
      Top             =   4140
      Width           =   3075
   End
End
Attribute VB_Name = "frmStagePeer"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmStagePeer.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Private Type userJoinPart
    fJoin As Boolean
    lDplayID As Long
    fIsHost As Boolean
    sUserName As String
    lSlotID As Long
End Type

Implements DirectPlay8Event
Private mlNumUserEvents As Long
Private moUsers() As userJoinPart
Dim fLoadingGame As Boolean

Private Sub cboUser_Click(Index As Integer)
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    
    'First check to see if this is my combo box.. If it is, then enable my check box
    If cboUser(Index).ItemData(0) = glMyPlayerID And (glMyPlayerID > 0) Then
        chkReady(Index).Enabled = True
    End If
    
    If DPlayEventsForm.IsHost Then
        If cboUser(Index).ListIndex = cboUser(Index).ListCount - 1 Then 'This slot is now closed
            'If we're the host kick the user in this slot out
            If cboUser(Index).ItemData(0) <> 0 And cboUser(Index).ItemData(0) <> -1 Then 'There is already a user in this slot, kick them out
                dpp.DestroyPeer cboUser(Index).ItemData(0), 0, ByVal 0&, 0
            End If
            'Notify everyone that we switched this one to closed
            lMsg = MsgCloseSlot
            lOffset = NewBuffer(oBuf)
            AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
            'Send the SlotID of this slot
            AddDataToBuffer oBuf, CLng(Index), SIZE_LONG, lOffset
            'Send this message to the joining player
            dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_NOLOOPBACK
            'Set the item data to -1 to signify this slot is closed
            cboUser(Index).ItemData(0) = -1
        ElseIf cboUser(Index).ListIndex = cboUser(Index).ListCount - 2 Then 'This slot is now open
            If cboUser(Index).ListCount > 2 Then 'There is already someone in this slot
                cboUser(Index).ListIndex = 0
                Exit Sub
            End If
            cboUser(Index).ItemData(0) = 0
            lMsg = MsgOpenSlot
            lOffset = NewBuffer(oBuf)
            AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
            'Send the SlotID of this slot
            AddDataToBuffer oBuf, CLng(Index), SIZE_LONG, lOffset
            'Send this message to the joining player
            dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_NOLOOPBACK
        End If
    End If
    'Lets see if we can start now
    UpdateStartButton
End Sub

Private Sub chkReady_Click(Index As Integer)
    'We can only click our own check box, so if we do, notify everyone else
    Dim lOffset As Long
    Dim lMsg As Long
    Dim oBuf() As Byte
    
    On Error Resume Next
    lOffset = NewBuffer(oBuf)
    lMsg = MsgClickReady
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    'Send the index
    AddDataToBuffer oBuf, Index, LenB(Index), lOffset
    'Send the value
    AddDataToBuffer oBuf, CLng(chkReady(Index).Value), SIZE_LONG, lOffset
    'Send the buffer
    dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_NOLOOPBACK
    'Lets see if we can start now
    UpdateStartButton
    
End Sub

Private Sub cmdCancel_Click()
    Unload Me
End Sub

Private Sub cmdStartGame_Click()
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    
    'It's time to start the game
    lMsg = MsgStartGame
    lOffset = NewBuffer(oBuf)
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    'Send this message to everyone
    dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_NOLOOPBACK
    'Now load the actual game form
    fLoadingGame = True
    frmApp.LoadGame GetNumPlayers
    Unload Me
End Sub

Private Sub Form_Load()

    Dim lCount As Long
    
    'Add a 'Open' and 'Closed' item to each box
    'Then make sure the box is visible along with the 'ready' check box
    For lCount = 0 To 9
        cboUser(lCount).AddItem "Open"
        cboUser(lCount).AddItem "Closed"
    Next
    
    'Oh good, we want to play a multiplayer game.
    'First lets get the dplay connection started
    
    'Here we will init our DPlay objects
    InitDPlay
    'Now we can create a new Connection Form (which will also be our message pump)
    Set DPlayEventsForm = New DPlayConnect
    
    'Start the connection form (it will either create or join a session)
    If Not DPlayEventsForm.StartConnectWizard(dx, dpp, AppGuid, 10, Me, False) Then
        Cleanup
        End
    Else 'We did choose to play a game
        gsUserName = DPlayEventsForm.UserName
        'Add a 'Open' and 'Closed' item to each box
        'Then make sure the box is visible along with the 'ready' check box
        For lCount = 0 To 9
            If DPlayEventsForm.NumPlayers > lCount Then
                If Not DPlayEventsForm.IsHost Then
                    cboUser(lCount).Enabled = False
                End If
                cboUser(lCount).ListIndex = 0
                cboUser(lCount).Visible = True
                chkReady(lCount).Visible = True
            End If
        Next
        If DPlayEventsForm.IsHost Then Me.Caption = Me.Caption & " (HOST)"
    End If
End Sub

Private Sub Form_Unload(Cancel As Integer)
    DPlayEventsForm.DoSleep 50
    If Not fLoadingGame Then
        Me.Hide
        Cleanup
    End If
End Sub

Private Function GetName(ByVal lID As Long) As String
    Dim lCount As Long
    
    'Rather than call GetPeerInfo everytime, we will just use our text in
    'our combo box.
    GetName = vbNullString
    For lCount = 0 To 9
        If cboUser(lCount).ItemData(0) = lID Then 'This is the player
            GetName = cboUser(lCount).Text
            Exit For
        End If
    Next
End Function

Private Sub tmrUpdate_Timer()
    Dim lCount As Long
    Dim lMsg As Long, oBuf() As Byte
    Dim lOffset As Long
    
    If mlNumUserEvents = 0 Then Exit Sub
    Dim oTemp As userJoinPart
    
    'Get a copy of the event
    oTemp = moUsers(mlNumUserEvents)
    'Decrement the count
    mlNumUserEvents = mlNumUserEvents - 1
    'Get rid of our array if it's no longer necessary
    If mlNumUserEvents = 0 Then Erase moUsers
    With oTemp
    Debug.Print "Got here.. Info:"; .fIsHost; .fJoin; .lDplayID; .lSlotID; .sUserName
    If .fJoin Then
        If DPlayEventsForm.IsHost Then 'If we are the host
            If Not .fIsHost Then 'Don't notify ourselves
                'We are the host, let this person join, and then tell everyone which slot to put them in
                'Find the first open slot
                'We ignore Slot 0 since that's the host's slot
                For lCount = 1 To 9
                    If cboUser(lCount).ItemData(0) = 0 Then
                        Exit For
                    End If
                Next
                'Add this user to our list
                cboUser(lCount).AddItem .sUserName, 0
                cboUser(lCount).ItemData(0) = .lDplayID
                cboUser(lCount).ListIndex = 0
                'Ok, lCount now holds the first open slot
                lMsg = MsgPutPlayerInSlot
                lOffset = NewBuffer(oBuf)
                AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
                'Now add the slot number this player is in
                AddDataToBuffer oBuf, lCount, LenB(lCount), lOffset
                'Now add the player id
                AddDataToBuffer oBuf, .lDplayID, LenB(.lDplayID), lOffset
                'Send this message to everyone
                dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_NOLOOPBACK Or DPNSEND_GUARANTEED
                'Now that everyone knows what slot to put them in, we need to tell this person
                'that just joined where everyone else already is.
                For lCount = 0 To 9
                    If (cboUser(lCount).ItemData(0) <> 0) And (cboUser(lCount).ItemData(0) <> -1) Then
                        If cboUser(lCount).ItemData(0) <> .lDplayID Then 'No need to pass this person twice
                            lMsg = MsgPutPlayerInSlot
                            lOffset = NewBuffer(oBuf)
                            AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
                            'Now add the slot number this player is in
                            AddDataToBuffer oBuf, lCount, LenB(lCount), lOffset
                            'Now add the player id
                            AddDataToBuffer oBuf, cboUser(lCount).ItemData(0), SIZE_LONG, lOffset
                            'Send this message to the joining player
                            dpp.SendTo .lDplayID, oBuf, 0, DPNSEND_NOLOOPBACK Or DPNSEND_GUARANTEED
                            'Send the state of the Ready checkbox
                            lOffset = NewBuffer(oBuf)
                            lMsg = MsgClickReady
                            AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
                            'Send the index
                            AddDataToBuffer oBuf, CInt(lCount), SIZE_INTEGER, lOffset
                            'Send the value
                            AddDataToBuffer oBuf, CLng(chkReady(lCount).Value), SIZE_LONG, lOffset
                            'Send the buffer
                            dpp.SendTo .lDplayID, oBuf, 0, DPNSEND_NOLOOPBACK Or DPNSEND_GUARANTEED
                        End If
                    ElseIf cboUser(lCount).ItemData(0) = 0 Then
                        'Open this slot on the client machine
                        lMsg = MsgOpenSlot
                        lOffset = NewBuffer(oBuf)
                        AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
                        'Send the SlotID of this slot
                        AddDataToBuffer oBuf, lCount, LenB(lCount), lOffset
                        'Send this message to the joining player
                        dpp.SendTo .lDplayID, oBuf, 0, DPNSEND_NOLOOPBACK Or DPNSEND_GUARANTEED
                    ElseIf cboUser(lCount).ItemData(0) = -1 Then
                        'Close this slot on the client machine
                        lMsg = MsgCloseSlot
                        lOffset = NewBuffer(oBuf)
                        AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
                        'Send the SlotID of this slot
                        AddDataToBuffer oBuf, lCount, LenB(lCount), lOffset
                        'Send this message to the joining player
                        dpp.SendTo .lDplayID, oBuf, 0, DPNSEND_NOLOOPBACK Or DPNSEND_GUARANTEED
                    End If
                Next
            Else
                'Add ourselves to our list
                cboUser(0).AddItem .sUserName, 0
                cboUser(0).ItemData(0) = .lDplayID
                'Lock our box so we don't change anything
                cboUser(0).Locked = True
                chkReady(0).Enabled = True
            End If
        Else
            'add this user to our list
            cboUser(.lSlotID).AddItem .sUserName, 0
            cboUser(.lSlotID).ItemData(0) = .lDplayID
            cboUser(.lSlotID).ListIndex = 0
            If (.lDplayID = glMyPlayerID) Then chkReady(.lSlotID).Enabled = True
        End If
    Else 'This is a disconnect
        For lCount = 0 To 9
            'Basically here we will scroll through each of the combo boxes.
            'For each box that we find that has a user in it, see if that user
            'is the one who just left
            If cboUser(lCount).ItemData(0) = .lDplayID Then
                'Remove this person
                cboUser(lCount).RemoveItem 0
                cboUser(lCount).ItemData(0) = 0
                cboUser(lCount).ListIndex = 0
                chkReady(lCount).Value = vbUnchecked
                Exit For
            End If
        Next
    End If
    End With
    
End Sub

Private Sub txtSend_KeyPress(KeyAscii As Integer)
    Dim lMsg As Long, lOffset As Long
    Dim sChatMsg As String
    Dim oBuf() As Byte
    
    If KeyAscii = vbKeyReturn Then
        If txtSend.Text = vbNullString Then Exit Sub
        'Send this message to everyone
        lMsg = MsgChat
        lOffset = NewBuffer(oBuf)
        AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
        sChatMsg = txtSend.Text
        AddStringToBuffer oBuf, sChatMsg, lOffset
        txtSend.Text = vbNullString
        KeyAscii = 0
        dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_NOLOOPBACK
        UpdateChat "<" & gsUserName & ">" & sChatMsg, txtChat
    End If
End Sub

Private Sub UpdateStartButton()
    Dim lCount As Long
    Dim fReady As Boolean
    Dim lNumPlayers As Long
    
    'Here we will update the start button and any text to let the user know
    'what is going on
    lNumPlayers = 0
    If DPlayEventsForm.IsHost Then
        'The host can enable the start button when everyone is ready.
        'Assume we're ready
        fReady = True
        For lCount = 0 To 9
            'Basically here we will scroll through each of the combo boxes.
            'For each box that we find that has a user in it, see if that user
            'has clicked the Ready box.
            If (cboUser(lCount).ItemData(0) <> 0) And (cboUser(lCount).ItemData(0) <> -1) Then
                If chkReady(lCount).Value <> vbChecked Then
                    'We are not ready
                    fReady = False
                End If
                'Increment the number of players
                lNumPlayers = lNumPlayers + 1
            End If
        Next
        
        'Are we ready?
        If lNumPlayers < 2 Then
            lblInfo.Caption = "Waiting for more players to join..."
        Else
            cmdStartGame.Enabled = fReady
            If fReady Then
                lblInfo.Caption = "You can start the session anytime..."
            Else
                lblInfo.Caption = "Waiting for everyone to click ready..."
            End If
        End If
    Else
        'The only thing we can do is update the text here.  It is not possible
        'for someone who is not the host to start the session.
        
        'Assume we're ready
        fReady = True
        For lCount = 0 To 9
            'Basically here we will scroll through each of the combo boxes.
            'For each box that we find that has a user in it, see if that user
            'has clicked the Ready box.
            If (cboUser(lCount).ItemData(0) <> 0) And (cboUser(lCount).ItemData(0) <> -1) Then
                If chkReady(lCount).Value <> vbChecked Then
                    'We are not ready
                    fReady = False
                    Exit For
                End If
            End If
        Next
        
        'Are we ready?
        If fReady Then
            lblInfo.Caption = "Waiting for the host to start the session..."
        Else
            lblInfo.Caption = "Waiting for everyone to click ready..."
        End If
    End If
    
End Sub

Private Function GetNumPlayers() As Long
    Dim lCount As Long

    For lCount = 0 To 9
        'Basically here we will scroll through each of the combo boxes.
        If (cboUser(lCount).ItemData(0) <> 0) And (cboUser(lCount).ItemData(0) <> -1) Then
            'Increment the number of players
            GetNumPlayers = GetNumPlayers + 1
        End If
    Next

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
    If dpnotify.hResultCode = DPNERR_HOSTREJECTEDCONNECTION Then
        'For some reason we could not connect.  All available slots must be closed.
        MsgBox "All available slots in this session were closed by the host.", vbOKOnly Or vbInformation, "No open slot"
        DPlayEventsForm.CloseForm Me
    ElseIf dpnotify.hResultCode <> 0 Then
        'For some reason we could not connect.  All available slots must be closed.
        MsgBox "Connect Failed.  Error: 0x" & CStr(Hex$(dpnotify.hResultCode)) & "  - This sample will now close.", vbOKOnly Or vbCritical, "Closing"
        DPlayEventsForm.CloseForm Me
    End If
End Sub

Private Sub DirectPlay8Event_CreateGroup(ByVal lGroupID As Long, ByVal lOwnerID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_CreatePlayer(ByVal lPlayerID As Long, fRejectMsg As Boolean)
    Dim dpPeer As DPN_PLAYER_INFO
    
    'Get the peer info for this player and see if it's us..
    dpPeer = dpp.GetPeerInfo(lPlayerID)
    If (dpPeer.lPlayerFlags And DPNPLAYER_LOCAL) = DPNPLAYER_LOCAL Then
        glMyPlayerID = lPlayerID
    End If
    If (DPlayEventsForm.IsHost) Then
        Dim lTemp As Long
        
        lTemp = mlNumUserEvents + 1
        ReDim Preserve moUsers(lTemp)
        With moUsers(lTemp)
            .fJoin = True
            .lDplayID = lPlayerID
            .sUserName = dpPeer.Name
            .fIsHost = (dpPeer.lPlayerFlags And DPNPLAYER_HOST) = DPNPLAYER_HOST
        End With
        mlNumUserEvents = lTemp
    End If
End Sub

Private Sub DirectPlay8Event_DestroyGroup(ByVal lGroupID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_DestroyPlayer(ByVal lPlayerID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'Someone just left, remove there slot and make it open again
    Dim lTemp As Long
    
    lTemp = mlNumUserEvents + 1
    ReDim Preserve moUsers(lTemp)
    With moUsers(lTemp)
        .fJoin = False
        .lDplayID = lPlayerID
    End With
    mlNumUserEvents = lTemp
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
    'Here we will check to see if there are any open slots for the user to join
    Dim lCount As Long
    Dim fFoundOpenSlot As Boolean
    fFoundOpenSlot = False
    For lCount = 0 To 9
        If CLng(cboUser(lCount).ItemData(0)) = 0 Then
            fFoundOpenSlot = True
            Exit For
        End If
    Next
    If Not fFoundOpenSlot Then 'There are no open slots
        fRejectMsg = True 'Reject the message and do not let them join.
    End If
    
End Sub

Private Sub DirectPlay8Event_IndicatedConnectAborted(fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_InfoNotify(ByVal lMsgID As Long, ByVal lNotifyID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_Receive(dpnotify As DxVBLibA.DPNMSG_RECEIVE, fRejectMsg As Boolean)
    Dim lMsg As Long, lOffset As Long
    Dim dpPeer As DPN_PLAYER_INFO, sName As String
    Dim sChat As String
    Dim lPlayerID As Long, lSlotID As Long
    Dim iIndex As Integer, lValue As Long
    
    With dpnotify
    GetDataFromBuffer .ReceivedData, lMsg, LenB(lMsg), lOffset
    Select Case lMsg
    Case MsgPutPlayerInSlot
        GetDataFromBuffer .ReceivedData, lSlotID, LenB(lSlotID), lOffset
        GetDataFromBuffer .ReceivedData, lPlayerID, LenB(lPlayerID), lOffset
        dpPeer = dpp.GetPeerInfo(lPlayerID)
        
        Dim lTemp As Long
        
        lTemp = mlNumUserEvents + 1
        ReDim Preserve moUsers(lTemp)
        With moUsers(lTemp)
            .fJoin = True
            .lDplayID = lPlayerID
            .sUserName = dpPeer.Name
            .fIsHost = False
            .lSlotID = lSlotID
        End With
        mlNumUserEvents = lTemp
    Case MsgOpenSlot 'Open this slot
        GetDataFromBuffer .ReceivedData, lSlotID, LenB(lSlotID), lOffset
        cboUser(lSlotID).ListIndex = cboUser(lSlotID).ListCount - 2
    Case MsgCloseSlot 'Close this slot
        GetDataFromBuffer .ReceivedData, lSlotID, LenB(lSlotID), lOffset
        If cboUser(lSlotID).ListCount > 2 Then 'There is someone in this slot that just got kicked
            cboUser(lSlotID).RemoveItem 0
        End If
        chkReady(lSlotID).Value = vbUnchecked
        cboUser(lSlotID).ListIndex = cboUser(lSlotID).ListCount - 1
    Case MsgChat
        sName = GetName(.idSender)
        sChat = GetStringFromBuffer(.ReceivedData, lOffset)
        UpdateChat "<" & sName & "> " & sChat, txtChat
    Case MsgClickReady
        GetDataFromBuffer .ReceivedData, iIndex, LenB(iIndex), lOffset
        GetDataFromBuffer .ReceivedData, lValue, LenB(lValue), lOffset
        chkReady(iIndex).Value = lValue
    Case MsgStartGame
        fLoadingGame = True
        frmApp.LoadGame GetNumPlayers
        Unload Me
        Exit Sub
    End Select
    End With
    'Lets see if we can start now
    UpdateStartButton
End Sub

Private Sub DirectPlay8Event_SendComplete(dpnotify As DxVBLibA.DPNMSG_SEND_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_TerminateSession(dpnotify As DxVBLibA.DPNMSG_TERMINATE_SESSION, fRejectMsg As Boolean)
    'The session just ended for some reason.  We may have been kicked out.
    If dpnotify.hResultCode = DPNERR_HOSTTERMINATEDSESSION Then
        MsgBox "The host has closed the slot you were in.  This sample will now exit.", vbOKOnly Or vbInformation, "Exiting"
    Else
        MsgBox "This session has been lost.  This sample will now exit.", vbOKOnly Or vbInformation, "Exiting"
    End If
    DPlayEventsForm.CloseForm Me
End Sub

