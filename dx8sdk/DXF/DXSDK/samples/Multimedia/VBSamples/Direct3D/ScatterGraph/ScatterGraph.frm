VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form GraphForm 
   Caption         =   "Data Analysis Scatter Graph"
   ClientHeight    =   6420
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   7875
   BeginProperty Font 
      Name            =   "MS Sans Serif"
      Size            =   12
      Charset         =   0
      Weight          =   400
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   Icon            =   "ScatterGraph.frx":0000
   LinkTopic       =   "Form1"
   ScaleHeight     =   428
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   525
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton Command1 
      Caption         =   "Command1"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   18
         Charset         =   0
         Weight          =   700
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   435
      Left            =   1920
      TabIndex        =   0
      Top             =   5820
      Visible         =   0   'False
      Width           =   495
   End
   Begin MSComDlg.CommonDialog CommonDialog1 
      Left            =   1080
      Top             =   5760
      _ExtentX        =   847
      _ExtentY        =   847
      _Version        =   393216
   End
   Begin VB.Timer Timer1 
      Enabled         =   0   'False
      Interval        =   10
      Left            =   240
      Top             =   5760
   End
   Begin VB.Menu MENU_POPUP 
      Caption         =   "POPUPMENU"
      Visible         =   0   'False
      Begin VB.Menu MENU_EXITMENU 
         Caption         =   "Exit Menu!"
      End
      Begin VB.Menu MENU_LOAD 
         Caption         =   "Load Data From File!"
      End
      Begin VB.Menu MENU_RESET 
         Caption         =   "Reset Orientation!"
      End
      Begin VB.Menu MENU_CONNECT 
         Caption         =   "Show connecting lines"
         Checked         =   -1  'True
      End
      Begin VB.Menu MENU_LINES 
         Caption         =   "Show height lines"
         Checked         =   -1  'True
      End
      Begin VB.Menu MENU_FOOTLINES 
         Caption         =   "Show foot lines"
         Checked         =   -1  'True
      End
      Begin VB.Menu MENU_BASE 
         Caption         =   "Show base plane"
         Checked         =   -1  'True
      End
      Begin VB.Menu MENU_ROTATE 
         Caption         =   "Auto Rotate"
         Checked         =   -1  'True
      End
   End
End
Attribute VB_Name = "GraphForm"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       ScatterGraph.frm
'  Content:    Implementation of a plot graph in 3 dimensions
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Option Explicit

Dim m_maxX As Double
Dim m_minX As Double
Dim m_maxY As Double
Dim m_minY As Double
Dim m_maxZ As Double
Dim m_minZ As Double
Dim m_maxsize As Double
Dim m_minSize As Double

Dim m_extX As Double
Dim m_extY As Double
Dim m_extZ As Double
Dim m_extSize As Double

Dim m_scalex As Single
Dim m_scaley As Single
Dim m_scalez As Single
Dim m_scalesize As Single

Dim m_xHeader As String
Dim m_yHeader As String
Dim m_zHeader As String
Dim m_sizeHeader As String


Dim m_binit As Boolean
Dim m_bGraphInit As Boolean
Dim m_bMinimized As Boolean


Dim m_graphroot As CD3DFrame
Dim m_quad1 As CD3DFrame
Dim m_quad2 As CD3DFrame
Dim m_XZPlaneFrame As CD3DFrame

Dim m_bRot As Boolean
Dim m_bHeightLines As Boolean
Dim m_bConnectlines As Boolean
Dim m_bShowBase As Boolean
Dim m_bFootLines As Boolean

Dim m_drawtext As String
Dim m_drawtextpos As RECT
Dim m_drawtextEnable As Boolean

Dim m_formatX As String
Dim m_formatY As String
Dim m_formatZ As String
Dim m_formatSize As String

Dim m_data As Collection
Dim m_hwnd As Long
Dim m_vbfont As IFont
Dim m_vbfont2 As IFont
Dim m_font2height  As Long

Dim m_lastX As Single
Dim m_lasty As Single
Dim m_bMouseDown As Boolean


Dim m_Tex As Direct3DTexture8


Dim m_LabelX As CD3DFrame
Dim m_LabelY As CD3DFrame
Dim m_LabelZ As CD3DFrame


Dim m_meshobj As D3DXMesh
Dim m_meshplane As D3DXMesh
Dim m_font As D3DXFont
Dim m_font2 As D3DXFont



'Camera variables
Dim m_fElapsedTime As Single

Dim m_vVelocity  As D3DVECTOR
Dim m_fYawVelocity As Single
Dim m_fPitchVelocity As Single

Dim m_fYaw As Single
Dim m_fPitch As Single
Dim m_vPosition As D3DVECTOR

Dim m_bKey(256) As Boolean
Dim m_matView As D3DMATRIX
Dim m_matOrientation As D3DMATRIX

