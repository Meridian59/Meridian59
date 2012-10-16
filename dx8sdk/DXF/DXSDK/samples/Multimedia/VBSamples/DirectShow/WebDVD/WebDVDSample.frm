VERSION 5.00
Object = "{38EE5CE1-4B62-11D3-854F-00A0C9C898E7}#1.0#0"; "mswebdvd.dll"
Begin VB.Form frmWebDVDSample 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "WebDVD Sample"
   ClientHeight    =   3930
   ClientLeft      =   -19140
   ClientTop       =   345
   ClientWidth     =   6255
   Icon            =   "WebDVDSample.frx":0000
   LinkTopic       =   "Form1"
   LockControls    =   -1  'True
   MaxButton       =   0   'False
   ScaleHeight     =   3930
   ScaleWidth      =   6255
   StartUpPosition =   2  'CenterScreen
   Begin VB.Frame fraMenus 
      Height          =   3855
      Left            =   60
      TabIndex        =   10
      Top             =   0
      Width           =   6135
      Begin VB.CommandButton cmdResume 
         Caption         =   "Resume"
         Height          =   315
         Left            =   4545
         TabIndex        =   9
         ToolTipText     =   "Resume Playback"
         Top             =   3420
         Width           =   1470
      End
      Begin VB.CommandButton cmdShowMenu 
         Caption         =   "Show Menu"
         Height          =   315
         Left            =   4545
         TabIndex        =   8
         ToolTipText     =   "Show Menu"
         Top             =   3120
         Width           =   1470
      End
      Begin VB.ListBox lstMenus 
         Height          =   1230
         ItemData        =   "WebDVDSample.frx":0442
         Left            =   3840
         List            =   "WebDVDSample.frx":0444
         TabIndex        =   0
         ToolTipText     =   "Select a Menu"
         Top             =   1440
         Width           =   2175
      End
      Begin VB.CommandButton cmdPlay 
         Caption         =   "Play"
         Height          =   315
         Left            =   120
         TabIndex        =   1
         ToolTipText     =   "Play"
         Top             =   3120
         Width           =   975
      End
      Begin VB.CommandButton cmdStop 
         Caption         =   "Stop"
         Height          =   315
         Left            =   1080
         TabIndex        =   2
         ToolTipText     =   "Stop"
         Top             =   3120
         Width           =   975
      End
      Begin VB.CommandButton cmdPause 
         Caption         =   "Pause"
         Height          =   315
         Left            =   2040
         TabIndex        =   3
         ToolTipText     =   "Pause"
         Top             =   3120
         Width           =   975
      End
      Begin VB.CommandButton cmdEject 
         Caption         =   "Eject"
         Height          =   315
         Left            =   3000
         TabIndex        =   4
         ToolTipText     =   "Eject"
         Top             =   3120
         Width           =   1095
      End
      Begin VB.CommandButton cmdActivateButton 
         Caption         =   "Activate Button"
         Height          =   315
         Left            =   120
         TabIndex        =   5
         ToolTipText     =   "Activate button"
         Top             =   3420
         Width           =   1335
      End
      Begin VB.CommandButton cmdPlayPrevChapter 
         Caption         =   "Last Chapter"
         Height          =   315
         Left            =   1425
         TabIndex        =   6
         ToolTipText     =   "Play previous chapter"
         Top             =   3420
         Width           =   1345
      End
      Begin VB.CommandButton cmdPlayNextChapter 
         Caption         =   "Next Chapter"
         Height          =   315
         Left            =   2775
         TabIndex        =   7
         ToolTipText     =   "Play next chapter"
         Top             =   3420
         Width           =   1325
      End
      Begin MSWEBDVDLibCtl.MSWebDVD MSWebDVD1 
         Height          =   2495
         Left            =   120
         TabIndex        =   11
         Top             =   180
         Width           =   3615
         _cx             =   6376
         _cy             =   4401
         DisableAutoMouseProcessing=   0   'False
         BackColor       =   1048592
         EnableResetOnStop=   0   'False
         ColorKey        =   1048592
         WindowlessActivation=   0   'False
      End
      Begin VB.Line Line1 
         X1              =   3840
         X2              =   6000
         Y1              =   1080
         Y2              =   1080
      End
      Begin VB.Label lblDescriptor 
         Caption         =   "This sample demonstrates the use of the Microsoft WebDVD control within a Visual Basic playback application."
	 Height          =   795
         Left            =   3840
         TabIndex        =   15
         Top             =   180
         Width           =   2175
      End
      Begin VB.Label lblChoices 
         Caption         =   "Menu Choices:"
         Height          =   195
         Left            =   3840
         TabIndex        =   14
         Top             =   1200
         Width           =   2115
      End
      Begin VB.Label lblTimeTracker 
         Caption         =   "Time Tracker:"
         Height          =   195
         Left            =   120
         TabIndex        =   13
         Top             =   2820
         Width           =   1335
      End
      Begin VB.Label lblTimeTrackerValue 
         Height          =   195
         Left            =   1560
         TabIndex        =   12
         Top             =   2820
         Width           =   2160
      End
   End
End

