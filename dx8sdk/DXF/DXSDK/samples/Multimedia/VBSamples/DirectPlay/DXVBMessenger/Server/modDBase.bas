Attribute VB_Name = "modDBase"
Option Explicit
Option Compare Text
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
'
'  Copyright (C) 1999-2001 Microsoft Corporation.  All Rights Reserved.
'
'  File:       modDBase.bas
'
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

Private Type FriendOnlineType
    sFriendName As String
    fOnline As Boolean
    fFriend As Boolean
End Type

Public Enum LogonTypes
    LogonSuccess
    InvalidPassword
    AccountDoesNotExist
End Enum

Public goDOM As DOMDocument 'The main xml document

Public Sub OpenClientDatabase()

    On Error GoTo ErrOut
    Dim sMedia As String
    
    sMedia = AddDirSep(GetDXSampleFolder) & "vbsamples\media\vbMsgSrv.xml"
    Set goDOM = New DOMDocument
    'Create a new XML Doc
    goDOM.async = False
    goDOM.Load sMedia 'This will load the xml document, if it's available, otherwise it will create a new one
    If Not goDOM.hasChildNodes Then 'This is an empty XML file
        MsgBox "The default XML data structure could not be found.  Creating a new one.", vbOKOnly Or vbInformation, "New XML."
        'Create our default file
        CreateDefaultXMLStructure
    End If
    Exit Sub
ErrOut:
    MsgBox "There was an error trying to load the XML file.  Creating a new one.", vbOKOnly Or vbInformation, "New XML."
    Set goDOM = Nothing
    Set goDOM = New DOMDocument
    'Create our default file
    CreateDefaultXMLStructure
End Sub

Public Sub CloseDownDB()
    MarkEveryoneLoggedOff
    SaveXMLStructure
    Set goDOM = Nothing
End Sub

'Check to see if this user already exists.  If they do, then we can't create a new account
'with this username.
Public Function DoesUserExist(ByVal sUserName As String) As Boolean
    'Let's see if this user exists
    Dim oNode As IXMLDOMNode
    Dim oNodes As IXMLDOMNodeList
    
    'Get a list of all client names
    Set oNodes = goDOM.selectNodes("MessengerServerDB/ClientInfo/Row/ClientName")
    For Each oNode In oNodes
        'See if we are in that list
        'We use the childnodes(0) since the only member of the 'ClientName' node
        'is the text that contains the name
        If oNode.childNodes(0).nodeTypedValue = sUserName Then
            DoesUserExist = True
            Exit Function
        End If
    Next
    DoesUserExist = False
End Function

Public Function LogonUser(ByVal sUserName As String, ByVal sPwd As String) As LogonTypes

    Dim oNode As IXMLDOMNode
    Dim sPassword As String
    
    Set oNode = ReturnUserNode(sUserName)
    If oNode Is Nothing Then
        LogonUser = AccountDoesNotExist
        Exit Function
    End If
    'Ok, this user does exist.  First lets decrypt the password sent from the client
    sPassword = EncodePassword(sPwd, glClientSideEncryptionKey)
    'Now check this password against what's listed in the db.
    'The next sibling of the 'ClientName' node is the password node.
    'The first child of that node is the actual text value of the password
    If oNode.nextSibling.childNodes(0).nodeTypedValue = EncodePassword(sPassword, glServerSideEncryptionKey) Then
        'The passwords match, logon was successful
        LogonUser = LogonSuccess
        Exit Function
    Else
        'Invalid password, let the user know
        LogonUser = InvalidPassword
        Exit Function
    End If
End Function

Public Sub AddUser(ByVal sUserName As String, ByVal sPwd As String, ByVal lCurrentDPlayID As Long)
    Dim sPassword As String
    
    'First decrypt the password
    sPassword = EncodePassword(sPwd, glClientSideEncryptionKey)
    'Now add this user to our xml structure
    AddUserXML sUserName, EncodePassword(sPassword, glServerSideEncryptionKey), True, lCurrentDPlayID
End Sub

