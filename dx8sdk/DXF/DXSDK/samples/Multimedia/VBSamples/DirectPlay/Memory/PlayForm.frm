VERSION 5.00
Begin VB.Form frmGameBoard 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "DirectPlay Memory"
   ClientHeight    =   7200
   ClientLeft      =   3150
   ClientTop       =   2400
   ClientWidth     =   8745
   Icon            =   "PlayForm.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   ScaleHeight     =   480
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   583
   StartUpPosition =   2  'CenterScreen
   Begin VB.Timer tmrTerminate 
      Enabled         =   0   'False
      Interval        =   10
      Left            =   8985
      Top             =   1680
   End
   Begin VB.Timer tmrResign 
      Enabled         =   0   'False
      Interval        =   10
      Left            =   8985
      Top             =   1200
   End
   Begin VB.CommandButton cmdExit 
      Cancel          =   -1  'True
      Caption         =   "E&xit"
      BeginProperty Font 
         Name            =   "Verdana"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   615
      Left            =   6720
      TabIndex        =   9
      Top             =   1740
      Visible         =   0   'False
      Width           =   1995
   End
   Begin VB.Frame Frame1 
      BeginProperty Font 
         Name            =   "Verdana"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1455
      Index           =   1
      Left            =   6720
      TabIndex        =   3
      Top             =   1760
      Width           =   1935
      Begin VB.Label LabelScore 
         Alignment       =   2  'Center
         Caption         =   "0"
         BeginProperty Font 
            Name            =   "Verdana"
            Size            =   36
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   975
         Index           =   1
         Left            =   120
         TabIndex        =   5
         Top             =   360
         Width           =   1695
      End
   End
   Begin VB.Frame Frame1 
      BeginProperty Font 
         Name            =   "Verdana"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1455
      Index           =   2
      Left            =   6720
      TabIndex        =   2
      Top             =   3400
      Width           =   1935
      Begin VB.Label LabelScore 
         Alignment       =   2  'Center
         Caption         =   "0"
         BeginProperty Font 
            Name            =   "Verdana"
            Size            =   36
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   975
         Index           =   2
         Left            =   120
         TabIndex        =   6
         Top             =   360
         Width           =   1695
      End
   End
   Begin VB.Frame Frame1 
      BeginProperty Font 
         Name            =   "Verdana"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1455
      Index           =   3
      Left            =   6720
      TabIndex        =   1
      Top             =   5040
      Width           =   1935
      Begin VB.Label LabelScore 
         Alignment       =   2  'Center
         Caption         =   "0"
         BeginProperty Font 
            Name            =   "Verdana"
            Size            =   36
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   975
         Index           =   3
         Left            =   120
         TabIndex        =   7
         Top             =   360
         Width           =   1695
      End
   End
   Begin VB.Frame Frame1 
      Caption         =   "Turns"
      BeginProperty Font 
         Name            =   "Verdana"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1455
      Index           =   0
      Left            =   6720
      TabIndex        =   0
      Top             =   120
      Width           =   1935
      Begin VB.Label LabelScore 
         Alignment       =   2  'Center
         Caption         =   "0"
         BeginProperty Font 
            Name            =   "Verdana"
            Size            =   36
            Charset         =   0
            Weight          =   700
            Underline       =   0   'False
            Italic          =   0   'False
            Strikethrough   =   0   'False
         EndProperty
         Height          =   975
         Index           =   0
         Left            =   120
         TabIndex        =   4
         Top             =   360
         Width           =   1695
      End
   End
   Begin VB.Label lblChat 
      Caption         =   "Press Enter to chat, Alt+F4 to resign."
      BeginProperty Font 
         Name            =   "Verdana"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   570
      Left            =   105
      TabIndex        =   8
      Top             =   6570
      Width           =   8700
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   35
      Left            =   5520
      Stretch         =   -1  'True
      Top             =   5520
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   34
      Left            =   4440
      Stretch         =   -1  'True
      Top             =   5520
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   33
      Left            =   3360
      Stretch         =   -1  'True
      Top             =   5520
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   32
      Left            =   2280
      Stretch         =   -1  'True
      Top             =   5520
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   31
      Left            =   1200
      Stretch         =   -1  'True
      Top             =   5520
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   30
      Left            =   120
      Stretch         =   -1  'True
      Top             =   5520
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   29
      Left            =   5520
      Stretch         =   -1  'True
      Top             =   4440
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   28
      Left            =   4440
      Stretch         =   -1  'True
      Top             =   4440
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   27
      Left            =   3360
      Stretch         =   -1  'True
      Top             =   4440
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   26
      Left            =   2280
      Stretch         =   -1  'True
      Top             =   4440
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   25
      Left            =   1200
      Stretch         =   -1  'True
      Top             =   4440
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   24
      Left            =   120
      Stretch         =   -1  'True
      Top             =   4440
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   23
      Left            =   5520
      Stretch         =   -1  'True
      Top             =   3360
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   22
      Left            =   4440
      Stretch         =   -1  'True
      Top             =   3360
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   21
      Left            =   3360
      Stretch         =   -1  'True
      Top             =   3360
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   20
      Left            =   2280
      Stretch         =   -1  'True
      Top             =   3360
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   19
      Left            =   1200
      Stretch         =   -1  'True
      Top             =   3360
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   18
      Left            =   120
      Stretch         =   -1  'True
      Top             =   3360
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   17
      Left            =   5520
      Stretch         =   -1  'True
      Top             =   2280
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   16
      Left            =   4440
      Stretch         =   -1  'True
      Top             =   2280
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   15
      Left            =   3360
      Stretch         =   -1  'True
      Top             =   2280
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   14
      Left            =   2280
      Stretch         =   -1  'True
      Top             =   2280
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   13
      Left            =   1200
      Stretch         =   -1  'True
      Top             =   2280
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   12
      Left            =   120
      Stretch         =   -1  'True
      Top             =   2280
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   11
      Left            =   5520
      Stretch         =   -1  'True
      Top             =   1200
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   10
      Left            =   4440
      Stretch         =   -1  'True
      Top             =   1200
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   9
      Left            =   3360
      Stretch         =   -1  'True
      Top             =   1200
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   8
      Left            =   2280
      Stretch         =   -1  'True
      Top             =   1200
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   7
      Left            =   1200
      Stretch         =   -1  'True
      Top             =   1200
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   6
      Left            =   120
      Stretch         =   -1  'True
      Top             =   1200
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   5
      Left            =   5520
      Stretch         =   -1  'True
      Top             =   120
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   4
      Left            =   4440
      Stretch         =   -1  'True
      Top             =   120
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   3
      Left            =   3360
      Stretch         =   -1  'True
      Top             =   120
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   2
      Left            =   2280
      Stretch         =   -1  'True
      Top             =   120
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   1
      Left            =   1200
      Stretch         =   -1  'True
      Top             =   120
      Width           =   1005
   End
   Begin VB.Image Image1 
      BorderStyle     =   1  'Fixed Single
      Height          =   1005
      Index           =   0
      Left            =   120
      Stretch         =   -1  'True
      Top             =   120
      Width           =   1005
   End
