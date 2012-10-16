Attribute VB_Name = "D3DInit"

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       D3DInit.bas
'  Content:    VB D3DFramework global initialization module
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

' DOC:  Use with
' DOC:        D3DAnimation.cls
' DOC:        D3DFrame.cls
' DOC:        D3DMesh.cls
' DOC:        D3DSelectDevice.frm (optional)
' DOC:
' DOC:  Short list of usefull functions
' DOC:        D3DUtil_Init                  first call to framework
' DOC:        D3DUtil_LoadFromFile          loads an x-file
' DOC:        D3DUtil_SetupDefaultScene     setup a camera lights and materials
' DOC:        D3DUtil_SetupCamera           point camera
' DOC:        D3DUtil_SetupMediaPath        set directory to load textures from
' DOC:        D3DUtil_PresentAll            show graphic on the screen
' DOC:        D3DUtil_ResizeWindowed        resize for windowed modes
' DOC:        D3DUtil_ResizeFullscreen      resize to fullscreen mode
' DOC:        D3DUtil_CreateTextureInPool   create a texture

Option Explicit

Private Declare Function GetWindowRect Lib "user32.dll" (ByVal hwnd As Long, ByRef RECT As RECT) As Long
Private Declare Function GetClientRect Lib "user32.dll" (ByVal hwnd As Long, ByRef RECT As RECT) As Long
Private Declare Function SetWindowPos Lib "user32.dll" (ByVal hwnd As Long, ByVal hwndafter As Long, ByVal x As Long, ByVal y As Long, ByVal cx As Long, ByVal cy As Long, ByVal options As Long) As Long
Private Declare Function SetWindowLongA Lib "user32.dll" (ByVal hwnd As Long, ByVal nIndex As Long, ByVal newVal As Long) As Long
Private Declare Function GetWindowLongA Lib "user32.dll" (ByVal hwnd As Long, ByVal nIndex As Long) As Long


'------------------------------------------------------------------
'  User Defined types
'------------------------------------------------------------------

' DOC: Type that hold info about a display mode
Public Type D3DUTIL_MODEINFO
    lWidth As Long                                  'Screen width in this mode
    lHeight As Long                                 'Screen height in this mode
    format As CONST_D3DFORMAT                       'Pixel format in this mode
    VertexBehavior As CONST_D3DCREATEFLAGS          'Whether this mode does SW or HW vertex processing
End Type

' DOC: Type that hold info about a particular back buffer format
Public Type D3DUTIL_FORMATINFO
    format As CONST_D3DFORMAT
    usage As Long
    bCanDoWindowed As Boolean
    bCanDoFullscreen As Boolean
End Type

Public Type D3DUTIL_DEVTYPEINFO
    'List of compatible Modes for this device
    lNumModes As Long
    Modes() As D3DUTIL_MODEINFO
        
    'List of compatible formats for this device
    lNumFormats As Long
    FormatInfo() As D3DUTIL_FORMATINFO

    lCurrentMode As Long        'index into modes array of the current mode
    
End Type

' DOC: Type that holds info about adapters installed on the system
Public Type D3DUTIL_ADAPTERINFO
    'Device data
    DeviceType As CONST_D3DDEVTYPE                  'Reference, HAL
    d3dcaps As D3DCAPS8                             'Caps of this device
    sDesc As String                                 'Name of this device
    lCanDoWindowed As Long                          'Whether or not this device can work windowed
                
    DevTypeInfo(2) As D3DUTIL_DEVTYPEINFO           'format and display mode list for hal=1 for ref=2
    
    d3dai As D3DADAPTER_IDENTIFIER8
            
    DesktopMode As D3DDISPLAYMODE
    
    'CurrentState
    bWindowed As Boolean        'currently in windowed mode
    bReference As Boolean       'currently using reference rasterizer

    
    
End Type

'------------------------------------------------------------------
' DOC: Usefull globals
'------------------------------------------------------------------
'
Public g_Adapters() As D3DUTIL_ADAPTERINFO      ' Array of Adapter infos
Public g_lCurrentAdapter As Long                ' current adapter (index into infos)
Public g_lNumAdapters As Long                   ' size of the g_Adapters array
Public g_EnumCallback As Object                 ' object that defines VerifyDevice function

Public g_dx As DirectX8                         ' Root objects for DX8
Public g_d3dx As D3DX8                          ' Root objects for d3dx
Public g_d3d As Direct3D8                       ' Root object for d3d
Public g_dev As Direct3DDevice8                 ' D3D device
    
                                                ' Current state (use as read only)
Public g_d3dCaps As D3DCAPS8                    ' Current caps of g_dev
Public g_devType As CONST_D3DDEVTYPE            ' Current device type (hardware or software)
Public g_behaviorflags As Long                  ' Current VertexProcessing (hardware or software)
Public g_focushwnd As Long                      ' Current focus window handle
Public g_d3dpp As D3DPRESENT_PARAMETERS         ' Current presentation parameters

  
Public g_lWindowWidth As Long                   ' backbuffer width of windowed state
Public g_lWindowHeight As Long                  ' backbuffer  height of windowed state
Public g_WindowRect As RECT                     ' size of window (including title bar)



'------------------------------------------------------------------
' Public Functions
'------------------------------------------------------------------

