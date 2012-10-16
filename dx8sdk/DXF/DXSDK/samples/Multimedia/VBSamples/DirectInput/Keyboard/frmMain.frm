VERSION 5.00
Begin VB.Form frmMain 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "DirectInput Keyboard Sample"
   ClientHeight    =   3435
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   5880
   Icon            =   "frmMain.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   3435
   ScaleWidth      =   5880
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer tmrKey 
      Left            =   0
      Top             =   0
   End
   Begin VB.ListBox lstKeys 
      Height          =   2595
      Left            =   120
      TabIndex        =   0
      Top             =   360
      Width           =   5655
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       FrmMain.frm
'  Content:    This sample will show one way to use DirectInput with the keyboard
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Dim dx As New DirectX8
Dim di As DirectInput8
Dim diDEV As DirectInputDevice8
Dim diState As DIKEYBOARDSTATE
Dim iKeyCounter As Integer
Dim aKeys(255) As String


Private Sub Form_Load()

    Set di = dx.DirectInputCreate()
        
    If Err.Number <> 0 Then
        MsgBox "Error starting Direct Input, please make sure you have DirectX installed", vbApplicationModal
        End
    End If
        
        
    Set diDEV = di.CreateDevice("GUID_SysKeyboard")
    
    diDEV.SetCommonDataFormat DIFORMAT_KEYBOARD
    diDEV.SetCooperativeLevel Me.hWnd, DISCL_BACKGROUND Or DISCL_NONEXCLUSIVE
    
    Me.Show
    
    diDEV.Acquire
        
    tmrKey.Interval = 10
    tmrKey.Enabled = True
    

End Sub

Private Sub Form_Unload(Cancel As Integer)
    diDEV.Unacquire
End Sub

Private Sub tmrKey_Timer()
    lstKeys.Clear
    
    diDEV.GetDeviceStateKeyboard diState
    
    For iKeyCounter = 0 To 255
        If diState.Key(iKeyCounter) <> 0 Then
            lstKeys.AddItem KeyNames(iKeyCounter)
        End If
    Next
    DoEvents
End Sub





