VERSION 5.00
Object = "{B0EDF154-910A-11D2-B632-00C04F79498E}#1.0#0"; "msvidctl.dll"
Begin VB.Form frmMain 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Microsoft Video Control -  VB Sample Application"
   ClientHeight    =   4875
   ClientLeft      =   2130
   ClientTop       =   2730
   ClientWidth     =   9135
   Icon            =   "main.frx":0000
   LinkTopic       =   "Form1"
   LockControls    =   -1  'True
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   325
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   609
   ShowInTaskbar   =   0   'False
   Begin VB.TextBox txtChannel 
      Height          =   375
      Left            =   1200
      TabIndex        =   25
      Top             =   2160
      Visible         =   0   'False
      Width           =   615
   End
   Begin VB.TextBox txtSID 
      Height          =   375
      Left            =   1200
      TabIndex        =   23
      Top             =   1680
      Visible         =   0   'False
      Width           =   615
   End
   Begin VB.CommandButton cmdInfo 
      Caption         =   "Info"
      Height          =   495
      Left            =   1200
      TabIndex        =   22
      ToolTipText     =   "Display tuner information and FPS count"
      Top             =   1080
      Visible         =   0   'False
      Width           =   855
   End
   Begin VB.CommandButton cmdEnterDVB 
      Caption         =   "Enter"
      Height          =   495
      Left            =   120
      TabIndex        =   20
      ToolTipText     =   "Enter DVB Channel"
      Top             =   1800
      Visible         =   0   'False
      Width           =   975
   End
   Begin VB.CommandButton cmdViewNext 
      Caption         =   "Next Tuner"
      Height          =   495
      Left            =   120
      TabIndex        =   19
      ToolTipText     =   "Select the next tuner in the list"
      Top             =   1080
      Visible         =   0   'False
      Width           =   975
   End
   Begin VB.CommandButton cmdCaptureOff 
      Caption         =   "Hide Capture"
      Height          =   495
      Left            =   7305
      TabIndex        =   18
      ToolTipText     =   "Click to hide the captured frame window"
      Top             =   3960
      Visible         =   0   'False
      Width           =   1695
   End
   Begin VB.CommandButton cmdSeekUpDigital 
      Caption         =   "Ch. Up"
      Height          =   495
      Left            =   1200
      TabIndex        =   17
      ToolTipText     =   "ATSC Physical Channel Up"
      Top             =   3600
      Visible         =   0   'False
      Width           =   975
   End
   Begin VB.CommandButton cmdVolumeDown 
      Caption         =   "Vol. Down"
      Height          =   495
      Left            =   2280
      TabIndex        =   16
      ToolTipText     =   "Click to decrease volume"
      Top             =   4200
      Visible         =   0   'False
      Width           =   975
   End
   Begin VB.CommandButton cmdSeekDownDigital 
      Caption         =   "Ch. Down"
      Height          =   495
      Left            =   1200
      TabIndex        =   15
      ToolTipText     =   "ATSC Physical Channel Down"
      Top             =   4200
      Visible         =   0   'False
      Width           =   975
   End
   Begin VB.CommandButton cmdCapture 
      Caption         =   "Capture Video Frame"
      Height          =   495
      Left            =   4200
      TabIndex        =   14
      ToolTipText     =   "Click to capture a frame of video"
      Top             =   3960
      Visible         =   0   'False
      Width           =   1815
   End
   Begin VB.CommandButton cmdEnterAnalog 
      Caption         =   "Enter"
      Height          =   495
      Left            =   120
      TabIndex        =   13
      ToolTipText     =   "Enter NTSC Channel"
      Top             =   2400
      Visible         =   0   'False
      Width           =   975
   End
   Begin VB.CommandButton cmdEnterATSC 
      Caption         =   "Enter"
      Height          =   495
      Left            =   120
      TabIndex        =   12
      ToolTipText     =   "Enter ATSC Channel"
      Top             =   3000
      Visible         =   0   'False
      Width           =   975
   End
   Begin VB.CommandButton cmdVolumeUp 
      Caption         =   "Vol. Up"
      Height          =   495
      Left            =   2280
      TabIndex        =   11
      ToolTipText     =   "Click to increase volume"
      Top             =   3600
      Visible         =   0   'False
      Width           =   975
   End
   Begin VB.TextBox txtMinorChannel 
      Height          =   375
      Left            =   1200
      TabIndex        =   8
      Top             =   3120
      Visible         =   0   'False
      Width           =   615
   End
   Begin VB.TextBox txtPhysicalChannel 
      Height          =   375
      Left            =   1200
      TabIndex        =   6
      Top             =   2640
      Visible         =   0   'False
      Width           =   615
   End
   Begin VB.CommandButton cmdSeekDownAnalog 
      Caption         =   "Ch. Down"
      Height          =   495
      Left            =   120
      TabIndex        =   5
      ToolTipText     =   "NTSC Channel Down"
      Top             =   4200
      Visible         =   0   'False
      Width           =   975
   End
   Begin VB.CommandButton cmdSeekUpAnalog 
      Caption         =   "Ch. Up"
      Height          =   495
      Left            =   120
      TabIndex        =   4
      ToolTipText     =   "NTSC Channel Up"
      Top             =   3600
      Visible         =   0   'False
      Width           =   975
   End
   Begin VB.CommandButton cmdPowerOff 
      Caption         =   "Power Off"
      Height          =   495
      Left            =   2760
      TabIndex        =   3
      ToolTipText     =   "Done with playback of selected source"
      Top             =   1680
      Visible         =   0   'False
      Width           =   1095
   End
   Begin VB.CommandButton cmdPowerOn 
      Caption         =   "Power On"
      Height          =   495
      Left            =   2760
      TabIndex        =   2
      ToolTipText     =   "Start playing selected source (please be patient)"
      Top             =   1080
      Width           =   1095
   End
   Begin VB.ComboBox cbSource 
      Height          =   315
      Left            =   120
      TabIndex        =   0
      Text            =   "Combo1"
      ToolTipText     =   "Choose one of the tuners from the combo box and press Power On.  Note that building the graph may take up to 10 seconds."
      Top             =   480
      Width           =   3975
   End
   Begin VB.Label lblChannel 
      Caption         =   "Channel"
      Height          =   255
      Left            =   1920
      TabIndex        =   26
      Top             =   2220
      Visible         =   0   'False
      Width           =   735
   End
   Begin VB.Label lblSID 
      Caption         =   "SID"
      Height          =   255
      Left            =   1920
      TabIndex        =   24
      Top             =   1740
      Visible         =   0   'False
      Width           =   495
   End
   Begin VB.Label lblInfo 
      Height          =   375
      Left            =   4200
      TabIndex        =   21
      Top             =   4440
      Visible         =   0   'False
      Width           =   3735
   End
   Begin MSVidCtlLibCtl.MSVidCtl VidControl 
      Height          =   3600
      Left            =   4200
      TabIndex        =   10
      ToolTipText     =   "Microsoft Video Control window"
      Top             =   240
      Width           =   4800
      _cx             =   42672403
      _cy             =   42670286
      AutoSize        =   0   'False
      Enabled         =   -1  'True
      Object.TabStop         =   -1  'True
      BackColor       =   0
   End
   Begin VB.Label lblMinorChannel 
      Caption         =   "Minor Channel"
      Height          =   255
      Left            =   1920
      TabIndex        =   9
      Top             =   3180
      Visible         =   0   'False
      Width           =   1455
   End
   Begin VB.Label lblPhysicalChannel 
      Caption         =   "Physical Channel"
      Height          =   255
      Left            =   1920
      TabIndex        =   7
      Top             =   2700
      Visible         =   0   'False
      Width           =   1335
   End
   Begin VB.Label lblSourceInUse 
      Caption         =   "Playback Source"
      Height          =   255
      Left            =   120
      TabIndex        =   1
      Top             =   120
      Visible         =   0   'False
      Width           =   3975
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'*******************************************************************************
'*       This is a part of the Microsoft Platform SDK Code Samples.
'*       Copyright (C) 1999-2001 Microsoft Corporation.
'*       All rights reserved.
'*       This source code is only intended as a supplement to
'*       Microsoft Development Tools and/or SDK documentation.
'*******************************************************************************

