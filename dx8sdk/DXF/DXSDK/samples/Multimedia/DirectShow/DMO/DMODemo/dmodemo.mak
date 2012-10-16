# Microsoft Developer Studio Generated NMAKE File, Based on dmodemo.dsp
!IF "$(CFG)" == ""
CFG=dmodemo - Win32 Debug
!MESSAGE No configuration specified. Defaulting to dmodemo - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "dmodemo - Win32 Release" && "$(CFG)" != "dmodemo - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dmodemo.mak" CFG="dmodemo - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dmodemo - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "dmodemo - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "dmodemo - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\dmodemo.exe"


CLEAN :
	-@erase "$(INTDIR)\appstream.obj"
	-@erase "$(INTDIR)\dmodemo.obj"
	-@erase "$(INTDIR)\dmodemo.res"
	-@erase "$(INTDIR)\dsutil.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\wave.obj"
	-@erase "$(OUTDIR)\dmodemo.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\BaseClasses" /I "..\..\..\common\include" /I "..\..\..\..\..\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D _WIN32_WINNT=0x0400 /Fp"$(INTDIR)\dmodemo.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\dmodemo.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dmodemo.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=comctl32.lib dsound.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib amstrmid.lib msdmo.lib dmoguids.lib dxguid.lib shlwapi.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\dmodemo.pdb" /machine:I386 /out:"$(OUTDIR)\dmodemo.exe" /libpath:"..\..\..\..\..\lib" /OPT:NOREF /OPT:ICF /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\appstream.obj" \
	"$(INTDIR)\dmodemo.obj" \
	"$(INTDIR)\wave.obj" \
	"$(INTDIR)\dsutil.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\dmodemo.res"

"$(OUTDIR)\dmodemo.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dmodemo - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\dmodemo.exe"


CLEAN :
	-@erase "$(INTDIR)\appstream.obj"
	-@erase "$(INTDIR)\dmodemo.obj"
	-@erase "$(INTDIR)\dmodemo.res"
	-@erase "$(INTDIR)\dsutil.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\wave.obj"
	-@erase "$(OUTDIR)\dmodemo.exe"
	-@erase "$(OUTDIR)\dmodemo.ilk"
	-@erase "$(OUTDIR)\dmodemo.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\..\BaseClasses" /I "..\..\..\common\include" /I "..\..\..\..\..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D _WIN32_WINNT=0x0400 /Fp"$(INTDIR)\dmodemo.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\dmodemo.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dmodemo.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=comctl32.lib dsound.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib amstrmid.lib msdmo.lib dmoguids.lib dxguid.lib shlwapi.lib dxerr8.lib /nologo /incremental:yes /pdb:"$(OUTDIR)\dmodemo.pdb" /debug /machine:IX86 /out:"$(OUTDIR)\dmodemo.exe" /libpath:"..\..\..\..\..\lib" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\appstream.obj" \
	"$(INTDIR)\dmodemo.obj" \
	"$(INTDIR)\wave.obj" \
	"$(INTDIR)\dsutil.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\dmodemo.res"

"$(OUTDIR)\dmodemo.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("dmodemo.dep")
!INCLUDE "dmodemo.dep"
!ELSE 
!MESSAGE Warning: cannot find "dmodemo.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "dmodemo - Win32 Release" || "$(CFG)" == "dmodemo - Win32 Debug"
SOURCE=.\appstream.cpp

"$(INTDIR)\appstream.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\dmodemo.cpp

"$(INTDIR)\dmodemo.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\wave.c

"$(INTDIR)\wave.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\dmodemo.rc

"$(INTDIR)\dmodemo.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=..\..\..\Common\src\dsutil.cpp

"$(INTDIR)\dsutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\Common\src\dxutil.cpp

"$(INTDIR)\dxutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

