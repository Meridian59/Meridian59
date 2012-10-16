VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmClient 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "vbMessenger Service (Not logged in)"
   ClientHeight    =   4740
   ClientLeft      =   150
   ClientTop       =   720
   ClientWidth     =   4170
   Icon            =   "frmClient.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4740
   ScaleWidth      =   4170
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer tmrUpdate 
      Enabled         =   0   'False
      Interval        =   50
      Left            =   4650
      Top             =   2700
   End
   Begin VB.Timer tmrExit 
      Interval        =   50
      Left            =   8100
      Top             =   840
   End
   Begin MSComctlLib.ImageList imlTree 
      Left            =   4680
      Top             =   1140
      _ExtentX        =   1005
      _ExtentY        =   1005
      BackColor       =   -2147483643
      ImageWidth      =   16
      ImageHeight     =   16
      MaskColor       =   12632256
      _Version        =   393216
      BeginProperty Images {2C247F25-8591-11D1-B16A-00C0F0283628} 
         NumListImages   =   2
         BeginProperty ListImage1 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmClient.frx":030A
            Key             =   ""
         EndProperty
         BeginProperty ListImage2 {2C247F27-8591-11D1-B16A-00C0F0283628} 
            Picture         =   "frmClient.frx":0C34
            Key             =   ""
         EndProperty
      EndProperty
   End
   Begin MSComctlLib.TreeView tvwFriends 
      Height          =   4695
      Left            =   0
      TabIndex        =   0
      Top             =   0
      Width           =   4155
      _ExtentX        =   7329
      _ExtentY        =   8281
      _Version        =   393217
      Indentation     =   88
      LabelEdit       =   1
      Style           =   7
      ImageList       =   "imlTree"
      Appearance      =   1
   End
   Begin VB.Menu mnuFile 
      Caption         =   "&File"
      Begin VB.Menu mnuLogon 
         Caption         =   "&Log on..."
         Shortcut        =   ^L
      End
      Begin VB.Menu mnuLogoff 
         Caption         =   "Lo&g Off"
         Shortcut        =   ^X
      End
      Begin VB.Menu mnuSep 
         Caption         =   "-"
      End
      Begin VB.Menu mnuExit 
         Caption         =   "E&xit"
      End
   End
   Begin VB.Menu mnuOptions 
      Caption         =   "&Options"
      Begin VB.Menu mnuAddFriend 
         Caption         =   "&Add Friend..."
         Shortcut        =   ^A
      End
      Begin VB.Menu mnuBlock 
         Caption         =   "&Block User..."
         Shortcut        =   ^B
      End
      Begin VB.Menu mnuSep1 
         Caption         =   "-"
      End
      Begin VB.Menu mnuSendIM 
         Caption         =   "&Send Message..."
         Shortcut        =   ^S
      End
   End
   Begin VB.Menu mnuPop 
      Caption         =   "pop"
      Visible         =   0   'False
      Begin VB.Menu mnuSend 
         Caption         =   "Send Message"
      End
   End
   Begin VB.Menu mnuPopTray 
      Caption         =   "pop2"
      Visible         =   0   'False
      Begin VB.Menu mnuExitTray 
         Caption         =   "E&xit"
      End
   End
End
Attribute VB_Name = "frmClient"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Option Compare Text
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmClient.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Implements DirectPlay8Event
Private Const msAppTitle As String = "vbMessenger Service"

Private mfExit As Boolean
Private oLog As frmLogin
Private oLeafOnline As Node
Private oLeafOffline As Node
Private oMsgWnd() As frmMsgTemplate
Private mfServerExit As Boolean

Private Sub Form_Load()
    'Initialize DirectPlay
    Set gofrmClient = Me
    InitDPlay
    'Lets put an icon in the system tray
    With sysIcon
        .cbSize = LenB(sysIcon)
        .hwnd = Me.hwnd
        .uFlags = NIF_DOALL
        .uCallbackMessage = WM_MOUSEMOVE
        .hIcon = Me.Icon
        .sTip = msAppTitle & " - Not logged in." & vbNullChar
    End With
    Shell_NotifyIcon NIM_ADD, sysIcon
    SetupDefaultTree
    EnableLoggedinUI False
    EnableSendUI False
    Me.Caption = msAppTitle & " - Not logged in."
End Sub

Private Sub Form_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim ShellMsg As Long
    
    ShellMsg = X / Screen.TwipsPerPixelX
    Select Case ShellMsg
    Case WM_LBUTTONDBLCLK
        Me.Visible = True
        Me.SetFocus
    Case WM_RBUTTONUP
        PopupMenu mnuPopTray
    End Select
    
End Sub

