Attribute VB_Name = "modDPlay"
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       modDPlay.bas
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

' Win32 declares
Public Declare Function GetUserName Lib "advapi32.dll" Alias "GetUserNameA" (ByVal lpBuffer As String, nSize As Long) As Long

'Constants
Public Const AppGuid = "{74377695-900D-4fdb-98F7-AC6BCAD2C631}"

Public dx As DirectX8
Public dpp As DirectPlay8Peer
Public glNumPlayers As Long

Public gfHost As Boolean
Public DPlayEventsForm As DPlayConnect

Public Sub InitDPlay()
    Set dx = New DirectX8
    Set dpp = dx.DirectPlayPeerCreate
End Sub

Public Sub Cleanup()
    If Not dpp Is Nothing Then dpp.UnRegisterMessageHandler 'Stop taking messages
    'Close our peer connection
    If Not dpp Is Nothing Then dpp.Close
    'Lose references to peer and dx objects
    Set dpp = Nothing
    Set dx = Nothing
    DPlayEventsForm.GoUnload
End Sub

