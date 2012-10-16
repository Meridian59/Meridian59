VERSION 5.00
Begin VB.Form frmTrack 
   Caption         =   "Track Menu"
   ClientHeight    =   3270
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   5805
   Icon            =   "frmTrack.frx":0000
   LinkTopic       =   "Form2"
   LockControls    =   -1  'True
   ScaleHeight     =   3270
   ScaleWidth      =   5805
   StartUpPosition =   1  'CenterOwner
   Begin VB.Frame fraFixture 
      Height          =   2715
      Left            =   75
      TabIndex        =   7
      Top             =   0
      Width           =   5640
      Begin VB.TextBox txtPriority 
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
         TabIndex        =   0
         Top             =   225
         Width           =   3975
      End
      Begin VB.TextBox txtBlank 
         BeginProperty DataFormat 
            Type            =   5
            Format          =   "0"
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
         TabIndex        =   1
         Top             =   705
         Width           =   3975
      End
      Begin VB.TextBox txtMuted 
         BeginProperty DataFormat 
            Type            =   5
            Format          =   "0"
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
         TabIndex        =   2
         Top             =   1185
         Width           =   3975
      End
      Begin VB.TextBox txtStartTime 
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
         Top             =   1665
         Width           =   3975
      End
      Begin VB.TextBox txtStopTime 
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
         Top             =   2145
         Width           =   3975
      End
      Begin VB.Label lblPriority 
         Caption         =   "Priority"
         Height          =   255
         Left            =   150
         TabIndex        =   12
         Top             =   345
         Width           =   1095
      End
      Begin VB.Label lblBlank 
         Caption         =   "Blank"
         Height          =   255
         Left            =   150
         TabIndex        =   11
         Top             =   825
         Width           =   1095
      End
      Begin VB.Label lblMuted 
         Caption         =   "Muted"
         Height          =   255
         Left            =   150
         TabIndex        =   10
         Top             =   1305
         Width           =   615
      End
      Begin VB.Label lblStart 
         Caption         =   "Start"
         Height          =   255
         Left            =   150
         TabIndex        =   9
         Top             =   1785
         Width           =   1215
      End
      Begin VB.Label lblStop 
         Caption         =   "Stop"
         Height          =   255
         Left            =   150
         TabIndex        =   8
         Top             =   2265
         Width           =   1215
      End
   End
   Begin VB.CommandButton cmdOk 
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   340
      Left            =   3300
      TabIndex        =   5
      Top             =   2850
      Width           =   1095
   End
   Begin VB.CommandButton cmdCancel 
      Caption         =   "Cancel"
      Height          =   340
      Left            =   4500
      TabIndex        =   6
      Top             =   2850
      Width           =   1215
   End
End
Attribute VB_Name = "frmTrack"
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
Private Const DIALOG_TITLE = "Track Help"


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
                .txtPriority.Text = vbNullString
                .txtBlank.Text = vbNullString
                .txtMuted.Text = vbNullString
                .txtStartTime.Text = vbNullString
                .txtStopTime.Text = vbNullString
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
            ' * procedure description:  occures when the 'Ok' command button is pressed
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdOk_Click()
            Dim nResultant As VbMsgBoxResult
            On Local Error GoTo ErrLine
            
            'validation code
            If frmTrack.txtPriority.Text = vbNullString Then
                nResultant = MsgBox("You Must Set Priority", , DIALOG_TITLE)
                frmTrack.txtPriority.SetFocus
                Exit Sub
            End If
            If frmTrack.txtBlank.Text = vbNullString Then
                nResultant = MsgBox("You Must Set Blank", , DIALOG_TITLE)
                frmTrack.txtBlank.SetFocus
                Exit Sub
            End If
            If frmTrack.txtMuted.Text = vbNullString Then
                nResultant = MsgBox("You Must Set Muted", , DIALOG_TITLE)
                frmTrack.txtMuted.SetFocus
                Exit Sub
            End If
            If frmTrack.txtStartTime.Text = vbNullString Then
                nResultant = MsgBox("You Must Set Start", , DIALOG_TITLE)
                frmTrack.txtStartTime.SetFocus
                Exit Sub
            End If
            If frmTrack.txtStopTime.Text = vbNullString Then
                nResultant = MsgBox("You Must Set Stop", , DIALOG_TITLE)
                frmTrack.txtStopTime.SetFocus
                Exit Sub
            End If
            If CLng(frmTrack.txtStartTime.Text) > CLng(frmTrack.txtStopTime.Text) Then
                nResultant = MsgBox("Start Must Be Lower Than Stop", , DIALOG_TITLE)
                frmTrack.txtStartTime.SetFocus
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
            ' * procedure description:  occures when the user presses the "Cancel" button
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
