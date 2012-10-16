VERSION 5.00
Object = "{86CF1D34-0C5F-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCT2.OCX"
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmServer 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "DirectPlay Simple Server"
   ClientHeight    =   4875
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   3660
   Icon            =   "frmServer.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4875
   ScaleWidth      =   3660
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdStartServer 
      Caption         =   "Start Server"
      Default         =   -1  'True
      Height          =   375
      Left            =   1283
      TabIndex        =   9
      Top             =   4080
      Width           =   1095
   End
   Begin VB.ListBox lstUser 
      Height          =   1815
      Left            =   120
      TabIndex        =   8
      Top             =   2160
      Width           =   3375
   End
   Begin MSComctlLib.StatusBar sBar 
      Align           =   2  'Align Bottom
      Height          =   375
      Left            =   0
      TabIndex        =   7
      Top             =   4500
      Width           =   3660
      _ExtentX        =   6456
      _ExtentY        =   661
      Style           =   1
      _Version        =   393216
      BeginProperty Panels {8E3867A5-8586-11D1-B16A-00C0F0283628} 
         NumPanels       =   1
         BeginProperty Panel1 {8E3867AB-8586-11D1-B16A-00C0F0283628} 
         EndProperty
      EndProperty
   End
   Begin MSComCtl2.UpDown udUsers 
      Height          =   315
      Left            =   3180
      TabIndex        =   5
      Top             =   1740
      Width           =   240
      _ExtentX        =   423
      _ExtentY        =   556
      _Version        =   393216
      Value           =   50
      BuddyControl    =   "txtUsers"
      BuddyDispid     =   196611
      OrigLeft        =   1800
      OrigTop         =   660
      OrigRight       =   2040
      OrigBottom      =   975
      Max             =   1000
      Min             =   1
      SyncBuddy       =   -1  'True
      BuddyProperty   =   65547
      Enabled         =   -1  'True
   End
   Begin VB.TextBox txtUsers 
      Height          =   315
      Left            =   2760
      Locked          =   -1  'True
      TabIndex        =   4
      Text            =   "50"
      Top             =   1740
      Width           =   435
   End
   Begin VB.TextBox txtSession 
      Height          =   315
      Left            =   120
      TabIndex        =   3
      Text            =   "vbDirectPlaySession"
      Top             =   1320
      Width           =   3315
   End
   Begin VB.ListBox lstSP 
      Height          =   645
      Left            =   120
      TabIndex        =   1
      Top             =   420
      Width           =   3375
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Select the server's service provider"
      Height          =   195
      Index           =   2
      Left            =   120
      TabIndex        =   6
      Top             =   120
      Width           =   3435
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Session Name"
      Height          =   195
      Index           =   1
      Left            =   120
      TabIndex        =   2
      Top             =   1080
      Width           =   1275
   End
   Begin VB.Label lbl 
      BackStyle       =   0  'Transparent
      Caption         =   "Maximum users:"
      Height          =   255
      Index           =   0
      Left            =   240
      TabIndex        =   0
      Top             =   1800
      Width           =   2415
   End
   Begin VB.Menu mnuPop 
      Caption         =   "PopUp"
      Visible         =   0   'False
      Begin VB.Menu mnuShow 
         Caption         =   "Show"
      End
      Begin VB.Menu mnuStart 
         Caption         =   "Start Server"
      End
      Begin VB.Menu mnuSep 
         Caption         =   "-"
      End
      Begin VB.Menu mnuExit 
         Caption         =   "Exit"
      End
   End
End
Attribute VB_Name = "frmServer"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmServer.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Implements DirectPlay8Event
Private mfExit As Boolean
Private Enum MsgTypes
    Msg_NoOtherPlayers
    Msg_NumPlayers
    Msg_SendWave
End Enum

