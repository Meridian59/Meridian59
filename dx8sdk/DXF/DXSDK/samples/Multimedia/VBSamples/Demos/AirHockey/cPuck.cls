VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "cPuck"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

Private Const mnMaxSpinSpeed As Single = 0.9
'Here we will encapsulate all of the code needed for the puck
'Local variables for the properties of the puck
Private moPosition As D3DVECTOR 'Current position of the puck
Private moVelocity As D3DVECTOR 'Current velocity of the puck
Private moLastPosition As D3DVECTOR 'Last position of the puck

Public Spinning As Boolean 'Is the puck currently spinning?
Public MaximumPuckVelocity As Single

Private mnSpinDir As Single 'Direction of the pucks spinning
Private mlPuckTime As Long 'Last time the puck was updated
Private mnPuckSpin As Single

Private moPuck As CD3DFrame 'D3D Mesh for the puck
'Default spin speed
Private mnDefaultSpin As Single

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
    'Update the velocity, but make sure it isn't too high
    EnsurePuckVelocityIsBelowMax
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

'Different methods from the puck.
Public Sub Init(ByVal sMedia As String, sFile As String)
    Set moPuck = D3DUtil_LoadFromFile(AddDirSep(sMedia) & sFile, Nothing, Nothing)
End Sub

Public Sub UpdatePosition()
    Dim RealVelocity As D3DVECTOR
    
    'Here we will update the position of the puck
    'and move it based on the velocity assigned.
    If mlPuckTime = 0 Then mlPuckTime = timeGetTime
    'First calculate the 'real' velocity (based on the time)
    RealVelocity.X = ((timeGetTime - mlPuckTime) / 100) * moVelocity.X
    RealVelocity.Y = ((timeGetTime - mlPuckTime) / 100) * moVelocity.Y
    RealVelocity.z = ((timeGetTime - mlPuckTime) / 100) * moVelocity.z
    'Let's save our current position
    moLastPosition = moPosition
        
    moPosition.X = moPosition.X + RealVelocity.X
    moPosition.Y = moPosition.Y + RealVelocity.Y
    moPosition.z = moPosition.z + RealVelocity.z
    
    If Spinning Then
        'Update Puck Spin
        mnPuckSpin = mnPuckSpin + ((((timeGetTime - mlPuckTime) / 100) * mnDefaultSpin) * mnSpinDir)
        If mnPuckSpin > 2 * g_pi Then mnPuckSpin = 0
    End If
    
    mlPuckTime = timeGetTime
End Sub

Public Sub Render(dev As Direct3DDevice8)
    Dim matRot As D3DMATRIX, matTrans As D3DMATRIX
    Dim matPuck As D3DMATRIX
    
    D3DXMatrixRotationAxis matRot, vec3(0, 1, 0), mnPuckSpin
    D3DXMatrixTranslation matTrans, moPosition.X, moPosition.Y, moPosition.z
    D3DXMatrixMultiply matPuck, matRot, matTrans
    
    moPuck.SetMatrix matPuck
    moPuck.Render dev

End Sub

Public Sub LaunchPuck()
    Randomize
    DefaultStartPosition
    Do While (D3DXVec3Length(moVelocity) < (MaximumPuckVelocity / 4)) And (Abs(moVelocity.z) < 0.2) 'Make sure there is *some* z movement
        moVelocity.z = Rnd * (MaximumPuckVelocity / 3)
        moVelocity.X = Rnd * (MaximumPuckVelocity / 3)
        If Rnd > 0.5 Then moVelocity.X = moVelocity.X * -1
        If Rnd < 0.5 Then moVelocity.z = moVelocity.z * -1
    Loop
End Sub

Public Sub DefaultStartPosition()
    moPosition = vec3(0, 2.5, 0)
    moVelocity = vec3(0, 0, 0)
    moLastPosition = vec3(0, 0, 0)
End Sub

