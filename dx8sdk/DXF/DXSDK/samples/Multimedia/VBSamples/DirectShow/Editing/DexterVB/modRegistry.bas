Attribute VB_Name = "modRegistry"
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
' * PRIVATE INTERFACE- DATA STRUCTURES
' *
' *
            Private Type ACL
                    AclRevision As Byte
                    Sbz1 As Byte
                    AclSize As Integer
                    AceCount As Integer
                    Sbz2 As Integer
            End Type
            
            Private Type FILETIME
                    dwLowDateTime As Long
                    dwHighDateTime As Long
            End Type
            
            Private Type SECURITY_ATTRIBUTES
                    nLength As Long
                    lpSecurityDescriptor As Long
                    bInheritHandle As Long
            End Type
            
            Private Type SECURITY_DESCRIPTOR
                    Revision As Byte
                    Sbz1 As Byte
                    Control As Long
                    Owner As Long
                    Group As Long
                    Sacl As ACL
                    Dacl As ACL
            End Type
            
            
            
            
            
' **************************************************************************************************************************************
' * PRIVATE INTERFACE- PREDEFINED CONSTANTS
' *
' *
            Private Const MAX_PATH = 255
            Private Const ERROR_SUCCESS = 0
            
            Private Const KEY_ALL_ACCESS = &H1F0000 '((STANDARD_RIGHTS_ALL Or KEY_QUERY_VALUE Or KEY_SET_VALUE Or KEY_CREATE_SUB_KEY Or KEY_ENUMERATE_SUB_KEYS Or KEY_NOTIFY Or KEY_CREATE_LINK) And (Not SYNCHRONIZE))
            Private Const KEY_CREATE_LINK = &H20
            Private Const KEY_CREATE_SUB_KEY = &H4
            Private Const KEY_ENUMERATE_SUB_KEYS = &H8
            Private Const KEY_EVENT = &H1      ''"" Event contains key event record
            Private Const KEY_EXECUTE = &H1  '((KEY_READ) And (Not SYNCHRONIZE))
            Private Const KEY_FULL_MATCH_SEARCH = &H1
            Private Const KEY_LENGTH_MASK = &HFFFF0000
            Private Const KEY_NOTIFY = &H10
            Private Const KEY_PARTIAL_MATCH_SEARCH = &H2
            Private Const KEY_QUERY_VALUE = &H1
            Private Const KEY_READ = KEY_QUERY_VALUE '((STANDARD_RIGHTS_READ Or KEY_QUERY_VALUE Or KEY_ENUMERATE_SUB_KEYS Or KEY_NOTIFY) And (Not SYNCHRONIZE))
            Private Const KEY_SET_VALUE = &H2
            Private Const KEY_WRITE = KEY_SET_VALUE   '((STANDARD_RIGHTS_WRITE Or KEY_SET_VALUE Or KEY_CREATE_SUB_KEY) And (Not SYNCHRONIZE))
            
            Private Const REG_CREATED_NEW_KEY = &H1
            Private Const REG_FULL_RESOURCE_DESCRIPTOR = 9
            Private Const REG_LEGAL_CHANGE_FILTER = &H2 ' (REG_NOTIFY_CHANGE_NAME Or REG_NOTIFY_CHANGE_ATTRIBUTES Or REG_NOTIFY_CHANGE_LAST_SET Or REG_NOTIFY_CHANGE_SECURITY)
            Private Const REG_LEGAL_OPTION = 0 '(REG_OPTION_RESERVED Or REG_OPTION_NON_VOLATILE Or REG_OPTION_VOLATILE Or REG_OPTION_CREATE_LINK Or REG_OPTION_BACKUP_RESTORE)
            Private Const REG_NOTIFY_CHANGE_ATTRIBUTES = &H2
            Private Const REG_NOTIFY_CHANGE_LAST_SET = &H4
            Private Const REG_NOTIFY_CHANGE_NAME = &H1
            Private Const REG_NOTIFY_CHANGE_SECURITY = &H8
            Private Const REG_OPENED_EXISTING_KEY = &H2
            Private Const REG_OPTION_BACKUP_RESTORE = 4
            Private Const REG_OPTION_CREATE_LINK = 2
            Private Const REG_OPTION_NON_VOLATILE = 0
            Private Const REG_OPTION_RESERVED = 0
            Private Const REG_OPTION_VOLATILE = 1
            Private Const REG_REFRESH_HIVE = &H2
            Private Const REG_RESOURCE_REQUIREMENTS_LIST = 10
            Private Const REG_WHOLE_HIVE_VOLATILE = &H1
            
            
            
            
