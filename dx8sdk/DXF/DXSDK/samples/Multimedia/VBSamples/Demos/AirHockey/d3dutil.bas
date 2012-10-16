Attribute VB_Name = "D3DUtil"


'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       D3DUtil.Bas
'  Content:    VB D3DFramework utility module
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

' DOC:  Use with
' DOC:        D3DAnimation.cls
' DOC:        D3DFrame.cls
' DOC:        D3DMesh.cls
' DOC:        D3DSelectDevice.frm (optional)
' DOC:
' DOC:  Short list of usefull functions
' DOC:        D3DUtil_Init                  first call to framework
' DOC:        D3DUtil_LoadFromFile          loads an x-file
' DOC:        D3DUtil_SetupDefaultScene     setup a camera lights and materials
' DOC:        D3DUtil_SetupCamera           point camera
' DOC:        D3DUtil_SetupMediaPath        set directory to load textures from
' DOC:        D3DUtil_PresentAll            show graphic on the screen
' DOC:        D3DUtil_ResizeWindowed        resize for windowed modes
' DOC:        D3DUtil_ResizeFullscreen      resize to fullscreen mode
' DOC:        D3DUtil_CreateTextureInPool   create a texture


Option Explicit


' DOC: DXLockArray8 & DXUnlockArray8
' DOC:
' DOC: These are Helper functions that allow textures, vertex buffers, and index buffers
' DOC: to look like VB arrays to the VB user.
' DOC: It is imperative that Lock be matched with unlock or undefined behaviour may result
' DOC: It is imperative that DXLockarray8 be matched with DXUnlockArray8 or undefined behaviour may result
' DOC:
' DOC: DXLockArray8
' DOC:       resource    - can be Direct3DTexture8,Direct3dVertexBuffer8, or a Direct3DIndexBuffer
' DOC:       addr        - is the number provide by IndexBuffer.Lock,Testure.Lock etc
' DOC:       arr()       - a VB array that can be used to shadow video memory
' DOC: DXUnlockArray8
' DOC:       resource    - the resource passed to DXLockArray8
' DOC:       arr()       - the VB array passed to DXLockArray8
' DOC:
' DOC: Example
' DOC:           dim m_vertBuff as Direct3DVertexBuffer  'we assume this has been created
' DOC:           dim m_vertCount as long                 'we assume this has been set
' DOC:
' DOC:           Dim addr As Long                        'will holds the address the D3D
' DOC:                                                   'managed memory
' DOC:           dim verts() as D3DVERTEX                'array that we want to point to
' DOC:                                                   'D3D managed memory
' DOC:
' DOC:           redim verts(m_vertCount)                'ensure the size is large
' DOC:                                                   'enough for the data and has
' DOC:                                                   'as many dimensions as needed
' DOC:                                                   '(1d for vertex buffer, 2d for
' DOC:                                                   ' surfaces, 3d for volumes)
' DOC:                                                   'resize the array once and
' DOC:                                                   'reuse for frequent manipulation
' DOC:
' DOC:           m_vertBuff.Lock 0, Len(verts(0)) * m_vertCount, addr, 0
' DOC:
' DOC:           DXLockArray8 m_vertBuff, addr, verts
' DOC:
' DOC:           for i = 0 to m_vertCount-1
' DOC:               verts(i).x=i ' or what ever you want to dow with the data
' DOC:           next
' DOC:
' DOC:           DXUnlockArray8 m_vertBuff, verts
' DOC:
' DOC:           VB.Unlock
'
Public Declare Function DXLockArray8 Lib "dx8vb.dll" (ByVal resource As Direct3DResource8, ByVal addr As Long, arr() As Any) As Long
Public Declare Function DXUnlockArray8 Lib "dx8vb.dll" (ByVal resource As Direct3DResource8, arr() As Any) As Long



'DOC: Texture Load data applied to all textures
'DOC: can be accessed by g_TextureSampling variable
Private Type TextureParams
    enable As Boolean           'enable texture sampling
    
    width As Long               'default width of textures
    height As Long              'default height of textures
    miplevels As Long           'default number of miplevels
    mipfilter As Long           'default mipmap filter
    filter As Long              'default texture filter
    fmt As CONST_D3DFORMAT      'default texture format
    fmtTrans As CONST_D3DFORMAT 'default transparent format
    colorTrans As Long          'default transparent color
    
End Type


'DOC: Rotate key used in conjuction with the CD3DAnimation class
Public Type D3DROTATEKEY
    time As Long
    nFloats As Long
    quat As D3DQUATERNION
End Type

'DOC: Scale or Translate key used in conjuction with the CD3DAnimation class
Public Type D3DVECTORKEY
    time As Long
    nFloats As Long
    vec As D3DVECTOR
End Type

'DOC: Pick record using with CD3DPick class
Public Type D3D_PICK_RECORD
    hit As Long
    triFaceid As Long
    a       As Single
    b       As Single
    dist   As Single
End Type

'DOC: see D3DUtil_Timer
Public Enum TIMER_COMMAND
          TIMER_RESET = 1         '- to reset the timer
          TIMER_start = 2         '- to start the timer
          TIMER_STOP = 3          '- to stop (or pause) the timer
          TIMER_ADVANCE = 4       '- to advance the timer by 0.1 seconds
          TIMER_GETABSOLUTETIME = 5 '- to get the absolute system time
          TIMER_GETAPPTIME = 6      '- to get the current time
          TIMER_GETELLAPSEDTIME = 7 '- to get the ellapsed time
