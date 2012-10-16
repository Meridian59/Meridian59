VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "cCamera"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

'Here we will encapsulate all of the code needed for the camera

Private Enum DefaultCameraViews
    DefaultView
    OverHeadView
    SideOverheadView1
    SideOverheadView2
    OpponentView
    CustomView
End Enum

'Here are the constants for the default view
Private Const mnDefaultX As Single = 0
Private Const mnDefaultY As Single = 10
Private Const mnDefaultZ As Single = -25
'Here are the constants for the overhead views
Private Const mnOverheadX As Single = 0
Private Const mnOverheadY As Single = 28
Private Const mnOverheadZ As Single = -1
'Here are the constants for the side overhead views
Private Const mnSide1X As Single = 25
Private Const mnSide1Y As Single = 12.5
Private Const mnSide1Z As Single = 0
Private Const mnSide2X As Single = -25
Private Const mnSide2Y As Single = 12.5
Private Const mnSide2Z As Single = 0
'Here are the constants for the opponent views
Private Const mnOpponentX As Single = 0
Private Const mnOpponentY As Single = 10
Private Const mnOpponentZ As Single = 25

'Local variables for the properties of the puck
Private moPosition As D3DVECTOR 'Current position of the camera
Private moVelocity As D3DVECTOR 'Current velocity of the camera
Private moDest As D3DVECTOR 'Destination of the camera
Private mlCameraTime As Long 'Last time the puck was updated
Private moLastPosition As D3DVECTOR 'Last position of the camera
'The default camera views
Public CameraView As Long

'Position property
Public Property Let Position(oPos As D3DVECTOR)
    moPosition = oPos
End Property

Public Property Get Position() As D3DVECTOR
    Position = moPosition
End Property

'Velocity property
Public Property Let Velocity(oVel As D3DVECTOR)
    moVelocity = oVel
End Property

Public Property Get Velocity() As D3DVECTOR
    Velocity = moVelocity
End Property

'LastPosition prop
Public Property Let LastPosition(oLastPos As D3DVECTOR)
    moLastPosition = oLastPos
End Property

Public Property Get LastPosition() As D3DVECTOR
    LastPosition = moLastPosition
End Property

'Dest property
Public Property Let Dest(oPos As D3DVECTOR)
    moDest = oPos
End Property

Public Property Get Dest() As D3DVECTOR
    Dest = moDest
End Property

'Methods
Public Sub UpdatePosition()
    Dim RealVelocity As D3DVECTOR
    Dim DistancePointX As Single
    Dim DistancePointY As Single
    Dim DistancePointZ As Single
    
    'Here we will update the position of the camera
    'and move it based on the velocity assigned.
    If mlCameraTime = 0 Then mlCameraTime = timeGetTime
    'First calculate the 'real' velocity (based on the time)
    RealVelocity.x = ((timeGetTime - mlCameraTime) / 100) * moVelocity.x
    RealVelocity.y = ((timeGetTime - mlCameraTime) / 100) * moVelocity.y
    RealVelocity.z = ((timeGetTime - mlCameraTime) / 100) * moVelocity.z
    'Let's save our current position
    moLastPosition = moPosition
    'Now let's see if moving our position will move us past our destination
    'if it does, move us to our destination
    
    'First check the X axis
    DistancePointX = Sqr((moDest.x - moPosition.x) * (moDest.x - moPosition.x))
    If DistancePointX < RealVelocity.x Then
        moPosition.x = moDest.x 'We've arrived
        moVelocity.x = 0
    Else
        moPosition.x = moPosition.x + RealVelocity.x 'We haven't got to our destination yet, keep going
    End If
    'Now check the Y axis
    DistancePointY = Sqr((moDest.y - moPosition.y) * (moDest.y - moPosition.y))
    If DistancePointY < RealVelocity.y Then
        moPosition.y = moDest.y 'We've arrived
        moVelocity.y = 0
    Else
        moPosition.y = moPosition.y + RealVelocity.y 'We haven't got to our destination yet, keep going
    End If
    'Now check the Z axis
    DistancePointZ = Sqr((moDest.z - moPosition.z) * (moDest.z - moPosition.z))
    If DistancePointZ < RealVelocity.z Then
        moPosition.z = moDest.z 'We've arrived
        moVelocity.z = 0
    Else
        moPosition.z = moPosition.z + RealVelocity.z 'We haven't got to our destination yet, keep going
    End If
    'Make sure our velocity is going in the right direction
    If DistancePointX < Sqr((moDest.x - moPosition.x) * (moDest.x - moPosition.x)) Then
        'It's not, reverse it
        moVelocity.x = moVelocity.x * -1
    End If
    If DistancePointY < Sqr((moDest.y - moPosition.y) * (moDest.y - moPosition.y)) Then
        'It's not, reverse it
        moVelocity.y = moVelocity.y * -1
    End If
    If DistancePointZ < Sqr((moDest.z - moPosition.z) * (moDest.z - moPosition.z)) Then
        'It's not, reverse it
        moVelocity.z = moVelocity.z * -1
    End If
    mlCameraTime = timeGetTime
End Sub

Public Sub NextCameraPosition(ByVal lPlayerID As Long)
    If CameraView = CustomView Then
        CameraView = DefaultView
    Else
        CameraView = CameraView + 1
        If CameraView = CustomView Then
            CameraView = DefaultView
        End If
    End If
    UpdateToNewPosition lPlayerID
End Sub

Public Sub SetCameraPosition(ByVal lCameraPos As Long, ByVal lPlayerID As Long)
    CameraView = lCameraPos
    If CameraView <> CustomView Then UpdateToNewPosition lPlayerID
End Sub

Private Sub UpdateToNewPosition(ByVal lPlayerID As Long)
    
    Select Case CameraView
    Case DefaultView
        If lPlayerID = 0 Then
            moDest.x = mnDefaultX
            moDest.y = mnDefaultY
            moDest.z = mnDefaultZ
        Else 'Default view should be the opponents view
            moDest.x = mnOpponentX
            moDest.y = mnOpponentY
            moDest.z = mnOpponentZ
        End If
    Case OpponentView
        If lPlayerID = 1 Then
            moDest.x = mnDefaultX
            moDest.y = mnDefaultY
            moDest.z = mnDefaultZ
        Else 'Default view should be the opponents view
            moDest.x = mnOpponentX
            moDest.y = mnOpponentY
            moDest.z = mnOpponentZ
        End If
    Case OverHeadView
        moDest.x = mnOverheadX
        moDest.y = mnOverheadY
        moDest.z = mnOverheadZ
    Case SideOverheadView1
        moDest.x = mnSide1X
        moDest.y = mnSide1Y
        moDest.z = mnSide1Z
    Case SideOverheadView2
        moDest.x = mnSide2X
        moDest.y = mnSide2Y
        moDest.z = mnSide2Z
    End Select
    'Set up a default velocity
    moVelocity.x = 3
    moVelocity.y = 3
    moVelocity.z = 3

End Sub

Private Sub Class_Initialize()
    CameraView = DefaultView
End Sub
