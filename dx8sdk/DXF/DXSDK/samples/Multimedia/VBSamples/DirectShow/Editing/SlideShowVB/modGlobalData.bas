Attribute VB_Name = "modGlobalData"
'*******************************************************************************
'*       This is a part of the Microsoft DXSDK Code Samples.
'*       Copyright (C) 1999-2001 Microsoft Corporation.
'*       All rights reserved.
'*       This source code is only intended as a supplement to
'*       Microsoft Development Tools and/or SDK documentation.
'*       See these sources for detailed information regarding the
'*       Microsoft samples programs.
'*******************************************************************************
Option Explicit
Option Base 0
Option Compare Text


' **************************************************************************************************************************************
' * GLOBAL INTERFACE- DATA
' *
' *
            Global gbl_objMediaControl As IMediaControl 'playback control
            Global gbl_objTimeline As AMTimeline  'application timeline object
            Global gbl_objRenderEngine As RenderEngine  'application render engine
            Global gbl_objVideoWindow As IVideoWindow 'application video window for playback
            


' **************************************************************************************************************************************
' * GLOBAL INTERFACE- CONSTANTS
' *
' *
            Global Const SLIDESHOWVB_CLIPLENGTH As Double = 4#  'how long each clip lasts on the timeline
            Global Const SLIDESHOWVB_VIDEOTYPE As String = "{73646976-0000-0010-8000-00AA00389B71}"
            Global Const SLIDESHOWVB_AUDIOTYPE As String = "{73647561-0000-0010-8000-00AA00389B71}"


' **************************************************************************************************************************************
' * GLOBAL INTERFACE- APPLICATION ENTRY POINT
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: Main
            ' * procedure description:  Application Entry Point.
            ' *
            ' ******************************************************************************************************************************
            Public Sub Main()
            On Local Error GoTo ErrLine
            
            Load frmMain
            frmMain.Move 0, 0
            frmMain.Visible = True
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
