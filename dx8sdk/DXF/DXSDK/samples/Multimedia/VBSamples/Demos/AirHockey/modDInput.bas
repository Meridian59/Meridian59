Attribute VB_Name = "modDInput"
Option Explicit

Private Enum DefaultCameraViews
    DefaultView
    OverHeadView
    SideOverheadView1
    SideOverheadView2
    OpponentView
    CustomView
End Enum

Private Const mnMouseSensitivity As Single = 0.02
Private Const mnMaxZThresh As Single = 35
Private Const mnMaxYThresh As Single = 50
Private Const mnMaxXThresh As Single = 35
Private mnLastX As Single
Private mnLastY As Single

'DirectInput variables, etc
Public Const glBufferSize As Long = 10
Public Const gnVelocityBoost As Single = 1.1

Public DI As DirectInput8
Public DIMouse As DirectInputDevice8
Public gfMovingCamera As Boolean

Public Function InitDInput(oForm As Form) As Boolean
  
  Dim diProp As DIPROPLONG
  
  On Error GoTo FailedInput
  
  InitDInput = True
  Set DI = dx.DirectInputCreate
  Set DIMouse = DI.CreateDevice("guid_SysMouse")
  Call DIMouse.SetCommonDataFormat(DIFORMAT_MOUSE)
  Call DIMouse.SetCooperativeLevel(oForm.hwnd, DISCL_FOREGROUND Or DISCL_EXCLUSIVE)
  
  ' Set the buffer size
  diProp.lHow = DIPH_DEVICE
  diProp.lObj = 0
  diProp.lData = glBufferSize
  Call DIMouse.SetProperty("DIPROP_BUFFERSIZE", diProp)

  'Acquire the mouse
  DIMouse.Acquire
  Exit Function
  
FailedInput:
    InitDInput = False

End Function

Public Sub CleanupDInput()
    On Error Resume Next
    'Unacquire the mouse
    If Not (DIMouse Is Nothing) Then DIMouse.Unacquire
    'Destroy our objects
    Set DIMouse = Nothing
    Set DI = Nothing
End Sub

Public Sub ProcessMouseData()
    'This is where we respond to any change in mouse state. Usually this will be an axis movement
    'or button press or release, but it could also mean we've lost acquisition.

    Dim diDeviceData(1 To glBufferSize) As DIDEVICEOBJECTDATA
    Dim lNumItems As Long
    Dim lCount As Integer
    Dim vOldPaddle As D3DVECTOR
    Static OldSequence As Long
  
    On Error GoTo INPUTLOST 'In case we lost the mouse
    DIMouse.Acquire 'Just in case
    lNumItems = DIMouse.GetDeviceData(diDeviceData, 0)
     On Error GoTo 0 'Reset our error
    
    vOldPaddle = goPaddle(glMyPaddleID).Position
    ' Process data
    For lCount = 1 To lNumItems
        Select Case diDeviceData(lCount).lOfs
        Case DIMOFS_X 'We moved the X axis
            If gfMovingCamera Then
                With goCamera.Position
                    .x = .x + (diDeviceData(lCount).lData * mnMouseSensitivity)
                    goCamera.SetCameraPosition CustomView, glMyPaddleID
                    If Abs(.x) > mnMaxXThresh Then
                        'Whoops too much
                        .x = mnMaxXThresh * (.x / Abs(.x))
                    End If
                End With
            Else
                goPaddle(glMyPaddleID).LastPosition = goPaddle(glMyPaddleID).Position
                With goPaddle(glMyPaddleID).Position
                    .x = .x + (diDeviceData(lCount).lData * mnMouseSensitivity)
                    If .x > (gnSideLeftWallEdge - (gnPaddleRadius)) Then
                        .x = (gnSideLeftWallEdge - (gnPaddleRadius))
                    ElseIf .x < (gnSideRightWallEdge + (gnPaddleRadius)) Then
                        .x = (gnSideRightWallEdge + (gnPaddleRadius))
                    End If
                End With
                goPaddle(glMyPaddleID).Velocity.x = goPaddle(glMyPaddleID).Position.x - goPaddle(glMyPaddleID).LastPosition.x
                goPaddle(glMyPaddleID).LastVelocityTick = timeGetTime
            End If
        Case DIMOFS_Y 'We moved the Y axis
            If gfMovingCamera Then
                With goCamera.Position
                    .z = .z - (diDeviceData(lCount).lData * mnMouseSensitivity)
                    goCamera.SetCameraPosition CustomView, glMyPaddleID
                    If Abs(.z) > mnMaxZThresh Then
                        'Whoops too much
                        .z = mnMaxZThresh * (.z / Abs(.z))
                    End If
                End With
            Else
                goPaddle(glMyPaddleID).LastPosition = goPaddle(glMyPaddleID).Position
                With goPaddle(glMyPaddleID).Position
                    .z = .z - (diDeviceData(lCount).lData * mnMouseSensitivity)
                    'The front and rear walls are count to the Z axis
                    If glMyPaddleID = 0 Then
                        If .z > -2 Then
                            .z = -2
                        ElseIf .z < (gnFarWallEdge + (gnPaddleRadius)) Then
                            .z = (gnFarWallEdge + (gnPaddleRadius))
                        End If
                    Else
                        If .z > (gnNearWallEdge - (gnPaddleRadius)) Then
                            .z = (gnNearWallEdge - (gnPaddleRadius))
                        ElseIf .z < 2 Then
                            .z = 2
                        End If
                    End If
                End With
                goPaddle(glMyPaddleID).Velocity.z = goPaddle(glMyPaddleID).Position.z - goPaddle(glMyPaddleID).LastPosition.z
                goPaddle(glMyPaddleID).LastVelocityTick = timeGetTime
            End If
    
        Case DIMOFS_BUTTON1
            gfMovingCamera = (diDeviceData(lCount).lData And &H80 = &H80)
    
        End Select
        EnsurePaddleReality vOldPaddle, goPaddle(glMyPaddleID)
    Next lCount
    Exit Sub
    
INPUTLOST:
    If (Err.Number = DIERR_INPUTLOST) Or (Err.Number = DIERR_NOTACQUIRED) Then
        'We no longer have the mouse..
    End If
End Sub

Public Sub GetAndHandleDinput()
    'First let's handle the mouse
    ProcessMouseData
    'Now we can worry about keyboard
    'If we have a joystick selected check that too
End Sub
