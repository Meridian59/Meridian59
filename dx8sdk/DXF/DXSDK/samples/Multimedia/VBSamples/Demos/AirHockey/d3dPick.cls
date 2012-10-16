VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "CD3DPick"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       D3DPick.cls
'  Content:    D3D Visual Basic Framework Pick object
'              See raypack and viewport pick entrypoints
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Option Explicit

Dim m_item() As D3D_PICK_RECORD
Dim m_frame() As CD3DFrame
Dim m_mesh() As CD3DMesh
Dim m_count As Long
Dim m_maxsize As Long
Const kGrowSize = 10

'-----------------------------------------------------------------------------
' Name: GetCount
' Dest: returns number of items picked
'-----------------------------------------------------------------------------
Public Function GetCount() As Long
    GetCount = m_count
End Function

'-----------------------------------------------------------------------------
' Name: GetRecord
' Desc: returns the properties of a given pick item
'-----------------------------------------------------------------------------
Public Sub GetRecord(i As Long, ByRef a As Single, ByRef b As Single, ByRef dist As Single, ByRef triFaceid As Long)
    a = m_item(i).a
    b = m_item(i).b
    dist = m_item(i).dist
    triFaceid = m_item(i).triFaceid
End Sub

'-----------------------------------------------------------------------------
' Name: GetFrame
' Desc: returns the frame of a given pick item
'-----------------------------------------------------------------------------
Public Function GetFrame(i As Long) As CD3DFrame
    Set GetFrame = m_frame(i)
End Function

'-----------------------------------------------------------------------------
' Name: GetMesh
' Desc: returns the mesh of a given pick item
'-----------------------------------------------------------------------------
Public Function GetMesh(i As Long) As CD3DMesh
    Set GetMesh = m_mesh(i)
End Function

'-----------------------------------------------------------------------------
' Name: FindNearest
' Desc: returns the index of the pick with the smallest distance (closest to viewer)
'-----------------------------------------------------------------------------
Public Function FindNearest() As Long
    Dim q As Long, mindist As Single, i As Long
    q = -1
    mindist = 1E+38
    For i = 0 To m_count - 1
        If m_item(i).dist < mindist Then
            q = i
            mindist = m_item(i).dist
        End If
    Next
    FindNearest = q
End Function


'-----------------------------------------------------------------------------
' Name: FindFurthest
' Desc: returns the index of the pick with the largest distance
'-----------------------------------------------------------------------------
Public Function FindFurthest() As Long
    Dim q As Long, maxdist As Single, i As Long
    q = -1
    maxdist = -1E+38
    For i = 0 To m_count - 1
        If m_item(i).dist < maxdist Then
            q = i
            maxdist = m_item(i).dist
        End If
    Next
    FindFurthest = q
End Function


'-----------------------------------------------------------------------------
' Name: Destroy
' Desc: Release all references
'-----------------------------------------------------------------------------
Public Function Destroy()
    ReDim m_mesh(0)
    ReDim m_frame(0)
    m_count = 0
    m_maxsize = 0
    
End Function


'-----------------------------------------------------------------------------
' Name: ViewportPick
' Params:
'    frame      parent of frame heirarchy to pick from
'    x          x screen coordinate in pixels
'    y          y screen coordinate in pixels
'
' Note: After call GetCount to see if any objets where hit
'-----------------------------------------------------------------------------
Public Function ViewportPick(frame As CD3DFrame, X As Single, Y As Single)
    Destroy
    
    Dim viewport As D3DVIEWPORT8
    Dim world As D3DMATRIX
    Dim proj As D3DMATRIX
    Dim view As D3DMATRIX
    
    'NOTE the following functions will fail on PURE HAL devices
    'use ViewportPickEx if working with pureHal devices
    
    g_dev.GetViewport viewport
    world = g_identityMatrix
    g_dev.GetTransform D3DTS_VIEW, view
    g_dev.GetTransform D3DTS_PROJECTION, proj
    
    ViewportPick = ViewportPickEx(frame, viewport, proj, view, world, X, Y)
    
End Function

'-----------------------------------------------------------------------------
' Name: ViewportPickEx
' Desc: Aux function for ViewportPick
'-----------------------------------------------------------------------------
Public Function ViewportPickEx(frame As CD3DFrame, viewport As D3DVIEWPORT8, proj As D3DMATRIX, view As D3DMATRIX, world As D3DMATRIX, X As Single, Y As Single) As Boolean
    
    If frame.Enabled = False Then Exit Function
    
    Dim vIn As D3DVECTOR, vNear As D3DVECTOR, vFar As D3DVECTOR, vDir As D3DVECTOR
    Dim bHit As Boolean, i As Long
    
    If frame Is Nothing Then Exit Function
        
                        
    Dim currentMatrix As D3DMATRIX
    Dim NewWorldMatrix As D3DMATRIX
    
    currentMatrix = frame.GetMatrix
        
    'Setup our basis matrix for this frame
    D3DXMatrixMultiply NewWorldMatrix, currentMatrix, world
    
    vIn.X = X:    vIn.Y = Y
    
    'Compute point on Near Clip plane at cursor
    vIn.z = 0
    D3DXVec3Unproject vNear, vIn, viewport, proj, view, NewWorldMatrix
    
    'compute point on far clip plane at cursor
    vIn.z = 1
    D3DXVec3Unproject vFar, vIn, viewport, proj, view, NewWorldMatrix

    'Comput direction vector
    D3DXVec3Subtract vDir, vFar, vNear
                
    
    
    Dim item As D3D_PICK_RECORD
    
    
    'Check all child meshes
    'Even if we got a hit we continue as the next mesh may be closer
    Dim childMesh As CD3DMesh
    For i = 0 To frame.GetChildMeshCount() - 1
        
        Set childMesh = frame.GetChildMesh(i)
        
        If Not childMesh Is Nothing Then
            g_d3dx.Intersect childMesh.mesh, vNear, vDir, item.hit, item.triFaceid, item.a, item.b, item.dist, 0
        End If
        
        If item.hit <> 0 Then
            InternalAddItem frame, childMesh, item
            item.hit = 0
        End If
        bHit = True
    Next
    
    'check pick for all child frame
    Dim childFrame As CD3DFrame
    For i = 0 To frame.GetChildFrameCount() - 1
        Set childFrame = frame.GetChildFrame(i)
        bHit = bHit Or _
                ViewportPickEx(childFrame, viewport, proj, view, NewWorldMatrix, X, Y)
    Next

    ViewportPickEx = bHit