Private Sub Form_QueryUnload(Cancel As Integer, UnloadMode As Integer)
    If Not mfExit Then
        Cancel = 1
        Me.Hide
    End If
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Dim lCount As Long
    'Cleanup the DPlay objects
    Cleanup
    'Remove all the forms
    On Error Resume Next
    Dim lNum As Long
    lNum = UBound(oMsgWnd)
    For lCount = 0 To lNum
        Unload oMsgWnd(lCount)
        Set oMsgWnd(lCount) = Nothing
    Next
    Erase oMsgWnd
    'Remove the icon from the system tray
    Shell_NotifyIcon NIM_DELETE, sysIcon
End Sub
Private Sub mnuAddFriend_Click()
    'Let's get the name of the friend we want to add
    Dim sFriend As String
    
    sFriend = InputBox("Please enter the name of the friend you wish to add", "Add Friend")
    If sFriend = vbNullString Then
        'nothing was entered
        MsgBox "You must enter a friends name to add one.", vbOKOnly Or vbInformation, "Nothing entered."
        Exit Sub
    ElseIf sFriend = gsUserName Then
        'Entered our own name
        MsgBox "Everyone wants to be friends with themselves, but in this sample, it's not allowed.", vbOKOnly Or vbInformation, "Don't enter your name."
        Exit Sub
    End If
    'Ok, let's add the friend
    AddFriend sFriend
End Sub

Private Sub mnuBlock_Click()
    'Let's get the name of the friend we want to block
    Dim sFriend As String
    
    sFriend = InputBox("Please enter the name of the user you wish to block", "Block user")
    If sFriend = vbNullString Then
        'nothing was entered
        MsgBox "You must enter a user name to block one.", vbOKOnly Or vbInformation, "Nothing entered."
        Exit Sub
    ElseIf sFriend = gsUserName Then
        'Entered our own name
        MsgBox "Why would you want to block yourself?.", vbOKOnly Or vbInformation, "Don't enter your name."
        Exit Sub
    End If
    'Ok, let's add the friend
    BlockUser sFriend
End Sub

Private Sub mnuExit_Click()
    mfExit = True
    Unload Me
End Sub

Private Sub mnuExitTray_Click()
    mnuExit_Click
End Sub

Private Sub mnuLogoff_Click()
    EnableLoggedinUI False
    gfConnected = False
    gfCreatePlayer = False
    gfLoggedIn = False
    gsUserName = vbNullString
    gsPass = vbNullString
    gsServerName = vbNullString
    Me.Caption = "vbMessenger Service (Not logged in)"
    UpdateText "vbMessenger Service (Not logged in)"
    SetupDefaultTree
    'Initialize DirectPlay
    InitDPlay
End Sub

Private Sub mnuLogon_Click()
    'They want to log on, show the logon screen
    Set oLog = New frmLogin
    oLog.Show , Me
End Sub

Private Sub EnableLoggedinUI(ByVal fEnable As Boolean)

    mnuAddFriend.Enabled = fEnable
    mnuBlock.Enabled = fEnable
    mnuLogoff.Enabled = fEnable
    mnuLogon.Enabled = Not fEnable
End Sub

Private Sub EnableSendUI(ByVal fEnable As Boolean)
    mnuSend.Enabled = fEnable
    mnuSendIM.Enabled = fEnable
End Sub

Private Sub mnuSend_Click()
    mnuSendIM_Click 'Go ahead and send a message
End Sub

Private Sub mnuSendIM_Click()
    Dim frm As frmMsgTemplate
    
    If InStr(tvwFriends.SelectedItem.Text, " ") > 0 Then
        Set frm = GetMsgWindow(Left$(tvwFriends.SelectedItem.Text, InStr(tvwFriends.SelectedItem.Text, " ") - 1))
        frm.UserName = Left$(tvwFriends.SelectedItem.Text, InStr(tvwFriends.SelectedItem.Text, " ") - 1)
    Else
        Set frm = GetMsgWindow(tvwFriends.SelectedItem.Text)
        frm.UserName = tvwFriends.SelectedItem.Text
    End If
    frm.Show
    frm.SetFocus

End Sub

Private Sub tmrExit_Timer()
    If mfServerExit Then 'Gotta quit now
        tmrExit.Enabled = False
        MsgBox "The server has disconnected.  This session will now end.", vbOKOnly Or vbInformation, "Exiting..."
        mfExit = True
        Unload Me
        End
    End If
End Sub

Private Sub tmrUpdate_Timer()
    tmrUpdate.Enabled = False
    If gfCreatePlayer Then
        CreatePlayer 'We're creating a player
    Else
        LogonPlayer 'We're just logging in
    End If
End Sub

Private Sub tvwFriends_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim oNode As Node
    
    If Button = vbRightButton Then 'They right clicked, should we show the menu?
        If tvwFriends.SelectedItem.Parent Is Nothing Then
            Set oNode = oLeafOffline
        Else
            Set oNode = tvwFriends.SelectedItem
        End If
        If (oNode.Children = 0) And oNode <> oLeafOffline Then
            PopupMenu mnuPop
        End If
    End If