End
Attribute VB_Name = "frmGameBoard"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       PlayForm.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Implements DirectPlay8Event
'Here is where all of the main gameplay will be taking place.
Private Const mlMaxText As Long = 50

'Keep track of what the first cell picked was
Private fFirstPick As Boolean
Private lFirstCell As Long
Private fGame As Boolean
Private lTurnCount As Long
Private mfResign As Boolean
Private mlTerminateCode As Long

Private Sub cmdExit_Click()
    'Game over, we wanna leave
    Unload Me
End Sub

' Keystroke handler
' Enter: open Chat dialog
Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
    Dim sMsg As String, lOffset As Long
    Dim oBuf() As Byte
    
    If (KeyCode = vbKeyReturn) And (gbNumPlayers > 1) Then
        'Lets chat
        sMsg = InputBox$("Enter the text you want to send:", "Chat Message")
        If sMsg = vbNullString Then Exit Sub
        If Len(sMsg) > mlMaxText Then
            sMsg = Left$(sMsg, mlMaxText)
        End If
        'Send our chat
        lOffset = NewBuffer(oBuf)
        AddDataToBuffer oBuf, CByte(MSG_CHAT), SIZE_BYTE, lOffset
        AddStringToBuffer oBuf, sMsg, lOffset
        SendMessage oBuf
    End If
