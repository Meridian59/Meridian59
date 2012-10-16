# Microsoft Developer Studio Generated NMAKE File, Based on PlayMotif.dsp
!IF "$(CFG)" == ""
CFG=PlayMotif - Win32 Debug
!MESSAGE No configuration specified. Defaulting to PlayMotif - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "PlayMotif - Win32 Release" && "$(CFG)" != "PlayMotif - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PlayMotif.mak" CFG="PlayMotif - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PlayMotif - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PlayMotif - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "PlayMotif - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\PlayMotif.exe"


CLEAN :
	-@erase "$(INTDIR)\dmutil.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\PlayMotif.obj"
	-@erase "$(INTDIR)\PlayMotif.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\PlayMotif.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\PlayMotif.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\PlayMotif.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PlayMotif.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dxerr8.lib winmm.lib dxguid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\PlayMotif.pdb" /machine:I386 /out:"$(OUTDIR)\PlayMotif.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\PlayMotif.obj" \
	"$(INTDIR)\dmutil.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\PlayMotif.res"

"$(OUTDIR)\PlayMotif.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PlayMotif - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\PlayMotif.exe"


CLEAN :
	-@erase "$(INTDIR)\dmutil.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\PlayMotif.obj"
	-@erase "$(INTDIR)\PlayMotif.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\PlayMotif.exe"
	-@erase "$(OUTDIR)\PlayMotif.ilk"
	-@erase "$(OUTDIR)\PlayMotif.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\..\common\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\PlayMotif.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\PlayMotif.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PlayMotif.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dxerr8.lib winmm.lib dxguid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\PlayMotif.pdb" /debug /machine:I386 /out:"$(OUTDIR)\PlayMotif.exe" /pdbtype:sept /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\PlayMotif.obj" \
	"$(INTDIR)\dmutil.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\PlayMotif.res"

"$(OUTDIR)\PlayMotif.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("PlayMotif.dep")
!INCLUDE "PlayMotif.dep"
!ELSE 
!MESSAGE Warning: cannot find "PlayMotif.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "PlayMotif - Win32 Release" || "$(CFG)" == "PlayMotif - Win32 Debug"
SOURCE=.\PlayMotif.cpp

"$(INTDIR)\PlayMotif.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\PlayMotif.rc

"$(INTDIR)\PlayMotif.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=..\..\common\src\dmutil.cpp

"$(INTDIR)\dmutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\src\dxutil.cpp

"$(INTDIR)\dxutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

