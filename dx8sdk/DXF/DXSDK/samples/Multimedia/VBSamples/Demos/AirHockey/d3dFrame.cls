VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "CD3DFrame"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       D3DFrame.cls
'  Content:    D3D Visual Basic Framework Frame object
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''


Option Explicit

Public ObjectName As String             'Name of object
Public Enabled As Boolean               'True to render

Dim m_Matrix As D3DMATRIX               'local coordinate system matrix
Dim m_MatConcat As D3DMATRIX            'world matrix coordinate system
Dim m_MatConcatInv As D3DMATRIX         'inv world matrix

Dim m_NumFrames As Long                 'Number of child frame
Dim m_NumMesh As Long                   'Number of child mesh
Dim m_MaxFrames As Long                 'Max number of frames before reallocation
Dim m_MaxMesh As Long                   'Max number of meshes befor reallocation
Dim m_DeletedFrames As Long             'Number of frame deleted
Dim m_DeletedMesh As Long               'Number of mesh deleted

Dim m_D3DMesh() As CD3DMesh             'List of child Meshes
Dim m_D3DFrame() As CD3DFrame           'List of Child Frame

Dim m_PreRenderEvent As DirectXEvent8   'Callback for prerender event
Dim m_PreRenderId As Long               'Handle to send to prerender event
Dim m_PostRenderEvent As DirectXEvent8  'Callback for postrender event
Dim m_PostRenderId As Long              'Handle to send to postrender event

Dim m_SphereRadius As Single            'Bounding radius
Dim m_SphereWorldPos As D3DVECTOR       'Bounding sphere center in world coord
Dim m_SphereModelPos As D3DVECTOR       'Bounding sphere center in local coord

Dim m_pos As D3DVECTOR                  'Position as set by SetPosition
Dim m_quat As D3DQUATERNION             'Orientations as set by SetOrientation
Dim m_rotcenter As D3DVECTOR            '
Dim m_scale As Single                   'Uniform scale as set by SetScale

Const kArrayStep = 5

Public Enum CombineTypeEnum
    COMBINE_BEFORE = 0
    COMBINE_AFTER = 1
    COMBINE_replace = 2
End Enum

Public Enum TransformOp
    TRANSFORM_COMPUTE = 0
    TRANSFORM_FROMUPDATEFRAME = 1
    TRANSFORM_DISABLE = 2
End Enum


'-----------------------------------------------------------------------------
' Name: SetPreRenderCallback
' Desc: callback to be called before Frame renders children
'-----------------------------------------------------------------------------
Public Function SetPreRenderCallback(cb As DirectXEvent8, id As Long)
    Set m_PreRenderEvent = cb
    m_PreRenderId = id
End Function

'-----------------------------------------------------------------------------
' Name: SetPostRenderCallback
' Desc: callback to be called after Frame renders children
'-----------------------------------------------------------------------------
Public Function SetPostRenderCallback(cb As DirectXEvent8, id As Long)
    Set m_PostRenderEvent = cb
    m_PostRenderId = id
End Function



'-----------------------------------------------------------------------------
' Name: AddChild
' Desc: adds another frame as a child
'-----------------------------------------------------------------------------
Public Sub AddChild(child As CD3DFrame)
    If m_MaxFrames = 0 Then
        m_MaxFrames = kArrayStep
        ReDim m_D3DFrame(m_MaxFrames)
    ElseIf m_MaxFrames <= m_NumFrames Then
        m_MaxFrames = m_MaxFrames + kArrayStep
        ReDim Preserve m_D3DFrame(m_MaxFrames)
    End If
    Set m_D3DFrame(m_NumFrames) = child
    m_NumFrames = m_NumFrames + 1
End Sub

'-----------------------------------------------------------------------------
' Name: SetPosition
' Desc: sets position of the object in the parents coordinate system
' Note: will replace whatever matrix is set
'-----------------------------------------------------------------------------
Sub SetPosition(vec As D3DVECTOR)
    m_pos = vec
    UpdateMatrix
End Sub