Public Sub ChangePuckVelocity(oPaddle As cPaddle, oAudio As cAudio, Optional ByVal fIgnoreMax As Boolean = False)

    Dim vDir As D3DVECTOR
    Dim a As Single, b As Single, c As Single
    Dim t0 As Single, t1 As Single
    Dim vIntersect As D3DVECTOR, vIntersectHigh As D3DVECTOR
    Dim oPlane As D3DPLANE, matReflect As D3DMATRIX
    Dim oPoint As D3DVECTOR, vNewVelDir As D3DVECTOR
    Dim vPuck As D3DVECTOR, tSmall As Single
    Dim nVelocity As Single, nVelocityPaddle As Single
    Dim vNewVelPad As D3DVECTOR
  
    'We hit with the paddle, randomly change the spin direction
    UpdatePuckSpin
    glPaddleCollideTime = timeGetTime
    'gfRecentlyHitPaddle = True
    'Notify the user that the puck hit the paddle by playing a sound
    If Not (oAudio Is Nothing) Then oAudio.PlayHitSound
    'Let's store the original velocity
    nVelocity = D3DXVec3Length(moVelocity)
    nVelocityPaddle = D3DXVec3Length(oPaddle.Velocity) * gnPaddleMass
    'First we need to find the intersection point
    'To do that we first need to solve for t:
    'x = Dxt + x0
    'z = Dzt + z0
    D3DXVec3Subtract vPuck, moPosition, oPaddle.Position
    D3DXVec3Normalize vDir, moVelocity
    a = 1 ' (vDir.x ^ 2) + (vDir.z ^ 2) will always be one since the vector is normalized
    b = (2 * vPuck.X * vDir.X) + (2 * vPuck.z * vDir.z)
    c = ((vPuck.X ^ 2) + (vPuck.z ^ 2) - ((gnPaddleRadius + gnPuckRadius) ^ 2))
    't = (-b ± SQR(b²-4ac))/2a
    If (b ^ 2) - (4 * a * c) > 0 Then
        t0 = (-b + Sqr((b ^ 2) - (4 * a * c))) / (2 * a)
        t1 = (-b - Sqr((b ^ 2) - (4 * a * c))) / (2 * a)
    Else 'We shouldn't hit this case, but just in case.
        t0 = 0
        t1 = 0
    End If
    
    Dim vInt1 As D3DVECTOR, vInt2 As D3DVECTOR
    Dim vDifInt1 As D3DVECTOR, vDifInt2 As D3DVECTOR
    'Find both possible intersection points
    vInt1.X = (vDir.X * t0) + vPuck.X: vInt1.z = (vDir.z * t0) + vPuck.z
    vInt2.X = (vDir.X * t1) + vPuck.X: vInt2.z = (vDir.z * t1) + vPuck.z
    'Find the difference from the starting location
    D3DXVec3Subtract vDifInt1, oPaddle.Position, vInt1
    D3DXVec3Subtract vDifInt2, oPaddle.Position, vInt2
        
    'Find the smallest t
    'If t0 > t1 Then
    If D3DXVec3Length(vDifInt1) < D3DXVec3Length(vDifInt2) Then
        tSmall = t1
    Else
        tSmall = t0
    End If
    'Let's get the intersected point
    vIntersect.X = (vDir.X * tSmall) + vPuck.X
    vIntersect.z = (vDir.z * tSmall) + vPuck.z
    
    'Create a new vector with an enormously high Y field to create our reflection plane
    vIntersectHigh = vIntersect
    vIntersectHigh.Y = 500
    'Let's create a plane from this point
    D3DXPlaneFromPoints oPlane, vec3(0, 0, 0), vIntersect, vIntersectHigh
    
    'Now we can create a reflection matrix based on this plane
    D3DXMatrixReflect matReflect, oPlane
    'Create a new point that is reflected
    D3DXVec3TransformCoord oPoint, vPuck, matReflect
    D3DXVec3Subtract vNewVelDir, oPoint, vIntersect
    'Normalize the vector
    D3DXVec3Normalize vNewVelDir, vNewVelDir
    vNewVelDir.X = -vNewVelDir.X
    vNewVelDir.z = -vNewVelDir.z
    D3DXVec3Scale moVelocity, vNewVelDir, nVelocity
    If nVelocityPaddle > 0 Then 'The paddle is moving, add it's velocity
        'Now let's add the velocity of the paddle to our resulting velocity
        D3DXVec3Normalize vNewVelPad, oPaddle.Velocity
        D3DXVec3Scale vNewVelPad, vNewVelPad, nVelocityPaddle
        D3DXVec3Add moVelocity, moVelocity, vNewVelPad
    End If
    Debug.Print "Old Velocity:"; nVelocity; "  - New Velocity:"; D3DXVec3Length(moVelocity)
    'If we are limiting the velocity to it's maximum (most times), do so
    If Not fIgnoreMax Then EnsurePuckVelocityIsBelowMax
End Sub

