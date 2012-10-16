VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "comdlg32.ocx"
Begin VB.Form GraphForm 
   Caption         =   "Data Analysis Bar Graph"
   ClientHeight    =   6420
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   7875
   BeginProperty Font 
      Name            =   "MS Sans Serif"
      Size            =   13.5
      Charset         =   0
      Weight          =   700
      Underline       =   0   'False
      Italic          =   0   'False
      Strikethrough   =   0   'False
   EndProperty
   Icon            =   "BarGraph.frx":0000
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
'  File:       BarGraph.frm
'  Content:    Implementation of a 3D BarGraph
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

Dim m_binit As Boolean
Dim m_bGraphInit As Boolean
Dim m_bMinimized As Boolean

Dim m_graphroot As CD3DFrame
Dim m_quad1 As CD3DFrame
Dim m_quad2 As CD3DFrame
Dim m_XZPlaneFrame As CD3DFrame

Dim m_bRot As Boolean
Dim m_bShowBase As Boolean


Dim m_drawtext As String
Dim m_drawtextpos As RECT
Dim m_drawtextEnable As Boolean


Dim m_data As Collection
Dim m_hwnd As Long
Dim m_vbfont As IFont
Dim m_vbfont2 As IFont
Dim m_font2height  As Long

Dim m_lastX As Single
Dim m_lasty As Single
Dim m_bMouseDown As Boolean

Dim m_Tex As Direct3DTexture8

Dim m_meshobj As D3DXMesh
Dim m_meshplane As D3DXMesh
Dim m_font As D3DXFont
Dim m_font2 As D3DXFont

Dim m_mediadir As String

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


Const kdx = 256&
Const kdy = 256&
Const kScale = 8


Dim m_GraphTitle As String
Dim m_RowLabels As Collection
Dim m_ColLabels As Collection
Dim m_cols As Long
Dim m_rows As Long
Dim m_barmesh() As D3DXMesh
Dim m_labelmesh() As D3DXMesh
Dim m_LabelTex() As Direct3DTexture8
Dim m_sizex As Single
Dim m_sizez As Single
Dim m_ColTextures() As String
Dim m_RowTextures() As String

Const D3DFVF_VERTEX = D3DFVF_XYZ Or D3DFVF_NORMAL Or D3DFVF_TEX1


Implements DirectXEvent8

Sub DestroyDeviceObjects()

    Set m_graphroot = Nothing
    Set m_quad1 = Nothing
    Set m_quad2 = Nothing
    Set m_XZPlaneFrame = Nothing
    ReDim m_LabelTex(0)
    ReDim m_barmesh(0)
    ReDim m_labelmesh(0)
    
    
End Sub

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
    
    
    m_bRot = True
    
    D3DXMatrixTranslation m_matOrientation, 0, 0, 0
    
    m_vPosition = vec3(0, 0, -20)

    m_sizex = 1
    m_sizez = 1
    
    Set m_RowLabels = New Collection
    Set m_ColLabels = New Collection
    m_RowLabels.Add "XXX"
    m_ColLabels.Add "ZZZ"

    m_bShowBase = True
    
    DeleteDeviceObjects
    InitDeviceObjects
    LoadFileAsBarGraph (m_mediadir + "bargraphdata.csv")
    ComputeDataExtents
    RestoreDeviceObjects
    BuildGraph
    
    
    DoEvents

    'Initialze camera matrices
    g_dev.GetTransform D3DTS_VIEW, m_matView

    
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