'-----------------------------------------------------------------------------
' Name: GetPosition
' Desc: returns the position of the object in the parents coordinate system
'-----------------------------------------------------------------------------
Function GetPosition() As D3DVECTOR
    GetPosition.X = m_Matrix.m41
    GetPosition.Y = m_Matrix.m42
    GetPosition.z = m_Matrix.m43

End Function

'-----------------------------------------------------------------------------
' Name: SetOrientation
' Desc: sets the orientation of the object
' Note: use D3DXQuaternionRotationAxis to create the quaternion
' Note: will replace whatever matrix is set
'-----------------------------------------------------------------------------
Sub SetOrientation(quat As D3DQUATERNION)
    m_quat = quat
    UpdateMatrix
End Sub

'-----------------------------------------------------------------------------
' Name: GetOrientation
' Desc: returns what ever was passed to SetOrientation
' Note: does not inspect the matrix to obtain orientation
'-----------------------------------------------------------------------------
Function GetOrientation() As D3DQUATERNION
    GetOrientation = m_quat
End Function


'-----------------------------------------------------------------------------
' Name: SetScale
' Desc: Sets the uniform scale of all children
' Note: will replace whatever matrix is set
'-----------------------------------------------------------------------------
Sub SetScale(val As Single)
    m_scale = val
    UpdateMatrix
End Sub

'-----------------------------------------------------------------------------
' Name: GetScale
' Desc: returns what ever was passed to SetScale
' Note: does not inspect the matrix to obtain scale
'-----------------------------------------------------------------------------
Function GetScale() As Single
    GetScale = m_scale
End Function



'-----------------------------------------------------------------------------
' Name: SetOrientationCenter
' Desc: Sets the pivot point for the Frame
' Note: will replace whatever matrix is set
'-----------------------------------------------------------------------------
Sub SetOrientationCenter(vec As D3DVECTOR)
    m_rotcenter = vec
    UpdateMatrix
End Sub

'-----------------------------------------------------------------------------
' Name: GetOrientationCenter
' Desc: returns whatever pivot point was set
' Note: does not inspect the matrix to obtain pivot
'-----------------------------------------------------------------------------
Function GetOrientationCenter() As D3DVECTOR
    GetOrientationCenter = m_rotcenter
End Function


'-----------------------------------------------------------------------------
' Name: AddMesh
' Desc: Adds a Child Mesh object to the frame
'-----------------------------------------------------------------------------
Public Sub AddMesh(childMesh As CD3DMesh)
    If m_MaxMesh = 0 Then
        m_MaxMesh = kArrayStep
        ReDim m_D3DMesh(m_MaxMesh)
    ElseIf m_MaxMesh <= m_NumMesh Then
        m_MaxMesh = m_MaxMesh + kArrayStep
        ReDim Preserve m_D3DMesh(m_MaxMesh)
    End If
    Set m_D3DMesh(m_NumMesh) = childMesh
    m_NumMesh = m_NumMesh + 1
    
    
End Sub



'-----------------------------------------------------------------------------
' Name: AddD3DXMesh
' Desc: Adds a D3DX mesh object to the frame
' Note: The d3dxmesh object is first encapuslated in a CD3DMesh object
'-----------------------------------------------------------------------------
Public Function AddD3DXMesh(mesh As D3DXMesh) As CD3DMesh
    
    Dim childMesh As CD3DMesh
    Set childMesh = New CD3DMesh
    childMesh.InitFromD3DXMesh mesh
    
    If m_MaxMesh = 0 Then
        m_MaxMesh = kArrayStep
        ReDim m_D3DMesh(m_MaxMesh)
    ElseIf m_MaxMesh <= m_NumMesh Then
        m_MaxMesh = m_MaxMesh + kArrayStep
        ReDim Preserve m_D3DMesh(m_MaxMesh)
    End If
    Set m_D3DMesh(m_NumMesh) = childMesh
    m_NumMesh = m_NumMesh + 1
    
    Set AddD3DXMesh = childMesh
End Function

'-----------------------------------------------------------------------------
' Name: GetMatrix
' Desc: Returns the matrix for the local coordinate system
'-----------------------------------------------------------------------------
Public Function GetMatrix() As D3DMATRIX
    GetMatrix = m_Matrix
