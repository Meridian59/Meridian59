Attribute VB_Name = "modAudio"
Option Explicit

'We will keep our Audio vars here
Dim dmPerf As DirectMusicPerformance8
Dim dmLoad As DirectMusicLoader8
Dim dmSegBank() As DirectMusicSegment8
Dim dmSegHit() As DirectMusicSegment8
Dim dmScore As DirectMusicSegment8

Public glNumBankSounds As Long
Public glNumHitSounds As Long

Public Sub SetNumberSounds()
    Dim sFile As String
    Dim lCount As Long
    
    lCount = 1
    sFile = Dir$(App.path & "\sounds\bank" & format$(CStr(lCount), "00") & ".wav")
    Do While sFile <> vbNullString
        lCount = lCount + 1
        sFile = Dir$(App.path & "\sounds\bank" & format$(CStr(lCount), "00") & ".wav")
    Loop
    glNumBankSounds = lCount - 1
    lCount = 1
    sFile = Dir$(App.path & "\sounds\hit" & format$(CStr(lCount), "00") & ".wav")
    Do While sFile <> vbNullString
        lCount = lCount + 1
        sFile = Dir$(App.path & "\sounds\hit" & format$(CStr(lCount), "00") & ".wav")
    Loop
    glNumHitSounds = lCount - 1
End Sub

Public Function InitAudio() As Boolean
    
    Dim lCount As Long, dma As DMUS_AUDIOPARAMS
    
    On Error GoTo FailedInit
    InitAudio = True
    'Create our objects
    Set dmPerf = dx.DirectMusicPerformanceCreate
    Set dmLoad = dx.DirectMusicLoaderCreate
    'Get the total number of sounds we have for each type of sound
    SetNumberSounds
    
    'Using that information create an array of segments
    ReDim dmSegBank(1 To glNumBankSounds)
    ReDim dmSegHit(1 To glNumHitSounds)
    
    'Create a default audio path
    dmPerf.InitAudio frmAir.hwnd, DMUS_AUDIOF_ALL, dma, , DMUS_APATH_SHARED_STEREOPLUSREVERB, 128
    'Load each of the sounds
    For lCount = 1 To glNumBankSounds
        Set dmSegBank(lCount) = dmLoad.LoadSegment(App.path & "\sounds\bank" & format$(CStr(lCount), "00") & ".wav")
        dmSegBank(lCount).Download dmPerf.GetDefaultAudioPath
    Next
    For lCount = 1 To glNumHitSounds
        Set dmSegHit(lCount) = dmLoad.LoadSegment(App.path & "\sounds\hit" & format$(CStr(lCount), "00") & ".wav")
        dmSegHit(lCount).Download dmPerf.GetDefaultAudioPath
    Next
    Set dmScore = dmLoad.LoadSegment(App.path & "\sounds\score.wav")
    dmScore.Download dmPerf.GetDefaultAudioPath
    Exit Function

FailedInit:
    InitAudio = False
End Function

Public Sub PlayRandomBankSound()
    Dim lRnd As Long
    
    'Pick a valid 'Bank' sound randomly and play it
    Randomize
    lRnd = CLng(Rnd * glNumBankSounds) + 1
    Do While lRnd < 1 Or lRnd > glNumBankSounds
        lRnd = CLng(Rnd * glNumBankSounds) + 1
    Loop
    dmPerf.PlaySegmentEx dmSegBank(lRnd), DMUS_SEGF_SECONDARY, 0
End Sub

Public Sub PlayRandomHitSound()
    Dim lRnd As Long
    
    'Pick a valid 'Hit' sound randomly and play it
    Randomize
    lRnd = CLng(Rnd * glNumHitSounds) + 1
    Do While lRnd < 1 Or lRnd > glNumHitSounds
        lRnd = CLng(Rnd * glNumHitSounds) + 1
    Loop
    dmPerf.PlaySegmentEx dmSegHit(lRnd), DMUS_SEGF_SECONDARY, 0
End Sub

Public Sub PlayScoreSound()
    'Play the sound that happens when we score
    dmPerf.PlaySegmentEx dmScore, DMUS_SEGF_SECONDARY, 0
End Sub

Public Sub CleanupAudio()

    On Error Resume Next
    Dim lCount As Long
    
    'Unload all of our sounds off of the audio path and destroy them
    For lCount = 1 To glNumBankSounds
        dmSegBank(lCount).Unload dmPerf.GetDefaultAudioPath
        Set dmSegBank(lCount) = Nothing
    Next
    For lCount = 1 To glNumHitSounds
        dmSegHit(lCount).Unload dmPerf.GetDefaultAudioPath
        Set dmSegHit(lCount) = Nothing
    Next
    If Not (dmScore Is Nothing) Then dmScore.Unload dmPerf.GetDefaultAudioPath
    Set dmScore = Nothing
    
    'Destroy the rest of the objects
    Set dmPerf = Nothing
    Set dmLoad = Nothing
End Sub