Function KeyNames(iNum As Integer) As String

    aKeys(&H1) = "DIK_ESCAPE"
    aKeys(&H2) = "DIK_1"
    aKeys(&H3) = "DIK_2"
    aKeys(&H4) = "DIK_3"
    aKeys(&H5) = "DIK_4"
    aKeys(&H6) = "DIK_5"
    aKeys(&H7) = "DIK_6"
    aKeys(&H8) = "DIK_7"
    aKeys(&H9) = "DIK_8"
    aKeys(&HA) = "DIK_9"
    aKeys(&HB) = "DIK_0"
    aKeys(&HC) = "DIK_MINUS"             ' - on main keyboard
    aKeys(&HD) = "DIK_EQUALS"
    aKeys(&HE) = "DIK_BACK"                  ' backspace
    aKeys(&HF) = "DIK_TAB"
    aKeys(&H10) = "DIK_Q"
    aKeys(&H11) = "DIK_W"
    aKeys(&H12) = "DIK_E"
    aKeys(&H13) = "DIK_R"
    aKeys(&H14) = "DIK_T"
    aKeys(&H15) = "DIK_Y"
    aKeys(&H16) = "DIK_U"
    aKeys(&H17) = "DIK_I"
    aKeys(&H18) = "DIK_O"
    aKeys(&H19) = "DIK_P"
    aKeys(&H1A) = "DIK_LBRACKET"
    aKeys(&H1B) = "DIK_RBRACKET"
    aKeys(&H1C) = "DIK_RETURN" ' Enter on main keyboard
    aKeys(&H1D) = "DIK_LCONTROL"
    aKeys(&H1E) = "DIK_A"
    aKeys(&H1F) = "DIK_S"
    aKeys(&H20) = "DIK_D"
    aKeys(&H21) = "DIK_F"
    aKeys(&H22) = "DIK_G"
    aKeys(&H23) = "DIK_H"
    aKeys(&H24) = "DIK_J"
    aKeys(&H25) = "DIK_K"
    aKeys(&H26) = "DIK_L"
    aKeys(&H27) = "DIK_SEMICOLON"
    aKeys(&H28) = "DIK_APOSTROPHE"
    aKeys(&H29) = "DIK_GRAVE" ' accent grave
    aKeys(&H2A) = "DIK_LSHIFT"
    aKeys(&H2B) = "DIK_BACKSLASH"
    aKeys(&H2C) = "DIK_Z"
    aKeys(&H2D) = "DIK_X"
    aKeys(&H2E) = "DIK_C"
    aKeys(&H2F) = "DIK_V"
    aKeys(&H30) = "DIK_B"
    aKeys(&H31) = "DIK_N"
    aKeys(&H32) = "DIK_M"
    aKeys(&H33) = "DIK_COMMA"
    aKeys(&H34) = "DIK_PERIOD" ' . on main keyboard
    aKeys(&H35) = "DIK_SLASH" ' / on main keyboard
    aKeys(&H36) = "DIK_RSHIFT"
    aKeys(&H37) = "DIK_MULTIPLY" ' * on numeric keypad
    aKeys(&H38) = "DIK_LMENU" ' left Alt
    aKeys(&H39) = "DIK_SPACE"
    aKeys(&H3A) = "DIK_CAPITAL"
    aKeys(&H3B) = "DIK_F1"
    aKeys(&H3C) = "DIK_F2"
    aKeys(&H3D) = "DIK_F3"
    aKeys(&H3E) = "DIK_F4"
    aKeys(&H3F) = "DIK_F5"
    aKeys(&H40) = "DIK_F6"
    aKeys(&H41) = "DIK_F7"
    aKeys(&H42) = "DIK_F8"
    aKeys(&H43) = "DIK_F9"
    aKeys(&H44) = "DIK_F10"
    aKeys(&H45) = "DIK_NUMLOCK"
    aKeys(&H46) = "DIK_SCROLL" ' Scroll Lock
    aKeys(&H47) = "DIK_NUMPAD7"
    aKeys(&H48) = "DIK_NUMPAD8"
    aKeys(&H49) = "DIK_NUMPAD9"
    aKeys(&H4A) = "DIK_SUBTRACT" ' - on numeric keypad
    aKeys(&H4B) = "DIK_NUMPAD4"
    aKeys(&H4C) = "DIK_NUMPAD5"
    aKeys(&H4D) = "DIK_NUMPAD6"
    aKeys(&H4E) = "DIK_ADD" ' + on numeric keypad
    aKeys(&H4F) = "DIK_NUMPAD1"
    aKeys(&H50) = "DIK_NUMPAD2"
    aKeys(&H51) = "DIK_NUMPAD3"
    aKeys(&H52) = "DIK_NUMPAD0"
    aKeys(&H53) = "DIK_DECIMAL" ' . on numeric keypad
    aKeys(&H56) = "DIK_OEM_102 < > | on UK/Germany keyboards"
    aKeys(&H57) = "DIK_F11"
    aKeys(&H58) = "DIK_F12"
    aKeys(&H64) = "DIK_F13 on (NEC PC98) "
    aKeys(&H65) = "DIK_F14 on (NEC PC98) "
    aKeys(&H66) = "DIK_F15 on (NEC PC98) "
    aKeys(&H70) = "DIK_KANA on (Japanese keyboard)"
    aKeys(&H73) = "DIK_ABNT_C1 / ? on Portugese (Brazilian) keyboards "
    aKeys(&H79) = "DIK_CONVERT on (Japanese keyboard)"
    aKeys(&H7B) = "DIK_NOCONVERT on (Japanese keyboard)"
    aKeys(&H7D) = "DIK_YEN on (Japanese keyboard)"
    aKeys(&H7E) = "DIK_ABNT_C2 on Numpad . on Portugese (Brazilian) keyboards "
    aKeys(&H8D) = "DIK_NUMPADEQUALS = on numeric keypad (NEC PC98) "
    aKeys(&H90) = "DIK_PREVTRACK on Previous Track (DIK_CIRCUMFLEX on Japanese keyboard) "
    aKeys(&H91) = "DIK_AT (NEC PC98) "
    aKeys(&H92) = "DIK_COLON (NEC PC98) "
    aKeys(&H93) = "DIK_UNDERLINE (NEC PC98) "
    aKeys(&H94) = "DIK_KANJI on (Japanese keyboard)"
    aKeys(&H95) = "DIK_STOP (NEC PC98) "
    aKeys(&H96) = "DIK_AX (Japan AX) "
    aKeys(&H97) = "DIK_UNLABELED (J3100) "
    aKeys(&H99) = "DIK_NEXTTRACK" ' Next Track
    aKeys(&H9C) = "DIK_NUMPADENTER" ' Enter on numeric keypad
    aKeys(&H9D) = "DIK_RCONTROL"
    aKeys(&HA0) = "DIK_MUTE" ' Mute
    aKeys(&HA1) = "DIK_CALCULATOR" ' Calculator
    aKeys(&HA2) = "DIK_PLAYPAUSE" ' Play / Pause
    aKeys(&HA4) = "DIK_MEDIASTOP" ' Media Stop
    aKeys(&HAE) = "DIK_VOLUMEDOWN" ' Volume -
    aKeys(&HB0) = "DIK_VOLUMEUP" ' Volume +
    aKeys(&HB2) = "DIK_WEBHOME" ' Web home
    aKeys(&HB3) = "DIK_NUMPADCOMMA" ' , on numeric keypad (NEC PC98)
    aKeys(&HB5) = "DIK_DIVIDE" ' / on numeric keypad
    aKeys(&HB7) = "DIK_SYSRQ"
    aKeys(&HB8) = "DIK_RMENU" ' right Alt
    aKeys(&HC5) = "DIK_PAUSE" ' Pause
    aKeys(&HC7) = "DIK_HOME" ' Home on arrow keypad
    aKeys(&HC8) = "DIK_UP" ' UpArrow on arrow keypad
    aKeys(&HC9) = "DIK_PRIOR" ' PgUp on arrow keypad
    aKeys(&HCB) = "DIK_LEFT" ' LeftArrow on arrow keypad
    aKeys(&HCD) = "DIK_RIGHT" ' RightArrow on arrow keypad
    aKeys(&HCF) = "DIK_END" ' End on arrow keypad
    aKeys(&HD0) = "DIK_DOWN" ' DownArrow on arrow keypad
    aKeys(&HD1) = "DIK_NEXT" ' PgDn on arrow keypad
    aKeys(&HD2) = "DIK_INSERT" ' Insert on arrow keypad
    aKeys(&HD3) = "DIK_DELETE" ' Delete on arrow keypad
    aKeys(&HDB) = "DIK_LWIN" ' Left Windows key
    aKeys(&HDC) = "DIK_RWIN" ' Right Windows key
    aKeys(&HDD) = "DIK_APPS" ' AppMenu key
    aKeys(&HDE) = "DIK_POWER" ' System Power
    aKeys(&HDF) = "DIK_SLEEP" ' System Sleep
    aKeys(&HE3) = "DIK_WAKE" ' System Wake
    aKeys(&HE5) = "DIK_WEBSEARCH" ' Web Search
    aKeys(&HE6) = "DIK_WEBFAVORITES" ' Web Favorites
    aKeys(&HE7) = "DIK_WEBREFRESH" ' Web Refresh
    aKeys(&HE8) = "DIK_WEBSTOP" ' Web Stop
    aKeys(&HE9) = "DIK_WEBFORWARD" ' Web Forward
    aKeys(&HEA) = "DIK_WEBBACK" ' Web Back
    aKeys(&HEB) = "DIK_MYCOMPUTER" ' My Computer
    aKeys(&HEC) = "DIK_MAIL" ' Mail
    aKeys(&HED) = "DIK_MEDIASELECT" ' Media Select

    KeyNames = aKeys(iNum)

End Function

