VERSION 5.00
Begin VB.UserControl SubClasser 
   CanGetFocus     =   0   'False
   ClientHeight    =   495
   ClientLeft      =   0
   ClientTop       =   0
   ClientWidth     =   510
   HasDC           =   0   'False
   InvisibleAtRuntime=   -1  'True
   Picture         =   "SubClasser.ctx":0000
   ScaleHeight     =   495
   ScaleWidth      =   510
   ToolboxBitmap   =   "SubClasser.ctx":0442
End
Attribute VB_Name = "SubClasser"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = True
Option Explicit
Private mlHwnd As Long
Private mfNeedUnhook As Boolean

Public Event WindowsMessage(ByVal uMsg As Long, ByVal wParam As Long, ByVal lParam As Long)

Public Sub Hook(ByVal hWnd As Long)
    mlHwnd = hWnd
    modWndProc.Hook mlHwnd, Me
    mfNeedUnhook = True
End Sub

Public Sub UnHook()
    modWndProc.UnHook mlHwnd
    mfNeedUnhook = False
End Sub

Friend Sub GotMessage(ByVal uMsg As Long, ByVal wParam As Long, ByVal lParam As Long)
    RaiseEvent WindowsMessage(uMsg, wParam, lParam)
End Sub

Private Sub UserControl_Terminate()
    If mfNeedUnhook Then UnHook
End Sub
