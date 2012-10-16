# Microsoft Developer Studio Project File - Name="SimpleClient" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=SimpleClient - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "simpleclient.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "simpleclient.mak" CFG="SimpleClient - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SimpleClient - Win32 Debug Unicode" (based on "Win32 (x86) Application")
!MESSAGE "SimpleClient - Win32 Release Unicode" (based on "Win32 (x86) Application")
!MESSAGE "SimpleClient - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "SimpleClient - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SimpleClient - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\common\include" /I "..\common" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "WIN32" /D "_WIN32_DCOM" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /machine:I386 /stack:0x10000,0x10000

!ELSEIF  "$(CFG)" == "SimpleClient - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\common\include" /I "..\common" /D "_WINDOWS" /D "_DEBUG" /D "WIN32" /D "_WIN32_DCOM" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBC" /pdbtype:sept /stack:0x10000,0x10000
# SUBTRACT LINK32 /incremental:no /nodefaultlib

!ELSEIF  "$(CFG)" == "SimpleClient - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Win32_Debug_Unicode"
# PROP Intermediate_Dir "Win32_Debug_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\common\include" /I "..\common" /D "_WINDOWS" /D "_DEBUG" /D "WIN32" /D "_WIN32_DCOM" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\common\include" /I "..\common" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "_WIN32_DCOM" /D "UNICODE" /D "_UNICODE" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBC" /pdbtype:sept
# SUBTRACT BASE LINK32 /incremental:no /nodefaultlib
# ADD LINK32 dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBC" /pdbtype:sept /stack:0x10000,0x10000
# SUBTRACT LINK32 /incremental:no /nodefaultlib

!ELSEIF  "$(CFG)" == "SimpleClient - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Win32_Release_Unicode"
# PROP Intermediate_Dir "Win32_Release_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\common\include" /I "..\common" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "WIN32" /D "_WIN32_DCOM" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\common\include" /I "..\common" /D "NDEBUG" /D "_MBCS" /D "_WINDOWS" /D "WIN32" /D "_WIN32_DCOM" /D "UNICODE" /D "_UNICODE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /machine:I386 /stack:0x10000,0x10000

!ENDIF 

# Begin Target

# Name "SimpleClient - Win32 Release"
# Name "SimpleClient - Win32 Debug"
# Name "SimpleClient - Win32 Debug Unicode"
# Name "SimpleClient - Win32 Release Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\SimpleClient.cpp
# End Source File
# Begin Source File

SOURCE=..\common\simpleclientserver.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\directx.ico
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\SimpleClient.rc
# End Source File
# End Group
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\common\src\dxutil.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\include\dxutil.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\netclient.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\include\netclient.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\include\netclientres.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\readme.txt
# End Source File
# End Target
# End Project
