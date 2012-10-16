# Microsoft Developer Studio Project File - Name="GargleDMO" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=GargleDMO - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GargleDMO.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GargleDMO.mak" CFG="GargleDMO - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GargleDMO - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GargleDMO - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GargleDMO - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Zi /Od /I "..\..\..\..\..\include" /I "medparambase" /I "controlbase" /I "dmobase" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "WIN32"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msdmo.lib dmoguids.lib strmiids.lib /nologo /stack:0x200000,0x200000 /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept /libpath:"..\..\..\..\..\lib"
# SUBTRACT LINK32 /nodefaultlib
# Begin Custom Build - Performing registration
OutDir=.\Debug
TargetPath=.\Debug\GargleDMO.dll
InputPath=.\Debug\GargleDMO.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "GargleDMO - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /Zi /Od /I "..\..\..\..\..\include" /I "..\dsdmobase\inc" /I "medparambase" /I "controlbase" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MD /W3 /O2 /I "..\..\..\..\..\include" /I "medparambase" /I "controlbase" /I "dmobase" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "WIN32"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msdmo.lib dmoguids.lib strmiids.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept /libpath:"..\..\..\..\..\lib" /libpath:"controlbase"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msdmo.lib dmoguids.lib strmiids.lib /nologo /stack:0x200000,0x200000 /subsystem:windows /dll /pdb:none /machine:I386 /libpath:"..\..\..\..\..\lib"
# SUBTRACT LINK32 /map /nodefaultlib
# Begin Custom Build - Performing registration
OutDir=.\Release
TargetPath=.\Release\GargleDMO.dll
InputPath=.\Release\GargleDMO.dll
SOURCE="$(InputPath)"

"$(OutDir)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
	
# End Custom Build

!ENDIF 

# Begin Target

# Name "GargleDMO - Win32 Debug"
# Name "GargleDMO - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\GargDMOProp.cpp

!IF  "$(CFG)" == "GargleDMO - Win32 Debug"

!ELSEIF  "$(CFG)" == "GargleDMO - Win32 Release"

# ADD CPP /D "NDEBUG"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Gargle.cpp

!IF  "$(CFG)" == "GargleDMO - Win32 Debug"

!ELSEIF  "$(CFG)" == "GargleDMO - Win32 Release"

# ADD CPP /D "NDEBUG"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GargleDMO.cpp

!IF  "$(CFG)" == "GargleDMO - Win32 Debug"

!ELSEIF  "$(CFG)" == "GargleDMO - Win32 Release"

# ADD CPP /D "NDEBUG"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\GargleDMO.def
# End Source File
# Begin Source File

SOURCE=.\GargleDMO.rc
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "GargleDMO - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

!ELSEIF  "$(CFG)" == "GargleDMO - Win32 Release"

# ADD CPP /D "NDEBUG" /Yc"stdafx.h"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\GargDMOProp.h
# End Source File
# Begin Source File

SOURCE=.\Gargle.h
# End Source File
# Begin Source File

SOURCE=.\igargle.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\GargDMOProp.rgs
# End Source File
# Begin Source File

SOURCE=.\Gargle.rgs
# End Source File
# End Group
# End Target
# End Project
# Section GargleDMO : {00000000-0000-0000-0000-800000800000}
# 	1:15:IDR_GARGDMOPROP:106
# 	1:20:IDS_TITLEGargDMOProp:103
# 	1:15:IDD_GARGDMOPROP:107
# 	1:24:IDS_DOCSTRINGGargDMOProp:105
# 	1:23:IDS_HELPFILEGargDMOProp:104
# End Section
