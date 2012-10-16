VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "cAudio"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

'We will keep our Audio vars here
Private dmPerf As DirectMusicPerformance8
Private dmLoad As DirectMusicLoader8
Private dmMusic As DirectMusicSegment8
Private dmMusicPath As DirectMusicAudioPath8

Private dmSegBank As cAudioFile
Private dmSegHit As cAudioFile
Private dmScore As cAudioFile

Public PlaySounds As Boolean
Public PlayMusic As Boolean

Private mlSoundVolume As Long
Private mlMusicVolume As Long

Private Sub InitializeBackgroundMusic(ByVal sPath As String)
    If dmMusicPath Is Nothing Then 'We haven't created our path yet
        Set dmMusicPath = dmPerf.CreateStandardAudioPath(DMUS_APATH_SHARED_STEREOPLUSREVERB, 128, True)
    End If
    If Not (dmMusic Is Nothing) Then
        dmMusic.Unload dmMusicPath
        Set dmMusic = Nothing
    End If
    Set dmMusic = dmLoad.LoadSegment(sPath)
    dmMusic.Download dmMusicPath
    dmMusic.SetStandardMidiFile
End Sub

Public Sub StartBackgroundMusic()
    If Not PlayMusic Then Exit Sub
    If Not (dmMusic Is Nothing) Then
        'Keep repeating over and over again
        dmMusic.SetRepeats INFINITE
        dmPerf.PlaySegmentEx dmMusic, DMUS_SEGF_DEFAULT, 0, dmMusicPath, dmMusicPath
    End If
End Sub

Public Sub StopBackgroundMusic()
    If Not (dmMusic Is Nothing) Then
        'Lets just stop
        dmPerf.StopEx dmMusic, 0, 0
    End If
End Sub

Public Sub PlayBankSound()
    If Not PlaySounds Then Exit Sub
    'Play the sound that happens when the puck hits the side wall
    dmSegBank.Play dmPerf
End Sub

Public Sub PlayHitSound()
    If Not PlaySounds Then Exit Sub
    'Play the sound that happens when a paddle hits the puck
    dmSegHit.Play dmPerf
End Sub

Public Sub PlayScoreSound()
    If Not PlaySounds Then Exit Sub
    'Play the sound that happens when we score
    dmScore.Play dmPerf
End Sub

Public Property Let MusicVolume(ByVal lVol As Long)
    mlMusicVolume = lVol
    'Actually set the volume
    If Not (dmMusicPath Is Nothing) Then dmMusicPath.SetVolume lVol, 0
End Property

Public Property Get MusicVolume() As Long
    MusicVolume = mlMusicVolume
End Property

Public Property Let SoundVolume(ByVal lVol As Long)
    mlSoundVolume = lVol
    'Actually set the volume
    If Not (dmPerf Is Nothing) Then
        If Not (dmPerf.GetDefaultAudioPath Is Nothing) Then dmPerf.GetDefaultAudioPath.SetVolume lVol, 0
    End If
End Property

Public Property Get SoundVolume() As Long
    SoundVolume = mlSoundVolume
End Property

Public Function InitAudio() As Boolean
    
    Dim lCount As Long, dma As DMUS_AUDIOPARAMS
    
    InitAudio = True
    On Error GoTo FailedInit
    'Create our objects
    Set dmPerf = dx.DirectMusicPerformanceCreate
    Set dmLoad = dx.DirectMusicLoaderCreate
    
    'Create a default audio path
    dmPerf.InitAudio frmAir.hwnd, DMUS_AUDIOF_ALL, dma, , DMUS_APATH_SHARED_STEREOPLUSREVERB, 128
    
    'Create the sound objects
    Set dmSegBank = New cAudioFile
    Set dmSegHit = New cAudioFile
    Set dmScore = New cAudioFile
    'Load each of the sounds
    dmSegBank.InitSounds dmPerf, dmLoad, App.path & "\sounds\", "bank", ".wav"
    dmSegHit.InitSounds dmPerf, dmLoad, App.path & "\sounds\", "hit", ".wav"
    dmScore.InitSounds dmPerf, dmLoad, App.path & "\sounds\", "score", ".wav", True
    
    InitializeBackgroundMusic App.path & "\sounds\music.mid"
    'Init the volume
    SoundVolume = mlSoundVolume
    MusicVolume = mlMusicVolume
    Exit Function

FailedInit:
    InitAudio = False
End Function

Private Sub Class_Initialize()
    PlaySounds = True
    Set dmSegBank = Nothing
    Set dmSegHit = Nothing
    Set dmScore = Nothing
    Set dmMusic = Nothing
    Set dmPerf = Nothing
    Set dmLoad = Nothing
End Sub

Private Sub Class_Terminate()
    'On Error Resume Next
    'Unload all of our sounds off of the audio path and destroy them
    StopBackgroundMusic
    Set dmSegBank = Nothing
    Set dmSegHit = Nothing
    Set dmScore = Nothing
    If Not (dmMusic Is Nothing) Then
        dmMusic.Unload dmMusicPath
        Set dmMusic = Nothing
    End If
    
    Set dmMusicPath = Nothing
    If Not (dmPerf Is Nothing) Then
        'Closedown
        dmPerf.CloseDown
    End If
    'Destroy the rest of the objects
    Set dmPerf = Nothing
    Set dmLoad = Nothing
End Sub
