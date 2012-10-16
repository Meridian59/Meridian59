VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form frmMain 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Filter and Pin Viewer"
   ClientHeight    =   6060
   ClientLeft      =   60
   ClientTop       =   615
   ClientWidth     =   8025
   Icon            =   "builder.frx":0000
   LinkTopic       =   "frmMain"
   LockControls    =   -1  'True
   MaxButton       =   0   'False
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   6060
   ScaleWidth      =   8025
   Begin VB.Frame frameFilter 
      Caption         =   "Filter Graph"
      Height          =   2652
      Left            =   75
      TabIndex        =   18
      Top             =   75
      Width           =   7875
      Begin VB.CommandButton cmdStop 
         Caption         =   "Stop"
         Height          =   315
         Left            =   2175
         TabIndex        =   3
         Top             =   2175
         Width           =   840
      End
      Begin VB.CommandButton cmdPause 
         Caption         =   "Pause"
         Height          =   315
         Left            =   1200
         TabIndex        =   2
         Top             =   2175
         Width           =   840
      End
      Begin VB.CommandButton cmdRun 
         Caption         =   "Run"
         Height          =   315
         Left            =   225
         TabIndex        =   1
         Top             =   2175
         Width           =   840
      End
      Begin VB.CommandButton cmdSource 
         Caption         =   "Add &Source Filter..."
         Height          =   315
         Left            =   3960
         TabIndex        =   6
         Top             =   2175
         Width           =   1572
      End
      Begin VB.ListBox listFilters 
         Height          =   1620
         ItemData        =   "builder.frx":0442
         Left            =   3960
         List            =   "builder.frx":0444
         Sorted          =   -1  'True
         TabIndex        =   5
         Top             =   480
         Width           =   3675
      End
      Begin VB.CommandButton cmdAddRegFilter 
         Caption         =   "&Add ->"
         Height          =   315
         Left            =   3120
         TabIndex        =   4
         Top             =   480
         Width           =   732
      End
      Begin VB.ListBox listRegFilters 
         Height          =   1620
         ItemData        =   "builder.frx":0446
         Left            =   240
         List            =   "builder.frx":0448
         Sorted          =   -1  'True
         TabIndex        =   0
         Top             =   480
         Width           =   2775
      End
      Begin VB.Label lblFilters 
         Caption         =   "Filters in current filter &graph"
         Height          =   252
         Left            =   3960
         TabIndex        =   13
         Top             =   240
         Width           =   2052
      End
      Begin VB.Label lblRegFilters 
         Caption         =   "&Registered filters"
         Height          =   252
         Left            =   240
         TabIndex        =   12
         Top             =   240
         Width           =   2052
      End
   End
   Begin VB.Frame framePinInfo 
      Caption         =   "Filter"
      Height          =   3132
      Left            =   75
      TabIndex        =   11
      Top             =   2850
      Width           =   7875
      Begin VB.TextBox txtPinInfo 
         Height          =   1620
         Left            =   3975
         Locked          =   -1  'True
         MultiLine       =   -1  'True
         ScrollBars      =   3  'Both
         TabIndex        =   10
         Top             =   1200
         Width           =   3675
      End
      Begin VB.CommandButton cmdConnect 
         Caption         =   "Co&nnect One Pin..."
         Height          =   315
         Left            =   2025
         TabIndex        =   9
         Top             =   2500
         Width           =   1692
      End
      Begin VB.CommandButton cmdRender 
         Caption         =   "&Connect Downstream"
         Height          =   315
         Left            =   225
         TabIndex        =   8
         Top             =   2500
         Width           =   1692
      End
      Begin VB.ListBox listPins 
         Height          =   1230
         ItemData        =   "builder.frx":044A
         Left            =   225
         List            =   "builder.frx":044C
         Sorted          =   -1  'True
         TabIndex        =   7
         Top             =   1200
         Width           =   3480
      End
      Begin VB.Label lblFilterName 
         Height          =   255
         Left            =   1425
         TabIndex        =   20
         Top             =   225
         Width           =   4815
      End
      Begin VB.Label lblFilter 
         Caption         =   "Filter name:"
         Height          =   255
         Left            =   240
         TabIndex        =   19
         Top             =   240
         Width           =   975
      End
      Begin VB.Label lblVendor 
         Caption         =   "Vendor: "
         Height          =   255
         Left            =   240
         TabIndex        =   17
         Top             =   480
         Width           =   735
      End
      Begin VB.Label lblVendorInfo 
         Height          =   255
         Left            =   1440
         TabIndex        =   16
         Top             =   480
         Width           =   4935
      End
      Begin VB.Label lblPinListbox 
         Caption         =   "&Pins in selected filter"
         Height          =   255
         Left            =   240
         TabIndex        =   14
         Top             =   940
         Width           =   2055
      End
      Begin VB.Label lblPinInfo 
         Caption         =   "Information for selected pin"
         Height          =   255
         Left            =   3975
         TabIndex        =   15
         Top             =   940
         Width           =   2175
      End
   End
   Begin MSComDlg.CommonDialog ctrlCommonDlg 
      Left            =   8760
      Top             =   -120
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
      CancelError     =   -1  'True
      Flags           =   4096
   End
   Begin VB.Menu mnuFilterGraph 
      Caption         =   "&FilterGraph"
      Begin VB.Menu mnu_FilterGraphNew 
         Caption         =   "&New (empty)"
      End
      Begin VB.Menu mnu_FilterGraphGenerate 
         Caption         =   "&Generate from input file..."
      End
      Begin VB.Menu mnu_Separator1 
         Caption         =   "-"
      End
      Begin VB.Menu mnu_FilterGraphRun 
         Caption         =   "&Run"
      End
      Begin VB.Menu mnu_FilterGraphPause 
         Caption         =   "&Pause"
      End
      Begin VB.Menu mnu_FilterGraphStop 
         Caption         =   "&Stop"
      End
      Begin VB.Menu mnu_Separator2 
         Caption         =   "-"
      End
      Begin VB.Menu mnu_FilterGraphExit 
         Caption         =   "E&xit"
      End
   End
   Begin VB.Menu mnu_Options 
      Caption         =   "&Options"
      Begin VB.Menu mnu_BuildCustomGraph 
         Caption         =   "&Build custom graph"
      End
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
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

