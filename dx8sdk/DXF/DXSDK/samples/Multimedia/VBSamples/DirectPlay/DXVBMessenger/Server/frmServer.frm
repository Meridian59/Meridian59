VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmServer 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "vbMessenger Server"
   ClientHeight    =   4515
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   3645
   Icon            =   "frmServer.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4515
   ScaleWidth      =   3645
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer tmrSaveXML 
      Interval        =   60000
      Left            =   3660
      Top             =   1410
   End
   Begin VB.Timer tmrLogon 
      Interval        =   50
      Left            =   3660
      Top             =   960
   End
   Begin VB.Timer tmrLogoff 
      Interval        =   50
      Left            =   3660
      Top             =   480
   End
   Begin VB.ListBox lstUsers 
      Height          =   3765
      Left            =   60
      TabIndex        =   1
      Top             =   360
      Width           =   3495
   End
   Begin MSComctlLib.StatusBar sBar 
      Align           =   2  'Align Bottom
      Height          =   375
      Left            =   0
      TabIndex        =   0
      Top             =   4140
      Width           =   3645
      _ExtentX        =   6429
      _ExtentY        =   661
      Style           =   1
      SimpleText      =   " "
      _Version        =   393216
      BeginProperty Panels {8E3867A5-8586-11D1-B16A-00C0F0283628} 
         NumPanels       =   1
         BeginProperty Panel1 {8E3867AB-8586-11D1-B16A-00C0F0283628} 
         EndProperty
      EndProperty
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "Users currently in this session"
      Height          =   255
      Left            =   60
      TabIndex        =   2
      Top             =   60
      Width           =   3495
   End
   Begin VB.Menu mnuPop 
      Caption         =   "PopUp"
      Visible         =   0   'False
      Begin VB.Menu mnuShow 
         Caption         =   "Show"
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
Private mfLogoffTimer As Boolean
Private msLogoffName As String
Private mfLogonTimer As Boolean
Private msLogonName As String

Private Sub StartServer()
    Dim appdesc As DPN_APPLICATION_DESC
    
    'Now set up the app description
    With appdesc
        .guidApplication = AppGuid
        .lMaxPlayers = 1000 'This seems like a nice round number
        .SessionName = "vbMessengerServer"
        .lFlags = DPNSESSION_CLIENT_SERVER Or DPNSESSION_NODPNSVR 'We must pass the client server flags if we are a server
    End With
    
    'Now set up our address value
    dpa.SetSP DP8SP_TCPIP
    dpa.AddComponentLong DPN_KEY_PORT, glDefaultPort 'Use a specific port
    
    'Now start the server
    dps.Host appdesc, dpa
    UpdateText "Server running...  (" & CStr(glNumPlayers) & "/1000 clients connected.)"
    
End Sub

Private Sub Form_Load()
    
    dps.RegisterMessageHandler Me
    'Lets put an icon in the system tray
    With sysIcon
        .cbSize = LenB(sysIcon)
        .hwnd = Me.hwnd
        .uFlags = NIF_DOALL
        .uCallbackMessage = WM_MOUSEMOVE
        .hIcon = Me.Icon
        .sTip = "Server running...  (" & CStr(glNumPlayers) & "/1000 clients connected.)" & vbNullChar
    End With
    Shell_NotifyIcon NIM_ADD, sysIcon
    'Open the database
    OpenClientDatabase
    'Start the server
    StartServer

End Sub

Private Sub Form_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim ShellMsg As Long
    
    ShellMsg = X / Screen.TwipsPerPixelX
    Select Case ShellMsg
    Case WM_LBUTTONDBLCLK
        mnuShow_Click
    Case WM_RBUTTONUP
        'Show the menu
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
    'Remove the icon from the system tray
    Shell_NotifyIcon NIM_DELETE, sysIcon
    'Close the database
    CloseDownDB
    'Cleanup the dplay objects
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

Private Sub tmrSaveXML_Timer()
    Static lCount As Long
    
    'Every 5 minutes we will save the xml
    lCount = lCount + 1
    If lCount >= 5 Then
        lCount = 0
        SaveXMLStructure
    End If
End Sub

Private Sub UpdateText(sNewText As String)
    
    sBar.SimpleText = sNewText
    
    'modify our icon text
    sysIcon.sTip = sNewText & vbNullChar
    sysIcon.uFlags = NIF_TIP
    Shell_NotifyIcon NIM_MODIFY, sysIcon