End Enum


'DOC: Info on a per texture basis
Private Type TexPoolEntry
    Name As String
    tex As Direct3DTexture8
    nextDelNode As Long
End Type



'------------------------------------------------------------------
' DOC: Usefull globals
'------------------------------------------------------------------


Public g_bDontDrawTextures As Boolean           ' Debuging switches
Public g_bClipMesh As Boolean                   ' Debuging switches
Public g_bLoadSkins  As Boolean                 ' Debuging switches
Public g_bLoadNoAlpha As Boolean                ' Debuging switches

                                                ' view frustrum (use as read only)
Public g_fov As Single                          ' view frustrum field of view
Public g_aspect As Single                       ' view frustrum aspect ratio
Public g_znear As Single                        ' view frustrum near plane
Public g_zfar As Single                         ' view frustrom far plane

                                                ' Matrices (use as read only)
Public g_identityMatrix As D3DMATRIX            ' Filled with Identity Matrix after D3DUtil_Init
Public g_worldMatrix As D3DMATRIX               ' Filled with current world matrix
Public g_viewMatrix As D3DMATRIX                ' Filled with current view matrix
Public g_projMatrix As D3DMATRIX                ' Filled with current projection matrix

                                                ' Clipplanes: use to ComputeClipPlanes to initialize
                                                ' helpfull for view frustrum culling
Public g_ClipPlanes() As D3DPLANE               ' Clipplane plane array
Public g_numClipPlanes As Long                  ' Number of clip planes in g_ClipPlanes

Public light0 As D3DLIGHT8                      ' light type usefull in imediate pane
Public light1 As D3DLIGHT8                      ' light type usefull in imediate pane
  
Public g_TextureSampling As TextureParams       ' defines how CreateTextureInPool sample textures

Public g_TextureLoadCallback  As Object         ' object that implements LoadTextureCallback(sName as string) as Direct3dTexture8
Public g_bUseTextureLoadCallback As Boolean     ' enables disables callback
  
Public g_mediaPath As String                    ' Path to media and texture
                                                ' read/write - must have ending backslash
                                                ' best to use SetMediaPath to initialize



'------------------------------------------------------------------
' Global constants
'------------------------------------------------------------------

Public Const g_pi = 3.1415
Public Const g_InvertRotateKey = True   'flag to turn on fix for animation key problem
Public Const D3DFVF_VERTEX = D3DFVF_XYZ Or D3DFVF_NORMAL Or D3DFVF_TEX1

'------------------------------------------------------------------
' Locals
'------------------------------------------------------------------

' TexturePool Mangement data. see..
'  D3DUTIL_LoadTextureIntoPool
'  D3DUTIL_AddTextureToPool
'  D3DUTIL_ReleaseTextureFromPool
'  D3DUTIL_ReleaseAllTexturesFromPool
'
Dim m_texPool() As TexPoolEntry
Dim m_maxPool As Long
Dim m_nextEmpty As Long
Dim m_firstDel As Long

Const kGrowSize = 10


'------------------------------------------------------------------
' Functions
'------------------------------------------------------------------

'-----------------------------------------------------------------------------
'DOC: D3DUtil_SetupDefaultScene
'DOC:
'DOC: helper function that initializes some default lighting and render states
'DOC:
'DOC: remarks:
'DOC:   sets defaults for
'DOC:   g_fov, g_aspect, g_znear, g_zfar
'DOC:   g_identityMatrix, g_projMatrix, g_ViewMatrix, g_worldMatrix
'DOC:   set device state for project view and world matrices
'DOC:   set device state for 2 directional lights (0 and 1)
'DOC:   set device state for a default grey material
'-----------------------------------------------------------------------------

Public Sub D3DUtil_SetupDefaultScene()
    
    g_fov = g_pi / 4
    g_aspect = 1
    g_znear = 1
    g_zfar = 3000
    
    If g_lWindowHeight <> 0 And g_lWindowWidth <> 0 Then g_aspect = g_lWindowHeight / g_lWindowWidth
    
    D3DXMatrixIdentity g_identityMatrix
    
    D3DXMatrixPerspectiveFovLH g_projMatrix, g_fov, g_aspect, g_znear, g_zfar
    
    g_dev.SetTransform D3DTS_PROJECTION, g_projMatrix
    
    D3DXMatrixLookAtLH g_viewMatrix, vec3(0, 0, -20), vec3(0, 0, 0), vec3(0, 1, 0)
    
    g_dev.SetTransform D3DTS_VIEW, g_viewMatrix
                 
    g_dev.SetTransform D3DTS_WORLD, g_identityMatrix
    
    'default light0
    
    light0.Ambient = ColorValue4(1, 0.1, 0.1, 0.1)
    light0.diffuse = ColorValue4(1, 1, 1, 1)
    light0.Type = D3DLIGHT_DIRECTIONAL
    light0.Range = 10000
    light0.Direction.x = -1
    light0.Direction.y = -1
    light0.Direction.z = -1
    D3DXVec3Normalize light0.Direction, light0.Direction
    g_dev.SetLight 0, light0
    g_dev.LightEnable 0, 1 'true
    
    'default light1
    
    light1.Ambient = ColorValue4(1, 0.3, 0.3, 0.3)
    light1.diffuse = ColorValue4(1, 1, 1, 1)
    light1.Type = D3DLIGHT_DIRECTIONAL
    light1.Range = 10000
    light1.Direction.x = 1
    light1.Direction.y = -1
    light1.Direction.z = -1
    D3DXVec3Normalize light1.Direction, light1.Direction
    'g_dev.SetLight 1, light1
    'g_dev.LightEnable 1, 1 'true
        
        
    'set first material
    Dim material0 As D3DMATERIAL8
    material0.Ambient = ColorValue4(1, 0.2, 0.2, 0.2)
    material0.diffuse = ColorValue4(1, 0.5, 0.5, 0.5)
    material0.power = 10
    g_dev.SetMaterial material0
    
    With g_dev
        Call .SetRenderState(D3DRS_AMBIENT, &H10101010)
        Call .SetRenderState(D3DRS_CLIPPING, 1)             'CLIPPING IS ON
        Call .SetRenderState(D3DRS_LIGHTING, 1)             'LIGHTING IS ON
        Call .SetRenderState(D3DRS_ZENABLE, 1)              'USE ZBUFFER
        Call .SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD)
        
    End With
    