Dim m_MediaDir As String

Const kdx = 256&
Const kdy = 256&

Const D3DFVF_VERTEX = D3DFVF_XYZ Or D3DFVF_NORMAL Or D3DFVF_TEX1

Friend Sub Init(hwnd As Long, font As IFontDisp, font2 As IFontDisp)
    Dim i As Long
    
    'Save hwnd
    m_hwnd = hwnd
    
    'convert IFontDisp to Ifont
    Set m_vbfont = font
    Set m_vbfont2 = font2
    
    'initialized d3d
    m_binit = D3DUtil_Init(hwnd, True, 0, 0, D3DDEVTYPE_HAL, Nothing)
        
    'exit if initialization failed
    If m_binit = False Then End
    
    DeleteDeviceObjects
    InitDeviceObjects
    BuildDefaultDataList
    ComputeDataExtents
    BuildGraph
    RestoreDeviceObjects
    
    DoEvents
    
    m_bRot = True
    m_xHeader = "X Axis"
    m_yHeader = "Y Axis"
    m_zHeader = "Z Axis"
    m_sizeHeader = "s"
    
    m_vPosition = vec3(0, 0, -20)

    'Initialze camera matrices
    g_dev.GetTransform D3DTS_VIEW, m_matView
    D3DXMatrixTranslation m_matOrientation, 0, 0, 0

    Timer1.Enabled = True
    Call DXUtil_Timer(TIMER_start)
    
End Sub

Private Sub BuildDefaultDataList()
    
    Set m_data = New Collection
    
    Dim i As Single
    
    For i = 1 To 40 Step 2
        AddEntry "pt" + CStr(i), 1 / CSng(i), (i * i) - 25 * i, CSng(i), (0.7 + i / 16), D3DCOLORVALUEtoLONG(ColorValue4(1, 1, 0.5 + i / 20, i / 80)), ""
    Next
    
    m_formatX = "0.000"
    m_formatY = "0.000"
    m_formatZ = "0.000"
    m_formatSize = "0.000"
    m_bConnectlines = True
    m_bHeightLines = True
    m_bShowBase = True
    m_bFootLines = True
    
    m_xHeader = "X Axis"
    m_yHeader = "Y Axis"
    m_zHeader = "Z Axis"
    m_sizeHeader = "s"
    
End Sub

Sub RestoreDeviceObjects()

    g_lWindowWidth = Me.ScaleWidth
    g_lWindowHeight = Me.ScaleHeight
    D3DUtil_SetupDefaultScene
    
    D3DUtil_SetupCamera vec3(0, 5, -20), vec3(0, 0, 0), vec3(0, 1, 0)
    
    'allow the application to show both sides of all surfaces
    g_dev.SetRenderState D3DRS_CULLMODE, D3DCULL_NONE
    
    'turn on min filtering since our text is often smaller
    'than original size
    g_dev.SetTextureStageState 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR
    
    
     Set m_font = g_d3dx.CreateFont(g_dev, m_vbfont.hFont)
    Set m_font2 = g_d3dx.CreateFont(g_dev, m_vbfont2.hFont)
        
End Sub

Private Sub ComputeDataExtents()
    Dim mind As Single
    Dim maxd As Single
    Dim entry As DataEntry
    
    mind = -9E+20
    maxd = 9E+20
    
    m_maxX = mind:    m_maxY = mind:    m_maxZ = mind:    m_maxsize = mind
    m_minX = maxd:    m_minY = maxd:    m_minZ = maxd:    m_minSize = maxd
   
    
    'Dim entry As DataEntry
    For Each entry In m_data
                        
        If entry.datax > m_maxX Then m_maxX = entry.datax
        If entry.datay > m_maxY Then m_maxY = entry.datay
        If entry.dataz > m_maxZ Then m_maxZ = entry.dataz
        If entry.dataSize > m_maxsize Then m_maxsize = entry.dataSize
        
        If entry.datax < m_minX Then m_minX = entry.datax
        If entry.datay < m_minY Then m_minY = entry.datay
        If entry.dataz < m_minZ Then m_minZ = entry.dataz
        If entry.dataSize < m_minSize Then m_minSize = entry.dataSize
                
    Next

    m_extX = m_maxX - m_minX
    m_extY = m_maxY - m_minY
    m_extZ = m_maxZ - m_minZ
    m_extSize = m_maxsize - m_minSize
    
    Dim kScale As Single
    kScale = 5
    
    m_scalex = 1
    m_scaley = 1
    m_scalez = 1
    m_scalesize = 1
    
    If m_maxX > Abs(m_minX) Then
        If m_maxX <> 0 Then m_scalex = kScale / m_maxX
    Else
        If m_minX <> 0 Then m_scalex = kScale / Abs(m_minX)
    End If
    
    If m_maxY > Abs(m_minY) Then
        If m_maxY <> 0 Then m_scaley = kScale / m_maxY
    Else
        If m_minY <> 0 Then m_scaley = kScale / Abs(m_minY)
    End If
   

    If m_maxZ > Abs(m_minZ) Then
        If m_maxZ <> 0 Then m_scalez = kScale / m_maxZ
    Else
        If m_minZ <> 0 Then m_scalez = kScale / Abs(m_minZ)
    End If
    
    
    If m_maxsize = 0 Then m_maxsize = 1
    m_scalesize = 1 * (kScale) / m_maxsize
        

    
    'scale graph data to fit
    For Each entry In m_data
                     
        entry.x = entry.datax * m_scalex
        entry.y = entry.datay * m_scaley
        entry.z = entry.dataz * m_scalez
        entry.size = entry.dataSize * m_scalesize
    
    Next