End Sub

Private Sub tmrLogoff_Timer()
    'Log this user off
    If mfLogoffTimer Then
        NotifyFriends msLogoffName, Msg_FriendLogoff
    End If
    msLogoffName = vbNullString
    mfLogoffTimer = False
End Sub

Private Sub tmrLogon_Timer()
    If mfLogonTimer Then
        mfLogonTimer = False
        NotifyFriends msLogonName, Msg_FriendLogon 'Tell everyone who has me marked as a friend that I'm online
        GetFriendsOfMineOnline msLogonName 'Find out if any of my friends are online and tell me
    End If
    msLogonName = vbNullString
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
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_DestroyGroup(ByVal lGroupID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_DestroyPlayer(ByVal lPlayerID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    Dim lCount As Long
    
    On Local Error GoTo ErrOut 'So we don't get an InvalidPlayer error when checking on the host
    
    'Update the DB to show a logoff
    UpdateDBToShowLogoff lPlayerID
    'Remove this player from our listbox
    For lCount = lstUsers.ListCount - 1 To 0 Step -1
        If lstUsers.ItemData(lCount) = lPlayerID Then
            mfLogoffTimer = True
            msLogoffName = lstUsers.List(lCount)
            glNumPlayers = glNumPlayers - 1
            lstUsers.RemoveItem lCount
            Exit For
        End If
    Next

ErrOut:
    UpdateText "Server running...  (" & CStr(glNumPlayers) & "/1000 clients connected.)"
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
    'We need to get each message we receive from a client, process it, and respond accordingly
    Dim lMsg As Long, lOffset As Long
    Dim oNewMsg() As Byte, lNewOffSet As Long
    Dim sUserName As String, sPass As String
    Dim lNewMsg As Long, fLoggedin As Boolean
    Dim sChatMsg As String, sFromMsg As String
    
    With dpnotify
    GetDataFromBuffer .ReceivedData, lMsg, LenB(lMsg), lOffset
    Select Case lMsg 'The server will only receive certain messages.  Handle those.
    Case Msg_AddFriend 'They want to add a friend to their list
        sUserName = GetStringFromBuffer(.ReceivedData, lOffset)
        If Not DoesUserExist(sUserName) Then
            'This user does not exist, notify the person that they cannot be added
            lNewMsg = Msg_FriendDoesNotExist
            lNewOffSet = NewBuffer(oNewMsg)
            AddDataToBuffer oNewMsg, lNewMsg, LenB(lNewMsg), lNewOffSet
            dps.SendTo .idSender, oNewMsg, 0, 0
        Else
            'Great, add this user to our friend list
            fLoggedin = AddFriend(.idSender, sUserName, True)
            lNewMsg = Msg_FriendAdded
            lNewOffSet = NewBuffer(oNewMsg)
            AddDataToBuffer oNewMsg, lNewMsg, LenB(lNewMsg), lNewOffSet
            AddStringToBuffer oNewMsg, sUserName, lNewOffSet
            dps.SendTo .idSender, oNewMsg, 0, DPNSEND_SYNC
            If fLoggedin Then
                lNewMsg = Msg_FriendLogon
                lNewOffSet = NewBuffer(oNewMsg)
                AddDataToBuffer oNewMsg, lNewMsg, LenB(lMsg), lNewOffSet
                AddStringToBuffer oNewMsg, sUserName, lNewOffSet
                dps.SendTo .idSender, oNewMsg, 0, 0
            End If
        End If
    Case Msg_BlockFriend 'They want to block a friend from their list
        sUserName = GetStringFromBuffer(.ReceivedData, lOffset)
        If Not DoesUserExist(sUserName) Then
            'This user does not exist, notify the person that they cannot be blocked
            lNewMsg = Msg_BlockUserDoesNotExist
            lNewOffSet = NewBuffer(oNewMsg)
            AddDataToBuffer oNewMsg, lNewMsg, LenB(lNewMsg), lNewOffSet
            dps.SendTo .idSender, oNewMsg, 0, 0
        Else
            'Great, block this user in our friend list
            AddFriend .idSender, sUserName, False
            lNewMsg = Msg_FriendBlocked
            lNewOffSet = NewBuffer(oNewMsg)
            AddDataToBuffer oNewMsg, lNewMsg, LenB(lNewMsg), lNewOffSet
            AddStringToBuffer oNewMsg, sUserName, lNewOffSet
            dps.SendTo .idSender, oNewMsg, 0, 0
        End If
    Case Msg_CreateNewAccount 'They want to create a new account
        sUserName = GetStringFromBuffer(.ReceivedData, lOffset)
        sPass = GetStringFromBuffer(.ReceivedData, lOffset)
        If DoesUserExist(sUserName) Then
            'This user already exists, inform the person so they can try a new name
            lNewMsg = Msg_UserAlreadyExists
            lNewOffSet = NewBuffer(oNewMsg)
            AddDataToBuffer oNewMsg, lNewMsg, LenB(lNewMsg), lNewOffSet
            dps.SendTo .idSender, oNewMsg, 0, 0
        Else
            'Great, this username doesn't exist.  Now lets add this user
            AddUser sUserName, sPass, .idSender
            'We don't need to inform anyone we are logged on, because
            'no one could have us listed as a friend yet
            
            'Notify the user they logged on successfully
            lNewMsg = Msg_LoginSuccess
            lNewOffSet = NewBuffer(oNewMsg)
            AddDataToBuffer oNewMsg, lNewMsg, LenB(lNewMsg), lNewOffSet
            dps.SendTo .idSender, oNewMsg, 0, 0
            
            'Increment our user count
            glNumPlayers = glNumPlayers + 1
            'Add this user to our list of users currently online
            lstUsers.AddItem sUserName & " 0x" & Hex$(.idSender)
            lstUsers.ItemData(lstUsers.ListCount - 1) = .idSender
            UpdateText "Server running...  (" & CStr(glNumPlayers) & "/1000 clients connected.)"
        End If
    
    Case Msg_Login 'They have requested a login, check name/password
        sUserName = GetStringFromBuffer(.ReceivedData, lOffset)
        sPass = GetStringFromBuffer(.ReceivedData, lOffset)
        Select Case LogonUser(sUserName, sPass) 'Try to log on the user
        Case LogonSuccess 'Great, they logged on
            UpdateDBToShowLogon sUserName, dpnotify.idSender 'Update the DB to show I'm online
            'Notify the user they logged on successfully
            lNewMsg = Msg_LoginSuccess
            lNewOffSet = NewBuffer(oNewMsg)
            AddDataToBuffer oNewMsg, lNewMsg, LenB(lNewMsg), lNewOffSet
            dps.SendTo .idSender, oNewMsg, 0, 0
            mfLogonTimer = True
            msLogonName = sUserName
            'Increment our user count
            glNumPlayers = glNumPlayers + 1
            'Add this user to our list of users currently online
            lstUsers.AddItem sUserName & " 0x" & Hex$(.idSender)
            lstUsers.ItemData(lstUsers.ListCount - 1) = .idSender
            UpdateText "Server running...  (" & CStr(glNumPlayers) & "/1000 clients connected.)"
            
        Case InvalidPassword 'Let the user know that they didn't type in the right password
            'Notify the user they sent the wrong password
            lNewMsg = Msg_InvalidPassword
            lNewOffSet = NewBuffer(oNewMsg)
            AddDataToBuffer oNewMsg, lNewMsg, LenB(lNewMsg), lNewOffSet
            dps.SendTo .idSender, oNewMsg, 0, 0
        Case AccountDoesNotExist 'Let the user know this account isn't in the DB
            'Notify the user that this account doesn't exist
            lNewMsg = Msg_InvalidUser
            lNewOffSet = NewBuffer(oNewMsg)
            AddDataToBuffer oNewMsg, lNewMsg, LenB(lNewMsg), lNewOffSet
            dps.SendTo .idSender, oNewMsg, 0, 0
        End Select
    Case Msg_SendMessage 'They are trying to send a message to someone
        sUserName = GetStringFromBuffer(.ReceivedData, lOffset)
        sFromMsg = GetStringFromBuffer(.ReceivedData, lOffset)
        sChatMsg = GetStringFromBuffer(.ReceivedData, lOffset)
        SendMessage sUserName, sFromMsg, sChatMsg
    End Select
    End With
End Sub

Private Sub DirectPlay8Event_SendComplete(dpnotify As DxVBLibA.DPNMSG_SEND_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_TerminateSession(dpnotify As DxVBLibA.DPNMSG_TERMINATE_SESSION, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

