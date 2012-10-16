VERSION 5.00
Begin VB.Form frmSelectPin 
   Caption         =   "Connect to Pin"
   ClientHeight    =   3210
   ClientLeft      =   4890
   ClientTop       =   4920
   ClientWidth     =   6270
   LinkTopic       =   "Form2"
   LockControls    =   -1  'True
   PaletteMode     =   1  'UseZOrder
   ScaleHeight     =   3210
   ScaleWidth      =   6270
   StartUpPosition =   2  'CenterScreen
   Begin VB.ListBox listPins 
      Height          =   1425
      ItemData        =   "Selpin.frx":0000
      Left            =   3360
      List            =   "Selpin.frx":0002
      TabIndex        =   3
      Top             =   360
      Width           =   2655
   End
   Begin VB.ListBox listFilters 
      Height          =   1425
      ItemData        =   "Selpin.frx":0004
      Left            =   240
      List            =   "Selpin.frx":0006
      TabIndex        =   2
      Top             =   360
      Width           =   2655
   End
   Begin VB.CommandButton Cancel 
      Cancel          =   -1  'True
      Caption         =   "Cancel"
      Height          =   375
      Left            =   3360
      TabIndex        =   1
      Top             =   2640
      Width           =   975
   End
   Begin VB.CommandButton OK 
      Caption         =   "OK"
      Default         =   -1  'True
      Height          =   375
      Left            =   1920
      TabIndex        =   0
      Top             =   2640
      Width           =   975
   End
   Begin VB.Label Label2 
      Caption         =   "Pins"
      Height          =   252
      Left            =   3240
      TabIndex        =   7
      Top             =   120
      Width           =   492
   End
   Begin VB.Label VendorInfoLabel 
      Caption         =   "Vendor Info:"
      Height          =   252
      Left            =   120
      TabIndex        =   6
      Top             =   2160
      Width           =   972
   End
   Begin VB.Label VendorInfo 
      Height          =   252
      Left            =   1320
      TabIndex        =   5
      Top             =   2160
      Visible         =   0   'False
      Width           =   2772
   End
   Begin VB.Label Label1 
      Caption         =   "Filters"
      Height          =   255
      Left            =   120
      TabIndex        =   4
      Top             =   120
      Width           =   495
   End
End
Attribute VB_Name = "frmSelectPin"
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

Public g_objFI As IFilterInfo
Public g_objPI As IPinInfo
Public g_objMC As IMediaControl

Public OtherDir As Long
Public bOK As Boolean


' **************************************************************************************************************************************
' * PRIVATE INTERFACE- INTRINSIC VBFORM EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: Form_Load
            ' * procedure description:   fills the filters listbox with all filters in the current filter graph
            ' *
            ' ******************************************************************************************************************************
            Private Sub Form_Load()
            On Local Error GoTo ErrLine
            Call RefreshFilters
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            


' **************************************************************************************************************************************
' * PRIVATE INTERFACE- CONTROL EVENT HANDLERS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: Cancel_Click
            ' * procedure description:    cancel command button click event-  no pin connection is made
            ' *
            ' ******************************************************************************************************************************
            Private Sub Cancel_Click()
            On Local Error GoTo ErrLine
            bOK = False: Call Hide
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: listFilters_Click
            ' * procedure description:    when the user clicks on a specific filter in the filter graph, this creates the
            ' *                                         list of pins for that filter in the pin listbox
            ' ******************************************************************************************************************************
            Private Sub listFilters_Click()
            Dim pin As IPinInfo
            Dim pfilter As IFilterInfo
            Dim pinOther As IPinInfo
            On Local Error GoTo ErrLine
            
            'enumerate through each filter in the filter collection
            For Each pfilter In g_objMC.FilterCollection
                If LCase(pfilter.Name) = LCase(listFilters.Text) Then
                    ' display the information and pins for the selected filter
                    Set g_objFI = pfilter  ' global FilterInfo object
                    VendorInfo.Caption = pfilter.VendorInfo
                    listPins.Clear
                    'enumerate through each pin in the filter
                    For Each pin In pfilter.Pins
                        Set pinOther = pin.ConnectedTo
                        If Err.Number <> 0 Then
                            If pin.Direction <> OtherDir Then
                                'append the pin's name to the listbox
                                listPins.AddItem pin.Name
                            End If
                        End If
                    Next
                End If
            Next
            
            'reset the selected index
            If listPins.ListCount > 0 Then
               listPins.ListIndex = 0
            End If
            'clean-up & dereference local data
            If Not pin Is Nothing Then Set pin = Nothing
            If Not pfilter Is Nothing Then Set pfilter = Nothing
            If Not pinOther Is Nothing Then Set pinOther = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Resume Next
            End Sub
            
            
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: RefreshFilters
            ' * procedure description:    fills the filters listbox with all filters in the current filter graph
            ' *
            ' ******************************************************************************************************************************
            Public Sub RefreshFilters()
            Dim pin As IPinInfo
            Dim filter As IFilterInfo
            Dim pinOther As IPinInfo
            On Local Error GoTo ErrLine
            
            'clear the filter listbox
            listFilters.Clear
               
            For Each filter In g_objMC.FilterCollection
                For Each pin In filter.Pins
                    On Error Resume Next
                    Set pinOther = pin.ConnectedTo
                    If Err.Number <> 0 Then
                        If pin.Direction <> OtherDir Then
                            listFilters.AddItem filter.Name
                            Exit For
                        End If
                    End If
                Next
            Next
            
            'reset the list index
            If listFilters.ListCount > 0 Then
               listFilters.ListIndex = 0
            End If
            
            'clean-up & dereference local data
            If Not pin Is Nothing Then Set pin = Nothing
            If Not filter Is Nothing Then Set filter = Nothing
            If Not pinOther Is Nothing Then Set pinOther = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Resume Next
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: OK_Click
            ' * procedure description:    connect the selected pins, if possible. if no connection is possible, the pin
            ' *                                         selection box closes and the program continues normally.
            ' ******************************************************************************************************************************
            Private Sub OK_Click()
            Dim objPinInfo As IPinInfo
            On Local Error GoTo ErrLine
            
            For Each objPinInfo In g_objFI.Pins
                If objPinInfo.Name = listPins.Text Then
                    Set g_objPI = objPinInfo
                    bOK = True
                    Exit For
                End If
            Next
            
            'unload form
            Unload Me
            
            'clean-up & dereference local data
            If Not objPinInfo Is Nothing Then Set objPinInfo = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: listPins_Click
            ' * procedure description:    When a new pin is selected, store it in the module-level pin object
            ' *
            ' ******************************************************************************************************************************
            Private Sub listPins_Click()
            Dim objPinInfo As IPinInfo
            On Local Error GoTo ErrLine
             
            'enumerate the pins
            For Each objPinInfo In g_objFI.Pins
              If LCase(objPinInfo.Name) = LCase(listPins.Text) Then
                 Set g_objPI = objPinInfo 'reset the selected module-level pin
              End If
            Next
            
            'clean-up & dereference local data
            If Not objPinInfo Is Nothing Then Set objPinInfo = Nothing
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