' **************************************************************************************************************************************
' * PRIVATE INTERFACE- WIN32 API DECLARATIONS
' *
' *
            Private Declare Function RegCloseKey Lib "advapi32.dll" (ByVal hKey As Long) As Long
            Private Declare Function RegConnectRegistry Lib "advapi32.dll" Alias "RegConnectRegistryA" (ByVal lpMachineName As String, ByVal hKey As Long, phkResult As Long) As Long
            Private Declare Function RegCreateKey Lib "advapi32.dll" Alias "RegCreateKeyA" (ByVal hKey As Long, ByVal lpSubKey As String, phkResult As Long) As Long
            Private Declare Function RegCreateKeyEx Lib "advapi32.dll" Alias "RegCreateKeyExA" (ByVal hKey As Long, ByVal lpSubKey As String, ByVal Reserved As Long, ByVal lpClass As String, ByVal dwOptions As Long, ByVal samDesired As Long, lpSecurityAttributes As SECURITY_ATTRIBUTES, phkResult As Long, lpdwDisposition As Long) As Long
            Private Declare Function RegDeleteKey Lib "advapi32.dll" Alias "RegDeleteKeyA" (ByVal hKey As Long, ByVal lpSubKey As String) As Long
            Private Declare Function RegDeleteValue Lib "advapi32.dll" Alias "RegDeleteValueA" (ByVal hKey As Long, ByVal lpValueName As String) As Long
            Private Declare Function RegEnumKey Lib "advapi32.dll" Alias "RegEnumKeyA" (ByVal hKey As Long, ByVal dwIndex As Long, ByVal lpName As String, ByVal cbName As Long) As Long
            Private Declare Function RegEnumKeyEx Lib "advapi32.dll" Alias "RegEnumKeyExA" (ByVal hKey As Long, ByVal dwIndex As Long, ByVal lpName As String, lpcbName As Long, ByVal lpReserved As Long, ByVal lpClass As String, lpcbClass As Long, lpftLastWriteTime As FILETIME) As Long
            Private Declare Function RegEnumValue Lib "advapi32.dll" Alias "RegEnumValueA" (ByVal hKey As Long, ByVal dwIndex As Long, ByVal lpValueName As String, lpcbValueName As Long, ByVal lpReserved As Long, lpType As Long, lpData As Byte, lpcbData As Long) As Long
            Private Declare Function RegFlushKey Lib "advapi32.dll" (ByVal hKey As Long) As Long
            Private Declare Function RegGetKeySecurity Lib "advapi32.dll" (ByVal hKey As Long, ByVal SecurityInformation As Long, pSecurityDescriptor As SECURITY_DESCRIPTOR, lpcbSecurityDescriptor As Long) As Long
            Private Declare Function RegLoadKey Lib "advapi32.dll" Alias "RegLoadKeyA" (ByVal hKey As Long, ByVal lpSubKey As String, ByVal lpFile As String) As Long
            Private Declare Function RegNotifyChangeKeyValue Lib "advapi32.dll" (ByVal hKey As Long, ByVal bWatchSubtree As Long, ByVal dwNotifyFilter As Long, ByVal hEvent As Long, ByVal fAsynchronus As Long) As Long
            Private Declare Function RegOpenKey Lib "advapi32.dll" Alias "RegOpenKeyA" (ByVal hKey As Long, ByVal lpSubKey As String, phkResult As Long) As Long
            Private Declare Function RegOpenKeyEx Lib "advapi32.dll" Alias "RegOpenKeyExA" (ByVal hKey As Long, ByVal lpSubKey As String, ByVal ulOptions As Long, ByVal samDesired As Long, phkResult As Long) As Long
            Private Declare Function RegQueryInfoKey Lib "advapi32.dll" Alias "RegQueryInfoKeyA" (ByVal hKey As Long, ByVal lpClass As String, lpcbClass As Long, ByVal lpReserved As Long, lpcSubKeys As Long, lpcbMaxSubKeyLen As Long, lpcbMaxClassLen As Long, lpcValues As Long, lpcbMaxValueNameLen As Long, lpcbMaxValueLen As Long, lpcbSecurityDescriptor As Long, lpftLastWriteTime As FILETIME) As Long
            Private Declare Function RegQueryValue Lib "advapi32.dll" Alias "RegQueryValueA" (ByVal hKey As Long, ByVal lpSubKey As String, ByVal lpValue As String, lpcbValue As Long) As Long
            Private Declare Function RegQueryValueEx Lib "advapi32.dll" Alias "RegQueryValueExA" (ByVal hKey As Long, ByVal lpValueName As String, ByVal lpReserved As Long, lpType As Long, lpData As Any, lpcbData As Long) As Long
            Private Declare Function RegReplaceKey Lib "advapi32.dll" Alias "RegReplaceKeyA" (ByVal hKey As Long, ByVal lpSubKey As String, ByVal lpNewFile As String, ByVal lpOldFile As String) As Long
            Private Declare Function RegRestoreKey Lib "advapi32.dll" Alias "RegRestoreKeyA" (ByVal hKey As Long, ByVal lpFile As String, ByVal dwFlags As Long) As Long
            Private Declare Function RegSaveKey Lib "advapi32.dll" Alias "RegSaveKeyA" (ByVal hKey As Long, ByVal lpFile As String, lpSecurityAttributes As SECURITY_ATTRIBUTES) As Long
            Private Declare Function RegSetKeySecurity Lib "advapi32.dll" (ByVal hKey As Long, ByVal SecurityInformation As Long, pSecurityDescriptor As SECURITY_DESCRIPTOR) As Long
            Private Declare Function RegSetValue Lib "advapi32.dll" Alias "RegSetValueA" (ByVal hKey As Long, ByVal lpSubKey As String, ByVal dwType As Long, ByVal lpData As String, ByVal cbData As Long) As Long
            Private Declare Function RegSetValueEx Lib "advapi32.dll" Alias "RegSetValueExA" (ByVal hKey As Long, ByVal lpValueName As String, ByVal Reserved As Long, ByVal dwType As Long, lpData As Any, ByVal cbData As Long) As Long
            Private Declare Function RegUnLoadKey Lib "advapi32.dll" Alias "RegUnLoadKeyA" (ByVal hKey As Long, ByVal lpSubKey As String) As Long
            