End Sub


'-----------------------------------------------------------------------------
'DOC: D3DUtil_SetupCamera
'DOC: Params
'DOC:   fromV   world space vector of camera position
'DOC:   toV     world space vector of position camera is looking toward
'DOC:   upV     world space vector of cameras up direction
'DOC: Remarks
'DOC:   effects g_viewMatrix and device ViewMatrix state
'DOC:   Make sure upV is different than the direction of sight
'-----------------------------------------------------------------------------
 Sub D3DUtil_SetupCamera(fromV As D3DVECTOR, toV As D3DVECTOR, upV As D3DVECTOR)
 
    D3DXMatrixLookAtLH g_viewMatrix, fromV, toV, upV
    g_dev.SetTransform D3DTS_VIEW, g_viewMatrix
    
 End Sub

 
'-----------------------------------------------------------------------------
'DOC: D3DUtil_ClearAll
'DOC: Params
'DOC:   col     color to clear the backbuffer
'DOC: Remarks
'DOC:   convenience function that assumes a rendertarget with a zbuffer and
'DOC:   no stencil
'-----------------------------------------------------------------------------

Function D3DUtil_ClearAll(col As Long)
    g_dev.Clear 0, ByVal 0, D3DCLEAR_TARGET Or D3DCLEAR_ZBUFFER, col, 1, 0
End Function




'-----------------------------------------------------------------------------
'DOC: D3DUtil_PresentAll
'DOC: Params
'DOC:   hwnd    hwnd to present to
'DOC: Remarks
'DOC:   Convience function that presents the contents of a backbuffer to an hwnd
'DOC:
'-----------------------------------------------------------------------------
Function D3DUtil_PresentAll(hwnd As Long)
    On Local Error Resume Next
    g_dev.Present ByVal 0, ByVal 0, hwnd, ByVal 0
    
    'some video cards leave the divide by zero flag set after Present or EndScene
    'here we force a divide by zero to force vb to reset the flag so the next
    'math operation doesnt give a divide by zero error
    Dim dummy As Single
    dummy = dummy / 0
    Err.Clear
    
End Function


'-----------------------------------------------------------------------------
'DOC: ColorValue4
'DOC: Params
'DOC:   a r g b   values valid between 0.0 and 1.0
'DOC: Return Value
'DOC:   a filled D3DCOLORVALUE type
'-----------------------------------------------------------------------------
Function ColorValue4(a As Single, r As Single, g As Single, b As Single) As D3DCOLORVALUE
    Dim c As D3DCOLORVALUE
    c.a = a
    c.r = r
    c.g = g
    c.b = b
    ColorValue4 = c
End Function

'-----------------------------------------------------------------------------
'DOC: Vec2
'DOC: Params
'DOC:   x y z   vector values
'DOC: Return Value
'DOC:   a filled D3DVECTOR type
'-----------------------------------------------------------------------------
Function vec2(x As Single, y As Single) As D3DVECTOR2
    vec2.x = x
    vec2.y = y
End Function


'-----------------------------------------------------------------------------
'DOC: Vec3
'DOC: Params
'DOC:   x y z   vector values
'DOC: Return Value
'DOC:   a filled D3DVECTOR type
'-----------------------------------------------------------------------------
Function vec3(x As Single, y As Single, z As Single) As D3DVECTOR
    vec3.x = x
    vec3.y = y
    vec3.z = z
End Function


'-----------------------------------------------------------------------------
'DOC: Vec4
'DOC: Params
'DOC:   x y z w  vector values
'DOC: Return Value
'DOC:   a filled D3DVECTOR type
'-----------------------------------------------------------------------------
Function vec4(x As Single, y As Single, z As Single, w As Single) As D3DVECTOR4
    vec4.x = x
    vec4.y = y
    vec4.z = z
    vec4.w = w
End Function


'-----------------------------------------------------------------------------
'DOC: D3DUtil_RotationAxis
'DOC: Params
'DOC:   x y z               axis of rotation
'DOC:   rotationInDegrees   rotationInDegrees
'DOC:
'DOC: Return Value
'DOC:   a filled D3DQUATERNION type
'-----------------------------------------------------------------------------
Function D3DUtil_RotationAxis(x As Single, y As Single, z As Single, rotationInDegrees As Single) As D3DQUATERNION
    Dim quat As D3DQUATERNION
    D3DXQuaternionRotationAxis quat, vec3(x, y, z), (rotationInDegrees / 180) * g_pi
    D3DUtil_RotationAxis = quat
End Function



'-----------------------------------------------------------------------------
'DOC: D3DUtil_CreateTexture()
'DOC: Params
'DOC:   dev     Direct3DDevice  (almost always g_dev)
'DOC:   strFile name of the file to load
'DOC:   fmt     prefered format (may be UNKNOWN)
'DOC: Return Value
'DOC:   a Direct3DTexture object
'DOC: Remarks
'DOC:   Helper function to create a texture. It checks the root path first,
'DOC:   then tries the media path (as set by setMediaPath)
'------------------------------------------------------------------------------

Function D3DUtil_CreateTexture(dev As Direct3DDevice8, strFile As String, fmt As CONST_D3DFORMAT) As Direct3DTexture8

    On Local Error GoTo ErrOut
    Dim strPath As String
    Dim tex As Direct3DTexture8
    
    strPath = strFile
    If Dir$(strFile) = "" Then strPath = g_mediaPath + strFile
    
    Set tex = g_d3dx.CreateTextureFromFileEx(dev, strPath, _
                        D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, _
                        fmt, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, ByVal 0, ByVal 0)
    
    Set D3DUtil_CreateTexture = tex
    Exit Function
    
ErrOut:
    Set D3DUtil_CreateTexture = Nothing
End Function
 
 
 
'-----------------------------------------------------------------------------
'DOC: D3DUtil_CreateTextureInPool()
'DOC: Params
'DOC:   dev     Direct3DDevice (all most always g_dev)
'DOC:   strFile name of the texture to load
'DOC:   fmt     prefered format (may be UNKNOWN)
'DOC: Return Value
'DOC:   a Direct3DTexture object
'DOC: Remarks
'DOC:   Helper function to create a texture. It checks the root path first,
'DOC:   then tries the media path (as set by setMediaPath)
'DOC:   This function differs from D3DUtil_CreateTexture in that
'DOC:   Multiple calls with the same texture name will result in
'DOC:   returning the same texture object
'DOC:   To release all texture to allow a device change see
'DOC:   D3DUtil_ReleaseAllTexturesFromPool
'-----------------------------------------------------------------------------

Function D3DUtil_CreateTextureInPool(dev As Direct3DDevice8, strFile As String, ByVal fmt As CONST_D3DFORMAT) As Direct3DTexture8
    
    'Debug.Print strFile
              
    On Local Error GoTo ErrOut
      
    Dim strPath As String
    Dim tex As Direct3DTexture8
    
    Dim transcolor As Long
    Dim miplev As Long
    Dim filter As Long
    Dim mipfilter As Long
    Dim w As Long
    Dim h As Long
    
    If strFile = "" Then Exit Function
    
    Set tex = D3DUtil_FindTextureInPool(strFile)
    If Not tex Is Nothing Then
        Set D3DUtil_CreateTextureInPool = tex
        Exit Function
    End If
        
    
    strPath = strFile
    If Dir$(strFile) = "" Then strPath = g_mediaPath + strFile
    
    
    If Not g_TextureSampling.enable Then
        fmt = D3DFMT_UNKNOWN
        miplev = D3DX_DEFAULT
        filter = D3DX_DEFAULT
        mipfilter = D3DX_DEFAULT
        w = D3DX_DEFAULT
        h = D3DX_DEFAULT
        transcolor = 0
    Else
        miplev = g_TextureSampling.miplevels
        filter = g_TextureSampling.filter
        mipfilter = g_TextureSampling.mipfilter
        w = g_TextureSampling.width
        h = g_TextureSampling.height
        fmt = g_TextureSampling.fmt
        transcolor = 0
    End If
     
    'Check for color keys
    If (InStr(strFile, "_t.") <> 0) And g_TextureSampling.enable Then
       fmt = g_TextureSampling.fmtTrans
       transcolor = g_TextureSampling.colorTrans Or &HFF000000
    End If
            
    If (InStr(strFile, ".tif") <> 0) Then
        transcolor = 0
        fmt = g_TextureSampling.fmtTrans
    End If
    
    Set tex = g_d3dx.CreateTextureFromFileEx(dev, strPath, _
                   w, h, miplev, 0, fmt, D3DPOOL_MANAGED, _
                   filter, mipfilter, transcolor, ByVal 0, ByVal 0)
         
    D3DUtil_AddTextureToPool tex, strFile
    
    Set D3DUtil_CreateTextureInPool = tex
    Exit Function
    
ErrOut:
    Set D3DUtil_CreateTextureInPool = Nothing
End Function
 
'-----------------------------------------------------------------------------
'DOC: D3DUtil_ReleaseTextureFromPool()
'DOC: Params
'DOC:   strFileName name of the texture to release
'-----------------------------------------------------------------------------

Sub D3DUtil_ReleaseTextureFromPool(strFileName As String)
    Dim i As Long
    For i = 0 To m_nextEmpty - 1
        If (m_texPool(i).Name = strFileName) Then
            m_texPool(i).nextDelNode = m_firstDel
            m_firstDel = i
            m_texPool(i).Name = ""
            Set m_texPool(i).tex = Nothing
            Exit Sub
        End If
    Next