End Sub

Private Sub Form_Load()

    ' Initialize scoreboard
    If gbNumPlayers > 1 Then DPlayEventsForm.RegisterCallback Me
    InitLocalGame
    ' Erase chat prompt if only one player.
    If gbNumPlayers = 1 Then
        lblChat.Caption = vbNullString
        cmdExit.Visible = True
        SetupBoard
    Else
        ' Put user name on caption bar to ease debugging of multiple sessions on one machine
        Me.Caption = Me.Caption & " - " & gsUserName
        If gfHost Then Me.Caption = Me.Caption & " (HOST) - Your turn"
    End If
End Sub


Private Sub Form_Unload(Cancel As Integer)
    mfResign = True
    If Not (DPlayEventsForm Is Nothing) Then DPlayEventsForm.DoSleep 50
    Cleanup
    frmIntro.Visible = True
    frmIntro.EnableButtons True
End Sub

' This is where the action takes place. In each turn the player clicks on two empty squares,
' making their pictures visible. The two pictures revealed in the previous turn are hidden
' as soon as the first square is clicked, unless they are a match. The player can click on
' an unmatched picture to begin the turn, in which case it remains visible.
' A message is broadcast whenever a square is shown or hidden.

Private Sub Image1_MouseDown(Index As Integer, Button As Integer, Shift As Integer, X As Single, Y As Single)
  
    Dim fGameOver As Boolean
    Dim lCount As Long, lOffset As Long
    Dim oBuf() As Byte
    
    ' Not your turn, bub.
    If gbNumPlayers > 1 Then If glPlayerIDs(glCurrentPlayer) <> glMyPlayerID Then Exit Sub
    
    If Button = vbLeftButton Then 'Button = Left
        ' If picture already showing and this is second pick, ignore click.
        ' If picture showing and is already one of a match, ignore click.
        If Image1(Index).Picture <> 0 And ((Not fFirstPick) Or gfMatchedCells(Index)) Then
            Exit Sub
        End If
    
        
        If fFirstPick Then ' First Pick
        ' Hide previous picks unless they were a match.
            For lCount = 0 To NumCells - 1
                If Not gfMatchedCells(lCount) Then 'Not Matched
                    Set Image1(lCount).Picture = Nothing
                End If 'Not Matched
            Next lCount
            ' Tell the other players to update the display. We don't specify which
            ' squares, but just tell them to hide unmatched squares.
            If gbNumPlayers > 1 Then 'NumPlayers > 1
                lOffset = NewBuffer(oBuf)
                AddDataToBuffer oBuf, CByte(MSG_HIDEPIECES), SIZE_BYTE, lOffset
                SendMessage oBuf
            End If 'NumPlayers > 1
            ' Remember this one
            lFirstCell = Index
            fFirstPick = False
            ShowPic Index
        Else
            ShowPic Index
            ' Second pick
            fFirstPick = True  ' Reset for next time
            ' In solitaire game, show number of turns as score
            If gbNumPlayers = 1 Then '1 Player?
                lTurnCount = lTurnCount + 1
                frmGameBoard.LabelScore(0).Caption = lTurnCount
            End If '1 Player?
    
            ' Check for match
            If gbPicArray(lFirstCell) = gbPicArray(Index) Then
                ' There was a match
                gfMatchedCells(Index) = True
                gfMatchedCells(lFirstCell) = True
    
                ' Check for win and increment score (# of matches)
                fGameOver = IsGameOver
                ' Increment score display only in multiplayer.
                ' For solitaire, the score is the turn count.
                If gbNumPlayers > 1 Then
                    'Update the scoreboard for multiplayer games
                    UpdateScoreboard
    
                    lOffset = NewBuffer(oBuf)
                    AddDataToBuffer oBuf, CByte(MSG_MATCHED), SIZE_BYTE, lOffset
                    'Get the array of matchings cells in
                    For lCount = 0 To NumCells - 1
                        AddDataToBuffer oBuf, gfMatchedCells(lCount), LenB(gfMatchedCells(lCount)), lOffset
                    Next
                    ' Get scores into message
                    For lCount = 0 To MaxPlayers - 1
                        AddDataToBuffer oBuf, gbPlayerScores(lCount), LenB(gbPlayerScores(lCount)), lOffset
                    Next
                    SendMessage oBuf
                End If ' DirectPlay exists
            Else
                ' There was no match.
                ' Broadcast turn-end message
            
                If gbNumPlayers > 1 Then
                    lOffset = NewBuffer(oBuf)
                    AddDataToBuffer oBuf, CByte(MSG_TURNEND), SIZE_BYTE, lOffset
                    SendMessage oBuf
            
                    ' Pass control to next player & advance scoreboard highlight
                    AdvanceTurn
                End If  'More than one player
            
            End If ' match or no match
            
            ' If solitaire win, offer choice to play again
            If fGameOver And gbNumPlayers = 1 Then
                If MsgBox("Play again?", vbYesNo, "Game Over") = vbNo Then End
                SetupBoard
                InitLocalGame
            End If
        End If
    End If

