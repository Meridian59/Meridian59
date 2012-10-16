VERSION 5.00
Begin VB.Form DPlayConnect 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Multiplayer connect"
   ClientHeight    =   3330
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   6345
   Icon            =   "DplayCon.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3330
   ScaleWidth      =   6345
   StartUpPosition =   3  'Windows Default
   Begin VB.Frame fraWiz 
      BorderStyle     =   0  'None
      Height          =   3195
      Index           =   1
      Left            =   120
      TabIndex        =   5
      Top             =   60
      Width           =   6195
      Begin VB.CommandButton cmdCancelGame 
         Caption         =   "Cancel"
         Height          =   315
         Left            =   5040
         TabIndex        =   12
         Top             =   2880
         Width           =   1095
      End
      Begin VB.CommandButton cmdRefresh 
         Caption         =   "S&tart Search"
         Height          =   315
         Left            =   5040
         TabIndex        =   11
         Top             =   60
         Width           =   1095
      End
      Begin VB.CommandButton cmdJoin 
         Caption         =   "&Join"
         Height          =   315
         Left            =   60
         TabIndex        =   8
         Top             =   2880
         Width           =   1095
      End
      Begin VB.CommandButton cmdCreate 
         Caption         =   "&Create"
         Height          =   315
         Left            =   1200
         TabIndex        =   7
         Top             =   2880
         Width           =   1095
      End
      Begin VB.ListBox lstGames 
         Height          =   2400
         Left            =   60
         TabIndex        =   6
         Top             =   420
         Width           =   6075
      End
      Begin VB.Label Label3 
         BackStyle       =   0  'Transparent
         Caption         =   "Click 'Start Search' to look for a session, or create one of your own."
         Height          =   255
         Left            =   60
         TabIndex        =   25
         Top             =   120
         Width           =   4935
      End
   End
   Begin VB.Frame fraWiz 
      BorderStyle     =   0  'None
      Height          =   3195
      Index           =   0
      Left            =   120
      TabIndex        =   2
      Top             =   60
      Width           =   6195
      Begin VB.CommandButton cmdCancel 
         Cancel          =   -1  'True
         Caption         =   "Cancel"
         Height          =   315
         Left            =   3900
         TabIndex        =   10
         Top             =   2760
         Width           =   1095
      End
      Begin VB.CommandButton cmdOk 
         Caption         =   "OK"
         Default         =   -1  'True
         Height          =   315
         Left            =   5040
         TabIndex        =   9
         Top             =   2760
         Width           =   1095
      End
      Begin VB.ListBox lstSP 
         Height          =   1815
         Left            =   60
         TabIndex        =   4
         Top             =   900
         Width           =   6075
      End
      Begin VB.TextBox txtUserName 
         Height          =   285
         Left            =   60
         TabIndex        =   1
         Top             =   300
         Width           =   6075
      End
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   "Select your ser&vice provider:"
         Height          =   255
         Index           =   1
         Left            =   60
         TabIndex        =   3
         Top             =   660
         Width           =   3915
      End
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   "&Player Name:"
         Height          =   255
         Index           =   0
         Left            =   60
         TabIndex        =   0
         Top             =   60
         Width           =   3915
      End
   End
   Begin VB.Frame fraWiz 
      BorderStyle     =   0  'None
      Height          =   3195
      Index           =   2
      Left            =   120
      TabIndex        =   13
      Top             =   60
      Width           =   6195
      Begin VB.Frame Frame1 
         Caption         =   "Extra Session Options"
         Height          =   1995
         Left            =   60
         TabIndex        =   20
         Top             =   660
         Width           =   5955
         Begin VB.CheckBox chkMigrate 
            Alignment       =   1  'Right Justify
            Caption         =   "Migrate &Host"
            Height          =   255
            Left            =   30
            TabIndex        =   21
            Top             =   600
            Width           =   2895
         End
         Begin VB.TextBox txtUsers 
            Height          =   285
            Left            =   2490
            TabIndex        =   19
            Top             =   240
            Width           =   435
         End
         Begin VB.Label Label1 
            BackStyle       =   0  'Transparent
            Caption         =   "N&umber of players in this session:"
            Height          =   255
            Index           =   2
            Left            =   60
            TabIndex        =   18
            Top             =   285
            Width           =   2355
         End
      End
      Begin VB.TextBox txtGameName 
         Height          =   285
         Left            =   60
         TabIndex        =   17
         Top             =   300
         Width           =   6075
      End
      Begin VB.CommandButton cmdOkCreate 
         Caption         =   "OK"
         Height          =   315
         Left            =   5040
         TabIndex        =   15
         Top             =   2760
         Width           =   1095
      End
      Begin VB.CommandButton cmdCancelCreate 
         Caption         =   "Cancel"
         Height          =   315
         Left            =   3900
         TabIndex        =   14
         Top             =   2760
         Width           =   1095
      End
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   "Please enter the session &name"
         Height          =   255
         Index           =   3
         Left            =   60
         TabIndex        =   16
         Top             =   60
         Width           =   2235
      End
   End
   Begin VB.Timer tmrExpire 
      Interval        =   500
      Left            =   7500
      Top             =   780
   End
   Begin VB.Frame fraWiz 
      BorderStyle     =   0  'None
      Height          =   3195
      Index           =   3
      Left            =   60
      TabIndex        =   22
      Top             =   60
      Width           =   6195
      Begin VB.CommandButton cmdCancelLobby 
         Caption         =   "Cancel"
         Height          =   315
         Left            =   2340
         TabIndex        =   24
         Top             =   1500
         Width           =   1455
      End
      Begin VB.Label Label2 
         BackStyle       =   0  'Transparent
         Caption         =   "Waiting for lobby connection"
         Height          =   375
         Left            =   2100
         TabIndex        =   23
         Top             =   1140
         Width           =   2115
      End
   End