Public Sub CheckCollisions(oPaddle() As cPaddle, Optional oAudio As cAudio = Nothing)
    'First we should check to see if we are scoring in this frame.
    Dim nDistance As Single
    Dim lCount As Long, fCollided As Boolean
    Dim lCollided As Long, nCollideDist As Single
    
    If gfScored Then Exit Sub
    'Check to see if the puck has collided with any of the walls
    'We could do an exhaustive check to see if any of the polygons collide, but since the table
    'is static, in the name of faster calculations, we will use a group of constants defining the
    'edges of the walls.  We will check those instead.
        
    'If the puck does hit one of the walls, we can easily calculate it's new direction by simply reversing
    'it's velocity (of that vector).  If we want to be even more accurate we can lower the velocity by a small amount as well
    
    'The left and right walls are bound to the X axis
    If moPosition.X > (gnSideLeftWallEdge - (gnPuckRadius)) Then
        'We hit the wall
        'Reverse the velocity of the X axis
        moVelocity = vec3((moVelocity.X * -1) * gnVelocityDamp, 0, moVelocity.z)
        moPosition = vec3((gnSideLeftWallEdge - (gnPuckRadius)), moPosition.Y, moPosition.z)
        If Not (oAudio Is Nothing) Then oAudio.PlayBankSound
        gfRecentlyHitPaddle = False
    ElseIf moPosition.X < (gnSideRightWallEdge + (gnPuckRadius)) Then
        'We hit the wall
        moVelocity = vec3((moVelocity.X * -1) * gnVelocityDamp, 0, moVelocity.z)
        moPosition = vec3((gnSideRightWallEdge + (gnPuckRadius)), moPosition.Y, moPosition.z)
        If Not (oAudio Is Nothing) Then oAudio.PlayBankSound
        gfRecentlyHitPaddle = False
    End If
    
    'The front and rear walls are count to the Z axis
    If moPosition.z > (gnNearWallEdge - (gnPuckRadius)) Then
        'Only reverse the velocity if we hit the sides of the 'scoring area'
        If (moPosition.X > (gnScoringEdgeLeft - (gnPuckRadius))) Or (moPosition.X < (gnScoringEdgeRight + (gnPuckRadius))) Then
            'We hit the wall
            'Reverse the velocity of the Z axis
            moVelocity = vec3(moVelocity.X, 0, (moVelocity.z * -1) * gnVelocityDamp)
            moPosition = vec3(moPosition.X, moPosition.Y, gnNearWallEdge - (gnPuckRadius))
            If Not (oAudio Is Nothing) Then oAudio.PlayBankSound
            gfRecentlyHitPaddle = False
        End If
    ElseIf moPosition.z < (gnFarWallEdge + (gnPuckRadius)) Then
        If (moPosition.X > (gnScoringEdgeLeft - (gnPuckRadius))) Or (moPosition.X < (gnScoringEdgeRight - (gnPuckRadius))) Then
            'We hit the wall
            moVelocity = vec3(moVelocity.X, 0, (moVelocity.z * -1) * gnVelocityDamp)
            moPosition = vec3(moPosition.X, moPosition.Y, gnFarWallEdge + (gnPuckRadius))
            If Not (oAudio Is Nothing) Then oAudio.PlayBankSound
            gfRecentlyHitPaddle = False
        End If
    End If
    
    'Next we should check to see if the puck has collided with either of the paddles
    'We will use a simple formula to determine if the puck has collided with one of the
    'paddles.  Simply put if the distance between the center of the puck, and the center
    'of the paddle in question is greater than the radius of the puck + the radius of the
    'paddle, they haven't collided
    Dim vecDif As D3DVECTOR
    
    If ((timeGetTime - glPaddleCollideTime) > glMinDelayPaddleHit) Or (Not gfRecentlyHitPaddle) Then
        gfRecentlyHitPaddle = False
        For lCount = 0 To 1 'Both paddles
            'We only check the X/Z coords because in this demo the puck will never leave the table
            'so it will maintain a constant Y coord.
            D3DXVec3Subtract vecDif, moPosition, oPaddle(lCount).Position
            nDistance = D3DXVec3Length(vecDif)
            If nDistance < (gnPaddleRadius + gnPuckRadius) Then 'They have collided
                nCollideDist = nDistance
                lCollided = lCount
                fCollided = True
                If gfMultiplayer Then
                    'Let each client handle it's own collision detection
                    'in a multiplayer game.  This balances the load between
                    'the host machine, and the client machine and gives the
                    'most realistic playing feel.
                    If glMyPaddleID = lCount Then 'We collided with our paddle
                        ChangePuckVelocity oPaddle(lCount), oAudio
                        SendPuck
                        SendCollidePaddle
                    End If
                Else
                    ChangePuckVelocity oPaddle(lCount), oAudio
                End If
            End If
        Next
    End If
    ' Make sure we aren't colliding anymore
    If fCollided Then EnsurePuckIsNotInPaddle nCollideDist, oPaddle(lCollided)
    'Lastly we should check if we have scored (on either side)
    If gfMultiplayer And (Not gfHost) Then Exit Sub 'Only the host should check for scoring
    If moPosition.z > (gnNearWallEdge) Then
        'We scored!
        goPuck.DropPuckIntoScoringPosition goAudio
    ElseIf moPosition.z < (gnFarWallEdge) Then
        'We scored!
        goPuck.DropPuckIntoScoringPosition goAudio
    End If