End Sub


' Update scores and check for win

Public Function IsGameOver() As Boolean
    
    Dim lCount As Integer, Response As Integer
    Dim fEnd As Boolean
    
    gbPlayerScores(glCurrentPlayer) = gbPlayerScores(glCurrentPlayer) + 1
    
    ' If any cells are still blank, game is not over
    fEnd = True
    For lCount = 0 To NumCells - 1
        If Not gfMatchedCells(lCount) Then
            fEnd = False
        End If
    Next lCount
    IsGameOver = fEnd
    
End Function

' Game initialization for all players, including setting up the scoreboard for the
' current number and order of players. Global game initialization (setting up the pieces)
' is handled by the host through SetupBoard.

Public Sub InitLocalGame()

    Dim lCount As Integer
    Dim PlayerInfo As DPN_PLAYER_INFO
    
    fFirstPick = True
    lTurnCount = 0
    
    ' Highlight current player
    glCurrentPlayer = 0
    Frame1(glCurrentPlayer).ForeColor = vbHighlight
    LabelScore(glCurrentPlayer).ForeColor = vbHighlight
  
    ' Hide superfluous scoreboxes and initialize scores
    For lCount = 0 To MaxPlayers - 1
        gbPlayerScores(lCount) = 0
        If lCount >= gbNumPlayers Then
            Frame1(lCount).Visible = False
        Else
            Frame1(lCount).Visible = True
            LabelScore(lCount).Caption = 0
        End If
    Next lCount
   
    ' Get names of players and label scoreboxes. The correct order has been
    ' stored in the gPlayerIDs array, which is initialized by the host
    ' and passed to the other players.
    If gbNumPlayers > 1 Then
        For lCount = 0 To gbNumPlayers - 1
            PlayerInfo = dpp.GetPeerInfo(glPlayerIDs(lCount))
            Frame1(lCount).Caption = PlayerInfo.Name
            Frame1(lCount).Tag = glPlayerIDs(lCount)
            If PlayerInfo.lPlayerFlags And DPNPLAYER_LOCAL Then
                glMyPlayerID = glPlayerIDs(lCount)
            End If
        Next lCount
    End If
    
    ' Erase the pictures and matches
    For lCount = 0 To NumCells - 1
        Image1(lCount).Picture = Nothing
        gfMatchedCells(lCount) = False
    Next lCount

End Sub

