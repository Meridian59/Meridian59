Attribute VB_Name = "DplayModule"
Option Explicit

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       dplay.bas
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Public Enum DPLAY_MSGS
    MSG_CHANGEGROUP
    MSG_CHANGETALK
    MSG_SERVERCHANGEGROUP
End Enum

'Constants
Public Const AppGuid = "{F5230441-9B71-88DA-998C-00207547A14D}"

'DirectX Variables
Public dvServer As DirectPlayVoiceServer8
Public dvClient As DirectPlayVoiceClient8
Public dx As DirectX8
Public dpp As DirectPlay8Peer
Public oSession As DVSESSIONDESC
Public oSound As DVSOUNDDEVICECONFIG
Public oClient As DVCLIENTCONFIG

Public glGroupID(1 To 5) As Long

'Misc Vars
Public glMyPlayerID As Long
Public fGotSettings As Boolean

Public DPlayEventsForm As DPlayConnect

Public Sub InitDPlay()
    Set dx = New DirectX8
    Set dpp = dx.DirectPlayPeerCreate
    Set dvServer = dx.DirectPlayVoiceServerCreate
    Set dvClient = dx.DirectPlayVoiceClientCreate
End Sub

Public Sub Cleanup()
    
    On Error Resume Next
    'Turn off our error handling
    If Not (DPlayEventsForm Is Nothing) Then
        If Not (dpp Is Nothing) Then dpp.UnRegisterMessageHandler
        If Not (dvClient Is Nothing) Then dvClient.UnRegisterMessageHandler
        If Not (dvServer Is Nothing) Then dvServer.UnRegisterMessageHandler
        dvClient.Disconnect 0
        DPlayEventsForm.DoSleep 50
        If DPlayEventsForm.IsHost Then dvServer.StopSession 0
        
        If Not dpp Is Nothing Then dpp.Close
        DPlayEventsForm.GoUnload
        'Destroy the objects
        Set dvClient = Nothing
        Set dvServer = Nothing
        Set dpp = Nothing
        Set dx = Nothing
    End If
End Sub
    
Private Sub Main()
    'Here is where we will start
    InitDPlay
    
    Set DPlayEventsForm = New DPlayConnect
    If Not DPlayEventsForm.StartConnectWizard(dx, dpp, AppGuid, 20) Then
        Cleanup
    Else 'We did choose to play a game
        If Not (DPlayEventsForm.IsHost) Then frmVoiceSettings.ClientOnly
        frmVoiceSettings.Show vbModal
        If Not fGotSettings Then 'We quit for some unknown reason.
            Cleanup
            Exit Sub
        End If
        frmVoice.Show vbModeless
        If DPlayEventsForm.IsHost Then frmVoice.Caption = frmVoice.Caption & " (HOST)"
    End If
    
End Sub

Public Sub RemovePlayerFromAllGroups(lPlayerID As Long)
    On Error Resume Next 'We don't care about any errors..
    Dim lCount As Long
    
    For lCount = 1 To 5
        dpp.RemovePlayerFromGroup glGroupID(lCount), lPlayerID, 0
    Next
    Err.Clear
    'Ignore the errors about Player not in group
End Sub