End Sub

Public Sub AddEntry(sName As String, x As Double, y As Double, z As Double, size As Double, color As Long, data As Variant)
    On Local Error GoTo errOut
    Dim entry As New DataEntry
    entry.dataname = sName
    entry.datax = x
    entry.datay = y
    entry.dataz = z
    entry.dataSize = size
    entry.color = color
    entry.data = data
    m_data.Add entry
    Exit Sub
errOut:
    MsgBox "unable to add entry"
End Sub


Public Sub DrawGraph()
    Dim entry As DataEntry
    Dim hr As Long
    
    If m_binit = False Then Exit Sub
    
    'See what state the device is in.
    hr = g_dev.TestCooperativeLevel
    If hr = D3DERR_DEVICENOTRESET Then
        g_dev.Reset g_d3dpp
        RestoreDeviceObjects
    ElseIf hr <> 0 Then
        Exit Sub
    End If
    
    m_graphroot.UpdateFrames
             
    'Clear the previous render with the backgroud color
    'We clear to grey but notice that we are using a hexidecimal
    'number to represent Alpha Red Green and blue
    D3DUtil_ClearAll &HFF707070
    
    'set the ambient lighting level
    g_dev.SetRenderState D3DRS_AMBIENT, &HFFC0C0C0
    
    
    g_dev.BeginScene
        

        
    
    'only render objects underneath the xzplane
    m_quad1.Enabled = False
    m_quad2.Enabled = True
    m_XZPlaneFrame.Enabled = False
    m_graphroot.Render g_dev

    'render the objects in front of xz plane
    m_quad1.Enabled = True
    m_quad2.Enabled = False
    m_XZPlaneFrame.Enabled = False
    m_graphroot.Render g_dev
        
        
        
    DrawLines 0
  
    DrawAxisNameSquare 0    'x axis
    DrawAxisNameSquare 2    'z axis
    
        
    'draw pop up text
    If m_drawtextEnable Then
        g_d3dx.DrawText m_font, &HFF00FFFF, m_drawtext, m_drawtextpos, 0
    End If
    
    Dim rc As RECT
    rc.Top = 20:    rc.Left = 10
    g_d3dx.DrawText m_font, &HFF00FFFF, "Height = " + m_yHeader, rc, 0
    rc.Top = 40:    rc.Left = 10
    g_d3dx.DrawText m_font, &HFF00FFFF, "Size = " + m_sizeHeader, rc, 0
    
    
    
    'render the xzplane with transparency
    If m_bShowBase Then
        m_quad1.Enabled = False
        m_quad2.Enabled = False
        m_XZPlaneFrame.Enabled = True
        m_graphroot.Render g_dev
    End If
    
    g_dev.EndScene
    
    D3DUtil_PresentAll m_hwnd

End Sub