' **************************************************************************************************************************************
' * PUBLIC INTERFACE- ENUMERATIONS
' *
' *
            Public Enum hKey
            HKEY_CLASSES_ROOT = &H80000000
            HKEY_CURRENT_CONFIG = &H80000005
            HKEY_CURRENT_USER = &H80000001
            HKEY_DYN_DATA = &H80000006
            HKEY_LOCAL_MACHINE = &H80000002
            HKEY_PERFORMANCE_DATA = &H80000004
            HKEY_USERS = &H80000003
            End Enum
            
            
            Public Enum EntryFormat
            REG_BINARY = 0                              'Binary data in any form.
            REG_DWORD = 1                              'A 32-bit number.
            REG_DWORD_LITTLE_ENDIAN = 2  'A 32-bit number in little-endian format. This is equivalent to REG_DWORD.
            REG_DWORD_BIG_ENDIAN = 3       'A 32-bit number in big-endian format.
            REG_EXPAND_SZ = 4                       'A null-terminated string that contains unexpanded references to environment variables
            REG_LINK = 5                                   'A Unicode symbolic link.
            REG_MULTI_SZ = 6                          'An array of null-terminated strings, terminated by two null characters.
            REG_NONE = 7                                 'No defined value type.
            REG_RESOURCE_LIST = 8              'A device-driver resource list.
            REG_SZ = 9                                      'A null-terminated string. It will be a Unicode or ANSI string depending on whether you use Unicode or ANSI.
            End Enum



