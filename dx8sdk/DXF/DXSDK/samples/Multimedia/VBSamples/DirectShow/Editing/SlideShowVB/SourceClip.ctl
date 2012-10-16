VERSION 5.00
Begin VB.UserControl SourceClip 
   BackColor       =   &H00FF0000&
   BorderStyle     =   1  'Fixed Single
   ClientHeight    =   2100
   ClientLeft      =   0
   ClientTop       =   0
   ClientWidth     =   1860
   EditAtDesignTime=   -1  'True
   KeyPreview      =   -1  'True
   LockControls    =   -1  'True
   OLEDropMode     =   1  'Manual
   ScaleHeight     =   2100
   ScaleWidth      =   1860
   ToolboxBitmap   =   "SourceClip.ctx":0000
   Begin VB.Frame fraFixture 
      BackColor       =   &H00000000&
      BorderStyle     =   0  'None
      Height          =   2115
      Left            =   0
      OLEDropMode     =   1  'Manual
      TabIndex        =   0
      Top             =   0
      Width           =   1815
      Begin VB.Label lblClipName 
         Alignment       =   2  'Center
         Appearance      =   0  'Flat
         BackColor       =   &H80000005&
         BackStyle       =   0  'Transparent
         ForeColor       =   &H00FFFFFF&
         Height          =   255
         Left            =   0
         OLEDropMode     =   1  'Manual
         TabIndex        =   1
         Top             =   1800
         Width           =   1815
      End
      Begin VB.Image imgSourceClip 
         Appearance      =   0  'Flat
         BorderStyle     =   1  'Fixed Single
         Height          =   1695
         Left            =   0
         OLEDropMode     =   1  'Manual
         Picture         =   "SourceClip.ctx":0312
         Stretch         =   -1  'True
         Top             =   0
         Width           =   1815
      End
   End
End
Attribute VB_Name = "SourceClip"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
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

Private m_bstrFilter As String
Private m_nBorderStyle As Long
Private m_nBorderColor As Long
Private m_bstrMediaFile As String
Private m_sngBorderSize As Single
Private m_boolShowMediaInfo As Boolean

'default application value(s)
Private Const APP_SEPTUM_SIZE As Single = 60 'default septum size; in twips
Private Const APP_DIVISIONAL_PERCENTAGE As Single = 0.75 'default divisional; in percent

'default design-time property value(s)
Private Const DEFAULT_BORDERSIZE As Single = 5 'default border size, in pixels
Private Const DEFAULT_BORDERSTYLE As Long = 1 'default border style
Private Const DEFAULT_BORDERCOLOR As Long = vbBlack 'default border color, vbBlack
Private Const DEFAULT_SHOWMEDIAINFO As Boolean = True 'default show info pane
Private Const DEFAULT_MEDIAFILE As String = vbNullString 'default media file path/name
Private Const DEFAULT_FILTER As String = ".avi;.mov;.mpg;.mpeg;.bmp;.jpg;.jpeg;.gif" 'default supported video media files


' **************************************************************************************************************************************
' * PUBLIC INTERFACE- EVENTS
' *
' *
           Public Event Import(bstrFileName As String, Cancel As Boolean)
Attribute Import.VB_Description = "Occurs when media is imported into the control by a user.  Set 'Cancel' to true to inhibit the operation."


' **************************************************************************************************************************************
' * PUBLIC INTERFACE- CONTROL ENUMERATIONS
' *
' *
            Public Enum SRCClipBorderStyleConstants
            None = 0
            FixedSingle = 1
            End Enum


' **************************************************************************************************************************************
' * PUBLIC INTERFACE- CONTROL PROPERTIES
' *
' *

            ' ******************************************************************************************************************************
            ' * procedure name: BorderColor
            ' * procedure description:  Returns either the elected or default border color.
            ' *
            ' ******************************************************************************************************************************
            Public Property Get BorderColor() As OLE_COLOR
Attribute BorderColor.VB_Description = "Returns or assigns the controls border color."
Attribute BorderColor.VB_ProcData.VB_Invoke_Property = ";Appearance"
            On Local Error GoTo ErrLine
            
            'obtain from module-level
            BorderColor = m_nBorderColor
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: BorderColor
            ' * procedure description:  Allows the client to assign a color to the controls border.
            ' *
            ' ******************************************************************************************************************************
            Public Property Let BorderColor(RHS As OLE_COLOR)
            On Local Error GoTo ErrLine
            
            'assign to module-level
            m_nBorderColor = RHS
            
            'reset bordercolor
            If UserControl.BackColor <> RHS Then
               UserControl.BackColor = RHS
            End If
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: BorderStyle
            ' * procedure description:  Returns the style of the border around the control. Arguments are  0 - None or 1- Fixed Single
            ' *
            ' ******************************************************************************************************************************
            Public Property Get BorderStyle() As SRCClipBorderStyleConstants
