VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "cPaddle"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

'Here we will encapsulate all of the code needed for the Paddle
'Local variables for the properties of the Paddle
Private moPosition As D3DVECTOR 'Current position of the Paddle
Private moVelocity As D3DVECTOR 'Current velocity of the Paddle
Private moLastPosition As D3DVECTOR 'Last position of the Paddle
Private moPaddle As CD3DFrame 'D3D Mesh for the Paddle
Private mlPaddleTime As Long 'Last time the Paddle was updated
Private mlTransparantPaddle As Boolean
Public LastVelocityTick As Long 'Last time the paddle's velocity changed
Public PaddleID As Long

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

'Transparent property
Public Property Let Transparent(ByVal fTrans As Boolean)
    Dim oMesh As CD3DMesh, oMaterial As D3DMATERIAL8
    Dim lNumMaterial As Long, lCount As Long
    
    mlTransparantPaddle = fTrans
    'now set the property
    Set oMesh = moPaddle.FindChildObject("paddle", 0)
    lNumMaterial = oMesh.GetMaterialCount
    For lCount = 0 To lNumMaterial - 1
        oMaterial = oMesh.GetMaterial(lCount)
        If fTrans Then
            oMaterial.diffuse.a = 0.5
        Else
            oMaterial.diffuse.a = 1
        End If
        oMesh.SetMaterial lCount, oMaterial
    Next
End Property

Public Property Get Transparent() As Boolean
    Transparent = mlTransparantPaddle
End Property

'Methods
Public Sub Init(ByVal sMedia As String, sFile As String)
    Set moPaddle = D3DUtil_LoadFromFile(AddDirSep(sMedia) & sFile, Nothing, Nothing)
End Sub

Public Sub UpdateTime()
    mlPaddleTime = timeGetTime
End Sub

Public Sub UpdatePosition()
    Dim RealVelocity As D3DVECTOR
    
    'Here we will update the position of the paddle
    'and move it based on the velocity assigned.
    If mlPaddleTime = 0 Then mlPaddleTime = timeGetTime
    'First calculate the 'real' velocity (based on the time)
    RealVelocity.X = ((timeGetTime - mlPaddleTime) / 100) * moVelocity.X
    RealVelocity.Y = ((timeGetTime - mlPaddleTime) / 100) * moVelocity.Y
    RealVelocity.z = ((timeGetTime - mlPaddleTime) / 100) * moVelocity.z
    'Let's save our current position
    moLastPosition = moPosition
        
    moPosition.X = moPosition.X + RealVelocity.X
    moPosition.Y = moPosition.Y + RealVelocity.Y
    moPosition.z = moPosition.z + RealVelocity.z
    
    mlPaddleTime = timeGetTime
End Sub

Public Sub Render(dev As Direct3DDevice8)
    Dim matPaddle As D3DMATRIX
    
    D3DXMatrixIdentity matPaddle
    D3DXMatrixTranslation matPaddle, moPosition.X, moPosition.Y, moPosition.z
    moPaddle.SetMatrix matPaddle
    moPaddle.Render dev
End Sub

Public Sub CleanupFrame()
    Set moPaddle = Nothing
End Sub

Public Sub EnsureReality(oldPos As D3DVECTOR, oPuck As cPuck)
    Dim vecDif As D3DVECTOR, nDistance As Single
    Dim vNewVel As D3DVECTOR, nVel As Single
    Dim fMovePaddle As Boolean
    
    'We do *not* want to go 'inside' the puck, don't let it happen
    D3DXVec3Subtract vecDif, oPuck.Position, moPosition
    nDistance = D3DXVec3Length(vecDif)
    If nDistance < (gnPuckRadius + gnPaddleRadius) Then
        'Ok, we are within the puck, now who should move?  The puck or the paddle?
        With moPosition
            fMovePaddle = False
            If .z < (gnFarWallEdge + (gnPaddleRadius + gnPuckRadius)) Then
                fMovePaddle = True
            End If
            If .z > (gnNearWallEdge - (gnPaddleRadius + gnPuckRadius)) Then
                fMovePaddle = True
            End If
            If .X < (gnSideRightWallEdge + (gnPaddleRadius + gnPuckRadius)) Then
                fMovePaddle = True
            End If
            If .X > (gnSideLeftWallEdge - (gnPaddleRadius + gnPuckRadius)) Then
                fMovePaddle = True
            End If
        End With
        
        If fMovePaddle Then
            'Move the paddle back out so it's not hitting the puck
            Dim vDir As D3DVECTOR, vScale As D3DVECTOR, vPaddleVel As D3DVECTOR
            Dim vPaddleDif As D3DVECTOR
    
            D3DXVec3Subtract vPaddleDif, oPuck.Position, moPosition
            D3DXVec3Subtract vPaddleVel, oldPos, moPosition
            'Get the direction vector by normalizing the pucks velocity
            D3DXVec3Normalize vDir, vPaddleVel
            'Scale the length of the two vectors, plus a little more.
            D3DXVec3Scale vScale, vDir, D3DXVec3Length(vPaddleDif) '(gnPaddleRadius / 4)
            'Move the paddle to it's new location
            D3DXVec3Add moPosition, oldPos, vScale
        'Else We can ignore the case of the puck needing to move because that will
              'happen in checkcollisions call for the puck
        End If
    End If
