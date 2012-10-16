VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "CInputMapper"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       ActionMap.cls
'  Content:    Use DirectInput action mapper to interpret input from many devices
'
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Option Explicit

Dim m_NumberofSemantics As Long
Dim m_diaf As DIACTIONFORMAT
Dim m_DIEnum As DirectInputEnumDevices8
Dim m_Devices(100) As DirectInputDevice8
Dim m_DeviceTypes(100) As Long
Dim m_NumDevices As Long
Dim m_DI As DirectInput8
Dim m_bInit As Boolean
Dim m_hwnd As Long
Dim m_strUserName As String
Dim m_cdParams As DICONFIGUREDEVICESPARAMS

Function GetDevice(i As Long) As DirectInputDevice8
    Set GetDevice = m_Devices(i)
End Function

Function GetNumDevices() As Long
    GetNumDevices = m_NumDevices
    
End Function

Function GetDInput() As DirectInput8
    Set GetDInput = m_DI
End Function

Function ConfigureDevices(Optional bAllowEdit = False)
    ReDim m_cdParams.ActionFormats(0)
    ReDim m_cdParams.UserNames(0)
    
    Dim i As Long
    
    m_cdParams.ActionFormats(0) = m_diaf
    m_cdParams.FormatCount = 1
    m_cdParams.UserCount = 1
    m_cdParams.UserNames(0) = m_strUserName
    If bAllowEdit Then
        m_DI.ConfigureDevices 0, m_cdParams, DICD_EDIT
    Else
        m_DI.ConfigureDevices 0, m_cdParams, DICD_DEFAULT
    End If
    
    m_diaf = m_cdParams.ActionFormats(0)
    
    'release existing devices
    For i = 1 To m_NumDevices
        If Not m_Devices(i) Is Nothing Then m_Devices(i).Unacquire
        Set m_Devices(i) = Nothing
    Next
    
    Set m_DIEnum = Nothing
    
    Dim ret As Long
    ret = CreateDevicesFromMAP(m_hwnd, m_strUserName, m_diaf.ActionMapName, _
                    m_diaf.guidActionMap, m_diaf.lGenre, m_diaf.lBufferSize, _
                    m_diaf.lAxisMin, m_diaf.lAxisMax)
    
End Function


'-----------------------------------------------------------------------------
' Name: CreateDevicesFromMap()
' Desc: Creation method for the class. Creates DInput, and enumerated (i.e.
'       builds a list) of "suitable" devices. By "suitable", we mean devices
'       that work with the DInput genre specified in the DIACTIONFORMAT
'       structure.
'-----------------------------------------------------------------------------
Function CreateDevicesFromMAP(hWnd As Long, UserName As String, MapName As String, MapGuid As String, Genre As CONST_DIGENRE, Optional buffersize = 16, Optional AxisMin = -100, Optional AxisMax = 100) As Boolean
    

    On Local Error Resume Next
    Dim i As Long
    
    ' Copy passed in arguments for internal use.
    m_hwnd = hWnd
    m_strUserName = UserName
    m_diaf.guidActionMap = MapGuid
    m_diaf.lGenre = Genre
    m_diaf.lBufferSize = buffersize
    m_diaf.lAxisMax = AxisMax
    m_diaf.lAxisMin = AxisMin
    m_diaf.ActionMapName = MapName
    
    'Create DInput
    Dim dx As DirectX8
    Set dx = New DirectX8
    Set m_DI = dx.DirectInputCreate()
    Set dx = Nothing
    
    m_diaf.lActionCount = m_NumberofSemantics

    ' Enumerate "suitable" devices that are attached
    Set m_DIEnum = m_DI.GetDevicesBySemantics(m_strUserName, m_diaf, 0)
    If Err.Number <> 0 Then
        CreateDevicesFromMAP = False
        Exit Function
    End If
    
    
    Dim devinst As DirectInputDeviceInstance8
    
    For i = 1 To m_DIEnum.GetCount
    
        Set devinst = m_DIEnum.GetItem(i)
        Set m_Devices(i) = m_DI.CreateDevice(devinst.GetGuidInstance)
        m_DeviceTypes(i) = devinst.GetDevType
        Set devinst = Nothing
        
        If m_DeviceTypes(i) = DI8DEVTYPE_MOUSE Then
            Dim dipl As DIPROPLONG
            dipl.lHow = DIPH_DEVICE
            dipl.lData = DIPROPAXISMODE_REL
            m_Devices(i).SetProperty "DIPROP_AXISMODE", dipl
        End If

        ' Obtain the action to device control mapping.
        m_Devices(i).BuildActionMap m_diaf, m_strUserName, 0
    
        ' Once actions have been mapped to the device controls the app can review
        ' the mapping and may want to modify the map. When done, call
        ' SetActionMap() to put the map into effect
        m_Devices(i).SetActionMap m_diaf, m_strUserName, 0

        ' Set the cooperative level
        m_Devices(i).SetCooperativeLevel m_hwnd, DISCL_EXCLUSIVE Or DISCL_FOREGROUND
    Next

    m_NumDevices = m_DIEnum.GetCount
    m_bInit = True
    CreateDevicesFromMAP = True
End Function


'-----------------------------------------------------------------------------
' Name: ClearMap()
' Desc:
'-----------------------------------------------------------------------------
Sub ClearMap()
    On Local Error Resume Next
    
    Dim i As Long
    
    m_NumberofSemantics = 0
    ReDim m_diaf.ActionArray(0)
    m_bInit = False
        
    For i = 0 To m_NumDevices
        If Not m_Devices(i) Is Nothing Then
            m_Devices(i).Unacquire
            Set m_Devices(i) = Nothing
        End If
    Next
    
    Set m_DI = Nothing
    Set m_DIEnum = Nothing
    

End Sub

'-----------------------------------------------------------------------------
' Name: AddAction()
' Desc:
'-----------------------------------------------------------------------------
Sub AddAction(user As Long, semantic As Long, flags As Long, strName As String)
    If m_bInit Then
        Debug.Print "can not add actions after CreateDevicesFromMAP has been called"
        Exit Sub
    End If
    
    ReDim Preserve m_diaf.ActionArray(m_NumberofSemantics)
        
    With m_diaf.ActionArray(m_NumberofSemantics)
        .ActionName = strName
        .lAppData = user
        .lFlags = flags
        .lSemantic = semantic
    End With
    m_NumberofSemantics = m_NumberofSemantics + 1
End Sub



