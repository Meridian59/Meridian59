# Microsoft Developer Studio Generated NMAKE File, Based on GargleDMO.dsp
!IF "$(CFG)" == ""
CFG=GargleDMO - Win32 Release
!MESSAGE No configuration specified. Defaulting to GargleDMO - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "GargleDMO - Win32 Debug" && "$(CFG)" != "GargleDMO - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "GargleDMO - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\GargleDMO.dll" ".\Debug\regsvr32.trg"

!ELSE 

ALL : "ControlBase - Win32 Debug" "MedParamBase - Win32 Debug" "$(OUTDIR)\GargleDMO.dll" ".\Debug\regsvr32.trg"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"MedParamBase - Win32 DebugCLEAN" "ControlBase - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\GargDMOProp.obj"
	-@erase "$(INTDIR)\Gargle.obj"
	-@erase "$(INTDIR)\GargleDMO.obj"
	-@erase "$(INTDIR)\GargleDMO.pch"
	-@erase "$(INTDIR)\GargleDMO.res"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\GargleDMO.dll"
	-@erase "$(OUTDIR)\GargleDMO.exp"
	-@erase "$(OUTDIR)\GargleDMO.ilk"
	-@erase "$(OUTDIR)\GargleDMO.lib"
	-@erase "$(OUTDIR)\GargleDMO.pdb"
	-@erase ".\Debug\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /Zi /Od /I "..\..\..\..\..\include" /I "medparambase" /I "controlbase" /I "dmobase" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\GargleDMO.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\GargleDMO.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\GargleDMO.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msdmo.lib dmoguids.lib strmiids.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\GargleDMO.pdb" /debug /machine:I386 /def:".\GargleDMO.def" /out:"$(OUTDIR)\GargleDMO.dll" /implib:"$(OUTDIR)\GargleDMO.lib" /pdbtype:sept /libpath:"..\..\..\..\..\lib" /stack:0x200000,0x200000
DEF_FILE= \
	".\GargleDMO.def"
LINK32_OBJS= \
	"$(INTDIR)\GargDMOProp.obj" \
	"$(INTDIR)\Gargle.obj" \
	"$(INTDIR)\GargleDMO.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\GargleDMO.res" \
	".\MedParamBase\Debug\MedParamBase.lib" \
	".\ControlBase\Debug\ControlBase.lib"

"$(OUTDIR)\GargleDMO.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\Debug
TargetPath=.\Debug\GargleDMO.dll
InputPath=.\Debug\GargleDMO.dll
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
<< 
	

!ELSEIF  "$(CFG)" == "GargleDMO - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\GargleDMO.dll" ".\Release\regsvr32.trg"

!ELSE 

ALL : "ControlBase - Win32 Release" "MedParamBase - Win32 Release" "$(OUTDIR)\GargleDMO.dll" ".\Release\regsvr32.trg"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"MedParamBase - Win32 ReleaseCLEAN" "ControlBase - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\GargDMOProp.obj"
	-@erase "$(INTDIR)\Gargle.obj"
	-@erase "$(INTDIR)\GargleDMO.obj"
	-@erase "$(INTDIR)\GargleDMO.pch"
	-@erase "$(INTDIR)\GargleDMO.res"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\GargleDMO.dll"
	-@erase "$(OUTDIR)\GargleDMO.exp"
	-@erase "$(OUTDIR)\GargleDMO.lib"
	-@erase ".\Release\regsvr32.trg"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Od /I "..\..\..\..\..\include" /I "medparambase" /I "controlbase" /I "dmobase" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\GargleDMO.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\GargleDMO.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\GargleDMO.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msdmo.lib dmoguids.lib strmiids.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\GargleDMO.pdb" /machine:I386 /def:".\GargleDMO.def" /out:"$(OUTDIR)\GargleDMO.dll" /implib:"$(OUTDIR)\GargleDMO.lib" /pdbtype:sept /libpath:"..\..\..\..\..\lib" /stack:0x200000,0x200000
DEF_FILE= \
	".\GargleDMO.def"
LINK32_OBJS= \
	"$(INTDIR)\GargDMOProp.obj" \
	"$(INTDIR)\Gargle.obj" \
	"$(INTDIR)\GargleDMO.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\GargleDMO.res" \
	".\MedParamBase\Release\MedParamBase.lib" \
	".\ControlBase\Release\ControlBase.lib"

"$(OUTDIR)\GargleDMO.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

OutDir=.\Release
TargetPath=.\Release\GargleDMO.dll
InputPath=.\Release\GargleDMO.dll
SOURCE="$(InputPath)"

