VERSION 5.00
Begin VB.Form frmGraphics 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Graphic Options"
   ClientHeight    =   3360
   ClientLeft      =   45
   ClientTop       =   285
   ClientWidth     =   6405
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3360
   ScaleWidth      =   6405
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.CommandButton cmdOk 
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   5340
      TabIndex        =   0
      Top             =   2880
      Width           =   975
   End
   Begin VB.CommandButton cmdCancel 
      Cancel          =   -1  'True
      Caption         =   "Cancel"
      Height          =   375
      Left            =   4260
      TabIndex        =   5
      Top             =   2880
      Width           =   975
   End
   Begin VB.Frame Frame1 
      Caption         =   "Options"
      Height          =   2655
      Left            =   60
      TabIndex        =   6
      Top             =   120
      Width           =   6255
      Begin VB.CommandButton cmdChgMode 
         Caption         =   "Change..."
         Height          =   315
         Left            =   180
         TabIndex        =   1
         Top             =   900
         Width           =   1575
      End
      Begin VB.OptionButton optOffice 
         Caption         =   "Office Lobby (Less detailed, higher performance)"
         Height          =   255
         Left            =   120
         TabIndex        =   4
         Top             =   2280
         Width           =   4935
      End
      Begin VB.OptionButton optGame 
         Caption         =   "Game Room (More detailed, lower performance)"
         Height          =   255
         Left            =   120
         TabIndex        =   3
         Top             =   1980
         Value           =   -1  'True
         Width           =   4935
      End
      Begin VB.CheckBox chkDrawRoom 
         Caption         =   "Display the room where the game is taking place."
         Height          =   315
         Left            =   120
         TabIndex        =   2
         Top             =   1260
         Width           =   4095
      End
      Begin VB.Label lblMode 
         BackStyle       =   0  'Transparent
         BorderStyle     =   1  'Fixed Single
         Caption         =   "Label1"
         Height          =   255
         Left            =   1245
         TabIndex        =   10
         Top             =   570
         Width           =   4815
      End
      Begin VB.Label Label2 
         BackStyle       =   0  'Transparent
         Caption         =   "Display Mode"
         Height          =   255
         Index           =   2
         Left            =   180
         TabIndex        =   9
         Top             =   600
         Width           =   1095
      End
      Begin VB.Label lblRoom 
         BackStyle       =   0  'Transparent
         Caption         =   "If the room is displayed, what type of room should be displayed?"
         Height          =   255
         Left            =   120
         TabIndex        =   8
         Top             =   1680
         Width           =   4635
      End
      Begin VB.Label Label2 
         BackStyle       =   0  'Transparent
         Caption         =   "You can control different options here to improve graphic quality, or performance"
         Height          =   255
         Index           =   0
         Left            =   120
         TabIndex        =   7
         Top             =   240
         Width           =   6015
      End
   End
End
Attribute VB_Name = "frmGraphics"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub chkDrawRoom_Click()
    EnableRoomInfo (chkDrawRoom.Value = vbChecked)
End Sub

Private Sub cmdCancel_Click()
    Unload Me
End Sub

Private Sub cmdChgMode_Click()
    goDev.Show vbModal
    'Now that we've potentially changed modes, update them
    If goDev.Windowed Then
        lblMode.Caption = "Windowed mode"
    Else
        lblMode.Caption = goDev.ModeString
    End If
End Sub

Private Sub cmdOk_Click()
    SaveAudioSettings
    Unload Me
End Sub

Private Sub Form_Load()
    'Now update the display
    chkDrawRoom.Value = Abs(goRoom.DrawRoom)
    optGame.Value = goRoom.BarRoom
    optOffice.Value = Not goRoom.BarRoom
    If goDev.Windowed Then
        lblMode.Caption = "Windowed mode"
    Else
        lblMode.Caption = goDev.ModeString
    End If
    EnableRoomInfo (chkDrawRoom.Value = vbChecked)
End Sub

Private Sub SaveAudioSettings()
    goRoom.DrawRoom = (chkDrawRoom.Value = vbChecked)
    goRoom.BarRoom = optGame.Value
End Sub

Private Sub Form_Unload(Cancel As Integer)
    'We're leaving the form, save the settings
    SaveSetting gsKeyName, gsSubKey, "DrawRoom", goRoom.DrawRoom
    SaveSetting gsKeyName, gsSubKey, "RoomIsBarRoom", goRoom.BarRoom
    SaveSetting gsKeyName, gsSubKeyGraphics, "Windowed", goDev.Windowed
    SaveSetting gsKeyName, gsSubKeyGraphics, "AdapterID", goDev.Adapter
    SaveSetting gsKeyName, gsSubKeyGraphics, "Mode", goDev.Mode
    
End Sub

Private Sub EnableRoomInfo(ByVal fEnable As Boolean)
    lblRoom.Enabled = fEnable
    optGame.Enabled = fEnable
    optOffice.Enabled = fEnable
End Sub