Private Sub cmdStartServer_Click()
    Dim AppDesc As DPN_APPLICATION_DESC
    
    If gfStarted Then Exit Sub
    If Val(txtUsers.Text) < 1 Then
        MsgBox "I'm sorry, you must allow at least 1 user to join your server.", vbOKOnly Or vbInformation, "Increase users"
        Exit Sub
    End If
    
    If txtSession.Text = vbNullString Then
        MsgBox "I'm sorry, you must enter a session name.", vbOKOnly Or vbInformation, "No session name"
        Exit Sub
    End If
    
    'Save our current session name for later runs
    SaveSetting "VBDirectPlay", "Defaults", "ServerGameName", txtSession.Text
    
    'Now set up the app description
    With AppDesc
        .guidApplication = AppGuid
        .lMaxPlayers = Val(txtUsers.Text)
        .SessionName = txtSession.Text
        .lFlags = DPNSESSION_CLIENT_SERVER 'We must pass the client server flags if we are a server
    End With
    
    'Now set up our address value
    dpa.SetSP dps.GetServiceProvider(lstSP.ListIndex + 1).Guid
    
    'Now start the server
    dps.Host AppDesc, dpa
    
    gfStarted = True
    sBar.SimpleText = "Server running...  (" & CStr(glNumPlayers) & "/" & txtUsers.Text & " clients connected.)"
    
    'modify our icon text
    sysIcon.sTip = "Server running...  (" & CStr(glNumPlayers) & "/" & txtUsers.Text & " clients connected.)" & vbNullChar
    sysIcon.uFlags = NIF_TIP
    Shell_NotifyIcon NIM_MODIFY, sysIcon
    
    cmdStartServer.Enabled = False
End Sub

Private Sub Form_Load()
    Dim lCount As Long
    Dim dpn As DPN_SERVICE_PROVIDER_INFO
    
    dps.RegisterMessageHandler Me
    'First load our list of Service Providers into our box
    For lCount = 1 To dps.GetCountServiceProviders
        dpn = dps.GetServiceProvider(lCount)
        lstSP.AddItem dpn.Name
        'Pick the TCP/IP connection by default
        If InStr(dpn.Name, "TCP") Then lstSP.ListIndex = lstSP.ListCount - 1
    Next
    If lstSP.ListIndex < 0 Then lstSP.ListIndex = 0
    txtSession.Text = GetSetting("VBDirectPlay", "Defaults", "ServerGameName", "vbDirectPlayServer")
    sBar.SimpleText = "Server not running..."
    
    'Lets put an icon in the system tray
    With sysIcon
        .cbSize = LenB(sysIcon)
        .hwnd = Me.hwnd
        .uFlags = NIF_DOALL
        .uCallbackMessage = WM_MOUSEMOVE
        .hIcon = Me.Icon
        .sTip = "vbDirectPlayServer - Server not running" & vbNullChar
    End With
    Shell_NotifyIcon NIM_ADD, sysIcon

End Sub

Private Sub Form_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim ShellMsg As Long
    
    ShellMsg = X / Screen.TwipsPerPixelX
    Select Case ShellMsg
    Case WM_LBUTTONDBLCLK
        mnuShow_Click
    Case WM_RBUTTONUP
        'Show the menu
        If gfStarted Then mnuStart.Enabled = False
        PopupMenu mnuPop, , , , mnuShow
    End Select
    
End Sub

Private Sub Form_QueryUnload(Cancel As Integer, UnloadMode As Integer)
    If Not mfExit Then
        Cancel = 1
        Me.Hide
    End If
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Shell_NotifyIcon NIM_DELETE, sysIcon
    Cleanup
End Sub

Private Sub mnuExit_Click()
    mfExit = True
    Unload Me
End Sub

Private Sub mnuShow_Click()
    Me.Visible = True
    Me.SetFocus
End Sub

Private Sub mnuStart_Click()
    cmdStartServer_Click
End Sub

Private Sub udUsers_Change()
    Dim AppDesc As DPN_APPLICATION_DESC
    
    If gfStarted Then
        'We need to reset our max users
        AppDesc = dps.GetApplicationDesc(0)
        AppDesc.lMaxPlayers = udUsers.Value
        dps.SetApplicationDesc AppDesc, 0
        sBar.SimpleText = "Server running...  (" & CStr(glNumPlayers) & "/" & txtUsers.Text & " clients connected.)"
        
        'modify our icon text
        sysIcon.sTip = "Server running...  (" & CStr(glNumPlayers) & "/" & txtUsers.Text & " clients connected.)" & vbNullChar
        sysIcon.uFlags = NIF_TIP
        Shell_NotifyIcon NIM_MODIFY, sysIcon
        NotifyEveryoneOfNumPlayers
    End If
