VERSION 5.00
Object = "{1F6AF2BA-798F-4586-8F76-CD0DB05515D9}#1.0#0"; "vb_SubClass.ocx"
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   4290
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   5580
   LinkTopic       =   "Form1"
   ScaleHeight     =   286
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   372
   StartUpPosition =   3  'Windows Default
   Begin vbSubClass.SubClasser oSubClass 
      Left            =   120
      Top             =   3720
      _ExtentX        =   873
      _ExtentY        =   873
   End
   Begin VB.PictureBox Picture1 
      Height          =   3615
      Left            =   0
      ScaleHeight     =   3555
      ScaleWidth      =   4635
      TabIndex        =   0
      Top             =   0
      Width           =   4695
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
'-----------------------------------------------------------------------------
' File: Dolphin.cpp
'
' Desc: Sample of swimming dolphin
'
'       Note: This code uses the D3D Framework helper library.
'
'
' Copyright (C) 1999-2001 Microsoft Corporation. All rights reserved.
'-----------------------------------------------------------------------------


Option Explicit


'-----------------------------------------------------------------------------
' Globals variables and definitions
'-----------------------------------------------------------------------------
Private Const WM_POWERBROADCAST = &H218
Private Const PBT_APMQUERYSUSPEND = 0
Private Const PBT_APMRESUMESUSPEND = &H7
Private Const PBT_APMQUERYSTANDBY = &H1
Private Const PBT_APMRESUMESTANDBY = &H8
Private Const PBT_APMBATTERYLOW = &H9

Const WATER_COLOR = &H6688&
Const AMBIENT_COLOR = &H33333333
Const kMesh1 = 0
Const kMesh2 = 1
Const kMesh3 = 2

Private Type DOLPHINVERTEX
    p As D3DVECTOR
    n As D3DVECTOR
    tu As Single
    tv As Single
End Type
Const VertexFVF& = D3DFVF_XYZ Or D3DFVF_NORMAL Or D3DFVF_TEX1

Private Type MESHTOOL
    VertB As Direct3DVertexBuffer8
    NumVertices As Long
    Vertices() As DOLPHINVERTEX
End Type

'Dolphin objects
Dim m_DolphinGroupObject As CD3DFrame
Dim m_DolphinObject As CD3DFrame
Dim m_DolphinMesh As CD3DMesh
Dim m_DolphinMesh01 As CD3DMesh
Dim m_DolphinMesh02 As CD3DMesh
Dim m_DolphinMesh03 As CD3DMesh
Dim m_DolphinTex As Direct3DTexture8

'Seafloor objects
Dim m_FloorObject As CD3DFrame
Dim m_SeaFloorMesh As CD3DMesh

Dim m_meshtool(3) As MESHTOOL
Dim m_dest As MESHTOOL

'Textures for the water caustics
Dim m_CausticTextures() As Direct3DTexture8
Dim m_CurrentCausticTexture As Direct3DTexture8
Dim m_media As String

Dim g_ftime As Single
Dim mfNotSuspended As Boolean


'-----------------------------------------------------------------------------
' Name: Form_Load()
' Desc:
'-----------------------------------------------------------------------------
Private Sub Form_Load()
    Me.Show
    DoEvents
    oSubClass.Hook Me.hWnd
    'setup defaults
    Init
    
    'setup d3d
    D3DUtil_DefaultInitWindowed 0, Picture1.hWnd
    m_media = FindMediaDir("dolphin_group.x")
    D3DUtil_SetMediaPath m_media
    
    InitDeviceObjects
    RestoreDeviceObjects
    
    DXUtil_Timer TIMER_START
    mfNotSuspended = True
    Do While mfNotSuspended
        FrameMove
        Render
        D3DUtil_PresentAll 0
        DoEvents
    Loop
    
End Sub

Private Sub Form_Resize()
    Picture1.width = Me.ScaleWidth
    Picture1.height = Me.ScaleHeight
End Sub

'-----------------------------------------------------------------------------
' Name: Form_Unload()
' Desc:
'-----------------------------------------------------------------------------
Private Sub Form_Unload(Cancel As Integer)
    DeleteDeviceObjects
    oSubClass.UnHook
    End
