VERSION 5.00
Begin VB.Form frmTransitions 
   Caption         =   "Transitions Menu"
   ClientHeight    =   3660
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6120
   Icon            =   "frmTransitions.frx":0000
   LinkTopic       =   "Form2"
   LockControls    =   -1  'True
   ScaleHeight     =   3660
   ScaleWidth      =   6120
   StartUpPosition =   1  'CenterOwner
   Begin VB.Frame fraFixture 
      Height          =   3165
      Left            =   75
      TabIndex        =   9
      Top             =   0
      Width           =   5940
      Begin VB.TextBox txtSwapInputs 
         Height          =   375
         Left            =   1830
         TabIndex        =   5
         Top             =   2625
         Width           =   3975
      End
      Begin VB.TextBox txtCutsOnly 
         Height          =   375
         Left            =   1830
         TabIndex        =   4
         Top             =   2145
         Width           =   3975
      End
      Begin VB.TextBox txtCutpoint 
         Height          =   375
         Left            =   1830
         TabIndex        =   3
         Top             =   1665
         Width           =   3975
      End
      Begin VB.TextBox txtStopTime 
         Height          =   375
         Left            =   1830
         TabIndex        =   2
         Top             =   1185
         Width           =   3975
      End
      Begin VB.TextBox txtStartTime 
         Height          =   375
         Left            =   1830
         TabIndex        =   1
         Top             =   705
         Width           =   3975
      End
      Begin VB.ComboBox cmbTransition 
         Height          =   315
         Left            =   1830
         Sorted          =   -1  'True
         TabIndex        =   0
         Top             =   225
         Width           =   3975
      End
      Begin VB.Label lblSwapInputs 
         Caption         =   "Swap Inputs"
         Height          =   255
         Left            =   150
         TabIndex        =   15
         Top             =   2745
         Width           =   975
      End
      Begin VB.Label lblCutsOnly 
         Caption         =   "Cuts Only"
         Height          =   255
         Left            =   150
         TabIndex        =   14
         Top             =   2265
         Width           =   1095
      End
      Begin VB.Label lblCutpoint 
         Caption         =   "Cutpoint"
         Height          =   255
         Left            =   150
         TabIndex        =   13
         Top             =   1785
         Width           =   1335
      End
      Begin VB.Label lblStop 
         Caption         =   "Stop"
         Height          =   255
         Left            =   150
         TabIndex        =   12
         Top             =   1305
         Width           =   615
      End
      Begin VB.Label lblStart 
         Caption         =   "Start"
         Height          =   255
         Left            =   150
         TabIndex        =   11
         Top             =   825
         Width           =   735
      End
      Begin VB.Label lblTransition 
         Caption         =   "Transition"
         Height          =   255
         Left            =   150
         TabIndex        =   10
         Top             =   345
         Width           =   1695
      End
   End
   Begin VB.CommandButton cmdOk 
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   340
      Left            =   3615
      TabIndex        =   6
      Top             =   3255
      Width           =   1095
   End
   Begin VB.CommandButton cmdCancel 
      Caption         =   "Cancel"
      Height          =   340
      Left            =   4815
      TabIndex        =   7
      Top             =   3255
      Width           =   1215
   End
   Begin VB.TextBox GetSubObjectGUIDB 
      Height          =   420
      Left            =   6300
      TabIndex        =   8
      Top             =   75
      Visible         =   0   'False
      Width           =   915
   End
End
Attribute VB_Name = "frmTransitions"
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
Private Const DIALOG_TITLE = "Transition Help"


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
            With cmbTransition
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
            ' * procedure description:  occures when the user presses the 'Ok' button
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdOk_Click()
            On Local Error GoTo ErrLine
            'hide dialog
            Me.Visible = False
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdOk_Click
            ' * procedure description:  occures when the user presses the 'Cancel' button
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdCancel_Click()
            On Local Error GoTo ErrLine
            'hide dialog
            Me.Visible = False
            m_intUnloadMode = 1
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
