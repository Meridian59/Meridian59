VERSION 5.00
Begin VB.Form Form1 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "DInput Action Mapping"
   ClientHeight    =   3840
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   4725
   Icon            =   "ActionMap.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   256
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   315
   ShowInTaskbar   =   0   'False
   StartUpPosition =   3  'Windows Default
   Begin VB.Frame Frame1 
      Height          =   855
      Left            =   0
      TabIndex        =   3
      Top             =   3000
      Width           =   4695
      Begin VB.Label Label2 
         Caption         =   "Pressing 'd' will allow you to view device configurations"
         Height          =   255
         Left            =   120
         TabIndex        =   5
         Top             =   480
         Width           =   4335
      End
      Begin VB.Label Label1 
         Caption         =   "Press escape to exit                                                                              "
         Height          =   255
         Left            =   120
         TabIndex        =   4
         Top             =   240
         Width           =   4335
         WordWrap        =   -1  'True
      End
   End
   Begin VB.Label GameStateList 
      Height          =   255
      Index           =   0
      Left            =   240
      TabIndex        =   2
      Top             =   360
      Visible         =   0   'False
      Width           =   3855
   End
   Begin VB.Label TSHIELD 
      Caption         =   "Label1"
      Height          =   375
      Left            =   3720
      TabIndex        =   1
      Top             =   5160
      Width           =   1215
   End
   Begin VB.Label TFIRE 
      Caption         =   "Label1"
      Height          =   255
      Left            =   1200
      TabIndex        =   0
      Top             =   5160
      Width           =   1095
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       ActionMap.frm
'  Content:    Use DirectInput action mapper to interpret input from many devices
'
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

'-----------------------------------------------------------------------------
' App-defined game actions for the DInput action mapper.
'-----------------------------------------------------------------------------

' The following constants are custom for each app, depending on what input the
' app needs. This simple sample is pretending to be a space simulator game, so
' relevant inputs are for turning left, thrusting, firing weapons, etc.. Also
' note that some constants are defines for the axes, which are for recieving
' axis data from joysticks and similiar analog devices.
Const INPUT_LEFTRIGHT_AXIS = 1
Const INPUT_UPDOWN_AXIS = 2
Const INPUT_TURNLEFT = 4
Const INPUT_TURNRIGHT = 5
Const INPUT_FORWARDTHRUST = 6
Const INPUT_REVERSETHRUST = 7
Const INPUT_FIREWEAPONS = 8
Const INPUT_ENABLESHIELD = 9
Const INPUT_DISPLAYGAMEMENU = 10
Const INPUT_QUITGAME = 11

 
Const kMapGuid = "{20CAA014-60BC-4399-BDD3-84AD65A38A1C}"
Const kUserName = "DInput 8 VB Sample User"
Const KGenre = DIVIRTUAL_SPACESIM

Dim m_mapper As New CInputMapper

Private Sub DeviceList_Click(Index As Integer)

End Sub

Private Sub Form_Load()
    Me.Show
    DefineActions
    
    Dim i As Long
    For i = 1 To 15
        Load Form1.GameStateList(i)
        Form1.GameStateList(i).Top = i * 20
        Form1.GameStateList(i).Visible = True
                
    Next
    
    If Not m_mapper.CreateDevicesFromMAP(Me.hWnd, kUserName, "Semantic Mapper VB Sample", kMapGuid, KGenre) Then
        MsgBox "unable to find any mappable input devices"
        End
    End If
    
    Do While InputLoop()
        DoEvents
    Loop
    
    m_mapper.ClearMap
    
    End
End Sub


' The following array is the global, app-defined game actions, which map real
' device input into a semantic. The first column is the app-defined semantics
' as defined above. These are the constants the game actually sees in its
' input loop. The second column is the physical action recieved by the device
' which is to be mapped to the app-defined semantic. For instance, in the
' array below, if the user hits the "Left" key on the keyboard, the app will
' recieve an input code equal to INPUT_TURNLEFT. The last column is a text
' string that DInput uses for displaying a configuration dialog box.

