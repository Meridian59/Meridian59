Attribute VB_Name = "modDPlayClient"
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       modDplayClient.bas
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'Sleep declare
Private Declare Sub Sleep Lib "kernel32" (ByVal dwMilliseconds As Long)
'Constants for the app
Public Const gsAppName As String = "vbMessengerClient"

'Public vars for the app
Public dx As DirectX8
Public dpc As DirectPlay8Client 'Client object
Public dpa As DirectPlay8Address 'Local address
Public dpas As DirectPlay8Address 'Host address
Public gsUserName As String
Public gsPass As String
Public gsServerName As String
Public gfConnected As Boolean
Public gfCreatePlayer As Boolean
Public gfLoggedIn As Boolean
Public gofrmClient As frmClient

Public Sub InitDPlay()

    Cleanup 'Just in case
    Set dx = New DirectX8
    Set dpc = dx.DirectPlayClientCreate 'Create the client object
    Set dpa = dx.DirectPlayAddressCreate 'Create an address
    Set dpas = dx.DirectPlayAddressCreate 'Create the servers address object
    
    dpc.RegisterMessageHandler gofrmClient
    
    'Set up the local address
    dpa.SetSP DP8SP_TCPIP
    
    'Set up the host address
    dpas.SetSP DP8SP_TCPIP
    dpas.AddComponentLong DPN_KEY_PORT, glDefaultPort
    
End Sub

Public Sub Cleanup()

    'Close may return DPNERR_UNINITIALIZED if we've already logged off, and we don't
    'care, so lets ignore errors here.
    On Error Resume Next
    'Shut down our message handler
    If Not dpc Is Nothing Then dpc.UnRegisterMessageHandler
    'Close down our session
    If Not dpc Is Nothing Then dpc.Close
    Sleep 50 'Lets wait a small portion of time
    DoEvents
    Set dpc = Nothing
    Set dpa = Nothing
    Set dpas = Nothing
    Set dx = Nothing
    
End Sub

Public Sub LogonPlayer()
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    
    'The connect call has been completed.  Now we can send over our logon information
    lOffset = NewBuffer(oBuf)
    lMsg = Msg_Login
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    AddStringToBuffer oBuf, gsUserName, lOffset
    AddStringToBuffer oBuf, gsPass, lOffset
    'Send the information
    dpc.Send oBuf, 0, 0

End Sub

Public Sub CreatePlayer()
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    
    'The connect call has been completed.  Now we can send over our logon information
    lOffset = NewBuffer(oBuf)
    lMsg = Msg_CreateNewAccount
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    AddStringToBuffer oBuf, gsUserName, lOffset
    AddStringToBuffer oBuf, gsPass, lOffset
    'Send the information
    dpc.Send oBuf, 0, 0

End Sub

Public Sub AddFriend(ByVal sFriend As String)
    
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    
    'Go ahead and add our friend
    lOffset = NewBuffer(oBuf)
    lMsg = Msg_AddFriend
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    AddStringToBuffer oBuf, sFriend, lOffset
    'Send the information
    dpc.Send oBuf, 0, 0

End Sub

Public Sub BlockUser(ByVal sFriend As String)
    
    Dim lMsg As Long, lOffset As Long
    Dim oBuf() As Byte
    
    'Go ahead and add our friend
    lOffset = NewBuffer(oBuf)
    lMsg = Msg_BlockFriend
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    AddStringToBuffer oBuf, sFriend, lOffset
    'Send the information
    dpc.Send oBuf, 0, 0

End Sub

