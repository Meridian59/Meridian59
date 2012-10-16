VERSION 5.00
Begin VB.Form frmSelectDevice 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Select Device"
   ClientHeight    =   2805
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   5865
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   2805
   ScaleWidth      =   5865
   ShowInTaskbar   =   0   'False
   StartUpPosition =   3  'Windows Default
   Begin VB.Frame optRenderingModeoptRenderingMode 
      Caption         =   "Rendering Mode"
      Height          =   1335
      Left            =   120
      TabIndex        =   7
      Top             =   1320
      Width           =   4575
      Begin VB.ComboBox cboFullScreenMode 
         Enabled         =   0   'False
         Height          =   315
         Left            =   2040
         Style           =   2  'Dropdown List
         TabIndex        =   10
         Top             =   720
         Width           =   2295
      End
      Begin VB.OptionButton optRenderingMode 
         Caption         =   "&Fullscreen mode"
         Height          =   375
         Index           =   1
         Left            =   240
         TabIndex        =   9
         Top             =   690
         Width           =   1455
      End
      Begin VB.OptionButton optRenderingMode 
         Caption         =   "Use desktop &window"
         Height          =   375
         Index           =   0
         Left            =   240
         TabIndex        =   8
         Top             =   240
         Value           =   -1  'True
         Width           =   1815
      End
   End
   Begin VB.CommandButton cmdCancel 
      Caption         =   "Cancel"
      Height          =   375
      Left            =   4800
      TabIndex        =   4
      Top             =   720
      Width           =   975
   End
   Begin VB.CommandButton cmdOk 
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   4800
      TabIndex        =   3
      Top             =   240
      Width           =   975
   End
   Begin VB.Frame Frame1 
      Caption         =   "Rendering device"
      Height          =   1095
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   4575
      Begin VB.ComboBox cboDevice 
         Height          =   315
         Left            =   1440
         Style           =   2  'Dropdown List
         TabIndex        =   6
         Top             =   600
         Width           =   2775
      End
      Begin VB.ComboBox cboAdapter 
         Height          =   315
         Left            =   1440
         Style           =   2  'Dropdown List
         TabIndex        =   2
         Top             =   240
         Width           =   2775
      End
      Begin VB.Label Label2 
         AutoSize        =   -1  'True
         Caption         =   "D3D &device:"
         Height          =   195
         Left            =   360
         TabIndex        =   5
         Top             =   660
         Width           =   900
      End
      Begin VB.Label Label1 
         AutoSize        =   -1  'True
         Caption         =   "&Adapter:"
         Height          =   195
         Left            =   360
         TabIndex        =   1
         Top             =   300
         Width           =   600
      End
   End
End
Attribute VB_Name = "frmSelectDevice"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit


Dim m_callback As Object

Public Sub SelectDevice(callback As Object)
    
    If callback Is Nothing Then Exit Sub
    Set m_callback = callback
    
    Dim dm As D3DDISPLAYMODE
    
    If g_d3dpp.Windowed = 0 Then

        m_callback.InvalidateDeviceObjects
    
        D3DUtil_ResetWindowed

        m_callback.RestoreDeviceObjects
        
    End If
    
    Me.Show 1
    
    Set m_callback = Nothing
    
End Sub


Private Sub cboAdapter_Click()
    
    Dim devtype As CONST_D3DDEVTYPE
      
    If (cboDevice.ListIndex = 1) Then
        devtype = D3DDEVTYPE_REF
    Else
        devtype = D3DDEVTYPE_HAL
    End If
       
    Call UpdateModes(cboAdapter.ListIndex, devtype)
    
End Sub

Private Sub cboDevice_Change()
    
    Dim devtype As CONST_D3DDEVTYPE
    
    If (cboDevice.ListIndex = 1) Then
        devtype = D3DDEVTYPE_REF
    Else
        devtype = D3DDEVTYPE_HAL
    End If
       
    Call UpdateModes(cboAdapter.ListIndex, devtype)
    
End Sub

Private Sub cmdCancel_Click()

    Set m_callback = Nothing
    Unload Me
    
End Sub