Public Sub BuildGraph()
    Dim entry As DataEntry
    Dim material As D3DMATERIAL8
    Dim newFrame As CD3DFrame
    Dim i As Long
    Dim d3ddm As D3DDISPLAYMODE
        
    If m_binit = False Then Exit Sub
    
    
    
    'Create rotatable root object
    Set m_graphroot = D3DUtil_CreateFrame(Nothing)
                
    'Create XZ plane for reference
    material.diffuse = LONGtoD3DCOLORVALUE(&H6FC0C0C0)
    material.Ambient = material.diffuse
    Set m_XZPlaneFrame = D3DUtil_CreateFrame(m_graphroot)
    m_XZPlaneFrame.AddD3DXMesh(m_meshplane).SetMaterialOverride material
    m_XZPlaneFrame.SetOrientation D3DUtil_RotationAxis(1, 0, 0, 90)
    
    Set m_quad1 = D3DUtil_CreateFrame(m_graphroot)
    Set m_quad2 = D3DUtil_CreateFrame(m_graphroot)
    
    Set m_LabelX = D3DUtil_CreateFrame(m_graphroot)
    m_LabelX.SetPosition vec3(0, 0, -6)
    
    Set m_LabelY = D3DUtil_CreateFrame(Nothing)
    m_LabelY.SetPosition vec3(-8, 8, 0)
    
    
    Set m_LabelZ = D3DUtil_CreateFrame(m_graphroot)
    m_LabelZ.SetPosition vec3(6, 0, 0)
    m_LabelZ.SetOrientation D3DUtil_RotationAxis(0, 1, 0, -90)
    
    
    Dim quadframe As CD3DFrame
    
    For Each entry In m_data
        If entry.y >= 0 Then Set quadframe = m_quad1
        If entry.y < 0 Then Set quadframe = m_quad2
                
        'Set material of objects
        material.diffuse = LONGtoD3DCOLORVALUE(entry.color)
        material.Ambient = material.diffuse
                
        'Create individual objects
        Set newFrame = D3DUtil_CreateFrame(quadframe)
        newFrame.SetScale entry.size
        newFrame.SetPosition vec3(entry.x, entry.y, entry.z)
        newFrame.AddD3DXMesh(m_meshobj).SetMaterialOverride material
        i = i + 1
        newFrame.ObjectName = Str(i)
   Next
   
   'Take care of labels
    Dim surf As Direct3DSurface8
    Dim rc As RECT
    Dim rts As D3DXRenderToSurface
    Dim rtsviewport As D3DVIEWPORT8
    
    Set surf = m_Tex.GetSurfaceLevel(0)
  
    rtsviewport.height = kdx
    rtsviewport.width = kdy
    rtsviewport.MaxZ = 1

    Call g_dev.GetDisplayMode(d3ddm)
    Set rts = g_d3dx.CreateRenderToSurface(g_dev, kdx, kdy, d3ddm.format, 1, D3DFMT_D16)
  
    rts.BeginScene surf, rtsviewport
    g_dev.Clear 0, ByVal 0, D3DCLEAR_TARGET Or D3DCLEAR_ZBUFFER, &HFFC0C0C0, 1, 0
        
    
    g_d3dx.DrawText m_font2, &HFF000000, m_xHeader, rc, DT_CALCRECT
    m_font2height = rc.bottom
    
    
    rc.Top = m_font2height * 0: rc.Left = 10: rc.bottom = 0: rc.Right = 0
    g_d3dx.DrawText m_font2, &HFF000000, m_xHeader, rc, DT_CALCRECT
    g_d3dx.DrawText m_font2, &HFF000000, m_xHeader, rc, 0
    
    rc.Top = m_font2height * 1: rc.Left = 10: rc.bottom = 0: rc.Right = 0
    g_d3dx.DrawText m_font2, &HFF000000, m_yHeader, rc, DT_CALCRECT
    g_d3dx.DrawText m_font2, &HFF000000, m_yHeader, rc, 0
    
    rc.Top = m_font2height * 2: rc.Left = 10: rc.bottom = 0: rc.Right = 0
    g_d3dx.DrawText m_font2, &HFF000000, m_zHeader, rc, DT_CALCRECT
    g_d3dx.DrawText m_font2, &HFF000000, m_zHeader, rc, 0
    
    rts.EndScene

   
   m_bGraphInit = True
End Sub


Public Sub InitDeviceObjects()
    
    Dim d3ddm As D3DDISPLAYMODE
    
    If m_binit = False Then Exit Sub
    

    Dim rc As RECT
    
    Set m_meshobj = g_d3dx.CreateSphere(g_dev, 0.1, 16, 16, Nothing)
    Set m_meshplane = g_d3dx.CreateBox(g_dev, 10, 10, 0.1, Nothing)
    Set m_font = g_d3dx.CreateFont(g_dev, m_vbfont.hFont)
    Set m_font2 = g_d3dx.CreateFont(g_dev, m_vbfont2.hFont)
    
    Call g_dev.GetDisplayMode(d3ddm)
    
    'Create Textures
    Set m_Tex = g_d3dx.CreateTexture(g_dev, kdx, kdx, 0, 0, d3ddm.format, D3DPOOL_MANAGED)
    
    
       
    
End Sub



