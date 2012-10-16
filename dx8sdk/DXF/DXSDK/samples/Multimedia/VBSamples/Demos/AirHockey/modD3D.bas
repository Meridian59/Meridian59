Attribute VB_Name = "modD3D"
Option Explicit

Public dx As New DirectX8

Public gfDrawRoomText As Boolean
Public glScreenWidth As Long
Public glScreenHeight As Long

'Extra misc vars
Public gfWireFrame As Boolean

Public Sub Render()

    Dim lCount As Long
    
    On Error Resume Next
    If g_dev Is Nothing Then Exit Sub

    ' Clear the backbuffer to a black color
    g_dev.Clear 0, ByVal 0, D3DCLEAR_TARGET Or D3DCLEAR_ZBUFFER, &H0&, 1#, 0
     
    ' Setup the world, view, and projection matrices
    SetupMatrices
 
    ' Begin the scene
    g_dev.BeginScene
    
    'Draw everything in either a solid fillmode, or wireframe
    If gfWireFrame Then
        g_dev.SetRenderState D3DRS_FILLMODE, D3DFILL_WIREFRAME
    Else
        g_dev.SetRenderState D3DRS_FILLMODE, D3DFILL_SOLID
    End If
    
    'Render the room
    goRoom.Render g_dev
    
    'Render the table
    goTable.Render g_dev
    
    'Now Paddle (0)
    goPaddle(0).Render g_dev
    
    'Now Paddle (1)
    goPaddle(1).Render g_dev
    
    'And finally the puck
    goPuck.Render g_dev
    'Now lets draw whatever text we need
    
    'We can draw text (don't draw text if we're currently fading)
    If Not goFade.AmFading Then
        goTextLittle.BeginText
        If gfGameCanBeStarted Then
            'If the game can be started, then draw the scores at the top of the screen
            If gfMultiplayer Then
                If glMyPaddleID = 0 Then
                    goTextLittle.DrawText gsUserName & ":" & glPlayerScore(0), 10, 5, &HFFFFFF00
                Else
                    goTextLittle.DrawText "Opponent:" & glPlayerScore(0), 10, 5, &HFFFFFFFF
                End If
                If glMyPaddleID = 1 Then
                    goTextLittle.DrawText gsUserName & ":" & glPlayerScore(1), glScreenWidth - 75, 5, &HFFFFFF00
                Else
                    goTextLittle.DrawText "Opponent:" & glPlayerScore(1), glScreenWidth - 75, 5, &HFFFFFFFF
                End If
            Else
                goTextLittle.DrawText "Player:" & glPlayerScore(0), 10, 5, &HFFFFFF00
                goTextLittle.DrawText "Computer:" & glPlayerScore(1), glScreenWidth - 75, 5, &HFFFFFFFF
            End If
        Else
            'The game can't be started yet (only in multiplayer)  Let the host know
            goTextLittle.DrawText "Waiting for the game to be started...", (glScreenWidth / 2) - 50, 5, &HFFFFFFFF
        End If
        'Here is a little helper text letting the user know to press Space
        'to launch the puck (will show up after 3 seconds, and stay on for 10 seconds)
        If (timeGetTime - glTimePuckScored > glDefaultDelayTime) And gfScored And Not gfGameOver And ((timeGetTime - glTimePuckScored < glDefaultDelayTimeGone + glDefaultDelayTime)) Then
            goPuck.DefaultStartPosition
            goPuck.Spinning = True
            goTextLittle.DrawText "Press <Space> to launch puck...", (glScreenWidth / 2) - 50, 25, &HFF0000FF
        End If
        
        'Here is a little helper text letting the user know to press F1
        'to turn of the room (will show up after 3 seconds, and stay on for 10 seconds)
        If (gfDrawRoomText And goRoom.DrawRoom) And (timeGetTime - glTimeNoRoom < glDefaultDelayTimeGone) Then
            goPuck.DefaultStartPosition
            goPuck.Spinning = True
            goTextLittle.DrawText "You can press F1 to turn off the drawing " & vbCrLf & " of the room, which will increase performance.", -15, glScreenHeight - 50, &HFFFF00FF
        End If
        If gfGameOver And ((timeGetTime - glTimeGameOver) > glDefaultDelayTime) And ((timeGetTime - glTimeGameOver < glDefaultDelayTimeGone + glDefaultDelayTime)) Then
            goTextLittle.DrawText "Press F3 to restart...", (glScreenWidth / 2) - 50, 25, &HFF0000FF
        End If
        goTextLittle.EndText
        goTextBig.BeginText
        If gfGameOver Then
            If gfMultiplayer Then
                If glPlayerScore(glMyPaddleID) > glPlayerScore(Abs(glMyPaddleID - 1)) Then
                    goTextBig.DrawText "Game over!!" & vbCrLf & "You win!!", (glScreenWidth / 2) - (glScreenWidth / 4), (glScreenHeight / 2) - (glScreenHeight / 4), &HFFDD11AA
                Else
                    goTextBig.DrawText "Game over!!" & vbCrLf & "You lose!!", (glScreenWidth / 2) - (glScreenWidth / 4), (glScreenHeight / 2) - (glScreenHeight / 4), &HFFDD11AA
                End If
            Else
                If glPlayerScore(0) > glPlayerScore(1) Then
                    goTextBig.DrawText "Game over!!" & vbCrLf & "You win!!", (glScreenWidth / 2) - (glScreenWidth / 4), (glScreenHeight / 2) - (glScreenHeight / 4), &HFFDD11AA
                Else
                    goTextBig.DrawText "Game over!!" & vbCrLf & "You lose!!", (glScreenWidth / 2) - (glScreenWidth / 4), (glScreenHeight / 2) - (glScreenHeight / 4), &HFFDD11AA
                End If
            End If
        End If
        If gfMultiplayer And gfNoSendData Then 'We've been disconnected sometime, notify the user
            goTextBig.DrawText "The connection with the other" & vbCrLf & "system was lost.", 5, (glScreenHeight / 2) - (glScreenHeight / 4), &HFFFFFF00
        End If
        goTextBig.EndText
    End If
    ' End the scene
    g_dev.EndScene
    ' Present the backbuffer contents to the front buffer (screen)
    D3DUtil_PresentAll 0
    
