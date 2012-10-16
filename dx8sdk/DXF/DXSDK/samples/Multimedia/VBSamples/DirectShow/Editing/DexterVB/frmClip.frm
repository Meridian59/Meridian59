VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form frmClip 
   Caption         =   "Clip Menu"
   ClientHeight    =   5715
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   7005
   Icon            =   "frmClip.frx":0000
   LinkTopic       =   "Form2"
   LockControls    =   -1  'True
   ScaleHeight     =   5715
   ScaleWidth      =   7005
   StartUpPosition =   1  'CenterOwner
   Begin VB.CommandButton cmdOK 
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   340
      Left            =   4500
      TabIndex        =   11
      Top             =   5325
      Width           =   1095
   End
   Begin VB.CommandButton cmdCancel 
      Caption         =   "Cancel"
      Height          =   340
      Left            =   5700
      TabIndex        =   12
      Top             =   5325
      Width           =   1215
   End
   Begin VB.Frame fraFixture 
      Height          =   5190
      Left            =   75
      TabIndex        =   13
      Top             =   0
      Width           =   6840
      Begin VB.TextBox txtMediaName 
         Height          =   375
         Left            =   1470
         Locked          =   -1  'True
         TabIndex        =   0
         Top             =   300
         Width           =   3975
      End
      Begin VB.TextBox txtMStart 
         BeginProperty DataFormat 
            Type            =   1
            Format          =   "0"
            HaveTrueFalseNull=   0
            FirstDayOfWeek  =   0
            FirstWeekOfYear =   0
            LCID            =   1033
            SubFormatType   =   1
         EndProperty
         Height          =   375
         Left            =   1470
         TabIndex        =   2
         Top             =   780
         Width           =   3975
      End
      Begin VB.TextBox txtMStop 
         BeginProperty DataFormat 
            Type            =   1
            Format          =   "0"
            HaveTrueFalseNull=   0
            FirstDayOfWeek  =   0
            FirstWeekOfYear =   0
            LCID            =   1033
            SubFormatType   =   1
         EndProperty
         Height          =   375
         Left            =   1470
         TabIndex        =   3
         Top             =   1260
         Width           =   3975
      End
      Begin VB.TextBox txtTStart 
         BeginProperty DataFormat 
            Type            =   1
            Format          =   "0"
            HaveTrueFalseNull=   0
            FirstDayOfWeek  =   0
            FirstWeekOfYear =   0
            LCID            =   1033
            SubFormatType   =   1
         EndProperty
         Height          =   375
         Left            =   1470
         TabIndex        =   4
         Top             =   1740
         Width           =   3975
      End
      Begin VB.TextBox txtTStop 
         BeginProperty DataFormat 
            Type            =   1
            Format          =   "0"
            HaveTrueFalseNull=   0
            FirstDayOfWeek  =   0
            FirstWeekOfYear =   0
            LCID            =   1033
            SubFormatType   =   1
         EndProperty
         Height          =   375
         Left            =   1470
         TabIndex        =   5
         Top             =   2220
         Width           =   3975
      End
      Begin VB.TextBox txtMediaLength 
         BeginProperty DataFormat 
            Type            =   1
            Format          =   "0"
            HaveTrueFalseNull=   0
            FirstDayOfWeek  =   0
            FirstWeekOfYear =   0
            LCID            =   1033
            SubFormatType   =   1
         EndProperty
         Height          =   375
         Left            =   1470
         TabIndex        =   6
         Top             =   2700
         Width           =   3975
      End
      Begin VB.TextBox txtStreamNumber 
         BeginProperty DataFormat 
            Type            =   1
            Format          =   "0"
            HaveTrueFalseNull=   0
            FirstDayOfWeek  =   0
            FirstWeekOfYear =   0
            LCID            =   1033
            SubFormatType   =   1
         EndProperty
         Height          =   375
         Left            =   1470
         TabIndex        =   7
         Top             =   3180
         Width           =   3975
      End
      Begin VB.TextBox txtFPS 
         BeginProperty DataFormat 
            Type            =   1
            Format          =   "0"
            HaveTrueFalseNull=   0
            FirstDayOfWeek  =   0
            FirstWeekOfYear =   0
            LCID            =   1033
            SubFormatType   =   1
         EndProperty
         Height          =   375
         Left            =   1470
         TabIndex        =   8
         Top             =   3660
         Width           =   3975
      End
      Begin VB.TextBox txtStretchMode 
         BeginProperty DataFormat 
            Type            =   5
            Format          =   ""
            HaveTrueFalseNull=   1
            TrueValue       =   "1"
            FalseValue      =   "0"
            NullValue       =   ""
            FirstDayOfWeek  =   0
            FirstWeekOfYear =   0
            LCID            =   1033
            SubFormatType   =   7
         EndProperty
         Height          =   375
         Left            =   1470
         TabIndex        =   9
         Top             =   4140
         Width           =   3975
      End
      Begin VB.TextBox txtMuted 
         BeginProperty DataFormat 
            Type            =   5
            Format          =   ""
            HaveTrueFalseNull=   1
            TrueValue       =   "1"
            FalseValue      =   "0"
            NullValue       =   ""
            FirstDayOfWeek  =   0
            FirstWeekOfYear =   0
            LCID            =   1033
            SubFormatType   =   7
         EndProperty
         Height          =   375
         Left            =   1470
         TabIndex        =   10
         Top             =   4620
         Width           =   3975
      End
      Begin VB.CommandButton cmdBrowse 
         Caption         =   "Browse"
         Height          =   375
         Left            =   5670
         TabIndex        =   1
         Top             =   300
         Width           =   975
      End
      Begin VB.Label lblMediaName 
         Caption         =   "MediaName"
         Height          =   255
         Left            =   150
         TabIndex        =   23
         Top             =   420
         Width           =   1095
      End
      Begin VB.Label lblMStart 
         Caption         =   "MStart"
         Height          =   255
         Left            =   150
         TabIndex        =   22
         Top             =   900
         Width           =   735
      End
      Begin VB.Label lblMStop 
         Caption         =   "MStop"
         Height          =   255
         Left            =   150
         TabIndex        =   21
         Top             =   1380
         Width           =   615
      End
      Begin VB.Label lblTStart 
         Caption         =   "TStart"
         Height          =   255
         Left            =   150
         TabIndex        =   20
         Top             =   1860
         Width           =   615
      End
      Begin VB.Label lblTStop 
         Caption         =   "TStop"
         Height          =   255
         Left            =   150
         TabIndex        =   19
         Top             =   2340
         Width           =   615
      End
      Begin VB.Label lblMediaLength 
         Caption         =   "MediaLength"
         Height          =   255
         Left            =   150
         TabIndex        =   18
         Top             =   2820
         Width           =   975
      End
      Begin VB.Label lblStreamNumber 
         Caption         =   "StreamNumber"
         Height          =   255
         Left            =   150
         TabIndex        =   17
         Top             =   3300
         Width           =   1215
      End
      Begin VB.Label lblFPS 
         Caption         =   "FPS"
         Height          =   255
         Left            =   150
         TabIndex        =   16
         Top             =   3780
         Width           =   375
      End
      Begin VB.Label lblStretchMode 
         Caption         =   "StretchMode"
         Height          =   255
         Left            =   150
         TabIndex        =   15
         Top             =   4260
         Width           =   975
      End
      Begin VB.Label lblMuted 
         Caption         =   "Muted"
         Height          =   255
         Left            =   150
         TabIndex        =   14
         Top             =   4740
         Width           =   495
      End
   End
   Begin MSComDlg.CommonDialog dlgCommonDialog2 
      Left            =   75
      Top             =   5250
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
End
Attribute VB_Name = "frmClip"
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