End
Attribute VB_Name = "DPlayConnect"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       DPlayCon.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'Sleep declare
Private Declare Sub Sleep Lib "kernel32" (ByVal dwMilliseconds As Long)
'GetTickCount declare
Private Declare Function GetTickCount Lib "kernel32" () As Long
'Declares for closing the form without waiting
Private Declare Function PostMessage Lib "user32" Alias "PostMessageA" (ByVal hwnd As Long, ByVal wMsg As Long, ByVal wParam As Long, ByVal lParam As Long) As Long
Private Const WM_CLOSE = &H10

'Host expire threshold constant
Private Const HOST_EXPIRE_THRESHHOLD As Long = 2000

Private Type HostFound
    AppDesc As DPN_APPLICATION_DESC
    AddressHost As String
    AddressDevice As String
    TimeLastFound As Long
End Type

Private Enum WizPanes
    PickProtocol
    CreateJoinGame
    CreateNewGame
    WaitForLobby
End Enum

Private Enum SearchingButton
    StartSearch
    StopSearch
End Enum

'Internal DirectX variables
Private moDPP As DirectPlay8Peer
Private moDPC As DirectPlay8Client
Private moDPA As DirectPlay8Address
Private moDX As DirectX8
Private moCallback As DirectPlay8Event
Private moDPLA As DirectPlay8LobbiedApplication

'App specific vars
Private msGuid As String
Private sUser As String
Private mlSearch As SearchingButton
Private sGameName As String
Private mlMax As Long
Private mlNumPlayers As Long
Private mfComplete As Boolean
Private mfHost As Boolean
Private mlEnumAsync As Long
Private mfGotEvent As Boolean
Private mfDoneWiz As Boolean

Private mlLobbyClientID As Long
Private mfCanUnload As Boolean

'We need to keep track of the hosts we get
Private moHosts() As HostFound
Private mlHostCount As Long
'Declaration for our API
Private Declare Function GetUserName Lib "advapi32.dll" Alias "GetUserNameA" (ByVal lpBuffer As String, nSize As Long) As Long
Private mfDoneEnum As Boolean
Private mfConnectComplete As Boolean

'We need to implement the Event model for DirectPlay so we can receive callbacks
Implements DirectPlay8Event
Implements DirectPlay8LobbyEvent

Private Function StartWizard(oDX As DirectX8, sGuid As String, ByVal lMaxPlayers As Long, Optional oCallback As DirectPlay8Event = Nothing, Optional ByVal fAllowMigrateHost As Boolean = True) As Boolean
    Dim lCount As Long, lIndex As Long
    Dim dpn As DPN_SERVICE_PROVIDER_INFO
    'Now we can start our connection
    
    mfCanUnload = False
    mlSearch = StartSearch
    mlHostCount = -1
    
    'First we need to keep track of our Peer Object, and app guid
    Set moDX = oDX
    Set moCallback = oCallback
    msGuid = sGuid
    mlMax = lMaxPlayers
    
    lIndex = GetSetting("VBDirectPlay", "Defaults", "SPListIndex", -1)
    If Not (moDPP Is Nothing) Then
        moDPP.RegisterMessageHandler Me
        'First load our list of Service Providers into our box
        For lCount = 1 To moDPP.GetCountServiceProviders
            dpn = moDPP.GetServiceProvider(lCount)
            lstSP.AddItem dpn.Name
            'Pick the TCP/IP connection by default
            If InStr(dpn.Name, "TCP") Then lstSP.ListIndex = lstSP.ListCount - 1
        Next
        If lstSP.ListIndex < 0 Then lstSP.ListIndex = 0
    ElseIf Not (moDPC Is Nothing) Then
        moDPC.RegisterMessageHandler Me
        'First load our list of Service Providers into our box
        For lCount = 1 To moDPC.GetCountServiceProviders
            dpn = moDPC.GetServiceProvider(lCount)
            lstSP.AddItem dpn.Name
            'Pick the TCP/IP connection by default
            If InStr(dpn.Name, "TCP") Then lstSP.ListIndex = lstSP.ListCount - 1
        Next
        If lstSP.ListIndex < 0 Then lstSP.ListIndex = 0
    End If
    If lIndex <> -1 And lIndex < lstSP.ListCount Then lstSP.ListIndex = lIndex
    lstSP.AddItem "Wait for Lobby Connection..."
    'Load the default Username for VBDirectPlay samples
    sUser = GetSetting("VBDirectPlay", "Defaults", "UserName", vbNullString)
    If sUser = vbNullString Then
        'If there is not a default username, then pick the currently
        'logged on username
        sUser = Space$(255)
        GetUserName sUser, 255
        sUser = Left$(sUser, InStr(sUser, Chr$(0)) - 1)
    End If
    chkMigrate.Visible = fAllowMigrateHost
    txtUserName.Text = sUser
    ShowPane PickProtocol
    Set moDPLA = dx.DirectPlayLobbiedApplicationCreate
    'Init the register handler here
    moDPLA.RegisterMessageHandler Me
    'Register this app (in case it isn't registered already)
    RegisterThisApp sGuid
    'Show this screen
    Me.Show vbModeless
    'We have this loop here rather than just displaying the form as a modal
    'dialog if we did just display the form as modal, it would not get a
    'button in the toolbar, since it would have a parent window that wasn't visible
    
    'By displaying the window modeless, and going into a loop we get to have our
    'icon on the taskbar, and keep the main form waiting until we are done in this form.
    Do While Not mfDoneWiz
        DoSleep 5 'Give other threads cpu time
    Loop
    'Get rid of the lobby interface if it isn't necessary
    If mlLobbyClientID <> 0 Then
        If Not (moDPP Is Nothing) Then
            moDPP.RegisterLobby mlLobbyClientID, moDPLA, DPNLOBBY_UNREGISTER
        ElseIf Not (moDPC Is Nothing) Then
            moDPC.RegisterLobby mlLobbyClientID, moDPLA, DPNLOBBY_UNREGISTER
        End If
    End If
    If Not (moDPLA Is Nothing) Then moDPLA.Close
    Set moDPLA = Nothing
    'Now we can return our success (or failure)
    StartWizard = mfComplete