Private Enum GraphState
StateStopped = 0
StatePaused = 1
StateRunning = 2
End Enum

Private m_objFilterInfo As Object        'IFilterInfo interface represents all filters in the current graph
Private m_objRegFilterInfo As Object ' IFilterInfo interface represents all registered filters on the system
Private m_objSelectedPin As Object  ' IPinInfo interface represents the pin the user opted for from listbox
Private m_objLocalFilterInfo As Object ' IFilterInfo interface represents the local working filter
Private m_objMediaControl As IMediaControl  'IMediaControl interface provided by IFilgraphManager
Private m_GraphState As GraphState  'enum value indicating whether the video is running, paused, or stopped




' **************************************************************************************************************************************
' * PRIVATE INTERFACE- PROCEDURES
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: AppendFilter
            ' * procedure description:    appends a filter to the current working graph
            ' *
            ' ******************************************************************************************************************************
            Private Sub AppendFilter(bstrFilterName As String, objFilter As IFilterInfo)
            Dim nCount As Long
            Dim LocalRegFilters As Object
            Dim objRegFilter As IRegFilterInfo
            On Local Error GoTo ErrLine
            
            'obtain reference to an IAMCollection containing the registered filters
            If Not m_objMediaControl Is Nothing Then
               If Not m_objMediaControl.RegFilterCollection Is Nothing Then
                  Set LocalRegFilters = m_objMediaControl.RegFilterCollection
               End If
            End If
            
            'step through each registered filter for verification
            'and proceed to append the filter if the filter is in fact valid..
            For nCount = 0 To (LocalRegFilters.Count - 1) Step 1
                LocalRegFilters.Item nCount, objRegFilter
                If LCase(objRegFilter.Name) = LCase(bstrFilterName) Then
                    objRegFilter.filter objFilter
                    Exit For
                End If
            Next
            
             'refresh the filter list
            Call RefreshFilters
            
            'clean-up & dereference local data
            If Not objRegFilter Is Nothing Then Set objRegFilter = Nothing
            If Not LocalRegFilters Is Nothing Then Set LocalRegFilters = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: RefreshRegFilters
            ' * procedure description:   update the listbox of registered filters (listbox listRegFilters)
            ' *
            ' ******************************************************************************************************************************
            Private Sub RefreshRegFilters()
            Dim objRegFilter As IRegFilterInfo
            On Local Error GoTo ErrLine
                
            'clear the collection
            listRegFilters.Clear
            
            'verify that we have a valid pointer
            If Not m_objRegFilterInfo Is Nothing Then
                'enumerate through each filter in the registered filters collection..
                For Each objRegFilter In m_objRegFilterInfo
                    'append the filtername to the listbox
                    listRegFilters.AddItem objRegFilter.Name
                Next
            End If
            
            'reset selected item in the listbox..
            If listRegFilters.ListCount > 0 Then
               listRegFilters.ListIndex = 0  ' select first in list
            End If
            
            'clean-up & dereference local data
            If Not objRegFilter Is Nothing Then Set objRegFilter = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            
            
            