Attribute VB_Name = "frmWebDVDSample"
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
            
            With lstMenus
               .AddItem "RootMenu", 0
               .AddItem "TitleMenu", 1
               .AddItem "AudioMenu", 2
               .AddItem "AngleMenu", 3
               .AddItem "ChapterMenu", 4
               .AddItem "SubpictureMenu", 5
            End With
            
            'set the root menu selected
            lstMenus.Selected(0) = True
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
                 Case 0 'vbFormControlMenu
                             Me.Move Screen.Width * 8, Screen.Height * 8
                             Me.Visible = False: Call MSWebDVD1.Stop
                 Case 1 'vbFormCode
                             Me.Move Screen.Width * 8, Screen.Height * 8
                             Me.Visible = False: Call MSWebDVD1.Stop
                 Case 2 'vbAppWindows
                             Me.Move Screen.Width * 8, Screen.Height * 8
                             Me.Visible = False: Call MSWebDVD1.Stop
                 Case 3 'vbAppTaskManager
                             Me.Move Screen.Width * 8, Screen.Height * 8
                             Me.Visible = False: Call MSWebDVD1.Stop
                 Case 4 'vbFormMDIForm
                             Exit Sub
                 Case 5 'vbFormOwner
                             Exit Sub
            End Select
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
            ' * procedure name: cmdPlay_Click
            ' * procedure description:  Occurs when the user clicks the "Play" command button
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdPlay_Click()
            On Local Error GoTo ErrLine
            
            'Start playback
            Call MSWebDVD1.Play
            Exit Sub
            
ErrLine:
            Call MsgBox(Err.Description, vbOKOnly + vbExclamation + vbApplicationModal, App.Title): Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdStop_Click
            ' * procedure description:  Occurs when the user clicks the "Stop" command button
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdStop_Click()
            On Local Error GoTo ErrLine
            
            'stop playback
            Call MSWebDVD1.Stop
            Exit Sub
            
ErrLine:
            Call MsgBox(Err.Description, vbOKOnly + vbExclamation + vbApplicationModal, App.Title): Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdPause_Click
            ' * procedure description:  Occurs when the user clicks the "Pause" command button
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdPause_Click()
            On Local Error GoTo ErrLine

            'pause playback
            Call MSWebDVD1.Pause
            Exit Sub
            
ErrLine:
            Call MsgBox(Err.Description, vbOKOnly + vbExclamation + vbApplicationModal, App.Title): Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdEject_Click
            ' * procedure description:  Occurs when the user clicks the "Eject" command button
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdEject_Click()
            On Local Error GoTo ErrLine
            
            'Eject disc from the drive
            Call MSWebDVD1.Eject
            Exit Sub
            
ErrLine:
            Call MsgBox(Err.Description, vbOKOnly + vbExclamation + vbApplicationModal, App.Title): Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdActivateButton_Click
            ' * procedure description:  Occurs when the user clicks the "ActivateButton" command button
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdActivateButton_Click()
            On Local Error GoTo ErrLine

            'activates the currently selected button (selected button is highlighted)
            Call MSWebDVD1.ActivateButton
            Exit Sub
            
ErrLine:
            Call MsgBox(Err.Description, vbOKOnly + vbExclamation + vbApplicationModal, App.Title): Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdPlayNextChapter_Click
            ' * procedure description:  Occurs when the user clicks the "PlayNextChapter" command button
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdPlayNextChapter_Click()
            On Local Error GoTo ErrLine

            'takes playback to next chapter within current title
            Call MSWebDVD1.PlayNextChapter
            Exit Sub
            
ErrLine:
            Call MsgBox(Err.Description, vbOKOnly + vbExclamation + vbApplicationModal, App.Title): Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdPlayPrevChapter_Click
            ' * procedure description:  Occurs when the user clicks the "PlayPrevChapter" command button
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdPlayPrevChapter_Click()
            On Local Error GoTo ErrLine

            'takes playback to previous chapter within current title
            Call MSWebDVD1.PlayPrevChapter
            Exit Sub
            
ErrLine:
            Call MsgBox(Err.Description, vbOKOnly + vbExclamation + vbApplicationModal, App.Title): Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdShowMenu_Click
            ' * procedure description:  Occurs when the user clicks the "ShowMenu" command button
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdShowMenu_Click()
            On Local Error GoTo ErrLine
            
                Select Case lstMenus.ListIndex
                    Case 0: Call MSWebDVD1.ShowMenu(3)  'Root
                    Case 1: Call MSWebDVD1.ShowMenu(2)  'Title
                    Case 2: Call MSWebDVD1.ShowMenu(5)  'Audio
                    Case 3: Call MSWebDVD1.ShowMenu(6)  'Angle
                    Case 4: Call MSWebDVD1.ShowMenu(7)  'Chapter
                    Case 5: Call MSWebDVD1.ShowMenu(4)  'Subpicture
                End Select
            Exit Sub
            
ErrLine:
            Call MsgBox(Err.Description, vbOKOnly + vbExclamation + vbApplicationModal, App.Title): Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdResume_Click
            ' * procedure description:  Occurs when the user clicks the "Resume" command button
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdResume_Click()
            On Local Error GoTo ErrLine

                ' Resume playback
                Call MSWebDVD1.Resume
            Exit Sub
            
ErrLine:
            Call MsgBox(Err.Description, vbOKOnly + vbExclamation + vbApplicationModal, App.Title): Err.Clear
            Exit Sub
            End Sub

            

            ' ******************************************************************************************************************************
            ' * procedure name: MSWebDVD1_DVDNotify
            ' * procedure description:  DVD notification event- occurs when a notification arrives from the dvd control
            ' *
            ' ******************************************************************************************************************************
            Private Sub MSWebDVD1_DVDNotify(ByVal lEventCode As Long, ByVal lParam1 As Variant, ByVal lParam2 As Variant)
            On Local Error GoTo ErrLine
            
            If 282 = lEventCode Then '282 is the event code for the time event
               'pass in param1 to get you the current time-convert to hh:mm:ss:ff format with DVDTimeCode2BSTR API
               If lblTimeTrackerValue.Caption <> CStr(MSWebDVD1.DVDTimeCode2bstr(lParam1)) Then _
                  lblTimeTrackerValue.Caption = CStr(MSWebDVD1.DVDTimeCode2bstr(lParam1))
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
