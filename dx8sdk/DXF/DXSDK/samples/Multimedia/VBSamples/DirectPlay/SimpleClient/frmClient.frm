VERSION 5.00
Begin VB.Form frmClient 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "vbSimple Client"
   ClientHeight    =   4470
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   5400
   Icon            =   "frmClient.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4470
   ScaleWidth      =   5400
   StartUpPosition =   3  'Windows Default
   Begin VB.Frame Rules 
      Caption         =   "Rules"
      Height          =   855
      Left            =   60
      TabIndex        =   6
      Top             =   120
      Width           =   5295
      Begin VB.Label Label1 
         BackStyle       =   0  'Transparent
         Caption         =   $"frmClient.frx":0442
         Height          =   615
         Index           =   1
         Left            =   60
         TabIndex        =   7
         Top             =   180
         Width           =   5055
      End
   End
   Begin VB.CommandButton cmdExit 
      Cancel          =   -1  'True
      Caption         =   "Exit"
      Height          =   375
      Left            =   3143
      TabIndex        =   5
      Top             =   4020
      Width           =   1215
   End
   Begin VB.CommandButton cmdFace 
      Caption         =   "Make Faces"
      Default         =   -1  'True
      Height          =   375
      Left            =   1043
      TabIndex        =   4
      Top             =   4020
      Width           =   1215
   End
   Begin VB.TextBox txtUserInfo 
      BackColor       =   &H8000000F&
      Height          =   1935
      Left            =   60
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      ScrollBars      =   2  'Vertical
      TabIndex        =   1
      Top             =   1980
      Width           =   5295
   End
   Begin VB.Frame Frame1 
      Caption         =   "User Stats"
      Height          =   915
      Left            =   60
      TabIndex        =   0
      Top             =   1020
      Width           =   5235
      Begin VB.Label lblSession 
         BackStyle       =   0  'Transparent
         Height          =   255
         Left            =   120
         TabIndex        =   3
         Top             =   240
         Width           =   4935
      End
      Begin VB.Label lblStats 
         BackStyle       =   0  'Transparent
         Height          =   255
         Left            =   120
         TabIndex        =   2
         Top             =   540
         Width           =   4995
      End
   End
End
Attribute VB_Name = "frmClient"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmClient.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Implements DirectPlay8Event
Private Enum MsgTypes
    Msg_NoOtherPlayers
    Msg_NumPlayers
    Msg_SendWave
End Enum

Private Sub cmdExit_Click()
    Unload Me
End Sub

Private Sub cmdFace_Click()
    'Now we just need to 'make faces'
    Dim oMsg() As Byte, lOffset As Long
    lOffset = NewBuffer(oMsg)
    AddDataToBuffer oMsg, CByte(1), SIZE_BYTE, lOffset
    dpc.Send oMsg, 0, DPNSEND_NOLOOPBACK
End Sub

Private Sub Form_Load()
        
    Set DPlayEventsForm = New DPlayConnect
    'First lets get the dplay connection started
    If Not DPlayEventsForm.StartClientConnectWizard(dx, dpc, AppGuid, 10, Me) Then
        Cleanup
        End
    End If

End Sub

Private Sub Form_Unload(Cancel As Integer)
    Me.Hide
    DPlayEventsForm.DoSleep 50
    Cleanup
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
    Dim AppDesc As DPN_APPLICATION_DESC
    
    If dpnotify.hResultCode <> 0 Then
        'For some reason we could not connect.  All available slots must be closed.
        MsgBox "Connect Failed.  Error: 0x" & CStr(Hex$(dpnotify.hResultCode)) & "  - This sample will now close.", vbOKOnly Or vbCritical, "Closing"
        DPlayEventsForm.CloseForm Me
    Else
        AppDesc = dpc.GetApplicationDesc(0)
        Me.Caption = AppDesc.SessionName
        lblSession = "Session Name: " & AppDesc.SessionName
        lblStats.Caption = "Total clients: " & CStr(AppDesc.lCurrentPlayers) & "/" & CStr(AppDesc.lMaxPlayers)
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
    'The server is telling us something.  What?
    Dim sPlayer As String, lOffset As Long
    Dim lMsg As Long, lNum As Long, lMax As Long
    
    GetDataFromBuffer dpnotify.ReceivedData, lMsg, LenB(lMsg), lOffset
    Select Case lMsg
    Case Msg_NumPlayers
        GetDataFromBuffer dpnotify.ReceivedData, lNum, LenB(lNum), lOffset
        GetDataFromBuffer dpnotify.ReceivedData, lMax, LenB(lMax), lOffset
        lblStats.Caption = "Total clients: " & CStr(lNum) & "/" & CStr(lMax)
    Case Msg_NoOtherPlayers
        txtUserInfo.Text = txtUserInfo.Text & "There are no other players to make funny faces at!" & vbCrLf
        txtUserInfo.SelStart = Len(txtUserInfo.Text)
    Case Msg_SendWave
        'The only data we will receive is player info
        sPlayer = GetStringFromBuffer(dpnotify.ReceivedData, lOffset)
        'Append the data to the end of the line, and autoscroll there
        txtUserInfo.Text = txtUserInfo.Text & sPlayer & " is making faces at you!" & vbCrLf
        txtUserInfo.SelStart = Len(txtUserInfo.Text)
    End Select
End Sub

Private Sub DirectPlay8Event_SendComplete(dpnotify As DxVBLibA.DPNMSG_SEND_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_TerminateSession(dpnotify As DxVBLibA.DPNMSG_TERMINATE_SESSION, fRejectMsg As Boolean)
    If dpnotify.hResultCode = DPNERR_HOSTTERMINATEDSESSION Then
        MsgBox "The host has terminated this session.  This sample will now exit.", vbOKOnly Or vbInformation, "Exiting"
    Else
        MsgBox "This session has been lost.  This sample will now exit.", vbOKOnly Or vbInformation, "Exiting"
    End If
    DPlayEventsForm.CloseForm Me
End Sub