' **************************************************************************************************************************************
' * PRIVATE INTERFACE- INTRINSIC VBFORM EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: Form_Initalize
            ' * procedure description:   fired intrinsically by visual basic when the class initalizes
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_Initalize()
            On Local Error GoTo ErrLine
            ' instantiate the FilgraphManager
            Set m_objMediaControl = New FilgraphManager
            'obtain an IAMCollection reference for the registered filters
            If Not m_objMediaControl Is Nothing Then
               If Not m_objMediaControl.RegFilterCollection Is Nothing Then
                  Set m_objRegFilterInfo = m_objMediaControl.RegFilterCollection
               End If
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            

            ' ******************************************************************************************************************************
            ' * procedure name: Form_Terminate
            ' * procedure description:   fired intrinsically by visual basic when the form terminates
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_Terminate()
            On Local Error GoTo ErrLine
            
            'clean-up & dereference
            If Not m_objFilterInfo Is Nothing Then Set m_objFilterInfo = Nothing
            If Not m_objSelectedPin Is Nothing Then Set m_objSelectedPin = Nothing
            If Not m_objMediaControl Is Nothing Then Set m_objMediaControl = Nothing
            If Not m_objRegFilterInfo Is Nothing Then Set m_objRegFilterInfo = Nothing
            If Not m_objLocalFilterInfo Is Nothing Then Set m_objLocalFilterInfo = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Form_Load
            ' * procedure description:   fired intrinsically by visual basic when the form loads
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_Load()
            On Local Error GoTo ErrLine
            ' initialize the display
            listFilters.Clear: listPins.Clear: txtPinInfo.Text = vbNullString
            ' update the display for registered filters
            Call RefreshRegFilters
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            



' **************************************************************************************************************************************
' * PRIVATE INTERFACE- INTRINSIC CONTROL EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: cmdAddRegFilter_Click
            ' * procedure description:  occures when the user manually appends a filter to the graph
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdAddRegFilter_Click()
            Dim objFilter As IFilterInfo
            Dim objRegFilter As IRegFilterInfo
            On Local Error GoTo ErrLine
            
            If Not m_objRegFilterInfo Is Nothing Then
               'enumerate all registered filters
               For Each objRegFilter In m_objRegFilterInfo ' listRegFilters
                   If LCase(objRegFilter.Name) = LCase(listRegFilters.Text) Then
                       objRegFilter.filter objFilter
                       If objFilter.IsFileSource Then
                          'display the common dialog
                          ctrlCommonDlg.CancelError = False
                          ctrlCommonDlg.ShowOpen
                          'assign the filter a filename
                          If ctrlCommonDlg.FileName <> vbNullString Then
                             objFilter.FileName = ctrlCommonDlg.FileName
                          Else: Exit Sub
                          End If
                       End If
                       Exit For
                   End If
               Next
            End If
            
            'refresh the filter list
            Call RefreshFilters
            
            'clean-up & dereference local data
            If Not objFilter Is Nothing Then Set objFilter = Nothing
            If Not objRegFilter Is Nothing Then Set objRegFilter = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdConnect_Click
            ' * procedure description:  occures when the user manually makes a connection
            ' *                                       connects the current selected pin, indicated by m_objSelectedPin , m_objMediaControl, and m_objFilterInfo
            ' ******************************************************************************************************************************
            Private Sub cmdConnect_Click()
            Dim objPI As IPinInfo
            On Local Error GoTo ErrLine ' if already connected, connect fails
            
            'set direction
            frmSelectPin.OtherDir = m_objSelectedPin.Direction
            Set frmSelectPin.g_objMC = m_objMediaControl
            Set frmSelectPin.g_objFI = m_objFilterInfo
            'refresh the filters & display the dialog
            frmSelectPin.RefreshFilters: frmSelectPin.Show 1
            
            If frmSelectPin.bOK Then
                If Not frmSelectPin.g_objPI Is Nothing Then _
                   Set objPI = frmSelectPin.g_objPI
                m_objSelectedPin.Connect objPI
                RefreshFilters  ' refresh the filters in the ui
            End If
            
            'clean-up & dereference local data
            If Not objPI Is Nothing Then Set objPI = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdRender_Click
            ' * procedure description:   occures when the user opts to connect the current graph, causing downstream pins to render
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdRender_Click()
            On Local Error GoTo ErrLine
            'render the selected pin
            m_objSelectedPin.Render
            'update the ui
            RefreshFilters
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdRender_Click
            ' * procedure description:   Appends a source filter to the graph that can read the given filename.
            ' *                                        Calls IMediaControl::AddSourceFilter
            ' ******************************************************************************************************************************
            Private Sub cmdSource_Click()
            Dim nCount As Long
            Dim objFilter As Object
            On Local Error GoTo ErrLine
            
            'display the common dialog
            ctrlCommonDlg.CancelError = False
            ctrlCommonDlg.filter = "Media files (*.mpg;*.avi;*.mov)|*.mpg;*.avi;*.mov|"
            ctrlCommonDlg.ShowOpen  ' open the source filter
            'append the source filter given the file
            If ctrlCommonDlg.FileName <> vbNullString Then
               'verify that the filter has not already been appended to the list
               For nCount = 0 To Me.listFilters.ListCount
                     If LCase(listFilters.List(nCount)) = LCase(ctrlCommonDlg.FileName) Then
                        'the item has already been appended to the list, so exit
                        Exit Sub
                     End If
               Next
               Call m_objMediaControl.AddSourceFilter(ctrlCommonDlg.FileName, objFilter)
               RefreshFilters  ' update all info displayed by this VB app
            End If
            Exit Sub
            
