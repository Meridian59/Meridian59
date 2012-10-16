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
' * PUBLIC INTERFACE- WIN32 API CONSTANTS
' *
' *
            Public Const FO_COPY = &H2
            Public Const FO_DELETE = &H3
            Public Const FO_MOVE = &H1
            Public Const FO_RENAME = &H4
            Public Const FOF_ALLOWUNDO = &H40
            Public Const FOF_CONFIRMMOUSE = &H2
            Public Const FOF_FILESONLY = &H80      ''"" on *.*, do only files
            Public Const FOF_MULTIDESTFILES = &H1
            Public Const FOF_NOCONFIRMATION = &H10      ''"" Don't prompt the user.
            Public Const FOF_NOCONFIRMMKDIR = &H200     ''"" don't confirm making any needed dirs
            Public Const FOF_NOCOPYSECURITYATTRIBS = &H800     ''"" dont copy NT file Security Attributes
            Public Const FOF_NOERRORUI = &H400     ''"" don't put up error UI
            Public Const FOF_NORECURSION = &H1000    ''"" don't recurse into directories.
            Public Const FOF_NO_CONNECTED_ELEMENTS = &H2000    ''"" don't operate on connected file elements.
            Public Const FOF_RENAMEONCOLLISION = &H8
            Public Const FOF_SILENT = &H4       ''"" don't create progress"report
            Public Const FOF_SIMPLEPROGRESS = &H100     ''"" means don't show names of files
            Public Const FOF_WANTMAPPINGHANDLE = &H20      ''"" Fill in SHFILEOPSTRUCT.hNameMappings
            Private Const MAX_PATH As Long = 255
            Private Const INVALID_HANDLE_VALUE = -1
            Private Const SEM_FAILCRITICALERRORS = &H1
            Private Const SEM_NOOPENFILEERRORBOX = &H8000


' **************************************************************************************************************************************
' * PUBLIC INTERFACE- WIN32 API DATA STRUCTURES
' *
' *
            Private Type FILETIME
                    dwLowDateTime As Long
                    dwHighDateTime As Long
            End Type
            
            Public Type WIN32_FIND_DATA
                    dwFileAttributes As Long
                    ftCreationTime As FILETIME
                    ftLastAccessTime As FILETIME
                    ftLastWriteTime As FILETIME
                    nFileSizeHigh As Long
                    nFileSizeLow As Long
                    dwReserved0 As Long
                    dwReserved1 As Long
                    cFileName As String * MAX_PATH
                    cAlternate As String * 14
            End Type
            
            Private Type SHFILEOPSTRUCT
                    hWnd As Long
                    wFunc As Long
                    pFrom As String
                    pTo As String
                    fFlags As Integer
                    fAnyOperationsAborted As Long
                    hNameMappings As Long
                    lpszProgressTitle As String '  only used if FOF_SIMPLEPROGRESS
            End Type


' **************************************************************************************************************************************
' * PUBLIC INTERFACE- WIN32 API DECLARATIONS
' *
' *
            Private Declare Function FindClose Lib "kernel32" (ByVal hFindFile As Long) As Long
            Private Declare Function SetErrorMode Lib "kernel32" (ByVal wMode As Long) As Long
            Private Declare Function SHFileOperation Lib "shell32.dll" Alias "SHFileOperationA" (lpFileOp As SHFILEOPSTRUCT) As Long
            Private Declare Function FindFirstFile Lib "kernel32" Alias "FindFirstFileA" (ByVal lpFileName As String, lpFindFileData As WIN32_FIND_DATA) As Long
            Private Declare Function GetTempPath Lib "kernel32" Alias "GetTempPathA" (ByVal nBufferLength As Long, ByVal lpBuffer As String) As Long