End Sub

Private Sub tvwFriends_NodeClick(ByVal Node As MSComctlLib.Node)
    Dim oNode As Node
    
    If Node.Parent Is Nothing Then
        Set oNode = oLeafOffline
    Else
        Set oNode = Node
    End If
    If (oNode.Children = 0) And oNode <> oLeafOffline Then
        EnableSendUI True
    Else
        EnableSendUI False
    End If
End Sub

Private Sub UpdateText(sNewText As String)
    
    'modify our icon text
    sysIcon.sTip = sNewText & vbNullChar
    sysIcon.uFlags = NIF_TIP
    Shell_NotifyIcon NIM_MODIFY, sysIcon

End Sub

Private Function GetMsgWindow(ByVal sUser As String) As frmMsgTemplate

    'Let's check to see if there is a window open
    Dim lCount As Long, lNumWindows As Long
    
    On Error Resume Next
    lNumWindows = UBound(oMsgWnd)
    If Err = 0 Then
        For lCount = 0 To lNumWindows
            If Not (oMsgWnd(lCount) Is Nothing) Then
                If sUser = oMsgWnd(lCount).UserName Then
                    Set GetMsgWindow = oMsgWnd(lCount)
                    Exit Function
                End If
            End If
        Next
        ReDim Preserve oMsgWnd(lNumWindows + 1)
        Set oMsgWnd(lNumWindows + 1) = New frmMsgTemplate
        Set GetMsgWindow = oMsgWnd(lNumWindows + 1)
    Else
        ReDim oMsgWnd(0)
        Set oMsgWnd(0) = New frmMsgTemplate
        Set GetMsgWindow = oMsgWnd(0)
    End If
        
End Function

