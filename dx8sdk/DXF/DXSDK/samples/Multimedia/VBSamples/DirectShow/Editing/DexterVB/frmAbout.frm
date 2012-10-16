VERSION 5.00
Begin VB.Form frmAbout 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "About Dexter VB Sample"
   ClientHeight    =   3555
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   5730
   ClipControls    =   0   'False
   Icon            =   "frmAbout.frx":0000
   LinkTopic       =   "Form2"
   LockControls    =   -1  'True
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   2453.724
   ScaleMode       =   0  'User
   ScaleWidth      =   5380.766
   ShowInTaskbar   =   0   'False
   StartUpPosition =   1  'CenterOwner
   Begin VB.PictureBox picIcon 
      AutoSize        =   -1  'True
      ClipControls    =   0   'False
      Height          =   540
      Left            =   240
      Picture         =   "frmAbout.frx":030A
      ScaleHeight     =   337.12
      ScaleMode       =   0  'User
      ScaleWidth      =   337.12
      TabIndex        =   1
      Top             =   240
      Width           =   540
   End
   Begin VB.CommandButton cmdOK 
      Cancel          =   -1  'True
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   345
      Left            =   4245
      TabIndex        =   0
      Top             =   2625
      Width           =   1260
   End
   Begin VB.CommandButton cmdSysInfo 
      Caption         =   "&System Info..."
      Height          =   345
      Left            =   4260
      TabIndex        =   2
      Top             =   3075
      Width           =   1245
   End
   Begin VB.Line Line1 
      BorderColor     =   &H00808080&
      BorderStyle     =   6  'Inside Solid
      Index           =   1
      X1              =   84.515
      X2              =   5309.398
      Y1              =   1687.583
      Y2              =   1687.583
   End
   Begin VB.Label lblDescription 
      Caption         =   "Copyright (C) 1999-2001 Microsoft Corporation"
      ForeColor       =   &H00000000&
      Height          =   1170
      Left            =   1050
      TabIndex        =   3
      Top             =   825
      Width           =   3885
   End
   Begin VB.Label lblTitle 
      Caption         =   "VB Dexter Sample Application"
      ForeColor       =   &H00000000&
      Height          =   255
      Left            =   1050
      TabIndex        =   5
      Top             =   240
      Width           =   3885
   End
   Begin VB.Line Line1 
      BorderColor     =   &H00FFFFFF&
      BorderWidth     =   2
      Index           =   0
      X1              =   98.6
      X2              =   5309.398
      Y1              =   1697.936
      Y2              =   1697.936
   End
   Begin VB.Label lblVersion 
      Caption         =   "Version 1.0"
      Height          =   225
      Left            =   1050
      TabIndex        =   6
      Top             =   525
      Width           =   3885
   End
   Begin VB.Label lblDisclaimer 
      Caption         =   "Warning: You must have DX8 or newer."
      ForeColor       =   &H00000000&
      Height          =   825
      Left            =   255
      TabIndex        =   4
      Top             =   2625
      Width           =   3870
   End
End
Attribute VB_Name = "frmAbout"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'*******************************************************************************
'*       This is a part of the Microsoft DXSDK Code Samples.
'*       Copyright (C) 1999-2001 Microsoft Corporation.
'*       All rights reserved.
'*       This source code is only intended as a supplement to
'*       Microsoft Development Tools and/or SDK documentation.
'*       See these sources for detailed information regarding the
'*       Microsoft samples programs.
'*******************************************************************************
Option Explicit
Option Base 0
Option Compare Text
            
            
' **************************************************************************************************************************************
' * PRIVATE INTERFACE- FORM EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: Form_Load
            ' * procedure description:  Occurs when a form is loaded.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_Load()
            On Local Error GoTo ErrLine
            Me.Caption = "About " & App.Title: lblTitle.Caption = App.Title
            lblVersion.Caption = "Version " & CStr(App.Major) & "." & CStr(App.Minor) & "." & CStr(App.Revision)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Form_QueryUnload
            ' * procedure description:  Occurs before a form or application closes.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_QueryUnload(Cancel As Integer, UnloadMode As Integer)
            On Local Error GoTo ErrLine
            
            Select Case UnloadMode
                Case vbFormControlMenu
                         '0 The user chose the Close command from the Control menu on the form.
                Case vbFormCode
                         '1 The Unload statement is invoked from code.
                Case vbAppWindows
                         '2 The current Microsoft Windows operating environment session is ending.
                Case vbAppTaskManager
                         '3 The Microsoft Windows Task Manager is closing the application.
                         End
                Case vbFormMDIForm
                         '4 An MDI child form is closing because the MDI form is closing.
                Case vbFormOwner
                         '5 A form is closing because its owner is closing
            End Select
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Form_Unload
            ' * procedure description:  Occurs when a form is about to be removed from the screen.
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_Unload(Cancel As Integer)
            On Local Error GoTo ErrLine
            With Me
               .Move 0 - (Screen.Width * 8), 0 - (Screen.Height * 8): .Visible = False
            End With
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub




' **************************************************************************************************************************************
' * PRIVATE INTERFACE- CONTROL EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: cmdSysInfo_Click
            ' * procedure description:  occures when the 'System Information' command button is pressed
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdSysInfo_Click()
            Dim bstrSysInfoPath As String
            On Local Error GoTo ErrLine
            
            'query the registry for a path to msinfo.exe, and execute the application for the user
            If Registry_DoesKeyExist(HKEY_LOCAL_MACHINE, "SOFTWARE\Microsoft\Shared Tools\MSINFO") Then
               bstrSysInfoPath = Registry_QueryEntryValue(HKEY_LOCAL_MACHINE, "SOFTWARE\Microsoft\Shared Tools\MSINFO", "Path", vbNullString)
               
               Call Shell(bstrSysInfoPath, vbNormalFocus)
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdOK_Click
            ' * procedure description:  occures when the 'ok' command button is pressed
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdOk_Click()
            On Local Error GoTo ErrLine
            Unload Me
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