Sub DeleteDeviceObjects()
    Set m_font = Nothing
    Set m_font2 = Nothing
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
                     
        entry.x = (entry.datax - m_maxX / 2) * m_scalex
        entry.y = (entry.datay) * m_scaley / 2
        entry.z = (entry.dataz - m_maxZ / 2) * m_scalez
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
    D3DUtil_ClearAll &HFF808080
    
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


    'DrawLines 0

   

    'draw pop up text
    If m_drawtextEnable Then
        m_font.Begin
        g_d3dx.DrawText m_font, &HFF000000, m_drawtext, m_drawtextpos, 0
        m_font.End
    End If


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
    If Not m_binit Then Exit Sub
    
    Dim entry As DataEntry
    Dim material As D3DMATERIAL8
    Dim newFrame As CD3DFrame
    Dim mesh As D3DXMesh
    Dim frameMesh As CD3DMesh
    Dim i As Long, j As Long
    Dim w As Single, h As Single
    Dim sv As Single, ev As Single
    Dim su As Single, eu As Single
    Dim d3ddm As D3DDISPLAYMODE
    
    If m_binit = False Then Exit Sub
    
    Set m_graphroot = Nothing
    Set m_quad1 = Nothing
    Set m_quad2 = Nothing
    
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
    
    
    
    
    Dim rc As RECT
    Dim surf As Direct3DSurface8
    Dim rts As D3DXRenderToSurface
    Dim rtsviewport As D3DVIEWPORT8
    
    Call g_dev.GetDisplayMode(d3ddm)
    Set rts = g_d3dx.CreateRenderToSurface(g_dev, kdx, kdy, d3ddm.format, 1, D3DFMT_D16)
    rtsviewport.height = kdx
    rtsviewport.width = kdy
    rtsviewport.MaxZ = 1
    
    Set surf = m_Tex.GetSurfaceLevel(0)
          
    rts.BeginScene surf, rtsviewport
    g_dev.Clear 0, ByVal 0, D3DCLEAR_TARGET Or D3DCLEAR_ZBUFFER, &HFFC0C0C0, 1, 0


    
    g_d3dx.DrawText m_font2, &HFF000000, "XXX", rc, DT_CALCRECT
    m_font2height = rc.bottom
    
    
    i = 0
    Dim item As Variant
    For Each item In m_RowLabels
        If m_font2height * i >= kdy Then Exit For
        rc.Top = m_font2height * i: rc.Left = 10: rc.bottom = 0: rc.Right = 0
        g_d3dx.DrawText m_font2, &HFF000000, item, rc, DT_CALCRECT
        g_d3dx.DrawText m_font2, &HFF000000, item, rc, 0
        i = i + 1
    Next
    For Each item In m_ColLabels
        If m_font2height * i >= kdy Then Exit For
        rc.Top = m_font2height * i: rc.Left = 10: rc.bottom = 0: rc.Right = 0
        g_d3dx.DrawText m_font2, &HFF000000, item, rc, DT_CALCRECT
        g_d3dx.DrawText m_font2, &HFF000000, item, rc, 0
        i = i + 1
    Next
    
    
    rts.EndScene
    
    i = 0
    Dim quadframe As CD3DFrame
    
    ReDim m_barmesh(0)
    For Each entry In m_data
        If entry.y >= 0 Then Set quadframe = m_quad1
        If entry.y < 0 Then Set quadframe = m_quad2
                
        'Set material of objects
        material.diffuse = LONGtoD3DCOLORVALUE(entry.color)
        material.Ambient = material.diffuse
                
        'Create individual objects
        Set newFrame = D3DUtil_CreateFrame(quadframe)
        newFrame.SetScale 1
        newFrame.SetPosition vec3(entry.x, entry.y / 2, entry.z)
        
        ReDim Preserve m_barmesh(i)
        Set m_barmesh(i) = g_d3dx.CreateBox(g_dev, m_sizex, Abs(entry.y), m_sizez, Nothing)
        newFrame.AddD3DXMesh(m_barmesh(i)).SetMaterialOverride material
        
        
        
        i = i + 1
        newFrame.ObjectName = Str(i)
    Next
        
    
    Dim strLabel As Variant
        
    w = m_sizex * 3:  h = 0.5
    
    i = 0
    If Not (m_cols = 0 Or m_rows = 0) Then
   
        ReDim m_labelmesh(m_rows + m_cols)
        ReDim m_LabelTex(m_rows + m_cols)
        
        For Each strLabel In m_ColLabels
        
        i = i + 1
        
        su = 0: eu = 0.5:
        sv = (m_font2height * (i - 1) / kdy)
        ev = (m_font2height * (i) / kdy)
                                    
        Set newFrame = CreateSheetWithTextureCoords(w, h, su, eu, sv, ev, m_Tex)
        newFrame.ObjectName = strLabel
        newFrame.SetPosition vec3(5.5, -h / 2, (i - m_maxZ / 2 - 1) * m_scalez)
        newFrame.AddRotation COMBINE_BEFORE, 0, 1, 0, g_pi / 2
        m_graphroot.AddChild newFrame
        
        Set newFrame = CreateSheetWithTextureCoords(w, h, su, eu, sv, ev, m_Tex)
        newFrame.ObjectName = strLabel
        newFrame.SetPosition vec3(-5.5, 5 - h / 2, (i - m_maxZ / 2 - 1) * m_scalez)
        newFrame.AddRotation COMBINE_BEFORE, 0, 1, 0, g_pi / 2
        m_graphroot.AddChild newFrame
        
        su = 0: eu = 1: sv = 0: ev = 1
        
        LoadTexture i, m_RowTextures(i)    'note row and col texture are swapped
        
        If Not m_LabelTex(i) Is Nothing Then
                Set newFrame = CreateSheetWithTextureCoords(w, w, su, eu, sv, ev, m_LabelTex(i))
                newFrame.ObjectName = strLabel + " picture"
                newFrame.SetPosition vec3(5.5, -h - w / 2, (i - m_maxZ / 2 - 1) * m_scalez)
                newFrame.AddRotation COMBINE_BEFORE, 0, 1, 0, g_pi / 2
                m_graphroot.AddChild newFrame
            End If
             

        Next
        
        j = 0
        For Each strLabel In m_RowLabels
             Set newFrame = D3DUtil_CreateFrame(m_graphroot)
             i = i + 1: j = j + 1
             
             
             su = 0: eu = 0.5:
             sv = (m_font2height * (i - 1) / kdy)
             ev = (m_font2height * (i) / kdy)
                                             
             Set newFrame = CreateSheetWithTextureCoords(w, h, su, eu, sv, ev, m_Tex)
             newFrame.ObjectName = strLabel
             newFrame.SetPosition vec3((j - m_maxX / 2 - 1) * m_scalex, -h / 2, -5.5)
             m_graphroot.AddChild newFrame
             
             Set newFrame = CreateSheetWithTextureCoords(w, h, su, eu, sv, ev, m_Tex)
             newFrame.ObjectName = strLabel
             newFrame.SetPosition vec3((j - m_maxX / 2 - 1) * m_scalex, 5 - h / 2, 5.5)
             m_graphroot.AddChild newFrame
             
             su = 0: eu = 1: sv = 0: ev = 1
             
             LoadTexture i, m_ColTextures(j)    'note row and col texture are swapped
             
             If Not m_LabelTex(i) Is Nothing Then
                Set newFrame = CreateSheetWithTextureCoords(w, w, su, eu, sv, ev, m_LabelTex(i))
                newFrame.ObjectName = strLabel + " picture"
                newFrame.SetPosition vec3((j - m_maxX / 2 - 1) * m_scalex, -h - w / 2, -5.5)
                m_graphroot.AddChild newFrame
            End If

        Next

    End If
    
        


    
    

   
    m_bGraphInit = True
