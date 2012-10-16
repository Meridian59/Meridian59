VERSION 5.00
Begin VB.Form frmMain 
   BackColor       =   &H00000000&
   BorderStyle     =   4  'Fixed ToolWindow
   Caption         =   "Club Metamorphous"
   ClientHeight    =   7140
   ClientLeft      =   3510
   ClientTop       =   1890
   ClientWidth     =   8310
   ForeColor       =   &H0000C000&
   Icon            =   "ClubMet.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   476
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   554
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdExit 
      BackColor       =   &H0080FF80&
      Cancel          =   -1  'True
      Caption         =   "Exit"
      Height          =   495
      Left            =   240
      TabIndex        =   12
      Top             =   6600
      Width           =   1215
   End
   Begin VB.CommandButton cmdAdmission 
      BackColor       =   &H00FFC0FF&
      Caption         =   "Admission"
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   240
      Style           =   1  'Graphical
      TabIndex        =   10
      Top             =   6000
      Width           =   1215
   End
   Begin VB.CommandButton cmdSpecials 
      BackColor       =   &H008080FF&
      Caption         =   "Dinner Specials"
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   240
      Style           =   1  'Graphical
      TabIndex        =   9
      Top             =   5400
      Width           =   1215
   End
   Begin VB.CommandButton cmdDirections 
      BackColor       =   &H0080C0FF&
      Caption         =   "Directions"
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   9.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   495
      Left            =   240
      Style           =   1  'Graphical
      TabIndex        =   8
      Top             =   4800
      Width           =   1215
   End
   Begin VB.PictureBox mnCan 
      BackColor       =   &H80000007&
      BorderStyle     =   0  'None
      Height          =   3795
      Left            =   2400
      ScaleHeight     =   253
      ScaleMode       =   3  'Pixel
      ScaleWidth      =   385
      TabIndex        =   7
      Top             =   1680
      Width           =   5775
   End
   Begin VB.Label lblStuff 
      BackColor       =   &H80000007&
      Caption         =   "Label2"
      ForeColor       =   &H8000000E&
      Height          =   1455
      Left            =   2340
      TabIndex        =   11
      Top             =   5580
      Width           =   5835
   End
   Begin VB.Label lblSunday 
      AutoSize        =   -1  'True
      BackColor       =   &H00000000&
      Caption         =   "Sunday"
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   20.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H0000C000&
      Height          =   465
      Left            =   240
      TabIndex        =   6
      Top             =   4200
      Width           =   1305
   End
   Begin VB.Label lblSaturday 
      AutoSize        =   -1  'True
      BackColor       =   &H00000000&
      Caption         =   "Saturday"
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   20.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H0000C000&
      Height          =   465
      Left            =   240
      TabIndex        =   5
      Top             =   3600
      Width           =   1605
   End
   Begin VB.Label lblFriday 
      AutoSize        =   -1  'True
      BackColor       =   &H00000000&
      Caption         =   "Friday"
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   20.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H0000C000&
      Height          =   465
      Left            =   240
      TabIndex        =   4
      Top             =   3000
      Width           =   1185
   End
   Begin VB.Label lblThursday 
      AutoSize        =   -1  'True
      BackColor       =   &H00000000&
      Caption         =   "Thursday"
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   20.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H0000C000&
      Height          =   465
      Left            =   240
      TabIndex        =   3
      Top             =   2400
      Width           =   1695
   End
   Begin VB.Label lblWednesday 
      AutoSize        =   -1  'True
      BackColor       =   &H00000000&
      Caption         =   "Wednesday"
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   20.25
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H0000FF00&
      Height          =   465
      Left            =   240
      TabIndex        =   2
      Top             =   1800
      Width           =   2025
   End
   Begin VB.Label lblName 
      Alignment       =   2  'Center
      BackColor       =   &H00000000&
      Caption         =   "Club Metamorphous"
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   36
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H000080FF&
      Height          =   915
      Left            =   480
      TabIndex        =   1
      Top             =   0
      Width           =   7455
   End
   Begin VB.Label Label1 
      BackColor       =   &H00000000&
      Caption         =   """The only thing that stays the same is a good time!"""
      BeginProperty Font 
         Name            =   "Times New Roman"
         Size            =   15.75
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   -1  'True
         Strikethrough   =   0   'False
      EndProperty
      ForeColor       =   &H000080FF&
      Height          =   495
      Left            =   840
      TabIndex        =   0
      Top             =   1020
      Width           =   6855
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       ClubMet.frm
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

'This application uses conditional compilation.  To run this sample in the IDE, you
'must first go to Project Properties (Project Menu-> Properties).  Then on the Make tab
'change the RunInIDE=0 to RunInIDE=1.

'This sample also shows developers how to combine the DX7 and DX8 DLL's to create
'an app with the latest DMusic and still use older functionality like DDraw