End Sub
 
 
'-----------------------------------------------------------------------------
'DOC: D3DUtil_FindTextureInPool
'DOC: Params
'DOC:   strFileName name of the texture to release
'DOC: Return value
'DOC:   texture that was found or Nothing if name not found
'DOC:   Will return the first match. Is case sensitive
'-----------------------------------------------------------------------------

Function D3DUtil_FindTextureInPool(strFileName As String) As Direct3DTexture8
    Dim i As Long
    For i = 0 To m_nextEmpty - 1
        If (m_texPool(i).Name = strFileName) Then
            Set D3DUtil_FindTextureInPool = m_texPool(i).tex
            Exit Function
        End If
    Next
End Function
 
 
'-----------------------------------------------------------------------------
'DOC: D3DUtil_ReleaseAllTexturesFromPool()
'DOC: Remarks
'DOC:   Release all textures from the pool
'DOC:   This is required before attempting to completely release a device
'DOC:   as the device still exists until all objects created from it
'DOC:   are also released or set to Nothing
'-----------------------------------------------------------------------------

Sub D3DUtil_ReleaseAllTexturesFromPool()
    ReDim m_texPool(0)
    m_firstDel = -1
    m_nextEmpty = 0
    m_maxPool = 0
End Sub

'-----------------------------------------------------------------------------
'DOC: D3DUtil_AddTextureToPool
'DOC: Params
'DOC:   tex         Direct3DTexture to add
'DOC:   strFile     name to associate with texture
'DOC: Remarks
'DOC:   Be aware that the Texture pool names are case sensitive
'-----------------------------------------------------------------------------

Function D3DUtil_AddTextureToPool(tex As Direct3DTexture8, strFile As String)
    
    'If the list is empty - makeit
    If m_maxPool = 0 Then
        ReDim m_texPool(kGrowSize)
        m_maxPool = kGrowSize
        m_firstDel = -1
        Set m_texPool(0).tex = tex
        m_texPool(0).Name = strFile
        m_texPool(0).nextDelNode = -1
        Exit Function
    End If
    
    'If there are items that are deleted then
    'use there slots
    If m_firstDel > -1 Then
        Set m_texPool(m_firstDel).tex = tex
        m_texPool(m_firstDel).Name = strFile
        m_firstDel = m_texPool(m_firstDel).nextDelNode
        m_texPool(m_firstDel).nextDelNode = -1
        Exit Function
    End If
    
    'If there are no slots left create a slot
    If m_maxPool <= m_nextEmpty Then
        m_maxPool = m_maxPool + kGrowSize
        ReDim Preserve m_texPool(m_maxPool)
    End If
    
    'assign the entry to the next available slot
    Set m_texPool(m_nextEmpty).tex = tex
    m_texPool(m_nextEmpty).Name = strFile
    m_texPool(m_nextEmpty).nextDelNode = -1
    m_nextEmpty = m_nextEmpty + 1
    
        
End Function
 
'-----------------------------------------------------------------------------
'DOC: D3DUtil_SetTextureLoadCallback
'DOC:
'DOC: Params
'DOC:   obj     can equal nothing to remove the callback from use
'DOC:           or an object that implements
'DOC:           LoadTextureCallback(sName as string) as Direct3dTexture8
'DOC:
'-----------------------------------------------------------------------------
Sub D3DUtil_SetTextureLoadCallback(obj As Object)
    Set g_TextureLoadCallback = obj
    
    g_bUseTextureLoadCallback = False
    If obj Is Nothing Then Exit Sub
    
    g_bUseTextureLoadCallback = True
End Sub


'-----------------------------------------------------------------------------
'DOC: D3DUtil_SetTextureSampling
'DOC:
'DOC: Params
'DOC:      bEnable     Enable/Disable sampling defaults
'DOC:      w           default width for all textures
'DOC:      h           default height for all texture
'DOC:      levels      default number of miplevels
'DOC:      fmt         default texture format
'DOC:      transfmt    default texture format for alpha textures
'DOC:      transcolor  transparent color to mask for textures with file names
'DOC:                  ending in xxxx_t.bmp or xxxx_t.yyy
'-----------------------------------------------------------------------------
Sub D3DUtil_SetTextureSampling(bEnable As Boolean, w As Long, h As Long, levels As Long, fmt As CONST_D3DFORMAT, transfmt As CONST_D3DFORMAT, transcolor As Long)
    With g_TextureSampling
        .enable = bEnable
        .width = w
        .height = h
        .miplevels = levels
        .mipfilter = D3DX_FILTER_BOX
        .filter = D3DX_FILTER_BOX
        .fmt = fmt
        .colorTrans = transcolor
        .fmtTrans = transfmt
    End With
End Sub


'-----------------------------------------------------------------------------
'DOC: D3DUtil_CreateFrame
'DOC:
'DOC: Params:
'DOC:       parent      Parent whos child is the returned frame
'DOC:                   can be Nothing
'DOC:
'DOC: Returns
'DOC:       New Frame object
'-----------------------------------------------------------------------------
Function D3DUtil_CreateFrame(parent As CD3DFrame) As CD3DFrame
    Set D3DUtil_CreateFrame = New CD3DFrame
    If parent Is Nothing Then Exit Function
    parent.AddChild D3DUtil_CreateFrame
End Function


