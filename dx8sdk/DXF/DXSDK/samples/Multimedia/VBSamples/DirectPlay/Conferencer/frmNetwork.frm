VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form frmNetwork 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "vbConferencer"
   ClientHeight    =   4605
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   3930
   Icon            =   "frmNetwork.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4605
   ScaleWidth      =   3930
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer tmrVoice 
      Enabled         =   0   'False
      Interval        =   10
      Left            =   6435
      Top             =   975
   End
   Begin VB.CheckBox chkVoice 
      Caption         =   "Enable Voice Chat"
      Height          =   255
      Left            =   1140
      TabIndex        =   9
      Top             =   3660
      Value           =   1  'Checked
      Width           =   1635
   End
   Begin MSComDlg.CommonDialog cdlSend 
      Left            =   6360
      Top             =   3180
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
      DialogTitle     =   "Send File"
      Filter          =   "Any File |*.*"
      Flags           =   4
      InitDir         =   "C:\"
   End
   Begin VB.Timer tmrJoin 
      Enabled         =   0   'False
      Interval        =   50
      Left            =   6420
      Top             =   540
   End
   Begin VB.Timer tmrUpdate 
      Enabled         =   0   'False
      Interval        =   10
      Left            =   6420
      Top             =   60
   End
   Begin VB.TextBox txtCall 
      Height          =   285
      Left            =   60
      TabIndex        =   0
      Top             =   300
      Width           =   2535
   End
   Begin VB.ListBox lstUsers 
      Height          =   2595
      Left            =   60
      TabIndex        =   3
      Top             =   1020
      Width           =   3795
   End
   Begin VB.CommandButton cmdHangup 
      Height          =   495
      Left            =   3240
      MaskColor       =   &H00FF0000&
      Picture         =   "frmNetwork.frx":030A
      Style           =   1  'Graphical
      TabIndex        =   2
      ToolTipText     =   "Hang up"
      Top             =   120
      UseMaskColor    =   -1  'True
      Width           =   495
   End
   Begin VB.CommandButton cmdCall 
      Default         =   -1  'True
      Height          =   495
      Left            =   2700
      MaskColor       =   &H000000FF&
      Picture         =   "frmNetwork.frx":0A0C
      Style           =   1  'Graphical
      TabIndex        =   1
      ToolTipText     =   "Call a friend"
      Top             =   120
      UseMaskColor    =   -1  'True
      Width           =   495
   End
   Begin VB.CommandButton cmdWhiteBoard 
      Height          =   495
      Left            =   2325
      MaskColor       =   &H000000FF&
      Picture         =   "frmNetwork.frx":110E
      Style           =   1  'Graphical
      TabIndex        =   6
      ToolTipText     =   "Use the whiteboard"
      Top             =   4020
      UseMaskColor    =   -1  'True
      Width           =   495
   End
   Begin VB.CommandButton cmdChat 
      Height          =   495
      Left            =   1125
      MaskColor       =   &H000000FF&
      Picture         =   "frmNetwork.frx":1A18
      Style           =   1  'Graphical
      TabIndex        =   4
      ToolTipText     =   "Chat with someone"
      Top             =   4020
      UseMaskColor    =   -1  'True
      Width           =   495
   End
   Begin VB.CommandButton cmdSendFile 
      Height          =   495
      Left            =   1725
      MaskColor       =   &H000000FF&
      Picture         =   "frmNetwork.frx":2322
      Style           =   1  'Graphical
      TabIndex        =   5
      ToolTipText     =   "Transfer files to someone"
      Top             =   4020
      UseMaskColor    =   -1  'True
      Width           =   495
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "Enter a name or IP to call"
      Height          =   195
      Index           =   1
      Left            =   60
      TabIndex        =   8
      Top             =   60
      Width           =   2475
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "Users currently in this session"
      Height          =   315
      Index           =   0
      Left            =   60
      TabIndex        =   7
      Top             =   780
      Width           =   3735
   End
   Begin VB.Menu mnuPopup 
      Caption         =   "PopUp"
      Visible         =   0   'False
      Begin VB.Menu mnuExit 
         Caption         =   "E&xit"
      End
   End
End
Attribute VB_Name = "frmNetwork"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmNetwork.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Implements DirectPlay8Event
Implements DirectPlayVoiceEvent8

