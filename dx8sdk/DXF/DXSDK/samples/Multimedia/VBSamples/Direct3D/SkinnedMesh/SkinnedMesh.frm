VERSION 5.00
Begin VB.Form frmSkinnedMesh 
   Caption         =   "Skinned Mesh"
   ClientHeight    =   6015
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   7530
   Icon            =   "SkinnedMesh.frx":0000
   LinkTopic       =   "Form3"
   ScaleHeight     =   401
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   502
   StartUpPosition =   3  'Windows Default
End
Attribute VB_Name = "frmSkinnedMesh"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       skinnedMesh.frm
'  Content:    Animate Skinned Geometry
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Option Explicit

Dim Character As CD3DFrame
Dim Animation As CD3DAnimation
Dim MediaDir As String
Dim m_bInit As Boolean
Dim m_bMinimized As Boolean


Private Sub Form_Load()
    Dim hr As Long
    
    Me.Show
    DoEvents
    
    
    'find a path to our media
    MediaDir = FindMediaDir("tiny.x")
    D3DUtil_SetMediaPath MediaDir
    
    ' Initialize D3D
    ' Note: D3DUtil_Init will attempt to use D3D Hardware acceleartion.
    ' If it is not available it attempt to use the Software Reference Rasterizer.
    ' If all fail it will display a message box indicating so.
    '
    m_bInit = D3DUtil_Init(Me.hwnd, True, 0, 0, D3DDEVTYPE_HAL, Me)
    If Not (m_bInit) Then End
                
    
    ' Create new D3D mesh and animation objects
    InitDeviceObjects
    
    ' Sets the state for those objects and the current D3D device
    RestoreDeviceObjects
    
    ' Start our timer
    DXUtil_Timer TIMER_start
    
    ' Run the simulation forever
    ' See Form_Keydown for exit processing
    Do While True
    
        ' Increment the simulation
        FrameMove
        
        ' Render one image of the simulation
        If Render Then
            
            ' Present the image to the screen
            D3DUtil_PresentAll g_focushwnd
        End If
        
        ' Allow for events to get processed
        DoEvents
        
    Loop
                   
    
End Sub

'-----------------------------------------------------------------------------
' Name: FrameMove()
' Desc:
'-----------------------------------------------------------------------------
Sub FrameMove()
    Dim apptime As Single
    
    'get ellapsed time since start of application
    apptime = DXUtil_Timer(TIMER_GETAPPTIME)
    
    'Have our animation pose our character
    Animation.SetTime (apptime) * 4000
            
    'Rotate the character
    Character.AddRotation COMBINE_replace, 0, 0, 1, 3.14 + (apptime) / 8
    
    'Update all frame matrices (required for skinning)
    Character.UpdateFrames

End Sub


'-----------------------------------------------------------------------------
' Name: Render()
' Desc:
'-----------------------------------------------------------------------------
Function Render() As Boolean
    Dim hr As Long
   
    Render = False
    'See what state the device is in.
    hr = g_dev.TestCooperativeLevel
    If hr = D3DERR_DEVICENOTRESET Then
        g_dev.Reset g_d3dpp
        RestoreDeviceObjects
    End If
    
    'dont bother rendering if we are not ready yet
    If hr <> 0 Then Exit Function
    Render = True

    'Clear the background to ARGB grey
    D3DUtil_ClearAll &HFF9090FF


    'Start the Scene
    g_dev.BeginScene
    
    'Render the character
    Character.RenderSkins
    
    'End the scene
    g_dev.EndScene
    

End Function

'-----------------------------------------------------------------------------
' Name: InitDeviceObjects()
' Desc:
'-----------------------------------------------------------------------------
Sub InitDeviceObjects()
    
    'Create an Animation object to hold any animations
    Set Animation = New CD3DAnimation
    
    'Load a skinned character
    Set Character = D3DUtil_LoadFromFileAsSkin(MediaDir + "tiny.x", Nothing, Animation)
    
End Sub


