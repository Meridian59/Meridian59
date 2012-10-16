Attribute VB_Name = "modDplay"
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       modDPlay.bas
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

'Constants
Public Const AppGuid = "{F5230441-9B71-88DA-998C-00207547A14C}"

'Types
Public Type PlayerInfo
    lPlayerID As Long
    fSilent As Boolean
End Type

'DirectX Variables
Public dvServer As DirectPlayVoiceServer8
Public dvClient As DirectPlayVoiceClient8
Public dx As DirectX8
Public dpp As DirectPlay8Peer
Public glMyID As Long
'Our connection form and message pump
Public DPlayEventsForm As DPlayConnect

'Misc Vars
Public fAmHost As Boolean

' Get the DirectPlay objects
Public Sub InitDPlay()
    Set dx = New DirectX8
    Set dpp = dx.DirectPlayPeerCreate
End Sub
' Shut down the DPlay objects
Public Sub Cleanup()
    
    On Error Resume Next
    'Turn off our error handling
    If Not (dpp Is Nothing) Then dpp.UnRegisterMessageHandler
    If Not (dvClient Is Nothing) Then dvClient.UnRegisterMessageHandler
    If Not (dvServer Is Nothing) Then dvServer.UnRegisterMessageHandler
    dvClient.Disconnect DVFLAGS_SYNC
    If fAmHost Then dvServer.StopSession 0
    'Destroy the objects
    Set dvClient = Nothing
    Set dvServer = Nothing
    
    If Not dpp Is Nothing Then dpp.Close
    DPlayEventsForm.GoUnload
    'Destroy the objects
    Set dpp = Nothing
    Set dx = Nothing
End Sub
