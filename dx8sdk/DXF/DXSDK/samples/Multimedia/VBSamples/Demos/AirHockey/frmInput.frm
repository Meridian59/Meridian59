VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmInput 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Input Options"
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
   Begin VB.Frame Frame1 
      Caption         =   "Controllers"
      Height          =   2715
      Left            =   60
      TabIndex        =   8
      Top             =   120
      Width           =   6255
      Begin MSComctlLib.Slider sldSens 
         Height          =   195
         Left            =   1080
         TabIndex        =   5
         Top             =   2400
         Width           =   4995
         _ExtentX        =   8811
         _ExtentY        =   344
         _Version        =   393216
         Min             =   1
         Max             =   50
         SelStart        =   1
         TickFrequency   =   5
         Value           =   1
      End
      Begin VB.ComboBox cboJoy 
         Height          =   315
         Left            =   300
         Style           =   2  'Dropdown List
         TabIndex        =   4
         Top             =   1980
         Width           =   5775
      End
      Begin VB.CheckBox chkJoy 
         Caption         =   "Joystick"
         Height          =   315
         Left            =   60
         TabIndex        =   3
         Top             =   1665
         Width           =   5655
      End
      Begin VB.CheckBox chkKeyboard 
         Caption         =   "Keyboard"
         Height          =   315
         Left            =   60
         TabIndex        =   2
         Top             =   1080
         Width           =   5655
      End
      Begin VB.CheckBox chkMouse 
         Caption         =   "Mouse"
         Height          =   315
         Left            =   120
         TabIndex        =   0
         Top             =   420
         Width           =   5655
      End
      Begin MSComctlLib.Slider sldMouseSens 
         Height          =   195
         Left            =   1140
         TabIndex        =   1
         Top             =   780
         Width           =   4935
         _ExtentX        =   8705
         _ExtentY        =   344
         _Version        =   393216
         Min             =   1
         Max             =   50
         SelStart        =   1
         TickFrequency   =   5
         Value           =   1
      End
      Begin MSComctlLib.Slider sldKeyboard 
         Height          =   195
         Left            =   1125
         TabIndex        =   12
         Top             =   1410
         Width           =   4935
         _ExtentX        =   8705
         _ExtentY        =   344
         _Version        =   393216
         Min             =   1
         Max             =   100
         SelStart        =   1
         TickFrequency   =   5
         Value           =   1
      End
      Begin VB.Label lblKeySens 
         BackStyle       =   0  'Transparent
         Caption         =   "Sensitivity"
         Height          =   255
         Left            =   345
         TabIndex        =   13
         Top             =   1410
         Width           =   735
      End
      Begin VB.Label lblMouseSens 
         BackStyle       =   0  'Transparent
         Caption         =   "Sensitivity"
         Height          =   255
         Left            =   360
         TabIndex        =   11
         Top             =   780
         Width           =   735
      End
      Begin VB.Label lblSens 
         BackStyle       =   0  'Transparent
         Caption         =   "Sensitivity"
         Height          =   255
         Left            =   300
         TabIndex        =   10
         Top             =   2400
         Width           =   735
      End
      Begin VB.Label Label2 
         BackStyle       =   0  'Transparent
         Caption         =   "Here you will select the controllers you wish to use during gameplay."
         Height          =   255
         Index           =   0
         Left            =   120
         TabIndex        =   9
         Top             =   180
         Width           =   4875
      End
   End
   Begin VB.CommandButton cmdOk 
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   5340
      TabIndex        =   7
      Top             =   2940
      Width           =   975
   End
   Begin VB.CommandButton cmdCancel 
      Cancel          =   -1  'True
      Caption         =   "Cancel"
      Height          =   375
      Left            =   4260
      TabIndex        =   6
      Top             =   2940
      Width           =   975
   End
End
Attribute VB_Name = "frmInput"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Private diDevEnum As DirectInputEnumDevices8

Private Sub chkJoy_Click()
    cboJoy.Enabled = (chkJoy.Value = vbChecked)
    lblSens.Enabled = (chkJoy.Value = vbChecked)
    sldSens.Enabled = (chkJoy.Value = vbChecked)
End Sub

Private Sub chkKeyboard_Click()
    If chkMouse.Value = vbUnchecked And chkKeyboard.Value = vbUnchecked Then
        MsgBox "You must leave at least the keyboard or the mouse enabled.", vbOKOnly Or vbInformation, "No basic input"
        chkKeyboard.Value = vbChecked
    End If
    lblKeySens.Enabled = (chkKeyboard.Value = vbChecked)
    sldKeyboard.Enabled = (chkKeyboard.Value = vbChecked)
