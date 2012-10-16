VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "CD3DAnimation"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       D3DAnimation.cls
'  Content:    D3D Visual Basic Framework Animation Class
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Option Explicit

Public ObjectName As String

Private Type KEYHEADER
    keytype As Long
    keycount As Long
End Type

Private Type RMROTATEKEY
    time As Long
    nFloats As Long
    w As Single
    x As Single
    y As Single
    z As Single
End Type

Private Type D3DMATRIXKEY
    time As Long
    nFloats As Long
    matrix As D3DMATRIX
End Type


Const kAnimGrowSize = 10

Dim m_RotateKeys() As D3DROTATEKEY
Dim m_ScaleKeys() As D3DVECTORKEY
Dim m_PositionKeys() As D3DVECTORKEY
Dim m_RMRotateKeys() As RMROTATEKEY
Dim m_MatrixKeys() As D3DMATRIXKEY

Dim m_NumRotateKeys As Long
Dim m_NumScaleKeys As Long
Dim m_NumPositionKeys As Long
Dim m_NumMatrixKeys As Long
Dim m_strFrameName As String
Dim m_frame As CD3DFrame
Dim m_iMatrixKey As Long


Dim m_Children() As CD3DAnimation
Dim m_NumChildren As Long
Dim m_MaxChildren As Long

'-----------------------------------------------------------------------------
' Name: ParseAnimSet
' Desc: called from D3DUtil_LoadFromFile
'-----------------------------------------------------------------------------
Friend Sub ParseAnimSet(FileData As DirectXFileData, parentFrame As CD3DFrame)
    On Local Error Resume Next
    ObjectName = FileData.GetName()
    
    Dim ChildData As DirectXFileData
    Dim NewAnim As CD3DAnimation
    Dim ChildObj As DirectXFileObject
    Dim ChildRef As DirectXFileReference
    
    Set ChildObj = FileData.GetNextObject()
    
     Do While Not ChildObj Is Nothing
    
        Set ChildData = ChildObj
        If Err.Number = 0 Then
  
            If ChildData.GetType = "TID_D3DRMAnimation" Then
                Set NewAnim = New CD3DAnimation
                AddChild NewAnim
                NewAnim.ParseAnim ChildData, Me, parentFrame
            End If
        End If
        
        Err.Clear
        Set ChildRef = ChildObj
        
        If Err.Number = 0 Then
            Set ChildData = ChildRef.Resolve
           
            Set NewAnim = New CD3DAnimation
            AddChild NewAnim
            NewAnim.ParseAnim ChildData, Me, parentFrame
        End If
        
        Err.Clear
        
        Set ChildObj = FileData.GetNextObject()
    Loop
    
End Sub

'-----------------------------------------------------------------------------
' Name: GetChild
' Desc: return child Animation
'-----------------------------------------------------------------------------

Public Function GetChild(i As Long) As CD3DAnimation
    Set GetChild = m_Children(i)
End Function

'-----------------------------------------------------------------------------
' Name: GetChildCount
' Desc: return number of child animations
'-----------------------------------------------------------------------------

Public Function GetChildCount() As Long
    GetChildCount = m_NumChildren
End Function


'-----------------------------------------------------------------------------
' Name: AddChild
' Desc: Add child animation
'-----------------------------------------------------------------------------

Public Sub AddChild(child As CD3DAnimation)
    If child Is Nothing Then Exit Sub
    
    If m_MaxChildren = 0 Then
        m_MaxChildren = kAnimGrowSize
        ReDim m_Children(m_MaxChildren)
    ElseIf m_NumChildren >= m_MaxChildren Then
        m_MaxChildren = m_MaxChildren + kAnimGrowSize
        ReDim Preserve m_Children(m_MaxChildren)
    End If
    
    Set m_Children(m_NumChildren) = child
    m_NumChildren = m_NumChildren + 1
End Sub


'-----------------------------------------------------------------------------
' Name: SetFrame
' Desc: set Frame to be animated
'-----------------------------------------------------------------------------
Public Sub SetFrame(frame As CD3DFrame)
    Set m_frame = frame
    m_strFrameName = frame.ObjectName
End Sub

'-----------------------------------------------------------------------------
' Name: GetFrame
' Desc: return frame being animated
'-----------------------------------------------------------------------------
Public Function GetFrame() As CD3DFrame
    Set GetFrame = m_frame