'-----------------------------------------------------------------------------
' Name: RestoreDeviceObjects()
' Desc:
'-----------------------------------------------------------------------------
Sub RestoreDeviceObjects()
    
    'Set up some lights and camera
    g_lWindowWidth = Me.ScaleWidth
    g_lWindowHeight = Me.ScaleHeight
    D3DUtil_SetupDefaultScene
    
    'position the camera
    D3DUtil_SetupCamera vec3(0, 800, 200), vec3(0, 0, 200), vec3(0, 0, 1)
    
End Sub

'-----------------------------------------------------------------------------
' Name: InvalidateDeviceObjects()
' Desc: Place code to release non managed objects here
'-----------------------------------------------------------------------------
Sub InvalidateDeviceObjects()
    'all objects are managed
End Sub

'-----------------------------------------------------------------------------
' Name: DeleteDeviceObjects()
' Desc:
'-----------------------------------------------------------------------------
Sub DeleteDeviceObjects()
    Set Animation = Nothing
    Set Character = Nothing
    m_bInit = False
End Sub

'-----------------------------------------------------------------------------
' Name: Form_KeyDown()
' Desc: Process key messages for exit and change device
'-----------------------------------------------------------------------------
Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
     Select Case KeyCode
        
        Case vbKeyEscape
            Unload Me
            
        Case vbKeyF2
                
            ' Pause the timer
            DXUtil_Timer TIMER_STOP
            
            ' Bring up the device selection dialog
            ' we pass in the form so the selection process
            ' can make calls into InitDeviceObjects
            ' and RestoreDeviceObjects
            frmSelectDevice.SelectDevice Me
            
            ' Restart the timer
            DXUtil_Timer TIMER_start
            
        Case vbKeyReturn
        
            ' Check for Alt-Enter if not pressed exit
            If Shift <> 4 Then Exit Sub
            
            ' If we are windowed go fullscreen
            ' If we are fullscreen returned to windowed
            If g_d3dpp.Windowed Then
                 D3DUtil_ResetFullscreen
            Else
                 D3DUtil_ResetWindowed
            End If
                             
            ' Call Restore after ever mode change
            ' because calling reset looses state that needs to
            ' be reinitialized
            RestoreDeviceObjects
           
    End Select
End Sub


'-----------------------------------------------------------------------------
' Name: Form_Resize()
' Desc: hadle resizing of the D3D backbuffer
'-----------------------------------------------------------------------------
Private Sub Form_Resize()

    ' If D3D is not initialized then exit
    If Not m_bInit Then Exit Sub
    
    ' If we are in a minimized state stop the timer and exit
    If Me.WindowState = vbMinimized Then
        DXUtil_Timer TIMER_STOP
        m_bMinimized = True
        Exit Sub
        
    ' If we just went from a minimized state to maximized
    ' restart the timer
    Else
        If m_bMinimized = True Then
            DXUtil_Timer TIMER_start
            m_bMinimized = False
        End If
    End If
    
    ' Dont let the window get too small
    If Me.ScaleWidth < 10 Then
        Me.width = Screen.TwipsPerPixelX * 10
        Exit Sub
    End If
    
    If Me.ScaleHeight < 10 Then
        Me.height = Screen.TwipsPerPixelY * 10
        Exit Sub
    End If

    'reset and resize our D3D backbuffer to the size of the window
    D3DUtil_ResizeWindowed Me.hwnd
    
    'All state get losts after a reset so we need to reinitialze it here
    RestoreDeviceObjects
    
End Sub

'-----------------------------------------------------------------------------
' Name: Form_Unload()
' Desc:
'-----------------------------------------------------------------------------
Private Sub Form_Unload(Cancel As Integer)
    DeleteDeviceObjects
    End
End Sub


'-----------------------------------------------------------------------------
' Name: Form_Unload()
' Desc:
'-----------------------------------------------------------------------------
Public Function VerifyDevice(flags As Long, format As CONST_D3DFORMAT) As Boolean
    If flags = D3DCREATE_HARDWARE_VERTEXPROCESSING Then Exit Function
    VerifyDevice = True
End Function