'Microsoft Video Control - Sample Visual Basic Application
Option Explicit
Dim TVPlayer As MSVidAnalogTunerDevice
Dim ATSCTune As IATSCChannelTuneRequest
Dim AnalogTune As IChannelTuneRequest
Dim DVBTune As IDVBTuneRequest
Dim AnalogTV As AnalogTVTuningSpace
Dim ATSCTV As New ATSCTuningSpace
Dim DVBSTV As New DVBSTuningSpace
Dim ATSCLoc As New ATSCLocator


Sub Form_Load()
    cbSource.AddItem ("NTSC Analog TV")
    cbSource.AddItem ("NTSC Analog TV w/CC")
    cbSource.AddItem ("ATSC Digital Antenna TV")
    cbSource.AddItem ("ATSC Digital Antenna TV w/CC & Mixing Mode")
    cbSource.AddItem ("DVB-S Digital TV")
    cbSource.AddItem ("DVB-S Digital TV w/CC & Mixing Mode")
    cbSource.Text = "Choose a playback source and click Power On"
End Sub

Sub cmdPowerOn_click()
'This function builds the correct graph depending on the user-selected broadcast type
    On Error GoTo ON_ERROR
    Dim TuningSpaceContainer As SystemTuningSpaces
    Set TuningSpaceContainer = CreateObject("BDATuner.SystemTuningSpaces")
    Dim TuningSpaceCollection As ITuningSpaces
    Dim TS As ITuningSpace
    Dim FeaturesColl As New MSVidFeatures
    Dim FeaturesAvailableColl As MSVidFeatures
    Dim Feature As IMSVidFeature
    Dim counter As Integer
    
    VidControl.MaintainAspectRatio = True
    VidControl.AutoSize = False
    
    'NTSC Analog TV playback init
    If cbSource.Text = "NTSC Analog TV" Then
        'Find all of the AnalogTV tuning spaces
        Set TuningSpaceCollection = TuningSpaceContainer.TuningSpacesForCLSID(NTSC_GUID)
        If TuningSpaceCollection.Count = 0 Then
            MsgBox ("Couldn't find an NTSC Tuning Space.")
            Call cmdPowerOff_click
            Exit Sub
        End If
        
        'Pick the tuning space named Cable
        For Each TS In TuningSpaceCollection
            If TS.UniqueName = "Cable" Then
                Set AnalogTV = TS
            End If
        Next
        
        If Not (AnalogTV.UniqueName = "Cable") Then
            MsgBox ("Couldn't find the cable TV tuning space on your system.  Re-install this tuning space.")
            Call cmdPowerOff_click
            Exit Sub
        End If
        
        'Create an AnalogTV tune request and view it
        Set AnalogTune = AnalogTV.CreateTuneRequest
        AnalogTune.Channel = 5
        VidControl.View AnalogTune
        CheckError "There was a problem with passing the analog TV tune request to the MSVidCtl.View() method."
        
        'Set FeaturesActive to nothing to disable CC (if it is on)
        Set FeaturesColl = New MSVidFeatures
        Set FeaturesColl = Nothing
        VidControl.FeaturesActive = FeaturesColl
        CheckError "There was a problem with setting the FeaturesActive collection to NULL."
        
        VidControl.Run
        CheckError "There was a problem running the graph.  Check that your TV tuner card and video card are properly installed."
        If (VidControl.State = STATE_UNBUILT) Then
                Call cmdPowerOff_click
                Exit Sub
        End If
        
        txtChannel.Text = VidControl.InputActive.Tune.Channel
        Call MakeAnalogTVToolsVisible
    
    'NTSC Analog TV playback init w/CC
    ElseIf cbSource.Text = "NTSC Analog TV w/CC" Then
        'Find all of the AnalogTV tuning spaces
        Set TuningSpaceCollection = TuningSpaceContainer.TuningSpacesForCLSID(NTSC_GUID)
        If TuningSpaceCollection.Count = 0 Then
            MsgBox ("Couldn't find an NTSC Tuning Space.")
            Call cmdPowerOff_click
            Exit Sub
        End If
        
        'Pick the tuning space named Cable
        For Each TS In TuningSpaceCollection
            If TS.UniqueName = "Cable" Then
                Set AnalogTV = TS
            End If
        Next
        
        If Not (AnalogTV.UniqueName = "Cable") Then
            MsgBox ("Couldn't find the cable TV tuning space on your system.  Please reinstall this tuning space.")
            Call cmdPowerOff_click
            Exit Sub
        End If
        
        'Create an AnalogTV tune request and view it
        Set AnalogTune = AnalogTV.CreateTuneRequest
        AnalogTune.Channel = 5
        VidControl.View AnalogTune
        CheckError "There was a problem with passing the analog TV tune request to the MSVidCtl.View() method."
        
        'Enable CC
        Set FeaturesAvailableColl = VidControl.FeaturesAvailable
        CheckError "There was a problem getting the FeaturesAvailable collection."
        
        For Each Feature In FeaturesAvailableColl
            If Feature.ClassID = CC_GUID Then
                Dim CCObj As MSVidClosedCaptioning
                Set CCObj = Feature
                CCObj.Enable = True
                FeaturesColl.Add Feature
                CheckError "There was a problem adding a feature to the collection."
            ElseIf Feature.ClassID = DATASVC_GUID Then
                FeaturesColl.Add Feature
                CheckError "There was a problem adding a feature to the collection."
            End If
        Next

        VidControl.FeaturesActive = FeaturesColl
        CheckError "There was a problem with setting the FeaturesActive collection."
        
        VidControl.Run
        CheckError "There was a problem running the graph.  Check that your TV tuner card and video card are properly installed."
        If (VidControl.State = STATE_UNBUILT) Then
                Call cmdPowerOff_click
                Exit Sub
        End If
        
        txtChannel.Text = VidControl.InputActive.Tune.Channel
        Call MakeAnalogTVToolsVisible
    
    'Digital TV playback init
    ElseIf cbSource.Text = "ATSC Digital Antenna TV" Then
        'Find the all of ATSC tuning spaces
        Set TuningSpaceCollection = TuningSpaceContainer.TuningSpacesForCLSID(ATSC_GUID)
        If TuningSpaceCollection.Count = 0 Then
            MsgBox ("Couldn't find an ATSC Tuning Space.")
            Call cmdPowerOff_click
            Exit Sub
        End If
        
        'Pick the tuning space named ATSC
        For Each TS In TuningSpaceCollection
            If TS.UniqueName = "ATSC" Then
                Set ATSCTV = TS
            End If
        Next
        
        If Not (ATSCTV.UniqueName = "ATSC") Then
            MsgBox ("Couldn't find the ATSC TV tuning space on your system.  Please reinstall this tuning space.")
            Call cmdPowerOff_click
            Exit Sub
        End If
        
        'Create a ATSC tune request and view it
        Set ATSCTune = ATSCTV.CreateTuneRequest
        ATSCLoc.PhysicalChannel = 46
        ATSCTune.Channel = -1
        ATSCTune.MinorChannel = -1
        ATSCTune.Locator = ATSCLoc
        
        'Set FeaturesActive to nothing to disable CC (if it is on)
        Set FeaturesColl = New MSVidFeatures
        'Set FeaturesColl = Nothing
        VidControl.FeaturesActive = FeaturesColl
        CheckError "There was a problem with setting the FeaturesActive collection to NULL."
        VidControl.View ATSCTune
        CheckError "There was a problem with passing the ATSC tune request to the MSVidCtl.View() method."
        VidControl.Build
        CheckError "Build"
        VidControl.Run
        CheckError "There was a problem running the graph.  Check that your TV tuner card and video card are properly installed."
        If (VidControl.State = STATE_UNBUILT) Then
            Call cmdPowerOff_click
            Exit Sub
        End If
        
        txtPhysicalChannel.Text = VidControl.InputActive.Tune.Locator.PhysicalChannel
        txtMinorChannel.Text = VidControl.InputActive.Tune.MinorChannel
        Call MakeDigitalTVToolsVisible
        'Hide these two buttons, as we can't mix in this mode
        cmdCapture.Visible = False
        cmdCaptureOff.Visible = False
        
    'Digital TV w/CC playback init
    ElseIf cbSource.Text = "ATSC Digital Antenna TV w/CC & Mixing Mode" Then
        'Find the all of ATSC tuning spaces
        Set TuningSpaceCollection = TuningSpaceContainer.TuningSpacesForCLSID(ATSC_GUID)
        If TuningSpaceCollection.Count = 0 Then
            MsgBox ("Couldn't find an ATSC Tuning Space.")
            Call cmdPowerOff_click
            Exit Sub
        End If
        
        'Pick the tuning space named ATSC
        For Each TS In TuningSpaceCollection
            If TS.UniqueName = "ATSC" Then
                Set ATSCTV = TS
            End If
        Next
        
        If Not (ATSCTV.UniqueName = "ATSC") Then
            MsgBox ("Couldn't find the ATSC TV tuning space on your system.  Please reinstall this tuning space.")
            Call cmdPowerOff_click
            Exit Sub
        End If
        
        'Create a ATSC tune request and view it
        Set ATSCTune = ATSCTV.CreateTuneRequest
        ATSCLoc.PhysicalChannel = 46
        ATSCTune.Channel = -1
        ATSCTune.MinorChannel = -1
        ATSCTune.Locator = ATSCLoc
        VidControl.View ATSCTune
        CheckError "There was a problem with passing the ATSC tune request to the MSVidCtl.View() method."
        
        'Enable CC (and mixing mode)
        Set FeaturesAvailableColl = VidControl.FeaturesAvailable
        CheckError "There was a problem getting the FeaturesAvailable collection."
        
        For Each Feature In FeaturesAvailableColl
            If Feature.ClassID = CC_GUID Then
                FeaturesColl.Add Feature
                CheckError "There was a problem adding a feature to the collection."
            End If
        Next

        VidControl.FeaturesActive = FeaturesColl
        CheckError "There was a problem with putting the FeaturesActive collection."
        
        VidControl.Run
        CheckError "There was a problem running the graph.  Check that your TV tuner card and video card are properly installed."
        If (VidControl.State = STATE_UNBUILT) Then
            Call cmdPowerOff_click
            Exit Sub
        End If
        
        txtPhysicalChannel.Text = VidControl.InputActive.Tune.Locator.PhysicalChannel
        txtMinorChannel.Text = VidControl.InputActive.Tune.MinorChannel
        Call MakeDigitalTVToolsVisible
        
    'Digital DVB-S TV playback init
    ElseIf cbSource.Text = "DVB-S Digital TV" Then
        'Find all of the DVB-S tuning spaces
        Set TuningSpaceCollection = TuningSpaceContainer.TuningSpacesForCLSID(DVBS_GUID)
        If TuningSpaceCollection.Count = 0 Then
            MsgBox ("Couldn't find a DVB Tuning Space.")
            Call cmdPowerOff_click
            Exit Sub
        End If
        
        'Pick any DVB-S tuning space we find
        For Each TS In TuningSpaceCollection
            If TS.UniqueName = "MYDVB" Then
                Set DVBSTV = TS
                Exit For
            End If
        Next
        
        If (IsNull(DVBSTV)) Then
            'If there is no tuning space exit
            MsgBox ("No MYDVB tuning space found. Please run the ViewDVB.htm file first")
            Call cmdPowerOff_click
        End If
        'Create a DVB tune request and view it
        Set DVBTune = DVBSTV.CreateTuneRequest
        CheckError "There was a problem creating a DVB-S tune request."
        DVBTune.SID = 101
        
        'Set FeaturesActive to nothing to disable CC (if it is on)
        Set FeaturesColl = New MSVidFeatures
        Set FeaturesColl = Nothing
        VidControl.FeaturesActive = FeaturesColl
        CheckError "There was a problem with setting the FeaturesActive collection to NULL."
        VidControl.View DVBTune
        CheckError "There was a problem with passing the DVB tune request to the MSVidCtl.View() method."
        VidControl.Run
        CheckError "There was a problem running the graph.  Check that your TV tuner card and video card are properly installed."
        If (VidControl.State = STATE_UNBUILT) Then
            Call cmdPowerOff_click
            Exit Sub
        End If
        
        Call MakeDigitalTVToolsVisible
        txtSID.Visible = True
        lblSID.Visible = True
        cmdEnterDVB.Visible = True
        'Hide these buttons, as they aren't used for DVB
        cmdCapture.Visible = False
        cmdCaptureOff.Visible = False
        txtPhysicalChannel.Visible = False
        txtMinorChannel.Visible = False
        lblPhysicalChannel.Visible = False
        lblMinorChannel.Visible = False
        txtChannel.Visible = False
        cmdEnterATSC.Visible = False
        cmdSeekUpDigital.Visible = False
        cmdSeekDownDigital.Visible = False
        
    'Digital DVB-S TV w/CC playback init
    ElseIf cbSource.Text = "DVB-S Digital TV w/CC & Mixing Mode" Then
        'Find all of the DVB-S tuning spaces
        Set TuningSpaceCollection = TuningSpaceContainer.TuningSpacesForCLSID(DVBS_GUID)
        If TuningSpaceCollection.Count = 0 Then
            MsgBox ("Couldn't find a DVB Tuning Space.")
            Call cmdPowerOff_click
            Exit Sub
        End If
        
        'Pick any DVB-S tuning space we find
        For Each TS In TuningSpaceCollection
            If Not (TS Is Nothing) Then
                Set DVBSTV = TS
            End If
        Next
        
        'Create a DVB tune request and view it
        Set DVBTune = DVBSTV.CreateTuneRequest
        CheckError "There was a problem creating the DVB-S tune request."
        DVBTune.SID = 101
        VidControl.View DVBTune
        CheckError "There was a problem with passing the DVB tune request to the MSVidCtl.View() method."
        
        'Enable CC (and mixing mode)
        Set FeaturesAvailableColl = VidControl.FeaturesAvailable
        CheckError "There was a problem getting the FeaturesAvailable collection."
        
        For Each Feature In FeaturesAvailableColl
            If Feature.ClassID = CC_GUID Then
                FeaturesColl.Add Feature
                CheckError "There was a problem adding a feature to the collection."
            End If
        Next

        VidControl.FeaturesActive = FeaturesColl
        CheckError "There was a problem with putting the FeaturesActive collection."
        
        VidControl.Run
        CheckError "There was a problem running the graph.  Check that your TV tuner card and video card are properly installed."
        If (VidControl.State = STATE_UNBUILT) Then
            Call cmdPowerOff_click
            Exit Sub
        End If
        
        Call MakeDigitalTVToolsVisible
        txtSID.Visible = True
        lblSID.Visible = True
        cmdEnterDVB.Visible = True
        'Hide the objects that aren't needed for DVB
        txtPhysicalChannel.Visible = False
        txtMinorChannel.Visible = False
        lblPhysicalChannel.Visible = False
        lblMinorChannel.Visible = False
        txtChannel.Visible = False
        cmdEnterATSC.Visible = False
        cmdSeekUpDigital.Visible = False
        cmdSeekDownDigital.Visible = False
        
    'User didn't pick a playback type
    Else
        MsgBox "You have chosen a playback type that is not implemented.  Please try again."
    End If
    
    Exit Sub
    
