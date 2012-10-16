# Microsoft Developer Studio Project File - Name="diconfig" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=diconfig - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "diconfig.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "diconfig.mak" CFG="diconfig - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "diconfig - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "diconfig - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "diconfig - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "diconfig - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "diconfig - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DICONFIG_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /O2 /I "$(dxsdkroot)\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DICONFIG_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib uuid.lib dinput8.lib ddraw.lib d3d8.lib d3dx8.lib /nologo /dll /machine:I386 /libpath:"$(dxsdkroot)\lib" /stack:0x200000,0x200000

!ELSEIF  "$(CFG)" == "diconfig - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Win32_Debug"
# PROP BASE Intermediate_Dir "Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DICONFIG_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /Zi /Od /I "$(dxsdkroot)\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DICONFIG_EXPORTS" /D "DEBUG" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib uuid.lib dinput8.lib ddraw.lib d3d8.lib d3dx8.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"$(dxsdkroot)\lib" /stack:0x200000,0x200000

!ELSEIF  "$(CFG)" == "diconfig - Win32 Debug Unicode"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GR /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DICONFIG_EXPORTS" /D "DEBUG" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GR /Zi /Od /I "$(dxsdkroot)\include" /D "_MBCS" /D "_USRDLL" /D "DICONFIG_EXPORTS" /D "DEBUG" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib uuid.lib dinput8.lib ddraw.lib d3d8.lib d3dx8.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib uuid.lib dinput8.lib ddraw.lib d3d8.lib d3dx8.lib /nologo /dll /debug /machine:I386 /pdbtype:sept /libpath:"$(dxsdkroot)\lib" /stack:0x200000,0x200000

!ELSEIF  "$(CFG)" == "diconfig - Win32 Release Unicode"

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
# ADD BASE CPP /nologo /MT /W3 /GR /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DICONFIG_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GR /O2 /I "$(dxsdkroot)\include" /D "_MBCS" /D "_USRDLL" /D "DICONFIG_EXPORTS" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib uuid.lib dinput8.lib ddraw.lib d3d8.lib d3dx8.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib uuid.lib dinput8.lib ddraw.lib d3d8.lib d3dx8.lib /nologo /dll /machine:I386 /libpath:"$(dxsdkroot)\lib" /stack:0x200000,0x200000

!ENDIF 

# Begin Target

# Name "diconfig - Win32 Release"
# Name "diconfig - Win32 Debug"
# Name "diconfig - Win32 Debug Unicode"
# Name "diconfig - Win32 Release Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cbitmap.cpp
# End Source File
# Begin Source File

SOURCE=.\cd3dsurf.cpp
# End Source File
# Begin Source File

SOURCE=.\cdevicecontrol.cpp
# End Source File
# Begin Source File

SOURCE=.\cdeviceui.cpp
# End Source File
# Begin Source File

SOURCE=.\cdeviceview.cpp
# End Source File
# Begin Source File

SOURCE=.\cdeviceviewtext.cpp
# End Source File
# Begin Source File

SOURCE=.\cdiacpage.cpp
# End Source File
# Begin Source File

SOURCE=.\cfguitrace.cpp
# End Source File
# Begin Source File

SOURCE=.\cfrmwrk.cpp
# End Source File
# Begin Source File

SOURCE=.\configwnd.cpp
# End Source File
# Begin Source File

SOURCE=.\constants.cpp
# End Source File
# Begin Source File

SOURCE=.\cyclestr.cpp
# End Source File
# Begin Source File

SOURCE=.\dconfig.rc
# End Source File
# Begin Source File

SOURCE=.\diactfrm.def
# End Source File
# Begin Source File

SOURCE=.\flexcheckbox.cpp
# End Source File
# Begin Source File

SOURCE=.\flexcombobox.cpp
# End Source File
# Begin Source File

SOURCE=.\flexinfobox.cpp
# End Source File
# Begin Source File

SOURCE=.\flexlistbox.cpp
# End Source File
# Begin Source File

SOURCE=.\flexmsgbox.cpp
# End Source File
# Begin Source File

SOURCE=.\flexscrollbar.cpp
# End Source File
# Begin Source File