' **************************************************************************************************************************************
' * PUBLIC INTERFACE- PROCEDURES
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: Registry_CreateKey
            ' * procedure description:   Create's a new key in the window's system registry.
            ' *                                        Returns the registry error code on failure, the new handle on success
            ' ******************************************************************************************************************************
            Public Function Registry_CreateKey(MainKey As hKey, SubKey As String) As Long
            Dim nRet As Long, nDisposition As Long, nKey As Long
            Dim sSubKey As String, nSubStart As Integer, SecAttr As SECURITY_ATTRIBUTES
            On Local Error GoTo ErrLine
            
            'check to ensure subkey is valid
            If Len(SubKey) = 0 Then Exit Function
            
            'check to ensure the mainkey is valid
            If CLng(MainKey) = 0 Then Exit Function
            
            'convert to upper case
            SubKey = UCase(SubKey)
            
            'check for backslash
            If Left(SubKey, 1) = "\" Then SubKey = Mid(SubKey, 2, Len(SubKey))
            
            'Create a new Key
            nRet = RegCreateKeyEx(CLng(MainKey), SubKey, 0, vbNullString, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, SecAttr, nKey, nDisposition)
            
            If nKey <> 0 Then  'the key was created successfully; return the handle.
               RegCloseKey (nKey)
               Registry_CreateKey = nRet
            Else 'an error occured, return zero and exit.
               Registry_CreateKey = 0
               Exit Function
            End If
            Exit Function
                        