ON_ERROR:
    Call ProcessGeneralErorr
End Sub

Sub ProcessGeneralErorr()
    On Error Resume Next
    VidControl.Stop
    VidControl.Decompose
    ATSCTune = Null
    AnalogTune = Null
    DVBTune = Null
    AnalogTV = Null
    ATSCTV = Null
    DVBSTV = Null
    ATSCLoc = Null
    CheckError "General error - graph cannot run"
End Sub

'User is done with this playback
Sub cmdPowerOff_click()
    On Error Resume Next
    Call HideTools
    VidControl.Stop
    VidControl.Decompose
    
    CheckError "There was a problem tearing down the graph."
End Sub

Sub MakeAnalogTVToolsVisible()
'Make the appropriate analog TV controls visible
    cmdPowerOff.Visible = True
    cmdCapture.Visible = True
    cmdCaptureOff.Visible = True
    cmdEnterAnalog.Visible = True
    lblChannel.Visible = True
    txtChannel.Visible = True
    lblSourceInUse.Caption = cbSource.Text
    lblSourceInUse.Visible = True
    cbSource.Visible = False
    txtChannel.Text = AnalogTune.Channel
    cmdVolumeUp.Visible = True
    cmdVolumeDown.Visible = True
    cmdSeekUpAnalog.Visible = True
    cmdSeekDownAnalog.Visible = True
    cmdViewNext.Visible = True
    cmdInfo.Visible = True
