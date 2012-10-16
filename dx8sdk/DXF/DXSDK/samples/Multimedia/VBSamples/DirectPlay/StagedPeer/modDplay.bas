Attribute VB_Name = "modDplay"
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       modDPlay.bas
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Public Enum vbDPlayStagerMsgType
    MsgChat
    MsgClickReady
    MsgOpenSlot
    MsgCloseSlot
    MsgKickUser
    MsgStartGame
    MsgPutPlayerInSlot
    MsgMakeFace
End Enum

'Constants
Public Const AppGuid = "{D66EC208-1BF0-48cb-AB20-18C321F31E1E}"

Public dx As DirectX8
Public dpp As DirectPlay8Peer

'App specific variables
Public gsUserName As String
Public glMyPlayerID As Long

'Our connection form and message pump
Public DPlayEventsForm As DPlayConnect

Public Sub InitDPlay()
    'Create our DX/DirectPlay objects
    Set dx = New DirectX8
    Set dpp = dx.DirectPlayPeerCreate
End Sub

Public Sub Cleanup()
    If Not (DPlayEventsForm Is Nothing) Then
        dpp.UnRegisterMessageHandler
        'Close down our session
        If Not (dpp Is Nothing) Then dpp.Close
        'Lose references to peer and dx objects
        DPlayEventsForm.DoSleep 50
        Set dpp = Nothing
        Set dx = Nothing
        'Get rid of our message pump
        DPlayEventsForm.GoUnload
    End If
End Sub

Public Sub UpdateChat(ByVal sString As String, oText As TextBox)
    'Update the window first
    oText.Text = oText.Text & sString & vbCrLf
    'Now limit the text in the window to be 16k
    If Len(oText.Text) > 16384 Then
        oText.Text = Right$(oText.Text, 16384)
    End If
    'Autoscroll the text
    oText.SelStart = Len(oText.Text)
End Sub

