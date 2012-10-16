Attribute VB_Name = "modDPlayServer"
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       modDPlayServer.bas
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

'Public vars for the app
Public dx As New DirectX8
Public dps As DirectPlay8Server
Public dpa As DirectPlay8Address
Public glNumPlayers As Long

Public Sub Main()
    If App.PrevInstance Then
        MsgBox "You can only run one instance of this server at a time.", vbOKOnly Or vbInformation, "Close other instance"
        Exit Sub
    End If
    'Set up the default DPlay objects
    InitDPlay
    'Show the form (which will start the server)
    frmServer.Show
End Sub

Public Sub InitDPlay()

    Set dps = dx.DirectPlayServerCreate
    Set dpa = dx.DirectPlayAddressCreate
    
End Sub

Public Sub Cleanup()

    'Shut down our message handler
    If Not dps Is Nothing Then dps.UnRegisterMessageHandler
    'Close down our session
    If Not dps Is Nothing Then dps.Close
    Set dps = Nothing
    Set dpa = Nothing
    Set dx = Nothing
    
End Sub

'Send a message to a player
Public Function SendMessage(ByVal sUser As String, ByVal sFrom As String, ByVal sChat As String) As Boolean

    Dim lSendID As Long, lMsg As Long
    Dim oBuf() As Byte, lOffset As Long
    
    'Before we send this message check to see if this user is blocked
    If AmIBlocked(sUser, sFrom) Then
        lSendID = GetCurrentDPlayID(sFrom)
        lMsg = Msg_UserBlocked
        lOffset = NewBuffer(oBuf)
        AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
        AddStringToBuffer oBuf, sUser, lOffset
        dps.SendTo lSendID, oBuf, 0, 0
    Else
        lSendID = GetCurrentDPlayID(sUser)
        If lSendID = 0 Then 'This person isn't logged on
            lSendID = GetCurrentDPlayID(sFrom)
            lMsg = Msg_UserUnavailable
            lOffset = NewBuffer(oBuf)
            AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
            AddStringToBuffer oBuf, sUser, lOffset
            AddStringToBuffer oBuf, sChat, lOffset
        Else
            lMsg = Msg_ReceiveMessage
            lOffset = NewBuffer(oBuf)
            AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
            AddStringToBuffer oBuf, sFrom, lOffset
            AddStringToBuffer oBuf, sChat, lOffset
        End If
        dps.SendTo lSendID, oBuf, 0, 0
    End If
    SendMessage = True

End Function
