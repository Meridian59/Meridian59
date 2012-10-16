Attribute VB_Name = "MediaDir"
Option Explicit
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       media.bas
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

'Registry constants
Private Const KEY_READ = 131097
Private Const REG_SZ = 1
Private Const HKEY_LOCAL_MACHINE = &H80000002
'Registry API's
Private Declare Function RegConnectRegistry Lib "advapi32.dll" Alias "RegConnectRegistryA" (ByVal lpMachineName As String, ByVal hKey As Long, phkResult As Long) As Long
Private Declare Function RegCloseKey Lib "advapi32.dll" (ByVal hKey As Long) As Long
Private Declare Function RegQueryValueEx Lib "advapi32.dll" Alias "RegQueryValueExA" (ByVal hKey As Long, ByVal lpValueName As String, ByVal lpReserved As Long, lpType As Long, ByVal lpData As String, lpcbData As Long) As Long
Private Declare Function RegOpenKeyEx Lib "advapi32.dll" Alias "RegOpenKeyExA" (ByVal hKey As Long, ByVal lpSubKey As String, ByVal ulOptions As Long, ByVal samDesired As Long, phkResult As Long) As Long

Public Function FindMediaDir(ByVal sFile As String, Optional ByVal fUseCMedia As Boolean = False) As String
    If Dir$(sFile, vbNormal) <> vbNullString Then 'This file is the current folder
        FindMediaDir = AddDirSep(CurDir)
        Exit Function
    End If
    If fUseCMedia Then
        FindMediaDir = AddDirSep(GetDXSampleFolder) & "media\"
    Else
        FindMediaDir = AddDirSep(GetDXSampleFolder) & "vbsamples\media\"
    End If
End Function

Public Function AddDirSep(ByVal sPath As String) As String
    AddDirSep = sPath
    If Right$(sPath, 1) <> "\" Then
        AddDirSep = sPath & "\"
    End If
End Function

Public Function GetDXSampleFolder() As String
    Dim lHandle As Long
    Dim lNewHandle As Long, sValue As String
    Dim lNewKey As Long
    
    RegConnectRegistry vbNullString, HKEY_LOCAL_MACHINE, lHandle
    RegOpenKeyEx lHandle, "SOFTWARE\Microsoft\DirectX SDK", 0, KEY_READ, lNewHandle
    sValue = Space$(255)
    RegQueryValueEx lNewHandle, "DX81SDK Samples Path", 0, REG_SZ, sValue, 255
    If sValue <> Space$(255) Then
        sValue = Left$(sValue, InStr(sValue, Chr$(0)) - 1)
    Else
        sValue = vbNullString
    End If
    RegCloseKey lNewHandle
    RegCloseKey lHandle
    GetDXSampleFolder = sValue
End Function
