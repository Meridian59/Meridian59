# Microsoft Developer Studio Project File - Name="Donuts3D" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Donuts3D - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "donuts3d.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "donuts3d.mak" CFG="Donuts3D - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Donuts3D - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Donuts3D - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Donuts3D - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 d3dx8.lib d3d8.lib d3dxof.lib dxguid.lib dinput8.lib dsound.lib winspool.lib dxerr8.lib ole32.lib winmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /machine:I386 /stack:0x1f4000,0x1f4000

!ELSEIF  "$(CFG)" == "Donuts3D - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\..\common\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 d3dx8dt.lib d3d8.lib d3dxof.lib dxguid.lib dinput8.lib dsound.lib winspool.lib dxerr8.lib ole32.lib winmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /stack:0x1f4000,0x1f4000

!ENDIF 

# Begin Target

# Name "Donuts3D - Win32 Release"
# Name "Donuts3D - Win32 Debug"
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\bangbang.wav
# End Source File
# Begin Source File

SOURCE=.\bounce.wav
# End Source File
# Begin Source File

SOURCE=.\c_bang.wav
# End Source File
# Begin Source File

SOURCE=.\d_bang.wav
# End Source File
# Begin Source File

SOURCE=.\donuts.bmp
# End Source File
# Begin Source File

SOURCE=.\donuts.ico
# End Source File
# Begin Source File

SOURCE=.\donuts.rc
# End Source File
# Begin Source File

SOURCE=.\gunfire.wav
# End Source File
# Begin Source File

SOURCE=.\hum.wav
# End Source File
# Begin Source File

SOURCE=.\level.wav
# End Source File
# Begin Source File

SOURCE=.\p_bang.wav
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\rev.wav
# End Source File
# Begin Source File

SOURCE=.\s_bang.wav
# End Source File
# Begin Source File

SOURCE=.\shield.wav
# End Source File
# Begin Source File

SOURCE=.\skid.wav
# End Source File
# Begin Source File

SOURCE=.\splash.bmp
# End Source File
# End Group
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\common\src\d3dfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\include\d3dfile.h
# End Source File
# Begin Source File

SOURCE=..\..\common\src\d3dfont.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\include\d3dfont.h
# End Source File
# Begin Source File

SOURCE=..\..\common\src\d3dutil.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\src\diutil.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\include\diutil.h
# End Source File
# Begin Source File

SOURCE=..\..\common\src\dmutil.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\include\dmutil.h
# End Source File
# Begin Source File

SOURCE=..\..\common\src\dxutil.cpp
# End Source File
# Begin Source File

SOURCE=..\..\common\include\dxutil.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\donuts.cpp
# End Source File
# Begin Source File

SOURCE=.\donuts.h
# End Source File
# Begin Source File

SOURCE=.\gamemenu.cpp
# End Source File
# Begin Source File

SOURCE=.\gamemenu.h
# End Source File
# End Target
# End Project