End Sub

Sub MakeDigitalTVToolsVisible()
'Make the appropriate TV controls visible
    cmdPowerOff.Visible = True
    cmdCapture.Visible = True
    cmdCaptureOff.Visible = True
    cmdSeekUpDigital.Visible = True
    cmdSeekDownDigital.Visible = True
    cmdEnterATSC.Visible = True
    lblSourceInUse.Caption = cbSource.Text
    cbSource.Visible = False
    lblSourceInUse.Visible = True
    lblMinorChannel.Visible = True
    txtMinorChannel.Visible = True
    lblPhysicalChannel.Visible = True
    txtPhysicalChannel.Visible = True
    cmdVolumeUp.Visible = True
    cmdVolumeDown.Visible = True
    cmdViewNext.Visible = True
    cmdInfo.Visible = True
End Sub

'Hide all the controls we don't need to see
Sub HideTools()
    cmdPowerOff.Visible = False
    cmdCapture.Visible = False
    cmdCaptureOff.Visible = False
    cmdSeekUpAnalog.Visible = False
    cmdSeekDownAnalog.Visible = False
    cmdSeekUpDigital.Visible = False
    cmdSeekDownDigital.Visible = False
    cmdEnterATSC.Visible = False
    cmdEnterAnalog.Visible = False
    lblChannel.Visible = False
    txtChannel.Visible = False
    cbSource.Visible = True
    lblSourceInUse.Visible = False
    lblMinorChannel.Visible = False
    txtMinorChannel.Visible = False
    lblPhysicalChannel.Visible = False
    txtPhysicalChannel.Visible = False
    cmdVolumeUp.Visible = False
    cmdVolumeDown.Visible = False
    cmdViewNext.Visible = False
    txtSID.Visible = False
    cmdEnterDVB.Visible = False
    lblSID.Visible = False
    lblInfo.Visible = False
    cmdInfo.Visible = False
