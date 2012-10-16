Attribute VB_Name = "modDplayClient"
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       modDPlayClient.bas
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Public Const AppGuid = "{5726CF1F-702B-4008-98BC-BF9C95F9E288}"

Public dx As New DirectX8
Public dpc As DirectPlay8Client

Public DPlayEventsForm As DPlayConnect

Public Sub Main()
    InitDPlay
    frmClient.Show
End Sub

Public Sub InitDPlay()
    Set dpc = dx.DirectPlayClientCreate
End Sub

Public Sub Cleanup()

    'Stop our message handler
    If Not dpc Is Nothing Then dpc.UnRegisterMessageHandler
    'Close down our session
    If Not dpc Is Nothing Then dpc.Close
    Set dpc = Nothing
    Set dx = Nothing
    'Get rid of our message pump
    DPlayEventsForm.GoUnload
    
End Sub