End Function

Public Function StartClientConnectWizard(oDX As DirectX8, oDPC As DirectPlay8Client, sGuid As String, ByVal lMaxPlayers As Long, Optional oCallback As DirectPlay8Event = Nothing, Optional ByVal fAllowMigrateHost As Boolean = True) As Boolean
    Set moDPP = Nothing
    Set moDPC = oDPC
    cmdCreate.Visible = False
    StartClientConnectWizard = StartWizard(oDX, sGuid, lMaxPlayers, oCallback, fAllowMigrateHost)
End Function

Public Function StartConnectWizard(oDX As DirectX8, oDPP As DirectPlay8Peer, sGuid As String, ByVal lMaxPlayers As Long, Optional oCallback As DirectPlay8Event = Nothing, Optional ByVal fAllowMigrateHost As Boolean = True) As Boolean
    Set moDPC = Nothing
    Set moDPP = oDPP
    cmdCreate.Visible = True
    StartConnectWizard = StartWizard(oDX, sGuid, lMaxPlayers, oCallback, fAllowMigrateHost)
End Function

Public Sub CloseForm(oForm As Form)
    'Anytime we need to close a form from within a DirectPlay callback
    'we need to use this function.  The reason is that DirectPlay uses multiple
    'threads to spawn all of it's messages back to the application.  However
    'it cannot close down until all of it's threads have returned.
    'If we attempt to simply call Unload Me in the callback, we will run into
    'a deadlock instance, since the callback will be running on the DirectPlay
    'thread waiting for the unload to finish, and the unload will be waiting
    'for the DirectPlay thread to finish.
    
    'PostMessage puts the message on the queue for our form and returns immediately
    'allowing the thread to finish
    PostMessage oForm.hwnd, WM_CLOSE, 0, 0
End Sub

Public Sub DoSleep(Optional ByVal lMilliSec As Long = 0)
    'The DoSleep function allows other threads to have a time slice
    'and still keeps the main VB thread alive (since DPlay callbacks
    'run on separate threads outside of VB).
    Sleep lMilliSec
    DoEvents
End Sub

Private Sub cmdCancel_Click()
    Unload Me
End Sub

Private Sub cmdCancelCreate_Click()
    'If they click cancel here, just go back to the last step
    ShowPane CreateJoinGame
End Sub

Private Sub cmdCancelGame_Click()
    'If they click cancel here, just go back to the first step
    ShowPane PickProtocol
End Sub

Private Sub cmdCancelLobby_Click()
    'Don't wait any more.
    moDPLA.SetAppAvailable False, 0
    ShowPane PickProtocol
End Sub

Private Sub cmdCreate_Click()
    Dim sDefault As String
    
    'Here we should get our default
    sDefault = GetSetting("VBDirectPlay", "Defaults", "GameName", vbNullString)
    txtGameName.Text = sDefault
    txtUsers.Text = CStr(mlMax)
    chkMigrate.Value = Val(GetSetting("VBDirectPlay", "Defaults", "HostMigrate"))
    'Show the create game screen
    ShowPane CreateNewGame
End Sub