Private Sub tmrResign_Timer()
    tmrResign.Enabled = False
    MsgBox "All other players have resigned.  You win!", vbOKOnly Or vbInformation, "Winner"
    DPlayEventsForm.CloseForm Me
End Sub

Public Sub UpdateScoreboard()

    Dim lCount As Integer

    For lCount = 0 To MaxPlayers - 1
      LabelScore(lCount).Caption = gbPlayerScores(lCount)
    Next lCount

End Sub

Private Sub UpdateChat(ByVal sText As String, sUser As String)
    'We need to update the chat window
    lblChat.Caption = sUser & " says: " & sText
End Sub

Public Sub AdvanceTurn()
  
    If Me.Visible Then
        ' Remove highlight from scorebox for last player
        Frame1(glCurrentPlayer).ForeColor = vbButtonText
        LabelScore(glCurrentPlayer).ForeColor = vbButtonText
    End If
    
    ' Advance the current player. Try till we find one that exists.
    ' Players who resigned are now 0 in gPlayerIDs.
    
    Do
        glCurrentPlayer = glCurrentPlayer + 1
        If glCurrentPlayer = MaxPlayers Then glCurrentPlayer = 0
    Loop Until glPlayerIDs(glCurrentPlayer) <> 0
    
    If Me.Visible Then
        ' Highlight scorebox for active player
        Frame1(glCurrentPlayer).ForeColor = vbHighlight
        LabelScore(glCurrentPlayer).ForeColor = vbHighlight
        UpdateScoreboard
    End If
    Me.Caption = "DirectPlay Memory - " & gsUserName
    If gfHost Then Me.Caption = Me.Caption & " (HOST)"
    If glPlayerIDs(glCurrentPlayer) = glMyPlayerID Then
        Me.Caption = Me.Caption & " - Your turn"
    End If
    
End Sub

Private Sub ShowPic(ByVal Index As Integer)
    Dim oBuf() As Byte, lOffset As Long
    ' Show the picture you clicked on
    Image1(Index).Picture = frmPics.Image1(gbPicArray(Index)).Picture
    ' Broadcast message to show picture
    If gbNumPlayers > 1 Then 'NumPlayers > 1
        lOffset = NewBuffer(oBuf)
        AddDataToBuffer oBuf, CByte(MSG_SHOWPIECE), SIZE_BYTE, lOffset
        AddDataToBuffer oBuf, CByte(Index), SIZE_BYTE, lOffset
        SendMessage oBuf
    End If 'NumPlayers > 1
End Sub

Private Sub tmrTerminate_Timer()
    tmrTerminate.Enabled = False
    If mfResign Then Exit Sub
    If mlTerminateCode = DPNERR_HOSTTERMINATEDSESSION Then
        MsgBox "The host has terminated this session.  This sample will now exit.", vbOKOnly Or vbInformation, "Exiting"
    Else
        MsgBox "This session has been lost.  This sample will now exit.", vbOKOnly Or vbInformation, "Exiting"
    End If
    DPlayEventsForm.CloseForm Me
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
    gbNumPlayers = gbNumPlayers + 1
    If gbNumPlayers = 1 And mfResign = False Then 'Everyone has resigned, you win!
        tmrResign.Enabled = True
    End If
    ' If current player quit, advance to next
    If glPlayerIDs(glCurrentPlayer) = lPlayerID Then AdvanceTurn
End Sub

