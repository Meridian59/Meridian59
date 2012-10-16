# Microsoft Developer Studio Generated NMAKE File, Based on multimon.dsp
!IF "$(CFG)" == ""
CFG=multimon - Win32 Debug
!MESSAGE No configuration specified. Defaulting to multimon - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "multimon - Win32 Release" && "$(CFG)" != "multimon - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "multimon.mak" CFG="multimon - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "multimon - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "multimon - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "multimon - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\multimon.exe"


CLEAN :
	-@erase "$(INTDIR)\multimon.obj"
	-@erase "$(INTDIR)\multimon.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\multimon.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\multimon.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\multimon.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\multimon.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dxguid.lib odbc32.lib odbccp32.lib dxerr8.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\multimon.pdb" /machine:I386 /out:"$(OUTDIR)\multimon.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\multimon.obj" \
	"$(INTDIR)\multimon.res"

"$(OUTDIR)\multimon.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "multimon - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\multimon.exe" "$(OUTDIR)\multimon.bsc"


CLEAN :
	-@erase "$(INTDIR)\multimon.obj"
	-@erase "$(INTDIR)\multimon.res"
	-@erase "$(INTDIR)\multimon.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\multimon.bsc"
	-@erase "$(OUTDIR)\multimon.exe"
	-@erase "$(OUTDIR)\multimon.ilk"
	-@erase "$(OUTDIR)\multimon.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\..\common\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\multimon.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\multimon.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\multimon.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\multimon.sbr"

"$(OUTDIR)\multimon.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=dxguid.lib odbc32.lib odbccp32.lib dxerr8.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\multimon.pdb" /debug /machine:I386 /out:"$(OUTDIR)\multimon.exe" /pdbtype:sept /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\multimon.obj" \
	"$(INTDIR)\multimon.res"

"$(OUTDIR)\multimon.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("multimon.dep")
!INCLUDE "multimon.dep"
!ELSE 
!MESSAGE Warning: cannot find "multimon.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "multimon - Win32 Release" || "$(CFG)" == "multimon - Win32 Debug"
SOURCE=.\multimon.cpp

!IF  "$(CFG)" == "multimon - Win32 Release"


"$(INTDIR)\multimon.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "multimon - Win32 Debug"


"$(INTDIR)\multimon.obj"	"$(INTDIR)\multimon.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\multimon.rc

"$(INTDIR)\multimon.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

