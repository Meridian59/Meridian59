VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "CD3DMesh"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       D3DMesh.cls
'  Content:    D3D VB Framework Mesh
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''


Option Explicit

Private Type BoneComboIdList
    List(4) As Long
End Type


' Base Objects
Public mesh As D3DXMesh                         'if not skinned, regular mesh object
Public skinmesh As D3DXSkinMesh                 'if skinned - skinned mesh object
Public bUseMaterials As Boolean                 'Use materials in object
Public bUseMaterialOverride As Boolean          'Use only override material
Public ObjectName As String                     'Name of object
Public bSkinned As Boolean                      'Inidicates if the object is a skin

' Culling objects
Dim m_SphereCenter As D3DVECTOR                 'center of bounding sphere
Public SphereRadius As Single                   'radius of bounding sphere
Public bHasSphere As Boolean                    'Inidcatges if bounding sphere is calculated

' Material and Adjacency information
Dim m_MaterialOverride As D3DMATERIAL8          'Override material to use if bUseMaterialOverride set
Dim m_NumMaterials As Long                      'Number of materials in object
Dim m_Materials() As D3DMATERIAL8               'Material List
Dim m_Textures() As Direct3DTexture8            'Texture List
Dim m_TextureNames() As String                  'Texture List Names
Dim m_AdjBuffer As D3DXBuffer                   'Adjacency buffer for the mesh


' Bone and skinning informatin
Dim m_BoneNames As D3DXBuffer                   'Names of Frame objects that are bones
Dim m_BoneMatrices As D3DXBuffer                'Matrix object per bone
Dim m_BoneComboTable As D3DXBuffer              'Groupings of bone material and triangles
Dim m_BoneFrames() As CD3DFrame                 'Frame objects that are bones
Dim m_maxFaceInfl As Long                       'Number of matrices that will influence a vertex
Dim m_worldMatHandles(4) As Long                'handle to multiple world matrices
Dim m_BoneOffsetMat() As D3DMATRIX              'Bone offset matrices from D3DXBuffers
Dim m_NumBoneCombos As Long                     'Size of bonecombo table
Dim m_BoneCombos() As D3DXBONECOMBINATION       'Combo table as returned from D3DX
Dim m_BoneComboIds() As BoneComboIdList         'BoneId portion of combotable
Dim m_BoneCount As Long                         'Number of bones
Dim m_bonesAttached As Boolean                  'Indicates if bones have been attached to the mesh

'-----------------------------------------------------------------------------
' Init
'-----------------------------------------------------------------------------
Sub Init()
    bUseMaterials = True
End Sub

'-----------------------------------------------------------------------------
' SphereCenter()
' returns Sphere Center since D3DVECTOR can not be public variable return value
'-----------------------------------------------------------------------------
Property Get SphereCenter() As D3DVECTOR
    SphereCenter = m_SphereCenter
End Property

'-----------------------------------------------------------------------------
' Name: InitFromFile
' Desc: Tries first to load mesh in current directory or using explicit path
'       If that fails loads mesh from SDK media path
'-----------------------------------------------------------------------------

Public Function InitFromFile(dev As Direct3DDevice8, Name As String) As Boolean
    Dim mtrlBuffer As D3DXBuffer
    Dim strPath As String
    Destroy
    
'    On Local Error Resume Next
    Set m_AdjBuffer = Nothing
    bSkinned = False
    
    Set mesh = g_d3dx.LoadMeshFromX(Name, D3DXMESH_MANAGED, dev, m_AdjBuffer, mtrlBuffer, m_NumMaterials)
    If Err.Number <> 0 Then
        Err.Clear
        On Local Error GoTo errOut
        strPath = g_mediaPath + Name
        Set mesh = g_d3dx.LoadMeshFromX(strPath, D3DXMESH_MANAGED, dev, m_AdjBuffer, mtrlBuffer, m_NumMaterials)
    
    End If
    
    Call InitMaterials(g_dev, mtrlBuffer)
    
    InitFromFile = True
    Exit Function
    
errOut:
    InitFromFile = False
End Function



'-----------------------------------------------------------------------------
' Name: InitFromXOF
' Desc: Load mesh from data provided by XOF api
'       Called from D3DUtil_LoadFromFile
'-----------------------------------------------------------------------------