End Sub

'User presses Enter button to change a DVB-S channel
Private Sub cmdEnterDVB_Click()
    On Error Resume Next
    DVBTune.SID = txtSID.Text
    VidControl.View DVBTune
    CheckError "There was a problem with passing the DVB tune request to the MSVidCtl.View() method."
End Sub

'User presses Enter button to change a ATSC channel
Private Sub cmdEnterATSC_Click()
    On Error Resume Next
    ATSCLoc.PhysicalChannel = txtPhysicalChannel.Text
    ATSCTune.Locator = ATSCLoc
    ATSCTune.MinorChannel = txtMinorChannel.Text
    ATSCTune.Channel = -1
    VidControl.View ATSCTune
    CheckError "There was a problem with passing the ATSC tune request to the MSVidCtl.View() method."
    
    txtPhysicalChannel.Text = VidControl.InputActive.Tune.Locator.PhysicalChannel
    txtMinorChannel.Text = VidControl.InputActive.Tune.MinorChannel
End Sub

'User presses Enter button to change a NTSC channel
Private Sub cmdEnterAnalog_Click()
    On Error Resume Next
    AnalogTune.Channel = txtChannel.Text
    VidControl.View AnalogTune
    CheckError "There was a problem with passing the analog TV tune request to the MSVidCtl.View() method."

    txtChannel.Text = VidControl.InputActive.Tune.Channel
