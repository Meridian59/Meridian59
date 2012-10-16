VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "cInput"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

'Here we will control all of the input, from any source
'List of camera views (same enum listed in the camera class)
Private Enum DefaultCameraViews
    DefaultView
    OverHeadView
    SideOverheadView1
    SideOverheadView2
    OpponentView
    CustomView
End Enum

'*NOTE*
'
'I may want to add Force Feedback support, if i do, I would do so here.

'Mouse constants
Private Const mlJoystickRange As Long = 35
Private Const mnMaxZThresh As Single = 35
Private Const mnMaxYThresh As Single = 50
Private Const mnMaxXThresh As Single = 35

'DirectInput variables, etc
Private Const glBufferSize As Long = 10
'DInput objects
Private di As DirectInput8
Private diMouse As DirectInputDevice8
Private diKeyboard As DirectInputDevice8
Private diJoystick As DirectInputDevice8
'Is the camera moving?
Private mfMovingCamera As Boolean

'Local properties to determine what controls should be used
Public UseMouse As Boolean
Public UseKeyboard As Boolean
Public UseJoystick As Boolean
Public JoystickGuid As String
Public JoystickSensitivity As Single
Public MouseSensitivity As Single
Public KeyboardSensitivity As Single

Public Property Get InputObject() As DirectInput8
    Set InputObject = di
End Property

Public Function InitDirectInput(oForm As Form) As Boolean
  
    Dim diProp As DIPROPLONG
    Dim diProp_Dead As DIPROPLONG
    Dim diProp_Range As DIPROPRANGE
    Dim diProp_Saturation As DIPROPLONG
    
    On Error GoTo FailedInput
    
    InitDirectInput = True
    'Create the DirectInput object, and all of the devices we need.
    If UseMouse Then
        Set diMouse = di.CreateDevice("guid_SysMouse")
        diMouse.SetCommonDataFormat DIFORMAT_MOUSE
        diMouse.SetCooperativeLevel oForm.hwnd, DISCL_FOREGROUND Or DISCL_EXCLUSIVE
    
        ' Set the buffer size
        diProp.lHow = DIPH_DEVICE
        diProp.lObj = 0
        diProp.lData = glBufferSize
        Call diMouse.SetProperty("DIPROP_BUFFERSIZE", diProp)
        'Acquire the mouse
        diMouse.Acquire
    End If
    
    If UseKeyboard Then
        Set diKeyboard = di.CreateDevice("GUID_SysKeyboard")
        
        diKeyboard.SetCommonDataFormat DIFORMAT_KEYBOARD
        diKeyboard.SetCooperativeLevel oForm.hwnd, DISCL_FOREGROUND Or DISCL_NONEXCLUSIVE
        'Acquire the keyboard
        diKeyboard.Acquire
    End If
    
    If UseJoystick Then
        On Error Resume Next
        Set diJoystick = di.CreateDevice(JoystickGuid)
        If Err Then 'This joystick doesn't exist anymore
            UseJoystick = False
            Exit Function
        End If
        On Error GoTo FailedInput
        diJoystick.SetCommonDataFormat DIFORMAT_JOYSTICK
        diJoystick.SetCooperativeLevel oForm.hwnd, DISCL_FOREGROUND Or DISCL_EXCLUSIVE
        
        'Set deadzone to 10 percent
        With diProp_Dead
            .lData = mlJoystickRange \ 20
            .lHow = DIPH_BYOFFSET
            .lObj = DIJOFS_X
            diJoystick.SetProperty "DIPROP_DEADZONE", diProp_Dead
            .lObj = DIJOFS_Y
            diJoystick.SetProperty "DIPROP_DEADZONE", diProp_Dead
        End With
        
        'Set saturation zones to 5 percent
        With diProp_Saturation
            .lData = mlJoystickRange \ 40
            .lHow = DIPH_BYOFFSET
            .lObj = DIJOFS_X
             diJoystick.SetProperty "DIPROP_SATURATION", diProp_Saturation
            .lObj = DIJOFS_Y
             diJoystick.SetProperty "DIPROP_SATURATION", diProp_Saturation
        End With
    
        'Just in case this device doesn't let us set the range
        On Error Resume Next
        'Set range for all axes
        With diProp_Range
            .lHow = DIPH_DEVICE
            .lMin = -mlJoystickRange
            .lMax = mlJoystickRange
        End With
        diJoystick.SetProperty "DIPROP_RANGE", diProp_Range
        On Error GoTo FailedInput
        
        diJoystick.Acquire
    End If
    
    Exit Function
  