Private Sub cmdJoin_Click()
    Dim HostAddr As DirectPlay8Address
    Dim DeviceAddr As DirectPlay8Address
    
    Dim dpApp As DPN_APPLICATION_DESC
    
    'You must select a game before you try to join one
    If lstGames.ListIndex < 0 Then
        MsgBox "You must first select a game from the list to join.", vbOKOnly Or vbInformation, "Select game."
        Exit Sub
    End If
    
    'Lets join the game
    Dim pInfo As DPN_PLAYER_INFO
    'Set up my peer info
    pInfo.Name = sUser
    pInfo.lInfoFlags = DPNINFO_NAME
    If Not (moDPP Is Nothing) Then
        moDPP.SetPeerInfo pInfo, DPNOP_SYNC
    ElseIf Not (moDPC Is Nothing) Then
        moDPC.SetClientInfo pInfo, DPNOP_SYNC
    End If
    mfDoneEnum = True

    With moHosts(lstGames.ItemData(lstGames.ListIndex)).AppDesc
        dpApp.guidApplication = .guidApplication
        dpApp.guidInstance = .guidInstance
        mlNumPlayers = .lMaxPlayers
    End With
    
    mfGotEvent = False
    mfConnectComplete = False
    'Lets get our host address
    If moHosts(lstGames.ItemData(lstGames.ListIndex)).AddressHost <> vbNullString Then
        Set HostAddr = moDX.DirectPlayAddressCreate
        HostAddr.BuildFromURL moHosts(lstGames.ItemData(lstGames.ListIndex)).AddressHost
    Else
        Set HostAddr = moDPA
    End If
    If moHosts(lstGames.ItemData(lstGames.ListIndex)).AddressDevice <> vbNullString Then
        Set DeviceAddr = moDX.DirectPlayAddressCreate
        DeviceAddr.BuildFromURL moHosts(lstGames.ItemData(lstGames.ListIndex)).AddressDevice
    Else
        Set DeviceAddr = moDPA
    End If
    If Not (moDPP Is Nothing) Then
        'Now we can join the selected session
        moDPP.Connect dpApp, HostAddr, DeviceAddr, DPNCONNECT_OKTOQUERYFORADDRESSING, ByVal 0&, 0
    ElseIf Not (moDPC Is Nothing) Then
        'Now we can join the selected session
        moDPC.Connect dpApp, HostAddr, DeviceAddr, DPNCONNECT_OKTOQUERYFORADDRESSING, ByVal 0&, 0
    End If
    Do While Not mfGotEvent 'Let's wait for our connectcomplete event
        DoSleep 5 'Give other threads cpu time
    Loop
    If mfConnectComplete Then
        'We've joined our game
        mfComplete = True
        mfHost = False
        'Clean up our address
        Set HostAddr = Nothing
        Set DeviceAddr = Nothing
        Set moDPA = Nothing
        Unload Me
    End If
End Sub

Private Sub cmdOk_Click()
    'They must specify a user name before we continue on to the next step
    If txtUserName.Text = vbNullString Then
        MsgBox "Please enter a username before going on.", vbOKOnly Or vbInformation, "No username"
        Exit Sub
    End If
    sUser = txtUserName.Text
    'Save the username
    SaveSetting "VBDirectPlay", "Defaults", "UserName", sUser
    SaveSetting "VBDirectPlay", "Defaults", "SPListIndex", lstSP.ListIndex
    
    If lstSP.ListIndex = lstSP.ListCount - 1 Then 'We want to wait for a lobby connection
        moDPLA.SetAppAvailable True, 0
        ShowPane WaitForLobby
    Else
        'Set up the address
        Set moDPA = moDX.DirectPlayAddressCreate
        If Not (moDPP Is Nothing) Then
            moDPA.SetSP moDPP.GetServiceProvider(lstSP.ListIndex + 1).Guid
        ElseIf Not (moDPC Is Nothing) Then
            moDPA.SetSP moDPC.GetServiceProvider(lstSP.ListIndex + 1).Guid
        End If
        'Switch to the next screen
        ShowPane CreateJoinGame
    End If
End Sub

Private Sub cmdOkCreate_Click()
    sGameName = txtGameName.Text
    If sGameName = vbNullString Then
        MsgBox "You must enter a session name to create a session.", vbOKOnly Or vbInformation, "No name."
        Exit Sub 'No need to continue
    End If
    If Val(txtUsers.Text) < 1 Then
        MsgBox "You must enter a number of max players.", vbOKOnly Or vbInformation, "No max players."
        Exit Sub 'No need to continue
    End If
    If Val(txtUsers.Text) > mlMax Then
        MsgBox "The number of maximum players you specified exceeds the number of maximum players allowed in this session." & vbCrLf & "Please lower the number of your maximum players.", vbOKOnly Or vbInformation, "Too many players."
        Exit Sub 'No need to continue
    End If
    mfHost = True
    SaveSetting "VBDirectPlay", "Defaults", "GameName", sGameName
    SaveSetting "VBDirectPlay", "Defaults", "HostMigrate", CStr(chkMigrate.Value)
    If Not chkMigrate.Visible Then chkMigrate.Value = vbUnchecked
    If Not CreateGame(sGameName, (chkMigrate.Value = vbChecked), Val(txtUsers.Text)) Then
        MsgBox "Unable to create session", vbCritical Or vbOKOnly, "No session"
        mfHost = False
        ShowPane CreateJoinGame
    Else
        'We've created our game, load our game screen and wait for people to join
        mfComplete = True
        'Clean up our address
        Set moDPA = Nothing
        Unload Me
    End If

