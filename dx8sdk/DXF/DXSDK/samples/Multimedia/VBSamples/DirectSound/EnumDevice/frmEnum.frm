VERSION 5.00
Begin VB.Form frmEnum 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "EnumDevices"
   ClientHeight    =   1740
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   3960
   Icon            =   "frmEnum.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   1740
   ScaleWidth      =   3960
   StartUpPosition =   3  'Windows Default
   Begin VB.ComboBox cboCapture 
      Height          =   315
      Left            =   1320
      Style           =   2  'Dropdown List
      TabIndex        =   6
      Top             =   840
      Width           =   2535
   End
   Begin VB.ComboBox cboSound 
      Height          =   315
      Left            =   1320
      Style           =   2  'Dropdown List
      TabIndex        =   5
      Top             =   420
      Width           =   2535
   End
   Begin VB.CommandButton cmdExit 
      Cancel          =   -1  'True
      Caption         =   "E&xit"
      Height          =   315
      Left            =   2880
      TabIndex        =   4
      Top             =   1260
      Width           =   975
   End
   Begin VB.CommandButton cmdCreate 
      Caption         =   "&Create"
      Default         =   -1  'True
      Height          =   315
      Left            =   120
      TabIndex        =   3
      Top             =   1260
      Width           =   975
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "Capture Device:"
      Height          =   255
      Index           =   2
      Left            =   120
      TabIndex        =   2
      Top             =   900
      Width           =   1215
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "Sound Device:"
      Height          =   255
      Index           =   1
      Left            =   120
      TabIndex        =   1
      Top             =   480
      Width           =   1215
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "This sample shows how to enumerate devices."
      Height          =   255
      Index           =   0
      Left            =   120
      TabIndex        =   0
      Top             =   60
      Width           =   4455
   End
End
Attribute VB_Name = "frmEnum"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmEnum.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Private dx As New DirectX8
Private dsEnum As DirectSoundEnum8
Private dscEnum As DirectSoundEnum8
Private ds As DirectSound8
Private dsc As DirectSoundCapture8

Private Sub cmdCreate_Click()
    On Error GoTo FailedCreate
        
    'Create a DirectSound object
    Set ds = dx.DirectSoundCreate(dsEnum.GetGuid(cboSound.ListIndex + 1))
    Set ds = Nothing 'We should get rid of it now, since we don't want to fail
                     'If the machine doesn't support full duplex
                     
    'Create a Capture Buffer
    Set dsc = dx.DirectSoundCaptureCreate(dscEnum.GetGuid(cboCapture.ListIndex + 1))
    Set dsc = Nothing 'Release it
    'Notify the user we succeeded
    MsgBox "DirectSound8 and DirectSoundCapture8 object creation succeeded.", vbOKOnly Or vbInformation, "Success"
    Exit Sub
    
FailedCreate:
    'Notify the user we failed
    MsgBox "DirectSound8 and DirectSoundCapture8 object creation failed.", vbOKOnly Or vbInformation, "Failure"
End Sub

Private Sub cmdExit_Click()
    'We're done exit
    Unload Me
End Sub

Private Sub CleanUp()
    Set dscEnum = Nothing
    Set dsEnum = Nothing
    Set dx = Nothing
End Sub

Private Sub Form_Load()
    'Enum the devices and load them into the box
    LoadEnum
End Sub

Private Sub LoadEnum()
    Dim lCount As Long
    
    On Error GoTo FailedEnum
    Set dsEnum = dx.GetDSEnum
    Set dscEnum = dx.GetDSCaptureEnum
    
    'Add each description to the combo box
    For lCount = 1 To dsEnum.GetCount
        cboSound.AddItem dsEnum.GetDescription(lCount)
    Next
    'Add each description to the combo box
    For lCount = 1 To dscEnum.GetCount
        cboCapture.AddItem dscEnum.GetDescription(lCount)
    Next
    On Error Resume Next
    'Select the first item in each combo box
    cboCapture.ListIndex = 0
    cboSound.ListIndex = 0
    Exit Sub
    
FailedEnum:
    MsgBox "Error enumerating DirectSound devices. " & vbCrLf & "Sample will now exit.", vbOKOnly Or vbInformation, "DirectSound Sample"
    Unload Me
End Sub

Private Sub Form_Unload(Cancel As Integer)
    CleanUp
End Sub