FailedInput:
    InitDirectInput = False

End Function

Private Sub ProcessMouseData(oPaddle As cPaddle, oPuck As cPuck)
    'This is where we respond to any change in mouse state. Usually this will be an axis movement
    'or button press or release

    Dim diDeviceData(1 To glBufferSize) As DIDEVICEOBJECTDATA
    Dim lNumItems As Long
    Dim lCount As Integer
    Dim lTempX As Single, lTempZ As Single
  
    On Error GoTo INPUTLOST 'In case we lost the mouse
    diMouse.Acquire 'Just in case
    lNumItems = diMouse.GetDeviceData(diDeviceData, 0)
    On Error GoTo 0 'Reset our error
    
    ' Process data
    For lCount = 1 To lNumItems
        Select Case diDeviceData(lCount).lOfs
        Case DIMOFS_X 'We moved the X axis
            If mfMovingCamera Then
                With goCamera.Position
                    If lTempZ = 0 Then lTempZ = .z
                    lTempX = .X + (diDeviceData(lCount).lData * MouseSensitivity)
                    goCamera.SetCameraPosition CustomView, oPaddle.PaddleID
                    If Abs(lTempX) > mnMaxXThresh Then
                        'Whoops too much
                        lTempX = mnMaxXThresh * (lTempX / Abs(lTempX))
                    End If
                End With
            Else
                With oPaddle.Position
                    If lTempZ = 0 Then lTempZ = .z
                    lTempX = .X + (diDeviceData(lCount).lData * MouseSensitivity)
                End With
            End If
        Case DIMOFS_Y 'We moved the Y axis
            If mfMovingCamera Then
                With goCamera.Position
                    If lTempX = 0 Then lTempX = .X
                    lTempZ = .z - (diDeviceData(lCount).lData * MouseSensitivity)
                    goCamera.SetCameraPosition CustomView, oPaddle.PaddleID
                    If Abs(lTempZ) > mnMaxZThresh Then
                        'Whoops too much
                        lTempZ = mnMaxZThresh * (lTempZ / Abs(lTempZ))
                    End If
                End With
            Else
                With oPaddle.Position
                    If lTempX = 0 Then lTempX = .X
                    lTempZ = .z - (diDeviceData(lCount).lData * MouseSensitivity)
                End With
            End If
        Case DIMOFS_BUTTON1
            mfMovingCamera = (diDeviceData(lCount).lData And &H80 = &H80)
        End Select
    Next lCount
    'Ok, this sequence is done, process the info, and move on
    If lTempX <> 0 And lTempZ <> 0 Then
        If mfMovingCamera Then
            goCamera.Position = vec3(lTempX, goCamera.Position.Y, lTempZ)
        Else
            oPaddle.LastPosition = oPaddle.Position
            oPaddle.Position = vec3(lTempX, oPaddle.Position.Y, lTempZ)
            oPaddle.Velocity = vec3(oPaddle.Position.X - oPaddle.LastPosition.X, oPaddle.Velocity.Y, oPaddle.Position.z - oPaddle.LastPosition.z)
            oPaddle.LastVelocityTick = timeGetTime
        End If
    End If
    MakeSurePaddleIsOnBoard oPaddle
    Exit Sub
    
INPUTLOST:
    If (Err.Number = DIERR_INPUTLOST) Or (Err.Number = DIERR_NOTACQUIRED) Then
        'We no longer have the mouse..
    End If
End Sub

