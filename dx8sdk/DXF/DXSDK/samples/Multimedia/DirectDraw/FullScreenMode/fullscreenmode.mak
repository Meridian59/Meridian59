# Microsoft Developer Studio Generated NMAKE File, Based on fullscreenmode.dsp
!IF "$(CFG)" == ""
CFG=FullScreenMode - Win32 Release
!MESSAGE No configuration specified. Defaulting to FullScreenMode - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "FullScreenMode - Win32 Release" && "$(CFG)" != "FullScreenMode - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fullscreenmode.mak" CFG="FullScreenMode - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FullScreenMode - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "FullScreenMode - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "FullScreenMode - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\fullscreenmode.exe"


CLEAN :
	-@erase "$(INTDIR)\ddutil.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\FullScreenMode.obj"
	-@erase "$(INTDIR)\FullScreenMode.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\fullscreenmode.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\fullscreenmode.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\FullScreenMode.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\fullscreenmode.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=winmm.lib dxguid.lib dxerr8.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\fullscreenmode.pdb" /machine:I386 /out:"$(OUTDIR)\fullscreenmode.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\FullScreenMode.obj" \
	"$(INTDIR)\ddutil.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\FullScreenMode.res"

"$(OUTDIR)\fullscreenmode.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "FullScreenMode - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\fullscreenmode.exe"


CLEAN :
	-@erase "$(INTDIR)\ddutil.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\FullScreenMode.obj"
	-@erase "$(INTDIR)\FullScreenMode.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\fullscreenmode.exe"
	-@erase "$(OUTDIR)\fullscreenmode.ilk"
	-@erase "$(OUTDIR)\fullscreenmode.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\..\common\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\fullscreenmode.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\FullScreenMode.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\fullscreenmode.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=winmm.lib dxguid.lib dxerr8.lib ddraw.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\fullscreenmode.pdb" /debug /machine:I386 /out:"$(OUTDIR)\fullscreenmode.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\FullScreenMode.obj" \
	"$(INTDIR)\ddutil.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\FullScreenMode.res"

"$(OUTDIR)\fullscreenmode.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("fullscreenmode.dep")
!INCLUDE "fullscreenmode.dep"
!ELSE 
!MESSAGE Warning: cannot find "fullscreenmode.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "FullScreenMode - Win32 Release" || "$(CFG)" == "FullScreenMode - Win32 Debug"
SOURCE=.\FullScreenMode.cpp

"$(INTDIR)\FullScreenMode.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\FullScreenMode.rc

"$(INTDIR)\FullScreenMode.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=..\..\common\src\ddutil.cpp

"$(INTDIR)\ddutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\src\dxutil.cpp

"$(INTDIR)\dxutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