End Sub


Public Sub InitDeviceObjects()
    
    Dim d3ddm As D3DDISPLAYMODE
    
    If m_binit = False Then Exit Sub
    

    Dim rc As RECT
    
    Set m_meshobj = g_d3dx.CreateBox(g_dev, 0.1, 0.1, 0.1, Nothing)
    Set m_meshplane = g_d3dx.CreateBox(g_dev, 10, 10, 0.1, Nothing)
    
    Call g_dev.GetDisplayMode(d3ddm)
    Set m_Tex = g_d3dx.CreateTexture(g_dev, kdx, kdx, 0, 0, d3ddm.format, D3DPOOL_MANAGED)
    
    Set m_font = g_d3dx.CreateFont(g_dev, m_vbfont.hFont)
    Set m_font2 = g_d3dx.CreateFont(g_dev, m_vbfont2.hFont)
       
       
    
End Sub




Private Sub DrawLines(quad As Long)
    
    g_dev.SetTransform D3DTS_WORLD, m_graphroot.GetMatrix
    
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
    
    
    Dim iOver As Long
    If IsNumeric(frame.ObjectName) Then
        iOver = val(frame.ObjectName)
        Set entry = m_data.item(iOver)
        With entry
            m_drawtext = .dataname + Chr(13)
        End With
        m_drawtextEnable = True
    End If

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