Public Sub UpdateDBToShowLogon(ByVal sPlayer As String, ByVal lCurrentDPlayID As Long)
    'Set the flag to show that we are currently logged on, and keep our current DPlay ID
    Dim oNode As IXMLDOMNode
    Dim oUpdate As IXMLDOMNode
    
    Set oNode = ReturnUserNode(sPlayer)
    Set oUpdate = GetNode(oNode, "CurrentlyLoggedIn")
    oUpdate.childNodes(0).nodeTypedValue = -1
    Set oUpdate = GetNode(oNode, "CurrentDPlayID")
    oUpdate.childNodes(0).nodeTypedValue = lCurrentDPlayID
End Sub

Public Sub UpdateDBToShowLogoff(ByVal lCurrentDPlayID As Long)
    'Set the flag to show that we are currently logged off, and lose our current DPlay ID
    Dim oNode As IXMLDOMNode
    Dim oUpdate As IXMLDOMNode
    
    Set oNode = ReturnUserNodeFromDPlayID(lCurrentDPlayID)
    Set oUpdate = GetNode(oNode, "CurrentlyLoggedIn")
    oUpdate.childNodes(0).nodeTypedValue = 0
    Set oUpdate = GetNode(oNode, "CurrentDPlayID")
    oUpdate.childNodes(0).nodeTypedValue = 0
End Sub

Public Sub NotifyFriends(ByVal sPlayer As String, ByVal lNotifyMsg As Long)
    Dim lMsg As Long
    Dim oBuf() As Byte, lOffset As Long
    Dim oNodes As IXMLDOMNodeList, oTemp As IXMLDOMNode
    Dim oNode As IXMLDOMNode, oNodeFriend As IXMLDOMNode
    
    'See if I'm anyone's friends
    Set oNodes = goDOM.selectNodes("MessengerServerDB/FriendList/Row/FriendName")
    For Each oNode In oNodes
        If oNode.childNodes(0).nodeTypedValue = sPlayer Then 'Yup, I am.  Notify each of them that I just logged on
            'First check to see if they are logged on
            Set oNodeFriend = ReturnUserNode(oNode.previousSibling.childNodes(0).nodeTypedValue)
            Set oTemp = GetNode(oNodeFriend, "CurrentDPlayID")
            If Not (oTemp Is Nothing) Then
                lMsg = lNotifyMsg
                lOffset = NewBuffer(oBuf)
                AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
                AddStringToBuffer oBuf, sPlayer, lOffset
                dps.SendTo CLng(oTemp.childNodes(0).nodeTypedValue), oBuf, 0, 0
            End If
            Set oNodeFriend = Nothing
            Set oTemp = Nothing
        End If
    Next
    
End Sub

Public Sub GetFriendsOfMineOnline(ByVal sPlayer As String)
    Dim lMsg As Long
    Dim oBuf() As Byte, lOffset As Long
    Dim oNodes As IXMLDOMNodeList
    Dim oNode As IXMLDOMNode, oNodeFriend As IXMLDOMNode
    
    Dim lSendID As Long
    Dim oFriends() As FriendOnlineType
    Dim lCount As Long
    'First we need to find out if I have any friends in my list
    Set oNodes = goDOM.selectNodes("MessengerServerDB/FriendList/Row/ClientName")
    ReDim oFriends(0)
    For Each oNode In oNodes
        If oNode.childNodes(0).nodeTypedValue = sPlayer Then 'Yup, I do.  Lets save them so I can notify them
            'Get the friend node
            Set oNodeFriend = ReturnUserNode(oNode.nextSibling.childNodes(0).nodeTypedValue)
                'First check to see if they are logged on
                ReDim Preserve oFriends(UBound(oFriends) + 1)
                With oFriends(UBound(oFriends))
                    .sFriendName = oNodeFriend.childNodes(0).nodeTypedValue
                    .fOnline = GetNode(oNodeFriend, "CurrentDPlayID").childNodes(0).nodeTypedValue <> 0
                    .fFriend = oNode.nextSibling.nextSibling.childNodes(0).nodeTypedValue
                End With
            Set oNodeFriend = Nothing
        End If
    Next
    
    'Get my DPlayID
    lSendID = GetCurrentDPlayID(sPlayer)
    'Now see if I have any friends
    lMsg = Msg_SendClientFriends
    lOffset = NewBuffer(oBuf)
    AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
    AddDataToBuffer oBuf, CLng(UBound(oFriends)), SIZE_LONG, lOffset
    For lCount = 1 To UBound(oFriends)
        AddDataToBuffer oBuf, oFriends(lCount).fFriend, LenB(oFriends(lCount).fFriend), lOffset
        AddStringToBuffer oBuf, oFriends(lCount).sFriendName, lOffset
    Next
    dps.SendTo lSendID, oBuf, 0, 0
    For lCount = 1 To UBound(oFriends)
        If oFriends(lCount).fOnline Then
            ReDim oBuf(0)
            lMsg = Msg_FriendLogon
            lOffset = NewBuffer(oBuf)
            AddDataToBuffer oBuf, lMsg, LenB(lMsg), lOffset
            AddStringToBuffer oBuf, oFriends(lCount).sFriendName, lOffset
            dps.SendTo lSendID, oBuf, 0, 0
        End If
    Next
    
