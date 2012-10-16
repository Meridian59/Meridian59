VERSION 5.00
Begin VB.Form frmTimeline 
   Caption         =   "TimeLine Menu"
   ClientHeight    =   4605
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6015
   Icon            =   "frmTimeline.frx":0000
   LinkTopic       =   "Form2"
   LockControls    =   -1  'True
   ScaleHeight     =   4605
   ScaleWidth      =   6015
   StartUpPosition =   1  'CenterOwner
   Begin VB.Frame fraFixture 
      Height          =   4065
      Left            =   75
      TabIndex        =   12
      Top             =   0
      Width           =   5865
      Begin VB.TextBox txtDirty 
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
         Left            =   1710
         TabIndex        =   5
         ToolTipText     =   "Not Setable By User / read Only"
         Top             =   2625
         Width           =   3975
      End
      Begin VB.TextBox txtDefaultFPS 
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
         Left            =   1710
         TabIndex        =   4
         ToolTipText     =   "Default Frames Per Second"
         Top             =   2145
         Width           =   3975
      End
      Begin VB.TextBox txtDuration 
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
         Left            =   1710
         TabIndex        =   3
         ToolTipText     =   "Not Setable By User / read Only"
         Top             =   1665
         Width           =   3975
      End
      Begin VB.TextBox txtEffectsEnabled 
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
         Left            =   1710
         TabIndex        =   2
         ToolTipText     =   "0 Disabled / 1 Enabled"
         Top             =   1185
         Width           =   3975
      End
      Begin VB.TextBox txtTransitionsEnabled 
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
         Left            =   1710
         TabIndex        =   1
         ToolTipText     =   "0 Disabled / 1 Enabled"
         Top             =   705
         Width           =   3975
      End
      Begin VB.TextBox txtInsertMode 
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
         Left            =   1710
         TabIndex        =   0
         ToolTipText     =   "1 Insert / 2 Overlay"
         Top             =   225
         Width           =   3975
      End
      Begin VB.ComboBox cmbDefaultTransition 
         Height          =   315
         Left            =   1710
         Sorted          =   -1  'True
         TabIndex        =   6
         Top             =   3105
         Width           =   3975
      End
      Begin VB.ComboBox cmbDefaultEffect 
         Height          =   315
         Left            =   1710
         Sorted          =   -1  'True
         TabIndex        =   7
         Top             =   3585
         Width           =   3975
      End
      Begin VB.Label lblDefaultEffect 
         Caption         =   "DefaultEffect"
         Height          =   255
         Left            =   150
         TabIndex        =   20
         Top             =   3705
         Width           =   1095
      End
      Begin VB.Label lblDefaultTransition 
         Caption         =   "DefaultTransition"
         Height          =   255
         Left            =   150
         TabIndex        =   19
         Top             =   3225
         Width           =   1455
      End
      Begin VB.Label lblDirty 
         Caption         =   "Dirty"
         Height          =   255
         Left            =   150
         TabIndex        =   18
         Top             =   2745
         Width           =   975
      End
      Begin VB.Label lblDefaultFPS 
         Caption         =   "DefaultFPS"
         Height          =   255
         Left            =   150
         TabIndex        =   17
         Top             =   2265
         Width           =   1575
      End
      Begin VB.Label lblDuration 
         Caption         =   "Duration"
         Height          =   255
         Left            =   150
         TabIndex        =   16
         Top             =   1785
         Width           =   615
      End
      Begin VB.Label lblEffectsEnable 
         Caption         =   "EffectsEnable"
         Height          =   255
         Left            =   150
         TabIndex        =   15
         Top             =   1305
         Width           =   1935
      End
      Begin VB.Label lblTransitionsEnable 
         Caption         =   "TransitionsEnable"
         Height          =   255
         Left            =   150
         TabIndex        =   14
         Top             =   825
         Width           =   1575
      End
      Begin VB.Label lblInsertMode 
         Caption         =   "InsertMode"
         Height          =   255
         Left            =   150
         TabIndex        =   13
         Top             =   345
         Width           =   1095
      End
   End
   Begin VB.CommandButton cmdCancel 
      Caption         =   "Cancel"
      Height          =   340
      Left            =   4695
      TabIndex        =   9
      Top             =   4170
      Width           =   1215
   End
   Begin VB.CommandButton cmdOk 
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   340
      Left            =   3495
      TabIndex        =   8
      Top             =   4170
      Width           =   1095
   End
   Begin VB.TextBox DefaultTransition 
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
      Left            =   7200
      TabIndex        =   10
      TabStop         =   0   'False
      Top             =   150
      Visible         =   0   'False
      Width           =   975
   End
   Begin VB.TextBox DefaultEffect 
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
      Left            =   7200
      TabIndex        =   11
      TabStop         =   0   'False
      Top             =   600
      Visible         =   0   'False
      Width           =   975
   End