'-----------------------------------------------------------------------------
' Name: D3DUtil_LoadFromFile
'-----------------------------------------------------------------------------
Function D3DUtil_LoadFromFile(strFile As String, parentFrame As CD3DFrame, parentAnimation As CD3DAnimation) As CD3DFrame
    On Local Error GoTo ErrOut
    Dim newFrame As CD3DFrame
    Set newFrame = New CD3DFrame
    
    g_bLoadSkins = False
    newFrame.InitFromFile g_dev, strFile, parentFrame, parentAnimation
    Set D3DUtil_LoadFromFile = newFrame
    
    'CONSIDER the need to set the FVF on load.
    newFrame.SetFVF g_dev, D3DFVF_VERTEX
    Exit Function
    
ErrOut:
    Set D3DUtil_LoadFromFile = Nothing
End Function


'-----------------------------------------------------------------------------
' Name: D3DUtil_LoadFromFileAsSkin
'-----------------------------------------------------------------------------
Function D3DUtil_LoadFromFileAsSkin(strFile As String, parentFrame As CD3DFrame, parentAnimation As CD3DAnimation) As CD3DFrame
    On Local Error GoTo ErrOut
    Dim newFrame As CD3DFrame
    Set newFrame = New CD3DFrame
    g_bLoadSkins = True
    newFrame.InitFromFile g_dev, strFile, parentFrame, parentAnimation
    newFrame.AttatchBonesToMesh newFrame
    Set D3DUtil_LoadFromFileAsSkin = newFrame
    Exit Function
    
ErrOut:
    Set D3DUtil_LoadFromFileAsSkin = Nothing
End Function


'-----------------------------------------------------------------------------
' Name: D3DUtil_LoadFromFileAsMesh
'-----------------------------------------------------------------------------
Function D3DUtil_LoadFromFileAsMesh(strFile As String) As CD3DMesh
    On Local Error GoTo ErrOut
    Dim newmesh As CD3DMesh
    Set newmesh = New CD3DMesh
    newmesh.InitFromFile g_dev, strFile
    Set D3DUtil_LoadFromFileAsMesh = newmesh
    newmesh.SetFVF g_dev, D3DFVF_VERTEX
    Exit Function
    
ErrOut:
    Set D3DUtil_LoadFromFileAsMesh = Nothing
End Function

'-----------------------------------------------------------------------------
' Name: D3DUtil_SetMediaPath
'-----------------------------------------------------------------------------
Sub D3DUtil_SetMediaPath(path As String)
    g_mediaPath = path
End Sub



'-----------------------------------------------------------------------------
' Name: D3DUtil_InitLight
'-----------------------------------------------------------------------------
Sub D3DUtil_InitLight(lgt As D3DLIGHT8, ty As CONST_D3DLIGHTTYPE, x As Single, y As Single, z As Single)
    lgt.Type = ty
    lgt.diffuse = ColorValue4(1, 1, 1, 1)
    lgt.Attenuation1 = 0.01
    lgt.Attenuation0 = 0
    lgt.Attenuation2 = 0
    lgt.Range = 99999999
    lgt.Direction.x = x
    lgt.Direction.y = y
    lgt.Direction.z = z
    lgt.Position.x = x
    lgt.Position.y = y
    lgt.Position.z = z
    
          
End Sub


'-----------------------------------------------------------------------------
' Name: D3DUtil_ComputeClipPlanes
'-----------------------------------------------------------------------------
Sub D3DUtil_ComputeClipPlanes(veye As D3DVECTOR, vat As D3DVECTOR, vUp As D3DVECTOR, fov As Single, front As Single, back As Single, aspect As Single)
    
    Dim vDir As D3DVECTOR
    Dim vright As D3DVECTOR
        
    Dim vFrontCenter As D3DVECTOR
    Dim vFrontUp As D3DVECTOR
    Dim vFrontRight As D3DVECTOR
    
    Dim vBackCenter As D3DVECTOR
    
    Dim vBackRight As D3DVECTOR
    Dim vbackLeft As D3DVECTOR
    
    Dim vBackRightTop As D3DVECTOR
    Dim vBackLeftTop As D3DVECTOR
    
    Dim vBackRightBot As D3DVECTOR
    Dim vBackLeftBot As D3DVECTOR
        
    Dim dx As Single
    Dim dy As Single
    
    'Establish our basis vector
    D3DXVec3Subtract vDir, vat, veye
    D3DXVec3Normalize vDir, vDir
    D3DXVec3Normalize vUp, vUp
    D3DXVec3Cross vright, vDir, vUp
    
    dx = Tan(fov / 2) * back
    dy = dx * aspect
        
    '
    '
    '              /|  vbackleft (top,bot)
    '             / |
    '        vfront |
    '           /|  |
    '       eye ----|  vbackcenter
    '           \|  |
    '            \  |dx
    '             \ |
    '              \|  vbackright (top,bot)
    '
    '
    
    
    'compute vbackcenter
    D3DXVec3Scale vBackCenter, vDir, back
    D3DXVec3Add vBackCenter, vBackCenter, veye
    
       
    'compute vbackright
    D3DXVec3Scale vBackRight, vright, dx
    D3DXVec3Add vBackRight, vBackCenter, vBackRight
    
    
    'compute vbackleft
    D3DXVec3Scale vbackLeft, vright, -dx
    D3DXVec3Add vbackLeft, vBackCenter, vbackLeft
 
    'compute vbackrighttop
    D3DXVec3Scale vBackRightTop, vUp, dy
    D3DXVec3Add vBackRightTop, vBackRight, vBackRightTop
    
    
    'compute vbacklefttop
    D3DXVec3Scale vBackLeftTop, vUp, dy
    D3DXVec3Add vBackLeftTop, vBackRight, vBackLeftTop
        
     'compute vbackrightbot
    D3DXVec3Scale vBackRightBot, vUp, -dy
    D3DXVec3Add vBackRightBot, vBackRight, vBackRightBot
       
    'compute vbackleftbot
    D3DXVec3Scale vBackLeftBot, vUp, -dy
    D3DXVec3Add vBackLeftBot, vBackRight, vBackLeftBot
    
        
    'compute vfrontcenter
    D3DXVec3Scale vFrontCenter, vDir, front
    D3DXVec3Add vFrontCenter, vFrontCenter, veye
 
    'compute vfrontright
    D3DXVec3Scale vFrontRight, vright, dx
    D3DXVec3Add vFrontRight, vFrontCenter, vFrontRight
 
    'compute vfrontup
    D3DXVec3Scale vFrontUp, vUp, dy
    D3DXVec3Add vFrontUp, vFrontCenter, vFrontUp
 
    ReDim g_ClipPlanes(6)
    g_numClipPlanes = 6
    
    
    
    'front plane
    D3DXPlaneFromPointNormal g_ClipPlanes(0), veye, vDir
    
    'back plane
    Dim vnegdir As D3DVECTOR
    D3DXVec3Scale vnegdir, vDir, -1
    D3DXPlaneFromPointNormal g_ClipPlanes(1), vBackCenter, vnegdir
    
    'right plane
    D3DXPlaneFromPoints g_ClipPlanes(2), veye, vBackRightTop, vBackRightBot
    
    'left plane
    D3DXPlaneFromPoints g_ClipPlanes(3), veye, vBackLeftTop, vBackLeftBot
    
    'top plane
    D3DXPlaneFromPoints g_ClipPlanes(4), veye, vBackLeftTop, vBackRightTop
    
    'bot plane
    D3DXPlaneFromPoints g_ClipPlanes(5), veye, vBackRightBot, vBackLeftBot
    
    g_numClipPlanes = 4
    
