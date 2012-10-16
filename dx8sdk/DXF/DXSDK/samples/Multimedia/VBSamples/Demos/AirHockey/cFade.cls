VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "cFade"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit

'This is an 'effects' class, that will fade a scene in or out
'by increasing the alpha channel on all objects
Public FadeInterval As Single
Public AmFading As Boolean
Public CanFade As Boolean
Private mlPixelShaderHandle As Long

'Methods used during fading

'This will update the global params for fading the scene in, or out
'Fading is simply accomplished by adding or subtracting the amount of light in the scene until
'it reaches a desired level.  Since the background is black anyway, we could have also
'simply slowly turned up the alpha on each of the objects, this is just the way I chose.
Public Sub Fade(ByVal nInterval As Long)
    If Not CanFade Then Exit Sub
    FadeInterval = nInterval
    AmFading = True
End Sub

Public Sub UpdateFade(oPuck As cPuck, oPaddle() As cPaddle, oTable As cTable, oRoom As cRoom)
    Dim fDoneFading As Boolean
    fDoneFading = True
    fDoneFading = oPuck.FadeMesh(FadeInterval) And oPaddle(0).FadeMesh(FadeInterval) And oPaddle(1).FadeMesh(FadeInterval) And oTable.FadeMesh(FadeInterval) And oRoom.FadeMesh(FadeInterval)
    AmFading = Not fDoneFading
End Sub

Private Sub Class_Initialize()
    'By default we will allow fading
    CanFade = True
End Sub