Private Sub ProcessKeyBoardData(oPaddle As cPaddle, oPuck As cPuck)
    
    'This is where we respond to any change in keyboard state. Usually this will be an axis movement
    'or button press or release
    
    Dim diKeys As DIKEYBOARDSTATE
    Dim lTempX As Single, lTempZ As Single
    
    On Error GoTo INPUTLOST 'In case we lost focus
    diKeyboard.Acquire 'Just in case
    diKeyboard.GetDeviceStateKeyboard diKeys
    
    If KeyPressed(diKeys, DIK_LEFTARROW) Or KeyPressed(diKeys, DIK_NUMPAD4) Then
        oPaddle.LastPosition = oPaddle.Position
        With oPaddle.Position
            lTempX = .X - KeyboardSensitivity
            oPaddle.Position = vec3(lTempX, .Y, .z)
        End With
        oPaddle.Velocity = vec3(oPaddle.Position.X - oPaddle.LastPosition.X, oPaddle.Velocity.Y, oPaddle.Velocity.z)
        oPaddle.LastVelocityTick = timeGetTime
    End If
    If KeyPressed(diKeys, DIK_RIGHTARROW) Or KeyPressed(diKeys, DIK_NUMPAD6) Then
        oPaddle.LastPosition = oPaddle.Position
        With oPaddle.Position
            lTempX = .X + KeyboardSensitivity
            oPaddle.Position = vec3(lTempX, .Y, .z)
        End With
        oPaddle.Velocity = vec3(oPaddle.Position.X - oPaddle.LastPosition.X, oPaddle.Velocity.Y, oPaddle.Velocity.z)
        oPaddle.LastVelocityTick = timeGetTime
    End If
    If KeyPressed(diKeys, DIK_UPARROW) Or KeyPressed(diKeys, DIK_NUMPAD8) Then
        oPaddle.LastPosition = oPaddle.Position
        With oPaddle.Position
            lTempZ = .z + KeyboardSensitivity
            oPaddle.Position = vec3(.X, .Y, lTempZ)
        End With
        oPaddle.Velocity = vec3(oPaddle.Velocity.X, oPaddle.Velocity.Y, oPaddle.Position.z - oPaddle.LastPosition.z)
        oPaddle.LastVelocityTick = timeGetTime
    End If
    If KeyPressed(diKeys, DIK_DOWNARROW) Or KeyPressed(diKeys, DIK_NUMPAD2) Then
        oPaddle.LastPosition = oPaddle.Position
        With oPaddle.Position
            lTempZ = .z - KeyboardSensitivity
            oPaddle.Position = vec3(.X, .Y, lTempZ)
        End With
        oPaddle.Velocity = vec3(oPaddle.Velocity.X, oPaddle.Velocity.Y, oPaddle.Position.z - oPaddle.LastPosition.z)
        oPaddle.LastVelocityTick = timeGetTime
    End If
    
    MakeSurePaddleIsOnBoard oPaddle
    Exit Sub
    
INPUTLOST:
    If (Err.Number = DIERR_INPUTLOST) Or (Err.Number = DIERR_NOTACQUIRED) Then
        'We no longer have the mouse..
    End If
End Sub

Private Sub ProcessJoystickData(oPaddle As cPaddle, oPuck As cPuck)
    
    'This is where we respond to any change in keyboard state. Usually this will be an axis movement
    'or button press or release
    
    Dim diJoy As DIJOYSTATE
    Dim lTempX As Single, lTempZ As Single
    
    On Error GoTo INPUTLOST 'In case we lost focus
    diJoystick.Acquire 'Just in case
    diJoystick.Poll
    diJoystick.GetDeviceStateJoystick diJoy
    
    If diJoy.X <> 0 Then
        oPaddle.LastPosition = oPaddle.Position
        With oPaddle.Position
            lTempX = .X + (diJoy.X * JoystickSensitivity)
            oPaddle.Position = vec3(lTempX, .Y, .z)
        End With
        oPaddle.Velocity = vec3(oPaddle.Position.X - oPaddle.LastPosition.X, oPaddle.Velocity.Y, oPaddle.Velocity.z)
        oPaddle.LastVelocityTick = timeGetTime
    End If
    If diJoy.Y <> 0 Then
        oPaddle.LastPosition = oPaddle.Position
        With oPaddle.Position
            lTempZ = .z - (diJoy.Y * JoystickSensitivity)
            oPaddle.Position = vec3(.X, .Y, lTempZ)
        End With
        oPaddle.Velocity = vec3(oPaddle.Velocity.X, oPaddle.Velocity.Y, oPaddle.Position.z - oPaddle.LastPosition.z)
        oPaddle.LastVelocityTick = timeGetTime
    End If
    
    MakeSurePaddleIsOnBoard oPaddle
    Exit Sub
    