Sub D3DUtil_Destory()
    Dim emptycaps As D3DCAPS8
    Dim emptyrect As RECT
    Dim emptypresent As D3DPRESENT_PARAMETERS
    
    D3DUtil_ReleaseAllTexturesFromPool
    
    Set g_dx = Nothing
    Set g_d3dx = Nothing
    Set g_dev = Nothing
    Set g_EnumCallback = Nothing
    
    g_focushwnd = 0
    g_behaviorflags = 0
    g_lWindowWidth = 0
    g_lWindowHeight = 0
    
    g_d3dCaps = emptycaps
    g_d3dpp = emptypresent
    g_WindowRect = emptyrect
    
    ReDim g_Adapters(0)
    g_lNumAdapters = 0
        
End Sub

'-----------------------------------------------------------------------------
'DOC: D3DUtil_Init
'DOC:   This function creates the following objects: DirectX8, Direct3D8,
'DOC:   Direc3DDevice8.
'DOC:
'DOC: Params:
'DOC:   bWindowed       Start in full screen or windowed mode
'DOC:
'DOC:   hwnd            starting hwnd to display graphics in
'DOC:                   This can be changed with a call to Reset if drawing
'DOC:                   to multiple windows
'DOC:                   For full screen operation the hwnd will have to belong
'DOC:                   to a top level window
'DOC:
'DOC:   AdapterIndex    The index of the display card to be used (most often 0)
'DOC:
'DOC:   ModeIndex       Ignored if bWindowed is TRUE
'DOC:                   Otherwise and index into the g_adapters(AdapterIndex).Modes
'DOC:                   array for a given width height and backbuffer format
'DOC:                   Use D3DUtil_FindDisplayMode to obtain an mode index given
'DOC:                   a desired height width and backbuffer format
'DOC:
'DOC:   CallbackObject  Can be Nothing or an object that has implemented
'DOC:                   VerifyDevice(usageflags as long ,format as CONST_D3DFORMAT)
'DOC:
'DOC:
'DOC: Remarks:
'DOC:   caps for the device are passed to VerifyDevice in the g_d3dcaps global
'DOC:
'-----------------------------------------------------------------------------

Public Function D3DUtil_Init(hwnd As Long, bWindowed As Boolean, AdapterIndex As Long, modeIndex As Long, devtype As CONST_D3DDEVTYPE, CallbackObject As Object) As Boolean
    
    On Local Error GoTo errOut

    ' Initialize the DirectX8 and d3dx8 objects
    If g_dx Is Nothing Then Set g_dx = New DirectX8
    If g_d3dx Is Nothing Then Set g_d3dx = New D3DX8
    
    ' Create the Direct3D object
    Set g_d3d = g_dx.Direct3DCreate

    ' Call the sub that builds a list of available adapters,
    ' adapter device types, and display modes
    Call D3DEnum_BuildAdapterList(CallbackObject)
    
    If bWindowed Then
        D3DUtil_Init = D3DUtil_InitWindowed(hwnd, AdapterIndex, devtype, True)
    Else
        D3DUtil_Init = D3DUtil_InitFullscreen(hwnd, AdapterIndex, modeIndex, devtype, True)
    End If
    
    Exit Function
    
errOut:
    Debug.Print "Failed D3DUtil_Init"
End Function


'-----------------------------------------------------------------------------
'DOC: D3DUtil_InitWindowed
'DOC:   This function creates the following objects: DirectX8, Direct3D8,
'DOC:   Direc3DDevice8.
'DOC:
'DOC: Params:
'DOC:
'DOC:   hwnd            starting hwnd to display graphics in
'DOC:                   This can be changed with a call to Reset if drawing
'DOC:                   to multiple windows
'DOC:                   For full screen operation the hwnd will have to belong
'DOC:                   to a top level window
'DOC:
'DOC:   AdapterIndex    The index of the display card to be used (most often 0)
'DOC:
'DOC:   DevType         Indicates if user wants HAL (hardware) or REF (software) rendering
'DOC:
'DOC:   bTryFallbacks   True if wants function to attempt to fallback to the reference device
'DOC:                   on faulure and display dialogs to that effect
'DOC:
'-----------------------------------------------------------------------------