Private Sub DrawLines(quad As Long)
    Dim entry As DataEntry
    Dim vLast As D3DVECTOR, vNext As D3DVECTOR
    Dim vGround As D3DVECTOR
    Dim vGround1 As D3DVECTOR
    Dim vGround2 As D3DVECTOR
    Dim i As Long
    
    'Link lines
    g_dev.SetTransform D3DTS_WORLD, m_graphroot.GetMatrix
    
    Set entry = m_data.item(1)
    vLast = vec3(entry.x, entry.y, entry.z)
    
    vGround = vLast
    vGround.y = 0
    
    Call DrawLine(vGround, vLast, &HFFFF0000)
    
    For i = 2 To m_data.count
        Set entry = m_data.item(i)
        vNext = vec3(entry.x, entry.y, entry.z)
        
        If m_bConnectlines Then
            Call DrawLine(vLast, vNext, &HFFFF00FF)
        End If
        
        vGround = vNext
        vGround.y = 0
        vGround1 = vGround
        vGround1.y = 0.1
        vGround2 = vLast
        vGround2.y = 0.1
        
        If m_bHeightLines Then
            Call DrawLine(vGround, vNext, &HFFFF0000)
        End If
        
        If m_bFootLines Then
            Call DrawLine(vGround1, vGround2, &HFF10FF30)
        End If
        
        vLast = vNext
    Next
    
    DrawLine vec3(-5, 0.1, 0), vec3(5, 0.1, 0), &HFF0&
    DrawLine vec3(0, 0.1, -5), vec3(0, 0.1, 5), &HFF0&
    
End Sub

Private Sub DrawLine(v1 As D3DVECTOR, v2 As D3DVECTOR, color As Long)
    
    Dim mat As D3DMATERIAL8
    mat.diffuse = LONGtoD3DCOLORVALUE(color)
    mat.Ambient = mat.diffuse
    g_dev.SetMaterial mat
    
    Dim dataOut(2) As D3DVERTEX
    LSet dataOut(0) = v1
    LSet dataOut(1) = v2
    g_dev.SetVertexShader D3DFVF_VERTEX
    g_dev.DrawPrimitiveUP D3DPT_LINELIST, 1, dataOut(0), Len(dataOut(0))
    
End Sub



Public Sub MouseOver(Button As Integer, Shift As Integer, x As Single, y As Single)
    
    If m_binit = False Then Exit Sub
    
    Dim pick As New CD3DPick
    Dim frame As CD3DFrame
    Dim nid As Long
    Dim entry As DataEntry
    
    'remove the XZ plane from consideration for pick
    m_XZPlaneFrame.Enabled = False
    m_quad1.Enabled = True
    m_quad2.Enabled = True
    
    
    pick.ViewportPick m_graphroot, x, y
    nid = pick.FindNearest()
    If nid < 0 Then
        m_drawtextEnable = False
        Exit Sub
    End If
        
    Set frame = pick.GetFrame(nid)
    
    'have matrices pre computed for scene graph
    m_graphroot.UpdateFrames
    
    'due some math to get position of item in screen space
    Dim viewport As D3DVIEWPORT8
    Dim projmatrix As D3DMATRIX
    Dim viewmatrix As D3DMATRIX
    Dim vOut As D3DVECTOR
    
    g_dev.GetViewport viewport
    g_dev.GetTransform D3DTS_PROJECTION, projmatrix
    g_dev.GetTransform D3DTS_VIEW, viewmatrix
    D3DXVec3Project vOut, vec3(0, 0, 0), viewport, projmatrix, viewmatrix, frame.GetUpdatedMatrix
            
    Debug.Print vOut.x, vOut.y, frame.ObjectName
    
    
    Dim destRect As RECT
    m_drawtextpos.Left = x - 20
    m_drawtextpos.Top = y - 70
    
    If m_drawtextpos.Left < 0 Then m_drawtextpos.Left = 1
    If m_drawtextpos.Top < 0 Then m_drawtextpos.Top = 1
    
    
    Set entry = m_data.item(val(frame.ObjectName))
    With entry
        m_drawtext = .dataname + Chr(13)
        m_drawtext = m_drawtext + " " + m_xHeader + "=" + format$(.datax, m_formatX) + Chr(13)
        m_drawtext = m_drawtext + " " + m_yHeader + "=" + format$(.datay, m_formatY) + Chr(13)
        m_drawtext = m_drawtext + " " + m_zHeader + "=" + format$(.dataz, m_formatZ) + Chr(13)
        m_drawtext = m_drawtext + " " + m_sizeHeader + "=" + format$(.dataSize, m_formatSize)
    End With
    m_drawtextEnable = True

End Sub