End Sub


Private Sub MESHTOOL_INIT(mt As MESHTOOL, m As D3DXMesh)
    Set mt.VertB = m.GetVertexBuffer
    mt.NumVertices = m.GetNumVertices
    ReDim mt.Vertices(mt.NumVertices)
    D3DVertexBuffer8GetData mt.VertB, 0, mt.NumVertices * Len(mt.Vertices(0)), 0, mt.Vertices(0)
End Sub
    
Private Sub MESHTOOL_DESTROY(mt As MESHTOOL)
   Set mt.VertB = Nothing
   ReDim mt.Vertices(0)
End Sub

Function FtoDW(f As Single) As Long
    Dim buf As D3DXBuffer
    Dim ret As Long
    Set buf = g_d3dx.CreateBuffer(4)
    g_d3dx.BufferSetData buf, 0, 4, 1, f
    g_d3dx.BufferGetData buf, 0, 4, 1, ret
    Set buf = Nothing
    
    FtoDW = ret
End Function


'-----------------------------------------------------------------------------
' Name: Init()
' Desc: Constructor
'-----------------------------------------------------------------------------
Sub Init()
    Me.Caption = "D3D Dolphin with Power Management Features"
    ReDim m_CausticTextures(32)
End Sub


'-----------------------------------------------------------------------------
' Name: OneTimeSceneInit()
' Desc: Called during initial app startup, this function performs all the
'       permanent initialization.
'-----------------------------------------------------------------------------
Sub OneTimeSceneInit()

End Sub



'-----------------------------------------------------------------------------
' Name: BlendMeshes()
' Desc: Does a linear interpolation between all vertex positions and normals
'       in two source meshes and outputs the result to the destination mesh.
'       Note: all meshes must contain the same number of vertices, and the
'       destination mesh must be in device memory.
'-----------------------------------------------------------------------------
Sub BlendMeshes(ByVal fWeight As Single)
    
    Dim fWeight1 As Single, fWeight2 As Single
    Dim vTemp1 As D3DVECTOR, vTemp2 As D3DVECTOR
    Dim i As Long, j As Long
    
    If (fWeight < 0) Then
        j = kMesh3
    Else
        j = kMesh1
    End If
     
    
    ' compute blending factors
    fWeight1 = fWeight
    If fWeight < 0 Then fWeight1 = -fWeight1
    fWeight2 = 1 - fWeight1

    ' Linearly Interpolate (LERP)positions and normals
    For i = 0 To m_dest.NumVertices - 1
        D3DXVec3Scale vTemp1, m_meshtool(kMesh2).Vertices(i).p, fWeight2
        D3DXVec3Scale vTemp2, m_meshtool(j).Vertices(i).p, fWeight1
        D3DXVec3Add m_dest.Vertices(i).p, vTemp1, vTemp2
        
        D3DXVec3Scale vTemp1, m_meshtool(kMesh2).Vertices(i).n, fWeight2
        D3DXVec3Scale vTemp2, m_meshtool(j).Vertices(i).n, fWeight1
        D3DXVec3Add m_dest.Vertices(i).n, vTemp1, vTemp2
    Next

    'Set the data
    D3DVertexBuffer8SetData m_dest.VertB, 0, m_dest.NumVertices * Len(m_dest.Vertices(0)), 0, m_dest.Vertices(0)
    
End Sub



'-----------------------------------------------------------------------------
' Name: FrameMove()
' Desc: Called once per frame, the call is the entry point for animating
'       the scene.
'-----------------------------------------------------------------------------