Private Sub DirectPlay8Event_DestroyGroup(ByVal lGroupID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_DestroyPlayer(ByVal lPlayerID As Long, ByVal lReason As Long, fRejectMsg As Boolean)
    Dim lCount As Long
    Dim fAdvance As Boolean
    
    On Error Resume Next
    gbNumPlayers = gbNumPlayers - 1
    If gbNumPlayers = 1 And mfResign = False Then 'Everyone has resigned, you win!
        tmrResign.Enabled = True
    End If
    ' If current player quit, advance to next
    If glPlayerIDs(glCurrentPlayer) = lPlayerID Then fAdvance = True
    'Remove this player ID from the list of users
    If gbNumPlayers > 1 Then
        For lCount = 0 To gbNumPlayers + 1
            If Frame1(lCount).Tag = lPlayerID Then
                Frame1(lCount).Visible = False
            End If
            'Remove this player ID from the list of users
            If glPlayerIDs(lCount) = lPlayerID Then glPlayerIDs(lCount) = 0
        Next lCount
    End If
    
    If fAdvance Then AdvanceTurn
End Sub

Private Sub DirectPlay8Event_EnumHostsQuery(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_QUERY, fRejectMsg As Boolean)
    'We don't want anyone to see this game once it's started... Disallow it.
    fRejectMsg = True
End Sub

Private Sub DirectPlay8Event_EnumHostsResponse(dpnotify As DxVBLibA.DPNMSG_ENUM_HOSTS_RESPONSE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_HostMigrate(ByVal lNewHostID As Long, fRejectMsg As Boolean)
    If lNewHostID = glMyPlayerID Then gfHost = True
End Sub

Private Sub DirectPlay8Event_IndicateConnect(dpnotify As DxVBLibA.DPNMSG_INDICATE_CONNECT, fRejectMsg As Boolean)
    'We don't want anyone connecting while we're already playing the game.. Disallow it.
    fRejectMsg = True
End Sub

Private Sub DirectPlay8Event_IndicatedConnectAborted(fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_InfoNotify(ByVal lMsgID As Long, ByVal lNotifyID As Long, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_Receive(dpnotify As DxVBLibA.DPNMSG_RECEIVE, fRejectMsg As Boolean)
    
    Dim lCount As Long, lOffset As Long
    Dim bMsg As Byte
    Dim bPiece As Byte, fMatched As Boolean, bScore As Byte
    Dim sChat As String, sPlayer As String
    
    'Here we will go through the messages
    'The first item in our byte array is the MSGID we passed in
    With dpnotify
    GetDataFromBuffer .ReceivedData, bMsg, LenB(bMsg), lOffset
    Select Case bMsg
    Case MSG_SHOWPIECE
        ' Show a tile that has been clicked
        GetDataFromBuffer .ReceivedData, bPiece, LenB(bPiece), lOffset
        frmGameBoard.Image1(bPiece).Picture = frmPics.Image1(gbPicArray(bPiece)).Picture
      
    Case MSG_HIDEPIECES
        ' Hide unmatched pieces because player has made the first pick.
        For lCount = 0 To NumCells - 1
            If Not gfMatchedCells(lCount) Then
                Image1(lCount).Picture = Nothing
            End If
        Next lCount
    
    Case MSG_MATCHED
    ' Retrieve matched cells array
        For lCount = 0 To NumCells - 1
            GetDataFromBuffer .ReceivedData, fMatched, LenB(fMatched), lOffset
            gfMatchedCells(lCount) = fMatched
        Next lCount
        
        ' Retrieve player scores array
        For lCount = 0 To MaxPlayers - 1
            GetDataFromBuffer .ReceivedData, bScore, LenB(bScore), lOffset
            gbPlayerScores(lCount) = bScore
        Next lCount
        ' Display current score
        frmGameBoard.UpdateScoreboard
    
    Case MSG_TURNEND
        AdvanceTurn
    
    Case MSG_CHAT
    ' Display chat message
        sPlayer = dpp.GetPeerInfo(dpnotify.idSender).Name
        sChat = GetStringFromBuffer(.ReceivedData, lOffset)
        UpdateChat sChat, sPlayer
    End Select
    End With
    
End Sub

Private Sub DirectPlay8Event_SendComplete(dpnotify As DxVBLibA.DPNMSG_SEND_COMPLETE, fRejectMsg As Boolean)
    'VB requires that we must implement *every* member of this interface
End Sub

Private Sub DirectPlay8Event_TerminateSession(dpnotify As DxVBLibA.DPNMSG_TERMINATE_SESSION, fRejectMsg As Boolean)
    mlTerminateCode = dpnotify.hResultCode
    tmrTerminate.Enabled = True
End Sub