Attribute BorderStyle.VB_Description = "Returns or assigns the style of the border around the control. Arguments are  0 - None or 1- Fixed Single"
Attribute BorderStyle.VB_ProcData.VB_Invoke_Property = ";Appearance"
            On Local Error GoTo ErrLine
            
            'obtain from module-level
            BorderStyle = m_nBorderStyle
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: BorderStyle
            ' * procedure description:  Assigns the style of the border around the control. Arguments are  0 - None or 1- Fixed Single
            ' *
            ' ******************************************************************************************************************************
            Public Property Let BorderStyle(RHS As SRCClipBorderStyleConstants)
            On Local Error GoTo ErrLine
            
            'assign to module-level
            m_nBorderStyle = RHS
            
            'update borderstyle of the component
            If RHS = None Then
               Me.BorderSize = 0
            End If
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: BorderSize
            ' * procedure description:  Returns the width of the controls border, in pixels.
            ' *
            ' ******************************************************************************************************************************
            Public Property Get BorderSize() As Single
Attribute BorderSize.VB_Description = "Returns or assigns the width of the controls border, in pixels."
Attribute BorderSize.VB_ProcData.VB_Invoke_Property = ";Appearance"
            On Local Error GoTo ErrLine
            
            'obtain from module-level
            BorderSize = m_sngBorderSize
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: BorderSize
            ' * procedure description:  Assigns the width of the controls border, in pixels.
            ' *
            ' ******************************************************************************************************************************
            Public Property Let BorderSize(RHS As Single)
            On Local Error GoTo ErrLine
            
            'assign to module-level; convert to pixels
            m_sngBorderSize = CLng((RHS))
            
            'if the bordersize is zero then reset the borderstyle to None
            If RHS = 0 Then
               Me.BorderStyle = None
               Call UserControl_Resize
            Else: Call UserControl_Resize
            End If
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Filter
            ' * procedure description:  Returns the semi colon delimited filter string for media MediaFile/export.
            ' *                                       Similar to the common dialog filter property.  Valid Filter String Example:  ".avi;.mpg;.bmp"
            ' ******************************************************************************************************************************
            Public Property Get Filter() As String