End
Attribute VB_Name = "frmTimeline"
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
Private Const DIALOG_TITLE = "Timeline Help"
            
            
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
                .txtInsertMode.Text = vbNullString
                .txtTransitionsEnabled.Text = vbNullString
                .txtEffectsEnabled.Text = vbNullString
                .txtDuration.Text = vbNullString
                .txtDefaultFPS.Text = vbNullString
                .txtDirty.Text = vbNullString
                .cmbDefaultTransition.Text = vbNullString
                .cmbDefaultEffect.Text = vbNullString
            End With
            
            With cmbDefaultTransition
                .AddItem "Barn"
                .AddItem "Blinds"
                .AddItem "BurnFilm"
                .AddItem "CenterCurls"
                .AddItem "ColorFade"
                .AddItem "Compositor"
                .AddItem "Curls"
                .AddItem "Curtains"
                .AddItem "Fade"
                .AddItem "FadeWhite"
                .AddItem "FlowMotion"
                .AddItem "GlassBlock"
                .AddItem "Grid"
                .AddItem "Inset"
                .AddItem "Iris"
                .AddItem "Jaws"
                .AddItem "Lens"
                .AddItem "LightWipe"
                .AddItem "Liquid"
                .AddItem "PageCurl"
                .AddItem "PeelABCD"
                .AddItem "Pixelate"
                .AddItem "RadialWipe"
                .AddItem "Ripple"
                .AddItem "RollDown"
                .AddItem "Slide"
                .AddItem "SMPTE Wipe"
                .AddItem "Spiral"
                .AddItem "Stretch"
                .AddItem "Threshold"
                .AddItem "Twister"
                .AddItem "Vacuum"
                .AddItem "Water"
                .AddItem "Wheel"
                .AddItem "Wipe"
                .AddItem "WormHole"
                .AddItem "Zigzag"
            End With
            
            With cmbDefaultEffect
                .AddItem "BasicImage"
                .AddItem "Blur"
                .AddItem "Chroma"
                .AddItem "DropShadow"
                .AddItem "Emboss"
                .AddItem "Engrave"
                .AddItem "Fade"
                .AddItem "Pixelate"
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
            ' * procedure description:  fired intrinsically when this form is loaded
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdOk_Click()
            Dim nResultant As VbMsgBoxResult
            On Local Error GoTo ErrLine
            
            'validation code
            If frmTimeline.txtInsertMode.Text = vbNullString Then
                nResultant = MsgBox("You Must Set txtInsertMode", , DIALOG_TITLE)
                frmTimeline.txtInsertMode.SetFocus
                Exit Sub
            End If
            If frmTimeline.txtTransitionsEnabled.Text = vbNullString Then
                nResultant = MsgBox("You Must Set txtTransitionsEnabled", , DIALOG_TITLE)
                frmTimeline.txtTransitionsEnabled.SetFocus
                Exit Sub
            End If
            If frmTimeline.txtEffectsEnabled.Text = vbNullString Then
                nResultant = MsgBox("You Must Set txtEffectsEnabled", , DIALOG_TITLE)
                frmTimeline.txtEffectsEnabled.SetFocus
                Exit Sub
            End If
            If frmTimeline.txtDefaultFPS.Text = vbNullString Then
                nResultant = MsgBox("You Must Set txtDefaultFPS", , DIALOG_TITLE)
                frmTimeline.txtDefaultFPS.SetFocus
                Exit Sub
            End If
            If frmTimeline.cmbDefaultTransition.Text = vbNullString Then
                nResultant = MsgBox("You Must Set cmbDefaultTransition", , DIALOG_TITLE)
                frmTimeline.cmbDefaultTransition.SetFocus
                Exit Sub
            End If
            If frmTimeline.cmbDefaultEffect.Text = vbNullString Then
                nResultant = MsgBox("You Must Set cmbDefaultEffect", , DIALOG_TITLE)
                frmTimeline.cmbDefaultEffect.SetFocus
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
            ' * procedure description:  occures when the user presses the 'Cancel' command button
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