ErrLine:

            Err.Clear
            If nKey <> 0 Then RegCloseKey (nKey) 'the key is open close it and exit
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Registry_DeleteKey
            ' * procedure description:   deletes an existing key in the window's system registry.
            ' *
            ' ******************************************************************************************************************************
            Public Function Registry_DeleteKey(MainKey As hKey, SubKey As String) As Long
            On Local Error GoTo ErrLine
            
            'check to ensure subkey is valid
            If Len(SubKey) = 0 Then Exit Function
            
            'check to ensure the mainkey is valid
            If CLng(MainKey) = 0 Then Exit Function
            
            'convert to upper case
            SubKey = UCase(SubKey)
            
            'check for backslash
            If Left(SubKey, 1) = "\" Then SubKey = Mid(SubKey, 2, Len(SubKey))
            
            'delete the key from the registry; if WinNT this will fail if the key has subkeys
            Registry_DeleteKey = RegDeleteKey(CLng(MainKey), SubKey)
            'exit
            Exit Function
            
            
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Registry_DoesKeyExist
            ' * procedure description:   Checks to ensure a key does in fact exist
            ' *
            ' ******************************************************************************************************************************
            Public Function Registry_DoesKeyExist(MainKey As hKey, SubKey As String) As Boolean
            Dim nSubHandle As Long
            On Local Error GoTo ErrLine
            
            'check to ensure subkey is valid
            If Len(SubKey) = 0 Then Exit Function
            
            'check to ensure the mainkey is valid
            If CLng(MainKey) = 0 Then Exit Function
            
            'convert to upper case
            SubKey = UCase(SubKey)
            
            'check for backslash
            If Left(SubKey, 1) = "\" Then SubKey = Mid(SubKey, 2, Len(SubKey))
            
            
            'open the key for read access
            RegOpenKeyEx CLng(MainKey), SubKey, 0, KEY_READ, nSubHandle
            
            'return and exit
            If nSubHandle <> 0 Then
               Registry_DoesKeyExist = True
               RegCloseKey (nSubHandle)
            Else:  Registry_DoesKeyExist = False
            End If
            Exit Function
            
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Registry_CreateEntry
            ' * procedure description:   Creates an entry for the user
            ' *
            ' ******************************************************************************************************************************
            Public Function Registry_CreateEntry(MainKey As hKey, SubKey As String, strEntry As String, Optional strEntryValue As String = vbNullString, Optional Format As EntryFormat = REG_SZ) As Long
            Dim nSubHandle As Long, nRet As Long, strBuffer As String
            On Local Error GoTo ErrLine
            'check to ensure subkey is valid
            If Len(SubKey) = 0 Then Exit Function
            
            'check to ensure the mainkey is valid
            If CLng(MainKey) = 0 Then Exit Function
            
            'check to ensure the entry's string data is null-terminated
            If Right(strEntryValue, 1) <> vbNullChar Then strEntryValue = (strEntryValue & vbNullChar)
            
            'form a buffer for the entry's string data to be passed the the api
            strBuffer = String(Len(strEntryValue), 0)
            strBuffer = strEntryValue
            
            'convert to upper case
            SubKey = UCase(SubKey)
            
            'check for backslash
            If Left(SubKey, 1) = "\" Then SubKey = Mid(SubKey, 2, Len(SubKey))
            
            'open the key with update value access; this should be all that is required to append an entry..
            nRet = RegOpenKeyEx(CLng(MainKey), SubKey, 0, KEY_SET_VALUE, nSubHandle)
            
            'check api return for success before continueing
            If nRet <> ERROR_SUCCESS Or nSubHandle = 0 Then Exit Function
            
            'set the new entry value to the key
            Registry_CreateEntry = RegSetValueEx(nSubHandle, strEntry, 0, CLng(Format), ByVal strBuffer, Len(strBuffer) + 1)
            
            'close the key handle
            RegCloseKey (nSubHandle)
            
            'exit
            Exit Function
            
            
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Registry_DeleteEntry
            ' * procedure description:   Delete's an entry in a registry subkey
            ' *
            ' ******************************************************************************************************************************
            Public Function Registry_DeleteEntry(MainKey As hKey, SubKey As String, strEntry As String) As Long
            Dim nSubHandle, nRet As Long
            On Local Error GoTo ErrLine
            'check to ensure subkey is valid
            If Len(SubKey) = 0 Then Exit Function
            
            'check to ensure the mainkey is valid
            If CLng(MainKey) = 0 Then Exit Function
            
            'check to ensure the entryname is valid
            If strEntry = vbNullString Then Exit Function
            'check that it is null terminated
            If Right(strEntry, 1) <> vbNullChar Then strEntry = (strEntry & vbNullChar)
            
            'convert to upper case
            SubKey = UCase(SubKey)
            
            'check for backslash
            If Left(SubKey, 1) = "\" Then SubKey = Mid(SubKey, 2, Len(SubKey))
            
            'open the key with local write access; this should be all that is required to append an entry..
            nRet = RegOpenKeyEx(CLng(MainKey), SubKey, 0, KEY_WRITE, nSubHandle)
            
            'check api return before continueing
            If nRet <> ERROR_SUCCESS Or nSubHandle = 0 Then Exit Function
            
            'attempt to delete the entry and return the result
            Registry_DeleteEntry = RegDeleteValue(nSubHandle, strEntry)
            
            'close the open key handle and exit
            RegCloseKey (nSubHandle)
            Exit Function
            
            
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Registry_UpdateEntry
            ' * procedure description:   Updates the value of an entry within a subkey; this function will create it if it does not exist.
            ' *
            ' ******************************************************************************************************************************
            Public Function Registry_UpdateEntry(MainKey As hKey, SubKey As String, strEntry As String, strEntryValue As String, Optional Format As EntryFormat = REG_SZ) As Long
            Dim nSubHandle As Long, nRet As Long, strBuffer As String
            On Local Error GoTo ErrLine
            'check to ensure subkey is valid
            If Len(SubKey) = 0 Then Exit Function
            
            'check to ensure the mainkey is valid
            If CLng(MainKey) = 0 Then Exit Function
            
            'check to ensure the entry's string data is null-terminated
            If Right(strEntryValue, 1) <> vbNullChar Then strEntryValue = (strEntryValue & vbNullChar)
            
            'form a buffer for the entry's string data to be passed the the api
            strBuffer = String(Len(strEntryValue), 0)
            strBuffer = strEntryValue
            
            'convert to upper case
            SubKey = UCase(SubKey)
            
            'check for backslash
            If Left(SubKey, 1) = "\" Then SubKey = Mid(SubKey, 2, Len(SubKey))
            
            'open the key with update value access; this should be all that is required to append an entry..
            nRet = RegOpenKeyEx(CLng(MainKey), SubKey, 0, KEY_SET_VALUE, nSubHandle)
            
            'check api return for success before continueing
            If nRet <> ERROR_SUCCESS Or nSubHandle = 0 Then Exit Function
            
            'set the new entry value to the key
            Registry_UpdateEntry = RegSetValueEx(nSubHandle, strEntry, 0, CLng(Format), ByVal strBuffer, Len(strBuffer) + 1)
            
            'close the key handle
            RegCloseKey (nSubHandle)
            
            'exit
            Exit Function
            
            
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Registry_QueryEntryValue
            ' * procedure description:   Returns the value of an entry; on error returns default
            ' *
            ' ******************************************************************************************************************************
            Public Function Registry_QueryEntryValue(MainKey As hKey, SubKey As String, strEntry As String, Optional Default As String = vbNullString) As String
            Dim nSubHandle As Long, nFileTime As FILETIME
            Dim nRet As Long, strBuffer As String, nMaxValueLen As Long
            On Local Error GoTo ErrLine
            
            'check to ensure subkey is valid
            If Len(SubKey) = 0 Then Exit Function
            
            'check to ensure the mainkey is valid
            If CLng(MainKey) = 0 Then Exit Function
            
            'Check to ensure the entry is valid
            If strEntry = vbNullString Then Exit Function
            
            'convert to upper case
            SubKey = UCase(SubKey)
            
            'check for backslash
            If Left(SubKey, 1) = "\" Then SubKey = Mid(SubKey, 2, Len(SubKey))
            
            'open the key and get a handle
            nRet = RegOpenKeyEx(CLng(MainKey), SubKey, 0, KEY_READ, nSubHandle)
            
            'check the api return
            If nRet <> ERROR_SUCCESS Or nSubHandle = 0 Then Exit Function
            
            'get the length of the largest given entry in the subkey so that we may be able to form a properly sized buffer
            nRet = RegQueryInfoKey(nSubHandle, vbNullString, 0, 0, 0, 0, 0, 0, 0, nMaxValueLen, 0, nFileTime)
            
            'set up a properly sized buffer given the known largest entry value size; set to MAX_PATH in case of last api failure
            If nMaxValueLen < 255 Then nMaxValueLen = 255
            strBuffer = String(nMaxValueLen, 0)
            
            'query the key for an entry value
            nMaxValueLen = Len(strBuffer) + 1
            nRet = RegQueryValueEx(nSubHandle, strEntry, 0, 0, ByVal strBuffer, nMaxValueLen)
            If nRet = ERROR_SUCCESS Then
                strBuffer = Left(strBuffer, nMaxValueLen)
                Registry_QueryEntryValue = strBuffer
            Else: Registry_QueryEntryValue = Default
            End If
            
            'close the handle, return the value and exit
            RegCloseKey (nSubHandle)
            Exit Function
            
            
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Registry_QueryEntryType
            ' * procedure description:   Returns the type of data an entry contains.
            ' *
            ' ******************************************************************************************************************************
            Public Function Registry_QueryEntryType(MainKey As hKey, SubKey As String, strEntry As String) As EntryFormat
            Dim nType As Long, nSubHandle As Long, nRet As Long
            On Local Error GoTo ErrLine
            'check to ensure subkey is valid
            If Len(SubKey) = 0 Then Exit Function
            
            'check to ensure the mainkey is valid
            If CLng(MainKey) = 0 Then Exit Function
            
            'Check to ensure the entry is valid
            If strEntry = vbNullString Then Exit Function
            
            'convert to upper case
            SubKey = UCase(SubKey)
            
            'check for backslash
            If Left(SubKey, 1) = "\" Then SubKey = Mid(SubKey, 2, Len(SubKey))
            
            'open the key for read access
            nRet = RegOpenKeyEx(CLng(MainKey), SubKey, 0, KEY_READ, nSubHandle)
            
            'check return on api call
            If nRet <> ERROR_SUCCESS Or nSubHandle = 0 Then Exit Function
            
            'query the entry in the key for any given type information
            nRet = RegQueryValueEx(nSubHandle, ByVal strEntry, 0, nType, 0, 0)
            
            Select Case nType
                  Case 0: Registry_QueryEntryType = REG_BINARY
                  Case 1: Registry_QueryEntryType = REG_DWORD
                  Case 2: Registry_QueryEntryType = REG_DWORD_BIG_ENDIAN
                  Case 3: Registry_QueryEntryType = REG_DWORD_LITTLE_ENDIAN
                  Case 4: Registry_QueryEntryType = REG_EXPAND_SZ
                  Case 5: Registry_QueryEntryType = REG_LINK
                  Case 6: Registry_QueryEntryType = REG_MULTI_SZ
                  Case 7: Registry_QueryEntryType = REG_NONE
                  Case 8: Registry_QueryEntryType = REG_RESOURCE_LIST
                  Case 9: Registry_QueryEntryType = REG_SZ
            End Select
            
            'exit
            Exit Function
                        