Private m_intUnloadMode As Integer
Private Const DIALOG_TITLE = "Clip Help"


' **************************************************************************************************************************************
' * PUBLIC INTERFACE- PROPERTIES
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: UnloadMode
            ' * procedure description:  Returns an integer specifying the method from which this dialog was last unloaded
            ' *
            ' ******************************************************************************************************************************
            Public Property Get UnloadMode() As Integer
            On Local Error GoTo ErrLine
            'return the value to the client
            UnloadMode = m_intUnloadMode
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
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
             'set default value(s)
            With Me
                .txtMediaName.Text = vbNullString
                .txtMStart.Text = vbNullString
                .txtMStop.Text = vbNullString
                .txtTStart.Text = vbNullString
                .txtTStop.Text = vbNullString
                .txtMediaLength.Text = vbNullString
                .txtStreamNumber.Text = vbNullString
                .txtFPS.Text = vbNullString
                .txtStretchMode.Text = vbNullString
                .txtMuted.Text = vbNullString
            End With
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
                         Cancel = 1: Me.Visible = False
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
            ' * procedure name: cmdOk_Click
            ' * procedure description:  occures when the 'Ok' command button is pressed; proceed to validate user input
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdOk_Click()
            Dim nResultant As VbMsgBoxResult
            On Local Error GoTo ErrLine
            
            'validation code
            If Me.txtMediaName.Text = vbNullString Then
                nResultant = MsgBox("You Must Select A File Name", , DIALOG_TITLE)
                Me.txtMediaName.SetFocus
                Exit Sub
            End If
            If Me.txtMStart.Text = vbNullString Then
                nResultant = MsgBox("You Must Set MStart", , DIALOG_TITLE)
                Me.txtMStart.SetFocus
                Exit Sub
            End If
            If Me.txtMStop.Text = vbNullString Then
                nResultant = MsgBox("You Must Set MStop", , DIALOG_TITLE)
                Me.txtMStop.SetFocus
                Exit Sub
            End If
            If Me.txtTStart.Text = vbNullString Then
                nResultant = MsgBox("You Must Set TStart", , DIALOG_TITLE)
                Me.txtTStart.SetFocus
                Exit Sub
            End If
            If Me.txtTStop.Text = vbNullString Then
                nResultant = MsgBox("You Must Set TStop", , DIALOG_TITLE)
                Me.txtTStop.SetFocus
                Exit Sub
            End If
            If Me.txtMediaLength.Text = vbNullString Then
                nResultant = MsgBox("You Must Set MediaLength", , DIALOG_TITLE)
                Me.txtMediaLength.SetFocus
                Exit Sub
            End If
            If Me.txtStreamNumber.Text = vbNullString Then
                nResultant = MsgBox("You Must Set StreamNumber", , DIALOG_TITLE)
                Me.txtStreamNumber.SetFocus
                Exit Sub
            End If
            If Me.txtFPS.Text = vbNullString Then
                nResultant = MsgBox("You Must Set FPS", , DIALOG_TITLE)
                Me.txtFPS.SetFocus
                Exit Sub
            End If
            If Me.txtStretchMode.Text = vbNullString Then
                nResultant = MsgBox("You Must Set StretchMode", , DIALOG_TITLE)
                Me.txtStretchMode.SetFocus
                Exit Sub
            End If
            If Me.txtMuted.Text = vbNullString Then
                nResultant = MsgBox("You Must Set Muted", , DIALOG_TITLE)
                Me.txtMuted.SetFocus
                Exit Sub
            End If
            If CLng(Me.txtMStart.Text) > CLng(Me.txtMStop.Text) Then
                nResultant = MsgBox("MStart Must Be Lower Then MStop", , DIALOG_TITLE)
                Me.txtMStart.SetFocus
                Exit Sub
            End If
            If CLng(Me.txtTStart.Text) > CLng(Me.txtTStop.Text) Then
                nResultant = MsgBox("TStart Must Be Lower Then TStop", , DIALOG_TITLE)
                Me.txtTStart.SetFocus
                Exit Sub
            End If
            
            'hide the dialog
            Me.Visible = False
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdCancel_Click
            ' * procedure description:  occures when the 'Cancel' command button is pressed
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdCancel_Click()
            On Local Error GoTo ErrLine
            'hide the dialog
            Me.Visible = False
            m_intUnloadMode = 1
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdBrowse_Click
            ' * procedure description:  occures when the 'Browse' command button is pressed
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdBrowse_Click()
            Dim bstrFilename As String
            On Local Error GoTo ErrLine
            
            'request a filename from the user..
            bstrFilename = ShowCommonDlgOpen(, vbNullString, "Media Files(*.asf, *.avi, *.au, *.mpeg, *.mpg, *.wav)")
            'validate the user's input from the common dialog
            If Len(bstrFilename) = 0 Then
               Exit Sub
            Else: Me.txtMediaName.Text = bstrFilename
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
