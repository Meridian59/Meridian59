VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmSplash 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "vb Air Hockey"
   ClientHeight    =   5400
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   7200
   Icon            =   "frmSplash.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   5400
   ScaleWidth      =   7200
   StartUpPosition =   3  'Windows Default
   Begin VB.Frame Frame1 
      Caption         =   "Options"
      Height          =   2115
      Left            =   60
      TabIndex        =   5
      Top             =   720
      Width           =   7095
      Begin VB.CommandButton cmdAudio 
         Caption         =   "&Audio Options..."
         Height          =   375
         Left            =   5040
         TabIndex        =   15
         Top             =   1560
         Width           =   1815
      End
      Begin VB.CommandButton cmdInput 
         Caption         =   "&Input Options..."
         Height          =   375
         Left            =   5040
         TabIndex        =   14
         Top             =   1080
         Width           =   1815
      End
      Begin VB.CommandButton cmdGraphicsOptions 
         Caption         =   "&Graphics Options..."
         Height          =   375
         Left            =   5040
         TabIndex        =   13
         Top             =   600
         Width           =   1815
      End
      Begin MSComctlLib.Slider sldSpeed 
         Height          =   255
         Left            =   150
         TabIndex        =   7
         Top             =   870
         Width           =   2055
         _ExtentX        =   3625
         _ExtentY        =   450
         _Version        =   393216
         LargeChange     =   50
         SmallChange     =   5
         Min             =   10
         Max             =   1000
         SelStart        =   10
         TickFrequency   =   100
         Value           =   10
      End
      Begin MSComctlLib.Slider sldScore 
         Height          =   255
         Left            =   150
         TabIndex        =   8
         Top             =   1470
         Width           =   2055
         _ExtentX        =   3625
         _ExtentY        =   450
         _Version        =   393216
         LargeChange     =   3
         Min             =   2
         Max             =   20
         SelStart        =   10
         TickFrequency   =   2
         Value           =   10
      End
      Begin MSComctlLib.Slider sldPaddleMass 
         Height          =   255
         Left            =   2310
         TabIndex        =   10
         Top             =   870
         Width           =   2055
         _ExtentX        =   3625
         _ExtentY        =   450
         _Version        =   393216
         LargeChange     =   50
         SmallChange     =   5
         Min             =   50
         Max             =   300
         SelStart        =   50
         TickFrequency   =   20
         Value           =   50
      End
      Begin VB.Label Label3 
         BackStyle       =   0  'Transparent
         Caption         =   $"frmSplash.frx":000C
         Height          =   435
         Left            =   60
         TabIndex        =   12
         Top             =   180
         Width           =   6930
      End
      Begin VB.Label Label2 
         BackStyle       =   0  'Transparent
         Caption         =   "Paddle Mass"
         Height          =   255
         Index           =   2
         Left            =   2430
         TabIndex        =   11
         Top             =   630
         Width           =   3615
      End
      Begin VB.Label Label2 
         BackStyle       =   0  'Transparent
         Caption         =   "Winning Score"
         Height          =   255
         Index           =   1
         Left            =   270
         TabIndex        =   9
         Top             =   1230
         Width           =   1215
      End
      Begin VB.Label Label2 
         BackStyle       =   0  'Transparent
         Caption         =   "Game Speed"
         Height          =   255
         Index           =   0
         Left            =   270
         TabIndex        =   6
         Top             =   630
         Width           =   1215
      End
   End
   Begin VB.CommandButton cmdExit 
      Cancel          =   -1  'True
      Caption         =   "Exit Game"
      Height          =   375
      Left            =   5220
      TabIndex        =   2
      Top             =   4860
      Width           =   1815
   End
   Begin VB.CommandButton cmdSingle 
      Caption         =   "&Single Player Game..."
      Height          =   375
      Left            =   5220
      TabIndex        =   0
      Top             =   4020
      Width           =   1815
   End
   Begin VB.CommandButton cmdMulti 
      Caption         =   "&Multiplayer Game..."
      Height          =   375
      Left            =   5220
      TabIndex        =   1
      Top             =   4440
      Width           =   1815
   End
   Begin VB.Label lblText 
      BackStyle       =   0  'Transparent
      Caption         =   "Label2"
      ForeColor       =   &H80000008&
      Height          =   2340
      Left            =   180
      TabIndex        =   4
      Top             =   2895
      Width           =   4920
   End
   Begin VB.Label Label1 
      Alignment       =   2  'Center
      BackStyle       =   0  'Transparent
      Caption         =   "Visual Basic Air Hockey"
      BeginProperty Font 
         Name            =   "Comic Sans MS"
         Size            =   21.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H000000FF&
      Height          =   675
      Left            =   120
      TabIndex        =   3
      Top             =   0
      Width           =   6915
   End