End Sub

Public Sub EnsurePuckIsNotInPaddle(ByVal nDistance As Single, oPaddle As cPaddle, Optional ByVal fSentPaddle As Boolean = False)
    'Move the paddle back out so it's not hitting the puck
    Dim vDir As D3DVECTOR, vScale As D3DVECTOR, vPaddleVel As D3DVECTOR

    If fSentPaddle Then
        D3DXVec3Subtract vPaddleVel, oPaddle.LastPosition, oPaddle.Position
        'Get the direction vector by normalizing the paddle's velocity
        D3DXVec3Normalize vDir, vPaddleVel
    Else
        'Get the direction vector by normalizing the pucks velocity
        D3DXVec3Normalize vDir, moVelocity
    End If
    'Scale the vector, just enough to get it out of the paddle
    D3DXVec3Scale vScale, vDir, (gnPuckRadius + gnPaddleRadius) - nDistance
    'Move the puck to it's new location
    D3DXVec3Add moPosition, moPosition, vScale
    'Now, let's increase the pucks velocity that much as well..
    If fSentPaddle Then D3DXVec3Add moVelocity, moVelocity, vScale
End Sub

Private Sub UpdatePuckSpin()
    Randomize
    If Rnd > 0.5 Then
        mnSpinDir = mnSpinDir * -1
        'Update the spin, change speed from 75%-125% of current speed..
        mnDefaultSpin = (Rnd * (mnSpinDir * 0.75)) + (mnSpinDir * 0.5)
        If Abs(mnDefaultSpin) > mnMaxSpinSpeed Then
            mnDefaultSpin = mnMaxSpinSpeed * (Abs(mnDefaultSpin) \ mnDefaultSpin)
        End If
    End If
End Sub

Public Sub CleanupFrame()
    moPuck.Destroy
    Set moPuck = Nothing
End Sub

Public Sub DropPuckIntoScoringPosition(oAudio As cAudio, Optional ByVal fFromReceive As Boolean = False)
    
    gfScored = True
    glTimeCompPaddle = 0
    If Not gfMultiplayer Then
        With goPaddle(1).Velocity
            .X = 0: .z = 0
        End With
    End If
    glTimePuckScored = timeGetTime
    oAudio.PlayScoreSound
    If gfMultiplayer Then
        If Not gfHost And Not fFromReceive Then Exit Sub
    End If
    'First stop the velocity
    moVelocity = vec3(0, 0, 0)
    
    With moPosition
        'Now position the puck
        If .z < 0 Then
            gPlayer(1).Score = gPlayer(1).Score + 1
            .z = gnFarWallEdge - 1.2
        ElseIf .z > 0 Then
            .z = gnNearWallEdge + 1.2
            gPlayer(0).Score = gPlayer(0).Score + 1
        End If
        If Abs(.X) > gnScoringEdgeLeft / 3 Then
            If Abs(.X) <> .X Then
                .X = gnScoringEdgeRight / 3
            Else
                .X = gnScoringEdgeLeft / 3
            End If
        End If
        .Y = gnPuckScored
    End With
    
    Spinning = False
    'If we are the host, notify everyone that we've scored
    If gfMultiplayer Then NotifyPlayersWeScored
    
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
    
    Set oMesh = moPuck.FindChildObject("puck", 0)
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

Public Sub PauseSystem(ByVal fPause As Boolean)
    If Not fPause Then
        mlPuckTime = timeGetTime
    End If
End Sub

'************
'Private functions that the public subs here will call, but the main application doesn't need to know about

Private Sub EnsurePuckVelocityIsBelowMax()
    Dim VelVec As D3DVECTOR
    'Let's make sure the puck's velocity isn't above the max,
    'and if it is, lower it to the max velocity
    If D3DXVec3Length(moVelocity) > MaximumPuckVelocity Then
        'Yup, lower the velocity to the max
        Dim vNrm As D3DVECTOR
        
        D3DXVec3Normalize vNrm, moVelocity
        D3DXVec3Scale VelVec, vNrm, MaximumPuckVelocity
        moVelocity = VelVec
    End If
End Sub

Private Sub Class_Initialize()
    mnSpinDir = 1
    mnDefaultSpin = 0.15
    Set moPuck = Nothing
    DefaultStartPosition
End Sub

Private Sub Class_Terminate()
    If Not moPuck Is Nothing Then moPuck.Destroy
    Set moPuck = Nothing
End Sub
