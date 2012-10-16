Attribute VB_Name = "modDplay"
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       modDplay.bas
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Public Enum vbDplayChatMsgType
    MsgChat
    MsgWhisper
End Enum

'Constants
Public Const AppGuid = "{EABD4D9B-6AA9-4c24-9D10-1A6701B07342}"

Public dx As DirectX8
Public dpp As DirectPlay8Peer

'App specific variables
Public gsUserName As String
'Our connection form and message pump
Public DPlayEventsForm As DPlayConnect

Public Sub InitDPlay()
    'Create our DX/DirectPlay objects
    Set dx = New DirectX8
    Set dpp = dx.DirectPlayPeerCreate
End Sub

Public Sub Cleanup()
    If Not (DPlayEventsForm Is Nothing) Then
        If Not (dpp Is Nothing) Then dpp.UnRegisterMessageHandler
        'Get rid of our message pump
        DPlayEventsForm.GoUnload
        'Close down our session
        If Not (dpp Is Nothing) Then dpp.Close
        DPlayEventsForm.DoSleep 50
        'Lose references to peer and dx objects
        Set dpp = Nothing
        Set dx = Nothing
    End If
    Set DPlayEventsForm = Nothing
End Sub


