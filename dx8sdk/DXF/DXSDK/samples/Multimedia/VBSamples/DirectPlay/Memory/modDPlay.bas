Attribute VB_Name = "modDPlay"
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       modDplay.bas
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

'Here are our msgs
Public Enum Const_MemoryMsgs
    MSG_SETUPBOARD
    MSG_SHOWPIECE
    MSG_HIDEPIECES
    MSG_TURNEND
    MSG_CHAT
    MSG_MATCHED
End Enum

'Constants
Public Const NumCells = 36
Public Const MaxPlayers = 4
Public Const AppGuid = "{31368C80-341E-4365-BD80-66D203D367BE}"

Public dx As DirectX8
Public dpp As DirectPlay8Peer

'App vars
Public gbNumPlayers As Byte
Public gsUserName As String
Public gbPicArray(NumCells) As Byte
Public gfMatchedCells(NumCells) As Boolean
Public glCurrentPlayer As Long
Public gbPlayerScores(MaxPlayers) As Byte
Public glPlayerIDs(MaxPlayers) As Long      ' Indexed by order of play
Public glMyPlayerID As Long
Public gfHost As Boolean

Public DPlayEventsForm As DPlayConnect

Public Sub InitDPlay()
    Set dx = New DirectX8
    Set dpp = dx.DirectPlayPeerCreate
End Sub

Public Sub Cleanup()
    'Terminate our session if there is one
    gbNumPlayers = 0
    If Not (DPlayEventsForm Is Nothing) Then
        'Turn off our message handler
        If Not (dpp Is Nothing) Then dpp.UnRegisterMessageHandler
        'Close down our session
        If Not (dpp Is Nothing) Then dpp.Close
        'Lose references to peer and dx objects
        Set dpp = Nothing
        Set dx = Nothing
        'Get rid of our message pump
        DPlayEventsForm.GoUnload
    End If
End Sub

' Assign pieces to cells and initialize the state. Done only by the host
' in the multiplayer game. In single-player, can be called to restart.

Public Sub SetupBoard()

Dim lCount As Integer
Dim Pic As Integer
Dim PicInstance As Integer
Dim RandCell As Integer

    ' Empty the image index array
    For lCount = 0 To NumCells - 1
        gbPicArray(lCount) = 0
    Next lCount
    
    ' Assign pictures to cells
    
    ' For every picture except #0, find two empty cells. The two leftover cells
    ' have picture #0 by default.
    ' PicArray indexes the play cells into the image cells stored on the invisible form.
    
    For Pic = 1 To NumCells \ 2 - 1
        For PicInstance = 1 To 2
            Randomize
            Do
                RandCell = Fix(Rnd * NumCells)
            Loop Until gbPicArray(RandCell) = 0
            gbPicArray(RandCell) = Pic
        Next PicInstance
    Next Pic

End Sub

Public Sub SendMessage(oBuf() As Byte)
    dpp.SendTo DPNID_ALL_PLAYERS_GROUP, oBuf, 0, DPNSEND_NOLOOPBACK Or DPNSEND_GUARANTEED
End Sub