End Function

'-----------------------------------------------------------------------------
' Name: SetMatrix
' Desc: Sets the matrix for the local coordinate system
' Note: This overrides any previous calls to functions such as
'       SetPosition, SetScale, SetOrientation
'-----------------------------------------------------------------------------
Public Function SetMatrix(m As D3DMATRIX)
    m_Matrix = m
End Function

'-----------------------------------------------------------------------------
' Name: GetChildMeshCount
' Desc: returns the number of child meshes
'-----------------------------------------------------------------------------
Public Function GetChildMeshCount()
    GetChildMeshCount = m_NumMesh
End Function


'-----------------------------------------------------------------------------
' Name: GetChildMesh
' Desc: returns a given child mesh
'-----------------------------------------------------------------------------
Public Function GetChildMesh(i As Long) As CD3DMesh
    Set GetChildMesh = m_D3DMesh(i)
End Function

'-----------------------------------------------------------------------------
' Name: GetChildFrame
' Desc: returns a given child frame
'-----------------------------------------------------------------------------
Public Function GetChildFrame(i As Long) As CD3DFrame
    Set GetChildFrame = m_D3DFrame(i)
End Function


'-----------------------------------------------------------------------------
' Name: GetChildFrameCount
' Desc: returns number of Child Frames
'-----------------------------------------------------------------------------
Public Function GetChildFrameCount()
    GetChildFrameCount = m_NumFrames
End Function



'-----------------------------------------------------------------------------
' Name: FindChildObject
' Desc: Given an object name (flags are ignored) return the first child with
'       a matching name. use the ClassName property to see if its a mesh
'       or a frame object
'-----------------------------------------------------------------------------

Public Function FindChildObject(Name As String, flags As Long) As Object
    Dim i As Long
    Dim aMesh As CD3DMesh
    Dim aFrame As CD3DFrame
    
    'check self
    If ObjectName = Name Then
        Set FindChildObject = Me
        Exit Function
    End If
    
    'check child mesh
    For i = 0 To m_NumMesh - 1
        Set aMesh = m_D3DMesh(i)
        If Not aMesh Is Nothing Then
            If aMesh.ObjectName = Name Then
                Set FindChildObject = aMesh
                Exit Function
            End If
        End If
    Next
    
    Dim o As Object
    'check child frames
    For i = 0 To m_NumFrames - 1
        Set aFrame = m_D3DFrame(i)
        If Not aFrame Is Nothing Then
            Set o = aFrame.FindChildObject(Name, flags)
            If Not o Is Nothing Then
                Set FindChildObject = o
                Exit Function
            End If
        End If
    Next
    Set FindChildObject = Nothing
End Function

'-----------------------------------------------------------------------------
' Name: FindChildParent
' Desc: given an object name find its parent frame
'-----------------------------------------------------------------------------

Public Function FindChildParent(Name As String, flags As Long) As CD3DFrame
    Dim i As Long
    Dim aMesh As CD3DMesh
    Dim aFrame As CD3DFrame
    
    'check child mesh
    For i = 0 To m_NumMesh - 1
        Set aMesh = m_D3DMesh(i)
        If Not aMesh Is Nothing Then
            If aMesh.ObjectName = Name Then
                Set FindChildParent = Me
                Exit Function
            End If
        End If
    Next
    
    Dim o As Object
    'check child frames
    For i = 0 To m_NumFrames - 1
        Set aFrame = m_D3DFrame(i)
        If Not aFrame Is Nothing Then
            If aFrame.ObjectName = Name Then
                FindChildParent = Me
            End If
            
            Set o = aFrame.FindChildParent(Name, flags)
            If Not o Is Nothing Then
                Set FindChildParent = o
                Exit Function
            End If
        End If
    Next
    
    Set FindChildParent = Nothing
    
End Function


'-----------------------------------------------------------------------------
' Name: Destroy
' Desc: Release all references
'-----------------------------------------------------------------------------
Public Sub Destroy()
    ReDim m_D3DMesh(0)
    ReDim m_D3DFrame(0)
    m_NumFrames = 0
    m_NumMesh = 0