ErrLine:
            If Err.Number = 32755 Then Exit Sub
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: RefreshFilters
            ' * procedure description:   Refresh the contents of the "Filters" combo box using the current IMediaControl interface
            ' *                                        Calls IMediaControl::AddSourceFilter
            ' ******************************************************************************************************************************
            Public Sub RefreshFilters()
            Dim objFilter As IFilterInfo
            On Local Error GoTo ErrLine
            
            'clear the listed filters
            listFilters.Clear
            'enumerate the current filters in the collection
            For Each objFilter In m_objMediaControl.FilterCollection
                listFilters.AddItem objFilter.Name 'append the filter's name
            Next
            'select the filter
            If listFilters.ListCount > 0 Then
                listFilters.ListIndex = 0  ' select first in list
            End If
            
            'clean-up & dereference local data
            If Not objFilter Is Nothing Then Set objFilter = Nothing
            Exit Sub
            
ErrLine:

            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: listFilters_Click
            ' * procedure description:   user clicked on a filter in the filters combo box or index; proceed to update pins..
            ' *                                        when filling the listfilters listbox
            ' ******************************************************************************************************************************
            Private Sub listFilters_Click()
            Dim objPinInfo As IPinInfo
            Dim objFilterInfo As IFilterInfo
            On Local Error GoTo ErrLine
            
            'update the pins listbox to show all of its pins
            For Each objFilterInfo In m_objMediaControl.FilterCollection
                If objFilterInfo.Name = listFilters.Text Then
                    Set m_objFilterInfo = objFilterInfo
                    lblFilterName.Caption = objFilterInfo.Name
                    lblVendorInfo.Caption = objFilterInfo.VendorInfo
                    'clear the listbox
                    listPins.Clear
                    'append all of this filter's pins to the list box
                    For Each objPinInfo In objFilterInfo.Pins
                        listPins.AddItem objPinInfo.Name
                    Next
                End If
            Next
            
            ' select the first in the list
            If listPins.ListCount > 0 Then
               listPins.ListIndex = 0
            End If
            
            'clean-up & dereference local data
            If Not objPinInfo Is Nothing Then Set objPinInfo = Nothing
            If Not objFilterInfo Is Nothing Then Set objFilterInfo = Nothing
            Exit Sub
            