Sub DefineActions()
    
    
    m_mapper.ClearMap
    
    ' Device input (joystick, etc.) that is pre-defined by DInput, according
    ' to genre type. The genre for this app is space simulators.
    With m_mapper
        .AddAction INPUT_LEFTRIGHT_AXIS, DIAXIS_SPACESIM_LATERAL, 0, "Turn"
        .AddAction INPUT_UPDOWN_AXIS, DIAXIS_SPACESIM_MOVE, 0, "Move"
        .AddAction INPUT_FIREWEAPONS, DIBUTTON_SPACESIM_FIRE, 0, "Shoot"
        .AddAction INPUT_ENABLESHIELD, DIBUTTON_SPACESIM_GEAR, 0, "Shield"
        .AddAction INPUT_DISPLAYGAMEMENU, DIBUTTON_SPACESIM_DISPLAY, 0, "Display"
        .AddAction INPUT_QUITGAME, DIBUTTON_SPACESIM_MENU, 0, "Quit Game"
    
        ' Keyboard input mappings
        .AddAction INPUT_FORWARDTHRUST, DIKEYBOARD_UP, 0, "Forward thrust"
        .AddAction INPUT_REVERSETHRUST, DIKEYBOARD_DOWN, 0, "Reverse thrust"
        .AddAction INPUT_FIREWEAPONS, DIKEYBOARD_F, 0, "Fire weapons"
        .AddAction INPUT_ENABLESHIELD, DIKEYBOARD_S, 0, "Enable shields"
        .AddAction INPUT_DISPLAYGAMEMENU, DIKEYBOARD_D, 0, "Display game menu"
        .AddAction INPUT_QUITGAME, DIKEYBOARD_ESCAPE, 0, "Quit game"
        .AddAction INPUT_TURNRIGHT, DIKEYBOARD_RIGHT, 0, "Right Turn"
        .AddAction INPUT_TURNLEFT, DIKEYBOARD_LEFT, 0, "Left Turn"
        
        ' Mouse input mappings
        .AddAction INPUT_LEFTRIGHT_AXIS, DIMOUSE_XAXIS, 0, "Turn"
        .AddAction INPUT_UPDOWN_AXIS, DIMOUSE_YAXIS, 0, "Move"
        .AddAction INPUT_FIREWEAPONS, DIMOUSE_BUTTON0, 0, "Fire weapons"
        .AddAction INPUT_ENABLESHIELD, DIMOUSE_BUTTON1, 0, "Enable shields"
        
    End With
    
    
End Sub