End Sub

Private Sub cmdRefresh_Click()
    
    If mlSearch = StartSearch Then
        'Time to enum our hosts
        mfDoneEnum = False
        Dim Desc As DPN_APPLICATION_DESC
        Desc.guidApplication = msGuid
        
        If Not (moDPP Is Nothing) Then
            mlEnumAsync = moDPP.EnumHosts(Desc, Nothing, moDPA, INFINITE, 0, INFINITE, DPNENUMHOSTS_OKTOQUERYFORADDRESSING, ByVal 0&, 0)
        ElseIf Not (moDPC Is Nothing) Then
            mlEnumAsync = moDPC.EnumHosts(Desc, Nothing, moDPA, INFINITE, 0, INFINITE, DPNENUMHOSTS_OKTOQUERYFORADDRESSING, ByVal 0&, 0)
        End If
        cmdRefresh.Caption = "Stop Search"
        mlSearch = StopSearch
    ElseIf mlSearch = StopSearch Then
        mfDoneEnum = True
        If Not (moDPP Is Nothing) Then
            If mlEnumAsync <> 0 Then moDPP.CancelAsyncOperation mlEnumAsync, 0
        ElseIf Not (moDPC Is Nothing) Then
            If mlEnumAsync <> 0 Then moDPC.CancelAsyncOperation mlEnumAsync, 0
        End If
        cmdRefresh.Caption = "Start Search"
        mlSearch = StartSearch
    End If
End Sub

Private Sub AddHostsToListBox(oHost As DPNMSG_ENUM_HOSTS_RESPONSE)
    Dim lFound As Long
    
    'Here we will add a host that was found to our list box (or ignore it
    'if it's already been added)
    If mfDoneEnum Then Exit Sub
    If mlHostCount = -1 Then
        'We have no hosts already. Clear our list, and add this one to the list.
        lstGames.Clear
        ReDim moHosts(0)
        moHosts(0).AppDesc = oHost.ApplicationDescription
        moHosts(0).AddressHost = oHost.AddressSenderUrl
        moHosts(0).AddressDevice = oHost.AddressDeviceUrl
        'Save the last time this host was found
        moHosts(0).TimeLastFound = GetTickCount
        With oHost.ApplicationDescription
            lstGames.AddItem .SessionName & " - " & CStr(.lCurrentPlayers) & "/" & CStr(.lMaxPlayers) & " - Latency:" & CStr(oHost.lRoundTripLatencyMS) & " ms"
        End With
        lstGames.ItemData(0) = 0
        mlHostCount = mlHostCount + 1
    Else
        Dim lCount As Long
        Dim fFound As Boolean
        
        For lCount = 0 To mlHostCount
            If moHosts(lCount).AppDesc.guidInstance = oHost.ApplicationDescription.guidInstance Then
                'Save the last time this host was found
                moHosts(lCount).TimeLastFound = GetTickCount
                fFound = True
                Exit For
            End If
        Next
        
        If Not fFound Then 'We need to add this to the list
            ReDim Preserve moHosts(mlHostCount + 1)
            moHosts(mlHostCount + 1).AppDesc = oHost.ApplicationDescription
            moHosts(mlHostCount + 1).AddressHost = oHost.AddressSenderUrl
            moHosts(mlHostCount + 1).AddressDevice = oHost.AddressDeviceUrl
            With oHost.ApplicationDescription
                lstGames.AddItem .SessionName & " - " & CStr(.lCurrentPlayers) & "/" & CStr(.lMaxPlayers) & " - Latency:" & CStr(oHost.lRoundTripLatencyMS) & " ms"
            End With
            'Save the last time this host was found
            moHosts(mlHostCount + 1).TimeLastFound = GetTickCount
            lstGames.ItemData(lstGames.ListCount - 1) = mlHostCount + 1
            mlHostCount = mlHostCount + 1
        Else 'We did find it, update the list
            For lFound = 0 To lstGames.ListCount - 1
                With oHost.ApplicationDescription
                If lstGames.ItemData(lFound) = lCount Then 'This is it
                    lstGames.List(lFound) = .SessionName & " - " & CStr(.lCurrentPlayers) & "/" & CStr(.lMaxPlayers) & " - Latency:" & CStr(oHost.lRoundTripLatencyMS) & " ms"
                End If
                End With
            Next
        End If
    End If
End Sub

'We will handle all of the msgs here, and report them all back to the callback sub
'in case the caller cares what's going on
Private Sub DirectPlay8Event_AddRemovePlayerGroup(ByVal lMsgID As Long, ByVal lPlayerID As Long, ByVal lGroupID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallback Is Nothing) Then moCallback.AddRemovePlayerGroup lMsgID, lPlayerID, lGroupID, fRejectMsg
End Sub