Sub FrameMove()

    'for camera movement
    m_fElapsedTime = DXUtil_Timer(TIMER_GETELLAPSEDTIME) * 1.3
    If m_fElapsedTime < 0 Then Exit Sub
        
        
    If m_bRot And m_bMouseDown = False Then
        m_graphroot.AddRotation COMBINE_BEFORE, 0, 1, 0, (g_pi / 40) * m_fElapsedTime
    End If
        
        
    ' Slow things down for the REF device
    If (g_devType = D3DDEVTYPE_REF) Then m_fElapsedTime = 0.05

    Dim fSpeed As Single
    Dim fAngularSpeed
    
    fSpeed = 5 * m_fElapsedTime
    fAngularSpeed = 1 * m_fElapsedTime

    ' Slowdown the camera movement
    D3DXVec3Scale m_vVelocity, m_vVelocity, 0.9
    m_fYawVelocity = m_fYawVelocity * 0.9
    m_fPitchVelocity = m_fPitchVelocity * 0.9

    ' Process keyboard input
    If (m_bKey(vbKeyRight)) Then m_vVelocity.x = m_vVelocity.x + fSpeed        '  Slide Right
    If (m_bKey(vbKeyLeft)) Then m_vVelocity.x = m_vVelocity.x - fSpeed         '  Slide Left
    If (m_bKey(vbKeyUp)) Then m_vVelocity.y = m_vVelocity.y + fSpeed           '  Move up
    If (m_bKey(vbKeyDown)) Then m_vVelocity.y = m_vVelocity.y - fSpeed         '  Move down
    
    If (m_bKey(vbKeyW)) Then m_vVelocity.z = m_vVelocity.z + fSpeed            '  Move Forward
    If (m_bKey(vbKeyS)) Then m_vVelocity.z = m_vVelocity.z - fSpeed            '  Move Backward
    
    If (m_bKey(vbKeyE)) Then m_fYawVelocity = m_fYawVelocity + fSpeed          '  Yaw right
    If (m_bKey(vbKeyQ)) Then m_fYawVelocity = m_fYawVelocity - fSpeed          '  Yaw left
    
    If (m_bKey(vbKeyZ)) Then m_fPitchVelocity = m_fPitchVelocity + fSpeed      '  turn down
    If (m_bKey(vbKeyA)) Then m_fPitchVelocity = m_fPitchVelocity - fSpeed      '  turn up
    
    

    ' Update the position vector
    Dim vT As D3DVECTOR, vTemp As D3DVECTOR
    D3DXVec3Scale vTemp, m_vVelocity, fSpeed
    D3DXVec3Add vT, vT, vTemp
    D3DXVec3TransformNormal vT, vT, m_matOrientation
    D3DXVec3Add m_vPosition, m_vPosition, vT
    
    If (m_vPosition.y < 1) Then m_vPosition.y = 1

    ' Update the yaw-pitch-rotation vector
    m_fYaw = m_fYaw + fAngularSpeed * m_fYawVelocity
    m_fPitch = m_fPitch + fAngularSpeed * m_fPitchVelocity
    If (m_fPitch < 0) Then m_fPitch = 0
    If (m_fPitch > g_pi / 2) Then m_fPitch = g_pi / 2

    Dim qR As D3DQUATERNION, det As Single
    D3DXQuaternionRotationYawPitchRoll qR, m_fYaw, m_fPitch, 0
    D3DXMatrixAffineTransformation m_matOrientation, 1.25, vec3(0, 0, 0), qR, m_vPosition
    D3DXMatrixInverse m_matView, det, m_matOrientation

        'set new view matrix
    g_dev.SetTransform D3DTS_VIEW, m_matView

End Sub

Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
    m_bKey(KeyCode) = True
End Sub

Private Sub Form_KeyUp(KeyCode As Integer, Shift As Integer)
    m_bKey(KeyCode) = False
End Sub

Private Sub Form_Load()
    
    'Show the form
    Me.Show
    DoEvents
        
    m_MediaDir = FindMediaDir("ScatterData.csv")
    D3DUtil.D3DUtil_SetMediaPath m_MediaDir
    
    'initialize the graph
    Init Me.hwnd, Me.font, Command1.font
    
    
End Sub

Private Sub Form_MouseDown(Button As Integer, Shift As Integer, x As Single, y As Single)
    If Button = 2 Then
        Me.PopupMenu MENU_POPUP
    Else
    
        '- save our current position
        m_bMouseDown = True
        m_lastX = x
        m_lasty = y
        
    End If
End Sub

Private Sub Form_MouseMove(Button As Integer, Shift As Integer, x As Single, y As Single)
        
    If m_binit = False Then Exit Sub
    
    If Button = 2 Then Exit Sub
    If m_bMouseDown = False Then
        Call MouseOver(Button, Shift, x, y)
    Else
        '- Rotate the object
        RotateTrackBall CInt(x), CInt(y)
    End If
    
    FrameMove
    DrawGraph
    
End Sub

Private Sub Form_MouseUp(Button As Integer, Shift As Integer, x As Single, y As Single)
    m_bMouseDown = False
End Sub


'-----------------------------------------------------------------------------
' Name: Form_Resize()
' Desc: hadle resizing of the D3D backbuffer
'-----------------------------------------------------------------------------
Private Sub Form_Resize()
    
    
    Timer1.Enabled = False
    
    ' If D3D is not initialized then exit
    If Not m_binit Then Exit Sub
    
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
     
    DeleteDeviceObjects

    'reset and resize our D3D backbuffer to the size of the window
    D3DUtil_ResizeWindowed Me.hwnd
    
    'All state get losts after a reset so we need to reinitialze it here
    RestoreDeviceObjects
    
    Timer1.Enabled = True
    
