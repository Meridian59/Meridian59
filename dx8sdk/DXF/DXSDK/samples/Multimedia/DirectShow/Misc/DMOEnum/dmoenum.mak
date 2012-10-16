# Microsoft Developer Studio Generated NMAKE File, Based on DMOEnum.dsp
!IF "$(CFG)" == ""
CFG=DMOEnum - Win32 Debug
!MESSAGE No configuration specified. Defaulting to DMOEnum - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "DMOEnum - Win32 Release" && "$(CFG)" != "DMOEnum - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DMOEnum.mak" CFG="DMOEnum - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DMOEnum - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "DMOEnum - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "DMOEnum - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\DMOEnum.exe"


CLEAN :
	-@erase "$(INTDIR)\DMOEnum.obj"
	-@erase "$(INTDIR)\DMOEnum.pch"
	-@erase "$(INTDIR)\DMOEnum.res"
	-@erase "$(INTDIR)\DMOEnumDlg.obj"
	-@erase "$(INTDIR)\namedguid.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\DMOEnum.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\DMOEnum.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\DMOEnum.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DMOEnum.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=strmiids.lib msdmo.lib dmoguids.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\DMOEnum.pdb" /machine:I386 /out:"$(OUTDIR)\DMOEnum.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\DMOEnum.obj" \
	"$(INTDIR)\DMOEnumDlg.obj" \
	"$(INTDIR)\namedguid.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\DMOEnum.res"

"$(OUTDIR)\DMOEnum.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "DMOEnum - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\DMOEnum.exe"


CLEAN :
	-@erase "$(INTDIR)\DMOEnum.obj"
	-@erase "$(INTDIR)\DMOEnum.pch"
	-@erase "$(INTDIR)\DMOEnum.res"
	-@erase "$(INTDIR)\DMOEnumDlg.obj"
	-@erase "$(INTDIR)\namedguid.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\DMOEnum.exe"
	-@erase "$(OUTDIR)\DMOEnum.ilk"
	-@erase "$(OUTDIR)\DMOEnum.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /Gi /GX /Zi /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /I "..\..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\DMOEnum.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\DMOEnum.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DMOEnum.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=strmiids.lib msdmo.lib dmoguids.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\DMOEnum.pdb" /debug /machine:I386 /out:"$(OUTDIR)\DMOEnum.exe" /pdbtype:sept /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\DMOEnum.obj" \
	"$(INTDIR)\DMOEnumDlg.obj" \
	"$(INTDIR)\namedguid.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\DMOEnum.res"

"$(OUTDIR)\DMOEnum.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("DMOEnum.dep")
!INCLUDE "DMOEnum.dep"
!ELSE 
!MESSAGE Warning: cannot find "DMOEnum.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "DMOEnum - Win32 Release" || "$(CFG)" == "DMOEnum - Win32 Debug"
SOURCE=.\DMOEnum.cpp

"$(INTDIR)\DMOEnum.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\DMOEnum.pch"


SOURCE=.\DMOEnum.rc

"$(INTDIR)\DMOEnum.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\DMOEnumDlg.cpp

"$(INTDIR)\DMOEnumDlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\DMOEnum.pch"


SOURCE=..\..\Common\namedguid.cpp

"$(INTDIR)\namedguid.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\DMOEnum.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "DMOEnum - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\DMOEnum.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\DMOEnum.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "DMOEnum - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /Gi /GX /Zi /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /I "..\..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\DMOEnum.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\DMOEnum.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