End Sub



'-----------------------------------------------------------------------------
' Name: InitFromXOF
' Desc: called from InitFromFile
'-----------------------------------------------------------------------------
Friend Sub InitFromXOF(dev As Direct3DDevice8, FileData As DirectXFileData, parent As CD3DFrame)
    
    Dim ChildData As DirectXFileData
    Dim ChildObj As DirectXFileObject
    Dim TypeGuid As String
    Dim newmesh As CD3DMesh
    Dim newFrame As CD3DFrame

    If Not parent Is Nothing Then parent.AddChild Me
    
    ObjectName = FileData.GetName()
    
    Do
        'list of object - should return nothing at end of list
        Set ChildData = FileData.GetNextObject()
        If ChildData Is Nothing Then Exit Do
    
        Select Case ChildData.GetType()
               
            Case "TID_D3DRMFrame"
                Set newFrame = New CD3DFrame
                newFrame.InitFromXOF dev, ChildData, Me
                Set newFrame = Nothing
  
            Case "TID_D3DRMMesh"
                Set newmesh = New CD3DMesh
                newmesh.InitFromXOF dev, ChildData
                AddMesh newmesh
                Set newmesh = Nothing
  
            Case "TID_D3DRMFrameTransformMatrix"
                ChildData.GetDataFromOffset "", 0, Len(m_Matrix), m_Matrix
            
        End Select
            
    Loop
    
End Sub


'-----------------------------------------------------------------------------
' Name: InitFromFile
' Desc: Called from D3DUtil_LoadFromFile
'-----------------------------------------------------------------------------
Public Function InitFromFile(dev As Direct3DDevice8, strFile As String, parent As CD3DFrame, animParent As CD3DAnimation) As Boolean


    Dim xfile As DirectXFile
    Dim enumX As DirectXFileEnum
    Dim ChildData As DirectXFileData
    Dim rootid As Long
    Dim childid As Long
    Dim strPath As String
    Dim newmesh As CD3DMesh
    Dim newFrame As CD3DFrame
    
    On Local Error GoTo errOut
                
    
    'create the X file object
    Set xfile = g_dx.DirectXFileCreate()
    
    'make sure it understand MeshBuilders etc
    xfile.RegisterDefaultTemplates
    
    On Local Error Resume Next
    
    'Open the file - will propegate file not found if fails
    Set enumX = xfile.CreateEnumObject(strFile)
    If Err.Number <> 0 Then
    
        On Local Error GoTo errOut
        strPath = g_mediaPath + strFile
        Set enumX = xfile.CreateEnumObject(strPath)
    
    End If
    
            
    'Inspect each top level object in the file
    Do
        Set ChildData = enumX.GetNextDataObject()
        If ChildData Is Nothing Then Exit Do
                
        If ChildData.GetType = "TID_D3DRMFrame" Then
            Set newFrame = New CD3DFrame
            newFrame.InitFromXOF dev, ChildData, Me
            Set newFrame = Nothing
            
        ElseIf ChildData.GetType = "TID_D3DRMMesh" Then
            Set newmesh = New CD3DMesh
            newmesh.InitFromXOF dev, ChildData
            AddMesh newmesh
            Set newmesh = Nothing
            
        ElseIf ChildData.GetType = "TID_D3DRMAnimationSet" Then
            'TODO Parse Animation Set Data
            If Not animParent Is Nothing Then
                animParent.ParseAnimSet ChildData, Me
            End If
        ElseIf ChildData.GetType = "TID_D3DRMMaterialSet" Then
            'TODO Parse TopLevel Material Data
            
        End If
    Loop

    If Not parent Is Nothing Then
        parent.AddChild Me
    End If
                    
    InitFromFile = True
    Exit Function
    
errOut:
    InitFromFile = False
    
End Function