'-----------------------------------------------------------------------------
' Name: InputLoop()
' Desc: This is the input loop for the app. Input is gathered from the DInput
'       devices, and output is displayed simply in the app's window.
'-----------------------------------------------------------------------------
Function InputLoop() As Boolean

    Dim didObjData As DIDEVICEOBJECTDATA
    Dim strOut As String
    
    On Local Error Resume Next
    
   'Static state of the app's input
    Static bTurningRight       As Boolean
    Static bReverseThrust      As Boolean
    Static bTurningLeft        As Boolean
    Static bForwardThrust      As Boolean
    Static bFiringWeapons      As Boolean
    Static bEnableShields      As Boolean
    Static bDisplayingMenu     As Boolean
    Static dwLRAxisData        As Long
    Static dwUDAxisData       As Long

    Dim nItems As Long
    Dim i As Long, j As Long
    Dim adod(10) As DIDEVICEOBJECTDATA
    Static nItemsSave(20) As Long
    
    Dim dev As DirectInputDevice8
    
    For i = 1 To m_mapper.GetNumDevices()
                
        nItems = 10
        Set dev = m_mapper.GetDevice(i)
        
        ' Need to ensure that the devices are acquired, and pollable devices
        ' are polled.
        dev.Acquire
        If Err.Number <> 0 Then GoTo skipDevice
        
        dev.Poll
        If Err.Number <> 0 Then GoTo skipDevice
        
            
        ' This call gets the data from the i'th device
        nItems = 0
        nItems = dev.GetDeviceData(adod, 0)
    


        ' Get the sematics codes. The number of input events is stored in
        ' "nItems", and all the events are stored in the "adod" array. Each
        ' event has a type stored in "uAppDate", and actual data stored in
        ' "nData".
        For j = 0 To nItems - 1
        
            If (adod(j).lUserData = INPUT_LEFTRIGHT_AXIS) Then  ' Left-right axis
                ' Parse the left-right axis data
                dwLRAxisData = adod(j).lData
                bTurningRight = False
                bTurningLeft = False
                Debug.Print "AXIS"
                If (dwLRAxisData > 0) Then bTurningRight = True
                If (dwLRAxisData < -0) Then bTurningLeft = True
    
            ElseIf (adod(j).lUserData = INPUT_UPDOWN_AXIS) Then   ' Up-down axis
                ' Parse the up-down axis data
                dwUDAxisData = adod(j).lData
                bReverseThrust = False
                bForwardThrust = False

                If (dwUDAxisData > 0) Then bReverseThrust = True
                If (dwUDAxisData < -0) Then bForwardThrust = True
            Else
                ' Non-axis stuff
            
                ' Non-axis data is recieved as "button pressed" or "button
                ' released". Parse input as such.
                Dim bState As Boolean
                If (adod(j).lData = &H80) Then bState = True
                Debug.Print "BUTTON"
                Select Case adod(j).lUserData
                
                    Case INPUT_TURNLEFT:
                        bTurningLeft = bState
                    Case INPUT_TURNRIGHT:
                        bTurningRight = bState
                    Case INPUT_FORWARDTHRUST:
                        bForwardThrust = bState
                    Case INPUT_REVERSETHRUST:
                        bReverseThrust = bState
                    Case INPUT_FIREWEAPONS:
                        bFiringWeapons = bState
                    Case INPUT_ENABLESHIELD:
                        bEnableShields = bState
                    Case INPUT_DISPLAYGAMEMENU:
                        bDisplayingMenu = bState
                        
                    Case INPUT_QUITGAME:
                        InputLoop = False
                        Exit Function
                End Select
            End If
        Next
        
skipDevice:
        Err.Clear
    Next

    ' Remove conflicts (in a game, you couldn't go left and right at the same
    ' time. Actual conflicts depend on the game logic, and not on the DInput
    ' semantic mappings.)
    If (bTurningLeft And bTurningRight) Then
        bTurningLeft = False:        bTurningRight = False
    End If
    If (bForwardThrust And bReverseThrust) Then
        bForwardThrust = False:     bReverseThrust = False
    End If
    If (bFiringWeapons And bEnableShields) Then bFiringWeapons = False

    ' The remainder of the this function is simply to output the results of
    ' gathering the input.
    GameStateList(1).Caption = "Turning Left " + Str(bTurningLeft)
    GameStateList(2).Caption = "Turning Right " + Str(bTurningRight)
    GameStateList(3).Caption = "Forward thrust " + Str(bForwardThrust)
    GameStateList(4).Caption = "Backward thrust " + Str(bReverseThrust)
    GameStateList(5).Caption = "Firing Weapons " + Str(bFiringWeapons)
    GameStateList(6).Caption = "Enable Shields" + Str(bEnableShields)
    GameStateList(7).Caption = "LR Axis " + Str(dwLRAxisData)
    GameStateList(8).Caption = "UD Axis " + Str(dwUDAxisData)
    
    If (bDisplayingMenu) Then
        
        m_mapper.ConfigureDevices True
        bDisplayingMenu = False
    End If
    
    InputLoop = True
    
End Function


Private Sub Form_Unload(Cancel As Integer)
    End
End Sub


