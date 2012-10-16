# Microsoft Developer Studio Project File - Name="duelvoice" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=duelvoice - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "duelvoice.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "duelvoice.mak" CFG="duelvoice - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "duelvoice - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "duelvoice - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "duelvoice - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 uuid.lib comctl32.lib ddraw.lib dinput.lib dsound.lib dplayx.lib dxguid.lib dxerr8.lib ole32.lib winmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /machine:I386 /stack:0x1f4000,0x1f4000
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "duelvoice - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "..\..\common\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 uuid.lib comctl32.lib ddraw.lib dinput.lib dsound.lib dplayx.lib dxguid.lib dxerr8.lib ole32.lib winmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /debug /machine:I386 /stack:0x1f4000,0x1f4000
# SUBTRACT LINK32 /nodefaultlib

!ENDIF 

# Begin Target

# Name "duelvoice - Win32 Release"
# Name "duelvoice - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\ddutil.cpp
# End Source File
# Begin Source File

SOURCE=.\ddutil.h
# End Source File
# Begin Source File

SOURCE=.\diutil.cpp
# End Source File
# Begin Source File

SOURCE=.\diutil.h
# End Source File
# Begin Source File

SOURCE=.\dpconnect.cpp
# End Source File
# Begin Source File

SOURCE=.\dputil.cpp
# End Source File
# Begin Source File

SOURCE=.\dputil.h
# End Source File
# Begin Source File

SOURCE=.\dsutil.cpp
# End Source File
# Begin Source File

SOURCE=.\dsutil.h
# End Source File
# Begin Source File

SOURCE=.\duel.cpp
# End Source File
# Begin Source File

SOURCE=.\duel.h
# End Source File
# Begin Source File

SOURCE=.\duelvoice.rc
# End Source File
# Begin Source File

SOURCE=.\gameproc.cpp
# End Source File
# Begin Source File

SOURCE=.\gameproc.h
# End Source File
# Begin Source File

SOURCE=.\gfx.cpp
# End Source File
# Begin Source File

SOURCE=.\gfx.h
# End Source File
# Begin Source File

SOURCE=.\lobby.cpp
# End Source File
# Begin Source File

SOURCE=.\lobby.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\util.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\csession.bmp
# End Source File
# Begin Source File

SOURCE=.\DUEL.BMP
# End Source File
# Begin Source File

SOURCE=.\duel.ico
# End Source File
# Begin Source File

SOURCE=.\osession.bmp
# End Source File
# Begin Source File

SOURCE=.\player.bmp
# End Source File
# Begin Source File

SOURCE=.\SPLASH.BMP
# End Source File
# End Group
# Begin Source File

SOURCE=.\Bfire.wav
# End Source File
# Begin Source File

SOURCE=.\Lboom.wav
# End Source File
# Begin Source File

SOURCE=.\Sboom.wav
# End Source File
# Begin Source File

SOURCE=.\Sbounce.wav
# End Source File
# Begin Source File

SOURCE=.\Sengine.wav
# End Source File
# Begin Source File

SOURCE=.\Sstart.wav
# End Source File
# Begin Source File

SOURCE=.\Sstop.wav
# End Source File
# End Target
# End Project