End Sub

'Change volume
Private Sub cmdVolumeUp_Click()
    On Error Resume Next
    If (VidControl.AudioRendererActive.Volume < 0) Then
        VidControl.AudioRendererActive.Volume = VidControl.AudioRendererActive.Volume + 1000
        CheckError "There was a problem with changing the volume."
    'Else
    '    MsgBox "Volume is set to maximum."
    End If
End Sub

'Change volume
Private Sub cmdVolumeDown_Click()
    On Error Resume Next
    If (VidControl.AudioRendererActive.Volume > -10000) Then
        VidControl.AudioRendererActive.Volume = VidControl.AudioRendererActive.Volume - 1000
        CheckError "There was a problem with changing the volume."
    'Else
    '    MsgBox "Volume is set to minimum."
    End If
End Sub

'Channel change up for ATSC
Private Sub cmdSeekUpDigital_Click()
    On Error Resume Next
    ATSCLoc.PhysicalChannel = ATSCLoc.PhysicalChannel + 1
    ATSCTune.Locator = ATSCLoc
    ATSCTune.MinorChannel = -1
    ATSCTune.Channel = -1
    VidControl.View ATSCTune
    CheckError "There was a problem with passing the ATSC tune request to the MSVidCtl.View() method."
    
    txtPhysicalChannel.Text = VidControl.InputActive.Tune.Locator.PhysicalChannel
    txtMinorChannel.Text = VidControl.InputActive.Tune.MinorChannel