Sub FrameMove()
    g_ftime = DXUtil_Timer(TIMER_GETAPPTIME) * 0.9
    
    Dim fKickFreq As Single, fPhase As Single, fBlendWeight As Single
    
    fKickFreq = g_ftime * 2
    fPhase = g_ftime / 3
    fBlendWeight = Sin(fKickFreq)


    ' Blend the meshes (which makes the dolphin appear to swim)
    Call BlendMeshes(fBlendWeight)

    ' Move the dolphin in a circle and have it undulate
    Dim vTrans As D3DVECTOR
    Dim qRot As D3DQUATERNION
    Dim matDolphin As D3DMATRIX
    Dim matTrans As D3DMATRIX, matRotate1 As D3DMATRIX, matRotate2 As D3DMATRIX
    
    'Scale dolphin geometery to 1/100 original
    D3DXMatrixScaling matDolphin, 0.01, 0.01, 0.01
    
    'add up and down roation (since modeled along x axis)
    D3DXMatrixRotationZ matRotate1, -Cos(fKickFreq) / 6
    D3DXMatrixMultiply matDolphin, matDolphin, matRotate1
    
    'add rotation to make dolphin point at tangent to the circle
    D3DXMatrixRotationY matRotate2, fPhase
    D3DXMatrixMultiply matDolphin, matDolphin, matRotate2
    
    
    
    'add traslation to make the dolphin move in a circle and bob up and down
    'in sync with its flippers
    D3DXMatrixTranslation matTrans, -5 * Sin(fPhase), Sin(fKickFreq) / 2, 10 - 10 * Cos(fPhase)
    
    'D3DXMatrixTranslation matTrans, 0, Sin(fKickFreq) / 2, 0
    D3DXMatrixMultiply matDolphin, matDolphin, matTrans
    
    
    m_DolphinObject.SetMatrix matDolphin
    
    ' Animate the caustic textures
    Dim tex As Long
    tex = CLng((g_ftime * 32)) Mod 32
    Set m_CurrentCausticTexture = m_CausticTextures(tex)

End Sub