Public Function InitFromXOF(dev As Direct3DDevice8, meshdata As DirectXFileData) As Boolean
    Dim mtrlBuffer As D3DXBuffer
    Dim bonename As String
    Dim i As Long
    Dim q As Long
        
        
    Destroy
    
    
    Set m_AdjBuffer = Nothing
    Set m_BoneMatrices = Nothing
    Set m_BoneNames = Nothing
    Set mesh = Nothing
    Set skinmesh = Nothing
    Set m_BoneMatrices = Nothing
    Set m_BoneComboTable = Nothing
    
    ObjectName = meshdata.GetName()
    
'    On Local Error GoTo errOut
    bSkinned = False
    
    'Users can set this variable to TRUE try the skinned load path
    If g_bLoadSkins = True Then
        Set skinmesh = g_d3dx.LoadSkinMeshFromXof(meshdata, D3DXMESH_MANAGED, dev, m_AdjBuffer, mtrlBuffer, m_NumMaterials, m_BoneNames, m_BoneMatrices)
        
        
        Dim pAdj As Long, AdjOut As D3DXBuffer
        pAdj = m_AdjBuffer.GetBufferPointer
        
        m_BoneCount = skinmesh.GetNumBones()
        
        If m_BoneCount = 0 Then
            
            ''''''''''''''''''''''''''''''''''''''''''''''''''''
            ' a skinned mesh with no bones is just a regular mesh
            ''''''''''''''''''''''''''''''''''''''''''''''''''''
            
            bSkinned = False
            Set mesh = skinmesh.GetOriginalMesh()
            'Set skinmesh = Nothing
            
        Else
        
            '''''''''''''''''''''''''''''''''''''''''''''''
            ' code specific to x files with skinning data in them
            '''''''''''''''''''''''''''''''''''''''''''''''
            
            bSkinned = True
            
            Set mesh = skinmesh.ConvertToBlendedMesh(D3DXMESH_SYSTEMMEM, ByVal pAdj, ByVal 0, m_NumBoneCombos, m_BoneComboTable, ByVal 0&, Nothing)
            Set m_AdjBuffer = Nothing
            Set m_AdjBuffer = AdjOut
            Set AdjOut = Nothing
            
            'retrieve number of influence (matrices) that a vertices could have
            'we support up to 4 corresponding to the 4 world matrices that can be set
            m_maxFaceInfl = skinmesh.GetMaxFaceInfluences()
            
            m_worldMatHandles(0) = D3DTS_WORLD
            m_worldMatHandles(1) = D3DTS_WORLD1
            m_worldMatHandles(2) = D3DTS_WORLD2
            m_worldMatHandles(3) = D3DTS_WORLD3
            
            ReDim m_BoneCombos(m_NumBoneCombos)
            ReDim m_BoneComboIds(m_NumBoneCombos)
            
            ' fill in our private table for bone combo data
            ' this inidicates which bones (matrices) need to be blended
            ' for a given subset in the mesh
            For q = 0 To m_NumBoneCombos - 1
                g_d3dx.BufferGetBoneCombo m_BoneComboTable, q, m_BoneCombos(q)
                g_d3dx.BufferGetBoneComboBoneIds m_BoneComboTable, q, m_maxFaceInfl, m_BoneComboIds(q).List(0)
            Next
            Set m_BoneComboTable = Nothing
                    
            ' fill in our private table for bone offset matrices
            ' these are the matrices that give the intitial displacement of mesh subsets
            ' release the d3dx buffer to save memory
            ReDim m_BoneOffsetMat(m_BoneCount)
            g_d3dx.BufferGetData m_BoneMatrices, 0, Len(m_BoneOffsetMat(0)), m_BoneCount, m_BoneOffsetMat(0)
            Set m_BoneMatrices = Nothing
            
            
        End If
    Else
    
         Set mesh = g_d3dx.LoadMeshFromXof(meshdata, D3DXMESH_MANAGED, dev, m_AdjBuffer, mtrlBuffer, m_NumMaterials)
         
    End If
    
    
    Call InitMaterials(g_dev, mtrlBuffer)
    
    InitFromXOF = True
    Exit Function
    
errOut:
    InitFromXOF = False
End Function


'-----------------------------------------------------------------------------
' Name: AttatchBonesToMesh
' Desc: Called to attach bones to a skin.
'       The BoneNames table is used to search out bone frames
'       in the children of the given parent frame
'
'       This must be done for any skinning animation to work
'-----------------------------------------------------------------------------

Friend Sub AttatchBonesToMesh(parent As CD3DFrame)
      ' get links to all the frames (bones)
        Dim i As Long
        Dim bonename As String
        ReDim m_BoneFrames(m_BoneCount)
        For i = 0 To m_BoneCount - 1
            bonename = g_d3dx.BufferGetBoneName(m_BoneNames, i)
            Set m_BoneFrames(i) = parent.FindChildObject(bonename, 0)
            If m_BoneFrames(i) Is Nothing Then
                Debug.Print "unable to find " + bonename
                Stop
            End If
        Next
        m_bonesAttached = True
        Set m_BoneNames = Nothing
End Sub


'-----------------------------------------------------------------------------
' Name: Optimize
' Desc: Re-organize the mesh for better performance
'
'-----------------------------------------------------------------------------

Sub Optimize()
    Dim s As Long
    Dim adjBuf1() As Long
    Dim adjBuf2() As Long
    Dim facemap() As Long
    Dim newmesh As D3DXMesh
    Dim vertexMap As D3DXBuffer
    
    s = m_AdjBuffer.GetBufferSize
    ReDim adjBuf1(s / 4)
    ReDim adjBuf2(s / 4)
    
    s = mesh.GetNumFaces
    ReDim facemap(s)
    
    g_d3dx.BufferGetData m_AdjBuffer, 0, 4, s * 3, adjBuf1(0)
    
    Set newmesh = mesh.Optimize(D3DXMESHOPT_ATTRSORT Or D3DXMESHOPT_VERTEXCACHE, adjBuf1(0), adjBuf2(0), facemap(0), vertexMap)
    
    If Not newmesh Is Nothing Then
        Set mesh = Nothing
        Set mesh = newmesh
    End If

End Sub




'-----------------------------------------------------------------------------
' Name: InitMaterials
' Desc: Helper function for creating mesh materials
'       called after initialization
'-----------------------------------------------------------------------------
Private Sub InitMaterials(d3ddevice As Direct3DDevice8, mtrlBuffer As D3DXBuffer)
    Dim i As Long
    
    If m_NumMaterials <= 0 Then Exit Sub
    
    ReDim m_Materials(m_NumMaterials)
    ReDim m_Textures(m_NumMaterials)
    ReDim m_TextureNames(m_NumMaterials)

    For i = 0 To m_NumMaterials - 1
        'copy material out of material buffer into our own structure
        g_d3dx.BufferGetMaterial mtrlBuffer, i, m_Materials(i)

        If g_bLoadNoAlpha Then m_Materials(i).diffuse.a = 1
        
        m_Materials(i).Ambient = m_Materials(i).diffuse
        
        m_TextureNames(i) = g_d3dx.BufferGetTextureName(mtrlBuffer, i)
        
        If g_bUseTextureLoadCallback Then
            Set m_Textures(i) = g_TextureLoadCallback.TextureLoadCallback(m_TextureNames(i))
        Else
            Set m_Textures(i) = D3DUtil_CreateTextureInPool(g_dev, m_TextureNames(i), D3DFMT_UNKNOWN)
        End If
    Next
    
    
End Sub



'-----------------------------------------------------------------------------
' Name: SetFVF
' Desc: Change the FVF of the current mesh
'----------------------------------------------------------------------------
Public Sub SetFVF(dev As Direct3DDevice8, fvf As Long)
    
    Dim tempMesh As D3DXMesh
    Dim verts() As D3DVERTEX

    If mesh Is Nothing Then Exit Sub
    
    Set tempMesh = mesh.CloneMeshFVF(D3DXMESH_MANAGED, fvf, dev)
    
        
    Set mesh = tempMesh
    
    
End Sub

'-----------------------------------------------------------------------------
' Name: GenerateNormals
' Desc: if the current mesh Flexible Vertex Format (FVF) has normals in it
'       that are not initialized. This function will fill them.
'       if no normals are present in the FVF this function will fire an
'       exception
'----------------------------------------------------------------------------
Public Sub ComputeNormals()
    Dim bm As D3DXBaseMesh
    Set bm = mesh
    g_d3dx.ComputeNormals bm
End Sub


'-----------------------------------------------------------------------------
' Name: FlipNormals
' Desc: Convenience function that flips normals for a D3DVERTEX mesh (default)
'----------------------------------------------------------------------------
Public Sub FlipNormals()
    Dim count As Long
    Dim size As Long
    Dim i As Long
    
    Dim verts() As D3DVERTEX
    
    Dim vb As Direct3DVertexBuffer8
    Set vb = mesh.GetVertexBuffer()
    
    
    
    
    size = g_d3dx.GetFVFVertexSize(mesh.GetFVF())
    count = mesh.GetNumVertices()
    
    
    
    If mesh.GetFVF() = D3DFVF_VERTEX Then
        ReDim verts(count)
        
        D3DVertexBuffer8GetData vb, 0, size * count, 0, verts(0)
        
        For i = 0 To count - 1
            verts(i).nx = -verts(i).nx
            verts(i).ny = -verts(i).ny
            verts(i).nz = -verts(i).nz
        Next
        
        D3DVertexBuffer8SetData vb, 0, size * count, 0, verts(0)
    Else
        Stop
    End If
    

End Sub


'-----------------------------------------------------------------------------
' Name: Translate
' Desc: all vertices are moved by x,y,z
'       note that object will still rotate about 0,0,0
'
'----------------------------------------------------------------------------
Public Sub Translate(x As Single, y As Single, z As Single)
    Dim count As Long
    Dim size As Long
    Dim i As Long
    
    Dim verts() As D3DVERTEX
    
    Dim vb As Direct3DVertexBuffer8
    Set vb = mesh.GetVertexBuffer()
    
    
    
    size = g_d3dx.GetFVFVertexSize(mesh.GetFVF())
    count = mesh.GetNumVertices()

    
    If mesh.GetFVF() = D3DFVF_VERTEX Then
        ReDim verts(count)
        
        D3DVertexBuffer8GetData vb, 0, size * count, 0, verts(0)
        
        For i = 0 To count - 1
            verts(i).x = verts(i).x + x
            verts(i).y = verts(i).y + y
            verts(i).z = verts(i).z + z
        Next
        
        D3DVertexBuffer8SetData vb, 0, size * count, 0, verts(0)
        
    End If
    

End Sub


'-----------------------------------------------------------------------------
' Name: GetLocalBox
' Desc: Returns the extent of the mesh in the local coordinate system
'----------------------------------------------------------------------------
Public Sub GetLocalBox(MinExt As D3DVECTOR, MaxExt As D3DVECTOR)
    g_d3dx.ComputeBoundingBoxFromMesh mesh, MinExt, MaxExt
End Sub

'-----------------------------------------------------------------------------
' Name: Destroy
' Desc: release any reference to frame and texture objects
'-----------------------------------------------------------------------------
Sub Destroy()
    
    'Releases all objects (does leave 1 element in the array)
    ReDim m_Textures(0)
    ReDim m_Materials(0)
    ReDim m_TextureNames(0)
    ReDim m_BoneFrames(0)
    ReDim m_BoneOffsetMat(0)
    ReDim m_BoneCombos(0)
    m_NumMaterials = 0
    bUseMaterials = True
    Set mesh = Nothing
    Set skinmesh = Nothing
End Sub


'-----------------------------------------------------------------------------
' Name: ComputeBoundingVolumes
' Desc: Makes BoundingSphere valid
'-----------------------------------------------------------------------------
Public Sub ComputeBoundingVolumes()
    g_d3dx.ComputeBoundingSphereFromMesh mesh, m_SphereCenter, SphereRadius
    bHasSphere = True
End Sub