Attribute Filter.VB_Description = "Returns or assigns a semi colon delimited filter string for media MediaFile/export.  Similar to the common dialog filter property.  Valid Filter String Example:  "".avi;.mpg;.bmp"""
Attribute Filter.VB_ProcData.VB_Invoke_Property = ";Misc"
            On Local Error GoTo ErrLine
            
            'return the filter
            Filter = m_bstrFilter
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Filter
            ' * procedure description:  Assigns the semi colon delimited filter string for media MediaFile/export.
            ' *                                       Similar to the common dialog filter property.  Valid Filter String Example:  ".avi;.mpg;.bmp"
            ' ******************************************************************************************************************************
            Public Property Let Filter(RHS As String)
            On Local Error GoTo ErrLine
            
            'assign the filter
            m_bstrFilter = RHS
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: MediaFile
            ' * procedure description:  Assigns the given media file to the control and maps it to the control for preview.
            ' *
            ' ******************************************************************************************************************************
            Public Property Let MediaFile(RHS As String)
            Dim nStreams As Long
            Dim boolCancel As Boolean
            Dim objMediaDet As MediaDet
            On Local Error GoTo ErrLine
            
            'raiseevent
            RaiseEvent Import(RHS, boolCancel)
            If boolCancel = True Then Exit Property
            
            'assign to module-level
            m_bstrMediaFile = RHS
            
            If HasVideoStream(RHS) Then
               'the media has been verified as having at least (1) valid video stream
               'so obtain a bitmap of the first frame of the first file dragged on to the usercontrol
               'or any of it's contingent controls and proceed to write out the bitmap to a temporary
               'file in the temp directory.  From the temp file we can load the poster frame into the control.
               Set objMediaDet = New MediaDet
               objMediaDet.FileName = RHS
               Call objMediaDet.WriteBitmapBits(0, CLng(imgSourceClip.Width / Screen.TwipsPerPixelX), CLng(imgSourceClip.Height / Screen.TwipsPerPixelY), CStr(GetTempDirectory & App.EXEName & ".bmp"))
               'map the bitmap back to the temporary surface
               If Not LoadPicture(GetTempDirectory & App.EXEName & ".bmp") Is Nothing Then _
                  Set imgSourceClip.Picture = LoadPicture(GetTempDirectory & App.EXEName & ".bmp")
                  If InStrRev(RHS, "\") > 0 Then
                     lblClipName.Caption = Trim(LCase(Mid(RHS, InStrRev(RHS, "\") + 1, Len(RHS))))
                     lblClipName.ToolTipText = Trim(LCase(Mid(RHS, InStrRev(RHS, "\") + 1, Len(RHS))))
                     imgSourceClip.ToolTipText = Trim(LCase(Mid(RHS, InStrRev(RHS, "\") + 1, Len(RHS))))
                  Else
                     lblClipName.Caption = vbNullString
                     lblClipName.ToolTipText = vbNullString
                     imgSourceClip.ToolTipText = vbNullString
                  End If
            Else
                  imgSourceClip.Picture = LoadPicture(vbNullString) 'disregard the picture
                  If InStrRev(RHS, "\") > 0 Then
                     lblClipName.Caption = Trim(LCase(Mid(RHS, InStrRev(RHS, "\") + 1, Len(RHS))))
                     lblClipName.ToolTipText = Trim(LCase(Mid(RHS, InStrRev(RHS, "\") + 1, Len(RHS))))
                     imgSourceClip.ToolTipText = Trim(LCase(Mid(RHS, InStrRev(RHS, "\") + 1, Len(RHS))))
                  Else
                     lblClipName.Caption = vbNullString
                     lblClipName.ToolTipText = vbNullString
                     imgSourceClip.ToolTipText = vbNullString
                  End If
            End If
            
            'clean-up & dereference
            If Not objMediaDet Is Nothing Then Set objMediaDet = Nothing
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: MediaFile
            ' * procedure description:  Returns the assigned media file for the control.
            ' *
            ' ******************************************************************************************************************************
            Public Property Get MediaFile() As String
Attribute MediaFile.VB_Description = "Returns or assigns the given media file to the control and maps it to the control for preview."
Attribute MediaFile.VB_ProcData.VB_Invoke_Property = ";Misc"
            On Local Error GoTo ErrLine
            
            'return the media file
            MediaFile = m_bstrMediaFile
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: BorderColor
            ' * procedure description:  Returns a boolean indicating if the media info is displayed for the given clip.
            ' *
            ' ******************************************************************************************************************************
            Public Property Get ShowMediaInfo() As OLE_CANCELBOOL
Attribute ShowMediaInfo.VB_Description = "Returns or assigns a value indicating if the media info is displayed for the given clip."
Attribute ShowMediaInfo.VB_ProcData.VB_Invoke_Property = ";Appearance"
            On Local Error GoTo ErrLine
            
            'obtain from module-level
            ShowMediaInfo = m_boolShowMediaInfo
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: ShowMediaInfo
            ' * procedure description:  Assigns a boolean indicating if the media info is displayed for the given clip.
            ' *
            ' ******************************************************************************************************************************
            Public Property Let ShowMediaInfo(RHS As OLE_CANCELBOOL)
            On Local Error GoTo ErrLine
            
            'assign to module-level
            m_boolShowMediaInfo = RHS
            
            'resize component to reflect update
            lblClipName.Visible = RHS
            Call UserControl_Resize
            Exit Property
            
ErrLine:
            Err.Clear
            Exit Property
            End Property
            

' **************************************************************************************************************************************
' * PRIVATE INTERFACE- USER CONTROL EVENTS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_AmbientChanged
            ' * procedure description:  Occurs when an ambient value was changed by the container of a user control
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_AmbientChanged(PropertyName As String)
            On Local Error GoTo ErrLine
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_AsyncReadComplete
            ' * procedure description:  Occurs when all of the data is available as a result of the AsyncRead method.
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_AsyncReadComplete(AsyncProp As AsyncProperty)
            On Local Error GoTo ErrLine
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_AsyncReadProgress
            ' * procedure description:  Occurs when more data is available as a result of the AsyncReadProgress method.
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_AsyncReadProgress(AsyncProp As AsyncProperty)
            On Local Error GoTo ErrLine
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub


            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_Click
            ' * procedure description:   Occurs when the user presses and then releases a mouse button over an object.
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_Click()
            On Local Error GoTo ErrLine
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_DragDrop
            ' * procedure description:  Occurs when a drag-and-drop operation is completed.
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_DragDrop(Source As Control, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_DragOver
            ' * procedure description:   Occurs when a drag-and-drop operation is in progress.
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_DragOver(Source As Control, X As Single, Y As Single, State As Integer)
            On Local Error GoTo ErrLine
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_GotFocus
            ' * procedure description:  Occurs when an object receives the focus.
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_GotFocus()
            On Local Error GoTo ErrLine
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_Hide
            ' * procedure description:  Occurs when the control's Visible property changes to False.
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_Hide()
            On Local Error GoTo ErrLine
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_Initialize
            ' * procedure description:  Occurs when an application creates an instance of a Form, MDIForm, or class.
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_Initialize()
            On Local Error GoTo ErrLine
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_InitProperties
            ' * procedure description:  Occurs the first time a user control or user document is created.
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_InitProperties()
            On Local Error GoTo ErrLine
            
            'set public property values for design time
             If UserControl.Ambient.UserMode = False Then
               Me.BorderColor = DEFAULT_BORDERCOLOR
               Me.BorderSize = DEFAULT_BORDERSIZE
               Me.BorderStyle = DEFAULT_BORDERSTYLE
               Me.Filter = DEFAULT_FILTER
               Me.MediaFile = DEFAULT_MEDIAFILE
               Me.ShowMediaInfo = DEFAULT_SHOWMEDIAINFO
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            

            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_KeyDown
            ' * procedure description:  Occurs when the user presses a key while an object has the focus.
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_KeyDown(KeyCode As Integer, Shift As Integer)
            On Local Error GoTo ErrLine
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_LostFocus
            ' * procedure description:  Occurs when an object loses the focus.
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_LostFocus()
            On Local Error GoTo ErrLine
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_OLEDragDrop
            ' * procedure description:  Occurs when data is dropped onto the control via an OLE drag/drop operation,
            ' *                                       and OLEDropMode is set to manual.
            ' ******************************************************************************************************************************
            Private Sub UserControl_OLEDragDrop(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            Call AppOLEDragDrop(Data, Effect, Button, Shift, X, Y)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_OLEDragOver
            ' * procedure description:  Occurs when the mouse is moved over the control during an OLE drag/drop operation,
            ' *                                       if its OLEDropMode property is set to manual.
            ' ******************************************************************************************************************************
            Private Sub UserControl_OLEDragOver(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single, State As Integer)
            On Local Error GoTo ErrLine
            Call AppOLEDragOver(Data, Effect, Button, Shift, X, Y, State)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_Paint
            ' * procedure description:  Occurs when any part of a form or PictureBox control is moved, enlarged, or exposed.
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_Paint()
            On Local Error GoTo ErrLine
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_ReadProperties
            ' * procedure description:  Occurs when a user control or user document is asked to read its data from a file.
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_ReadProperties(PropBag As PropertyBag)
            On Local Error GoTo ErrLine
            
            'obtain design time value(s) from the property bag during run-time
            Me.MediaFile = PropBag.ReadProperty("MediaFile", DEFAULT_MEDIAFILE)
            Me.Filter = PropBag.ReadProperty("Filter", DEFAULT_FILTER)
            Me.BorderColor = PropBag.ReadProperty("BorderColor", DEFAULT_BORDERCOLOR)
            Me.BorderSize = PropBag.ReadProperty("BorderSize", DEFAULT_BORDERSIZE)
            Me.ShowMediaInfo = PropBag.ReadProperty("ShowMediaInfo", DEFAULT_SHOWMEDIAINFO)
            Me.BorderStyle = PropBag.ReadProperty("BorderStyle", DEFAULT_BORDERSTYLE)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub



            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_Resize
            ' * procedure description:  Occurs when a form is first displayed or the size of an object changes.
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_Resize()
            Dim sngBorderSize As Single
            On Local Error GoTo ErrLine
            
            'assign from module-level
            sngBorderSize = m_sngBorderSize * Screen.TwipsPerPixelX
            
            'fixture frame absolute position
            If fraFixture.Top <> (UserControl.ScaleTop + sngBorderSize) Then fraFixture.Top = (UserControl.ScaleTop + sngBorderSize)
            If fraFixture.Left <> (UserControl.ScaleLeft + sngBorderSize) Then fraFixture.Left = (UserControl.ScaleLeft + sngBorderSize)
            If fraFixture.Width <> (UserControl.ScaleWidth - (sngBorderSize * 2)) Then fraFixture.Width = (UserControl.ScaleWidth - (sngBorderSize * 2))
            If fraFixture.Height <> (UserControl.ScaleHeight - (sngBorderSize * 2)) Then fraFixture.Height = (UserControl.ScaleHeight - (sngBorderSize * 2))
            
            If lblClipName.Visible = False Then
               'source clip picturebox relative position
               If imgSourceClip.Top <> imgSourceClip.Parent.ScaleTop Then imgSourceClip.Top = imgSourceClip.Parent.ScaleTop
               If imgSourceClip.Left <> imgSourceClip.Parent.ScaleLeft Then imgSourceClip.Left = imgSourceClip.Parent.ScaleLeft
               If imgSourceClip.Width <> imgSourceClip.Parent.ScaleWidth Then imgSourceClip.Width = imgSourceClip.Parent.ScaleWidth
               If imgSourceClip.Height <> imgSourceClip.Parent.ScaleHeight Then imgSourceClip.Height = imgSourceClip.Parent.ScaleHeight
               
            Else
               'source clip picturebox relative position
               If imgSourceClip.Top <> imgSourceClip.Parent.ScaleTop Then imgSourceClip.Top = imgSourceClip.Parent.ScaleTop
               If imgSourceClip.Left <> imgSourceClip.Parent.ScaleLeft Then imgSourceClip.Left = imgSourceClip.Parent.ScaleLeft
               If imgSourceClip.Width <> imgSourceClip.Parent.ScaleWidth Then imgSourceClip.Width = imgSourceClip.Parent.ScaleWidth
               If imgSourceClip.Height <> (imgSourceClip.Parent.ScaleHeight * APP_DIVISIONAL_PERCENTAGE) Then imgSourceClip.Height = (imgSourceClip.Parent.ScaleHeight * APP_DIVISIONAL_PERCENTAGE)
               
               'source clip filename relative to source clip picturebox
               If lblClipName.Top <> (imgSourceClip.Top + imgSourceClip.Height) + APP_SEPTUM_SIZE Then lblClipName.Top = (imgSourceClip.Top + imgSourceClip.Height) + APP_SEPTUM_SIZE
               If lblClipName.Left <> lblClipName.Parent.ScaleLeft Then lblClipName.Left = lblClipName.Parent.ScaleLeft
               If lblClipName.Width <> lblClipName.Parent.ScaleWidth Then lblClipName.Width = lblClipName.Parent.ScaleWidth
               If lblClipName.Height <> lblClipName.Parent.ScaleHeight - (imgSourceClip.Height + APP_SEPTUM_SIZE) Then lblClipName.Height = lblClipName.Parent.ScaleHeight - (imgSourceClip.Height + APP_SEPTUM_SIZE)
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_Show
            ' * procedure description:  Occurs when the control's Visible property changes to True.
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_Show()
            On Local Error GoTo ErrLine
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_Terminate
            ' * procedure description:  Occurs when all references to an instance of a Form, MDIForm, or class are removed from memory.
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_Terminate()
            On Local Error GoTo ErrLine
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: UserControl_WriteProperties
            ' * procedure description:  Occurs when a user control or user document is asked to write its data to a file.
            ' *
            ' ******************************************************************************************************************************
            Private Sub UserControl_WriteProperties(PropBag As PropertyBag)
            On Local Error GoTo ErrLine
            
            'persist design time value(s) to the property bag only after design-time
            If Ambient.UserMode = False Then
               Call PropBag.WriteProperty("Filter", Me.Filter, DEFAULT_FILTER)
               Call PropBag.WriteProperty("MediaFile", Me.MediaFile, DEFAULT_MEDIAFILE)
               Call PropBag.WriteProperty("BorderColor", Me.BorderColor, DEFAULT_BORDERCOLOR)
               Call PropBag.WriteProperty("BorderSize", Me.BorderSize, DEFAULT_BORDERSIZE)
               Call PropBag.WriteProperty("BorderStyle", Me.BorderStyle, DEFAULT_BORDERSTYLE)
               Call PropBag.WriteProperty("ShowMediaInfo", Me.ShowMediaInfo, DEFAULT_SHOWMEDIAINFO)
            End If
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            
' **************************************************************************************************************************************
' * PRIVATE INTERFACE- CONTROL EVENTS
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: imgSourceClip_OLEDragDrop
            ' * procedure description:  Occurs when a user control or user document is asked to write its data to a file.
            ' *
            ' ******************************************************************************************************************************
            Private Sub imgSourceClip_OLEDragDrop(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            Call AppOLEDragDrop(Data, Effect, Button, Shift, X, Y)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: imgSourceClip_OLEDragOver
            ' * procedure description:  Occurs when a user control or user document is asked to write its data to a file.
            ' *
            ' ******************************************************************************************************************************
            Private Sub imgSourceClip_OLEDragOver(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single, State As Integer)
            On Local Error GoTo ErrLine
            Call AppOLEDragOver(Data, Effect, Button, Shift, X, Y, State)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: lblClipName_OLEDragDrop
            ' * procedure description:  Occurs when the mouse is moved over the control during an OLE drag/drop operation,
            ' *                                       if its OLEDropMode property is set to manual.
            ' ******************************************************************************************************************************
            Private Sub lblClipName_OLEDragDrop(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            Call AppOLEDragDrop(Data, Effect, Button, Shift, X, Y)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: lblClipName_OLEDragOver
            ' * procedure description:  Occurs when the mouse is moved over the control during an OLE drag/drop operation,
            ' *                                       if its OLEDropMode property is set to manual.
            ' ******************************************************************************************************************************
            Private Sub lblClipName_OLEDragOver(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single, State As Integer)
            On Local Error GoTo ErrLine
            Call AppOLEDragOver(Data, Effect, Button, Shift, X, Y, State)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: fraFixture_OLEDragDrop
            ' * procedure description:  Occurs when the mouse is moved over the control during an OLE drag/drop operation,
            ' *                                       if its OLEDropMode property is set to manual.
            ' ******************************************************************************************************************************
            Private Sub fraFixture_OLEDragDrop(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            Call AppOLEDragDrop(Data, Effect, Button, Shift, X, Y)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: fraFixture_OLEDragOver
            ' * procedure description:  Occurs when the mouse is moved over the control during an OLE drag/drop operation,
            ' *                                       if its OLEDropMode property is set to manual.
            ' ******************************************************************************************************************************
            Private Sub fraFixture_OLEDragOver(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single, State As Integer)
            On Local Error GoTo ErrLine
            Call AppOLEDragOver(Data, Effect, Button, Shift, X, Y, State)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            


' **************************************************************************************************************************************
' * PRIVATE INTERFACE- PROCEDURES
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: AppOLEDragDrop
            ' * procedure description:  Occurs when the mouse is moved over the control during an OLE drag/drop operation,
            ' *                                       if its OLEDropMode property is set to manual.
            ' ******************************************************************************************************************************
            Private Sub AppOLEDragDrop(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single)
            On Local Error GoTo ErrLine
            
            'load the media clip
            Me.MediaFile = Data.Files(1)
            Exit Sub
            
ErrLine:
            Err.Clear
            Exit Sub
            End Sub
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: AppOLEDragOver
            ' * procedure description:  Occurs when the mouse is moved over the control during an OLE drag/drop operation,
            ' *                                       if its OLEDropMode property is set to manual.
            ' ******************************************************************************************************************************
            Private Sub AppOLEDragOver(Data As DataObject, Effect As Long, Button As Integer, Shift As Integer, X As Single, Y As Single, State As Integer)
            Dim nCount As Long
            Dim nCount2 As Long
            Dim varSupport() As String
            Dim bstrSupport As String
            Dim bstrFileName As String
            On Local Error GoTo ErrLine
            
            'set default(s)
            Effect = vbDropEffectNone
            If Me.Filter = vbNullString Then
               bstrSupport = DEFAULT_FILTER
            Else: bstrSupport = m_bstrFilter
            End If
            'split the supported files into an array, if this fails the effect will be vbDropEffectNone
            varSupport = Split(bstrSupport, ";")
            
            For nCount = 1 To Data.Files.Count
                For nCount2 = LBound(varSupport) To UBound(varSupport)
                     If LCase(varSupport(nCount2)) <> vbNullString Then
                        If InStr(LCase(Data.Files(nCount)), LCase(varSupport(nCount2))) > 0 Then
                           'match located, supported media file dropped..
                           Effect = vbDropEffectCopy
                           bstrFileName = Data.Files(nCount)
                           Data.Files.Clear: Data.Files.Add bstrFileName
                           Exit Sub
                        End If
                     End If
                Next
            Next
            'reset effect
            Effect = vbDropEffectNone
            Exit Sub
            
ErrLine:

            Err.Clear
            'reset effect
            Effect = vbDropEffectNone
            Exit Sub
            End Sub