End Sub

'If fFriend is True, then this person is a friend.  If it is False, then the person is blocked
Public Function AddFriend(ByVal lPlayerID As Long, ByVal sFriendName As String, ByVal fFriend As Boolean) As Boolean
    Dim oMyNode As IXMLDOMNode
    Dim oFriendNode As IXMLDOMNode
    Dim oNodeList As IXMLDOMNodeList, oNode As IXMLDOMNode, fFoundNode As Boolean
    
    AddFriend = False
    fFoundNode = False
    Set oFriendNode = ReturnUserNode(sFriendName)
    'Is this friend currently logged in?
    If GetNode(oFriendNode, "CurrentDPlayID").childNodes(0).nodeTypedValue <> 0 Then AddFriend = True
    Set oMyNode = ReturnUserNodeFromDPlayID(lPlayerID)
    
    Set oNodeList = goDOM.selectNodes("MessengerServerDB/FriendList/Row/ClientName")
    For Each oNode In oNodeList
        If (oNode.childNodes(0).nodeTypedValue = oMyNode.childNodes(0).nodeTypedValue) And _
            oNode.nextSibling.childNodes(0).nodeTypedValue = sFriendName Then
            fFoundNode = True
            'We found the node, update it
            oNode.nextSibling.nextSibling.childNodes(0).nodeTypedValue = fFriend
            Exit For
        End If
    Next
    If Not fFoundNode Then    'We need to add this node
        AddFriendXML oMyNode.childNodes(0).nodeTypedValue, sFriendName, fFriend
    End If
End Function

Public Function AmIBlocked(ByVal sMe As String, ByVal sFriend As String) As Boolean
    Dim oMyNode As IXMLDOMNode
    Dim oFriendNode As IXMLDOMNode
    Dim oNodeList As IXMLDOMNodeList, oNode As IXMLDOMNode
    
    AmIBlocked = False
    Set oFriendNode = ReturnUserNode(sFriend)
    'Is this friend currently logged in?
    Set oMyNode = ReturnUserNode(sMe)
    
    Set oNodeList = goDOM.selectNodes("MessengerServerDB/FriendList/Row/ClientName")
    For Each oNode In oNodeList
        If (oNode.childNodes(0).nodeTypedValue = oMyNode.childNodes(0).nodeTypedValue) And _
            oNode.nextSibling.childNodes(0).nodeTypedValue = sFriend Then
            'We found the node, update it
            AmIBlocked = (oNode.nextSibling.nextSibling.childNodes(0).nodeTypedValue = False)
            Exit For
        End If
    Next
End Function

Public Function GetCurrentDPlayID(ByVal sPlayer As String) As Long
    Dim oNode As IXMLDOMNode
    
    Set oNode = ReturnUserNode(sPlayer)
    'We need to get the current dplay id of this person
    GetCurrentDPlayID = CLng(GetNode(oNode, "CurrentDPlayID").childNodes(0).nodeTypedValue)
End Function

