VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "cTable"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

'Here we will encapsulate all of the code needed for the table
'Local variables for the properties of the table
Private moPosition As D3DVECTOR 'Current position of the table
Private moTable As CD3DFrame 'D3D Mesh for the table
Private mlTransparantPaddle As Boolean
Public DrawTable As Boolean ' You can also turn off the table (dunno why, but let'em)

'Position property
Public Property Let Position(oPos As D3DVECTOR)
    moPosition = oPos
End Property

Public Property Get Position() As D3DVECTOR
    Position = moPosition
End Property

'Transparent property
Public Property Let Transparent(ByVal fTrans As Boolean)
    Dim oMesh As CD3DMesh, oMaterial As D3DMATERIAL8
    Dim lNumMaterial As Long, lCount As Long
    
    mlTransparantPaddle = fTrans
    'now set the property
    Set oMesh = moTable.FindChildObject("table", 0)
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
    Set moTable = D3DUtil_LoadFromFile(AddDirSep(sMedia) & sFile, Nothing, Nothing)
End Sub

Public Sub Render(dev As Direct3DDevice8)
    Dim matTable As D3DMATRIX
    If DrawTable Then
        'Now the table
        D3DXMatrixIdentity matTable
        D3DXMatrixTranslation matTable, moPosition.X, moPosition.Y, moPosition.z
        moTable.SetMatrix matTable
        moTable.Render g_dev
    End If
End Sub

Public Sub CleanupFrame()
    moTable.Destroy
    Set moTable = Nothing
End Sub

Public Function FadeMesh(FadeInterval As Single) As Boolean
    Dim lNumMaterial As Long
    Dim lCount As Long
    Dim oMaterial As D3DMATERIAL8
    Dim fDoneFading As Boolean
    Dim oMesh As CD3DMesh
    Dim nInternalInterval As Single
    Static lFadeTime As Long
    
    FadeMesh = True
    nInternalInterval = FadeInterval
    If lFadeTime = 0 Then
        lFadeTime = timeGetTime
        Exit Function 'We'll do the fade next render pass
    End If
    nInternalInterval = (((timeGetTime - lFadeTime) / 1000000) * nInternalInterval)
    
    fDoneFading = True
    If Not DrawTable Then Exit Function
    Set oMesh = moTable.FindChildObject("table", 0)
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
    DrawTable = True
    Set moTable = Nothing
End Sub

Private Sub Class_Terminate()
    If Not moTable Is Nothing Then moTable.Destroy
    Set moTable = Nothing
End Sub
