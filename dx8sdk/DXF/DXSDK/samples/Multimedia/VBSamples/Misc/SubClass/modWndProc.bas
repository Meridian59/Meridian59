Attribute VB_Name = "modWndProc"
Option Explicit

Public Declare Function CallWindowProc Lib "user32" Alias "CallWindowProcA" (ByVal lpPrevWndFunc As Long, ByVal hWnd As Long, ByVal Msg As Long, ByVal wParam As Long, ByVal lParam As Long) As Long
Public Declare Function SetWindowLong Lib "user32" Alias "SetWindowLongA" (ByVal hWnd As Long, ByVal nIndex As Long, ByVal dwNewLong As Long) As Long

Public Const GWL_WNDPROC = -4

Public lpPrevWndProc As Long
Private moControl As SubClasser

Public Sub Hook(ByVal lHwnd As Long, oCon As SubClasser)
    lpPrevWndProc = SetWindowLong(lHwnd, GWL_WNDPROC, AddressOf WindowProc)
    Set moControl = oCon
End Sub

Public Sub UnHook(ByVal lHwnd As Long)
    Dim lngReturnValue As Long
    lngReturnValue = SetWindowLong(lHwnd, GWL_WNDPROC, lpPrevWndProc)
End Sub

Function WindowProc(ByVal hw As Long, ByVal uMsg As Long, ByVal wParam As Long, ByVal lParam As Long) As Long
    moControl.GotMessage uMsg, wParam, lParam
    WindowProc = CallWindowProc(lpPrevWndProc, hw, uMsg, wParam, lParam)
End Function