Private Sub CreateDefaultXMLStructure()
  Dim root As IXMLDOMNode
  Dim clientNode As IXMLDOMNode, friendNode As IXMLDOMNode
  'Create a 'root' node
  Set root = goDOM.createNode(NODE_ELEMENT, "MessengerServerDB", vbNullString)
  goDOM.appendChild root
  Set clientNode = goDOM.createNode(NODE_ELEMENT, "ClientInfo", vbNullString)
  root.appendChild clientNode
  Set friendNode = goDOM.createNode(NODE_ELEMENT, "FriendList", vbNullString)
  root.appendChild friendNode

End Sub

Private Sub AddUserXML(ByVal sUserName As String, ByVal sPassword As String, ByVal fLoggedOn As Boolean, ByVal lCurrentDPlayID As Long)
    Dim oClientNode As IXMLDOMNode, lCount As Long
    Dim oRowNode As IXMLDOMNode, oTableField As IXMLDOMNode
    Dim oText As IXMLDOMNode
    Dim oNodes As IXMLDOMNodeList
    
    'Ok, we need to add a user, first check to see if there are any nodes in our xml doc, if so, add them to those
    'if not, create new ones (we already did this when we created the file, just in case tho)
    If Not goDOM.hasChildNodes Then  'This is an empty XML file
        'Create our default file
        CreateDefaultXMLStructure
    End If
    'We know the first item is the main node
    Set oNodes = goDOM.selectNodes("MessengerServerDB/ClientInfo")
    For Each oClientNode In oNodes
        If oClientNode.nodeName = "ClientInfo" Then
            Set oRowNode = goDOM.createNode(NODE_ELEMENT, "Row", vbNullString)
            'now the Client name
            Set oText = Nothing
            Set oTableField = Nothing
            Set oTableField = goDOM.createNode(NODE_ELEMENT, "ClientName", vbNullString)
            Set oText = goDOM.createNode(NODE_TEXT, vbNullString, vbNullString)
            oText.nodeTypedValue = sUserName
            oTableField.appendChild oText
            oRowNode.appendChild oTableField
            'now the Client pwd
            Set oText = Nothing
            Set oTableField = Nothing
            Set oTableField = goDOM.createNode(NODE_ELEMENT, "ClientPassword", vbNullString)
            Set oText = goDOM.createNode(NODE_TEXT, vbNullString, vbNullString)
            oText.nodeTypedValue = sPassword
            oTableField.appendChild oText
            oRowNode.appendChild oTableField
            'now the currently logged in state
            Set oText = Nothing
            Set oTableField = Nothing
            Set oTableField = goDOM.createNode(NODE_ELEMENT, "CurrentlyLoggedIn", vbNullString)
            Set oText = goDOM.createNode(NODE_TEXT, vbNullString, vbNullString)
            oText.nodeTypedValue = fLoggedOn
            oTableField.appendChild oText
            oRowNode.appendChild oTableField
            'now the currently logged in state
            Set oText = Nothing
            Set oTableField = Nothing
            Set oTableField = goDOM.createNode(NODE_ELEMENT, "CurrentDPlayID", vbNullString)
            Set oText = goDOM.createNode(NODE_TEXT, vbNullString, vbNullString)
            oText.nodeTypedValue = lCurrentDPlayID
            oTableField.appendChild oText
            oRowNode.appendChild oTableField
            'Now actually add the row
            oClientNode.appendChild oRowNode
        End If
    Next
End Sub