End Function


'-----------------------------------------------------------------------------
' Name: ParseAnim
' Desc: Called by ParseAnimSet
'-----------------------------------------------------------------------------

Friend Sub ParseAnim(FileData As DirectXFileData, parentAnimation As CD3DAnimation, parentFrame As CD3DFrame)
    On Local Error Resume Next
    ObjectName = FileData.GetName()
    
    Dim dataSize As Long
    Dim KeyHead As KEYHEADER
    Dim size As Long
    Dim newFrame As CD3DFrame
    Dim ChildObj As DirectXFileObject
    Dim ChildData As DirectXFileData
    Dim ChildReference As DirectXFileReference
    Dim strChunkType As String
    Dim i As Long
    
    Set ChildObj = FileData.GetNextObject()
    
    Do While Not ChildObj Is Nothing
    
        Set ChildReference = ChildObj
        If Err.Number = 0 Then
        
            Set ChildData = ChildReference.Resolve()
                    
            
            If ChildData.GetType = "TID_D3DRMFrame" Then
                m_strFrameName = ChildData.GetName()
                Set m_frame = parentFrame.FindChildObject(m_strFrameName, 0)
            End If
            
            Set ChildReference = Nothing
        End If
        Err.Clear
        
        Set ChildData = ChildObj
        If Err.Number = 0 Then
        
            strChunkType = ChildData.GetType
            Select Case strChunkType
                Case "TID_D3DRMFrame"
                    Set newFrame = New CD3DFrame
                    newFrame.InitFromXOF g_dev, ChildData, parentFrame
                    Set newFrame = Nothing
                
                Case "TID_D3DRMAnimationOptions"
                    
                Case "TID_D3DRMAnimationKey"
                    dataSize = ChildData.GetDataSize("")
                    ChildData.GetDataFromOffset "", 0, 8, KeyHead

                    Select Case KeyHead.keytype
                        Case 0      'ROTATEKEY
                            ReDim m_RMRotateKeys(KeyHead.keycount)
                            ReDim m_RotateKeys(KeyHead.keycount)
                            size = Len(m_RMRotateKeys(0)) * KeyHead.keycount
                            ChildData.GetDataFromOffset "", 8, size, m_RMRotateKeys(0)
                            m_NumRotateKeys = KeyHead.keycount
                            
                            'NOTE x files are w x y z and QUATERNIONS are x y z w
                            'so we loop through on load and copy the values
                            For i = 0 To m_NumRotateKeys - 1
                                With m_RotateKeys(i)
                                    .time = m_RMRotateKeys(i).time
                                    If g_InvertRotateKey Then
                                        .quat.w = -m_RMRotateKeys(i).w
                                    Else
                                        .quat.w = m_RMRotateKeys(i).w
                                    End If
                                    .quat.x = m_RMRotateKeys(i).x
                                    .quat.y = m_RMRotateKeys(i).y
                                    .quat.z = m_RMRotateKeys(i).z
                                End With
                            Next
                            ReDim m_RMRotateKeys(0)
                            
                        Case 1      'SCALE KEY
                            ReDim m_ScaleKeys(KeyHead.keycount)
                            size = Len(m_ScaleKeys(0)) * KeyHead.keycount
                            ChildData.GetDataFromOffset "", 8, size, m_ScaleKeys(0)
                            m_NumScaleKeys = KeyHead.keycount
                
                        Case 2      'POSITION KEY
                            ReDim m_PositionKeys(KeyHead.keycount)
                            size = Len(m_PositionKeys(0)) * KeyHead.keycount
                            ChildData.GetDataFromOffset "", 8, size, m_PositionKeys(0)
                            m_NumPositionKeys = KeyHead.keycount
                            
                        Case 4      'MATRIX KEY
                            ReDim m_MatrixKeys(KeyHead.keycount)
                            size = Len(m_MatrixKeys(0)) * KeyHead.keycount
                            ChildData.GetDataFromOffset "", 8, size, m_MatrixKeys(0)
                            m_NumMatrixKeys = KeyHead.keycount
                            
                End Select

                    
            End Select
        End If
                    
        Set ChildData = Nothing
        Set ChildReference = Nothing
        
        Set ChildObj = FileData.GetNextObject()
    Loop
          


End Sub