'-----------------------------------------------------------------------------
' Name: Render()
' Desc: Called once per frame, the call is the entry point for 3d
'       rendering. This function sets up render states, clears the
'       viewport, and renders the scene.
'-----------------------------------------------------------------------------
Sub Render()

    Dim mat As D3DMATRIX
    Dim mat2 As D3DMATRIX
    Dim hr As Long
    
    
    hr = g_dev.TestCooperativeLevel()
    If hr = D3DERR_DEVICELOST Then
        
        'If the device is lost, exit and wait for it to come back.
        Exit Sub
    
    ElseIf hr = D3DERR_DEVICENOTRESET Then
            
        'The device became lost for some reason (probably an alt-tab) and now
        'Reset() needs to be called to try and get the device back.
        g_dev.Reset g_d3dpp
                
        'Restore Device objects
        RestoreDeviceObjects
        
    
    End If
    
    'Make sure the app isn't minimized.
    If Me.WindowState = vbMinimized Then Exit Sub

    
    
    ' Clear the backbuffer
    D3DUtil_ClearAll WATER_COLOR
    
    
    With g_dev
        .BeginScene
                
        
        ' Render the Seafloor. For devices that support one-pass multi-
        ' texturing, use the second texture stage to blend in the animated
        ' water caustics texture.
        If (g_d3dcaps.MaxTextureBlendStages > 1) Then

            ' Set up the 2nd texture stage for the animated water caustics
            .SetTexture 1, m_CurrentCausticTexture
            .SetTextureStageState 1, D3DTSS_COLOROP, D3DTOP_MODULATE
            .SetTextureStageState 1, D3DTSS_COLORARG1, D3DTA_TEXTURE
            .SetTextureStageState 1, D3DTSS_COLORARG2, D3DTA_CURRENT

            ' Tell D3D to automatically generate texture coordinates from the
            ' model's position in camera space. The texture transform matrix is
            ' setup so that the 'x' and 'z' coordinates are scaled to become the
            ' resulting 'tu' and 'tv' texture coordinates. The resulting effect
            ' is that the caustic texture is draped over the geometry from above.

            mat.m11 = 0.05:           mat.m12 = 0#
            mat.m21 = 0#:             mat.m22 = 0#
            mat.m31 = 0#:             mat.m32 = 0.05
            mat.m41 = Sin(g_ftime) / 8: mat.m42 = (Cos(g_ftime) / 10) - (g_ftime / 10)

            .SetTransform D3DTS_TEXTURE1, mat
            .SetTextureStageState 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION
            .SetTextureStageState 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2
        End If
        g_dev.SetRenderState D3DRS_AMBIENT, &HB0B0B0B0
        
        
        ' Finally, render the actual seafloor with the above states
        m_FloorObject.Render g_dev
        
        
        ' Disable the second texture stage
        If (g_d3dcaps.MaxTextureBlendStages > 1) Then
            .SetTextureStageState 1, D3DTSS_COLOROP, D3DTOP_DISABLE
        End If

        ' Render the dolphin in it's first pass.
        .SetRenderState D3DRS_AMBIENT, AMBIENT_COLOR
        m_DolphinObject.Render g_dev



        ' For devices that support one-pass multi-texturing, use the second
        ' texture stage to blend in the animated water caustics texture for
        ' the dolphin. This a little tricky because we only want caustics on
        ' the part of the dolphin that is lit from above. To acheive this
        ' effect, the dolphin is rendered alpha-blended with a second pass
        ' which has the caustic effects modulating the diffuse component
        '  which contains lighting-only information) of the geometry.
        If (g_d3dcaps.MaxTextureBlendStages > 1) Then

            ' For the 2nd pass of rendering the dolphin, turn on the caustic
            ' effects. Start with setting up the 2nd texture stage state, which
            ' will modulate the texture with the diffuse component. This actually
            ' only needs one stage, except that using a CD3DFile object makes that
            ' a little tricky.
            .SetTexture 1, m_CurrentCausticTexture
            .SetTextureStageState 1, D3DTSS_COLOROP, D3DTOP_MODULATE
            .SetTextureStageState 1, D3DTSS_COLORARG1, D3DTA_TEXTURE
            .SetTextureStageState 1, D3DTSS_COLORARG2, D3DTA_DIFFUSE


            ' Now, set up D3D to generate texture coodinates. This is the same as
            ' with the seafloor  the 'x' and 'z' position coordinates in camera
            ' space are used to generate the 'tu' and 'tv' texture coordinates),
            ' except our scaling factors are different in the texture matrix, to
            ' get a better looking result.

            mat2.m11 = 0.5: mat2.m12 = 0#
            mat2.m21 = 0#: mat2.m22 = 0#
            mat2.m31 = 0#: mat2.m32 = 0.5
            mat2.m41 = 0#: mat2.m42 = 0#

            .SetTransform D3DTS_TEXTURE1, mat2
            .SetTextureStageState 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION
            .SetTextureStageState 1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2

            ' Set the ambient color and fog color to pure black. Ambient is set
            ' to black because we still have a light shining from above, but we
            ' don't want any caustic effects on the dolphin's underbelly. Fog is
            ' set to black because we want the caustic effects to fade out in the
            ' distance just as the model does with the WATER_COLOR.
            .SetRenderState D3DRS_AMBIENT, &H0&
            .SetRenderState D3DRS_FOGCOLOR, &H0&

            ' Set up blending modes to add this caustics-only pass with the
            ' previous pass.
            .SetRenderState D3DRS_ALPHABLENDENABLE, 1 ' True
            .SetRenderState D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR
            .SetRenderState D3DRS_DESTBLEND, D3DBLEND_ONE

            ' Finally, render the caustic effects for the dolphin
            m_DolphinObject.Render g_dev

            ' After all is well and done, restore any munged texture stage states
            .SetTextureStageState 1, D3DTSS_COLOROP, D3DTOP_DISABLE
            .SetRenderState D3DRS_AMBIENT, AMBIENT_COLOR
            .SetRenderState D3DRS_FOGCOLOR, WATER_COLOR
            .SetRenderState D3DRS_ALPHABLENDENABLE, 0 'False

        End If

skipcaustic:

        ' End the scene.
        .EndScene
    End With

End Sub