Function D3DUtil_InitWindowed(hwnd As Long, AdapterIndex As Long, devtype As CONST_D3DDEVTYPE, bTryFallbacks As Boolean) As Boolean
   
    On Error GoTo errOut
    
        
    
    'save the current adapter
    g_lCurrentAdapter = AdapterIndex
    
    Dim d3ddm As D3DDISPLAYMODE

    ' Initialize the present parameters structure
    ' to use 1 back buffer and a 16 bit depth buffer
    ' change the autoDepthStencilFormat if you need stencil bits
    With g_d3dpp
        .BackBufferCount = 1
        .EnableAutoDepthStencil = 1
        .AutoDepthStencilFormat = D3DFMT_D16
        .hDeviceWindow = hwnd
    End With
        
    g_focushwnd = hwnd
    
    Dim rc As RECT
    Call GetClientRect(g_focushwnd, rc)
    g_lWindowWidth = rc.Right - rc.Left
    g_lWindowHeight = rc.bottom - rc.Top
    Call GetWindowRect(g_focushwnd, g_WindowRect)
    
    
    With g_Adapters(g_lCurrentAdapter)
    
        ' If running windowed, set the current desktop format
        ' as the format the device will use.
        ' note the the current mode and backbuffer width and height
        ' information is ignored by d3d
        Call g_d3d.GetAdapterDisplayMode(g_lCurrentAdapter, d3ddm)
        
        ' figure out if this format supports hardware acceleration
        ' by looking it up in our format list
        g_behaviorflags = D3DEnum_FindInFormatList(g_lCurrentAdapter, devtype, d3ddm.format)
        If g_behaviorflags <= 0 Then g_behaviorflags = D3DCREATE_SOFTWARE_VERTEXPROCESSING
        
        
        g_d3dpp.BackBufferFormat = d3ddm.format
        g_d3dpp.BackBufferWidth = 0
        g_d3dpp.BackBufferHeight = 0
        
        g_d3dpp.Windowed = 1
        g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD
        
                
        .bWindowed = True
        .DeviceType = devtype
        
        g_devType = devtype
       
                
        
        
    End With
            
    'Try to create the device now that we have everything set.
    On Local Error Resume Next
    Set g_dev = g_d3d.CreateDevice(g_lCurrentAdapter, devtype, g_focushwnd, g_behaviorflags, g_d3dpp)

    If Err.Number Then

        If bTryFallbacks = False Then Exit Function
        
        'If a HAL device was being attempted, try again with a REF device instead.
        If g_devType = D3DDEVTYPE_HAL Then
            Err.Clear

            'Make sure the user knows that this is less than an optimal 3D environment.
            MsgBox "No hardware support found. Switching to reference rasterizer.", vbInformation
            
            'reset our variable to use ref
            g_Adapters(g_lCurrentAdapter).DeviceType = D3DDEVTYPE_REF
            g_Adapters(g_lCurrentAdapter).bReference = True
            g_devType = D3DDEVTYPE_REF
            Set g_dev = g_d3d.CreateDevice(g_lCurrentAdapter, D3DDEVTYPE_REF, g_focushwnd, g_behaviorflags, g_d3dpp)
            
        End If
            
    End If

    If Err.Number Then
        
        'The app still hit an error. Both HAL and REF devices weren't created. The app will have to exit at this point.
        MsgBox "No suitable device was found to initialize D3D. Application will now exit.", vbCritical
        D3DUtil_InitWindowed = False
        End
        Exit Function

    End If

    'update our device caps data
    g_dev.GetDeviceCaps g_d3dCaps
    
    'set any state we need to initialize
    D3DXMatrixIdentity g_identityMatrix
    
    'set the reference flag if we choose a ref device
    With g_Adapters(g_lCurrentAdapter)
        If .DeviceType = D3DDEVTYPE_REF Then
            .bReference = True
        Else
            .bReference = False
        End If
    End With
    
    D3DUtil_InitWindowed = True
    Exit Function
    
errOut:
    Debug.Print "Failed D3DUtil_Init"

End Function


'-----------------------------------------------------------------------------
'DOC: D3DUtil_InitFullscreen
'DOC:   This function creates the following objects: DirectX8, Direct3D8,
'DOC:   Direc3DDevice8.
'DOC:
'DOC: Params:
'DOC:
'DOC:   hwnd            starting hwnd to display graphics in
'DOC:                   This can be changed with a call to Reset if drawing
'DOC:                   to multiple windows
'DOC:                   For full screen operation the hwnd will have to belong
'DOC:                   to a top level window
'DOC:
'DOC:   AdapterIndex    The index of the display card to be used (most often 0)
'DOC:
'DOC:   ModeIndex       index into the g_adapters(AdapterIndex).Modes for width
'DOC:                   height and format
'DOC:
'DOC:   DevType         Indicates if user wants HAL (hardware) or REF (software) rendering
'DOC:
'DOC:   bTryFallbacks   True if wants function to attempt to fallback to the reference device
'DOC:                   on faulure and display dialogs to that effect
'DOC:
'-----------------------------------------------------------------------------
Function D3DUtil_InitFullscreen(hwnd As Long, AdapterIndex As Long, modeIndex As Long, devtype As CONST_D3DDEVTYPE, bTryFallbacks As Boolean) As Boolean
    
    On Error GoTo errOut
    
    Dim ModeInfo As D3DUTIL_MODEINFO

    Dim rc As RECT
    
    'save the current adapter
    g_lCurrentAdapter = AdapterIndex
        
    
    g_focushwnd = hwnd
    
    
    'switching from windowed to fullscreen so save height and width
    If g_d3dpp.Windowed = 1 Then
        Call GetClientRect(g_focushwnd, rc)
        g_lWindowWidth = rc.Right - rc.Left
        g_lWindowHeight = rc.bottom - rc.Top
        Call GetWindowRect(g_focushwnd, g_WindowRect)
    End If

    
    ' Initialize the present parameters structure
    ' to use 1 back buffer and a 16 bit depth buffer
    ' change the autoDepthStencilFormat if you need stencil bits
    With g_d3dpp
        .BackBufferCount = 1
        .EnableAutoDepthStencil = 1
        .AutoDepthStencilFormat = D3DFMT_D16
        .hDeviceWindow = g_focushwnd
    End With
    
    
    
    
    With g_Adapters(g_lCurrentAdapter)
            
        With .DevTypeInfo(devtype)
            ModeInfo = .Modes(modeIndex)
            g_behaviorflags = .Modes(modeIndex).VertexBehavior
            
            g_d3dpp.BackBufferWidth = ModeInfo.lWidth
            g_d3dpp.BackBufferHeight = ModeInfo.lHeight
            g_d3dpp.BackBufferFormat = ModeInfo.format
            g_d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP
            g_d3dpp.Windowed = 0
            .lCurrentMode = modeIndex
        End With
             

        .bWindowed = False
        .DeviceType = devtype
        
        
        If g_behaviorflags = 0 Then g_behaviorflags = D3DCREATE_SOFTWARE_VERTEXPROCESSING
        g_devType = devtype
        
    End With
            
    'Try to create the device now that we have everything set.
    On Local Error Resume Next
    Set g_dev = g_d3d.CreateDevice(g_lCurrentAdapter, devtype, g_focushwnd, g_behaviorflags, g_d3dpp)

    If Err.Number Then
        If bTryFallbacks = False Then Exit Function

        'If a HAL device was being attempted, try again with a REF device instead.
        If g_devType = D3DDEVTYPE_HAL Then
            Err.Clear

            'Make sure the user knows that this is less than an optimal 3D environment.
            MsgBox "No hardware support found. Switching to reference rasterizer.", vbInformation
            
            'reset our variable to use ref
            g_Adapters(g_lCurrentAdapter).DeviceType = D3DDEVTYPE_REF
            g_devType = D3DDEVTYPE_REF
            Set g_dev = g_d3d.CreateDevice(g_lCurrentAdapter, D3DDEVTYPE_REF, g_focushwnd, g_behaviorflags, g_d3dpp)
            
        End If
            
    End If


    If Err.Number Then
        
        'The app still hit an error. Both HAL and REF devices weren't created. The app will have to exit at this point.
        MsgBox "No suitable device was found to initialize D3D. Application will now exit.", vbCritical
        D3DUtil_InitFullscreen = False
        End
        Exit Function

    End If

    'update our device caps data
    g_dev.GetDeviceCaps g_d3dCaps
    
    'set any state we need to initialize
    D3DXMatrixIdentity g_identityMatrix
    
    'set the reference flag if we choose a ref device
    With g_Adapters(g_lCurrentAdapter)
        If .DeviceType = D3DDEVTYPE_REF Then
           .bReference = True
        Else
           .bReference = False
        End If
    End With
    D3DUtil_InitFullscreen = True
    Exit Function
    