Private Sub DirectPlay8Event_AppDesc(fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallback Is Nothing) Then moCallback.AppDesc fRejectMsg
End Sub

Private Sub DirectPlay8Event_AsyncOpComplete(dpnotify As DxVBLibA.DPNMSG_ASYNC_OP_COMPLETE, fRejectMsg As Boolean)
    If dpnotify.AsyncOpHandle = mlEnumAsync Then mlEnumAsync = 0
    'VB requires that we must implement *every* member of this interface
    If (Not moCallback Is Nothing) Then moCallback.AsyncOpComplete dpnotify, fRejectMsg
End Sub

Private Sub DirectPlay8Event_ConnectComplete(dpnotify As DxVBLibA.DPNMSG_CONNECT_COMPLETE, fRejectMsg As Boolean)
    mfGotEvent = True
    If dpnotify.hResultCode = DPNERR_SESSIONFULL Then 'Already too many people joined up
        MsgBox "The maximum number of people allowed in this session have already joined.  Please choose a different session or create your own.", vbOKOnly Or vbInformation, "Full"
        ShowPane CreateJoinGame
    Else
        'We got our connect complete event
        mfConnectComplete = True
        'VB requires that we must implement *every* member of this interface
        If (Not moCallback Is Nothing) Then moCallback.ConnectComplete dpnotify, fRejectMsg
    End If
End Sub

Private Sub DirectPlay8Event_CreateGroup(ByVal lGroupID As Long, ByVal lOwnerID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallback Is Nothing) Then moCallback.CreateGroup lGroupID, lOwnerID, fRejectMsg
End Sub

Private Sub DirectPlay8Event_CreatePlayer(ByVal lPlayerID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallback Is Nothing) Then moCallback.CreatePlayer lPlayerID, fRejectMsg
End Sub

Private Sub DirectPlay8Event_DestroyGroup(ByVal lGroupID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallback Is Nothing) Then moCallback.DestroyGroup lGroupID, lReason, fRejectMsg
End Sub

Private Sub DirectPlay8Event_DestroyPlayer(ByVal lPlayerID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallback Is Nothing) Then moCallback.DestroyPlayer lPlayerID, lReason, fRejectMsg
End Sub

Private Sub DirectPlay8Event_EnumHostsQuery(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_QUERY, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallback Is Nothing) Then moCallback.EnumHostsQuery dpnotify, fRejectMsg
End Sub

Private Sub DirectPlay8Event_EnumHostsResponse(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_RESPONSE, fRejectMsg As Boolean)
    'Go ahead and add this to our list
    AddHostsToListBox dpnotify
    'VB requires that we must implement *every* member of this interface
    If (Not moCallback Is Nothing) Then moCallback.EnumHostsResponse dpnotify, fRejectMsg
End Sub

Private Sub DirectPlay8Event_HostMigrate(ByVal lNewHostID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallback Is Nothing) Then moCallback.HostMigrate lNewHostID, fRejectMsg
End Sub

Private Sub DirectPlay8Event_IndicateConnect(dpnotify As DxVBLibA.DPNMSG_INDICATE_CONNECT, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallback Is Nothing) Then moCallback.IndicateConnect dpnotify, fRejectMsg
End Sub

Private Sub DirectPlay8Event_IndicatedConnectAborted(fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallback Is Nothing) Then moCallback.IndicatedConnectAborted fRejectMsg
End Sub

Private Sub DirectPlay8Event_InfoNotify(ByVal lMsgID As Long, ByVal lNotifyID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallback Is Nothing) Then moCallback.InfoNotify lMsgID, lNotifyID, fRejectMsg
End Sub

Private Sub DirectPlay8Event_Receive(dpnotify As DxVBLibA.DPNMSG_RECEIVE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallback Is Nothing) Then moCallback.Receive dpnotify, fRejectMsg
End Sub

Private Sub DirectPlay8Event_SendComplete(dpnotify As DxVBLibA.DPNMSG_SEND_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallback Is Nothing) Then moCallback.SendComplete dpnotify, fRejectMsg
End Sub

Private Sub DirectPlay8Event_TerminateSession(dpnotify As DxVBLibA.DPNMSG_TERMINATE_SESSION, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallback Is Nothing) Then moCallback.TerminateSession dpnotify, fRejectMsg
End Sub