"$(OUTDIR)\regsvr32.trg" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	regsvr32 /s /c "$(TargetPath)" 
	echo regsvr32 exec. time > "$(OutDir)\regsvr32.trg" 
<< 
	

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("GargleDMO.dep")
!INCLUDE "GargleDMO.dep"
!ELSE 
!MESSAGE Warning: cannot find "GargleDMO.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "GargleDMO - Win32 Debug" || "$(CFG)" == "GargleDMO - Win32 Release"
SOURCE=.\GargDMOProp.cpp

!IF  "$(CFG)" == "GargleDMO - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /Zi /Od /I "..\..\..\..\..\include" /I "medparambase" /I "controlbase" /I "dmobase" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\GargleDMO.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\GargDMOProp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GargleDMO.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GargleDMO - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Od /I "..\..\..\..\..\include" /I "medparambase" /I "controlbase" /I "dmobase" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NDEBUG" /Fp"$(INTDIR)\GargleDMO.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\GargDMOProp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GargleDMO.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\Gargle.cpp

!IF  "$(CFG)" == "GargleDMO - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /Zi /Od /I "..\..\..\..\..\include" /I "medparambase" /I "controlbase" /I "dmobase" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\GargleDMO.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Gargle.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GargleDMO.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GargleDMO - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Od /I "..\..\..\..\..\include" /I "medparambase" /I "controlbase" /I "dmobase" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NDEBUG" /Fp"$(INTDIR)\GargleDMO.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\Gargle.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GargleDMO.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\GargleDMO.cpp

!IF  "$(CFG)" == "GargleDMO - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /Zi /Od /I "..\..\..\..\..\include" /I "medparambase" /I "controlbase" /I "dmobase" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\GargleDMO.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\GargleDMO.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GargleDMO.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GargleDMO - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Od /I "..\..\..\..\..\include" /I "medparambase" /I "controlbase" /I "dmobase" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NDEBUG" /Fp"$(INTDIR)\GargleDMO.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\GargleDMO.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GargleDMO.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\GargleDMO.rc

"$(INTDIR)\GargleDMO.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "GargleDMO - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /Zi /Od /I "..\..\..\..\..\include" /I "medparambase" /I "controlbase" /I "dmobase" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fp"$(INTDIR)\GargleDMO.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\GargleDMO.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GargleDMO - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Od /I "..\..\..\..\..\include" /I "medparambase" /I "controlbase" /I "dmobase" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NDEBUG" /Fp"$(INTDIR)\GargleDMO.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\GargleDMO.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

!IF  "$(CFG)" == "GargleDMO - Win32 Debug"

"MedParamBase - Win32 Debug" : 
   cd ".\MedParamBase"
   $(MAKE) /$(MAKEFLAGS) /F .\MedParamBase.mak CFG="MedParamBase - Win32 Debug" 
   cd ".."

"MedParamBase - Win32 DebugCLEAN" : 
   cd ".\MedParamBase"
   $(MAKE) /$(MAKEFLAGS) /F .\MedParamBase.mak CFG="MedParamBase - Win32 Debug" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "GargleDMO - Win32 Release"

"MedParamBase - Win32 Release" : 
   cd ".\MedParamBase"
   $(MAKE) /$(MAKEFLAGS) /F .\MedParamBase.mak CFG="MedParamBase - Win32 Release" 
   cd ".."

"MedParamBase - Win32 ReleaseCLEAN" : 
   cd ".\MedParamBase"
   $(MAKE) /$(MAKEFLAGS) /F .\MedParamBase.mak CFG="MedParamBase - Win32 Release" RECURSE=1 CLEAN 
   cd ".."

!ENDIF 

!IF  "$(CFG)" == "GargleDMO - Win32 Debug"

"ControlBase - Win32 Debug" : 
   cd ".\ControlBase"
   $(MAKE) /$(MAKEFLAGS) /F .\ControlBase.mak CFG="ControlBase - Win32 Debug" 
   cd ".."

"ControlBase - Win32 DebugCLEAN" : 
   cd ".\ControlBase"
   $(MAKE) /$(MAKEFLAGS) /F .\ControlBase.mak CFG="ControlBase - Win32 Debug" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "GargleDMO - Win32 Release"

"ControlBase - Win32 Release" : 
   cd ".\ControlBase"
   $(MAKE) /$(MAKEFLAGS) /F .\ControlBase.mak CFG="ControlBase - Win32 Release" 
   cd ".."

"ControlBase - Win32 ReleaseCLEAN" : 
   cd ".\ControlBase"
   $(MAKE) /$(MAKEFLAGS) /F .\ControlBase.mak CFG="ControlBase - Win32 Release" RECURSE=1 CLEAN 
   cd ".."

!ENDIF 


!ENDIF 