errOut:
    Debug.Print "Failed D3DUtil_InitFullscreen"
End Function


'-----------------------------------------------------------------------------
'DOC: D3DUtil_ResetWindowed
'DOC:
'DOC: Remarks
'DOC:   Used to move out of Fullscreen mode to windowed mode with out changing
'DOC:   the current device
'-----------------------------------------------------------------------------
Function D3DUtil_ResetWindowed() As Long
        Dim ws As Long
        Dim d3dppnew As D3DPRESENT_PARAMETERS
        
        On Local Error GoTo errOut
        
        d3dppnew.Windowed = 1
        d3dppnew.BackBufferFormat = g_Adapters(g_lCurrentAdapter).DesktopMode.format
        d3dppnew.EnableAutoDepthStencil = g_d3dpp.EnableAutoDepthStencil
        d3dppnew.AutoDepthStencilFormat = g_d3dpp.AutoDepthStencilFormat
        d3dppnew.SwapEffect = D3DSWAPEFFECT_DISCARD
        d3dppnew.hDeviceWindow = g_d3dpp.hDeviceWindow
        
        g_dev.Reset d3dppnew
        
        g_d3dpp = d3dppnew
        
        Const GWL_EXSTYLE = -20
        Const GWL_STYLE = -16
        Const WS_EX_TOPMOST = 8
        Const HWND_NOTOPMOST = -2
            
                
        With g_WindowRect
            Call SetWindowPos(g_focushwnd, HWND_NOTOPMOST, .Left, .Top, .Right - .Left, .bottom - .Top, 0)
            ws = GetWindowLongA(g_focushwnd, GWL_STYLE)
            If (ws And WS_EX_TOPMOST) = WS_EX_TOPMOST Then
                ws = ws - WS_EX_TOPMOST
                Call SetWindowLongA(g_focushwnd, GWL_STYLE, ws)
            End If
        End With
        
        
        DoEvents
        D3DUtil_ResetWindowed = 0
        Exit Function
errOut:
    D3DUtil_ResetWindowed = Err.Number
    Debug.Print "err in ResetWindow"
End Function


'-----------------------------------------------------------------------------
'DOC: D3DUtil_ResetFullscreen
'DOC:
'DOC: Remarks
'DOC:   Used to to toggle from windowed mode to the current fullscreen mode
'DOC:   Without changing the current device
'-----------------------------------------------------------------------------
Function D3DUtil_ResetFullscreen() As Long
        Dim hr As Long
        Dim lMode As Long
        Dim rc As RECT
        Dim devtype As CONST_D3DDEVTYPE
        On Local Error GoTo errOut
        If g_d3dpp.Windowed = 1 Then
            Call GetClientRect(g_focushwnd, rc)
            g_lWindowWidth = rc.Right - rc.Left
            g_lWindowHeight = rc.bottom - rc.Top
            Call GetWindowRect(g_focushwnd, g_WindowRect)
        End If
        
        devtype = g_Adapters(g_lCurrentAdapter).DeviceType
        With g_Adapters(g_lCurrentAdapter).DevTypeInfo(devtype)
            g_d3dpp.Windowed = 0
            g_d3dpp.BackBufferWidth = .Modes(.lCurrentMode).lWidth
            g_d3dpp.BackBufferHeight = .Modes(.lCurrentMode).lHeight
            g_d3dpp.BackBufferFormat = .Modes(.lCurrentMode).format
        End With
        
        g_dev.Reset g_d3dpp