End Sub

Public Sub SetupMatrices()
    
    Dim matView As D3DMATRIX
        
    D3DXMatrixLookAtLH matView, goCamera.Position, vec3(0#, 0#, 0#), vec3(0#, 1#, 0#)
    g_dev.SetTransform D3DTS_VIEW, matView
    
    Dim matProj As D3DMATRIX
    D3DXMatrixPerspectiveFovLH matProj, g_pi / 4, 1, 1, 110
    g_dev.SetTransform D3DTS_PROJECTION, matProj

End Sub

Public Sub RestoreDeviceObjects()

    ' Set miscellaneous render states
    With g_dev
        ' Set world transform
        Dim matWorld As D3DMATRIX
        D3DXMatrixIdentity matWorld
        .SetTransform D3DTS_WORLD, matWorld
        ' Set the projection matrix
        Dim matProj As D3DMATRIX
        Dim fAspect As Single
        fAspect = 1
        D3DXMatrixPerspectiveFovLH matProj, g_pi / 3, fAspect, 1, 1000
        .SetTransform D3DTS_PROJECTION, matProj
        
        .SetTextureStageState 0, D3DTSS_COLORARG1, D3DTA_TEXTURE
        .SetTextureStageState 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE
        .SetTextureStageState 0, D3DTSS_COLOROP, D3DTOP_MODULATE
        .SetTextureStageState 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR
        .SetTextureStageState 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR
        .SetTextureStageState 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR
        .SetTextureStageState 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR

        ' Set default render states
        .SetRenderState D3DRS_DITHERENABLE, 1 'True
        .SetRenderState D3DRS_SPECULARENABLE, 0 'False
        .SetRenderState D3DRS_ZENABLE, 1 'True
        .SetRenderState D3DRS_NORMALIZENORMALS, 1 'True

    End With
    
    Set goTextBig = Nothing
    Set goTextLittle = Nothing
    
    'Now create a new text object
    Set goTextLittle = New cText
    goTextLittle.InitText g_d3dx, g_dev, "Times New Roman", 8, True
    Set goTextBig = New cText
    goTextBig.InitText g_d3dx, g_dev, "Times New Roman", 18, True
    
End Sub

Public Sub InitDefaultLights(Optional ByVal lNumLights As Long = 2, Optional ByVal fFullAmbiantOnly As Boolean = False)
    With g_dev
        ' Set ambient light
        .SetRenderState D3DRS_AMBIENT, &HFFFFFFFF
        goFade.AmbientColor = &HFFFFFFFF
        goFade.MaxAmbientColor = goFade.AmbientColor
        If fFullAmbiantOnly Then Exit Sub
        If lNumLights < 1 Then Exit Sub 'Nothing to do
        
        ' Set ambient light
        'We will slowly lower the ambient light as each new light gets added
        .SetRenderState D3DRS_AMBIENT, &HFFBBBBBB
        goFade.AmbientColor = &HFFBBBBBB
        ' Turn on lighting
        .SetRenderState D3DRS_LIGHTING, 1
        
        'Turn on two lights one on each end of the table
        Dim light As D3DLIGHT8
        
        If lNumLights > 0 Then
            With light
                .Type = D3DLIGHT_DIRECTIONAL
                .diffuse.r = 0
                .diffuse.g = 0
                .diffuse.b = 0
                .Direction.x = 0
                .Direction.y = -10
                .Direction.z = 0
                .Range = 100000#
                .Position.x = 0
                .Position.y = 3
                .Position.z = 0
            End With
            goFade.LightColor(0) = light
    
            .SetLight 0, light                   'let d3d know about the light
            .LightEnable 0, 1                    'turn it on
            goFade.NumLight = 0
        End If
        
        
        If lNumLights > 1 Then
            .SetRenderState D3DRS_AMBIENT, &HFFAAAAAA
            goFade.AmbientColor = &HFFAAAAAA
            'Now turn on the second light if we can
            With light
                .Type = D3DLIGHT_DIRECTIONAL
                .Direction.x = 15
                .Direction.y = -10
                .Direction.z = -15
                .Range = 1000#
                .Position.x = -15
                .Position.y = 10
                .Position.z = 15
            End With
            goFade.LightColor(1) = light
    
            .SetLight 1, light                   'let d3d know about the light
            .LightEnable 1, 1                    'turn it on
            goFade.NumLight = 1
        End If

        
        If lNumLights > 3 Then
            .SetRenderState D3DRS_AMBIENT, 0
            goFade.AmbientColor = 0
            'Now turn on the third light if we can
            With light
                .Type = D3DLIGHT_DIRECTIONAL
                .Direction.x = -15
                .Direction.y = 10
                .Direction.z = 15
                .Range = 1000#
                .Position.x = 15
                .Position.y = -10
                .Position.z = -15
            End With
            goFade.LightColor(2) = light
    
            .SetLight 2, light                   'let d3d know about the light
            .LightEnable 2, 1                    'turn it on
            goFade.NumLight = 2
        End If
    End With
    goFade.MaxAmbientColor = goFade.AmbientColor

End Sub