Private Sub DirectXEvent8_DXCallback(ByVal i As Long)
    Dim w As Single
    Dim h As Single
    
    Dim w1 As Single, w2 As Single
    Dim h1 As Single, h2 As Single
    Dim sv As Single, ev As Single
    Dim su As Single, eu As Single
    
    Dim mat As D3DMATERIAL8
                  
    w = m_sizex * 1.4:  h = 0.4
        
    mat.Ambient = ColorValue4(1, 1, 1, 1)
    mat.diffuse = ColorValue4(1, 1, 1, 1)
    
        
    sv = (m_font2height * (i) / kdy)
    ev = (m_font2height * (i + 1) / kdy)
    
    'g_dev.SetTexture 0, m_Tex
    'g_dev.SetMaterial mat
    DrawLine vec3(1, 1, 1), vec3(0, 0, 0), &HFF00FF00
    
    w = m_sizex * 1.4:  h = 0.4
    
    'DrawSheet -w, w, -2 * h, 0, 0, 0.5, sv, ev
    
    'g_dev.SetTexture 0, m_LabelTex(i + 1)
    'DrawSheet -w, w, -2 * h - 2 * w, -2 * h, 0, 1, 0, 1
    
    
End Sub

Function CreateSheetWithTextureCoords(width As Single, height As Single, su As Single, eu As Single, sv As Single, ev As Single, texture As Direct3DTexture8) As CD3DFrame
    Dim frame As CD3DFrame
    Dim mesh As CD3DMesh
    Dim retd3dxMesh As D3DXMesh
    Dim vertexbuffer As Direct3DVertexBuffer8
    Dim verts(8) As D3DVERTEX
    Dim indices(12) As Integer
    Dim w As Single, d As Single, h1 As Single, h2 As Single
    
    w = width / 2
    h2 = height / 2
    h1 = -height / 2
    d = 0.01
    
    Dim whitematerial As D3DMATERIAL8
    whitematerial.diffuse = ColorValue4(1, 1, 1, 1)
    whitematerial.Ambient = whitematerial.diffuse
        
    'Create an empty d3dxmesh with room for 12 vertices and 12
    Set retd3dxMesh = g_d3dx.CreateMeshFVF(8, 12, D3DXMESH_MANAGED, D3DFVF_VERTEX, g_dev)
    
    
    'front face
    
    'add vertices
    With verts(0): .x = -w: .y = h2: .z = -d: .nz = 1: .tu = su: .tv = sv: End With
    With verts(1): .x = w: .y = h2: .z = -d: .nz = 1: .tu = eu: .tv = sv: End With
    With verts(2): .x = w: .y = h1: .z = -d: .nz = 1: .tu = eu: .tv = ev: End With
    With verts(3): .x = -w: .y = h1: .z = -d: .nz = 1: .tu = su: .tv = ev: End With
    
    'connect verices to make 2 triangles per face
    indices(0) = 0: indices(1) = 1: indices(2) = 2
    indices(3) = 0: indices(4) = 2: indices(5) = 3
    
    'back face
    With verts(4): .x = -w: .y = h1: .z = d: .nz = -1: .tu = eu: .tv = ev: End With
    With verts(5): .x = w: .y = h1: .z = d: .nz = -1: .tu = su: .tv = ev: End With
    With verts(6): .x = w: .y = h2: .z = d: .nz = -1: .tu = su: .tv = sv: End With
    With verts(7): .x = -w: .y = h2: .z = d: .nz = -1: .tu = eu: .tv = sv: End With
    indices(6) = 4: indices(7) = 5: indices(8) = 6
    indices(9) = 4: indices(10) = 6: indices(11) = 7
    
        
    
    D3DXMeshVertexBuffer8SetData retd3dxMesh, 0, Len(verts(0)) * 8, 0, verts(0)
    D3DXMeshIndexBuffer8SetData retd3dxMesh, 0, Len(indices(0)) * 12, 0, indices(0)
        
    Set frame = New CD3DFrame
    Set mesh = frame.AddD3DXMesh(retd3dxMesh)
    
    mesh.bUseMaterials = True
    mesh.SetMaterialCount 1
    mesh.SetMaterial 0, whitematerial
    mesh.SetMaterialTexture 0, texture
    
    Set CreateSheetWithTextureCoords = frame
End Function

Sub DrawSheet(w1 As Single, w2 As Single, h1 As Single, h2 As Single, su As Single, eu As Single, sv As Single, ev As Single)
    Dim verts(4) As D3DVERTEX

    g_dev.SetTexture 0, Nothing
    
    With verts(0): .x = w1: .y = h1: .tu = su: .tv = ev: .nz = -1: End With
    With verts(1): .x = w2: .y = h1: .tu = eu: .tv = ev: .nz = -1: End With
    With verts(2): .x = w2: .y = h2: .tu = eu: .tv = sv: .nz = -1: End With
    With verts(3): .x = w1: .y = h2: .tu = su: .tv = sv: .nz = -1: End With
    'g_dev.SetVertexShader D3DFVF_VERTEX
    'g_dev.DrawPrimitiveUP D3DPT_TRIANGLEFAN, 2, verts(0), Len(verts(0))
    
    
    With verts(0): .z = 0.01: .x = w2: .y = h1: .tu = su: .tv = ev: .nz = 1: End With
    With verts(1): .z = 0.01: .x = w1: .y = h1: .tu = eu: .tv = ev: .nz = 1: End With
    With verts(2): .z = 0.01: .x = w1: .y = h2: .tu = eu: .tv = sv: .nz = 1: End With
    With verts(3): .z = 0.01: .x = w2: .y = h2: .tu = su: .tv = sv: .nz = 1: End With
    'g_dev.SetVertexShader D3DFVF_VERTEX
    'g_dev.DrawPrimitiveUP D3DPT_TRIANGLEFAN, 2, verts(0), Len(verts(0))

End Sub

Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)
    m_bKey(KeyCode) = True
End Sub

Private Sub Form_KeyUp(KeyCode As Integer, Shift As Integer)
    m_bKey(KeyCode) = False
End Sub


Private Sub Form_Load()


    Me.Show
    DoEvents
    

    m_mediadir = FindMediaDir("bargraphdata.csv")
    D3DUtil_SetMediaPath m_mediadir
    
    Init Me.hwnd, Me.font, Command1.font
    
    'Start the timers and callbacks
    Call DXUtil_Timer(TIMER_start)
    Timer1.Enabled = True

    

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
    
    'remove references to FONTs
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


Private Sub MENU_LOAD_Click()
    Dim sFile As String
    
    
    'Stop the timers and callbacks
    Timer1.Enabled = False
    
    
    
    CommonDialog1.FileName = ""
    CommonDialog1.DefaultExt = "csv"
    CommonDialog1.filter = "csv|*.csv"
    CommonDialog1.InitDir = m_mediadir
    
    
    'On Local Error Resume Next
    CommonDialog1.ShowOpen
    sFile = CommonDialog1.FileName
    If sFile = "" Then Exit Sub
    LoadFileAsBarGraph sFile
    
    
    D3DUtil_Destory
    DestroyDeviceObjects
            
    D3DUtil_Init Me.hwnd, True, 0, 0, D3DDEVTYPE_HAL, Nothing
    InitDeviceObjects
    ComputeDataExtents
    BuildGraph
    RestoreDeviceObjects
    
    'restart the callbacks
    DXUtil_Timer (TIMER_RESET)
    DXUtil_Timer (TIMER_start)
    Timer1.Enabled = True
End Sub

Private Sub MENU_RESET_Click()
    m_graphroot.SetMatrix g_identityMatrix
    m_vPosition = vec3(0, 0, -20)
    m_fYaw = 0
    m_fPitch = 0

    Call D3DXMatrixTranslation(m_matOrientation, 0, 0, 0)
    D3DUtil_SetupDefaultScene
    g_dev.GetTransform D3DTS_VIEW, m_matView
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