errOut:
        D3DUtil_ResetFullscreen = Err.Number
End Function

'-----------------------------------------------------------------------------
'DOC: D3DEnum_BuildAdapterList
'DOC:   Used to intialzed a list of valid adapters and display modes
'DOC:
'DOC: Params:
'DOC:   EnumCallback    - can be Nothing or an object that has implemented
'DOC:                     VerifyDevice(usageflags as long ,format as CONST_D3DFORMAT)
'DOC:                     ussgeflags can be
'DOC:                           D3DCREATE_SOFTWARE_VERTEXPROCESSING
'DOC:                           D3DCREATE_HARDWARE_VERTEXPROCESSING
'DOC: Remarks:
'DOC:   caps for the device are passed to VerifyDevice in the g_d3dcaps global
'DOC:
'-----------------------------------------------------------------------------
Public Function D3DEnum_BuildAdapterList(EnumCallback As Object) As Boolean
    
    On Local Error GoTo errOut
    
    Dim lAdapter As Long
        
    ' empty the list
    Call D3DEnum_Cleanup
            
    ' create d3d and dx objects if not already created
    If g_dx Is Nothing Then Set g_dx = New DirectX8
    If g_d3d Is Nothing Then Set g_d3d = g_dx.Direct3DCreate
    If g_d3dx Is Nothing Then Set g_d3dx = New D3DX8
    
    ' save callback
    Set g_EnumCallback = EnumCallback
    
    ' Make space for new adapter
    g_lNumAdapters = g_d3d.GetAdapterCount
    ReDim g_Adapters(g_lNumAdapters)
    
    ' Loop through all the adapters on the system
    For lAdapter = 0 To g_lNumAdapters - 1
    
        ' build a list of valid backbuffer formats
        D3DEnum_BuildValidFormatList lAdapter, D3DDEVTYPE_HAL
        D3DEnum_BuildValidFormatList lAdapter, D3DDEVTYPE_REF
        
                
        ' build a list of valid display modes for those formats
        D3DEnum_BuildDisplayModeList lAdapter, D3DDEVTYPE_HAL
        D3DEnum_BuildDisplayModeList lAdapter, D3DDEVTYPE_REF
        
        ' get the adapter identifier
        g_d3d.GetAdapterIdentifier lAdapter, 0, g_Adapters(lAdapter).d3dai
        
    Next
    
    D3DEnum_BuildAdapterList = True
    Exit Function
    
errOut:
    Debug.Print "Failed D3DEnum_BuildAdapterList"
End Function



'-----------------------------------------------------------------------------
'DOC:  D3DUtil_ResizeWindowed
'DOC:
'DOC:  Paramters
'DOC:       hWnd device window
'DOCL
'DOC:  Remarks
'DOC:       use when already in windowed mode to resize the backbuffer
'DOC:       do not use to switch from fullscreen to windowed mode
'-----------------------------------------------------------------------------
Function D3DUtil_ResizeWindowed(hwnd As Long) As Boolean
    On Local Error GoTo errOut
    
    Dim d3dpp As D3DPRESENT_PARAMETERS
    Dim rc As RECT
    
    d3dpp = g_d3dpp
    
    If d3dpp.Windowed = 0 Then Exit Function
    
    g_focushwnd = hwnd
    Call GetClientRect(g_focushwnd, rc)
    g_lWindowWidth = rc.Right - rc.Left
    g_lWindowHeight = rc.bottom - rc.Top
    Call GetWindowRect(g_focushwnd, g_WindowRect)
    
        
    
    d3dpp.BackBufferWidth = 0 'g_lWindowWidth
    d3dpp.BackBufferHeight = 0 'g_lWindowHeight
    d3dpp.hDeviceWindow = hwnd
    d3dpp.Windowed = 1
               
    g_dev.Reset d3dpp
    
    g_d3dpp = d3dpp
    
    
    g_Adapters(g_lCurrentAdapter).bWindowed = True

    D3DUtil_ResizeWindowed = True
    Exit Function
    
errOut:
    Debug.Print "D3DUtil_ResizeWindowed failed - make sure width and height are in pixels"
    If Err.Number <> 0 Then 'The call to reset failed.
        D3DUtil_ResizeWindowed = False
        MsgBox "Could not reset the Direct3D Device." & vbCrLf & "This sample will now exit.", vbOKOnly Or vbCritical, "Failure"
        End
        Exit Function
    End If
End Function