End Function



'-----------------------------------------------------------------------------
' Name: RayPick
' Desc: given a ray cast it into a scene graph
' Params:
'    frame      parent of frame heirarchy to pick from
'    vOrig      origen of the ray to cast
'    vDir       direction of the ray
'
' Note: the following functions will fail on PURE HAL devices
'       use RayPickEx if working with pureHal devices
'       Call getCount to see if the ray hit any objects
'
'-----------------------------------------------------------------------------
Public Function RayPick(frame As CD3DFrame, vOrig As D3DVECTOR, vDir As D3DVECTOR)
    Destroy
        
    Dim world As D3DMATRIX
    
    
    g_dev.GetTransform D3DTS_WORLD, world
    
    RayPick = RayPickEx(frame, world, vOrig, vDir)
    
End Function

'-----------------------------------------------------------------------------
' Name: RayPickEx
' Desc: Aux function for RayPickEx
'-----------------------------------------------------------------------------
Public Function RayPickEx(frame As CD3DFrame, worldmatrix As D3DMATRIX, vOrig As D3DVECTOR, vDir As D3DVECTOR) As Boolean

    Dim NewWorldMatrix As D3DMATRIX     'world matrix for this stack frame
    Dim InvWorldMatrix As D3DMATRIX     'world matrix for this stack frame
    Dim currentMatrix As D3DMATRIX
    Dim i As Long, det As Single, bHit As Boolean
    Dim vNewDir As D3DVECTOR, vNewOrig As D3DVECTOR
        
    If frame Is Nothing Then Exit Function
        
    currentMatrix = frame.GetMatrix
        
    'Setup our basis matrix for this frame
    D3DXMatrixMultiply NewWorldMatrix, currentMatrix, worldmatrix
    D3DXMatrixInverse InvWorldMatrix, det, NewWorldMatrix
    
    
    ' we want to compute vdir and vOrig in model space
    ' note we use TransformNormal so we dont translate vDir
    ' just rotate it into a new dir
    Call D3DXVec3TransformNormal(vNewDir, vDir, InvWorldMatrix)
    Call D3DXVec3TransformCoord(vNewOrig, vOrig, InvWorldMatrix)
            
                
    Dim item As D3D_PICK_RECORD
    
    
    'Check all child meshes
    'Even if we got a hit we continue as the next mesh may be closer
    Dim childMesh As CD3DMesh
    For i = 0 To frame.GetChildMeshCount() - 1
        
        Set childMesh = frame.GetChildMesh(i)
        
        If Not childMesh Is Nothing Then
            Call D3DXVec3Scale(vDir, vDir, 1000) 'Workaround for d3dx Intersect bug
            g_d3dx.Intersect childMesh.mesh, vNewOrig, vDir, item.hit, item.triFaceid, item.a, item.b, item.dist, 0
        End If
        
        If item.hit <> 0 Then
            InternalAddItem frame, childMesh, item
            item.hit = 0
        End If
        bHit = True
    Next
    
    'check pick for all child frame
    Dim childFrame As CD3DFrame
    For i = 0 To frame.GetChildFrameCount() - 1
        Set childFrame = frame.GetChildFrame(i)
        bHit = bHit Or _
            RayPickEx(childFrame, NewWorldMatrix, vOrig, vDir)
    Next

    RayPickEx = bHit

End Function




'-----------------------------------------------------------------------------
' InternalAddItem
'-----------------------------------------------------------------------------
Private Sub InternalAddItem(parentFrame As CD3DFrame, mesh As CD3DMesh, item As D3D_PICK_RECORD)
    Dim maxsize As Long
    If m_maxsize = 0 Then
        ReDim m_item(kGrowSize)
        ReDim m_mesh(kGrowSize)
        ReDim m_frame(kGrowSize)
        m_maxsize = kGrowSize
    ElseIf m_count >= m_maxsize Then
        ReDim Preserve m_item(m_maxsize + kGrowSize)
        ReDim Preserve m_frame(m_maxsize + kGrowSize)
        ReDim Preserve m_mesh(m_maxsize + kGrowSize)
        m_maxsize = m_maxsize + kGrowSize
    End If
    
    Set m_mesh(m_count) = mesh
    Set m_frame(m_count) = parentFrame
    m_item(m_count) = item
                             
    m_count = m_count + 1
End Sub



