VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmAudio 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Audio Options"
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
      Caption         =   "Audio details"
      Height          =   2655
      Left            =   60
      TabIndex        =   6
      Top             =   120
      Width           =   6255
      Begin MSComctlLib.Slider sldVolume 
         Height          =   255
         Left            =   120
         TabIndex        =   2
         Top             =   1140
         Width           =   6075
         _ExtentX        =   10716
         _ExtentY        =   450
         _Version        =   393216
         LargeChange     =   10
         SmallChange     =   5
         Min             =   -100
         Max             =   0
         TickFrequency   =   10
      End
      Begin VB.CheckBox chkMusic 
         Caption         =   "Play Background Music"
         Height          =   255
         Left            =   120
         TabIndex        =   3
         Top             =   1500
         Width           =   5955
      End
      Begin VB.CheckBox chkPlaySounds 
         Caption         =   "Play Sounds"
         Height          =   255
         Left            =   120
         TabIndex        =   1
         Top             =   540
         Width           =   5955
      End
      Begin MSComctlLib.Slider sldMusic 
         Height          =   255
         Left            =   60
         TabIndex        =   4
         Top             =   2100
         Width           =   6075
         _ExtentX        =   10716
         _ExtentY        =   450
         _Version        =   393216
         LargeChange     =   10
         SmallChange     =   5
         Min             =   -100
         Max             =   0
         TickFrequency   =   10
      End
      Begin VB.Label lblMusic 
         BackStyle       =   0  'Transparent
         Caption         =   "Volume of background music"
         Height          =   255
         Left            =   120
         TabIndex        =   9
         Top             =   1800
         Width           =   3855
      End
      Begin VB.Label lblSound 
         BackStyle       =   0  'Transparent
         Caption         =   "Volume of ambient sounds (puck, scoring, etc)"
         Height          =   255
         Left            =   180
         TabIndex        =   8
         Top             =   840
         Width           =   3855
      End
      Begin VB.Label Label2 
         BackStyle       =   0  'Transparent
         Caption         =   "Here you can control the few audio settings."
         Height          =   255
         Index           =   0
         Left            =   120
         TabIndex        =   7
         Top             =   240
         Width           =   6015
      End
   End
End
Attribute VB_Name = "frmAudio"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub chkMusic_Click()
    sldMusic.Enabled = (chkMusic.Value = vbChecked)
    lblMusic.Enabled = (chkMusic.Value = vbChecked)
End Sub

Private Sub chkPlaySounds_Click()
    sldVolume.Enabled = (chkPlaySounds.Value = vbChecked)
    lblSound.Enabled = (chkPlaySounds.Value = vbChecked)
End Sub

Private Sub cmdCancel_Click()
    Unload Me
End Sub

Private Sub cmdOk_Click()
    SaveAudioSettings
    Unload Me
End Sub

Private Sub Form_Load()
    'Now update the display
    chkMusic.Value = Abs(goAudio.PlayMusic)
    chkPlaySounds.Value = Abs(goAudio.PlaySounds)
    sldVolume.Value = goAudio.SoundVolume / 25
    sldMusic.Value = goAudio.MusicVolume / 25

    'Update the UI
    sldMusic.Enabled = (chkMusic.Value = vbChecked)
    lblMusic.Enabled = (chkMusic.Value = vbChecked)
    sldVolume.Enabled = (chkPlaySounds.Value = vbChecked)
    lblSound.Enabled = (chkPlaySounds.Value = vbChecked)
End Sub

Private Sub SaveAudioSettings()
    goAudio.PlayMusic = (chkMusic.Value = vbChecked)
    goAudio.PlaySounds = (chkPlaySounds.Value = vbChecked)
    goAudio.MusicVolume = sldMusic.Value * 25
    goAudio.SoundVolume = sldVolume.Value * 25
End Sub

Private Sub Form_Unload(Cancel As Integer)
    'We're leaving the form, save the settings
    SaveSetting gsKeyName, gsSubKeyAudio, "UseBackgroundMusic", goAudio.PlayMusic
    SaveSetting gsKeyName, gsSubKeyAudio, "UseSound", goAudio.PlaySounds
    SaveSetting gsKeyName, gsSubKeyAudio, "MusicVolume", goAudio.MusicVolume
    SaveSetting gsKeyName, gsSubKeyAudio, "SoundVolume", goAudio.SoundVolume
End Sub