Private Sub cmdOk_Click()

    On Local Error Resume Next
    
    Dim bAdapterChanged As Boolean
    Dim bRasterizerChanged As Boolean
    Dim bRef As Boolean
    Dim lWindowed As Long
    Dim AdapterID As Long
    Dim ModeID As Long
    Dim devtype As CONST_D3DDEVTYPE
    
    AdapterID = cboAdapter.ListIndex
    ModeID = cboFullScreenMode.ListIndex
        
    ' see if user changed adapters
    If g_lCurrentAdapter <> AdapterID Then bAdapterChanged = True
                
    bRef = g_Adapters(g_lCurrentAdapter).bReference
        
    If (cboDevice.ListIndex = 1) Then
        devtype = D3DDEVTYPE_REF
    Else
        devtype = D3DDEVTYPE_HAL
    End If
    
    ' see if user changed rasterizers
    If (devtype = D3DDEVTYPE_REF And bRef = False) Then bRasterizerChanged = True
    If (devtype = D3DDEVTYPE_HAL And bRef = True) Then bRasterizerChanged = True
    
    
    If optRenderingMode(1).Value = True Then
        lWindowed = 0
    Else
        lWindowed = 1
    End If
        
    ' if they didnt change adapters or switch to refrast, then we can just use reset
    If bAdapterChanged = False And bRasterizerChanged = False Then
                
        'If trying to go Fullscreen
        If lWindowed = 0 Then
        
            'call g_dev.reset
            Call D3DUtil_ResizeFullscreen(g_focushwnd, cboFullScreenMode.ListIndex)
        
        Else
            
            Call D3DUtil_ResizeWindowed(g_focushwnd)
        
        End If
        
        'tell user needs to restore device objects
        m_callback.RestoreDeviceObjects
        
        'exit modal dialog
        Unload Me
        Exit Sub

    End If
    
    Set g_dev = Nothing
    
    D3DUtil_ReleaseAllTexturesFromPool
    
    'tell user to lose reference counts in its objects device objects
    m_callback.InvalidateDeviceObjects
    m_callback.DeleteDeviceObjects
    
    'Reinitialize D3D
    If lWindowed = 0 Then
        D3DUtil_InitFullscreen g_focushwnd, AdapterID, ModeID, devtype, True
    Else
        D3DUtil_InitWindowed g_focushwnd, AdapterID, devtype, True
    End If
    If g_dev Is Nothing Then
        'The app still hit an error. Both HAL and REF devices weren't created. The app will have to exit at this point.
        MsgBox "No suitable device was found to initialize D3D. Application will now exit.", vbCritical
        End
        Exit Sub
    End If
    
        
    'tell user to re-create device objects
    m_callback.InitDeviceObjects
    
    'tell user to restore device objects
    m_callback.RestoreDeviceObjects
    
    'exit modal dialog
    Unload Me

End Sub

Private Sub Form_Load()
        
    Call UpdateAdapters
    Call UpdateDevices(g_lCurrentAdapter)
    Call UpdateModes(g_lCurrentAdapter, g_Adapters(g_lCurrentAdapter).DeviceType)
    
End Sub

Private Sub UpdateAdapters()

    Dim i As Long, j As Long
    Dim sDescription As String
    
    cboAdapter.Clear
        
    For i = 0 To g_lNumAdapters - 1
        
        sDescription = vbNullString
'        For j = 0 To 511
'            sDescription = sDescription & Chr$(g_Adapters(i).d3dai.Description(j))
'        Next
'        sDescription = Replace$(sDescription, Chr$(0), " ")
        sDescription = StrConv(g_Adapters(i).d3dai.Description, vbUnicode)
        cboAdapter.AddItem sDescription
    Next
    
    cboAdapter.ListIndex = g_lCurrentAdapter
    
End Sub

Private Sub UpdateDevices(adapter As Long)

    Dim i As Long
    
    cboDevice.Clear
    
    cboDevice.AddItem "HAL"
    cboDevice.AddItem "REF"
    
    'If g_Adapters(g_lCurrentAdapter).bReference Then
    If g_Adapters(adapter).bReference Then
        cboDevice.ListIndex = 1
    Else
        cboDevice.ListIndex = 0
    End If
    
End Sub

Private Sub UpdateModes(adapter As Long, devtype As CONST_D3DDEVTYPE)

    Dim i As Long
    Dim pAdapter As D3DUTIL_ADAPTERINFO
    Dim sModeString As String
    
    cboFullScreenMode.Clear
            
    With g_Adapters(adapter).DevTypeInfo(devtype)
        For i = 0 To .lNumModes - 1
            sModeString = .Modes(i).lWidth & " x "
            sModeString = sModeString & .Modes(i).lHeight & " x "
            If .Modes(i).format = D3DFMT_X8R8G8B8 Or _
                .Modes(i).format = D3DFMT_A8R8G8B8 Or _
                .Modes(i).format = D3DFMT_R8G8B8 Then
                sModeString = sModeString & "32"
            Else
                sModeString = sModeString & "16"
            End If
            
            cboFullScreenMode.AddItem sModeString
        Next
        If cboFullScreenMode.ListCount > 0 Then cboFullScreenMode.ListIndex = .lCurrentMode
    End With
                
End Sub

Private Sub optRenderingMode_Click(Index As Integer)
    
    If Index = 1 Then
        cboFullScreenMode.Enabled = True
    Else
        cboFullScreenMode.Enabled = False
    End If
    
End Sub