End Sub

'- Rotate Track ball
'  given a point on the screen the mouse was moved to
'  simulate a track ball
Private Sub RotateTrackBall(x As Integer, y As Integer)

    
    Dim delta_x As Single, delta_y As Single
    Dim delta_r As Single, radius As Single, denom As Single, angle As Single
    
    ' rotation axis in camcoords, worldcoords, sframecoords
    Dim axisC As D3DVECTOR
    Dim wc As D3DVECTOR
    Dim axisS As D3DVECTOR
    Dim base As D3DVECTOR
    Dim origin As D3DVECTOR
    
    delta_x = x - m_lastX
    delta_y = y - m_lasty
    m_lastX = x
    m_lasty = y

            
     delta_r = Sqr(delta_x * delta_x + delta_y * delta_y)
     radius = 50
     denom = Sqr(radius * radius + delta_r * delta_r)
    
    If (delta_r = 0 Or denom = 0) Then Exit Sub
    angle = (delta_r / denom)

    axisC.x = (-delta_y / delta_r)
    axisC.y = (-delta_x / delta_r)
    axisC.z = 0


    'transform camera space vector to world space
    'm_largewindow.m_cameraFrame.Transform wc, axisC
    g_dev.GetTransform D3DTS_VIEW, g_viewMatrix
    D3DXVec3TransformCoord wc, axisC, g_viewMatrix
    
    
    'transform world space vector into Model space
    m_graphroot.UpdateFrames
    axisS = m_graphroot.InverseTransformCoord(wc)
        
    'transform origen camera space to world coordinates
    'm_largewindow.m_cameraFrame.Transform  wc, origin
    D3DXVec3TransformCoord wc, origin, g_viewMatrix
    
    'transfer cam space origen to model space
    base = m_graphroot.InverseTransformCoord(wc)
    
    axisS.x = axisS.x - base.x
    axisS.y = axisS.y - base.y
    axisS.z = axisS.z - base.z
    
    m_graphroot.AddRotation COMBINE_BEFORE, axisS.x, axisS.y, axisS.z, angle
    
End Sub


Private Sub Form_Paint()
    If Not m_binit Then Exit Sub
    If Not m_bGraphInit Then Exit Sub
    DrawGraph
End Sub

Private Sub Form_Unload(Cancel As Integer)
    End
End Sub

Private Sub MENU_BASE_Click()
    m_bShowBase = Not m_bShowBase
    MENU_BASE.Checked = m_bShowBase
End Sub

Private Sub MENU_CONNECT_Click()
    m_bConnectlines = Not m_bConnectlines
    MENU_CONNECT.Checked = m_bConnectlines
End Sub

Private Sub MENU_FOOTLINES_Click()
    m_bFootLines = Not m_bFootLines
    MENU_FOOTLINES.Checked = m_bFootLines
End Sub

Private Sub MENU_LINES_Click()
    m_bHeightLines = Not m_bHeightLines
    MENU_LINES.Checked = m_bHeightLines
End Sub

Private Sub MENU_LOAD_Click()
    Dim sFile As String
    
    CommonDialog1.FileName = ""
    CommonDialog1.DefaultExt = "csv"
    CommonDialog1.filter = "csv|*.csv"
    CommonDialog1.InitDir = m_MediaDir
    
    On Local Error Resume Next
    CommonDialog1.ShowOpen
    sFile = CommonDialog1.FileName
    If sFile = "" Then Exit Sub
    LoadFile sFile
    
    Set m_graphroot = Nothing
    Set m_quad1 = Nothing
    Set m_quad2 = Nothing
    Set m_XZPlaneFrame = Nothing
    
    ComputeDataExtents
    BuildGraph
    RestoreDeviceObjects
    
End Sub

Private Sub MENU_RESET_Click()
    m_graphroot.SetMatrix g_identityMatrix
    m_vPosition = vec3(0, 0, -20)
    m_fYaw = 0
    m_fPitch = 0
 
    Call D3DXMatrixTranslation(m_matOrientation, 0, 0, 0)
End Sub

Private Sub MENU_ROTATE_Click()
    m_bRot = Not m_bRot
    MENU_ROTATE.Checked = m_bRot
End Sub

Private Sub Timer1_Timer()
    If Not m_binit Then Exit Sub
    
    FrameMove
    DrawGraph
End Sub