End Sub

Private Sub NotifyEveryoneOfNumPlayers()
    Dim oBuf() As Byte
    Dim lMsg As Long, lOffset As Long
    
    'Here we will notify everyone currently in the session about the number of players in the session
    lOffset = NewBuffer(oBuf)
    lMsg = Msg_NumPlayers
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    AddDataToBuffer oBuf, glNumPlayers, LenB(glNumPlayers), lOffset
    AddDataToBuffer oBuf, CLng(udUsers.Value), SIZE_LONG, lOffset
    dps.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_NOLOOPBACK
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
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_CreateGroup(ByVal lGroupID As Long, ByVal lOwnerID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_CreatePlayer(ByVal lPlayerID As Long, fRejectMsg As Boolean)
    On Error Resume Next
    Dim dpPeer As DPN_PLAYER_INFO
    dpPeer = dps.GetClientInfo(lPlayerID)
    If Err Then Exit Sub
    glNumPlayers = glNumPlayers + 1
    sBar.SimpleText = "Server running...  (" & CStr(glNumPlayers) & "/" & txtUsers.Text & " clients connected.)"
    sysIcon.sTip = "Server running...  (" & CStr(glNumPlayers) & "/" & txtUsers.Text & " clients connected.)" & vbNullChar
    sysIcon.uFlags = NIF_TIP
    Shell_NotifyIcon NIM_MODIFY, sysIcon
    'Add this player to the list
    lstUser.AddItem dpPeer.Name & " DPlay ID: 0x" & Hex$(lPlayerID)
    lstUser.ItemData(lstUser.ListCount - 1) = lPlayerID
    NotifyEveryoneOfNumPlayers
End Sub

Private Sub DirectPlay8Event_DestroyGroup(ByVal lGroupID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_DestroyPlayer(ByVal lPlayerID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    Dim lCount As Long
    For lCount = lstUser.ListCount - 1 To 0 Step -1
        If lstUser.ItemData(lCount) = lPlayerID Then 'remove this player from the list
            lstUser.RemoveItem lCount
        End If
    Next
    glNumPlayers = glNumPlayers - 1
    sBar.SimpleText = "Server running...  (" & CStr(glNumPlayers) & "/" & txtUsers.Text & " clients connected.)"
    sysIcon.sTip = "Server running...  (" & CStr(glNumPlayers) & "/" & txtUsers.Text & " clients connected.)" & vbNullChar
    sysIcon.uFlags = NIF_TIP
    Shell_NotifyIcon NIM_MODIFY, sysIcon
    NotifyEveryoneOfNumPlayers
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
    Dim oNewMsg() As Byte, lOffset As Long
    Dim lMsg As Long
    
    'The only message we will receive from our client is one to make faces to everyone
    'else on the server, if there is someone else to make faces at, do it, otherwise let
    'them know
    If glNumPlayers > 1 Then
        lOffset = NewBuffer(oNewMsg)
        lMsg = Msg_SendWave
        AddDataToBuffer oNewMsg, lMsg, LenB(lMsg), lOffset
        AddStringToBuffer oNewMsg, dps.GetClientInfo(dpnotify.idSender).Name, lOffset
        dps.SendTo DPNID_ALL_PLAYERS_GROUP, oNewMsg, 0, DPNSEND_NOLOOPBACK
    Else
        lOffset = NewBuffer(oNewMsg)
        lMsg = Msg_NoOtherPlayers
        AddDataToBuffer oNewMsg, lMsg, LenB(lMsg), lOffset
        dps.SendTo DPNID_ALL_PLAYERS_GROUP, oNewMsg, 0, DPNSEND_NOLOOPBACK
    End If
End Sub

Private Sub DirectPlay8Event_SendComplete(dpnotify As DxVBLibA.DPNMSG_SEND_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_TerminateSession(dpnotify As DxVBLibA.DPNMSG_TERMINATE_SESSION, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub
