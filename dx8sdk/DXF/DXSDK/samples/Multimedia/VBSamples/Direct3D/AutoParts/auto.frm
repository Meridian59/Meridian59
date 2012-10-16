VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form Form1 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Auto Collision Parts Database"
   ClientHeight    =   8520
   ClientLeft      =   480
   ClientTop       =   615
   ClientWidth     =   10875
   Icon            =   "auto.frx":0000
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   568
   ScaleMode       =   3  'Pixel
   ScaleWidth      =   725
   Begin MSComctlLib.TreeView TreeView1 
      Height          =   3495
      Left            =   120
      TabIndex        =   27
      Top             =   480
      Width           =   3975
      _ExtentX        =   7011
      _ExtentY        =   6165
      _Version        =   393217
      HideSelection   =   0   'False
      Style           =   7
      Appearance      =   1
   End
   Begin VB.TextBox Text8 
      Enabled         =   0   'False
      Height          =   375
      Left            =   9000
      TabIndex        =   25
      Top             =   7380
      Width           =   1695
   End
   Begin VB.PictureBox Picture2 
      Height          =   735
      Left            =   120
      ScaleHeight     =   675
      ScaleWidth      =   10515
      TabIndex        =   19
      Top             =   4080
      Width           =   10575
      Begin VB.Label Label9 
         Caption         =   $"auto.frx":0442
         Height          =   495
         Left            =   120
         TabIndex        =   20
         Top             =   120
         Width           =   9495
      End
   End
   Begin VB.CommandButton Command1 
      Caption         =   "Add To Invoice"
      Height          =   495
      Left            =   120
      TabIndex        =   16
      Top             =   7920
      Width           =   3975
   End
   Begin MSComctlLib.ListView ListView1 
      Height          =   2355
      Left            =   4320
      TabIndex        =   15
      Top             =   4920
      Width           =   6375
      _ExtentX        =   11245
      _ExtentY        =   4154
      View            =   3
      LabelWrap       =   -1  'True
      HideSelection   =   0   'False
      _Version        =   393217
      ForeColor       =   -2147483640
      BackColor       =   -2147483643
      BorderStyle     =   1
      Appearance      =   1
      NumItems        =   5
      BeginProperty ColumnHeader(1) {BDD1F052-858B-11D1-B16A-00C0F0283628} 
         Key             =   "price"
         Text            =   "DESCRIPTION"
         Object.Width           =   5821
      EndProperty
      BeginProperty ColumnHeader(2) {BDD1F052-858B-11D1-B16A-00C0F0283628} 
         SubItemIndex    =   1
         Key             =   "part"
         Text            =   "PRICE"
         Object.Width           =   2117
      EndProperty
      BeginProperty ColumnHeader(3) {BDD1F052-858B-11D1-B16A-00C0F0283628} 
         SubItemIndex    =   2
         Key             =   "id"
         Text            =   "ID"
         Object.Width           =   2117
      EndProperty
      BeginProperty ColumnHeader(4) {BDD1F052-858B-11D1-B16A-00C0F0283628} 
         SubItemIndex    =   3
         Key             =   "modid"
         Text            =   "MODID"
         Object.Width           =   0
      EndProperty
      BeginProperty ColumnHeader(5) {BDD1F052-858B-11D1-B16A-00C0F0283628} 
         SubItemIndex    =   4
         Object.Width           =   38100
      EndProperty
   End
   Begin VB.CommandButton Command3 
      Caption         =   "Process Order"
      Height          =   495
      Left            =   7680
      TabIndex        =   3
      Top             =   7920
      Width           =   3015
   End
   Begin VB.CommandButton Command2 
      Caption         =   "Remove From Invoice"
      Height          =   495
      Left            =   4320
      TabIndex        =   2
      Top             =   7920
      Width           =   3135
   End
   Begin VB.PictureBox largepict 
      Height          =   3495
      Left            =   4320
      ScaleHeight     =   229
      ScaleMode       =   3  'Pixel
      ScaleWidth      =   421
      TabIndex        =   1
      Top             =   480
      Width           =   6375
   End
   Begin VB.PictureBox Picture1 
      Height          =   2835
      Left            =   120
      ScaleHeight     =   2775
      ScaleWidth      =   3915
      TabIndex        =   0
      Top             =   4920
      Width           =   3975
      Begin VB.TextBox Text7 
         Enabled         =   0   'False
         Height          =   285
         Left            =   1440
         TabIndex        =   26
         Top             =   2400
         Width           =   1695
      End
      Begin VB.TextBox Text6 
         Enabled         =   0   'False
         Height          =   285
         Left            =   1440
         TabIndex        =   24
         Top             =   2040
         Width           =   1695
      End
      Begin VB.TextBox Text5 
         Enabled         =   0   'False
         Height          =   285
         Left            =   1440
         TabIndex        =   13
         Top             =   1680
         Width           =   1695
      End
      Begin VB.TextBox Text4 
         Enabled         =   0   'False
         Height          =   285
         Left            =   1440
         TabIndex        =   11
         Top             =   1320
         Width           =   1695
      End
      Begin VB.TextBox Text3 
         Enabled         =   0   'False
         Height          =   285
         Left            =   1440
         TabIndex        =   9
         Top             =   960
         Width           =   1695
      End
      Begin VB.TextBox Text2 
         Enabled         =   0   'False
         Height          =   285
         Left            =   1440
         TabIndex        =   7
         Top             =   600
         Width           =   1695
      End
      Begin VB.TextBox Text1 
         Enabled         =   0   'False
         Height          =   285
         Left            =   1440
         TabIndex        =   4
         Top             =   240
         Width           =   1695
      End
      Begin VB.Label Label12 
         Caption         =   "MAKE"
         Height          =   255
         Left            =   120
         TabIndex        =   23
         Top             =   2400
         Width           =   735
      End
      Begin VB.Label Label11 
         Caption         =   "Label11"
         Height          =   15
         Left            =   240
         TabIndex        =   22
         Top             =   2640
         Width           =   735
      End
      Begin VB.Label Label10 
         Caption         =   "STOCK"
         Height          =   255
         Left            =   120
         TabIndex        =   21
         Top             =   2040
         Width           =   1095
      End
      Begin VB.Label Label5 
         Caption         =   "ASSEMBLY"
         Height          =   375
         Left            =   120
         TabIndex        =   12
         Top             =   1680
         Width           =   1335
      End
      Begin VB.Label Label4 
         Caption         =   "COMPAT PARTS"
         Height          =   255
         Left            =   120
         TabIndex        =   10
         Top             =   1320
         Width           =   1335
      End
      Begin VB.Label Label3 
         Caption         =   "PRICE"
         Height          =   255
         Left            =   120
         TabIndex        =   8
         Top             =   960
         Width           =   1095
      End
      Begin VB.Label Label2 
         Caption         =   "DESCRIPTION"
         Height          =   375
         Left            =   120
         TabIndex        =   6
         Top             =   600
         Width           =   1215
      End
      Begin VB.Label Label1 
         Caption         =   "PARTID"
         Height          =   255
         Left            =   120
         TabIndex        =   5
         Top             =   240
         Width           =   735
      End
   End
   Begin VB.Label Label8 
      Caption         =   "Select Assembly"
      Height          =   255
      Left            =   120
      TabIndex        =   18
      Top             =   120
      Width           =   2895
   End
   Begin VB.Label Label7 
      Caption         =   "Click On a Part From Assembly - Use the mouse to Rotate the Assembly"
      Height          =   255
      Left            =   4320
      TabIndex        =   17
      Top             =   120
      Width           =   5895
   End
   Begin VB.Label Label6 
      Caption         =   "TOTAL"
      Height          =   255
      Left            =   4440
      TabIndex        =   14
      Top             =   7380
      Width           =   1455
   End
   Begin VB.Menu MENU_FILE 
      Caption         =   "&File"
      Begin VB.Menu MENU_EXIT 
         Caption         =   "E&xit"
      End
   End
   Begin VB.Menu MENU_HELP 
      Caption         =   "&Help"
      Begin VB.Menu MENU_ABOUT 
         Caption         =   "&About..."
      End
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

'The model used by this sample, engine1.x, is provided courtesy of Viewpoint
'Digital, Inc. (www.viewpoint.com).  It is provided for use with this sample
'only and cannot be distributed with any application without prior written
'consent.  V6 Engine Model copyright 1999 Viewpoint Digital, Inc..

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       auto.frm
'  Content:    Example of display and picking geometry
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Dim m_assemblies(100) As CD3DFrame
Dim m_assemblyName(100) As String
Dim m_nAssembly As Long

Dim m_scene As CD3DFrame
Dim m_root As CD3DFrame
Dim m_bMouseDown As Boolean
Dim m_lastX As Integer
Dim m_lastY As Integer
Dim m_current As CD3DFrame

Dim m_bInLoad As Boolean
Dim m_binit As Boolean
Dim m_data As New Data
Dim fLoading As Boolean
Dim m_backcolor As Long
Dim m_mediadir As String

Implements DirectXEvent8



Private Sub DirectXEvent8_DXCallback(ByVal eventid As Long)
    Dim b As Boolean
    
End Sub

'- Form_Load
'
'  Initialize the D3DUtil Framework
'  Initialize the parts info text database
'  Initialize the treeview control

Private Sub Form_Load()
    Dim b As Boolean
    
    Me.Show
    DoEvents
    
    ' Initialize D3D Window
    b = D3DUtil_DefaultInitWindowed(0, largepict.hwnd)
    If b = False Then
        MsgBox "Exiting, Unable to initialize 3D device"
        End
    End If
    
    'Add some default light and turn on lighting
    g_lWindowWidth = largepict.ScaleWidth
    g_lWindowHeight = largepict.ScaleHeight
    D3DUtil.D3DUtil_SetupDefaultScene
    
    'Find Media Directory
    m_mediadir = FindMediaDir("partstable.txt", False)
    
    'Open Text Database
    m_data.InitData m_mediadir + "partstable.txt"
               
    'Save our initial background color
    m_backcolor = &HFF90D090
  
    'Fill the Tree view with its root node
    FillTreeViewControl
    
        
End Sub

     
'- SelectPart
'
'  fill in the text boxes given a certain identifier
'  from a model. We query the database for the identifier
'  and from there we get the rest of the info
Sub SelectPart(strName As String, strObject As String)
   
   If m_data.MoveToModelPartRecord(strName) = False Then Exit Sub
   
   Text1.Text = m_data.PartID
   Text2.Text = m_data.Description
   Text3.Text = format$(m_data.Price, "#0.00")
   Text4.Text = m_data.CompatibleParts
   Text5.Text = "Engine"
   Text6.Text = m_data.Stock
   Text7.Text = m_data.PartMake
   
   If Not m_root Is Nothing Then
        
        'Turn the selected object red
        If Not m_current Is Nothing Then
            With m_current.GetChildMesh(0)
                .bUseMaterials = True
                .bUseMaterialOverride = False
            End With
        End If
        
        Set m_current = m_scene.FindChildObject(strObject, 0)
        
        If Not (m_current Is Nothing) Then
            Dim mat As D3DMATERIAL8
            With m_current.GetChildMesh(0)
                .bUseMaterials = False
                .bUseMaterialOverride = True
                mat.emissive.r = 0.5
                mat.emissive.a = 1
                mat.diffuse.r = 0.3
                mat.diffuse.a = 1
                .SetMaterialOverride mat
            End With
        End If
    End If
   
   
errOut:
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
    delta_y = y - m_lastY
    m_lastX = x
    m_lastY = y

            
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
    m_scene.UpdateFrames
    axisS = m_root.InverseTransformCoord(wc)
        
    'transform origen camera space to world coordinates
    'm_largewindow.m_cameraFrame.Transform  wc, origin
    D3DXVec3TransformCoord wc, origin, g_viewMatrix
    
    'transfer cam space origen to model space
    base = m_root.InverseTransformCoord(wc)
    
    axisS.x = axisS.x - base.x
    axisS.y = axisS.y - base.y
    axisS.z = axisS.z - base.z
    
    m_root.AddRotation COMBINE_BEFORE, axisS.x, axisS.y, axisS.z, angle
    
End Sub

'- LoadAssembly
'
'  See if we have the assembly loaded
'  if not figure out which model to use from a db
'  and load it
'  by default it will attach it to the scene
Function LoadAssembly(sname As String) As Long
    
    
    Dim i As Long
    Dim strCap As String
    Dim strModel As String
    
    Static b As Boolean
    
    If b = True Then Exit Function
    b = True
    
    'make sure we dont habe it already
    For i = 1 To m_nAssembly
        If sname = m_assemblyName(i) Then
            LoadAssembly = i
            b = False
            Exit Function
        End If
    Next
    
    
    m_nAssembly = m_nAssembly + 1
    m_assemblyName(m_nAssembly) = sname
    
    
    'look up the model we need to load
    'for this example we only show 1 model
    'but one could query for the files from a database
    strModel = "engine1.x"
    strCap = Me.Caption
    Me.Caption = "Loading- please wait"
    DoEvents
    Err.Number = 0

    Form2.Top = Me.Top + Me.height / 4
    Form2.Left = Me.Left + Me.width / 8
    Form2.Show
    DoEvents
    
    Set m_assemblies(m_nAssembly) = New CD3DFrame
    b = m_assemblies(m_nAssembly).InitFromFile(g_dev, m_mediadir + strModel, Nothing, Nothing)
        
    
    If b = False Then
        Set m_assemblies(m_nAssembly) = Nothing
        m_assemblyName(m_nAssembly) = ""
        m_nAssembly = m_nAssembly - 1
        Unload Form2
        Me.Caption = strCap
        GoTo errOut
    End If
    Me.Caption = strCap
    
    m_assemblies(m_nAssembly).SetFVF g_dev, D3DFVF_VERTEX
    m_assemblies(m_nAssembly).ComputeNormals
    
    g_dev.SetRenderState D3DRS_AMBIENT, &H90909090
    
    
    'Release the previous scene
    Set m_scene = Nothing
    Set m_root = Nothing
    Set m_current = Nothing
    
    'Create a root object for the scene
    Set m_scene = New CD3DFrame

    'Create a new root object to use for rotation matrix
    Set m_root = D3DUtil_CreateFrame(m_scene)
    
    
    
    'Add our assembly to the tree
    m_root.AddChild m_assemblies(m_nAssembly)
       
    
    'Position our assembly
    m_assemblies(m_nAssembly).AddTranslation COMBINE_replace, 0, 0, 5
    
    
    
    'Recolor m_assemblies(m_nAssembly)
        
    LoadAssembly = m_nAssembly
    
    
    Unload Form2
    DoEvents
    If fLoading Then End
    
    RenderScene
    
    DoEvents
    
    Set m_root = m_assemblies(m_nAssembly)
    
    
    m_binit = True
    
errOut:
    
    
    b = False
    
    
    TreeView1.Enabled = True
    largepict.SetFocus
    DoEvents
    
End Function

     
' Command1_Click
' Add To Invoice
'
Private Sub Command1_Click()

    Dim itm As ListItem
    If Text1.Text = "" Then Exit Sub
    
    Set itm = ListView1.ListItems.Add(, , Text2.Text)
    itm.SubItems(1) = Text3.Text
    itm.SubItems(2) = Text1.Text
    Set ListView1.SelectedItem = itm
    itm.EnsureVisible
    Text8.Text = format(val(Text8.Text) + val(Text3.Text), "#0.00")
    
End Sub


' Command1_Click
' Delete from Invoice
'
Private Sub Command2_Click()
    If ListView1.SelectedItem Is Nothing Then Exit Sub
    
    Text8 = format(val(Text8.Text) - val(ListView1.SelectedItem.SubItems(1)), "#0.00")
    ListView1.ListItems.Remove ListView1.SelectedItem.index
End Sub

       
    
' Form_QueryUnload
'
Private Sub Form_QueryUnload(Cancel As Integer, UnloadMode As Integer)
    fLoading = True
End Sub



'- MouseDown
'
Private Sub largepict_MouseDown(Button As Integer, Shift As Integer, x As Single, y As Single)
    
    If m_binit = False Then Exit Sub
    
    Dim b As Boolean
    Dim mb As CD3DMesh
    Dim r As Integer, c As Integer
    Dim f As CD3DFrame
    Dim p As CD3DFrame
    Dim strName As String
    Dim pick As CD3DPick
    Dim n As Long
    
    '- save our current position
    m_bMouseDown = True
    m_lastX = x
    m_lastY = y
    
    
    If Button = 1 Then
        
        'Get the frame under the the mouse
        Set pick = New CD3DPick
        If Not pick.ViewportPick(m_scene, x, y) Then Exit Sub
                       
                        
        n = pick.FindNearest()
        If n < 0 Then Exit Sub
        Set f = pick.GetFrame(n)
        
                
        'Get its id and call SelectPart
        'to fill in our text boxes
        strName = f.ObjectName
        strName = Right$(strName, Len(strName) - 1)
        'The words V6 and Chevy are part of the manifold cover.
        If strName = "words" Or strName = "v6" Then strName = "manifoldt"
        
        SelectPart strName, f.ObjectName
        SelectTreeview strName
        DoEvents
        
    End If
    
    RenderScene
    
End Sub

'- MOUSE MOVE
'
Private Sub largepict_MouseMove(Button As Integer, Shift As Integer, x As Single, y As Single)
    
    '- dont do anything unless the mouse is down
    If m_bMouseDown = False Then
        Exit Sub
    End If
    
    '- Rotate the object
    RotateTrackBall CInt(x), CInt(y)
    
    '- Rerender
    RenderScene
        
End Sub

'- MOUSE UP
'  reset the mouse state
'
Private Sub largepict_MouseUp(Button As Integer, Shift As Integer, x As Single, y As Single)
    m_bMouseDown = False