ErrLine:

            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: listPins_Click
            ' * procedure description:   Proceed to Append detailed information on the selected pin to the textbox
            ' *                                        This procedure is designed whereas it will encapsulate the error if the pin
            ' *                                        cannot be connectedto, and still set the correct text.  This is by design.
            ' ******************************************************************************************************************************
            Private Sub listPins_Click()
            Dim strSnippet As String
            Dim objPinInfo As IPinInfo
            Dim objPeerFilter As IFilterInfo
            Dim objConnPinInfo As IPinInfo
            Dim objMediaTypeInfo As IMediaTypeInfo
            On Local Error GoTo ErrLine
            
            'enumerate throuch each pin
            For Each objPinInfo In m_objFilterInfo.Pins
                    
                    If LCase(objPinInfo.Name) = LCase(listPins.Text) Then
                       'reset the selected pin
                       Set m_objSelectedPin = objPinInfo
                       
                       'attempt to obtain a reference to the connected pin;
                       'always anticipating that an error could occure..
                       Set objConnPinInfo = objPinInfo.ConnectedTo
                       
                       If Not objConnPinInfo Is Nothing Then
                         'the above method completed without error, list the pin info..
                         strSnippet = "Connected to pin: " + objConnPinInfo.Name + " "
                         If Not objConnPinInfo Is Nothing Then
                            If Not objConnPinInfo.FilterInfo Is Nothing Then
                               Set objPeerFilter = objConnPinInfo.FilterInfo
                               strSnippet = strSnippet + " on filter: " + objPeerFilter.Name + " "
                               Set objMediaTypeInfo = objPinInfo.ConnectionMediaType
                               strSnippet = strSnippet + vbCrLf + "Media Type: " + objMediaTypeInfo.Type
                            End If
                         End If
                       End If
                       
                       'append info based upon pin direction..
                       If objPinInfo.Direction = 0 Then
                         strSnippet = strSnippet + " " + vbCrLf + "Direction: Input"
                       Else:  strSnippet = strSnippet + " " + vbCrLf + "Direction: Output"
                       End If
                     'set the information to the textbox..
                     txtPinInfo.Text = strSnippet
                     End If
            Next
            
            'clean-up & dereference local data
            If Not objPinInfo Is Nothing Then Set objPinInfo = Nothing
            If Not objPeerFilter Is Nothing Then Set objPeerFilter = Nothing
            If Not objConnPinInfo Is Nothing Then Set objConnPinInfo = Nothing
            If Not objMediaTypeInfo Is Nothing Then Set objMediaTypeInfo = Nothing
            Exit Sub
             
ErrLine:
            Err.Clear
            Resume Next
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: listRegFilters_DblClick
            ' * procedure description:  Double click event for registered filters listbox; appends the filter to the current graph
            ' *
            ' ******************************************************************************************************************************
            Private Sub listRegFilters_DblClick()
            On Local Error GoTo ErrLine
            cmdAddRegFilter_Click
            Exit Sub
             
