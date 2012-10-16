VERSION 5.00
Begin VB.Form frmWhiteBoard 
   Caption         =   "Whiteboard"
   ClientHeight    =   7200
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   9600
   Icon            =   "frmWhiteBoard.frx":0000
   LinkTopic       =   "Form1"
   ScaleHeight     =   7200
   ScaleWidth      =   9600
   StartUpPosition =   3  'Windows Default
   Begin VB.PictureBox picDraw 
      AutoRedraw      =   -1  'True
      BackColor       =   &H00FFFFFF&
      Height          =   7155
      Left            =   0
      ScaleHeight     =   7095
      ScaleWidth      =   9495
      TabIndex        =   0
      Top             =   0
      Width           =   9555
   End
   Begin VB.Menu Pop 
      Caption         =   "mnuPop"
      Visible         =   0   'False
      Begin VB.Menu mnuRed 
         Caption         =   "Draw with Red"
      End
      Begin VB.Menu mnuBlue 
         Caption         =   "Draw with Blue"
      End
      Begin VB.Menu mnuGreen 
         Caption         =   "Draw with Green"
      End
      Begin VB.Menu mnuGrey 
         Caption         =   "Draw with Grey"
      End
      Begin VB.Menu mnuPurp 
         Caption         =   "Draw with Purple"
      End
      Begin VB.Menu mnuYellow 
         Caption         =   "Draw with Yellow"
      End
      Begin VB.Menu mnuSep 
         Caption         =   "-"
      End
      Begin VB.Menu mnuClear 
         Caption         =   "Clear Board"
      End
   End
End
Attribute VB_Name = "frmWhiteBoard"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmWhiteBoard.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Implements DirectPlay8Event
Private mlColor As Long
Private mlLastX As Single: Private mlLastY As Single

Private Sub Form_Resize()
    picDraw.Move 0, 0, Me.Width, Me.Height
End Sub

Private Sub mnuBlue_Click()
    mlColor = RGB(0, 0, 255)
End Sub

Private Sub mnuClear_Click()
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    picDraw.Cls
    'Send the clear msg
    lOffset = NewBuffer(oBuf)
    lMsg = MsgClearWhiteBoard
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_NOLOOPBACK
End Sub

Private Sub mnuGreen_Click()
    mlColor = RGB(0, 255, 0)
End Sub

Private Sub mnuGrey_Click()
    mlColor = RGB(128, 128, 128)
End Sub

Private Sub mnuPurp_Click()
    mlColor = RGB(156, 56, 167)
End Sub

Private Sub mnuRed_Click()
    mlColor = RGB(255, 0, 0)
End Sub

Private Sub mnuYellow_Click()
    mlColor = RGB(255, 255, 0)
End Sub

Private Sub picDraw_MouseDown(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    If Button = vbLeftButton Then 'We are drawing
        If mlColor = 0 Then mlColor = RGB(255, 0, 0)
        'First draw the dot
        picDraw.PSet (X, Y), mlColor
        'Now tell everyone about it
        
        'Now let's send a message to draw this dot
        lOffset = NewBuffer(oBuf)
        lMsg = MsgSendDrawPixel
        AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
        AddDataToBuffer oBuf, mlColor, LenB(mlColor), lOffset
        AddDataToBuffer oBuf, X, SIZE_SINGLE, lOffset
        AddDataToBuffer oBuf, Y, SIZE_SINGLE, lOffset
        dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_NOLOOPBACK
        'Now store the last x/y
        mlLastX = X: mlLastY = Y
    End If
End Sub

Private Sub picDraw_MouseMove(Button As Integer, Shift As Integer, X As Single, Y As Single)
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    If Button = vbLeftButton Then 'We are drawing
        If mlColor = 0 Then mlColor = RGB(255, 0, 0)
        'First draw the dot
        picDraw.Line (mlLastX, mlLastY)-(X, Y), mlColor
        'Now tell everyone about it
        
        'Now let's send a message to draw this line
        lOffset = NewBuffer(oBuf)
        lMsg = MsgSendDrawLine
        AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
        AddDataToBuffer oBuf, mlColor, LenB(mlColor), lOffset
        AddDataToBuffer oBuf, mlLastX, SIZE_SINGLE, lOffset
        AddDataToBuffer oBuf, mlLastY, SIZE_SINGLE, lOffset
        AddDataToBuffer oBuf, X, SIZE_SINGLE, lOffset
        AddDataToBuffer oBuf, Y, SIZE_SINGLE, lOffset
        dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_NOLOOPBACK
        'Now store the last x/y
        mlLastX = X: mlLastY = Y
    End If
End Sub

Private Sub picDraw_MouseUp(Button As Integer, Shift As Integer, X As Single, Y As Single)
    If Button = vbRightButton Then
        PopupMenu Pop
    End If
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
    'All we care about in this form is what msgs we receive.
    Dim lMsg As Long, lOffset As Long
    Dim lColor As Long
    Dim lX As Single, lY As Single
    Dim lX1 As Single, lY1 As Single
    
    With dpnotify
    GetDataFromBuffer .ReceivedData, lMsg, LenB(lMsg), lOffset
    Select Case lMsg
    Case MsgSendDrawPixel
        GetDataFromBuffer .ReceivedData, lColor, LenB(lColor), lOffset
        GetDataFromBuffer .ReceivedData, lX, LenB(lX), lOffset
        GetDataFromBuffer .ReceivedData, lY, LenB(lY), lOffset
        On Error Resume Next
        picDraw.PSet (lX, lY), lColor
    Case MsgSendDrawLine
        GetDataFromBuffer .ReceivedData, lColor, LenB(lColor), lOffset
        GetDataFromBuffer .ReceivedData, lX, LenB(lX), lOffset
        GetDataFromBuffer .ReceivedData, lY, LenB(lY), lOffset
        GetDataFromBuffer .ReceivedData, lX1, LenB(lX), lOffset
        GetDataFromBuffer .ReceivedData, lY1, LenB(lY), lOffset
        On Error Resume Next
        picDraw.Line (lX, lY)-(lX1, lY1), lColor
    Case MsgClearWhiteBoard
        picDraw.Cls
    End Select
    End With
End Sub

Private Sub DirectPlay8Event_SendComplete(dpnotify As DxVBLibA.DPNMSG_SEND_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_TerminateSession(dpnotify As DxVBLibA.DPNMSG_TERMINATE_SESSION, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub
