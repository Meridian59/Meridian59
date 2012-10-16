VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "cText"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Option Explicit
'Helper class to encapsulate text drawing

'Here we will keep the font information and the calls to draw the text
Private moD3DXFont As D3DXFont

Public Sub InitText(d3dx As D3DX8, dev As Direct3DDevice8, ByVal sFontName As String, lSize As Long, fBold As Boolean)
    Dim oMyFont As IFont
    
    Set oMyFont = New StdFont
    oMyFont.Name = "Times New Roman"
    oMyFont.size = 8
    oMyFont.Bold = True
    
    Set moD3DXFont = d3dx.CreateFont(dev, oMyFont.hFont)

End Sub

Public Sub BeginText()
    moD3DXFont.Begin
End Sub

Public Sub EndText()
    moD3DXFont.End
End Sub

Public Sub DrawText(ByVal sText As String, X As Long, Y As Long, lColor As Long)
    Dim rcText As RECT
    
    'X and Y are in screen coords
    rcText.Left = X
    rcText.Top = Y
    'actually draw the text now, telling d3dx to build the rectangle based on the text and the x,y coord
    moD3DXFont.DrawTextW sText, -1, rcText, 0, lColor
End Sub

Private Sub Class_Initialize()
    Set moD3DXFont = Nothing
End Sub

Private Sub Class_Terminate()
    Set moD3DXFont = Nothing
End Sub