'-----------------------------------------------------------------------------
' Name: InitDeviceObjects()
' Desc: Initialize scene objects.
'-----------------------------------------------------------------------------
Function InitDeviceObjects() As Boolean
    
    Dim b As Boolean
    Dim t As Long
    Dim strName As String
    Dim i As Long
    'Set up textures for the water caustics
    
    For t = 0 To 31
        strName = m_media + "Caust" + Format$(t, "00") + ".tga"
        Set m_CausticTextures(t) = D3DUtil_CreateTexture(g_dev, strName, D3DFMT_UNKNOWN)
        If m_CausticTextures(t) Is Nothing Then
            Debug.Print "Unable to find media " + strName
            'End
        End If
    Next

    ' Load the file-based mesh objects
    Set m_DolphinGroupObject = D3DUtil_LoadFromFile(m_media + "dolphin_group.x", Nothing, Nothing)
    Set m_DolphinObject = D3DUtil_LoadFromFile(m_media + "dolphin.x", Nothing, Nothing)
    Set m_FloorObject = D3DUtil_LoadFromFile(m_media + "seafloor.x", Nothing, Nothing)
    
    
    ' // Gain access to the meshes
    Set m_DolphinMesh01 = m_DolphinGroupObject.FindChildObject("Dolph01", 0)
    Set m_DolphinMesh02 = m_DolphinGroupObject.FindChildObject("Dolph02", 0)
    Set m_DolphinMesh03 = m_DolphinGroupObject.FindChildObject("Dolph03", 0)
    Set m_DolphinMesh = m_DolphinObject.FindChildObject("Dolph02", 0).GetChildMesh(0)
    Set m_SeaFloorMesh = m_FloorObject.FindChildObject("SeaFloor", 0)

    ' Set the FVF type to something useful
    Call m_DolphinMesh01.SetFVF(g_dev, VertexFVF)
    Call m_DolphinMesh02.SetFVF(g_dev, VertexFVF)
    Call m_DolphinMesh03.SetFVF(g_dev, VertexFVF)
    Call m_DolphinMesh.SetFVF(g_dev, VertexFVF)
    Call m_SeaFloorMesh.SetFVF(g_dev, VertexFVF)
    
    
    Set m_DolphinTex = D3DUtil_CreateTexture(g_dev, m_media + "dolphin.bmp", D3DFMT_UNKNOWN)

    ' Scale the sea floor vertices, and add some bumpiness
    Dim seafloortool As MESHTOOL
    MESHTOOL_INIT seafloortool, m_SeaFloorMesh.mesh
    For i = 0 To seafloortool.NumVertices - 1
       seafloortool.Vertices(i).p.y = seafloortool.Vertices(i).p.y + Rnd(1) + Rnd(1) + Rnd(1)
       seafloortool.Vertices(i).tu = seafloortool.Vertices(i).tu * 10
       seafloortool.Vertices(i).tv = seafloortool.Vertices(i).tv * 10
    Next
    D3DVertexBuffer8SetData seafloortool.VertB, 0, Len(seafloortool.Vertices(0)) * seafloortool.NumVertices, 0, seafloortool.Vertices(0)
    MESHTOOL_DESTROY seafloortool

    MESHTOOL_INIT m_meshtool(kMesh1), m_DolphinMesh01.mesh
    MESHTOOL_INIT m_meshtool(kMesh2), m_DolphinMesh02.mesh
    MESHTOOL_INIT m_meshtool(kMesh3), m_DolphinMesh03.mesh
    MESHTOOL_INIT m_dest, m_DolphinMesh.mesh
    
    InitDeviceObjects = True
        
End Function

    