Private dx As New DXVBLibA.DirectX8
Dim day As Integer
Dim sJazz As DXVBLibA.DirectMusicStyle8
Dim sDance As DXVBLibA.DirectMusicStyle8
Dim sBigBand As DXVBLibA.DirectMusicStyle8
Dim sDisco As DXVBLibA.DirectMusicStyle8
Dim sClassical As DXVBLibA.DirectMusicStyle8
Dim sHeartland As DXVBLibA.DirectMusicStyle8
Dim cmp As DXVBLibA.DirectMusicChordMap8
Dim com As DXVBLibA.DirectMusicComposer8
Dim perf As DXVBLibA.DirectMusicPerformance8
Dim seg As DXVBLibA.DirectMusicSegment8
Dim loader As DXVBLibA.DirectMusicLoader8
Dim currentstyle As DXVBLibA.DirectMusicStyle8
Dim LabelNumber As Integer
Dim runit As Boolean

Private Sub cmdAdmission_Click()
     Call perf.PlaySegmentEx(currentstyle.GetMotif(currentstyle.GetMotifName(2)), DMUS_SEGF_SECONDARY Or DMUS_SEGF_BEAT, 0)
     lblStuff.Caption = ChangeStuffLabel(6)
End Sub

Private Sub cmdDirections_Click()
    
    Call perf.PlaySegmentEx(currentstyle.GetMotif(currentstyle.GetMotifName(0)), DMUS_SEGF_SECONDARY Or DMUS_SEGF_BEAT, 0)
    lblStuff.Caption = ChangeStuffLabel(0)
End Sub

Private Sub cmdExit_Click()
    runit = False
    Unload Me
End Sub

Private Sub cmdSpecials_Click()
     Call perf.PlaySegmentEx(currentstyle.GetMotif(currentstyle.GetMotifName(1)), DMUS_SEGF_SECONDARY Or DMUS_SEGF_BEAT, 0)
    lblStuff.Caption = ChangeStuffLabel(LabelNumber)
End Sub


Private Function ChangeStuffLabel(Index As Integer) As String
    Dim tString(9) As String
    
    Call ClearlblStuff
    
    'directions
    tString(0) = "Corner of 4th and Stewart, next to the new stadium!"
    
    'dinners
    tString(1) = "London Broil with Hollandaise sauce, baby red potatoes, green vegetables, and Lobster Bisque soup."
    tString(2) = "Grilled Mahi-Mahi on a bed of rice pilaf, green vegetables, and Ceasar salad"
    tString(3) = "Chicken Cordon Bleu, steamed vegetables, wild lemon rice, and clam chowder"
    tString(4) = "Bacon CheeseBurger, onion rings, and a vanilla shake"
    tString(5) = "Salmon in parchment, rice pilaf, green vegetables, and lentil soup."
    
    'Admission
    tString(6) = "Age 14 - 18, $4.50, age 19 and up, $7.00"
    
    ChangeStuffLabel = tString(Index)

End Function

Private Sub ClearlblStuff()
    lblStuff.Caption = ""
End Sub

Private Sub Form_Load()
    On Error GoTo err_out
    Show
    
    ClearlblStuff
    
    InitDD hwnd, mnCan
    DoEvents
    initDMusic
    DoEvents
    

    runit = True
    
    Do
        MoveFrame day
        DoEvents
    Loop
    
    End
err_out:
    MsgBox "Could not start application!", vbApplicationModal
    End
        
End Sub
Private Sub initDMusic()
    Dim dma As DMUS_AUDIOPARAMS
    
    On Error GoTo FailedInit
    Set perf = dx.DirectMusicPerformanceCreate
    Set com = dx.DirectMusicComposerCreate
    Set loader = dx.DirectMusicLoaderCreate

    perf.InitAudio Me.hwnd, DMUS_AUDIOF_ALL, dma, , DMUS_APATH_SHARED_STEREOPLUSREVERB, 128
    perf.SetMasterAutoDownload True

'Load the objects
#If RunInIDE = 1 Then
    Dim sMedia As String
    
    sMedia = FindMediaDir("bigband.sty")
    If sMedia <> vbNullString Then 'Media is not in current folder
        If (Left$(sMedia, 2) <> Left$(CurDir, 2)) And (InStr(Left$(sMedia, 2), ":") > 0) Then ChDrive Left$(sMedia, 2)
        ChDir sMedia
    End If
    
    Set sBigBand = loader.LoadStyle("BIGBAND.STY")
    Set sJazz = loader.LoadStyle("JAZZ.STY")
    Set sDisco = loader.LoadStyle("DISCO.STY")
    Set sClassical = loader.LoadStyle("CLASSICAL.STY")
    Set sDance = loader.LoadStyle("DANCEMIX.STY")
    Set sHeartland = loader.LoadStyle("HEARTLAND.STY")

    Set currentstyle = sHeartland
    Set cmp = loader.LoadChordMap("CHORDMAP.CDM")