'-----------------------------------------------------------------------------
'DOC:  D3DUtil_ResizeFullscreen
'DOC:
'DOC:  Paramters
'DOC:       hWnd device window
'DOC:       modeIndex index into Modes list
'DOC:
'DOC:  Remarks
'DOC:       D3DUtil_Init or D3DEnum_BuildAdapterList must have been called
'DOC:       prior to call D3DUtil_ResizeFullscreen
'DOC:       Use this method when moving from windowed mode to fullscreen
'DOC:       on the current device
'DOC:       Note that all device state is lost and that the caller
'DOC:       will need to call ther RestoreDeviceObjects function
'DOC:
'-----------------------------------------------------------------------------
Function D3DUtil_ResizeFullscreen(hwnd As Long, modeIndex As Long) As Boolean
    On Local Error GoTo errOut
    
    Dim d3dpp As D3DPRESENT_PARAMETERS
    Dim devtype As CONST_D3DDEVTYPE
    Dim prevmode As Long
    
    'let ResizeWindowed know we are trying to go fullscreen
    prevmode = g_d3dpp.Windowed
    g_d3dpp.Windowed = 0
        
    devtype = g_Adapters(g_lCurrentAdapter).DeviceType
    With g_Adapters(g_lCurrentAdapter).DevTypeInfo(devtype).Modes(modeIndex)
        d3dpp.BackBufferWidth = .lWidth
        d3dpp.BackBufferHeight = .lHeight
        d3dpp.BackBufferFormat = .format
        d3dpp.hDeviceWindow = hwnd
        d3dpp.AutoDepthStencilFormat = g_d3dpp.AutoDepthStencilFormat
        d3dpp.EnableAutoDepthStencil = g_d3dpp.EnableAutoDepthStencil
        d3dpp.SwapEffect = D3DSWAPEFFECT_FLIP
        d3dpp.Windowed = 0
    End With
    
    g_dev.Reset d3dpp
    
    g_d3dpp = d3dpp
    
    'reset succeeded so set new behavior flags
    With g_Adapters(g_lCurrentAdapter)
        g_behaviorflags = .DevTypeInfo(devtype).Modes(modeIndex).VertexBehavior
        .bWindowed = False
    End With
    
    D3DUtil_ResizeFullscreen = True
    Exit Function
    
errOut:
    'we where unsuccessfull in going fullscreen
    'indicate we are still in previous mode
    g_d3dpp.Windowed = prevmode
    Debug.Print "D3DUtil_ResizeWindowed failed - make sure width and height are in pixels"
End Function


'-----------------------------------------------------------------------------
'DOC: D3DUtil_DefaultInitWindowed
'DOC:   Used to intialzed D3DUtil device in a windowed mode
'DOC  Params:
'DOC:   iAdapter    DisplayAdapter ordinal
'DOC:   hwnd        Display hwnd
'DOC: Remarks:
'DOC:
'DOC:   Users can initialze the g_d3d and g_dev objects themselves
'DOC:   and not use this function be sure to initialize
'DOC:       g_iAdapter, g_devType,g_behaviorFlags,g_focushwnd,g_presentParams
'DOC:
'DOC:   This function defaults to using SOFTWARE_VERTEXPROCESSING
'DOC:   and requires HAL 3d support
'-----------------------------------------------------------------------------

Public Function D3DUtil_DefaultInitWindowed(iAdapter As Long, hwnd As Long) As Boolean
    On Local Error GoTo errOut
    
    Dim emptyparams As D3DPRESENT_PARAMETERS
    
    If g_dx Is Nothing Then Set g_dx = New DirectX8
    If g_d3dx Is Nothing Then Set g_d3dx = New D3DX8
    
    If g_d3d Is Nothing Then Set g_d3d = g_dx.Direct3DCreate
    
    
    g_lCurrentAdapter = iAdapter
    g_devType = D3DDEVTYPE_HAL
    g_behaviorflags = D3DCREATE_SOFTWARE_VERTEXPROCESSING
    g_focushwnd = hwnd
    g_d3dpp = emptyparams
    
    Dim dm As D3DDISPLAYMODE
    
    g_d3d.GetAdapterDisplayMode iAdapter, dm
    
    
    With g_d3dpp
        .BackBufferFormat = dm.format
        .EnableAutoDepthStencil = 1 'TRUE
        .AutoDepthStencilFormat = D3DFMT_D16
        .Windowed = 1   'TRUE
        .SwapEffect = D3DSWAPEFFECT_DISCARD
    End With
            
            
    Set g_dev = g_d3d.CreateDevice(iAdapter, g_devType, g_focushwnd, g_behaviorflags, g_d3dpp)
    
    g_dev.GetDeviceCaps g_d3dCaps
    
    D3DUtil_DefaultInitWindowed = True
    Exit Function
    
errOut:
End Function


'-----------------------------------------------------------------------------
'DOC: D3DUtil_DefaultInitFullscreen
'DOC:   Used to intialzed D3DUtil device in a windowed mode
'DOC  Params:
'DOC:   iAdapter    DisplayAdapter ordinal
'DOC:   hwnd        Display hwnd
'DOC:   w           width
'DOC    h           height
'DOC    fmt         desired format
'DOC: Remarks:
'DOC:
'DOC:   Users can initialze the g_d3d and g_dev objects themselves
'DOC:   and not use this function be sure to initialize
'DOC:       g_iAdapter, g_devType,g_behaviorFlags,g_focushwnd,g_presentParams
'DOC:
'DOC:   This function defaults to using SOFTWARE_VERTEXPROCESSING
'DOC:   and requires HAL 3d support
'-----------------------------------------------------------------------------