End Sub

'- largepict_Paint UP
'
Private Sub largepict_Paint()
    If Not m_binit Then Exit Sub
    RenderScene
End Sub


'- MENU_ABOUT_Click
'
Private Sub MENU_ABOUT_Click()
    MsgBox "The model used by this sample, engine1.x, is provided courtesy of Viewpoint" + Chr(10) + Chr(13) + _
        "Digital, Inc. (www.viewpoint.com).  It is provided for use with this sample" + Chr(10) + Chr(13) + _
        "only and cannot be distributed with any application without prior written" + Chr(10) + Chr(13) + _
        "consent.  V6 Engine Model copyright 1999 Viewpoint Digital, Inc.."
End Sub

' MENU_EXIT_Click
'
Private Sub MENU_EXIT_Click()
    End
End Sub


' TreeView1_Expand
'
Private Sub TreeView1_Expand(ByVal Node As MSComctlLib.Node)
    Dim i As Long
    
    Static b As Boolean
        
    If b Then Exit Sub
    b = True
        
    'See if they are asking for a new assembly alltogether
    If Mid$(Node.Tag, 1, 8) = "ASSMBLY:" Then
        m_bInLoad = True
        i = LoadAssembly(Node.Tag)
        If i = 0 Then
            MsgBox "Assembly not available at this time- try a different Engine"
            b = False
            Exit Sub
        End If
        
    End If
    
    b = False
    
End Sub

'- TreeView1_NodeClick
'
Private Sub TreeView1_NodeClick(ByVal Node As MSComctlLib.Node)
            
    Static b As Boolean
    If b Then Exit Sub
    b = True

    Dim o As CD3DFrame
    Dim i  As Long
    
    If Node.Tag = "" Then
        b = False
        Exit Sub
    End If
    
    'Fill in the text boxes
    SelectPart Node.Tag, "_" & Node.Tag
    DoEvents
        
    'Render
    RenderScene
    DoEvents
    
    b = False
End Sub



'- FillTreeViewControl
Sub FillTreeViewControl()
    TreeView1.Nodes.Clear
    
    Dim sPartID As String
    Dim sDesc As String
    
    
    'A non-demo application would build the tree view
    'from the database and dynamically load in new
    'information into the treeview
    
    Dim n As Node
    Call TreeView1.Nodes.Add(, , "ASSEMBLIES", "Assemblies - [click here to start]")
    
    Set n = TreeView1.Nodes.Add("ASSEMBLIES", tvwChild, "ENG V6 1996", "V6 4 Liter 1996 - [click here]")
    n.Tag = "ASSMBLY:ENG V6 1996"
    n.Selected = True
    
    TreeView1.Nodes.Add("ASSEMBLIES", tvwChild, "ENG V8 1998", "V8 6 Liter 1998 - [not available]").Tag = ""
    TreeView1.Nodes.Add("ASSEMBLIES", tvwChild, "OTHERENG", "Other Assemblies not available").Tag = ""
    
    m_data.MoveTop
    Do While m_data.IsEOF() = False
        sPartID = m_data.ModelPart
        sDesc = m_data.Description
        TreeView1.Nodes.Add("ENG V6 1996", tvwChild, sPartID, sDesc).Tag = sPartID
        m_data.MoveNext
    Loop
    
End Sub

Sub SelectTreeview(sname As String)
    On Local Error Resume Next
    TreeView1.Nodes(sname).Selected = True
    DoEvents
End Sub


'- RenderScene
'
Sub RenderScene()
    Dim hr As Long
    
    If m_scene Is Nothing Then Exit Sub
    
     
    
    'See what state the device is in.
    hr = g_dev.TestCooperativeLevel
    If hr = D3DERR_DEVICENOTRESET Then
        g_dev.Reset g_d3dpp
        
        'reset our state
        g_lWindowWidth = largepict.ScaleWidth
        g_lWindowHeight = largepict.ScaleHeight
        D3DUtil.D3DUtil_SetupDefaultScene
        DoEvents
    ElseIf hr <> 0 Then
        Exit Sub
    End If
    
    D3DXMatrixLookAtLH g_viewMatrix, vec3(0, 0, -1), vec3(0, 0, 0), vec3(0, 1, 0)
    g_dev.SetTransform D3DTS_VIEW, g_viewMatrix
 
    D3DUtil_ClearAll m_backcolor
    g_dev.BeginScene
    m_scene.Render g_dev
    g_dev.EndScene
    D3DUtil_PresentAll 0
    
End Sub
