# Microsoft Developer Studio Generated NMAKE File, Based on audiofx.dsp
!IF "$(CFG)" == ""
CFG=audiofx - Win32 Debug
!MESSAGE No configuration specified. Defaulting to audiofx - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "audiofx - Win32 Release" && "$(CFG)" != "audiofx - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "audiofx.mak" CFG="audiofx - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "audiofx - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "audiofx - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "audiofx - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\audiofx.exe"


CLEAN :
	-@erase "$(INTDIR)\audiofx.obj"
	-@erase "$(INTDIR)\audiofx.res"
	-@erase "$(INTDIR)\dmutil.obj"
	-@erase "$(INTDIR)\dsutil.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\audiofx.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\audiofx.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\audiofx.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\audiofx.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dxguid.lib dxerr8.lib comctl32.lib winmm.lib dsound.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\audiofx.pdb" /machine:I386 /out:"$(OUTDIR)\audiofx.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\audiofx.obj" \
	"$(INTDIR)\dmutil.obj" \
	"$(INTDIR)\dsutil.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\audiofx.res"

"$(OUTDIR)\audiofx.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "audiofx - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\audiofx.exe"


CLEAN :
	-@erase "$(INTDIR)\audiofx.obj"
	-@erase "$(INTDIR)\audiofx.res"
	-@erase "$(INTDIR)\dmutil.obj"
	-@erase "$(INTDIR)\dsutil.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\audiofx.exe"
	-@erase "$(OUTDIR)\audiofx.ilk"
	-@erase "$(OUTDIR)\audiofx.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\..\common\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\audiofx.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\audiofx.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\audiofx.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dxguid.lib dxerr8.lib comctl32.lib winmm.lib dsound.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\audiofx.pdb" /debug /machine:I386 /out:"$(OUTDIR)\audiofx.exe" /pdbtype:sept /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\audiofx.obj" \
	"$(INTDIR)\dmutil.obj" \
	"$(INTDIR)\dsutil.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\audiofx.res"

"$(OUTDIR)\audiofx.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("audiofx.dep")
!INCLUDE "audiofx.dep"
!ELSE 
!MESSAGE Warning: cannot find "audiofx.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "audiofx - Win32 Release" || "$(CFG)" == "audiofx - Win32 Debug"
SOURCE=.\audiofx.cpp

"$(INTDIR)\audiofx.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\audiofx.rc

"$(INTDIR)\audiofx.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=..\..\common\src\dmutil.cpp

"$(INTDIR)\dmutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\src\dsutil.cpp

"$(INTDIR)\dsutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\src\dxutil.cpp

"$(INTDIR)\dxutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