'-----------------------------------------------------------------------------
' Name: SetFVF
' Desc: Changes the Flexible Vertex Format of all child objects
'-----------------------------------------------------------------------------
Public Sub SetFVF(dev As Direct3DDevice8, fvf As Long)
    Dim aMesh As CD3DMesh
    Dim aFrame As CD3DFrame
    Dim i As Long
    
    For i = 0 To m_NumMesh - 1
        Set aMesh = m_D3DMesh(i)
        If Not aMesh Is Nothing Then
            aMesh.SetFVF g_dev, fvf
        End If
    Next
    
    For i = 0 To m_NumFrames - 1
        Set aFrame = m_D3DFrame(i)
        If Not aFrame Is Nothing Then
            aFrame.SetFVF g_dev, fvf
        End If
    Next

End Sub


'-----------------------------------------------------------------------------
' Name: FlipNormals
' Desc: Flips the normals on all child objects (provided they are of D3DFVF_VERTEX)
'-----------------------------------------------------------------------------
Public Sub FlipNormals()
    Dim aMesh As CD3DMesh
    Dim aFrame As CD3DFrame
    Dim i As Long
    
    For i = 0 To m_NumMesh - 1
        Set aMesh = m_D3DMesh(i)
        If Not aMesh Is Nothing Then
            aMesh.FlipNormals
        End If
    Next
    
    For i = 0 To m_NumFrames - 1
        Set aFrame = m_D3DFrame(i)
        If Not aFrame Is Nothing Then
            aFrame.FlipNormals
        End If
    Next

End Sub


'-----------------------------------------------------------------------------
' Name: ComputeNormals
' Desc: Computes normals on all child objects (provided they have a normal set set)
'-----------------------------------------------------------------------------
Public Sub ComputeNormals()
    Dim aMesh As CD3DMesh
    Dim aFrame As CD3DFrame
    Dim i As Long
    
    For i = 0 To m_NumMesh - 1
        Set aMesh = m_D3DMesh(i)
        If Not aMesh Is Nothing Then
            aMesh.ComputeNormals
        End If
    Next
    
    For i = 0 To m_NumFrames - 1
        Set aFrame = m_D3DFrame(i)
        If Not aFrame Is Nothing Then
            aFrame.ComputeNormals
        End If
    Next

End Sub


'-----------------------------------------------------------------------------
' Name: Optimize
' Desc:
'-----------------------------------------------------------------------------
Public Sub Optimize()
    Dim aMesh As CD3DMesh
    Dim aFrame As CD3DFrame
    Dim i As Long
    
    For i = 0 To m_NumMesh - 1
        Set aMesh = m_D3DMesh(i)
        If Not aMesh Is Nothing Then
            aMesh.Optimize
        End If
    Next
    
    For i = 0 To m_NumFrames - 1
        Set aFrame = m_D3DFrame(i)
        If Not aFrame Is Nothing Then
            aFrame.Optimize
        End If
    Next

End Sub



'-----------------------------------------------------------------------------
' Name: ComputeBoundingVolumes
' Desc: ComputesBoundingVolumes for all child objects
'-----------------------------------------------------------------------------
Public Sub ComputeBoundingVolumes()
        
    Dim aMesh As CD3DMesh
    Dim aFrame As CD3DFrame
    Dim i As Long
    
    For i = 0 To m_NumMesh - 1
        Set aMesh = m_D3DMesh(i)
        If Not aMesh Is Nothing Then
            aMesh.ComputeBoundingVolumes
        End If
    Next
    
    For i = 0 To m_NumFrames - 1
        Set aFrame = m_D3DFrame(i)
        If Not aFrame Is Nothing Then
            aFrame.ComputeBoundingVolumes
        End If
    Next

End Sub


'-----------------------------------------------------------------------------
' Name: UpdateFrames
' Desc: Called to precompute the WorldMatrices for each child object
' Note: Must be used prior to calling Mesh.RenderSkin
'-----------------------------------------------------------------------------
Public Sub UpdateFrames()
    ComputeChildMatricesEx g_identityMatrix
End Sub

'-----------------------------------------------------------------------------
' Name: GetUpdatedMatrix
' Desc: Returns the precomputed matrix
'-----------------------------------------------------------------------------
Public Function GetUpdatedMatrix() As D3DMATRIX
    GetUpdatedMatrix = m_MatConcat