'-----------------------------------------------------------------------------
' Name: RenderEx
' Desc: Render Mesh
' Params:
'   dev                         the device to draw to
'   bDrawOpaqueSubsets          draws all triangles that do not have alpha
'   bDrawOpaqueSubsets          draws all triangles that have alpha
'                               (note Blending renderstates are modified)
'
' Note: do not use for skinned meshes
'-----------------------------------------------------------------------------
Sub RenderEx(dev As Direct3DDevice8, bDrawOpaqueSubsets As Boolean, bDrawAlphaSubsets As Boolean)
    If mesh Is Nothing Then Exit Sub
    Dim i As Long
    
    'If bSkinned = True Then Exit Sub

    ' Frist, draw the subsets without alpha
    If (bDrawOpaqueSubsets) Then
        For i = 0 To m_NumMaterials - 1
        
                
        
            If (bUseMaterials) Then
                If m_Materials(i).diffuse.a = 1# Then
                    g_dev.SetMaterial m_Materials(i)
                    
                    If g_bDontDrawTextures Then
                        g_dev.SetTexture 0, Nothing
                    Else
                        g_dev.SetTexture 0, m_Textures(i)
                    End If
                    
                    mesh.DrawSubset i
                End If
            ElseIf (bUseMaterialOverride) Then
                If m_MaterialOverride.diffuse.a = 1# Then
                    If g_bDontDrawTextures Then
                        g_dev.SetTexture 0, Nothing
                    Else
                        g_dev.SetTexture 0, m_Textures(i)
                    End If
                    g_dev.SetMaterial m_MaterialOverride
                    mesh.DrawSubset i
                End If
            Else
                mesh.DrawSubset i
            End If
                    
        Next
    End If

    
    ' Then, draw the subsets with alpha
    If (bDrawAlphaSubsets And (bUseMaterialOverride Or bUseMaterials)) Then
        For i = 0 To m_NumMaterials - 1
        
            If (bUseMaterials) Then
                    If (m_Materials(i).diffuse.a < 1#) Then
                          g_dev.SetMaterial m_Materials(i)
                          g_dev.SetTexture 0, m_Textures(i)
                          mesh.DrawSubset i
                    End If
            ElseIf (bUseMaterialOverride) Then
                    If (m_MaterialOverride.diffuse.a < 1#) Then
                          g_dev.SetMaterial m_MaterialOverride
                          g_dev.SetTexture 0, m_Textures(i)
                          mesh.DrawSubset i
                    End If
            End If
        Next
    End If
    
End Sub

'-----------------------------------------------------------------------------
' Name: Render
' Desc: Render the mesh to the given device
'
' Note: Do not use for skinned meshes
'
'-----------------------------------------------------------------------------
Sub Render(dev As Direct3DDevice8)
    Dim i As Long
    
    If mesh Is Nothing Then Exit Sub
    
    If bSkinned = True Then Exit Sub
    
    If (bUseMaterials) Then
        For i = 0 To m_NumMaterials - 1
            g_dev.SetMaterial m_Materials(i)
            g_dev.SetTexture 0, m_Textures(i)
            mesh.DrawSubset i
        Next
    Else
        For i = 0 To m_NumMaterials - 1
            mesh.DrawSubset i
        Next

    End If
        
End Sub

'-----------------------------------------------------------------------------
' Name: RenderSkin
' Desc: Render the Mesh as skin
' Note: The mesh must have been loaded as a skin and bones must have been attached
'-----------------------------------------------------------------------------
Sub RenderSkin()

    If Not bSkinned Then Exit Sub
    
    Dim ipAttr As Long  'bonecombo attribute
    Dim matId As Long   'matrix id
    
    Dim i As Long
    Dim mat2 As D3DMATRIX
    Dim mat1 As D3DMATRIX
    Dim mat0 As D3DMATRIX

    
    g_dev.SetRenderState D3DRS_VERTEXBLEND, m_maxFaceInfl - 1
        
    For ipAttr = 0 To m_NumBoneCombos - 1
        For i = 0 To m_maxFaceInfl - 1
                matId = m_BoneComboIds(ipAttr).List(i)
                
                'If we get a MatId of -1 then all the vertex weights are 0
                'and we dont need to set the transform for this bone
                If matId <> -1 Then
                    mat0 = m_BoneFrames(matId).GetUpdatedMatrix()
                    mat1 = m_BoneOffsetMat(matId)
                    D3DXMatrixMultiply mat2, mat1, mat0
                    g_dev.SetTransform m_worldMatHandles(i), mat2
                End If
        Next
        g_dev.SetTexture 0, m_Textures(m_BoneCombos(ipAttr).AttribId)
        g_dev.SetMaterial m_Materials(m_BoneCombos(ipAttr).AttribId)
        
        mesh.DrawSubset ipAttr
        
    Next
    
    g_dev.SetRenderState D3DRS_VERTEXBLEND, 0
    
    
End Sub

'-----------------------------------------------------------------------------
' Name: GetMaterialCount
'
'---------------------------------------------------------------------------
Public Function GetMaterialCount() As Long
    GetMaterialCount = m_NumMaterials
End Function


'-----------------------------------------------------------------------------
' Name: SetMaterialOverride
' Desc: Sets the materail to be used in place of the ones loaded from file
' Note: to disable set bUseMaterialOverride to false
'-----------------------------------------------------------------------------
Public Sub SetMaterialOverride(m As D3DMATERIAL8)
    m_MaterialOverride = m
    bUseMaterialOverride = True
End Sub

'-----------------------------------------------------------------------------
' Name: GetMaterialOverride
' Desc:
'-----------------------------------------------------------------------------
Public Sub GetMaterialOverride(m As D3DMATERIAL8)
      m = m_MaterialOverride
End Sub


'-----------------------------------------------------------------------------
' Name: ClassName
' Desc:
'-----------------------------------------------------------------------------
Public Function ClassName() As String
    ClassName = "CD3DMesh"
End Function


'-----------------------------------------------------------------------------
' Name: InvalidateDeviceObjects
' Desc: Release reference to device dependent objects
'-----------------------------------------------------------------------------
Public Sub InvalidateDeviceObjects()
   'all framework objects are managed so nothing to do here
End Sub



'-----------------------------------------------------------------------------
' Name: RestoreDeviceObjects
' Desc: If we had any video memory objects they would need
'       to be reloaded here
'-----------------------------------------------------------------------------
Public Sub RestoreDeviceObjects(dev As Direct3DDevice8)
    
End Sub


'-----------------------------------------------------------------------------
' Name: InitFromD3DXMesh
' Desc: Allow mesh objects to be created from external D3DXMesh objects
'
'-----------------------------------------------------------------------------
Sub InitFromD3DXMesh(d3dxmeshIn As D3DXMesh)
    bUseMaterials = False
    ReDim m_Materials(1)
    ReDim m_Textures(1)
    m_NumMaterials = 1
    Set mesh = d3dxmeshIn
End Sub


'-----------------------------------------------------------------------------
' Name: SetMaterialCount
' Desc: If a mesh was initialized with InitFromD3DXMesh
'       This function can allocate space for Materials and Textures
'-----------------------------------------------------------------------------
Sub SetMaterialCount(n As Long)
    m_NumMaterials = n
    ReDim Preserve m_Materials(n)
    ReDim Preserve m_Textures(n)
End Sub

'-----------------------------------------------------------------------------
' Name: SetMaterialTexture
' Desc: Sets the texture for a given material subset
' Note: use nothing to remove a texture
'-----------------------------------------------------------------------------
Sub SetMaterialTexture(n As Long, tex As Direct3DTexture8)
    Set m_Textures(n) = tex
End Sub

'-----------------------------------------------------------------------------
' Name: GetMaterialTexture
' Desc: returns a given texture for a material subset
'-----------------------------------------------------------------------------
Function GetMaterialTexture(n As Long) As Direct3DTexture8
    Set GetMaterialTexture = m_Textures(n)
End Function


'-----------------------------------------------------------------------------
' Name: SetMaterial
' Desc: Sets the material properties for a given material subset
'-----------------------------------------------------------------------------
Sub SetMaterial(n As Long, material As D3DMATERIAL8)
    m_Materials(n) = material
End Sub


'-----------------------------------------------------------------------------
' Name: GetMaterial
' Desc: returns material properties for a material subset
'-----------------------------------------------------------------------------
Function GetMaterial(n As Long) As D3DMATERIAL8
    GetMaterial = m_Materials(n)
End Function