End Sub

'Channel change down for ATSC
Private Sub cmdSeekDownDigital_Click()
    On Error Resume Next
    ATSCLoc.PhysicalChannel = ATSCLoc.PhysicalChannel - 1
    ATSCTune.Locator = ATSCLoc
    ATSCTune.MinorChannel = -1
    ATSCTune.Channel = -1
    VidControl.View ATSCTune
    CheckError "There was a problem with passing the ATSC tune request to the MSVidCtl.View() method."
    
    txtPhysicalChannel.Text = VidControl.InputActive.Tune.Locator.PhysicalChannel
    txtMinorChannel.Text = VidControl.InputActive.Tune.MinorChannel
End Sub

'Channel change up for NTSC
Private Sub cmdSeekUpAnalog_Click()
    On Error Resume Next
    AnalogTune.Channel = AnalogTune.Channel + 1
    VidControl.View AnalogTune
    CheckError "There was a problem with passing the ATSC tune request to the MSVidCtl.View() method."
    
    txtChannel.Text = VidControl.InputActive.Tune.Channel
End Sub

'Channel change down for NTSC
Private Sub cmdSeekDownAnalog_Click()
    On Error Resume Next
    AnalogTune.Channel = AnalogTune.Channel - 1
    VidControl.View AnalogTune
    CheckError "There was a problem with passing the ATSC tune request to the MSVidCtl.View() method."
    
    txtChannel.Text = VidControl.InputActive.Tune.Channel