End Function


'-----------------------------------------------------------------------------
' Name: ComputeChildMatricesEx
' Desc: Aux function for UpdateFrames
'-----------------------------------------------------------------------------
Friend Sub ComputeChildMatricesEx(matSavedWorld As D3DMATRIX)
    
    
    Dim aFrame As CD3DFrame
    Dim i As Long
    Dim det As Single
    
    Call D3DXMatrixMultiply(m_MatConcat, m_Matrix, matSavedWorld)
    Call D3DXMatrixInverse(m_MatConcatInv, det, m_MatConcat)
           
    For i = 0 To m_NumFrames - 1
        Set aFrame = m_D3DFrame(i)
        If Not aFrame Is Nothing Then
            aFrame.ComputeChildMatricesEx m_MatConcat
        End If
    Next

End Sub





'-----------------------------------------------------------------------------
' Name: RenderEx
' Desc: Render Child Objects
' Params:
'       dev                     Device to render to
'       bDrawOpaqueSubsets      Render all objects with an material alpha of 1
'       bDrawAlphaSubsets       Render all objects with transparent alpha
'       transform
'                               TRANSFORM_COMPUT indicates to calculate world matrix
'                               TRANSFORM_FROMUPDATEFRAME inidcates to use precomputed matrix
'                               TRANSFORM_DISABLE does not set any world matrix
'                               and uses what ever has been currently set
'-----------------------------------------------------------------------------
Friend Sub RenderEx(dev As Direct3DDevice8, bDrawOpaqueSubsets As Boolean, bDrawAlphaSubsets As Boolean, transform As TransformOp)
    
    Dim matSavedWorld As D3DMATRIX
    Dim matWorld As D3DMATRIX
    Dim aFrame As CD3DFrame
    Dim aMesh As CD3DMesh
    Dim vCenter As D3DVECTOR
    
    If Not Enabled Then Exit Sub
    
    Dim i As Long
    
    
    If transform = TRANSFORM_COMPUTE Then
        Call g_dev.GetTransform(D3DTS_WORLD, matSavedWorld)
        Call D3DXMatrixMultiply(matWorld, m_Matrix, matSavedWorld)
        Call g_dev.SetTransform(D3DTS_WORLD, matWorld)
    ElseIf transform = TRANSFORM_FROMUPDATEFRAME Then
        Call g_dev.SetTransform(D3DTS_WORLD, m_MatConcat)
    End If
    
    
    If Not m_PreRenderEvent Is Nothing Then
        m_PreRenderEvent.DXCallback m_PreRenderId
    End If
    
    For i = 0 To m_NumMesh - 1
        Set aMesh = m_D3DMesh(i)
        If Not aMesh Is Nothing Then
            
            If g_bClipMesh And aMesh.bHasSphere Then
                D3DXVec3TransformCoord vCenter, aMesh.SphereCenter, matWorld
                If D3DUtil_IsSphereVisible(vCenter, aMesh.SphereRadius) <> 0 Then
                    aMesh.RenderEx g_dev, bDrawOpaqueSubsets, bDrawAlphaSubsets
                Else
                    'Debug.Print "cull " + aMesh.ObjectName
                End If
            Else
                aMesh.RenderEx g_dev, bDrawOpaqueSubsets, bDrawAlphaSubsets
            End If
        End If
    Next
    
    For i = 0 To m_NumFrames - 1
        Set aFrame = m_D3DFrame(i)
        If Not aFrame Is Nothing Then
            aFrame.RenderEx g_dev, bDrawOpaqueSubsets, bDrawAlphaSubsets, transform
        End If
    Next
        
    If transform = TRANSFORM_COMPUTE Then
        Call g_dev.SetTransform(D3DTS_WORLD, matSavedWorld)
    End If
    
    If Not m_PostRenderEvent Is Nothing Then
        m_PostRenderEvent.DXCallback m_PostRenderId
    End If


End Sub