ErrLine:

            Err.Clear
            Exit Function
            End Function
            
            
            
            ' ******************************************************************************************************************************
            ' * procedure name: Registry_DoesEntryExist
            ' * procedure description:   Checks to ensure an entry does in fact exist
            ' *
            ' ******************************************************************************************************************************
            Public Function Registry_DoesEntryExist(MainKey As hKey, SubKey As String, strEntry As String) As Boolean
            Dim nType As Long, nSubHandle As Long, nRet As Long
            On Local Error GoTo ErrLine
            'check to ensure subkey is valid
            If Len(SubKey) = 0 Then Exit Function
            
            'check to ensure the mainkey is valid
            If CLng(MainKey) = 0 Then Exit Function
            
            'Check to ensure the entry is valid
            If strEntry = vbNullString Then Exit Function
            
            'convert to upper case
            SubKey = UCase(SubKey)
            
            'check for backslash
            If Left(SubKey, 1) = "\" Then SubKey = Mid(SubKey, 2, Len(SubKey))
            
            'open the key for read access
            nRet = RegOpenKeyEx(CLng(MainKey), SubKey, 0, KEY_READ, nSubHandle)
            
            'check return on api call
            If nRet <> ERROR_SUCCESS Or nSubHandle = 0 Then Exit Function
            
            'query the entry in the key for any given type information
            nRet = RegQueryValueEx(nSubHandle, ByVal strEntry, 0, nType, 0, 0)
            
            'verify api return
            If nRet > 0 Then
               Registry_DoesEntryExist = True
            Else
               Registry_DoesEntryExist = False
            End If
            
            'exit
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
            
            