End Sub

'Capture current video frame and alpha blend over video
Private Sub cmdCapture_Click()
    On Error Resume Next
    Dim Alpha As Integer
    Dim TempVidRend As MSVidVideoRenderer
    Dim MyRect As IMSVidRect
    Dim Pict As IPictureDisp
    
    'The amount of opacity for the image over video is 75% visible
    Alpha = 75
    
    'Get the current video renderer
    Set TempVidRend = VidControl.VideoRendererActive
    CheckError "Failed to retrieve the current video renderer."
    
    'Capture the frame of video
    Set Pict = TempVidRend.Capture
    CheckError "Failed to capture the video frame."
    
    'Set the properties for the image and then display it
    TempVidRend.MixerBitmap = Pict
    TempVidRend.MixerBitmapOpacity = Alpha
    Set MyRect = TempVidRend.MixerBitmapPositionRect
    MyRect.Top = 10
    MyRect.Left = 10
    MyRect.Height = (VidControl.Height) / 4
    MyRect.Width = (VidControl.Width) / 4
    TempVidRend.MixerBitmapPositionRect = MyRect
    CheckError "Failed to display the frame capture.  Your video card may not be compatible with the WindowsXP Video Mixing Renderer."
End Sub

'Remove the alpha blended image
Private Sub cmdCaptureOff_Click()
    On Error Resume Next
    Dim TempVidRend As MSVidVideoRenderer
    Set TempVidRend = VidControl.VideoRendererActive
    CheckError "Failed to retrieve the current video renderer."
    TempVidRend.MixerBitmap = Nothing
    CheckError "Failed to disable MixerBitmap."
End Sub

Private Sub cmdViewNext_Click()
'Try the next tuner device
    On Error Resume Next
    VidControl.Stop
    If VidControl.InputActive.TuningSpace.CLSID = NTSC_GUID Then
        VidControl.ViewNext AnalogTune
        CheckError "Failed to ViewNext for NTSC."
    ElseIf VidControl.InputActive.TuningSpace.CLSID = ATSC_GUID Then
        VidControl.ViewNext ATSCTune
        CheckError "Failed to ViewNext for ATSC."
    ElseIf VidControl.InputActive.TuningSpace.CLSID = DVBS_GUID Then
        VidControl.ViewNext DVBTune
        CheckError "Failed to ViewNext for DVB."
    Else
        MsgBox "There is not a tuning space to match the current InputActive."
    End If
    VidControl.Run
    CheckError "Unable to run after changing InputActive."
End Sub

Private Sub cmdInfo_Click()
'Display Input Name and FPS
    lblInfo.Visible = True
    lblInfo.Caption = "Device Name: " & VidControl.InputActive.Name & "  FPS: " & (VidControl.VideoRendererActive.FramesPerSecond / 100)
End Sub