'-----------------------------------------------------------------------------
' Name: RenderSkins
' Desc: Searches the children for any mesh with skinned objects
'       and renders them
' Note: BoneMatrice must be precalulated before rendering
'       and bones must also be attached to appropriate mesh
'-----------------------------------------------------------------------------
Friend Sub RenderSkins()
    Dim i As Long
    Dim aMesh As CD3DMesh
    Dim aFrame As CD3DFrame
    
    If Not m_PreRenderEvent Is Nothing Then
        m_PreRenderEvent.DXCallback m_PreRenderId
    End If
        
    For i = 0 To m_NumMesh - 1
        Set aMesh = m_D3DMesh(i)
        If Not aMesh Is Nothing Then
            aMesh.RenderSkin
        End If
    Next
    
    For i = 0 To m_NumFrames - 1
        Set aFrame = m_D3DFrame(i)
        If Not aFrame Is Nothing Then
            aFrame.RenderSkins
        End If
    Next
    
    If Not m_PostRenderEvent Is Nothing Then
        m_PostRenderEvent.DXCallback m_PostRenderId
    End If

End Sub

'-----------------------------------------------------------------------------
' Name: AttatchBonesToMesh
' Desc:
'-----------------------------------------------------------------------------
Friend Sub AttatchBonesToMesh(parent As CD3DFrame)
    
    Dim i As Long
    Dim aMesh As CD3DMesh
    Dim aFrame As CD3DFrame
    
        
    For i = 0 To m_NumMesh - 1
        Set aMesh = m_D3DMesh(i)
        If Not aMesh Is Nothing Then
            aMesh.AttatchBonesToMesh parent
        End If
    Next
    
    For i = 0 To m_NumFrames - 1
        Set aFrame = m_D3DFrame(i)
        If Not aFrame Is Nothing Then
            aFrame.AttatchBonesToMesh parent
        End If
    Next
    
End Sub

'-----------------------------------------------------------------------------
' Name: Render
' Desc: render all child objects
'-----------------------------------------------------------------------------
Public Sub Render(dev As Direct3DDevice8)
        
    dev.SetTransform D3DTS_WORLD, g_identityMatrix
        
    'Render opaque subsets in  the meshes
    RenderEx g_dev, True, False, TRANSFORM_COMPUTE
        
    
    ' Enable alpha blending
    Call dev.SetRenderState(D3DRS_ALPHABLENDENABLE, 1)
    Call dev.SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA)
    Call dev.SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA)

    dev.SetTransform D3DTS_WORLD, g_identityMatrix
    
    RenderEx g_dev, False, True, TRANSFORM_COMPUTE
        
    Call dev.SetRenderState(D3DRS_ALPHABLENDENABLE, 0)
    
End Sub


'-----------------------------------------------------------------------------
' Name: ClassName
'-----------------------------------------------------------------------------
Public Function ClassName() As String
    ClassName = "CD3DFrame"
End Function


'-----------------------------------------------------------------------------
' Name: Class_Initialize()
'-----------------------------------------------------------------------------
Private Sub Class_Initialize()
    D3DXMatrixIdentity m_Matrix
    D3DXMatrixIdentity g_identityMatrix
        
    Call D3DXQuaternionIdentity(m_quat)
    m_scale = 1
    Enabled = True
    
End Sub

'-----------------------------------------------------------------------------
' Name: AddRotation
' Param:
'       combinetype     PreMultiply/PostMultiply/or replace current matrix
'       x,y,z           axis of rotation
'       rot             radians to rotate
'-----------------------------------------------------------------------------
Public Function AddRotation(combinetype As CombineTypeEnum, X As Single, Y As Single, z As Single, rot As Single)
    
    Dim mat As D3DMATRIX
    D3DXMatrixRotationAxis mat, vec3(X, Y, z), rot
    
    Select Case combinetype
        Case COMBINE_BEFORE
            D3DXMatrixMultiply m_Matrix, mat, m_Matrix
        Case COMBINE_AFTER
            D3DXMatrixMultiply m_Matrix, m_Matrix, mat
        Case COMBINE_replace
            m_Matrix = mat
    End Select

End Function


