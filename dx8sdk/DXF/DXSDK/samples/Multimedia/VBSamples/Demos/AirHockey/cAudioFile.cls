VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "cAudioFile"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

'Here we will control a 'set' of audio files
Private mlNumSounds As Long
Private dmSegments() As DirectMusicSegment8
Private moPath As DirectMusicAudioPath8

Public Sub InitSounds(dmPerf As DirectMusicPerformance8, dmLoader As DirectMusicLoader8, ByVal sPath As String, ByVal sFirstPart As String, ByVal sExtenstion As String, Optional fSingleFile As Boolean = False)
    Dim sFile As String
    Dim lCount As Long
    
    'Here we will take a 'group' of files (that group could only be 1 file)
    'and load them into our array
    Set moPath = dmPerf.GetDefaultAudioPath
    lCount = 1
    If fSingleFile Then
        sFile = Dir$(sPath & sFirstPart & sExtenstion)
    Else
        sFile = Dir$(sPath & sFirstPart & format$(CStr(lCount), "00") & sExtenstion)
    End If
    Do While sFile <> vbNullString
        ReDim Preserve dmSegments(1 To lCount)
        Set dmSegments(lCount) = dmLoader.LoadSegment(sPath & sFile)
        dmSegments(lCount).Download moPath
        lCount = lCount + 1
        If fSingleFile Then
            sFile = vbNullString
        Else
            sFile = Dir$
        End If
    Loop
    mlNumSounds = lCount - 1
End Sub

Public Sub Play(dmPerf As DirectMusicPerformance8)
    Dim lRnd As Long
    
    'Pick a valid sound randomly and play it
    Randomize
    lRnd = CLng(Rnd * mlNumSounds) + 1
    Do While lRnd < 1 Or lRnd > mlNumSounds
        lRnd = CLng(Rnd * mlNumSounds) + 1
    Loop
    dmPerf.PlaySegmentEx dmSegments(lRnd), DMUS_SEGF_SECONDARY, 0

End Sub

Private Sub Class_Initialize()
    'This should already have happened for us from VB, but just in case
    Erase dmSegments
    mlNumSounds = 0
End Sub

Private Sub Class_Terminate()
    Dim lCount As Long
    'Let's clean everything up
    For lCount = 1 To mlNumSounds
        'Unload and release all the segments
        If Not (dmSegments(lCount) Is Nothing) Then
            dmSegments(lCount).Unload moPath
            Set dmSegments(lCount) = Nothing
        End If
    Next
    'Clear up any data left over
    Erase dmSegments
End Sub
