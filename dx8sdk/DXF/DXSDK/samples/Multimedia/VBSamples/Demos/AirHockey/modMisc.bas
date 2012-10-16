Attribute VB_Name = "modMisc"
Option Explicit

Public Function aCos(dCos As Double) As Double
    On Error Resume Next 'Assume any errors mean the aCos = 0
    'Here we will figure out the arccosine..
    aCos = Atn(Sqr(1 - (dCos * dCos)) / dCos)
End Function

Public Sub PrintVector(vec As D3DVECTOR, Optional ByVal s As String = vbNullString)
    With vec
        Debug.Print s; " X="; .x; " Y="; .y; " Z="; .z
    End With
End Sub