'-----------------------------------------------------------------------------
' Name: AddScale
' Param:
'       combinetype     PreMultiply/PostMultiply/or replace current matrix
'       x,y,z           scale
'-----------------------------------------------------------------------------
Public Function AddScale(combinetype As CombineTypeEnum, X As Single, Y As Single, z As Single)
    
    Dim mat As D3DMATRIX
    D3DXMatrixScaling mat, X, Y, z
    Select Case combinetype
        Case COMBINE_BEFORE
            D3DXMatrixMultiply m_Matrix, mat, m_Matrix
        Case COMBINE_AFTER
            D3DXMatrixMultiply m_Matrix, m_Matrix, mat
        Case COMBINE_replace
            m_Matrix = mat
    End Select

End Function

'-----------------------------------------------------------------------------
' Name: AddTranslation
' Param:
'       combinetype     PreMultiply/PostMultiply/or replace current matrix
'       x,y,z           translation
'-----------------------------------------------------------------------------
Public Function AddTranslation(combinetype As CombineTypeEnum, X As Single, Y As Single, z As Single)
    
    Dim mat As D3DMATRIX
    D3DXMatrixTranslation mat, X, Y, z
    Select Case combinetype
        Case COMBINE_BEFORE
            D3DXMatrixMultiply m_Matrix, mat, m_Matrix
        Case COMBINE_AFTER
            D3DXMatrixMultiply m_Matrix, m_Matrix, mat
        Case COMBINE_replace
            m_Matrix = mat
    End Select

End Function


'-----------------------------------------------------------------------------
' Name: TransformCoord
' Desc: Transform a vector by the pre-computed matrix
'-----------------------------------------------------------------------------
Public Function TransformCoord(vIn As D3DVECTOR) As D3DVECTOR
    Dim vOut As D3DVECTOR
    D3DXVec3TransformCoord vOut, vIn, m_MatConcat
    TransformCoord = vOut
End Function

'-----------------------------------------------------------------------------
' Name: TransformNormal
' Desc: Transform a normal by the pre-computed matrix
'-----------------------------------------------------------------------------
Public Function TransformNormal(vIn As D3DVECTOR) As D3DVECTOR
    Dim vOut As D3DVECTOR
    D3DXVec3TransformNormal vOut, vIn, m_MatConcat
    TransformNormal = vOut
End Function


'-----------------------------------------------------------------------------
' Name: InverseTransformCoord
' Desc: Transform a vector by the inverse of the pre-computed matrix
'-----------------------------------------------------------------------------
Public Function InverseTransformCoord(vIn As D3DVECTOR) As D3DVECTOR
    Dim vOut As D3DVECTOR
    D3DXVec3TransformCoord vOut, vIn, m_MatConcatInv
    InverseTransformCoord = vOut
End Function

'-----------------------------------------------------------------------------
' Name: InverseTransformNormal
' Desc: Transform a normal by the inverse of the pre-computed matrix
'-----------------------------------------------------------------------------
Public Function InverseTransformNormal(vIn As D3DVECTOR) As D3DVECTOR
    Dim vOut As D3DVECTOR
    D3DXVec3TransformNormal vOut, vIn, m_MatConcatInv
    InverseTransformNormal = vOut
End Function


'-----------------------------------------------------------------------------
' Name: InvalidateDeviceObjects
' Desc:
'-----------------------------------------------------------------------------
Public Sub InvalidateDeviceObjects()
    'all objects are managed so nothing to do
End Sub


'-----------------------------------------------------------------------------
' Name: RestoreDeviceObjects
' Desc:
'-----------------------------------------------------------------------------
Public Sub RestoreDeviceObjects(dev As Direct3DDevice8)
    'all objects are managed so nothing to do
End Sub


'-----------------------------------------------------------------------------
' Name: UpdateMatrix
' Desc: Aux function for SetPosition, SetOrientation, SetScale
'-----------------------------------------------------------------------------
Private Sub UpdateMatrix()
    On Local Error Resume Next
    D3DXMatrixAffineTransformation m_Matrix, m_scale, m_rotcenter, m_quat, m_pos
End Sub