End Sub


'-----------------------------------------------------------------------------
'DOC: D3DUtil_IntersectTriangleCull
'DOC: Params
'DOC:   v0,v1,v2    points from a triangle
'DOC:   vDir        direction vector of a ray to intersect triangle
'DOC:   vOrig       origen of ray to interesect triangle
'DOC:   t           distance from origen to intersection
'DOC:   u,v         u v coordinates of intersection
'DOC: Return value
'DOC:   true if intersected, false if not
'DOC:
'DOC: See CD3DPick object if intersecting a ray with a mesh or frame
'-----------------------------------------------------------------------------

Function D3DUtil_IntersectTriangleCull(ByRef v0 As D3DVECTOR, ByRef v1 As D3DVECTOR, ByRef v2 As D3DVECTOR, vDir As D3DVECTOR, vOrig As D3DVECTOR, t As Single, u As Single, v As Single) As Boolean

    Dim edge1 As D3DVECTOR
    Dim edge2 As D3DVECTOR
    Dim pvec As D3DVECTOR
    Dim tvec As D3DVECTOR
    Dim qvec As D3DVECTOR
    Dim det As Single
    Dim fInvDet As Single
    
    'find vectors for the two edges sharing vert0
    D3DXVec3Subtract edge1, v1, v0
    D3DXVec3Subtract edge2, v2, v0
    
    'begin calculating the determinant - also used to caclulate u parameter
    D3DXVec3Cross pvec, vDir, edge2
    
    'if determinant is nearly zero, ray lies in plane of triangle
    det = D3DXVec3Dot(edge1, pvec)
    If (det < 0.0001) Then
        Exit Function
    End If
    
    'calculate distance from vert0 to ray origin
    D3DXVec3Subtract tvec, vOrig, v0

    'calculate u parameter and test bounds
    u = D3DXVec3Dot(tvec, pvec)
    If (u < 0 Or u > det) Then
        Exit Function
    End If
    
    'prepare to test v parameter
    D3DXVec3Cross qvec, tvec, edge1
    
    'calculate v parameter and test bounds
    v = D3DXVec3Dot(vDir, qvec)
    If (v < 0 Or (u + v > det)) Then
        Exit Function
    End If
    
    'calculate t, scale parameters, ray intersects triangle
    t = D3DXVec3Dot(edge2, qvec)
    fInvDet = 1 / det
    t = t * fInvDet
    u = u * fInvDet
    v = v * fInvDet
    If t = 0 Then Exit Function
    
    D3DUtil_IntersectTriangleCull = True
    
End Function


'-----------------------------------------------------------------------------
' Name: D3DUtil_IsSphereVisible
'-----------------------------------------------------------------------------
Function D3DUtil_IsSphereVisible(SphereCenterInWorldSpace As D3DVECTOR, SphereRadius As Single) As Long
    
    Dim i As Long
    Dim dist As Single
    
    For i = 0 To g_numClipPlanes - 1
        dist = D3DXMATH_PLANE.D3DXPlaneDotCoord(g_ClipPlanes(i), SphereCenterInWorldSpace)
        If dist < -1 * SphereRadius Then
            'sphere is completely behind the plane
            'if its behind any plane then its clipped
            'Debug.Print SphereRadius