' **************************************************************************************************************************************
' * PRIVATE INTERFACE- PROCEDURES
' *
' *
            ' ******************************************************************************************************************************
            ' * procedure name: Reg_KeyToStr
            ' * procedure description:   Returns a string denoteing the current key handle; this can be used later
            ' *                                        if you decide to extend remote registry access functionality into this module.
            ' ******************************************************************************************************************************
            Private Function Reg_KeyToStr(nKey As Long) As String
            On Local Error GoTo ErrLine
            
            Select Case nKey
                  Case HKEY_CLASSES_ROOT: Reg_KeyToStr = "HKEY_CLASSES_ROOT"
                  Case HKEY_CURRENT_CONFIG: Reg_KeyToStr = "HKEY_CURRENT_CONFIG"
                  Case HKEY_CURRENT_USER: Reg_KeyToStr = "HKEY_CURRENT_USER"
                  Case HKEY_LOCAL_MACHINE: Reg_KeyToStr = "HKEY_LOCAL_MACHINE"
                  Case HKEY_USERS: Reg_KeyToStr = "HKEY_USERS"
                  Case HKEY_DYN_DATA: Reg_KeyToStr = "HKEY_DYN_DATA"
                  Case HKEY_PERFORMANCE_DATA: Reg_KeyToStr = "HKEY_PERFORMANCE_DATA"
                  Case Else: Reg_KeyToStr = vbNullString
            End Select
            Exit Function
            
ErrLine:
            Err.Clear
            Exit Function
            End Function