'You can make bigger or smaller chunks here
Private Const mlFileChunkSize As Long = 512

'Variables for file transfers
Private moReceivedFiles As New Collection
Private moSendFiles As New Collection
Private mlSendUnique As Long
'Misc private variables
Private moCallBack As DirectPlay8Event
Private mfExit As Boolean
Private mfTerminate As Boolean
Private mlVoiceError As Long

Private Sub chkVoice_Click()
    If gfNoVoice Then Exit Sub 'Ignore this since voice chat isn't possible on this session
    If chkVoice.Value = vbChecked Then
        ConnectVoice Me
    ElseIf chkVoice.Value = vbUnchecked Then
        If Not (dvClient Is Nothing) Then dvClient.UnRegisterMessageHandler
        If Not (dvClient Is Nothing) Then dvClient.Disconnect DVFLAGS_SYNC
        Set dvClient = Nothing
    End If
End Sub

Private Sub cmdCall_Click()
    If txtCall.Text = vbNullString Then
        MsgBox "You must type the name or address of the person you wish to call before I can make the call.", vbOKOnly Or vbInformation, "No callee"
        Exit Sub
    End If
    Connect Me, txtCall.Text
End Sub

Private Sub cmdChat_Click()
    If lstUsers.ListCount < 2 Then
        MsgBox "You must have at least two people in the session before you can chat.", vbOKOnly Or vbInformation, "Not enough people"
        Exit Sub
    End If
    If ChatWindow Is Nothing Then Set ChatWindow = New frmChat
    ChatWindow.Show vbModeless
    'Notify everyone
    SendOpenChatWindowMessage
    Set moCallBack = ChatWindow
End Sub

Private Sub cmdHangup_Click()
    'Cleanup and quit
    mfExit = True
    Unload Me
End Sub

Private Sub cmdSendFile_Click()
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    
    If lstUsers.ListIndex < 0 Then
        MsgBox "You must select someone to send a file to before sending one.", vbOKOnly Or vbInformation, "No selection"
        Exit Sub
    End If
    If lstUsers.ListIndex < 1 Then
        MsgBox "You must select someone other than yourself to send a file to before sending one.", vbOKOnly Or vbInformation, "No selection"
        Exit Sub
    End If

    'Ok, we can send a file.. Let them pick one
    cdlSend.FileName = vbNullString
    On Error Resume Next
    cdlSend.ShowOpen
    If Err Or (cdlSend.FileName = vbNullString) Then Exit Sub 'They clicked cancel
    cdlSend.InitDir = GetFolder(cdlSend.FileName)
    'Otherwise start the file send
    LockSendCollection
    Dim f As frmProgress
    Set f = New frmProgress
    With f
        .sFileName = cdlSend.FileName
        .lDPlayID = lstUsers.ItemData(lstUsers.ListIndex)
        mlSendUnique = mlSendUnique + 1
        .FileUniqueID = mlSendUnique
        'We need to send a 'Request' message first
        lOffset = NewBuffer(oBuf)
        lMsg = MsgSendFileRequest
        AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
        AddDataToBuffer oBuf, mlSendUnique, LenB(mlSendUnique), lOffset
        AddStringToBuffer oBuf, StripFileName(cdlSend.FileName), lOffset
        dpp.SendTo lstUsers.ItemData(lstUsers.ListIndex), oBuf, 0, DPNSEND_NOLOOPBACK Or DPNSEND_GUARANTEED
    End With
    moSendFiles.Add f
    UnlockSendCollection
End Sub

Private Sub cmdWhiteBoard_Click()
    If lstUsers.ListCount < 2 Then
        MsgBox "You must have at least two people in the session before you can use the whiteboard.", vbOKOnly Or vbInformation, "Not enough people"
        Exit Sub
    End If
    If WhiteBoardWindow Is Nothing Then Set WhiteBoardWindow = New frmWhiteBoard
    WhiteBoardWindow.Show vbModeless
    'Notify everyone
    SendOpenWhiteBoardWindowMessage
    Set moCallBack = WhiteBoardWindow
End Sub

