# Microsoft Developer Studio Project File - Name="MazeClient" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=MazeClient - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MazeClient.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MazeClient.mak" CFG="MazeClient - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MazeClient - Win32 Debug Unicode" (based on "Win32 (x86) Application")
!MESSAGE "MazeClient - Win32 Release Unicode" (based on "Win32 (x86) Application")
!MESSAGE "MazeClient - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "MazeClient - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MazeClient - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\common\include" /I "..\mazecommon" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "WIN32" /D "_WIN32_DCOM" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 d3dx8dt.lib d3d8.lib d3dxof.lib dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /machine:I386 /stack:0x10000,0x10000

!ELSEIF  "$(CFG)" == "MazeClient - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\common\include" /I "..\mazecommon" /D "_WINDOWS" /D "_DEBUG" /D "WIN32" /D "_WIN32_DCOM" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 d3dx8dt.lib d3d8.lib d3dxof.lib dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBC" /pdbtype:sept /stack:0x10000,0x10000
# SUBTRACT LINK32 /incremental:no /nodefaultlib

!ELSEIF  "$(CFG)" == "MazeClient - Win32 Debug Unicode"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\common\include" /I "..\mazecommon" /D "_WINDOWS" /D "_DEBUG" /D "WIN32" /D "_WIN32_DCOM" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\common\include" /I "..\mazecommon" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "_WIN32_DCOM" /D "UNICODE" /D "_UNICODE" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 d3dx8dt.lib d3d8.lib d3dxof.lib dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBC" /pdbtype:sept
# SUBTRACT BASE LINK32 /incremental:no /nodefaultlib
# ADD LINK32 d3dx8dt.lib d3d8.lib d3dxof.lib dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBC" /pdbtype:sept /stack:0x10000,0x10000
# SUBTRACT LINK32 /incremental:no /nodefaultlib

!ELSEIF  "$(CFG)" == "MazeClient - Win32 Release Unicode"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\common\include" /I "..\mazecommon" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "WIN32" /D "_WIN32_DCOM" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\..\common\include" /I "..\mazecommon" /D "NDEBUG" /D "_MBCS" /D "_WINDOWS" /D "WIN32" /D "_WIN32_DCOM" /D "UNICODE" /D "_UNICODE" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ddraw.lib d3dx.lib dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 d3dx8dt.lib d3d8.lib d3dxof.lib dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /machine:I386 /stack:0x10000,0x10000

!ENDIF 

# Begin Target

# Name "MazeClient - Win32 Release"
# Name "MazeClient - Win32 Debug"
# Name "MazeClient - Win32 Debug Unicode"
# Name "MazeClient - Win32 Release Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\D3DGraphics.cpp
# End Source File
# Begin Source File

SOURCE=.\D3DGraphics.h
# End Source File
# Begin Source File

SOURCE=.\Main.cpp
# End Source File
# Begin Source File

SOURCE=.\SmartVB.h
# End Source File
# End Group
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\common\src\d3dfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\include\d3dfile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\d3dfont.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\include\d3dfont.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\d3dsaver.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\include\d3dsaver.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\d3dutil.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\include\d3dutil.h
# End Source File
# Begin Source File

SOURCE=..\mazecommon\dplay8client.cpp
# End Source File
# Begin Source File

SOURCE=..\mazecommon\dplay8client.h
# End Source File
# Begin Source File

SOURCE=..\MazeCommon\DummyConnector.h
# End Source File
# Begin Source File

SOURCE=..\..\..\common\src\dxutil.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\common\include\dxutil.h
# End Source File
# Begin Source File

SOURCE=..\MazeCommon\imazegraphics.h
# End Source File
# Begin Source File

SOURCE=..\mazecommon\Maze.cpp
# End Source File
# Begin Source File

SOURCE=..\mazecommon\Maze.h
# End Source File
# Begin Source File

SOURCE=..\MazeCommon\MazeApp.cpp
# End Source File
# Begin Source File

SOURCE=..\MazeCommon\MazeApp.h
# End Source File
# Begin Source File

SOURCE=..\mazecommon\MazeClient.cpp
# End Source File
# Begin Source File

SOURCE=..\mazecommon\MazeClient.h
# End Source File
# Begin Source File

SOURCE=.\MazeGraphics.h
# End Source File
# Begin Source File

SOURCE=..\mazecommon\MazeServer.cpp
# End Source File
# Begin Source File

SOURCE=..\mazecommon\MazeServer.h
# End Source File
# Begin Source File

SOURCE=..\mazecommon\NetAbstract.h
# End Source File
# Begin Source File

SOURCE=..\mazecommon\Packets.h
# End Source File
# Begin Source File

SOURCE=..\mazecommon\Random.h
# End Source File
# Begin Source File

SOURCE=..\mazecommon\SimpleStack.h
# End Source File
# Begin Source File

SOURCE=..\mazecommon\StressMazeGuid.h
# End Source File
# Begin Source File

SOURCE=..\mazecommon\SyncObjects.h
# End Source File
# Begin Source File

SOURCE=..\mazecommon\Trig.h
# End Source File
# Begin Source File

SOURCE=..\mazecommon\Vector2.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CeilingTexture.bmp
# End Source File
# Begin Source File

SOURCE=.\DirectX.ico
# End Source File
# Begin Source File

SOURCE=.\FloorTexture.bmp
# End Source File
# Begin Source File

SOURCE=.\Font.bmp
# End Source File
# Begin Source File

SOURCE=.\LocalIcon.bmp
# End Source File
# Begin Source File

SOURCE=.\mazeclient.rc
# End Source File
# Begin Source File

SOURCE=.\NetIcon.bmp
# End Source File
# Begin Source File

SOURCE=.\PlayerIcon.bmp
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\WallTexture.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=..\readme.txt
# End Source File
# Begin Source File

SOURCE=.\tank.x
# End Source File
# End Target
# End Project