Private Sub AddFriendXML(ByVal sUserName As String, ByVal sFriend As String, ByVal fFriend As Boolean)
    Dim oFriendNode As IXMLDOMNode, lCount As Long
    Dim oRowNode As IXMLDOMNode, oTableField As IXMLDOMNode
    Dim oText As IXMLDOMNode
    Dim oNodes As IXMLDOMNodeList
    
    'Ok, we need to add a user, first check to see if there are any nodes in our xml doc, if so, add them to those
    'if not, create new ones (we already did this when we created the file, just in case tho)
    If Not goDOM.hasChildNodes Then  'This is an empty XML file
        'Create our default file
        CreateDefaultXMLStructure
    End If
    
    Set oNodes = goDOM.selectNodes("MessengerServerDB/FriendList")
    For Each oFriendNode In oNodes
        If oFriendNode.nodeName = "FriendList" Then
            Set oRowNode = goDOM.createNode(NODE_ELEMENT, "Row", vbNullString)
            'now the Client name
            Set oText = Nothing
            Set oTableField = Nothing
            Set oTableField = goDOM.createNode(NODE_ELEMENT, "ClientName", vbNullString)
            Set oText = goDOM.createNode(NODE_TEXT, vbNullString, vbNullString)
            oText.nodeTypedValue = sUserName
            oTableField.appendChild oText
            oRowNode.appendChild oTableField
            'now the friend name
            Set oText = Nothing
            Set oTableField = Nothing
            Set oTableField = goDOM.createNode(NODE_ELEMENT, "FriendName", vbNullString)
            Set oText = goDOM.createNode(NODE_TEXT, vbNullString, vbNullString)
            oText.nodeTypedValue = sFriend
            oTableField.appendChild oText
            oRowNode.appendChild oTableField
            'now the friend state
            Set oText = Nothing
            Set oTableField = Nothing
            Set oTableField = goDOM.createNode(NODE_ELEMENT, "Friend", vbNullString)
            Set oText = goDOM.createNode(NODE_TEXT, vbNullString, vbNullString)
            oText.nodeTypedValue = fFriend
            oTableField.appendChild oText
            oRowNode.appendChild oTableField
            'Now actually add the row
            oFriendNode.appendChild oRowNode
        End If
    Next
End Sub


Private Function ReturnUserNode(ByVal sUserName As String) As IXMLDOMNode
    Dim oNode As IXMLDOMNode
    Dim oNodes As IXMLDOMNodeList
    
    'Get a list of all client names
    Set oNodes = goDOM.selectNodes("MessengerServerDB/ClientInfo/Row/ClientName")
    For Each oNode In oNodes
        'See if we are in that list
        'We use the childnodes(0) since the only member of the 'ClientName' node
        'is the text that contains the name
        If oNode.childNodes(0).nodeTypedValue = sUserName Then
            Set ReturnUserNode = oNode
            Exit Function
        End If
    Next
    Set ReturnUserNode = Nothing
End Function

Private Function ReturnUserNodeFromDPlayID(ByVal lID As Long) As IXMLDOMNode
    Dim oNode As IXMLDOMNode
    Dim oNodes As IXMLDOMNodeList
    
    'Get a list of all client names
    Set oNodes = goDOM.selectNodes("MessengerServerDB/ClientInfo/Row/CurrentDPlayID")
    For Each oNode In oNodes
        'See if we are in that list
        'We use the childnodes(0) since the only member of the 'CurrentDPlayID' node
        'is the text that contains the id
        If oNode.childNodes(0).nodeTypedValue = lID Then
            'The user node is 3 siblings before the DPlayID node
            Set ReturnUserNodeFromDPlayID = oNode.previousSibling.previousSibling.previousSibling
            Exit Function
        End If
    Next
    Set ReturnUserNodeFromDPlayID = Nothing
End Function

Private Function GetNode(ByVal oNode As IXMLDOMNode, ByVal sNodeName As String) As IXMLDOMNode
    Dim oUpdate As IXMLDOMNode
    
    On Error GoTo ExitFunc
    Set oUpdate = oNode.nextSibling
    Do
        If oUpdate.nodeName = sNodeName Then
            Set GetNode = oUpdate
            Exit Function
        End If
        Set oUpdate = oUpdate.nextSibling
    Loop
ExitFunc:
Set GetNode = Nothing
End Function

Private Function MarkEveryoneLoggedOff()
    Dim oNode As IXMLDOMNode
    Dim oNodes As IXMLDOMNodeList
    
    'Get a list of all client names
    Set oNodes = goDOM.selectNodes("MessengerServerDB/ClientInfo/Row/ClientName")
    For Each oNode In oNodes
        'Mark everyone as logged off
        oNode.nextSibling.nextSibling.childNodes(0).nodeTypedValue = 0
        oNode.nextSibling.nextSibling.nextSibling.childNodes(0).nodeTypedValue = 0
    Next
End Function

Public Sub SaveXMLStructure()
    'Save the XML structure out to a file
    goDOM.Save AddDirSep(GetDXSampleFolder) & "vbsamples\media\vbMsgSrv.xml"
End Sub