Private Sub Form_Load()
    'First start our server.  We need to be running a server in case
    'someone tries to connect to us.
    
    StartHosting Me
    'Add ourselves to the listbox
    lstUsers.AddItem gsUserName
    lstUsers.ItemData(0) = glMyPlayerID
    
    'Now put up our system tray icon
    With sysIcon
        .cbSize = LenB(sysIcon)
        .hwnd = Me.hwnd
        .uFlags = NIF_DOALL
        .uCallbackMessage = WM_MOUSEMOVE
        .hIcon = Me.Icon
        .sTip = "vbConferencer" & vbNullChar
    End With
    Shell_NotifyIcon NIM_ADD, sysIcon
End Sub

Private Sub Form_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim ShellMsg As Long
    
    ShellMsg = X / Screen.TwipsPerPixelX
    Select Case ShellMsg
    Case WM_LBUTTONDBLCLK
        ShowMyForm
    Case WM_RBUTTONUP
        'Show the menu
        'If gfStarted Then mnuStart.Enabled = False
        PopupMenu mnuPopup, , , , mnuExit
    End Select
End Sub

Private Sub Form_QueryUnload(Cancel As Integer, UnloadMode As Integer)
    If Not mfExit Then
        Cancel = 1
        Me.Hide
    End If
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Dim f As Form
    Dim lCount As Long
    
    Me.Hide
    Shell_NotifyIcon NIM_DELETE, sysIcon
    Cleanup
    For lCount = 1 To moSendFiles.Count 'Clear the collection
        moSendFiles.Remove 1
    Next
    Set moSendFiles = Nothing
    For lCount = 1 To moReceivedFiles.Count 'Clear the collection
        moReceivedFiles.Remove 1
    Next
    Set moReceivedFiles = Nothing
    
    For Each f In Forms
        If Not (f Is Me) Then
            Unload f
            Set f = Nothing
        End If
    Next
    DeleteCriticalSection goSendFile
    DeleteCriticalSection goReceiveFile
    End
End Sub

Private Sub mnuExit_Click()
    mfExit = True
    Unload Me
End Sub

Private Sub ShowMyForm()
    Me.Visible = True
End Sub

Private Sub tmrJoin_Timer()
    tmrJoin.Enabled = False
    MsgBox "The person you are trying to reach did not accept your call.", vbOKOnly Or vbInformation, "Didn't accept"
    StartHosting Me
End Sub

Public Sub UpdatePlayerList()
    Dim lCount As Long, dpPeer As DPN_PLAYER_INFO
    Dim lInner As Long, fFound As Boolean
    Dim lTotal As Long
    
    lTotal = dpp.GetCountPlayersAndGroups(DPNENUM_PLAYERS)
    If lTotal > 1 Then
        cmdHangup.Enabled = True
        cmdCall.Enabled = False
    End If
    For lCount = 1 To lTotal
        dpPeer = dpp.GetPeerInfo(dpp.GetPlayerOrGroup(lCount))
        If (dpPeer.lPlayerFlags And DPNPLAYER_LOCAL) = DPNPLAYER_LOCAL Then
            'Don't add me
        Else
            fFound = False
            'Make sure they're not already added
            For lInner = 0 To lstUsers.ListCount - 1
                If lstUsers.ItemData(lInner) = dpp.GetPlayerOrGroup(lCount) Then fFound = True
            Next
            If Not fFound Then
                'Go ahead and add them
                lstUsers.AddItem dpPeer.Name
                lstUsers.ItemData(lstUsers.ListCount - 1) = dpp.GetPlayerOrGroup(lCount)
            End If
        End If
    Next
End Sub

Private Sub SendOpenWhiteBoardWindowMessage()
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    
    'Now let's send a message asking the host to accept our call
    lOffset = NewBuffer(oBuf)
    lMsg = MsgShowWhiteBoard
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_NOLOOPBACK
End Sub

Private Sub SendOpenChatWindowMessage()
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    
    'Now let's send a message asking the host to accept our call
    lOffset = NewBuffer(oBuf)
    lMsg = MsgShowChat
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_NOLOOPBACK
End Sub

