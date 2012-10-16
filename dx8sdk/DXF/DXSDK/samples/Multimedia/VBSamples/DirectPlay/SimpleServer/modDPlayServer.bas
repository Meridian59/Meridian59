Attribute VB_Name = "modDPlayServer"
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       modDPlayServer.bas
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Public Const AppGuid = "{5726CF1F-702B-4008-98BC-BF9C95F9E288}"
Public Declare Function Shell_NotifyIcon Lib "shell32.dll" Alias "Shell_NotifyIconA" (ByVal dwMessage As Long, lpData As NOTIFYICONDATA) As Long

Public Type NOTIFYICONDATA
    cbSize As Long
    hwnd As Long
    uID As Long
    uFlags As Long
    uCallbackMessage As Long
    hIcon As Long
    sTip As String * 64
End Type
    
Public Const NIM_ADD = &H0
Public Const NIM_MODIFY = &H1
Public Const NIM_DELETE = &H2
Public Const NIF_MESSAGE = &H1
Public Const NIF_ICON = &H2
Public Const NIF_TIP = &H4
Public Const NIF_DOALL = NIF_MESSAGE Or NIF_ICON Or NIF_TIP
Public Const WM_MOUSEMOVE = &H200
Public Const WM_LBUTTONDBLCLK = &H203
Public Const WM_RBUTTONUP = &H205


Public dx As New DirectX8
Public dps As DirectPlay8Server
Public dpa As DirectPlay8Address
Public glNumPlayers As Long
Public gfStarted As Boolean
Public sysIcon As NOTIFYICONDATA

Public Sub Main()

    InitDPlay
    frmServer.Show
End Sub

Public Sub InitDPlay()

    Set dps = dx.DirectPlayServerCreate
    Set dpa = dx.DirectPlayAddressCreate
    
End Sub

Public Sub Cleanup()

    'Shut down our message handler
    If Not dps Is Nothing Then dps.UnRegisterMessageHandler
    'Close down our session
    If Not dps Is Nothing Then dps.Close
    Set dps = Nothing
    Set dpa = Nothing
    Set dx = Nothing
    
End Sub