Public Function D3DUtil_DefaultInitFullscreen(iAdapter As Long, hwnd As Long, w As Long, h As Long, fmt As CONST_D3DFORMAT) As Boolean
    On Local Error GoTo errOut
    
    Dim emptyparams As D3DPRESENT_PARAMETERS
    
    If g_dx Is Nothing Then Set g_dx = New DirectX8
    If g_d3dx Is Nothing Then Set g_d3dx = New D3DX8
    
    If g_d3d Is Nothing Then Set g_d3d = g_dx.Direct3DCreate
    
    
    g_lCurrentAdapter = iAdapter
    g_devType = D3DDEVTYPE_HAL
    g_behaviorflags = D3DCREATE_SOFTWARE_VERTEXPROCESSING
    g_focushwnd = hwnd
    g_d3dpp = emptyparams
    
    Dim dm As D3DDISPLAYMODE
    
    g_d3d.GetAdapterDisplayMode iAdapter, dm
    
    
    With g_d3dpp
        .BackBufferFormat = fmt
        .EnableAutoDepthStencil = 1 'TRUE
        .AutoDepthStencilFormat = D3DFMT_D16
        .BackBufferWidth = w
        .BackBufferHeight = h
        .Windowed = 0   'FALSE
        .SwapEffect = D3DSWAPEFFECT_FLIP
    End With
            
            
    Set g_dev = g_d3d.CreateDevice(iAdapter, g_devType, g_focushwnd, g_behaviorflags, g_d3dpp)
    
    g_dev.GetDeviceCaps g_d3dCaps
    
    D3DUtil_DefaultInitFullscreen = True
    Exit Function
    
errOut:
End Function





'-----------------------------------------------------------------------------
'DOC: D3DEnum_Cleanup
'DOC:   Used to release any reference to the callback object passed in
'DOC:   and deallocate memory
'DOC: Params:
'DOC:   none
'DOC: Remarks:
'DOC:   none
'-----------------------------------------------------------------------------
Public Sub D3DEnum_Cleanup()
    Set g_EnumCallback = Nothing
    ReDim g_Adapters(0)
End Sub


'------------------------------------------------------------------
' Private Functions
'------------------------------------------------------------------

'-----------------------------------------------------------------------------
' D3DEnum_BuildValidFormatList
'-----------------------------------------------------------------------------
Private Sub D3DEnum_BuildValidFormatList(lAdapter As Long, devtype As CONST_D3DDEVTYPE)
                        
        Dim lMode As Long
        Dim lUsage As Long
        Dim NumModes As Long
        Dim DisplayMode As D3DDISPLAYMODE
        Dim bCanDoWindowed As Boolean
        Dim bCanDoFullscreen As Boolean
                
        
        With g_Adapters(lAdapter).DevTypeInfo(devtype)
        
            ' Reset the number of available formats to
            .lNumFormats = 0
        
            ' Get the number of display modes
            ' a display mode is a size and format (ie 640x480 X8R8G8B8 60hz)
            NumModes = g_d3d.GetAdapterModeCount(lAdapter)
            ReDim .FormatInfo(NumModes)
                                
            ' Loop through all the display modes
            For lMode = 0 To NumModes - 1
                    
                ' Get information about this adapter in all the modes it supports
                Call g_d3d.EnumAdapterModes(lAdapter, lMode, DisplayMode)
                                
                ' See if the format is already in our format list
                If -1 <> D3DEnum_FindInFormatList(lAdapter, devtype, DisplayMode.format) Then GoTo Continue
                                    
                ' Check the compatiblity of the format
                
                lUsage = D3DEnum_CheckFormatCompatibility(lAdapter, devtype, DisplayMode.format, bCanDoWindowed, bCanDoFullscreen)
                                                                            
                ' Usage will come back -1 if VerifyDevice reject format
                If -1 = lUsage Then GoTo Continue
                
                ' Add the valid format and ussage
                .FormatInfo(.lNumFormats).format = DisplayMode.format
                .FormatInfo(.lNumFormats).usage = lUsage
                .FormatInfo(.lNumFormats).bCanDoWindowed = bCanDoWindowed
                .FormatInfo(.lNumFormats).bCanDoFullscreen = bCanDoFullscreen
                .lNumFormats = .lNumFormats + 1

                                
Continue:
            Next
            
        End With

End Sub


'-----------------------------------------------------------------------------
' D3DEnum_BuildDisplayModeList
'-----------------------------------------------------------------------------
Private Sub D3DEnum_BuildDisplayModeList(lAdapter As Long, devtype As CONST_D3DDEVTYPE)
                        
        Dim lMode As Long
        Dim NumModes As Long
        Dim DisplayMode As D3DDISPLAYMODE

        With g_Adapters(lAdapter).DevTypeInfo(devtype)
        
            ' Reset the number of validated display modes to 0
            .lNumModes = 0
            
            ' Get the number of display modes
            ' Note this list includes refresh rates
            ' a display mode is a size and format (ie 640x480 X8R8G8B8 60hz)
            NumModes = g_d3d.GetAdapterModeCount(lAdapter)

            ' Allocate space for our mode list
            ReDim .Modes(NumModes)

            ' Save the format of the desktop for windowed operation
            Call g_d3d.GetAdapterDisplayMode(lAdapter, g_Adapters(lAdapter).DesktopMode)
                                
            ' Loop through all the display modes
            For lMode = 0 To NumModes - 1
                    
                ' Get information about this adapter in all the modes it supports
                Call g_d3d.EnumAdapterModes(lAdapter, lMode, DisplayMode)
                
                ' filter out low resolution modes
                If DisplayMode.width < 640 Or DisplayMode.height < 400 Then GoTo Continue
                
                ' filter out modes allready in the list
                ' that might differ only in refresh rate
                If -1 <> D3DEnum_FindInDisplayModeList(lAdapter, devtype, DisplayMode) Then GoTo Continue
                
                
                ' filter out modes with formats that arent confirmed to work
                ' see BuildFormatList and ConfirmFormatList
                If -1 = D3DEnum_FindInFormatList(lAdapter, devtype, DisplayMode.format) Then GoTo Continue
                                                
                ' At this point, the modes format has been validated,
                ' is not a duplicate refresh rate, and not a low res mode
                ' Add the mode to the list of working modes for the adapter
                .Modes(.lNumModes).lHeight = DisplayMode.height
                .Modes(.lNumModes).lWidth = DisplayMode.width
                .Modes(.lNumModes).format = DisplayMode.format
                .lNumModes = .lNumModes + 1
                                            