Sub LoadFileAsBarGraph(sFile As String)
    
    
    If Dir$(sFile) = "" Then
        MsgBox "Unable to find " & sFile
        Exit Sub
    End If
    
    Dim fl As Long
    Dim strIn As String
    Dim strTrim As String
    Dim strFirstChar As String
    Dim splitArray
    Dim cols As Long
    Dim bFoundData As Boolean
    Dim bFoundHeader As Boolean
    Dim sName As String
    Dim x As Double
    Dim y As Double
    Dim z As Double
    Dim i As Long
    Dim olddata As Collection
    Dim oldcolLabels As Collection
    Dim oldRowLabels As Collection
    Dim oldCols As Long
    Dim oldRows As Long
    Dim strRowLabel As String
    Dim strColLabel As String
    Dim valout As Variant
    Dim strName As String
    Dim sizeout As Single
    Dim colorout As Long
    
    fl = FreeFile
        
    'On Local Error GoTo errOut
    
    Set olddata = m_data
    Set oldcolLabels = m_ColLabels
    Set oldRowLabels = m_RowLabels
    oldCols = m_cols
    oldRows = m_rows
    
    Set m_data = Nothing
    Set m_data = New Collection
    
    m_cols = 0
    m_rows = 0
    
    Set m_ColLabels = New Collection
    Set m_RowLabels = New Collection
    
    Open sFile For Input As fl
        
    Do While Not EOF(fl)
        Line Input #fl, strIn
        strTrim = Trim(strIn)
        
        'skip comment lines
        strFirstChar = Mid$(strTrim, 1, 1)
        If strFirstChar = "#" Or strFirstChar = ";" Then GoTo nextLine
        If strTrim = "" Then GoTo nextLine
        
        splitArray = Split(strTrim, ",")
        
        cols = UBound(splitArray) + 1
        If cols < 2 Then
            MsgBox "Comma delimited file must have at least a header row, header column, and data"
            GoTo closeOut
        End If
                
        Dim strData As String
        Dim q As Long
        
        'If we have not found numbers see if we found a header row
        If Not bFoundData Then
            If IsNumeric(splitArray(1)) = False Then
                
                'assume data is a header row
                m_cols = cols
                
                m_GraphTitle = CStr(splitArray(0))
                                
                ReDim m_ColTextures(UBound(splitArray))
                
                For i = 1 To m_cols - 1
                    strData = Trim(CStr(splitArray(i)))
                    strColLabel = strData
                    q = InStr(UCase(strData), "TEXTURE:")
                    If q <> 0 Then
                        m_ColTextures(i) = Mid$(strData, q + 8)
                        If q > 1 Then strColLabel = Mid$(strData, 1, q - 1)
                    End If
                    m_ColLabels.Add strColLabel
                Next
                bFoundHeader = True
                GoTo nextLine
            Else
                bFoundData = True
                If bFoundHeader = False Then
                    MsgBox "Comma delimited file must have first for be header row to label columns"
                    GoTo closeOut
                End If
            End If
        End If
        
        m_rows = m_rows + 1
        strData = Trim(splitArray(0))
        strRowLabel = strData
        q = InStr(UCase(strData), "TEXTURE:")
        ReDim Preserve m_RowTextures(m_rows)
        If q <> 0 Then
            m_RowTextures(m_rows) = Mid$(strData, q + 8)
            If q > 1 Then strRowLabel = Mid$(strData, 1, q - 1)
        End If
        
        m_RowLabels.Add strRowLabel
        
        sizeout = 1
        
        
        For i = 1 To m_cols - 1
            colorout = D3DCOLORVALUEtoLONG(ColorValue4(1, 1 - (2 + m_rows Mod 4) / 10, 0.2, 1 - ((i Mod 8)) / 10))
            strColLabel = m_ColLabels.item(i)
            valout = splitArray(i)
            strName = "(" & strRowLabel & "," & strColLabel & ") = " & CStr(valout)
            AddEntry strName, CDbl(i - 1), val(valout), CDbl(m_rows - 1), CDbl(sizeout), colorout, ""
        Next
        
        
nextLine:
    Loop
    
    Set olddata = Nothing
    Close fl
    
    m_sizex = (kScale / m_cols) * 0.5
    m_sizez = (kScale / m_rows) * 0.5
    
    Exit Sub
    
errOut:
    MsgBox "there was an error loading " & sFile

closeOut:
    
    'restore state
    Set m_data = olddata
    Set m_ColLabels = oldcolLabels
    Set m_RowLabels = oldRowLabels
    m_rows = oldRows
    m_cols = oldCols
    
    Close fl
End Sub