Private Sub RemovePlayer(ByVal lPlayerID As Long)
    Dim lCount As Long
    'Remove anyone who has this player id
    For lCount = 0 To lstUsers.ListCount - 1
        If lstUsers.ItemData(lCount) = lPlayerID Then lstUsers.RemoveItem lCount
    Next
    If Not (ChatWindow Is Nothing) Then ChatWindow.LoadAllPlayers
    'Let's see if there are any files being sent to this user
    Dim f As frmProgress
    LockSendCollection
    For Each f In moSendFiles
        If f.lDPlayID = lPlayerID Then
            'Notify the user
            MsgBox "Cancelled transfering file " & f.sFileName & " because the user quit."
            'Yup, get rid of this file
            EraseSendFile f.FileUniqueID
        End If
    Next
    UnlockSendCollection
    'Now look through the receive collection
    LockReceiveCollection
    For Each f In moReceivedFiles
        If f.lDPlayID = lPlayerID Then
            'Notify the user
            MsgBox "Cancelled receiving file " & f.sFileName & " because the user quit."
            'Yup, get rid of this file
            EraseReceiveFile f.FileUniqueID
        End If
    Next
    UnlockReceiveCollection
    If lstUsers.ListCount <= 1 Then 'We are the only person left
        cmdCall.Enabled = True
        cmdHangup.Enabled = False
    End If
End Sub