'-----------------------------------------------------------------------------
' Name: ComputeP1234
' Desc: Aux function to compute 4 nearest keys
'-----------------------------------------------------------------------------
Private Sub ComputeP1234(j As Long, maxNum As Long, ByRef p1 As Long, ByRef p2 As Long, ByRef p3 As Long, ByRef p4 As Long)

        p1 = j: p2 = j: p3 = j: p4 = j
            
            If j > 0 Then
                p1 = j - 2: p2 = j - 1
            End If
            If j = 1 Then
                p1 = j - 1: p2 = j - 1
            End If
            If j < (maxNum) - 1 Then p4 = j + 1
End Sub
        
        
'-----------------------------------------------------------------------------
' Name: SetTime
' Desc: Sets the matrix of the frame being animated
'-----------------------------------------------------------------------------
Public Sub SetTime(t As Single)
    Dim t2 As Single
    Dim i As Long, j As Long
    Dim p1 As Long, p2 As Long, p3 As Long, p4 As Long
    Dim f1 As Single, f2 As Single, f3 As Single, f4 As Single
    Dim rM As D3DMATRIX, rQuat As D3DQUATERNION, rPos As D3DVECTOR, rScale As D3DVECTOR

    Dim a As D3DVECTOR, b As D3DVECTOR
    Dim q1 As D3DQUATERNION, q2 As D3DQUATERNION
    Dim s As Single
    
    Dim child As CD3DAnimation
    Dim LastT As Single
    
    'Check children
    For i = 1 To m_NumChildren
        Set child = m_Children(i - 1)
        If Not child Is Nothing Then
            child.SetTime t
        End If
        Set child = Nothing
    Next
        
    If m_frame Is Nothing Then Exit Sub
    
    'set components to identity  incase we dont have any keys.
    D3DXMatrixIdentity rM
    rScale = vec3(1, 1, 1)
    
    D3DXQuaternionIdentity rQuat
        
    
    t2 = t
    
    'loop matrix keys
    If m_NumMatrixKeys > 0 Then
        t2 = t
        LastT = m_MatrixKeys(m_NumMatrixKeys - 1).time
        If t > LastT Then
            i = t \ LastT
            t2 = t - i * LastT
        Else
            
        End If
       
       
        'optimizations
        Dim tAt As Single, tNext1 As Single, tNext2 As Single
        
        If m_iMatrixKey < m_NumMatrixKeys - 2 Then
            tAt = m_MatrixKeys(m_iMatrixKey).time
            tNext1 = m_MatrixKeys(m_iMatrixKey + 1).time
            tNext2 = m_MatrixKeys(m_iMatrixKey + 2).time
            If tAt < t2 And t2 <= tNext1 Then Exit Sub
               
            If tNext1 < t2 And t2 <= tNext2 Then
               m_iMatrixKey = m_iMatrixKey + 1
               If m_iMatrixKey > m_NumMatrixKeys Then m_iMatrixKey = 0
               m_frame.SetMatrix m_MatrixKeys(m_iMatrixKey).matrix
            End If

        End If
        
        'linear search
        For i = 1 To m_NumMatrixKeys
            If m_MatrixKeys(i - 1).time > t2 Then
                m_frame.SetMatrix m_MatrixKeys(i - 1).matrix
                m_iMatrixKey = i - 1
           
                Exit Sub
            End If
        Next
        
     End If
    
    '.................
    
    
    'loop position keys
    If m_NumPositionKeys > 0 Then
        t2 = t
        LastT = m_PositionKeys(m_NumPositionKeys - 1).time
        If t > LastT Then
            i = t \ LastT
            t2 = t - i * LastT
        End If
    End If
    
    'Check Position Keys
    For i = 1 To m_NumPositionKeys
        j = i - 1
        
        If m_PositionKeys(j).time > t2 Then
                
            ComputeP1234 j, m_NumPositionKeys, p1, p2, p3, p4
            f1 = m_PositionKeys(p1).time
            f2 = m_PositionKeys(p2).time
            f3 = m_PositionKeys(p3).time
            f4 = m_PositionKeys(p4).time
                            
                
            If ((f3 - f2) = 0) Then
                s = 1
            Else
                s = (t2 - f2) / (f3 - f2)
            End If
    
            a = m_PositionKeys(p2).vec
            b = m_PositionKeys(p3).vec
            
            
            D3DXVec3Lerp rPos, a, b, s
            Exit For
        End If
    Next
        
        
        
    'loop scale keys
    If m_NumScaleKeys > 0 Then
        t2 = t
        LastT = m_ScaleKeys(m_NumScaleKeys - 1).time
        If t > LastT Then
            i = t \ LastT
            t2 = t - i * LastT
        End If
    End If
        
        
    'Check Scale Keys
    For i = 1 To m_NumScaleKeys
        j = i - 1
        If m_ScaleKeys(j).time > t2 Then
                
            ComputeP1234 j, m_NumScaleKeys, p1, p2, p3, p4
            f1 = m_ScaleKeys(p1).time
            f2 = m_ScaleKeys(p2).time
            f3 = m_ScaleKeys(p3).time
            f4 = m_ScaleKeys(p4).time
                
            If ((f3 - f2) = 0) Then
                s = 1
            Else
                s = (t2 - f2) / (f3 - f2)
            End If
    
            a = m_ScaleKeys(p2).vec
            b = m_ScaleKeys(p3).vec
            
            
            D3DXVec3Lerp rScale, a, b, s
            Exit For
        End If
    Next
        

    'loop rotate keys
    If m_NumRotateKeys > 0 Then
        t2 = t
        LastT = m_RotateKeys(m_NumRotateKeys - 1).time
        If t > LastT Then
            i = t \ LastT
            t2 = t - i * LastT
        End If
    End If
                
    'Check Rotate Keys
    For i = 1 To m_NumRotateKeys
        j = i - 1
            
        If m_RotateKeys(j).time > t2 Then
            
            
            
            ComputeP1234 j, m_NumRotateKeys, p1, p2, p3, p4
            f1 = m_RotateKeys(p1).time
            f2 = m_RotateKeys(p2).time
            f3 = m_RotateKeys(p3).time
            f4 = m_RotateKeys(p4).time
                
            If ((f3 - f2) = 0) Then
                s = 1
            Else
                s = (t2 - f2) / (f3 - f2)
            End If
    
            q1 = m_RotateKeys(p2).quat
            q2 = m_RotateKeys(p3).quat
            
            D3DXQuaternionSlerp rQuat, q1, q2, s
            Exit For
        End If
    Next
        
    Dim temp1 As D3DMATRIX
    Dim temp2 As D3DMATRIX
    Dim temp3 As D3DMATRIX
    
    D3DXMatrixScaling temp1, rScale.x, rScale.y, rScale.z
    D3DXMatrixRotationQuaternion temp2, rQuat
    D3DXMatrixTranslation temp3, rPos.x, rPos.y, rPos.z
    D3DXMatrixMultiply rM, temp1, temp2
    D3DXMatrixMultiply rM, rM, temp3
        
    m_frame.SetMatrix rM
    