Private Sub DirectPlay8LobbyEvent_Connect(dlNotify As DxVBLibA.DPL_MESSAGE_CONNECT, fRejectMsg As Boolean)
    Dim oDev As DirectPlay8Address, oHost As DirectPlay8Address
    Dim oSetting As DPL_CONNECTION_SETTINGS
    Dim pInfo As DPN_PLAYER_INFO
    
    On Local Error GoTo ErrOut
    mlLobbyClientID = dlNotify.ConnectId
    oSetting = moDPLA.GetConnectionSettings(mlLobbyClientID, 0)
    'We were just connected to from a lobby
    With oSetting
    If Not (moDPP Is Nothing) Then
        moDPP.RegisterLobby mlLobbyClientID, moDPLA, DPNLOBBY_REGISTER
    ElseIf Not (moDPC Is Nothing) Then
        moDPC.RegisterLobby mlLobbyClientID, moDPLA, DPNLOBBY_REGISTER
    End If
    'With ConnectionSettings
    If .PlayerName <> vbNullString Then
        sUser = .PlayerName
        'Am I the host
        If (.lFlags And DPLCONNECTSETTINGS_HOST) = DPLCONNECTSETTINGS_HOST Then
            'Get the device address to host on
            Set oDev = moDX.DirectPlayAddressCreate
            oDev.BuildFromURL dlNotify.dplMsgCon.AddressSenderUrl
            If Not CreateGameLobby(oDev, .ApplicationDescription) Then
                MsgBox "Unable to create session", vbCritical Or vbOKOnly, "No session"
                mfHost = False
                ShowPane CreateJoinGame
            Else
                'We've created our game, load our game screen and wait for people to join
                mfHost = True 'We are the host
                mfComplete = True
                'Clean up our address
                Set moDPA = Nothing
                Unload Me
            End If
        Else 'we want to connect to an running app
            sUser = .PlayerName
            'Set up my peer info
            pInfo.Name = sUser
            pInfo.lInfoFlags = DPNINFO_NAME
            'Go ahead and connect
            Set oDev = moDX.DirectPlayAddressCreate
            oDev.BuildFromURL dlNotify.dplMsgCon.AddressSenderUrl
            Set oHost = moDX.DirectPlayAddressCreate
            oHost.BuildFromURL dlNotify.dplMsgCon.AddressDeviceUrl
            If Not (moDPP Is Nothing) Then
                moDPP.SetPeerInfo pInfo, DPNOP_SYNC
                'Connect now
                moDPP.Connect .ApplicationDescription, oHost, oDev, 0, ByVal 0&, 0
            ElseIf Not (moDPC Is Nothing) Then
                moDPC.SetClientInfo pInfo, DPNOP_SYNC
                'Connect now
                moDPC.Connect .ApplicationDescription, oHost, oDev, 0, ByVal 0&, 0
            End If
            'Now we should wait until the connect complete event has fired
            Do While Not mfConnectComplete
                DoEvents
                'We need to sleep here since the Directplay callbacks run on separate
                'threads, and a tight loop with only doevents will not allow them enough
                'time to call into VB.  Sleep 'pauses' this thread for a short time,
                'allowing the callbacks to process
                Sleep 10
            Loop
            'We've joined our game
            mfComplete = True
            mfHost = False
            'Clean up our address
            Set moDPA = Nothing
            Unload Me
            
        End If
    Else
        ShowPane PickProtocol
    End If
    End With
    Exit Sub
ErrOut:
    Debug.Print "Error:" & CStr(Err.Number) & " - " & Err.Description
End Sub