' **************************************************************************************************************************************
' * PUBLIC INTERFACE- DEXTER PROCEDURES
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: GetPinInfo
            ' * procedure description:  Returns an IPinInfo interface given a filtergraph manager and IPin object.
            ' *                                       The derived IPinInfo interface can be utilized for gaining information on the elected pin.
            ' ******************************************************************************************************************************
            Public Function GetPinInfo(objFilterGraphManager As FilgraphManager, objPin As IPin) As IPinInfo
            Dim objPin2 As IPin
            Dim objPinInfo As IPinInfo
            Dim objFilterInfo As IFilterInfo
            Dim objPinCollection As Object
            Dim objlFilterCollection As Object
            On Local Error GoTo ErrLine
            
            'derive a filter collection from the filtergraph manager
            Set objlFilterCollection = objFilterGraphManager.FilterCollection
            
            'enumerate through the filter(s) in the collection
            For Each objFilterInfo In objlFilterCollection
                Set objPinCollection = objFilterInfo.Pins
                For Each objPinInfo In objPinCollection
                    Set objPin2 = objPinInfo.Pin
                    If objPin2 Is objPin Then
                        Set GetPinInfo = objPinInfo
                        Exit Function
                    End If
                Next
            Next
            
            'clean-up & dereference
            If Not objPin2 Is Nothing Then Set objPin2 = Nothing
            If Not objPinInfo Is Nothing Then Set objPinInfo = Nothing
            If Not objFilterInfo Is Nothing Then Set objFilterInfo = Nothing
            If Not objPinCollection Is Nothing Then Set objPinCollection = Nothing
            If Not objlFilterCollection Is Nothing Then Set objlFilterCollection = Nothing
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: AddFileWriterAndMux
            ' * procedure description:  Appends a filewriter and mux filter to the given filtergraph.
            ' *                                       The FileName as required for the filewriter and evaluates to the output file destination.
            ' ******************************************************************************************************************************
            Public Sub AddFileWriterAndMux(objFilterGraphManager As FilgraphManager, bstrFileName As String)
            Dim objFilterInfo As IFilterInfo
            Dim objRegisteredFilters As Object
            Dim objAVIMuxFilterInfo As IFilterInfo
            Dim objRegFilterInfo As IRegFilterInfo
            Dim objFileSinkFilterVB As IFileSinkFilterForVB
            On Local Error GoTo ErrLine
            
            'derive a collection of registered filters from the filtergraph manager
            Set objRegisteredFilters = objFilterGraphManager.RegFilterCollection
            
            'enumerate through the registered filters
            For Each objRegFilterInfo In objRegisteredFilters
                If Trim(LCase(objRegFilterInfo.Name)) = "file writer" Then
                    objRegFilterInfo.Filter objFilterInfo
                ElseIf Trim(LCase(objRegFilterInfo.Name)) = "avi mux" Then
                    objRegFilterInfo.Filter objAVIMuxFilterInfo
                End If
            Next
            
            'derive the file sink filter tailored for vb
            Set objFileSinkFilterVB = objFilterInfo.Filter
            'assign the filename to the sink filter
            Call objFileSinkFilterVB.SetFileName(bstrFileName, Nothing)
            
            'clean-up & dereference
            If Not objFilterInfo Is Nothing Then Set objFilterInfo = Nothing
            If Not objRegFilterInfo Is Nothing Then Set objRegFilterInfo = Nothing
            If Not objFileSinkFilterVB Is Nothing Then Set objFileSinkFilterVB = Nothing
            If Not objAVIMuxFilterInfo Is Nothing Then Set objAVIMuxFilterInfo = Nothing
            If Not objRegisteredFilters Is Nothing Then Set objRegisteredFilters = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: RenderGroupPins
            ' * procedure description:  Renders the Pins out for the given timeline using the given render engine.
            ' *
            ' ******************************************************************************************************************************
            Public Sub RenderGroupPins(objRenderEngine As RenderEngine, objTimeline As AMTimeline)
            Dim objPin As IPin
            Dim nCount As Long
            Dim nGroupCount As Long
            Dim objPinInfo As IPinInfo
            Dim objFilterGraphManager As FilgraphManager
            On Local Error GoTo ErrLine
            
            If Not objTimeline Is Nothing Then
               If Not objRenderEngine Is Nothing Then
                  'obtain the group count
                  objTimeline.GetGroupCount nGroupCount
                  'exit the procedure if there are no group(s)
                  If nGroupCount = 0 Then Exit Sub
                  'obtain the filtergraph
                  objRenderEngine.GetFilterGraph objFilterGraphManager
                  'enumerate through the groups & render the pins
                   For nCount = 0 To nGroupCount - 1
                       objRenderEngine.GetGroupOutputPin nCount, objPin
                       If Not objPin Is Nothing Then
                           Set objPinInfo = GetPinInfo(objFilterGraphManager, objPin)
                           If Not objPinInfo Is Nothing Then
                               Call objPinInfo.Render
                           End If
                       End If
                   Next
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Resume Next
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: TransitionFriendlyNameToProgID
            ' * procedure description:  Returns the programmatic identifier for the given transition friendly name
            ' *
            ' ******************************************************************************************************************************
            Public Function TransitionFriendlyNameToProgID(bstrTransitionFriendlyName As String) As String
            On Local Error GoTo ErrLine
            
            Select Case LCase(Trim(bstrTransitionFriendlyName))
                Case "default"
                         TransitionFriendlyNameToProgID = "DxtJpegDll.DxtJpeg"
                Case "slide"
                         TransitionFriendlyNameToProgID = "DXImageTransform.Microsoft.CrSlide"
                Case "fade"
                         TransitionFriendlyNameToProgID = "DXImageTransform.Microsoft.Fade"
                Case "ripple"
                         TransitionFriendlyNameToProgID = "DXImageTransform.MetaCreations.Water"
                Case "circle"
                         TransitionFriendlyNameToProgID = "DXImageTransform.MetaCreations.Grid"
                Case "burn film"
                         TransitionFriendlyNameToProgID = "DXImageTransform.MetaCreations.BurnFilm"
                Case "barn doors"
                         TransitionFriendlyNameToProgID = "DXImageTransform.Microsoft.CrBarn"
            End Select
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function



            
            
            
' **************************************************************************************************************************************
' * PUBLIC INTERFACE- GENERAL PROCEDURES
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
            ' * procedure name: GetTempDirectory
            ' * procedure description:  Returns a bstr String representing the fully qualified path to the system's temp directory
            ' *
            ' ******************************************************************************************************************************
            Public Function GetTempDirectory() As String
            Dim bstrBuffer As String * MAX_PATH
            On Local Error GoTo ErrLine
            
            'call the win32api
            Call GetTempPath(MAX_PATH, bstrBuffer)
            'parse & return the value to the client
            GetTempDirectory = Buffer_ParseEx(bstrBuffer)
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: File_Exists
            ' * procedure description:  Returns true if the specified file does in fact exist.
            ' *
            ' ******************************************************************************************************************************
            Public Function File_Exists(bstrFileName As String) As Boolean
            Dim WFD As WIN32_FIND_DATA, hFile As Long
            On Local Error GoTo ErrLine
            
            WFD.cFileName = bstrFileName & vbNullChar
            hFile = FindFirstFile(bstrFileName, WFD)
            File_Exists = hFile <> INVALID_HANDLE_VALUE
            Call FindClose(hFile)
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: File_Delete
            ' * procedure description:  This will delete a File. Pass any of the specified optionals to invoke those particular features.
            ' *
            ' ******************************************************************************************************************************
            Public Function File_Delete(bstrFileName As String, Optional SendToRecycleBin As Boolean = True, Optional Confirm As Boolean = True, Optional ShowProgress As Boolean = True) As Long
            Dim fileop As SHFILEOPSTRUCT
            Dim WFD As WIN32_FIND_DATA, hFile As Long
            On Local Error GoTo ErrLine
            
            'check argument
            If Right(bstrFileName, 1) = "\" Then bstrFileName = Left(bstrFileName, (Len(bstrFileName) - 1))
            'ensure the file exists
            WFD.cFileName = bstrFileName & vbNullChar
            hFile = FindFirstFile(bstrFileName, WFD)
            If hFile = INVALID_HANDLE_VALUE Then
               Call FindClose(hFile)
               Exit Function
            Else: Call FindClose(hFile)
            End If
            'set the error mode
            Call SetErrorMode(SEM_NOOPENFILEERRORBOX + SEM_FAILCRITICALERRORS)
            'set up the file operation by the specified optionals
            With fileop
                .hWnd = 0: .wFunc = FO_DELETE
                .pFrom = UCase(bstrFileName) & vbNullChar & vbNullChar
                If SendToRecycleBin Then   'goes to recycle bin
                   .fFlags = FOF_ALLOWUNDO
                   If Confirm = False Then .fFlags = .fFlags + FOF_NOCONFIRMATION  'do not confirm
                   If ShowProgress = False Then .fFlags = .fFlags + FOF_SILENT  'do not show progress
                Else 'just delete the file
                   If Confirm = False Then .fFlags = .fFlags + FOF_NOCONFIRMATION  'do not confirm
                   If ShowProgress = False Then .fFlags = .fFlags + FOF_SILENT  'do not show progress
                End If
            End With
            'execute the file operation, return any errors..
            File_Delete = SHFileOperation(fileop)
            Exit Function
            
ErrLine:
            File_Delete = Err.Number  'if there was a abend in the procedure, return that too..
            Err.Clear
            Exit Function
            End Function