Private Sub SetupDefaultTree()
    'Clear the tree first
    tvwFriends.Nodes.Clear
    'Let's add the two default icons into our treeview
    Set oLeafOnline = tvwFriends.Nodes.Add(, , "OnlineLeafKey", "Friends online", 1, 1)
    Set oLeafOffline = tvwFriends.Nodes.Add(, , "OfflineLeafKey", "Friends offline", 2, 2)
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
    If dpnotify.hResultCode <> 0 Then
        MsgBox "The server does not exist or is unavailable.", vbOKOnly Or vbInformation, "Unavailable"
    Else
        tmrUpdate.Enabled = True
    End If
    gfConnected = True
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
    'We need to get each message we receive from the server, process it, and respond accordingly
    Dim lMsg As Long, lOffset As Long
    Dim oNewMsg() As Byte, lNewOffSet As Long
    Dim sUsername As String, lNumFriends As Long, lCount As Long
    Dim lNewMsg As Long, oNode As Node
    Dim sChat As String, fChatFrm As frmMsgTemplate
    Dim fFriend As Boolean, fFound As Boolean
    
    With dpnotify
    GetDataFromBuffer .ReceivedData, lMsg, LenB(lMsg), lOffset
    Select Case lMsg 'The client will only receive certain messages.  Handle those.
    Case Msg_LoginSuccess 'Login successfully completed.
        'All we really need to do is get rid of the login screen.
        If Not (oLog Is Nothing) Then
            Unload oLog
            Set oLog = Nothing
        End If
        Unload frmCreate
        gfLoggedIn = True
        EnableLoggedinUI True
        Me.Caption = msAppTitle & " - (" & gsUserName & ")"
        UpdateText msAppTitle & " - (" & gsUserName & ")"
    Case Msg_InvalidPassword 'The server didn't like our password
        'The password they entered was invalid.
        MsgBox "The password you entered was invalid.", vbOKOnly Or vbInformation, "Not valid."
        oLog.cmdLogin.Enabled = True
        oLog.txtPassword = vbNullString
        oLog.txtPassword.SetFocus
    Case Msg_InvalidUser 'We do not exist on this server
        'This user does not exist
        MsgBox "The username you entered does not exist.", vbOKOnly Or vbInformation, "Not valid."
        oLog.cmdLogin.Enabled = True
    Case Msg_UserAlreadyExists 'We can't create this account since the user exists
        'This user already exists
        MsgBox "The username you entered already exists." & vbCrLf & "You must choose a different one.", vbOKOnly Or vbInformation, "Not valid."
        frmCreate.cmdLogin.Enabled = True
    Case Msg_SendClientFriends 'The server is going to send us a list of our current friends
        GetDataFromBuffer .ReceivedData, lNumFriends, LenB(lNumFriends), lOffset
        'Ok, now go through and add each friend to our 'offline' list (The server will notify who is online after this message
        For lCount = 1 To lNumFriends
            GetDataFromBuffer .ReceivedData, fFriend, LenB(fFriend), lOffset
            sUsername = GetStringFromBuffer(.ReceivedData, lOffset)
            'Add this user to our list
            If fFriend Then
                tvwFriends.Nodes.Add oLeafOffline, tvwChild, sUsername, sUsername, 2, 2
            Else
                tvwFriends.Nodes.Add oLeafOffline, tvwChild, sUsername, sUsername & " (BLOCKED)", 2, 2
            End If
        Next
        oLeafOffline.Expanded = True
        oLeafOnline.Expanded = True
    Case Msg_FriendAdded
        sUsername = GetStringFromBuffer(.ReceivedData, lOffset)
        fFound = False
        For Each oNode In tvwFriends.Nodes
            If oNode.Key = sUsername Then
                oNode.Text = sUsername
                fFound = True
            End If
        Next
        If Not fFound Then tvwFriends.Nodes.Add oLeafOffline, tvwChild, sUsername, sUsername, 2, 2
        'Friend added successfully
        MsgBox sUsername & " added successfully to your friends list.", vbOKOnly Or vbInformation, "Added."
    
    Case Msg_FriendBlocked
        sUsername = GetStringFromBuffer(.ReceivedData, lOffset)
        fFound = False
        For Each oNode In tvwFriends.Nodes
            If oNode.Key = sUsername Then
                oNode.Text = sUsername & " (BLOCKED)"
                fFound = True
            End If
        Next
        If Not fFound Then tvwFriends.Nodes.Add oLeafOffline, tvwChild, sUsername, sUsername & " (BLOCKED)", 2, 2
        'Friend blocked successfully
        MsgBox sUsername & " added successfully to your blocked list.", vbOKOnly Or vbInformation, "Added."
            
    Case Msg_FriendDoesNotExist
        'Friend doesn't exist
        MsgBox "You cannot add this friend, since they do not exist.", vbOKOnly Or vbInformation, "Unknown."
    
    Case Msg_BlockUserDoesNotExist
        'Friend doesn't exist
        MsgBox "You cannot block this user, since they do not exist.", vbOKOnly Or vbInformation, "Unknown."
        
    Case Msg_FriendLogon
        'We need to go through each of the current nodes and see if this is that friend
        sUsername = GetStringFromBuffer(.ReceivedData, lOffset)
        For Each oNode In tvwFriends.Nodes
            If oNode.Key = sUsername And oNode.Children = 0 Then
                oNode.Image = 1: oNode.SelectedImage = 1
                Set oNode.Parent = oLeafOnline
            End If
        Next
    Case Msg_FriendLogoff
        'We need to go through each of the current nodes and see if this is that friend
        sUsername = GetStringFromBuffer(.ReceivedData, lOffset)
        For Each oNode In tvwFriends.Nodes
            If oNode.Key = sUsername And oNode.Children = 0 Then
                oNode.Image = 2: oNode.SelectedImage = 2
                Set oNode.Parent = oLeafOffline
            End If
        Next
    Case Msg_ReceiveMessage
        'We need to go through each of the current forms and see if this is friend is loaded
        sUsername = GetStringFromBuffer(.ReceivedData, lOffset)
        sChat = GetStringFromBuffer(.ReceivedData, lOffset)
        Set fChatFrm = GetMsgWindow(sUsername)
        fChatFrm.UserName = sUsername
        fChatFrm.Show
        fChatFrm.SetFocus
        fChatFrm.AddChatMessage sChat
    
    Case Msg_UserBlocked
        'This user has blocked me
        sUsername = GetStringFromBuffer(.ReceivedData, lOffset)
        Set fChatFrm = GetMsgWindow(sUsername)
        fChatFrm.UserName = sUsername
        fChatFrm.Show
        fChatFrm.SetFocus
        fChatFrm.AddChatMessage "Your message to " & sUsername & " could not be delivered since they have blocked you.", , True
    Case Msg_UserUnavailable
        'This user is no longer available
        sUsername = GetStringFromBuffer(.ReceivedData, lOffset)
        sChat = GetStringFromBuffer(.ReceivedData, lOffset)
        Set fChatFrm = GetMsgWindow(sUsername)
        fChatFrm.UserName = sUsername
        fChatFrm.Show
        fChatFrm.SetFocus
        fChatFrm.AddChatMessage "Your message: " & vbCrLf & sChat & vbCrLf & "to " & sUsername & " could not be delivered since they are no longer available.", , True
    End Select
    End With
End Sub

Private Sub DirectPlay8Event_SendComplete(dpnotify As DxVBLibA.DPNMSG_SEND_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_TerminateSession(dpnotify As DxVBLibA.DPNMSG_TERMINATE_SESSION, fRejectMsg As Boolean)
    'We're no longer connected for some reason.
    mfServerExit = True
End Sub