Function CreateBoxWithTextureCoords(width As Single, height As Single, depth As Single) As D3DXMesh
    Dim mesh As CD3DMesh
    Dim retd3dxMesh As D3DXMesh
    Dim vertexbuffer As Direct3DVertexBuffer8
    Dim verts(28) As D3DVERTEX
    Dim indices(36) As Integer
    Dim w As Single, d As Single, h1 As Single, h2 As Single
    w = width / 2
    h2 = height / 2
    h1 = -height / 2
    d = depth / 2
    
    'Create an empty d3dxmesh with room for 12 vertices and 12
    Set retd3dxMesh = g_d3dx.CreateMeshFVF(4 * 6, 6 * 6, D3DXMESH_MANAGED, D3DFVF_VERTEX, g_dev)
    
    
    'front face
    
    'add vertices
    With verts(0): .x = -w: .y = h2: .z = -d: .nz = 1: .tu = 0: .tv = 0: End With
    With verts(1): .x = w: .y = h2: .z = -d: .nz = 1: .tu = 1: .tv = 0: End With
    With verts(2): .x = w: .y = h1: .z = -d: .nz = 1: .tu = 1: .tv = 1: End With
    With verts(3): .x = -w: .y = h1: .z = -d: .nz = 1: .tu = 0: .tv = 1: End With
    
    'connect verices to make 2 triangles per face
    indices(0) = 0: indices(1) = 1: indices(2) = 2
    indices(3) = 0: indices(4) = 2: indices(5) = 3
    
    'back face
    With verts(4): .x = -w: .y = h1: .z = d: .nz = -1: .tu = 0: .tv = 1: End With
    With verts(5): .x = w: .y = h1: .z = d: .nz = -1: .tu = 1: .tv = 1: End With
    With verts(6): .x = w: .y = h2: .z = d: .nz = -1: .tu = 1: .tv = 0: End With
    With verts(7): .x = -w: .y = h2: .z = d: .nz = -1: .tu = 0: .tv = 0: End With
    indices(6) = 4: indices(7) = 5: indices(8) = 6
    indices(9) = 4: indices(10) = 6: indices(11) = 7
    
    'right face
    With verts(8): .x = w: .y = h1: .z = -d: .nx = -1: .tu = 0: .tv = 0: End With
    With verts(9): .x = w: .y = h1: .z = d: .nx = -1: .tu = 1: .tv = 0: End With
    With verts(10): .x = w: .y = h2: .z = d: .nx = -1: .tu = 1: .tv = 1: End With
    With verts(11): .x = w: .y = h2: .z = -d: .nx = -1: .tu = 0: .tv = 1: End With
    indices(12) = 8: indices(13) = 9: indices(14) = 10
    indices(15) = 8: indices(16) = 10: indices(17) = 11
    
    'left face
    With verts(16): .x = -w: .y = h2: .z = -d: .nx = 1: .tu = 0: .tv = 1: End With
    With verts(17): .x = -w: .y = h2: .z = d: .nx = 1: .tu = 1: .tv = 1: End With
    With verts(18): .x = -w: .y = h1: .z = d: .nx = 1: .tu = 1: .tv = 0: End With
    With verts(19): .x = -w: .y = h1: .z = -d: .nx = 1: .tu = 0: .tv = 0: End With
    indices(18) = 16: indices(19) = 17: indices(20) = 18
    indices(21) = 16: indices(22) = 18: indices(23) = 19
    
    'top face
    With verts(20): .x = -w: .y = h2: .z = -d: .ny = -1: .tu = 0: .tv = 0: End With
    With verts(21): .x = -w: .y = h2: .z = d: .ny = -1: .tu = 1: .tv = 0: End With
    With verts(22): .x = w: .y = h2: .z = d: .ny = -1: .tu = 1: .tv = 1: End With
    With verts(23): .x = w: .y = h2: .z = -d: .ny = -1: .tu = 0: .tv = 1: End With
    indices(24) = 20: indices(25) = 21: indices(26) = 22
    indices(27) = 20: indices(28) = 22: indices(29) = 23
        
    'bottom  face
    With verts(24): .x = w: .y = h1: .z = -d: .ny = 1: .tu = 0: .tv = 1: End With
    With verts(25): .x = w: .y = h1: .z = d: .ny = 1: .tu = 1: .tv = 1: End With
    With verts(26): .x = -w: .y = h1: .z = d: .ny = 1: .tu = 1: .tv = 0: End With
    With verts(27): .x = -w: .y = h1: .z = -d: .ny = 1: .tu = 0: .tv = 0: End With
    indices(30) = 24: indices(31) = 25: indices(32) = 26
    indices(33) = 24: indices(34) = 26: indices(35) = 27
        
    
    D3DXMeshVertexBuffer8SetData retd3dxMesh, 0, Len(verts(0)) * 28, 0, verts(0)
    D3DXMeshIndexBuffer8SetData retd3dxMesh, 0, Len(indices(0)) * 36, 0, indices(0)
        
        
    
    Set CreateBoxWithTextureCoords = retd3dxMesh
End Function
 
Sub LoadTexture(i As Long, strFile As String)
        
    If strFile = "" Then Exit Sub
    
    Set m_LabelTex(i) = D3DUtil.D3DUtil_CreateTextureInPool(g_dev, strFile, D3DFMT_R5G6B5)
    If m_LabelTex(i) Is Nothing Then
        MsgBox "Unable to find " & strFile
    End If
End Sub