'            If i = 2 Then Stop
            D3DUtil_IsSphereVisible = 0
            Exit Function
        End If
    Next

    D3DUtil_IsSphereVisible = 1
    
End Function



'-----------------------------------------------------------------------------
' Name: D3DPLANE4
'-----------------------------------------------------------------------------
Function D3DPLANE4(a As Single, b As Single, c As Single, d As Single) As D3DPLANE
    
    D3DPLANE4.a = a
    D3DPLANE4.b = b
    D3DPLANE4.c = c
    D3DPLANE4.d = d
    
End Function

'-----------------------------------------------------------------------------
' Name: FtoDW
'
' For calls that require that a single be packed into a long
' (such as some calls to SetRenderState) this function will do just that
'-----------------------------------------------------------------------------
Function FtoDW(f As Single) As Long
    Dim buf As D3DXBuffer
    Dim l As Long
    Set buf = g_d3dx.CreateBuffer(4)
    g_d3dx.BufferSetData buf, 0, 4, 1, f
    g_d3dx.BufferGetData buf, 0, 4, 1, l
    FtoDW = l
End Function

'-----------------------------------------------------------------------------
' Name: LONGtoD3DCOLORVALUE
'-----------------------------------------------------------------------------
Function LONGtoD3DCOLORVALUE(color As Long) As D3DCOLORVALUE
    Dim a As Long, r As Long, g As Long, b As Long
        
    If color < 0 Then
        a = ((color And (&H7F000000)) / (2 ^ 24)) Or &H80&
    Else
        a = color / (2 ^ 24)
    End If
    r = (color And &HFF0000) / (2 ^ 16)
    g = (color And &HFF00&) / (2 ^ 8)
    b = (color And &HFF&)
    
    LONGtoD3DCOLORVALUE.a = a / 255
    LONGtoD3DCOLORVALUE.r = r / 255
    LONGtoD3DCOLORVALUE.g = g / 255
    LONGtoD3DCOLORVALUE.b = b / 255
        
End Function


'-----------------------------------------------------------------------------
' Name: D3DCOLORVALUEtoLONG
'-----------------------------------------------------------------------------

Function D3DCOLORVALUEtoLONG(cv As D3DCOLORVALUE) As Long
    Dim r As Long
    Dim g As Long
    Dim b As Long
    Dim a As Long
    Dim c As Long
    
    r = cv.r * 255
    g = cv.g * 255
    b = cv.b * 255
    a = cv.a * 255
    
    If a > 127 Then
        a = a - 128
        c = a * 2 ^ 24 Or &H80000000
        c = c Or r * 2 ^ 16
        c = c Or g * 2 ^ 8
        c = c Or b
    Else
        c = a * 2 ^ 24
        c = c Or r * 2 ^ 16
        c = c Or g * 2 ^ 8
        c = c Or b
    End If
    
    D3DCOLORVALUEtoLONG = c
End Function

'-----------------------------------------------------------------------------
' Name: DXUtil_Timer()
' Desc: Performs timer opertations. Use the following commands:
'          TIMER_RESET           - to reset the timer
'          TIMER_START           - to start the timer
'          TIMER_STOP            - to stop (or pause) the timer
'          TIMER_ADVANCE         - to advance the timer by 0.1 seconds
'          TIMER_GETABSOLUTETIME - to get the absolute system time
'          TIMER_GETAPPTIME      - to get the current time
'          TIMER_GETELLAPSEDTIME - to get the ellapsed time between calls
'-----------------------------------------------------------------------------
Function DXUtil_Timer(command As TIMER_COMMAND) As Single

    On Local Error Resume Next

    Static m_bTimerInitialized  As Boolean
    Static m_bUsingQPF         As Boolean
    Static m_fSecsPerTick  As Single
    Static m_fBaseTime    As Single
    Static m_fStopTime     As Single
    Static m_fLastTime As Single
    
    Dim fTime As Single

    ' Initialize the timer
    If (False = m_bTimerInitialized) Then
        m_bTimerInitialized = True
    End If

    fTime = Timer

    ' Reset the timer
    If (command = TIMER_RESET) Then
        m_fBaseTime = fTime
        m_fStopTime = 0
        m_fLastTime = 0
        DXUtil_Timer = 0
        Exit Function
    End If

    ' Return the current time
    If (command = TIMER_GETAPPTIME) Then
        DXUtil_Timer = fTime - m_fBaseTime
        Exit Function
    End If

    ' Start the timer
    If (command = TIMER_start) Then
        m_fBaseTime = m_fBaseTime + fTime - m_fStopTime
        m_fLastTime = m_fLastTime + fTime - m_fStopTime
        m_fStopTime = 0
    End If
    
    ' Stop the timer
    If (command = TIMER_STOP) Then
        m_fStopTime = fTime
    End If
     
    ' Advance the timer by 1/10th second
    If (command = TIMER_ADVANCE) Then
        m_fBaseTime = m_fBaseTime + fTime - (m_fStopTime + 0.1)
    End If
    
    
    ' Return ellapsed time
    If (command = TIMER_GETELLAPSEDTIME) Then
        DXUtil_Timer = fTime - m_fLastTime
        m_fLastTime = fTime
        If DXUtil_Timer < 0 Then DXUtil_Timer = 0
        Exit Function
    End If
    
    
    DXUtil_Timer = fTime
End Function




