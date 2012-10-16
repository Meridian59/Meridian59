Attribute VB_Name = "Globals"
'*******************************************************************************
'*       This is a part of the Microsoft Platform SDK Code Samples.
'*       Copyright (C) 1999-2001 Microsoft Corporation.
'*       All rights reserved.
'*       This source code is only intended as a supplement to
'*       Microsoft Development Tools and/or SDK documentation.
'*******************************************************************************

'Microsoft Video Control - Sample Visual Basic Application
Option Explicit

Public Const NTSC_GUID As String = "{8A674B4D-1F63-11D3-B64C-00C04F79498E}"
Public Const ATSC_GUID As String = "{A2E30750-6C3D-11D3-B653-00C04F79498E}"
Public Const DVBS_GUID As String = "{B64016F3-C9A2-4066-96F0-BD9563314726}"
Public Const CC_GUID As String = "{7F9CB14D-48E4-43B6-9346-1AEBC39C64D3}"
Public Const DATASVC_GUID As String = "{334125C0-77E5-11D3-B653-00C04F79498E}"

Sub Main()
    frmMain.Show
End Sub

'Use this for error reporting
Public Function CheckError(ErrorMsg As String)
    Dim Msg As Variant
    If Err.Number <> 0 Then
        Msg = ErrorMsg & vbCrLf & vbCrLf & "The error returned was:" & vbCrLf & Hex(Err.Number) & ": " & Err.Description
        MsgBox Msg, , "Error"
    End If
    'Reset the error so we don't get the same message over and over
    Err.Number = 0
End Function