#Else
    Set sBigBand = loader.LoadStyleFromResource(AddDirSep(App.Path) & App.EXEName & ".exe", "BIGBAND")
    Set sJazz = loader.LoadStyleFromResource(AddDirSep(App.Path) & App.EXEName & ".exe", "JAZZ")
    Set sDisco = loader.LoadStyleFromResource(AddDirSep(App.Path) & App.EXEName & ".exe", "DISCO")
    Set sClassical = loader.LoadStyleFromResource(AddDirSep(App.Path) & App.EXEName & ".exe", "CLASSICAL")
    Set sDance = loader.LoadStyleFromResource(AddDirSep(App.Path) & App.EXEName & ".exe", "DANCEMIX")
    Set sHeartland = loader.LoadStyleFromResource(AddDirSep(App.Path) & App.EXEName & ".exe", "HEARTLAND")

    Set currentstyle = sHeartland
    Set cmp = loader.LoadChordMapFromResource(AddDirSep(App.Path) & App.EXEName & ".exe", "CHORDMAP")
#End If
    Set seg = com.ComposeSegmentFromShape(sHeartland, 64, 0, 1, True, False, cmp)
    Call perf.PlaySegmentEx(seg, 0, 0)
    Exit Sub
    
FailedInit:
    MsgBox "Could not initialize DirectMusic." & vbCrLf & "This sample will exit.", vbOKOnly Or vbInformation, "Exiting..."
    Unload Me
   
End Sub
Private Sub ChangeMusic()
    Set seg = com.ComposeSegmentFromShape(currentstyle, 64, 0, 2, False, False, cmp)
    Call com.AutoTransition(perf, seg, DMUS_COMMANDT_FILL, DMUS_COMPOSEF_MEASURE, cmp)
End Sub

Private Sub Form_QueryUnload(Cancel As Integer, UnloadMode As Integer)
    runit = False
End Sub

Private Sub Form_Unload(Cancel As Integer)
    If Not (perf Is Nothing) Then perf.CloseDown
    End
End Sub

Private Sub lblFriday_Click()
    ClearlblStuff
    Set currentstyle = sDisco
    ChangeMusic
    day = 2: LabelNumber = 3
    lblStuff.Caption = LoadMSg(2)
End Sub

Private Sub lblFriday_MouseMove(Button As Integer, Shift As Integer, x As Single, y As Single)
    
    lblName.Font = "Courier New"
    lblName.ForeColor = &H8080FF
    
    lblFriday.ForeColor = &HFF&
    
    lblWednesday.ForeColor = &HC000&
    lblThursday.ForeColor = &HC000&
    lblSaturday.ForeColor = &HC000&
    lblSunday.ForeColor = &HC000&
End Sub

Private Sub lblSaturday_Click()
    ClearlblStuff
    Set currentstyle = sDance
    ChangeMusic
    day = 6: LabelNumber = 4
    lblStuff.Caption = LoadMSg(3)
End Sub

Private Sub lblSaturday_MouseMove(Button As Integer, Shift As Integer, x As Single, y As Single)
    
    lblName.Font = "Tahoma"
    lblName.ForeColor = &HC00000
    
    lblSaturday.ForeColor = &HFF&
    
    lblWednesday.ForeColor = &HC000&
    lblThursday.ForeColor = &HC000&
    lblFriday.ForeColor = &HC000&
    lblSunday.ForeColor = &HC000&
End Sub

Private Sub lblSunday_Click()
    ClearlblStuff
    Set currentstyle = sClassical
    ChangeMusic
    day = 5: LabelNumber = 5
    lblStuff.Caption = LoadMSg(4)
End Sub

Private Sub lblSunday_MouseMove(Button As Integer, Shift As Integer, x As Single, y As Single)
    lblName.Font = "Garamond"
    lblName.ForeColor = &HFFC0C0
    
    lblSunday.ForeColor = &HFF&
    
    lblWednesday.ForeColor = &HC000&
    lblThursday.ForeColor = &HC000&
    lblFriday.ForeColor = &HC000&
    lblSaturday.ForeColor = &HC000&
End Sub

Private Sub lblThursday_Click()
    ClearlblStuff
    Set currentstyle = sJazz
    ChangeMusic
    day = 3: LabelNumber = 2
    lblStuff.Caption = LoadMSg(1)
End Sub

Private Sub lblThursday_MouseMove(Button As Integer, Shift As Integer, x As Single, y As Single)
    lblName.Font = "Comic Sans MS"
    lblName.ForeColor = &H80FF80
    
    lblThursday.ForeColor = &HFF&
    
    lblWednesday.ForeColor = &HC000&
    lblFriday.ForeColor = &HC000&
    lblSaturday.ForeColor = &HC000&
    lblSunday.ForeColor = &HC000&
End Sub

Private Sub lblWednesday_Click()
    ClearlblStuff
    Set currentstyle = sBigBand
    ChangeMusic
    day = 1: LabelNumber = 1
    lblStuff.Caption = LoadMSg(0)
End Sub

Private Sub lblWednesday_MouseMove(Button As Integer, Shift As Integer, x As Single, y As Single)
    lblName.Font = "Times New Roman"
    lblName.ForeColor = &HFFFF&
    
    
    lblWednesday.ForeColor = &HFF&
    
    lblThursday.ForeColor = &HC000&
    lblFriday.ForeColor = &HC000&
    lblSaturday.ForeColor = &HC000&
    lblSunday.ForeColor = &HC000&
End Sub