ErrLine:
            Err.Clear
            Resume Next
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdPause_Click
            ' * procedure description:  duplicates the functionality of "Pause" under the FilterGraph menu
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdPause_Click()
            On Local Error GoTo ErrLine
            Call mnu_FilterGraphPause_Click
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdRun_Click
            ' * procedure description:   duplicates the functionality of "Run" under the FilterGraph menu
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdRun_Click()
            On Local Error GoTo ErrLine
            Call mnu_FilterGraphRun_Click
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: cmdStop_Click
            ' * procedure description:   duplicates the functionality of "Stop" under the FilterGraph menu
            ' *
            ' ******************************************************************************************************************************
            Private Sub cmdStop_Click()
            On Local Error GoTo ErrLine
            Call mnu_FilterGraphStop_Click
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            
' **************************************************************************************************************************************
' * PRIVATE INTERFACE- INTRINSIC MENU EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: mnu_FilterGraphExit_Click
            ' * procedure description:
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnu_FilterGraphExit_Click()
            Dim frm As Form
            On Local Error GoTo ErrLine
            
            'exit application
            For Each frm In Forms
               frm.Visible = False: Unload frm
            Next
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnu_FilterGraphGenerate_Click
            ' * procedure description:    User is initializing the filter graph based on a source file
            ' *                                         Create a new filter graph and then get all filters, connections
            ' ******************************************************************************************************************************
            Private Sub mnu_FilterGraphGenerate_Click()
            On Local Error GoTo ErrLine
            
            ' reset the application's module-level objects
            If Not m_objFilterInfo Is Nothing Then Set m_objFilterInfo = Nothing
            If Not m_objSelectedPin Is Nothing Then Set m_objSelectedPin = Nothing
            If Not m_objRegFilterInfo Is Nothing Then Set m_objRegFilterInfo = Nothing
            If Not m_objMediaControl Is Nothing Then Set m_objMediaControl = Nothing
            
            ' reinitialize the display
            listFilters.Clear: listPins.Clear: txtPinInfo.Text = ""
            
            ' create a new IMediaControl object
            Set m_objMediaControl = New FilgraphManager
            
            ' refresh the display for registered filters
            If Not m_objMediaControl Is Nothing Then
               If Not m_objMediaControl.RegFilterCollection Is Nothing Then
                  Set m_objRegFilterInfo = m_objMediaControl.RegFilterCollection
                  Call RefreshRegFilters
               End If
            End If
           
            ' use the common dialog to let the user select the input file
            ctrlCommonDlg.CancelError = False
            ctrlCommonDlg.filter = "Media files (*.mpg;*.avi;*.mov;*.wav)|*.mpg;*.avi;*.mov;*.wav|"
            ctrlCommonDlg.ShowOpen
           
            ' call IMediaControl::RenderFile to add all filters and connect all pins
            If ctrlCommonDlg.FileName <> vbNullString Then
               m_objMediaControl.RenderFile ctrlCommonDlg.FileName
               Call RefreshFilters  'update the ui
            End If
            Exit Sub
           
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnu_FilterGraphNew_Click
            ' * procedure description:    User wants to start with a fresh filter graph so proceed to reset the app.
            ' *                                         Create a new filter graph and then reset all filters, pins, and connections
            ' ******************************************************************************************************************************
            Private Sub mnu_FilterGraphNew_Click()
            On Local Error GoTo ErrLine
            
            ' reset the application's module-level objects
            If Not m_objFilterInfo Is Nothing Then Set m_objFilterInfo = Nothing
            If Not m_objSelectedPin Is Nothing Then Set m_objSelectedPin = Nothing
            If Not m_objRegFilterInfo Is Nothing Then Set m_objRegFilterInfo = Nothing
            If Not m_objMediaControl Is Nothing Then Set m_objMediaControl = Nothing
           
           ' create a new IMediaControl object
           Set m_objMediaControl = New FilgraphManager
           
            ' refresh the display for registered filters
            If Not m_objMediaControl Is Nothing Then
               If Not m_objMediaControl.RegFilterCollection Is Nothing Then
                  Set m_objRegFilterInfo = m_objMediaControl.RegFilterCollection
                  Call RefreshRegFilters
               End If
            End If
            
            ' clear the contents of the listboxes, textboxes, and labels
            listFilters.Clear: listPins.Clear
            txtPinInfo.Text = vbNullString: lblFilterName.Caption = vbNullString: lblVendorInfo.Caption = vbNullString
            
            ' set the current playback state to stopped
            m_GraphState = StateStopped
            Exit Sub
           
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnu_FilterGraphPause_Click
            ' * procedure description:    Pauses rendering using the IMediaControl interface
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnu_FilterGraphPause_Click()
            On Local Error GoTo ErrLine
            If Not m_objMediaControl Is Nothing Then
               If m_GraphState = StateStopped Then Exit Sub
               Call m_objMediaControl.Pause
               m_GraphState = StatePaused
            End If
            Exit Sub
           
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnu_FilterGraphRun_Click
            ' * procedure description:   Sets the playback state dependent on the current rendering state of the graph
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnu_FilterGraphRun_Click()
            Dim objPosition As IMediaPosition
            On Local Error GoTo ErrLine
            
            
            Select Case m_GraphState
            
                   Case GraphState.StateStopped
                            If Not m_objMediaControl Is Nothing Then
                               'obtain the current position
                               Set objPosition = m_objMediaControl
                               'reset the position to zero
                               objPosition.CurrentPosition = 0
                               'set state to running
                               m_objMediaControl.Run
                               'reset module-level variable
                               m_GraphState = StateRunning
                            End If
                   
                   
                   Case GraphState.StatePaused
                            If Not m_objMediaControl Is Nothing Then
                               'set state back to running
                               m_objMediaControl.Run
                               'reset module-level variable
                               m_GraphState = StateRunning
                            End If
                            
                            
                   Case GraphState.StateRunning
                            If Not m_objMediaControl Is Nothing Then
                               'obtain the current position
                               Set objPosition = m_objMediaControl
                               
                               If CLng(objPosition.CurrentPosition) = CLng(objPosition.Duration) Then
                                  'end of media reached, restart
                                  objPosition.CurrentPosition = 0
                                  'set state to running
                                  m_objMediaControl.Run
                                  'reset module-level variable
                                  m_GraphState = StateRunning
                                  
                               ElseIf CLng(objPosition.CurrentPosition) <> CLng(objPosition.Duration) Then
                                  'playback in progress, restart
                                  objPosition.CurrentPosition = 0
                                  'set state to running
                                  m_objMediaControl.Run
                                  'reset module-level variable
                                  m_GraphState = StateRunning
                                  
                               ElseIf CLng(objPosition.CurrentPosition) = 0 Then
                                  'playback ready, start
                                  m_objMediaControl.Run
                                  'reset module-level variable
                                  m_GraphState = StateRunning
                               End If
                            End If
                            
            End Select
            
            'clean-up & dereference local data
            If Not objPosition Is Nothing Then Set objPosition = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnu_FilterGraphStop_Click
            ' * procedure description:    Stops rendering using the IMediaControl interface
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnu_FilterGraphStop_Click()
            On Local Error GoTo ErrLine
            If Not m_objMediaControl Is Nothing Then
               Call m_objMediaControl.Stop
               m_GraphState = StateStopped
            End If
            Exit Sub
           
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: mnu_BuildCustomGraph_Click
            ' * procedure description:  allows the user to build a custom graph
            ' *
            ' *                                       This routine demonstrates a likely common use
            ' *                                       of these methods in Visual Basic applications:
            ' *                                       Directly creating the filter graph needed
            ' *                                       for a specific multimedia file.
            ' *                                       The graph has the following filters: AVI Source, AVI Decompressor,
            ' *                                       Video Renderer, AVI Splitter, and Audio Renderer.
            ' *                                       Note that these filters can be connected by reusing just
            ' *                                       two pin object variables, but for clarity of the example,
            ' *                                       all are defined using names that reflect their position
            ' *                                       in the filter graph.
            ' *                                       The filters are declared with their pins, as follows:
            ' *
            ' ******************************************************************************************************************************
            Private Sub mnu_BuildCustomGraph_Click()
            Dim pPin As IPinInfo
            Dim pSourceFilter As IFilterInfo  ' AVI source filter; has two pins
            Dim SourceOutputPin As IPinInfo  'Source Filter output pin
            Dim pAVISplitter As IFilterInfo ' AVI splitter
            Dim SplitterInPin As IPinInfo   ' AVI splitter pin "Input"
            Dim SplitterOut00Pin As IPinInfo  ' AVI splitter pin "Stream 00"
            Dim SplitterOut01Pin As IPinInfo  ' AVI splitter pin "Stream 01"
            Dim pDECFilter As IFilterInfo  ' AVI Decompressor; has two pins
            Dim DECInPin As IPinInfo   'AVI Decompressor pin "XForm In"
            Dim DECOutPin As IPinInfo   ' AVI Decompressor pin "XForm Out"
            Dim pVidRenderer As IFilterInfo ' Video renderer, has one pin
            Dim VidRendInPin As IPinInfo  ' Video Renderer pin "Input"
            Dim pAudioRenderer As IFilterInfo 'Audio renderer, has one pin
            Dim AudioRendInPin As IPinInfo ' Audio Renderer pin "Input"
            On Local Error GoTo ErrLine

            
            ' reinitialize all global variables
            If Not m_objRegFilterInfo Is Nothing Then Set m_objRegFilterInfo = Nothing
            If Not m_objFilterInfo Is Nothing Then Set m_objFilterInfo = Nothing
            If Not m_objSelectedPin Is Nothing Then Set m_objSelectedPin = Nothing
            If Not m_objMediaControl Is Nothing Then Set m_objMediaControl = Nothing
            
            ' create a new IMediaControl object
            Set m_objMediaControl = New FilgraphManager
            ' reset the listRegFilters again
            Set m_objRegFilterInfo = m_objMediaControl.RegFilterCollection
            
            'refresh reg filter list
            RefreshRegFilters
            
            ' reinitialize the display
            listFilters.Clear: listPins.Clear
            lblFilterName.Caption = vbNullString: lblVendorInfo.Caption = vbNullString
            txtPinInfo.Text = vbNullString: m_GraphState = StateStopped 'update state
                
            'Add source filter for an AVI file
            ctrlCommonDlg.CancelError = False
            ctrlCommonDlg.filter = "AVI files (*.avi)|*.avi"
            ctrlCommonDlg.ShowOpen  ' get the name of the source or filter graph file
            
            'verify the user's input and proceed..
            If ctrlCommonDlg.FileName <> vbNullString Then
                m_objMediaControl.AddSourceFilter ctrlCommonDlg.FileName, pSourceFilter
            Else:  Exit Sub
            End If
            
            ' Get the pins we need to connect
            For Each pPin In pSourceFilter.Pins
              Debug.Print pPin.Name
              If pPin.Name = "Output" Then
                  Set SourceOutputPin = pPin
              End If
            Next
            
            'Add DEC filter
            If Not pDECFilter Is Nothing Then
               AppendFilter "AVI Decompressor", pDECFilter
            End If
            
            'Print out list of pins on decompressor filter
            If Not pDECFilter Is Nothing Then
               If Not pDECFilter.Pins Is Nothing Then
                  For Each pPin In pDECFilter.Pins
                    Debug.Print pPin.Name
                    ' save specific pins to connect them
                    If pPin.Name = "XForm In" Then
                        Set DECInPin = pPin
                    End If
                    If pPin.Name = "XForm Out" Then
                        Set DECOutPin = pPin
                    End If
                  Next
              End If
            End If
            
            'Add AVI Splitter
            If Not pAVISplitter Is Nothing Then
               AppendFilter "AVI Splitter", pAVISplitter
            End If
            
            'Print out list of pins on decompressor filter
            If Not pAVISplitter Is Nothing Then
               If Not pAVISplitter.Pins Is Nothing Then
                  For Each pPin In pAVISplitter.Pins
                    Debug.Print pPin.Name
                    ' save specific pins to connect them
                    ' pin 0, pin 1
                    If pPin.Name = "input pin" Then
                        Set SplitterInPin = pPin
                    ElseIf pPin.Name = "Stream 00" Then
                        Set SplitterOut00Pin = pPin
                    ElseIf pPin.Name = "Stream 01" Then
                        Set SplitterOut01Pin = pPin
                    End If
                  Next
              End If
            End If
            
            'Connect Source video output pin to AVI splitter input pin
            If Not SourceOutputPin Is Nothing And Not SplitterInPin Is Nothing Then
                SourceOutputPin.Connect SplitterInPin
            End If
         
            ' Splitter now knows how many output pins it needs
            If Not pAVISplitter Is Nothing Then
               If Not pAVISplitter.Pins Is Nothing Then
                  For Each pPin In pAVISplitter.Pins
                    Debug.Print pPin.Name
                    ' save specific pins to connect them
                    ' pin 0, pin 1
                    If pPin.Name = "Stream 00" Then
                        Set SplitterOut00Pin = pPin
                    ElseIf pPin.Name = "Stream 01" Then
                        Set SplitterOut01Pin = pPin
                    End If
                  Next
               End If
            End If
            
            'Add Video Renderer filter and set its pin variables
            AppendFilter "Video Renderer", pVidRenderer
            'Print out list of pins on video renderer filter
            For Each pPin In pVidRenderer.Pins
              Debug.Print pPin.Name
              If pPin.Name = "Input" Then
                  Set VidRendInPin = pPin
              End If
            Next
            
            'Add Audio Renderer filter and set its pin variables
            AppendFilter "Audio Renderer", pAudioRenderer
            'Print out list of pins on audioo renderer filter
            For Each pPin In pAudioRenderer.Pins
              Debug.Print pPin.Name
              If InStr(pPin.Name, "Input") Then
                  Set AudioRendInPin = pPin
              End If
            Next
            
            
            ' Connect AVI splitter stream 01 to AVI decompressor
            If Not DECInPin Is Nothing And SplitterOut00Pin Is Nothing Then
                SplitterOut00Pin.Connect DECInPin
            End If
            
            'Connect DEC filter output pin to Video Renderer input pin
            If Not DECOutPin Is Nothing And Not VidRendInPin Is Nothing Then
                DECOutPin.Connect VidRendInPin
            End If
            
            ' Connect AVI splitter stream 01 to audio renderer
            ' continue if there is no audio connection for the source AVI file
            If Not AudioRendInPin Is Nothing And SplitterOut01Pin Is Nothing Then
                SplitterOut01Pin.Connect AudioRendInPin
            End If
            
            'refresh
            RefreshFilters
            Exit Sub
            
            
ErrLine:
            If Err.Number = 32755 Then Exit Sub
            MsgBox "Could not create the custom filter graph. Please select an .AVI file that uses the AVI splitter and AVI decompressor filters."
            Exit Sub
            End Sub