SOURCE=.\flextooltip.cpp
# End Source File
# Begin Source File

SOURCE=.\flextree.cpp
# End Source File
# Begin Source File

SOURCE=.\flexwnd.cpp
# End Source File
# Begin Source File

SOURCE=.\guids.c
# End Source File
# Begin Source File

SOURCE=.\iclassfact.cpp
# End Source File
# Begin Source File

SOURCE=.\ipageclassfact.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\populate.cpp
# End Source File
# Begin Source File

SOURCE=.\privcom.cpp
# End Source File
# Begin Source File

SOURCE=.\registry.cpp
# End Source File
# Begin Source File

SOURCE=.\selcontroldlg.cpp
# End Source File
# Begin Source File

SOURCE=.\uiglobals.cpp
# End Source File
# Begin Source File

SOURCE=.\usefuldi.cpp
# End Source File
# Begin Source File

SOURCE=.\viewselwnd.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\bidirlookup.h
# End Source File
# Begin Source File

SOURCE=.\cbitmap.h
# End Source File
# Begin Source File

SOURCE=.\cdevicecontrol.h
# End Source File
# Begin Source File

SOURCE=.\cdeviceui.h
# End Source File
# Begin Source File

SOURCE=.\cdeviceview.h
# End Source File
# Begin Source File

SOURCE=.\cdeviceviewtext.h
# End Source File
# Begin Source File

SOURCE=.\cdiacpage.h
# End Source File
# Begin Source File

SOURCE=.\cfguitrace.h
# End Source File
# Begin Source File

SOURCE=.\cfrmwrk.h
# End Source File
# Begin Source File

SOURCE=.\collections.h
# End Source File
# Begin Source File

SOURCE=.\configwnd.h
# End Source File
# Begin Source File

SOURCE=.\constants.h
# End Source File
# Begin Source File

SOURCE=.\cyclestr.h
# End Source File
# Begin Source File

SOURCE=.\defines.h
# End Source File
# Begin Source File

SOURCE=.\dicfgres.h
# End Source File
# Begin Source File

SOURCE=.\flexcheckbox.h
# End Source File
# Begin Source File

SOURCE=.\flexcombobox.h
# End Source File
# Begin Source File

SOURCE=.\flexinfobox.h
# End Source File
# Begin Source File

SOURCE=.\flexlistbox.h
# End Source File
# Begin Source File

SOURCE=.\flexmsg.h
# End Source File
# Begin Source File

SOURCE=.\flexmsgbox.h
# End Source File
# Begin Source File

SOURCE=.\flexscrollbar.h
# End Source File
# Begin Source File

SOURCE=.\flextooltip.h
# End Source File
# Begin Source File

SOURCE=.\flextree.h
# End Source File
# Begin Source File

SOURCE=.\flexwnd.h
# End Source File
# Begin Source File

SOURCE=.\iclassfact.h
# End Source File
# Begin Source File

SOURCE=.\id3dsurf.h
# End Source File
# Begin Source File

SOURCE=.\idiacpage.h
# End Source File
# Begin Source File

SOURCE=.\ifrmwrk.h
# End Source File
# Begin Source File

SOURCE=.\ipageclassfact.h
# End Source File
# Begin Source File

SOURCE=.\iuiframe.h
# End Source File
# Begin Source File

SOURCE=.\ltrace.h
# End Source File
# Begin Source File

SOURCE=.\main.h
# End Source File
# Begin Source File

SOURCE=.\ourguids.h
# End Source File
# Begin Source File

SOURCE=.\pagecommon.h
# End Source File
# Begin Source File

SOURCE=.\populate.h
# End Source File
# Begin Source File

SOURCE=.\privcom.h
# End Source File
# Begin Source File

SOURCE=.\registry.h
# End Source File
# Begin Source File

SOURCE=.\selcontroldlg.h
# End Source File
# Begin Source File

SOURCE=.\uielements.h
# End Source File
# Begin Source File

SOURCE=.\uiglobals.h
# End Source File
# Begin Source File

SOURCE=.\useful.h
# End Source File
# Begin Source File

SOURCE=.\usefuldi.h
# End Source File
# Begin Source File

SOURCE=.\viewselwnd.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
