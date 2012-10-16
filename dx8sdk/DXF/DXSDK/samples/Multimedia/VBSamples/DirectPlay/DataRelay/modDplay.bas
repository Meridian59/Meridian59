Attribute VB_Name = "modDplay"
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       modDplay.bas
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'Sleep declare
Private Declare Sub Sleep Lib "kernel32" (ByVal dwMilliseconds As Long)

Public Enum vbDplayDataRelayMsgType
    MSG_GamePacket
    MSG_PacketReceive
End Enum

'Constants
Public Const AppGuid = "{0A947595-45D1-48f0-AEE2-E7CF851A1EEE}"

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
        'Get rid of our message pump
        DPlayEventsForm.GoUnload
        dpp.UnRegisterMessageHandler
        'Close down our session
        DPlayEventsForm.DoSleep 50
        If Not (dpp Is Nothing) Then dpp.Close
        'Lose references to peer and dx objects
        Set dpp = Nothing
        Set dx = Nothing
    End If
End Sub