End Sub

Public Sub DoComputerAI(oPuck As cPuck)

    Dim vOldPos As D3DVECTOR
    Dim nTempX As Single, nTempZ As Single
    
    'We'll create a simplistic AI opponent
    vOldPos = moPosition
    'Let's just set the velocity of the paddle
    moLastPosition = moPosition
    With moPosition
        If Abs(oPuck.Position.X > .X) Then
            nTempX = Min(oPuck.Velocity.X, gnComputerMaximumVelocity)
        Else
            nTempX = Min(oPuck.Velocity.X, -gnComputerMaximumVelocity)
        End If
        If Abs(oPuck.Position.z - .z) > Abs(oPuck.LastPosition.z - .z) Then
            nTempZ = gnComputerMaximumVelocity
        Else
            nTempZ = -gnComputerMaximumVelocity
        End If
    End With
    moVelocity = vec3(nTempX, 0, nTempZ)
    'If the puck is in *front* of the paddle, just move the paddle directly backwards
    If moPosition.z < oPuck.Position.z Then
        moVelocity = vec3(0, 0, gnComputerMaximumVelocity)
    End If
    UpdatePosition
    EnsureReality vOldPos, oPuck
    
    With moPosition
        nTempX = .X
        nTempZ = .z
        If PaddleID = 0 Then
            If nTempZ > -(gnPaddleRadius * 1.5) Then
                nTempZ = -(gnPaddleRadius * 1.5)
            ElseIf nTempZ < (gnFarWallEdge + (gnPaddleRadius)) Then
                nTempZ = (gnFarWallEdge + (gnPaddleRadius))
            End If
        Else
            If nTempZ > (gnNearWallEdge - (gnPaddleRadius)) Then
                nTempZ = (gnNearWallEdge - (gnPaddleRadius))
            ElseIf nTempZ < (gnPaddleRadius * 1.5) Then
                nTempZ = (gnPaddleRadius * 1.5)
            End If
        End If
        If nTempX < (gnSideRightWallEdge + (gnPaddleRadius)) Then
            nTempX = (gnSideRightWallEdge + (gnPaddleRadius))
        End If
        If nTempX > (gnSideLeftWallEdge - (gnPaddleRadius)) Then
            nTempX = (gnSideLeftWallEdge - (gnPaddleRadius))
        End If
        moPosition = vec3(nTempX, moPosition.Y, nTempZ)
    End With
End Sub

Public Function FadeMesh(FadeInterval As Single) As Boolean
    Dim lNumMaterial As Long
    Dim lCount As Long
    Dim oMaterial As D3DMATERIAL8
    Dim fDoneFading As Boolean
    Dim oMesh As CD3DMesh
    Dim nInternalInterval As Single
    Static lFadeTime As Long
    
    nInternalInterval = FadeInterval
    If lFadeTime = 0 Then
        lFadeTime = timeGetTime
        Exit Function 'We'll do the fade next render pass
    End If
    nInternalInterval = (((timeGetTime - lFadeTime) / 1000000) * nInternalInterval)
    
    Set oMesh = moPaddle.FindChildObject("paddle", 0)
    fDoneFading = True
    lNumMaterial = oMesh.GetMaterialCount
    For lCount = 0 To lNumMaterial - 1
        oMaterial = oMesh.GetMaterial(lCount)
        If nInternalInterval > 0 And oMaterial.diffuse.a <= 1 Then
            oMaterial.diffuse.a = oMaterial.diffuse.a + nInternalInterval
            fDoneFading = False
        ElseIf nInternalInterval < 0 And oMaterial.diffuse.a >= -1 Then
            oMaterial.diffuse.a = oMaterial.diffuse.a + nInternalInterval
            fDoneFading = False
        End If
        oMesh.SetMaterial lCount, oMaterial
    Next
    FadeMesh = fDoneFading
End Function

Private Sub Class_Initialize()
    Set moPaddle = Nothing
End Sub

Private Sub Class_Terminate()
    Set moPaddle = Nothing
End Sub

Private Function Min(ByVal nVal As Single, nVal2 As Single) As Single
    If Abs(nVal) < Abs(nVal2) Then
        Min = nVal
    Else
        Min = nVal2
    End If
End Function