End Sub


'-----------------------------------------------------------------------------
' Name: AddRotateKey
' Desc:
'-----------------------------------------------------------------------------

Sub AddRotateKey(t As Long, quat As D3DQUATERNION)

    ReDim Preserve m_RotateKeys(m_NumRotateKeys)
                   
    With m_RotateKeys(m_NumRotateKeys)
        .time = t
        .quat = quat
    End With
    m_NumRotateKeys = m_NumRotateKeys + 1
End Sub

'-----------------------------------------------------------------------------
' Name: AddScaleKey
' Desc:
'-----------------------------------------------------------------------------
                            
Sub AddScaleKey(t As Long, scalevec As D3DVECTOR)

    ReDim Preserve m_ScaleKeys(m_NumScaleKeys)
    
    
                   
    With m_ScaleKeys(m_NumScaleKeys)
        .time = t
        .vec = scalevec
    End With
    
    m_NumScaleKeys = m_NumScaleKeys + 1
End Sub
                        
'-----------------------------------------------------------------------------
' Name: AddPositionKey
' Desc:
'-----------------------------------------------------------------------------
Sub AddPositionKey(t As Long, posvec As D3DVECTOR)

    ReDim Preserve m_PositionKeys(m_NumPositionKeys)
    
   
                   
    With m_PositionKeys(m_NumPositionKeys)
        .time = t
        .vec = posvec
    End With
    m_NumPositionKeys = m_NumPositionKeys + 1
End Sub