'-----------------------------------------------------------------------------
' Name: RestoreDeviceObjects()
' Desc: Restore device-memory objects and state after a device is created or
'       resized.
'-----------------------------------------------------------------------------
Sub RestoreDeviceObjects()

    m_DolphinGroupObject.RestoreDeviceObjects g_dev
    m_DolphinObject.RestoreDeviceObjects g_dev
    m_FloorObject.RestoreDeviceObjects g_dev
    
    
    ' Set miscellaneous render states
    With g_dev
 
        
        ' Set world transform
        Dim matWorld As D3DMATRIX
        D3DXMatrixIdentity matWorld
        .SetTransform D3DTS_WORLD, matWorld


       ' Set the app view matrix for normal viewing
        Dim vEyePt As D3DVECTOR, vLookatPt As D3DVECTOR, vUpVec As D3DVECTOR
        Dim matView As D3DMATRIX
        vEyePt = vec3(0, 0, -5)
        vLookatPt = vec3(0, 0, 0)
        vUpVec = vec3(0, 1, 0)
        D3DXMatrixLookAtLH matView, vEyePt, vLookatPt, vUpVec
        .SetTransform D3DTS_VIEW, matView

        
        ' Set the projection matrix
        Dim matProj As D3DMATRIX
        Dim fAspect As Single
        fAspect = 1
        D3DXMatrixPerspectiveFovLH matProj, g_pi / 3, fAspect, 1, 10000
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

        ' Turn on fog, for underwater effect
        Dim fFogStart  As Single
        Dim fFogEnd As Single
        fFogStart = 1
        fFogEnd = 50
        .SetRenderState D3DRS_FOGENABLE, 1 ' True
        .SetRenderState D3DRS_FOGCOLOR, WATER_COLOR
        .SetRenderState D3DRS_FOGTABLEMODE, D3DFOG_NONE
        .SetRenderState D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR
        .SetRenderState D3DRS_RANGEFOGENABLE, 0 'False
        .SetRenderState D3DRS_FOGSTART, FtoDW(fFogStart)
        .SetRenderState D3DRS_FOGEND, FtoDW(fFogEnd)
            
        ' Create a directional light
        Dim light As D3DLIGHT8
        D3DUtil_InitLight light, D3DLIGHT_DIRECTIONAL, 0, -1, 0
        .SetLight 0, light
        .LightEnable 0, 1 'True
        .SetRenderState D3DRS_LIGHTING, 1 'TRUE
        .SetRenderState D3DRS_AMBIENT, AMBIENT_COLOR
    End With
    

    
    
End Sub





'-----------------------------------------------------------------------------
' Name: InvalidateDeviceObjects()
' Desc: Called when the device-dependant objects are about to be lost.
'-----------------------------------------------------------------------------
Sub InvalidateDeviceObjects()
    m_FloorObject.InvalidateDeviceObjects
    m_DolphinGroupObject.InvalidateDeviceObjects
    m_DolphinObject.InvalidateDeviceObjects
    
End Sub


'-----------------------------------------------------------------------------
' Name: DeleteDeviceObjects()
' Desc: Called when the app is exitting, or the device is being changed,
'       this function deletes any device dependant objects.
'----------------------------------------------------------------------
Sub DeleteDeviceObjects()
    m_FloorObject.Destroy
    m_DolphinGroupObject.Destroy
    m_DolphinObject.Destroy
    
    MESHTOOL_DESTROY m_meshtool(0)
    MESHTOOL_DESTROY m_meshtool(1)
    MESHTOOL_DESTROY m_meshtool(2)
    MESHTOOL_DESTROY m_dest

    Set m_DolphinGroupObject = Nothing
    Set m_DolphinObject = Nothing
    Set m_DolphinMesh = Nothing
    Set m_DolphinMesh01 = Nothing
    Set m_DolphinMesh02 = Nothing
    Set m_DolphinMesh03 = Nothing
    Set m_FloorObject = Nothing
    Set m_SeaFloorMesh = Nothing
    
End Sub

Private Sub oSubClass_WindowsMessage(ByVal uMsg As Long, ByVal wParam As Long, ByVal lParam As Long)
    
    Select Case uMsg
    Case WM_POWERBROADCAST 'Something is happening
        Select Case wParam
        Case PBT_APMQUERYSUSPEND, PBT_APMBATTERYLOW, PBT_APMQUERYSTANDBY
            'We're going into Standby mode, or suspend mode, we need to pause the sample
            mfNotSuspended = False 'Stop the render loop
            DeleteDeviceObjects 'Delete everything
            
        Case PBT_APMRESUMESUSPEND, PBT_APMRESUMESTANDBY
            'We're returning, go ahead and restart the sample
            'setup defaults
            Init
            
            'setup d3d
            D3DUtil_DefaultInitWindowed 0, Picture1.hWnd
            m_media = FindMediaDir("dolphin_group.x")
            D3DUtil_SetMediaPath m_media
            
            InitDeviceObjects
            RestoreDeviceObjects
            
            DXUtil_Timer TIMER_START
            mfNotSuspended = True
        End Select
    End Select

End Sub