Private Sub DirectPlay8LobbyEvent_ConnectionSettings(ConnectionSettings As DxVBLibA.DPL_MESSAGE_CONNECTION_SETTINGS)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8LobbyEvent_Disconnect(ByVal DisconnectID As Long, ByVal lReason As Long)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8LobbyEvent_Receive(dlNotify As DxVBLibA.DPL_MESSAGE_RECEIVE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8LobbyEvent_SessionStatus(ByVal status As Long, ByVal lHandle As Long)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub Form_QueryUnload(Cancel As Integer, UnloadMode As Integer)
    If Not mfCanUnload Then Cancel = 1
    Me.Hide
    mfDoneWiz = True
End Sub

Private Sub Form_Unload(Cancel As Integer)
    'Clean up our lobbied app
    If Not (moDPLA Is Nothing) Then
        moDPLA.Close
    End If
    Set moDPLA = Nothing
    'Clean up our address
    Set moDPA = Nothing
End Sub

Private Sub lstGames_DblClick()
    cmdJoin_Click
End Sub

Private Function CreateGame(ByVal sGameName As String, ByVal fHostMigrate As Boolean, ByVal lNumPlayers As Long) As Boolean
    On Error GoTo ErrOut
    
    'We want to host our own game
    Dim pInfo As DPN_PLAYER_INFO
    Dim AppDesc As DPN_APPLICATION_DESC
    
    'Now set up the app description
    With AppDesc
        .guidApplication = msGuid
        .lMaxPlayers = lNumPlayers
        .SessionName = sGameName
        If fHostMigrate Then
            .lFlags = .lFlags Or DPNSESSION_MIGRATE_HOST
        End If
    End With
    mlNumPlayers = lNumPlayers
    'Set up my peer info
    pInfo.Name = sUser
    pInfo.lInfoFlags = DPNINFO_NAME
    moDPP.SetPeerInfo pInfo, DPNOP_SYNC
    
    moDPP.Host AppDesc, moDPA, DPNHOST_OKTOQUERYFORADDRESSING
    
    CreateGame = True
    
    Exit Function

ErrOut:
    CreateGame = False
    Debug.Print "Error;"; Err; " - "; Err.Description
End Function

Private Sub lstSP_DblClick()
    cmdOk_Click
End Sub

Public Property Get IsHost() As Boolean
    IsHost = mfHost
End Property

Public Property Get SessionName() As String
    SessionName = sGameName
End Property

Public Property Get UserName() As String
    UserName = sUser
End Property

Public Sub GoUnload()
    tmrExpire.Enabled = False
    mfCanUnload = True
    Unload Me
End Sub

Public Sub RegisterCallback(oCallback As DirectPlay8Event)
    Set moCallback = oCallback
End Sub

Public Property Get NumPlayers() As Long
    NumPlayers = mlNumPlayers
End Property

Private Sub ShowPane(ByVal lIndex As WizPanes)
    'Here we will show the correct pane, and do whatever else
    'we might need to do to get the step set up.
    fraWiz(lIndex).ZOrder
    Select Case lIndex
    Case PickProtocol
        cmdOk.Default = True
        cmdCancel.Cancel = True
        Me.Caption = App.EXEName & " - Choose Protocol"
    Case CreateJoinGame
        cmdCancelGame.Cancel = True
        Me.Caption = App.EXEName & " - Create or Join Session"
    Case CreateNewGame
        cmdOkCreate.Default = True
        cmdCancelCreate.Cancel = True
        txtGameName.SetFocus
        Me.Caption = App.EXEName & " - Create Session"
    Case WaitForLobby
        cmdCancelLobby.Cancel = True
        cmdCancelLobby.Default = True
        cmdCancelLobby.SetFocus
        Me.Caption = App.EXEName & " - Wait for lobby connection"
    End Select
End Sub

Private Function CreateGameLobby(oHostAddr As DirectPlay8Address, newDesc As DPN_APPLICATION_DESC) As Boolean
    On Error GoTo ErrOut
    
    'We want to host our own game
    Dim pInfo As DPN_PLAYER_INFO
    
    'Set up my peer info
    pInfo.Name = sUser
    pInfo.lInfoFlags = DPNINFO_NAME
    'We are only using the Peer object, since the client object *can't* host a session
    moDPP.SetPeerInfo pInfo, DPNOP_SYNC
    
    newDesc.lFlags = newDesc.lFlags Or DPNSESSION_MIGRATE_HOST 'Turn on Migrate host by default
    newDesc.lMaxPlayers = mlMax 'Let the max players join
    sGameName = newDesc.SessionName
    moDPP.Host newDesc, oHostAddr
    CreateGameLobby = True
    
    Exit Function

ErrOut:
    CreateGameLobby = False
    Debug.Print "Error;" & CStr(Err.Number) & " - " & Err.Description
End Function

Private Sub RegisterThisApp(sGuid As String)
    Dim dplProg As DPL_PROGRAM_DESC
    'We need to register this program in case we aren't already registered
    With dplProg
        .ApplicationName = App.EXEName
        .Description = "VB DirectPlay SDK Sample"
        .ExecutableFilename = App.EXEName & ".exe"
        .ExecutablePath = App.Path
        .LauncherFilename = App.EXEName & ".exe"
        .LauncherPath = App.Path
        .guidApplication = sGuid
    End With
    moDPLA.RegisterProgram dplProg, 0
End Sub

Private Sub tmrExpire_Timer()
    
    'We need to periodically expire the hosts that are in this list in case they are
    'no longer hosting or what have you.
    Dim lCount As Long, lIndex As Long
    Dim lInner As Long
    
    On Error GoTo LeaveSub 'If there are no hosts, just go
    For lCount = 0 To UBound(moHosts)
        If (GetTickCount - moHosts(lCount).TimeLastFound) > HOST_EXPIRE_THRESHHOLD Then
            'Yup, this guy expired.. remove him from the list
            For lIndex = lstGames.ListCount - 1 To 0 Step -1
                If lstGames.ItemData(lIndex) = lCount Then 'this is the one
                    lstGames.RemoveItem lIndex
                End If
            Next
            moHosts(lCount).AddressDevice = vbNullString
            moHosts(lCount).AddressHost = vbNullString
            'Now we need an internal loop to 'remove' all of the old hosts info
            For lInner = lCount + 1 To UBound(moHosts)
                moHosts(lInner - 1).AddressDevice = moHosts(lInner).AddressDevice
                moHosts(lInner - 1).AddressHost = moHosts(lInner).AddressHost
                moHosts(lInner - 1).AppDesc = moHosts(lInner).AppDesc
                moHosts(lInner - 1).TimeLastFound = moHosts(lInner).TimeLastFound
            Next
            'Now we need to decrement each of the remaining items in the listbox
            For lIndex = lstGames.ListCount - 1 To 0 Step -1
                If lstGames.ItemData(lIndex) > lCount Then 'decrement this one
                    lstGames.ItemData(lIndex) = lstGames.ItemData(lIndex) - 1
                End If
            Next
            mlHostCount = mlHostCount - 1
            If UBound(moHosts) > 0 Then
                ReDim Preserve moHosts(UBound(moHosts) - 1)
            Else
                Erase moHosts 'This will just erase the memory
            End If
        End If
    Next
LeaveSub:
End Sub