Continue:
            Next
            
        End With

End Sub

'-----------------------------------------------------------------------------
' D3DEnum_FindInDisplayModeList
'-----------------------------------------------------------------------------
Private Function D3DEnum_FindInDisplayModeList(lAdapter As Long, devtype As CONST_D3DDEVTYPE, DisplayMode As D3DDISPLAYMODE) As Long
    
    Dim lMode As Long
    Dim NumModes As Long
    
    NumModes = g_Adapters(lAdapter).DevTypeInfo(devtype).lNumModes
    D3DEnum_FindInDisplayModeList = -1
    
    For lMode = 0 To NumModes - 1
      With g_Adapters(lAdapter).DevTypeInfo(devtype).Modes(lMode)
          If .lWidth = DisplayMode.width And _
              .lHeight = DisplayMode.height And _
              .format = DisplayMode.format Then
              D3DEnum_FindInDisplayModeList = lMode
              Exit Function
          End If
      End With
    Next
    
End Function

'-----------------------------------------------------------------------------
' D3DEnum_FindInFormatList
'-----------------------------------------------------------------------------
Private Function D3DEnum_FindInFormatList(lAdapter As Long, devtype As CONST_D3DDEVTYPE, format As CONST_D3DFORMAT) As Long
    
    Dim lFormat As Long
    Dim NumFormats As Long
    
    NumFormats = g_Adapters(lAdapter).DevTypeInfo(devtype).lNumFormats
    D3DEnum_FindInFormatList = -1
    
    For lFormat = 0 To NumFormats - 1
      With g_Adapters(lAdapter).DevTypeInfo(devtype).FormatInfo(lFormat)
          If .format = format Then
             D3DEnum_FindInFormatList = .usage
             Exit Function
          End If
      End With
    Next
    
    D3DEnum_FindInFormatList = -1
    
End Function


'-----------------------------------------------------------------------------
' D3DEnum_CheckFormatCompatibility
'-----------------------------------------------------------------------------
Private Function D3DEnum_CheckFormatCompatibility(lAdapter As Long, DeviceType As CONST_D3DDEVTYPE, format As CONST_D3DFORMAT, ByRef OutCanDoWindowed As Boolean, ByRef OutCanDoFullscreen As Boolean) As Long
        On Local Error GoTo errOut

        D3DEnum_CheckFormatCompatibility = -1
        
        Dim d3dcaps As D3DCAPS8
        Dim flags As Long

        ' Filter out incompatible backbuffers
        ' Note: framework always has the backbuffer and the frontbuffer (screen) format matching
        OutCanDoWindowed = True: OutCanDoFullscreen = True
        If 0 <> g_d3d.CheckDeviceType(lAdapter, DeviceType, format, format, 0) Then OutCanDoWindowed = False
        If 0 <> g_d3d.CheckDeviceType(lAdapter, DeviceType, format, format, 1) Then OutCanDoFullscreen = False
        If (OutCanDoWindowed = False) And (OutCanDoFullscreen = False) Then Exit Function

        ' If no form was passed in to use as a callback
        ' then default to sofware vertex processing

        ' Get the device capablities
        g_d3d.GetDeviceCaps lAdapter, DeviceType, g_d3dCaps
        g_Adapters(lAdapter).d3dcaps = g_d3dCaps
        
        ' If user doesnt want to verify the device (didnt provide a callback)
        ' fall back to software
        D3DEnum_CheckFormatCompatibility = D3DCREATE_SOFTWARE_VERTEXPROCESSING
        If g_EnumCallback Is Nothing Then Exit Function
        
        ' Confirm the device for HW vertex processing
        flags = D3DCREATE_HARDWARE_VERTEXPROCESSING
        D3DEnum_CheckFormatCompatibility = flags
        If g_d3dCaps.DevCaps And D3DDEVCAPS_HWTRANSFORMANDLIGHT Then
           If g_EnumCallback.VerifyDevice(flags, format) Then Exit Function
        End If
        
        ' Try Software VertexProcesing
        flags = D3DCREATE_SOFTWARE_VERTEXPROCESSING
        D3DEnum_CheckFormatCompatibility = flags
        If g_EnumCallback.VerifyDevice(flags, format) Then Exit Function
                                
        ' Fail
        D3DEnum_CheckFormatCompatibility = -1
        
        Exit Function
errOut:

End Function