INPUTLOST:
    If (Err.Number = DIERR_INPUTLOST) Or (Err.Number = DIERR_NOTACQUIRED) Then
        'We no longer have the joystick..
    End If
End Sub

Public Sub GetAndHandleInput(oPaddle As cPaddle, oPuck As cPuck)

    Dim vOldPaddle As D3DVECTOR
    
    oPaddle.Velocity = vec3(0, 0, 0)
    vOldPaddle = oPaddle.Position
    If UseMouse Then
        'First let's handle the mouse
        ProcessMouseData oPaddle, oPuck
    End If
        
    If UseKeyboard Then
        'Now we can worry about keyboard
        ProcessKeyBoardData oPaddle, oPuck
    End If
    
    If UseJoystick Then
        'If we have a joystick selected check that too
        ProcessJoystickData oPaddle, oPuck
    End If
    oPaddle.EnsureReality vOldPaddle, oPuck
End Sub

'Helper function to determine if a key is pressed
Private Function KeyPressed(diKeys As DIKEYBOARDSTATE, Key As Byte)
    KeyPressed = (diKeys.Key(Key) And &H80 = &H80)
End Function

Private Function MakeSurePaddleIsOnBoard(oPaddle As cPaddle)
    Dim lTempZ As Single, lTempX As Single
    lTempX = oPaddle.Position.X
    lTempZ = oPaddle.Position.z
    
    'Don't let the paddle leave the left or right sides of the table
    If lTempX > (gnSideLeftWallEdge - (gnPaddleRadius)) Then
        lTempX = (gnSideLeftWallEdge - (gnPaddleRadius))
    ElseIf lTempX < (gnSideRightWallEdge + (gnPaddleRadius)) Then
        lTempX = (gnSideRightWallEdge + (gnPaddleRadius))
    End If
    'Depending on which end of the table we are *supposed* to be on,
    'restrict our movement.
    If oPaddle.PaddleID = 0 Then
        If lTempZ > -(gnPaddleRadius * 1.5) Then
            lTempZ = -(gnPaddleRadius * 1.5)
        ElseIf lTempZ < (gnFarWallEdge + (gnPaddleRadius)) Then
            lTempZ = (gnFarWallEdge + (gnPaddleRadius))
        End If
    Else
        If lTempZ > (gnNearWallEdge - (gnPaddleRadius)) Then
            lTempZ = (gnNearWallEdge - (gnPaddleRadius))
        ElseIf lTempZ < (gnPaddleRadius * 1.5) Then
            lTempZ = (gnPaddleRadius * 1.5)
        End If
    End If
    
    oPaddle.Position = vec3(lTempX, oPaddle.Position.Y, lTempZ)
End Function

Private Sub Class_Initialize()
    Set diMouse = Nothing
    Set diKeyboard = Nothing
    Set diJoystick = Nothing
    Set di = Nothing
    Set di = dx.DirectInputCreate
End Sub

Private Sub Class_Terminate()
    On Error Resume Next 'Ignore any errors, we're cleaning everything up
    'Unacquire the mouse
    If Not (diMouse Is Nothing) Then diMouse.Unacquire
    If Not (diKeyboard Is Nothing) Then diKeyboard.Unacquire
    If Not (diJoystick Is Nothing) Then diJoystick.Unacquire
    'Destroy our objects
    Set diMouse = Nothing
    Set diKeyboard = Nothing
    Set diJoystick = Nothing
    Set di = Nothing
End Sub
