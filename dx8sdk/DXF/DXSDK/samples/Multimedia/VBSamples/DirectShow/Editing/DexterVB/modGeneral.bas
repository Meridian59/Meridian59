Attribute VB_Name = "modGeneral"
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
' * PRIVATE INTERFACE- CONSTANTS
' *
' *
            Private Const MAX_PATH = 255
            


' **************************************************************************************************************************************
' * PRIVATE INTERFACE- DATA STRUCTURES
' *
' *
            Private Type GUID
               Guid1 As Long
               Guid2 As Long
               Guid3 As Long
               Guid4(0 To 7) As Byte
            End Type



' **************************************************************************************************************************************
' * PRIVATE INTERFACE- DECLARATIONS
' *
' *
            Private Declare Function GetComputerName Lib "kernel32" Alias "GetComputerNameA" (ByVal lpBuffer As String, nSize As Long) As Long
            Private Declare Function GetComputerNameW Lib "kernel32" (lpBuffer As Any, nSize As Long) As Long
            Private Declare Function CoCreateGuid Lib "OLE32.DLL" (pGUID As GUID) As Long
            Private Declare Function StringFromGUID2 Lib "OLE32.DLL" (pGUID As GUID, ByVal PointerToString As Long, ByVal MaxLength As Long) As Long



' **************************************************************************************************************************************
' * PUBLIC INTERFACE- PROCEDURES
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: Buffer_ParseEx
            ' * procedure description:   Parse's a fixed length string buffer of all vbNullCharacters AND vbNullStrings.
            ' *                                        Argument bstrBuffer evaluates to either an ANSII or Unicode BSTR string buffer.
            ' *                                        (bstrBuffer is almost always the output from a windows api call which needs parsed)
            ' *
            ' ******************************************************************************************************************************
            Public Function Buffer_ParseEx(bstrBuffer As String) As String
            Dim iCount As Long, bstrChar As String, bstrReturn As String
            On Local Error GoTo ErrLine
            
            For iCount = 1 To Len(bstrBuffer) 'set up a loop to remove the vbNullChar's from the buffer.
                  bstrChar = Strings.Mid(bstrBuffer, iCount, 1)
                  If bstrChar <> vbNullChar And bstrChar <> vbNullString Then bstrReturn = (bstrReturn + bstrChar)
            Next
            Buffer_ParseEx = bstrReturn
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: System_GetComputerName
            ' * procedure description:   Returns the name associated with the local system.
            ' *
            ' ******************************************************************************************************************************
            Public Function System_GetComputerName() As String
            Dim bstrBuffer As String * MAX_PATH, bstrReturn As String
            On Local Error GoTo ErrLine
            'obtain the computer name via the win32 api
            GetComputerName bstrBuffer, Len(bstrBuffer) + 1
            'assign the fixed length buffer to a variable length string
            bstrReturn = bstrBuffer
            'return the value to the client
            System_GetComputerName = Buffer_ParseEx(bstrReturn)
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: ShowCommonDlgOpen
            ' * procedure description:
            ' *
            ' ******************************************************************************************************************************
            Public Function ShowCommonDlgOpen(Optional bstrCurrentDirectory As String, Optional bstrDefaultExtension As String, Optional bstrFilter As String) As String
            Dim ctrl As Object
            On Local Error GoTo ErrLine
            
            'instantiate control
            If Not CreateObject("MSComDlg.CommonDialog.1") Is Nothing Then
               Set ctrl = CreateObject("MSComDlg.CommonDialog.1")
            ElseIf Not CreateObject("MSComDlg.CommonDialog") Is Nothing Then
               Set ctrl = CreateObject("MSComDlg.CommonDialog")
            End If
            
            If Not ctrl Is Nothing Then
               'set properties
               ctrl.Filter = bstrFilter
               ctrl.DefaultExt = bstrDefaultExtension
               ctrl.InitDir = bstrCurrentDirectory
               ctrl.ShowOpen
               'return to client
               ShowCommonDlgOpen = ctrl.FileName
            End If
            
            'clean-up & dereference
            If Not ctrl Is Nothing Then Set ctrl = Nothing
            Exit Function
            
ErrLine:

            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: ShowCommonDlgSave
            ' * procedure description:
            ' *
            ' ******************************************************************************************************************************
            Public Function ShowCommonDlgSave(Optional bstrCurrentDirectory As String, Optional bstrDefaultExtension As String, Optional bstrFilter As String) As String
            Dim ctrl As Object
            On Local Error GoTo ErrLine
            
            'instantiate control
            If Not CreateObject("MSComDlg.CommonDialog.1") Is Nothing Then
               Set ctrl = CreateObject("MSComDlg.CommonDialog.1")
            ElseIf Not CreateObject("MSComDlg.CommonDialog") Is Nothing Then
               Set ctrl = CreateObject("MSComDlg.CommonDialog")
            End If
            
            If Not ctrl Is Nothing Then
               'set properties
               ctrl.Filter = bstrFilter
               ctrl.DefaultExt = bstrDefaultExtension
               ctrl.InitDir = bstrCurrentDirectory
               ctrl.ShowSave
               'return to client
               ShowCommonDlgSave = ctrl.FileName
            End If
            
            'clean-up & dereference
            If Not ctrl Is Nothing Then Set ctrl = Nothing
            Exit Function
            
ErrLine:

            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: GetGUID
            ' * procedure description:  returns a random global unique identifier
            ' *
            ' ******************************************************************************************************************************
            Public Function GetGUID() As String
            Dim udtGUID As GUID, bstrGUID As String, nResultant As Long
            On Local Error GoTo ErrLine
            
            nResultant = CoCreateGuid(udtGUID)
            If nResultant Then
               bstrGUID = vbNullString
            Else
                bstrGUID = String$(38, 0)
                StringFromGUID2 udtGUID, StrPtr(bstrGUID), 39
            End If
            GetGUID = bstrGUID
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