End
Attribute VB_Name = "frmSplash"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Dim fStartingGame As Boolean

Private Sub cmdAudio_Click()
    'Here we will allow the changing of the audio options.
    'Load the options screen
    frmAudio.Show vbModal, Me
End Sub

Private Sub cmdExit_Click()
    'Quit the game
    Unload Me
End Sub

Private Sub cmdGraphicsOptions_Click()
    'Here we will allow the changing of the graphics options.
    'Load the options screen
    frmGraphics.Show vbModal, Me
End Sub

Private Sub cmdInput_Click()
    'Here we will allow the changing of the input options.
    'Load the options screen
    frmInput.Show vbModal, Me
End Sub

Private Sub cmdMulti_Click()
    fStartingGame = True
    SaveAll
    gfMultiplayer = True
    gfGameCanBeStarted = False
    'Go ahead and load the game
    frmAir.Show vbModeless
End Sub

Private Sub cmdSingle_Click()
    fStartingGame = True
    SaveAll
    gfMultiplayer = False
    gfGameCanBeStarted = True
    'Go ahead and load the game
    frmAir.Show vbModeless
End Sub

Private Sub Form_Load()
    gfScored = True
    
    sldSpeed.Value = CLng(GetSetting(gsKeyName, gsSubKey, "Speed", 780))
    sldScore.Value = CLng(GetSetting(gsKeyName, gsSubKey, "WinningScore", glDefaultWinningScore))
    sldPaddleMass.Value = CLng(GetSetting(gsKeyName, gsSubKey, "PaddleMass", 100))
    
    lblText.Caption = "Options during gameplay:" & vbCrLf & vbCrLf & _
        "<Space> Start game (launch puck from center)" & vbCrLf & _
        "<F1> Toggle showing the game room" & vbCrLf & _
        "<F3> Restart the game after someone has won" & vbCrLf & _
        "<F4> Toggle the game tables transparency" & vbCrLf & _
        "<F5> Toggle showing the game table" & vbCrLf & _
        "<F6> Toggle the paddles transparency" & vbCrLf & _
        "<V> Scroll through default views" & vbCrLf & _
        "<W> Toggle wireframe mode" & vbCrLf & vbCrLf & _
        "Use the right mouse button to get a different view."
    
    'Init objects and load values
    LoadObjects
End Sub

Private Sub Form_Unload(Cancel As Integer)
    If Not fStartingGame Then 'We're not starting, cleanup
        Set goPuck = Nothing
        Set goPaddle(0) = Nothing
        Set goPaddle(1) = Nothing
        Set goCamera = Nothing
        Set goTable = Nothing
        Set goRoom = Nothing
        Set goInput = Nothing
        Set goAudio = New cAudio
        Set goDev = Nothing
        End
    End If
End Sub

Private Sub SaveAll()
    gnVelocityDamp = sldSpeed.Value / 1000
    glUserWinningScore = sldScore.Value
    gnPaddleMass = sldPaddleMass.Value / 100
    goPuck.MaximumPuckVelocity = gnVelocityDamp * glMaxPuckSpeedConstant
    
    SaveSetting gsKeyName, gsSubKey, "Speed", CStr(sldSpeed.Value)
    SaveSetting gsKeyName, gsSubKey, "WinningScore", CStr(sldScore.Value)
    SaveSetting gsKeyName, gsSubKey, "PaddleMass", CStr(sldPaddleMass.Value)
End Sub

