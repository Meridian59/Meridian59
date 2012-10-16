VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "cRoom"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit
Private Const mnRoomX As Single = 0
Private Const mnRoomY As Single = 2
Private Const mnRoomZ As Single = 0

Private Const mnBarRoomX As Single = 0
Private Const mnBarRoomY As Single = -5
Private Const mnBarRoomZ As Single = 0

Private Const mnLobbyScaleX As Single = 8
Private Const mnLobbyScaleY As Single = 5
Private Const mnLobbyScaleZ As Single = 9

Private moRoom As CD3DFrame               ' Our Room frame
Private moOfficeRoom As CD3DFrame               ' Our Room frame

Public DrawRoom As Boolean 'Should we draw the room at all
Public BarRoom As Boolean 'Should we draw the bar or the MS lobby

'Methods
Public Sub Init(ByVal sMedia As String, sRoom As String, sLobby As String)
    Set moRoom = D3DUtil_LoadFromFile(AddDirSep(sMedia) & sRoom, Nothing, Nothing)
    Set moOfficeRoom = D3DUtil_LoadFromFile(AddDirSep(sMedia) & sLobby, Nothing, Nothing)
End Sub

Public Sub CleanupFrame()
    If Not (moRoom Is Nothing) Then moRoom.Destroy
    If Not (moOfficeRoom Is Nothing) Then moOfficeRoom.Destroy
    Set moRoom = Nothing
    Set moOfficeRoom = Nothing
End Sub

Public Sub Render(dev As Direct3DDevice8)
    Dim matRoom As D3DMATRIX
    Dim matScale As D3DMATRIX
    
    If DrawRoom Then
        If BarRoom Then
            'First the room
            D3DXMatrixIdentity matRoom
            D3DXMatrixTranslation matRoom, mnBarRoomX, mnBarRoomY, mnBarRoomZ
            moRoom.SetMatrix matRoom
            moRoom.Render g_dev
        Else
            'First the room
            D3DXMatrixIdentity matRoom
            D3DXMatrixTranslation matRoom, mnRoomX, mnRoomY, mnRoomZ
            D3DXMatrixScaling matScale, mnLobbyScaleX, mnLobbyScaleY, mnLobbyScaleZ
            D3DXMatrixMultiply matRoom, matRoom, matScale
            moOfficeRoom.SetMatrix matRoom
            moOfficeRoom.Render g_dev
        End If
    End If
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
    
    If Not DrawRoom Then Exit Function
    fDoneFading = True
    If BarRoom Then
        Set oMesh = moRoom.FindChildObject("room", 0)
    Else
        Set oMesh = moOfficeRoom.FindChildObject("Unnamed_0", 0)
    End If
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
    DrawRoom = True
    Set moRoom = Nothing
    Set moOfficeRoom = Nothing
End Sub

Private Sub Class_Terminate()
    If Not (moRoom Is Nothing) Then moRoom.Destroy
    If Not (moOfficeRoom Is Nothing) Then moOfficeRoom.Destroy
    
    Set moRoom = Nothing
    Set moOfficeRoom = Nothing
End Sub