Sub LoadFile(sFile As String)
    
    
    If Dir$(sFile) = "" Then
        MsgBox "Unable to find " + sFile
        Exit Sub
    End If
    
    Dim fl As Long
    Dim strIn As String
    Dim strTrim As String
    Dim strFirstChar As String
    Dim splitArray
    Dim cols As Long
    Dim bFoundData As Boolean
    Dim sName As String
    Dim x As Double
    Dim y As Double
    Dim z As Double
    Dim size As Double
    Dim color As Long
    Dim data
    Dim i As Long
    Dim olddata As Collection
    
    fl = FreeFile
    
    On Local Error GoTo errOut
    
    Set olddata = m_data
    Set m_data = New Collection
    
    Open sFile For Input As fl
        
    Do While Not EOF(fl)
        Line Input #fl, strIn
        strTrim = Trim(strIn)
        
        'skip comment lines
        strFirstChar = Mid$(strTrim, 1, 1)
        If strFirstChar = "#" Or strFirstChar = ";" Then GoTo nextLine
        If strTrim = "" Then GoTo nextLine
        
        splitArray = Split(strTrim, ",")
        
        cols = UBound(splitArray)
        If cols < 4 Then
            MsgBox "Comma delimited file must have at least 4 columns (name,x,y,z)"
            Exit Sub
        End If
                
        
        'If we have not found numbers see if we found a header row
        If Not bFoundData Then
            If IsNumeric(splitArray(1)) = False Then
                'assume data is a header row
                m_xHeader = CStr(splitArray(1))
                m_yHeader = CStr(splitArray(2))
                m_zHeader = CStr(splitArray(3))
                m_sizeHeader = CStr(splitArray(4))
                GoTo nextLine
            Else
                bFoundData = True
            End If
        End If
        
        sName = CStr(splitArray(0))
        x = val(splitArray(1))
        y = val(splitArray(2))
        z = val(splitArray(3))
        
        'set defaults
        i = i + 1
        size = 1
        color = D3DCOLORVALUEtoLONG(ColorValue4(1, (10 + i Mod 20) / 30, 0.3, (10 + (i Mod 40)) / 50))
        data = ""
        
        If cols >= 4 Then size = val(splitArray(4))
        If cols >= 5 Then color = val(splitArray(5))
        If cols >= 6 Then data = splitArray(6)
        
        AddEntry sName, x, y, z, size, color, data
        
        
nextLine:
    Loop
    
    Set olddata = Nothing
    Close fl
    Exit Sub
    
errOut:
    Set m_data = olddata
    MsgBox "there was an error loading " + sFile
    Close fl
End Sub

Sub DrawAxisNameSquare(i As Long)

    Dim verts(4) As D3DVERTEX
    Dim w As Single
    Dim h As Single
    Dim mat As D3DMATERIAL8
    Dim sv As Single
    Dim ev As Single
    
    
    w = 2:    h = 0.25
        
    

    mat.Ambient = ColorValue4(1, 1, 1, 1)
    mat.diffuse = ColorValue4(1, 1, 1, 1)
    
    
    
    sv = (m_font2height * (i) / kdy)
    ev = (m_font2height * (i + 1) / kdy)
    

    Select Case i
        Case 0
            g_dev.SetTransform D3DTS_WORLD, m_LabelX.GetUpdatedMatrix
            
        Case 1
            'Y axis now part of HUD
            Exit Sub
        Case 2
            g_dev.SetTransform D3DTS_WORLD, m_LabelZ.GetUpdatedMatrix
            
    End Select
        
    g_dev.SetTexture 0, m_Tex
    g_dev.SetMaterial mat
    
    With verts(0): .x = -w: .y = -h: .tu = 0: .tv = ev: .nz = -1: End With
    With verts(1): .x = w: .y = -h: .tu = 1: .tv = ev: .nz = -1: End With
    With verts(2): .x = w: .y = h: .tu = 1: .tv = sv: .nz = -1: End With
    With verts(3): .x = -w: .y = h: .tu = 0: .tv = sv: .nz = -1: End With
    g_dev.SetVertexShader D3DFVF_VERTEX
    g_dev.DrawPrimitiveUP D3DPT_TRIANGLEFAN, 2, verts(0), Len(verts(0))
    
    
    With verts(0): .z = 0.01: .x = w: .y = -h: .tu = 0: .tv = ev: .nz = 1: End With
    With verts(1): .z = 0.01: .x = -w: .y = -h: .tu = 1: .tv = ev: .nz = 1: End With
    With verts(2): .z = 0.01: .x = -w: .y = h: .tu = 1: .tv = sv: .nz = 1: End With
    With verts(3): .z = 0.01: .x = w: .y = h: .tu = 0: .tv = sv: .nz = 1: End With
    g_dev.SetVertexShader D3DFVF_VERTEX
    g_dev.DrawPrimitiveUP D3DPT_TRIANGLEFAN, 2, verts(0), Len(verts(0))
    

End Sub

Sub DeleteDeviceObjects()
    Set m_font = Nothing
    Set m_font2 = Nothing
End Sub