Private Function StripFileName(ByVal sFile As String) As String
    'Get rid of the path to the file (Strip everything after the last \)
    If InStr(sFile, "\") Then
        StripFileName = Right$(sFile, Len(sFile) - InStrRev(sFile, "\"))
    Else
        StripFileName = sFile
    End If
End Function

Private Sub SendNextFilePart(ByVal lUniqueID As Long)
    
    Dim lNewMsg As Long, lNewOffSet As Long
    Dim oBuf() As Byte, lChunkSize As Long
    Dim oFile() As Byte, f As frmProgress
    
    'First we need to find the correct file in our send list
    LockSendCollection
    Set f = GetSendProgressForm(lUniqueID)
    With f
        'Send this chunk
        lNewOffSet = NewBuffer(oBuf)
        lNewMsg = MsgSendFilePart
        AddDataToBuffer oBuf, lNewMsg, LenB(lNewMsg), lNewOffSet
        AddDataToBuffer oBuf, .FileUniqueID, SIZE_LONG, lNewOffSet
        'Is this chunk bigger than the amount we will send?
        If .lCurrentPos + mlFileChunkSize > .lFileSize Then
            'First send the chunksize
            lChunkSize = .lFileSize - .lCurrentPos
        Else
            lChunkSize = mlFileChunkSize
        End If
        AddDataToBuffer oBuf, lChunkSize, LenB(lChunkSize), lNewOffSet
        ReDim oFile(1 To lChunkSize)
        'Now read in a chunk that size
        If .filNumber = 0 Then
            .filNumber = FreeFile
            Open .sFileName For Binary Access Read As #.filNumber
        End If
        Get #.filNumber, , oFile
        AddDataToBuffer oBuf, oFile(1), lChunkSize, lNewOffSet
        dpp.SendTo .lDPlayID, oBuf, 0, DPNSEND_NOLOOPBACK Or DPNSEND_GUARANTEED
        .lCurrentPos = .lCurrentPos + lChunkSize
        'Update our transfer window
        .SetValue .lCurrentPos
        If .lCurrentPos >= .lFileSize Then
            Close #.filNumber
            'Now get rid of this member of the array
            EraseSendFile .FileUniqueID
        End If
    End With
    UnlockSendCollection
End Sub

Public Sub EraseSendFile(ByVal lUnique As Long)
    Dim lCount As Long, f As frmProgress
    
    'First we need to find the correct file in our send list
    LockSendCollection
    For lCount = moSendFiles.Count To 1 Step -1
        Set f = moSendFiles.Item(lCount)
        If f.FileUniqueID = lUnique Then
            moSendFiles.Remove lCount
            Unload f
            Set f = Nothing
            Exit For
        End If
    Next
    UnlockSendCollection
End Sub
Public Sub EraseReceiveFile(ByVal lUnique As Long)
    Dim lCount As Long, f As frmProgress
    
    'First we need to find the correct file in our send list
    LockReceiveCollection
    For lCount = moReceivedFiles.Count To 1 Step -1
        Set f = moReceivedFiles.Item(lCount)
        If f.FileUniqueID = lUnique Then
            moReceivedFiles.Remove lCount
            Unload f.RequestForm
            Set f.RequestForm = Nothing
            Unload f
            Set f = Nothing
            Exit For
        End If
    Next
    UnlockReceiveCollection
End Sub

Private Function GetSendProgressForm(ByVal lUnique As Long) As frmProgress
    Dim f As frmProgress
    
    LockSendCollection
    For Each f In moSendFiles
        If f.FileUniqueID = lUnique Then
            Set GetSendProgressForm = f
            Exit For
        End If
    Next
    UnlockSendCollection
End Function

Private Function GetReceiveProgressForm(ByVal lUnique As Long) As frmProgress
    Dim f As frmProgress
    
    LockReceiveCollection
    For Each f In moReceivedFiles
        If f.FileUniqueID = lUnique Then
            Set GetReceiveProgressForm = f
            Exit For
        End If
    Next
    UnlockReceiveCollection
End Function

Private Function GetFolder(ByVal sFile As String) As String
    Dim lCount As Long
    
    For lCount = Len(sFile) To 1 Step -1
        If Mid$(sFile, lCount, 1) = "\" Then
            GetFolder = Left$(sFile, lCount)
            Exit Function
        End If
    Next
    GetFolder = vbNullString
End Function

Private Sub tmrUpdate_Timer()
    tmrUpdate.Enabled = False
    If Not mfTerminate Then
        MsgBox "The person you are trying to reach is not available.", vbOKOnly Or vbInformation, "Unavailable"
    End If
    StartHosting Me
    mfTerminate = False
End Sub

Private Sub tmrVoice_Timer()
    tmrVoice.Enabled = False
    MsgBox "Could not start DirectPlayVoice.  This sample will not have any voice capablities." & vbCrLf & "Error:" & CStr(mlVoiceError), vbOKOnly Or vbInformation, "No Voice"
    gfNoVoice = True
    chkVoice.Value = vbUnchecked
    chkVoice.Enabled = False
End Sub

'We will hold a critical section for the two separate collections
'This will ensure that two threads can't access the data at the same time
Public Sub LockSendCollection()
    EnterCriticalSection goSendFile
End Sub

Public Sub UnlockSendCollection()
    LeaveCriticalSection goSendFile
End Sub

Public Sub LockReceiveCollection()
    EnterCriticalSection goReceiveFile
End Sub

Public Sub UnlockReceiveCollection()
    LeaveCriticalSection goReceiveFile
End Sub

'We will handle all of the msgs here, and report them all back to the callback sub
'in case the caller cares what's going on
Private Sub DirectPlay8Event_AddRemovePlayerGroup(ByVal lMsgID As Long, ByVal lPlayerID As Long, ByVal lGroupID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallBack Is Nothing) Then moCallBack.AddRemovePlayerGroup lMsgID, lPlayerID, lGroupID, fRejectMsg
End Sub

Private Sub DirectPlay8Event_AppDesc(fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallBack Is Nothing) Then moCallBack.AppDesc fRejectMsg
End Sub

Private Sub DirectPlay8Event_AsyncOpComplete(dpnotify As DxVBLibA.DPNMSG_ASYNC_OP_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallBack Is Nothing) Then moCallBack.AsyncOpComplete dpnotify, fRejectMsg
End Sub

Private Sub DirectPlay8Event_ConnectComplete(dpnotify As DxVBLibA.DPNMSG_CONNECT_COMPLETE, fRejectMsg As Boolean)
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    
    If dpnotify.hResultCode = 0 Then 'Success!
        cmdHangup.Enabled = True
        'Now let's send a message asking the host to accept our call
        lOffset = NewBuffer(oBuf)
        lMsg = MsgAskToJoin
        AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
        dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_NOLOOPBACK
    Else
        tmrUpdate.Enabled = True
    End If
    
    'VB requires that we must implement *every* member of this interface
    If (Not moCallBack Is Nothing) Then moCallBack.ConnectComplete dpnotify, fRejectMsg
End Sub

Private Sub DirectPlay8Event_CreateGroup(ByVal lGroupID As Long, ByVal lOwnerID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallBack Is Nothing) Then moCallBack.CreateGroup lGroupID, lOwnerID, fRejectMsg
End Sub

Private Sub DirectPlay8Event_CreatePlayer(ByVal lPlayerID As Long, fRejectMsg As Boolean)
    Dim dpPeer As DPN_PLAYER_INFO
    On Error Resume Next
    dpPeer = dpp.GetPeerInfo(lPlayerID)
    If (dpPeer.lPlayerFlags And DPNPLAYER_LOCAL) = DPNPLAYER_LOCAL Then
        glMyPlayerID = lPlayerID
        lstUsers.ItemData(0) = glMyPlayerID
    End If
    'VB requires that we must implement *every* member of this interface
    If (Not moCallBack Is Nothing) Then moCallBack.CreatePlayer lPlayerID, fRejectMsg
End Sub

Private Sub DirectPlay8Event_DestroyGroup(ByVal lGroupID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallBack Is Nothing) Then moCallBack.DestroyGroup lGroupID, lReason, fRejectMsg
End Sub

Private Sub DirectPlay8Event_DestroyPlayer(ByVal lPlayerID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    Dim dpPeer As DPN_PLAYER_INFO
    On Error Resume Next
    If lPlayerID <> glMyPlayerID Then 'ignore removing myself
        RemovePlayer lPlayerID
    End If
    If Not (ChatWindow Is Nothing) Then Set moCallBack = ChatWindow 'If the chat window is open, let them know about the departure.
    'VB requires that we must implement *every* member of this interface
    If (Not moCallBack Is Nothing) Then moCallBack.DestroyPlayer lPlayerID, lReason, fRejectMsg
    
End Sub

Private Sub DirectPlay8Event_EnumHostsQuery(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_QUERY, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallBack Is Nothing) Then moCallBack.EnumHostsQuery dpnotify, fRejectMsg
End Sub

Private Sub DirectPlay8Event_EnumHostsResponse(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_RESPONSE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallBack Is Nothing) Then moCallBack.EnumHostsResponse dpnotify, fRejectMsg
End Sub

Private Sub DirectPlay8Event_HostMigrate(ByVal lNewHostID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallBack Is Nothing) Then moCallBack.HostMigrate lNewHostID, fRejectMsg
End Sub

Private Sub DirectPlay8Event_IndicateConnect(dpnotify As DxVBLibA.DPNMSG_INDICATE_CONNECT, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallBack Is Nothing) Then moCallBack.IndicateConnect dpnotify, fRejectMsg
End Sub

Private Sub DirectPlay8Event_IndicatedConnectAborted(fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallBack Is Nothing) Then moCallBack.IndicatedConnectAborted fRejectMsg
End Sub

Private Sub DirectPlay8Event_InfoNotify(ByVal lMsgID As Long, ByVal lNotifyID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallBack Is Nothing) Then moCallBack.InfoNotify lMsgID, lNotifyID, fRejectMsg
End Sub

Private Sub DirectPlay8Event_Receive(dpnotify As DxVBLibA.DPNMSG_RECEIVE, fRejectMsg As Boolean)
    Dim lNewMsg As Long, lNewOffSet As Long
    Dim oBuf() As Byte, f As frmProgress
    
    Dim lMsg As Long, lOffset As Long
    Dim frmJoin As frmJoinRequest
    Dim dpPeer As DPN_PLAYER_INFO
    Dim sFile As String, lUnique As Long
    Dim oFile() As Byte, lFileSize As Long
    
    Dim lChunkSize As Long, oData() As Byte
    
    With dpnotify
    GetDataFromBuffer .ReceivedData, lMsg, LenB(lMsg), lOffset
    Select Case lMsg
    Case MsgChat, MsgWhisper 'Make sure chat messages get to the chat window
        If ChatWindow Is Nothing Then
            Set ChatWindow = New frmChat
        End If
        ChatWindow.Show
        Set moCallBack = ChatWindow
    Case MsgSendDrawPixel, MsgClearWhiteBoard
        If WhiteBoardWindow Is Nothing Then
            Set WhiteBoardWindow = New frmWhiteBoard
        End If
        WhiteBoardWindow.Show
        Set moCallBack = WhiteBoardWindow
    Case MsgAskToJoin
        If gfHost Then
            'We are the host, pop up the 'Ask to join dialog
            dpPeer = dpp.GetPeerInfo(dpnotify.idSender)
            Set frmJoin = New frmJoinRequest
            frmJoin.SetupRequest Me, dpnotify.idSender, dpPeer.Name
            frmJoin.Show vbModeless
        End If
    Case MsgAcceptJoin
        'We have been accepted
        'Enumerate all the players and add anyone we don't already have listed
        UpdatePlayerList
        If Not (ChatWindow Is Nothing) Then ChatWindow.LoadAllPlayers
        ConnectVoice Me
    Case MsgRejectJoin
        'We have been rejected
        tmrJoin.Enabled = True
        'We need to use a timer here, without it, we would be attempting to cleanup
        'our dplay objects to restart our host before this message was done being processed.
    Case MsgShowChat
        'Someone wants to chat.  Open the chat window
        If ChatWindow Is Nothing Then Set ChatWindow = New frmChat
        ChatWindow.Show vbModeless
        Set moCallBack = ChatWindow
    Case MsgShowWhiteBoard
        'Someone wants to draw.  Open the whiteboard window
        If WhiteBoardWindow Is Nothing Then Set WhiteBoardWindow = New frmWhiteBoard
        WhiteBoardWindow.Show vbModeless
        Set moCallBack = WhiteBoardWindow
    Case MsgSendFileRequest
        'Someone wants to send us a file.  Should we accept?
        dpPeer = dpp.GetPeerInfo(dpnotify.idSender)
        GetDataFromBuffer .ReceivedData, lUnique, LenB(lUnique), lOffset
        sFile = GetStringFromBuffer(.ReceivedData, lOffset)
        LockReceiveCollection
        Set f = New frmProgress
        With f
            .FileUniqueID = lUnique
            .sFileName = sFile
            .lDPlayID = dpnotify.idSender
            Set .RequestForm = New frmTransferRequest
            .RequestForm.SetupRequest Me, dpPeer.Name, .sFileName, .FileUniqueID, dpnotify.idSender
            .RequestForm.Show vbModeless
        End With
        moReceivedFiles.Add f
        UnlockReceiveCollection
    Case MsgSendFileDeny
        'We don't care about this file
        GetDataFromBuffer .ReceivedData, lUnique, LenB(lUnique), lOffset
        'Now remove this one
        EraseSendFile lUnique
    Case MsgSendFileAccept
        'Ok, they do want us to send the file to them.. We will send it in chunks
        'First we will send the file info
        GetDataFromBuffer .ReceivedData, lUnique, LenB(lUnique), lOffset
        'First we need to find the correct file in our send list
        LockSendCollection
        Set f = GetSendProgressForm(lUnique)
        lNewOffSet = NewBuffer(oBuf)
        lMsg = MsgSendFileInfo
        AddDataToBuffer oBuf, lMsg, LenB(lMsg), lNewOffSet
        With f
            .lFileSize = FileLen(.sFileName)
            AddDataToBuffer oBuf, .FileUniqueID, SIZE_LONG, lNewOffSet
            AddDataToBuffer oBuf, .lFileSize, SIZE_LONG, lNewOffSet
            dpp.SendTo .lDPlayID, oBuf, 0, DPNSEND_NOLOOPBACK Or DPNSEND_GUARANTEED
            .SetFile .sFileName
            .SetMax .lFileSize
            .SetValue 0
            .Show
        End With
        UnlockSendCollection
    Case MsgSendFileInfo
        'They just send us the file size, save it
        GetDataFromBuffer .ReceivedData, lUnique, LenB(lUnique), lOffset
        'First we need to find the correct file in our receive list
        LockReceiveCollection
        Set f = GetReceiveProgressForm(lUnique)
        With f
            GetDataFromBuffer dpnotify.ReceivedData, lFileSize, LenB(lFileSize), lOffset
            .lFileSize = lFileSize
            .SetFile .sFileName, True
            .SetMax .lFileSize
            .SetValue 0
            .Show
        End With
        'Acknowledge that we received this part
        lNewMsg = MsgAckFilePart
        lNewOffSet = NewBuffer(oBuf)
        AddDataToBuffer oBuf, lNewMsg, LenB(lNewMsg), lNewOffSet
        AddDataToBuffer oBuf, lUnique, LenB(lUnique), lNewOffSet
        dpp.SendTo dpnotify.idSender, oBuf, 0, DPNSEND_NOLOOPBACK Or DPNSEND_GUARANTEED
        UnlockReceiveCollection
    Case MsgSendFilePart
        'They just send us the file size, save it
        GetDataFromBuffer .ReceivedData, lUnique, LenB(lUnique), lOffset
        GetDataFromBuffer .ReceivedData, lChunkSize, LenB(lChunkSize), lOffset
        'First we need to find the correct file in our receive list
        LockReceiveCollection
        Set f = GetReceiveProgressForm(lUnique)
        With f
            ReDim oData(1 To lChunkSize)
            'We just received a file part..  Append this to our current file
            If .filNumber = 0 Then
                .filNumber = FreeFile
                If Dir$(App.Path & "\" & .sFileName) <> vbNullString Then Kill App.Path & "\" & .sFileName
                Open App.Path & "\" & .sFileName For Binary Access Write As #.filNumber
            End If
            GetDataFromBuffer dpnotify.ReceivedData, oData(1), lChunkSize, lOffset
            Put #.filNumber, , oData
            'Is this the end of the file?
            .lCurrentPos = .lCurrentPos + lChunkSize
            .SetValue .lCurrentPos
            If .lCurrentPos >= .lFileSize Then
                'We're done with the file
                Close #.filNumber
                EraseReceiveFile .FileUniqueID
            Else
                'Acknowledge that we received this part
                lNewMsg = MsgAckFilePart
                lNewOffSet = NewBuffer(oBuf)
                AddDataToBuffer oBuf, lNewMsg, LenB(lNewMsg), lNewOffSet
                AddDataToBuffer oBuf, lUnique, LenB(lUnique), lNewOffSet
                dpp.SendTo dpnotify.idSender, oBuf, 0, DPNSEND_NOLOOPBACK Or DPNSEND_GUARANTEED
            End If
        End With
        UnlockReceiveCollection
    Case MsgAckFilePart
        GetDataFromBuffer .ReceivedData, lUnique, LenB(lUnique), lOffset
        SendNextFilePart lUnique
    Case MsgNewPlayerJoined
        UpdatePlayerList 'Update our list here
        If Not (ChatWindow Is Nothing) Then ChatWindow.LoadAllPlayers 'And in the chat window if we need to
    End Select
    End With
    
    If (Not moCallBack Is Nothing) Then moCallBack.Receive dpnotify, fRejectMsg
End Sub

Private Sub DirectPlay8Event_SendComplete(dpnotify As DxVBLibA.DPNMSG_SEND_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallBack Is Nothing) Then moCallBack.SendComplete dpnotify, fRejectMsg
End Sub

Private Sub DirectPlay8Event_TerminateSession(dpnotify As DxVBLibA.DPNMSG_TERMINATE_SESSION, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
    If (Not moCallBack Is Nothing) Then moCallBack.TerminateSession dpnotify, fRejectMsg
    mfTerminate = True
    tmrUpdate.Enabled = True
End Sub

Private Sub DirectPlayVoiceEvent8_ConnectResult(ByVal ResultCode As Long)
    Dim lTargets(0) As Long
    
    lTargets(0) = DVID_ALLPLAYERS
    On Error Resume Next
    'Connect the client
    dvClient.SetTransmitTargets lTargets, 0
    If Err.Number <> 0 And Err.Number <> DVERR_PENDING Then
        mlVoiceError = Err.Number
        tmrVoice.Enabled = True
        Exit Sub
    End If

End Sub

Private Sub DirectPlayVoiceEvent8_CreateVoicePlayer(ByVal playerID As Long, ByVal flags As Long)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlayVoiceEvent8_DeleteVoicePlayer(ByVal playerID As Long)
    'VB requires that we must implement *every* member of this interface
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

Private Sub DirectPlayVoiceEvent8_PlayerOutputLevel(ByVal SourcePlayerID As Long, ByVal PeakLevel As Long)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlayVoiceEvent8_PlayerVoiceStart(ByVal SourcePlayerID As Long)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlayVoiceEvent8_PlayerVoiceStop(ByVal SourcePlayerID As Long)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlayVoiceEvent8_RecordStart(ByVal PeakVolume As Long)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlayVoiceEvent8_RecordStop(ByVal PeakVolume As Long)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlayVoiceEvent8_SessionLost(ByVal ResultCode As Long)
    'VB requires that we must implement *every* member of this interface
End Sub
