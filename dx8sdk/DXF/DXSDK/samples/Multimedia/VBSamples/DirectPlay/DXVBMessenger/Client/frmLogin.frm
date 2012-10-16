VERSION 5.00
Begin VB.Form frmLogin 
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Login"
   ClientHeight    =   3255
   ClientLeft      =   45
   ClientTop       =   285
   ClientWidth     =   4680
   Icon            =   "frmLogin.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3255
   ScaleWidth      =   4680
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.CommandButton cmdCancel 
      Cancel          =   -1  'True
      Caption         =   "Cancel"
      Height          =   375
      Left            =   2520
      TabIndex        =   5
      Top             =   2760
      Width           =   1035
   End
   Begin VB.CommandButton cmdCreate 
      Caption         =   "&Create Account"
      Height          =   375
      Left            =   60
      TabIndex        =   4
      Top             =   2760
      Width           =   1335
   End
   Begin VB.CommandButton cmdLogin 
      Caption         =   "Log on"
      Default         =   -1  'True
      Height          =   375
      Left            =   3600
      TabIndex        =   6
      Top             =   2760
      Width           =   1035
   End
   Begin VB.TextBox txtServerName 
      Height          =   285
      Left            =   60
      TabIndex        =   3
      Top             =   2400
      Width           =   4515
   End
   Begin VB.CheckBox chkRemember 
      Caption         =   "Remember this password"
      Height          =   255
      Left            =   60
      TabIndex        =   2
      Top             =   1860
      Width           =   4515
   End
   Begin VB.TextBox txtPassword 
      Height          =   285
      IMEMode         =   3  'DISABLE
      Left            =   60
      PasswordChar    =   "*"
      TabIndex        =   1
      Top             =   1500
      Width           =   4515
   End
   Begin VB.TextBox txtUserName 
      Height          =   285
      Left            =   60
      TabIndex        =   0
      Top             =   900
      Width           =   4515
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "Server Name:"
      Height          =   195
      Index           =   3
      Left            =   60
      TabIndex        =   10
      Top             =   2160
      Width           =   1395
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "Password:"
      Height          =   195
      Index           =   2
      Left            =   60
      TabIndex        =   9
      Top             =   1260
      Width           =   915
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "UserName:"
      Height          =   195
      Index           =   1
      Left            =   60
      TabIndex        =   8
      Top             =   660
      Width           =   915
   End
   Begin VB.Label Label1 
      BackStyle       =   0  'Transparent
      Caption         =   "Please type in your username, password and server to connect to, or click the 'Create Account' button..."
      Height          =   495
      Index           =   0
      Left            =   60
      TabIndex        =   7
      Top             =   120
      Width           =   4575
   End
End
Attribute VB_Name = "frmLogin"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       frmLogin.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Private Sub cmdCancel_Click()
    Unload Me
End Sub

Private Sub cmdCreate_Click()
    Unload Me
    frmCreate.Show , frmClient
End Sub

Private Sub cmdLogin_Click()
    Dim AppDesc As DPN_APPLICATION_DESC
    
    gfCreatePlayer = False
    If txtServerName.Text = vbNullString Then 'They didn't enter a server name
        MsgBox "You must enter a server name.", vbOKOnly Or vbInformation, "No server name."
        Exit Sub
    End If
    If txtPassword.Text = vbNullString Then 'They didn't enter a password
        MsgBox "You must enter a password.", vbOKOnly Or vbInformation, "No password."
        Exit Sub
    End If
    If txtUserName.Text = vbNullString Then 'They didn't enter a user name
        MsgBox "You must enter a user name.", vbOKOnly Or vbInformation, "No user name."
        Exit Sub
    End If
    cmdLogin.Enabled = False
    'Now let's save the settings
    SaveSetting gsAppName, "Startup", "ServerName", txtServerName.Text
    SaveSetting gsAppName, "Startup", "Username", txtUserName.Text
    If chkRemember.Value = vbChecked Then
        SaveSetting gsAppName, "Startup", "Password", txtPassword.Text
    Else
        SaveSetting gsAppName, "Startup", "Password", vbNullString
    End If
    If gsServerName = vbNullString Then gsServerName = txtServerName.Text

    If gfConnected And (gsServerName = txtServerName.Text) Then
        'Save the username/password
        gsPass = EncodePassword(txtPassword.Text, glClientSideEncryptionKey)
        gsUserName = txtUserName.Text
        LogonPlayer
    Else
        If gfConnected Then
            InitDPlay 'Re-Initialize DPlay
        End If
        dpas.AddComponentString DPN_KEY_HOSTNAME, txtServerName.Text 'We only want to connect on this host
        'First set up our application description
        With AppDesc
            .guidApplication = AppGuid
        End With
        'Save the username/password
        gsPass = EncodePassword(txtPassword.Text, glClientSideEncryptionKey)
        gsUserName = txtUserName.Text
        On Error Resume Next
        'Try to connect to this server
        dpc.Connect AppDesc, dpas, dpa, 0, ByVal 0&, 0
        If Err.Number <> 0 Then
            MsgBox "This server could not be contacted.  Please check the server name and try again.", vbOKOnly Or vbInformation, "Not found."
            cmdLogin.Enabled = True
            Exit Sub
        End If
    End If
    
End Sub

Private Sub Form_Load()

    'First retrieve the settings
    txtServerName.Text = GetSetting(gsAppName, "Startup", "ServerName", vbNullString)
    txtUserName.Text = GetSetting(gsAppName, "Startup", "Username", vbNullString)
    txtPassword.Text = GetSetting(gsAppName, "Startup", "Password", vbNullString)
    If txtPassword.Text <> vbNullString Then chkRemember.Value = vbChecked 'We remembered our password
    cmdLogin.Enabled = True
End Sub