End Sub

Private Sub chkMouse_Click()
    If chkMouse.Value = vbUnchecked And chkKeyboard.Value = vbUnchecked Then
        MsgBox "You must leave at least the keyboard or the mouse enabled.", vbOKOnly Or vbInformation, "No basic input"
        chkMouse.Value = vbChecked
    End If
    lblMouseSens.Enabled = (chkMouse.Value = vbChecked)
    sldMouseSens.Enabled = (chkMouse.Value = vbChecked)
End Sub

Private Sub cmdCancel_Click()
    Unload Me
End Sub

Private Sub cmdOk_Click()
    SaveAudioSettings
    Unload Me
End Sub

Private Sub Form_Load()
    Dim lIndex As Long
    
    'Now update the display
    chkMouse.Value = Abs(goInput.UseMouse)
    chkKeyboard.Value = Abs(goInput.UseKeyboard)
    chkJoy.Value = Abs(goInput.UseJoystick)
    sldSens.Value = (goInput.JoystickSensitivity * 100000)
    sldMouseSens.Value = (goInput.MouseSensitivity * 1000)
    sldKeyboard.Value = (goInput.KeyboardSensitivity * 1000)
    
    cboJoy.Enabled = (chkJoy.Value = vbChecked)
    lblSens.Enabled = (chkJoy.Value = vbChecked)
    sldSens.Enabled = (chkJoy.Value = vbChecked)
    lblMouseSens.Enabled = (chkMouse.Value = vbChecked)
    sldMouseSens.Enabled = (chkMouse.Value = vbChecked)
    lblKeySens.Enabled = (chkKeyboard.Value = vbChecked)
    sldKeyboard.Enabled = (chkKeyboard.Value = vbChecked)
    
    'Now, let's fill up the ui for the joysticks
    Set diDevEnum = goInput.InputObject.GetDIDevices(DI8DEVCLASS_GAMECTRL, DIEDFL_ATTACHEDONLY)
    If diDevEnum.GetCount = 0 Then
        chkJoy.Enabled = False
        cboJoy.Enabled = False
    Else
        'Ok, there *are* joysticks.  Load them into the combo box
        Dim lCount As Long
        lIndex = 0
        For lCount = 1 To diDevEnum.GetCount
            cboJoy.AddItem diDevEnum.GetItem(lCount).GetInstanceName
            If diDevEnum.GetItem(lCount).GetGuidInstance = goInput.JoystickGuid Then lIndex = lCount - 1
        Next
        cboJoy.ListIndex = lIndex
    End If
End Sub

Private Sub SaveAudioSettings()
    goInput.UseMouse = (chkMouse.Value = vbChecked)
    If goInput.UseMouse Then
        goInput.MouseSensitivity = sldMouseSens.Value / 1000
    End If
    goInput.UseKeyboard = (chkKeyboard.Value = vbChecked)
    If goInput.UseKeyboard Then
        goInput.KeyboardSensitivity = sldKeyboard.Value / 1000
    End If
    goInput.UseJoystick = (chkJoy.Value = vbChecked)
    If goInput.UseJoystick Then
        goInput.JoystickGuid = diDevEnum.GetItem(cboJoy.ListIndex + 1).GetGuidInstance
        goInput.JoystickSensitivity = sldSens.Value / 100000
    End If

End Sub

Private Sub Form_Unload(Cancel As Integer)
    'We're leaving the form, save the settings
    SaveSetting gsKeyName, gsSubKeyInput, "UseMouse", goInput.UseMouse
    SaveSetting gsKeyName, gsSubKeyInput, "UseKeyboard", goInput.UseKeyboard
    SaveSetting gsKeyName, gsSubKeyInput, "UseJoystick", goInput.UseJoystick
    If goInput.UseJoystick Then
        SaveSetting gsKeyName, gsSubKeyInput, "JoystickGuid", goInput.JoystickGuid
        SaveSetting gsKeyName, gsSubKeyInput, "JoystickSensitivity", goInput.JoystickSensitivity
    End If
    If goInput.UseMouse Then
        SaveSetting gsKeyName, gsSubKeyInput, "MouseSensitivity", goInput.MouseSensitivity
    End If
    If goInput.UseKeyboard Then
        SaveSetting gsKeyName, gsSubKeyInput, "KeyboardSensitivity", goInput.KeyboardSensitivity
    End If
End Sub

