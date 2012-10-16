# Microsoft Developer Studio Generated NMAKE File, Based on ddrawxcl.dsp
!IF "$(CFG)" == ""
CFG=ddrawxcl - Win32 Debug
!MESSAGE No configuration specified. Defaulting to ddrawxcl - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "ddrawxcl - Win32 Release" && "$(CFG)" != "ddrawxcl - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ddrawxcl.mak" CFG="ddrawxcl - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ddrawxcl - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ddrawxcl - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "ddrawxcl - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\ddrawxcl.exe"


CLEAN :
	-@erase "$(INTDIR)\ddrawobj.obj"
	-@erase "$(INTDIR)\ddrawxcl.obj"
	-@erase "$(INTDIR)\ddrawxcl.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vidplay.obj"
	-@erase "$(OUTDIR)\ddrawxcl.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\ddrawxcl.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ddrawxcl.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\baseclasses\release\strmbase.lib quartz.lib kernel32.lib user32.lib comdlg32.lib ole32.lib oleaut32.lib gdi32.lib ddraw.lib msvcrt.lib uuid.lib advapi32.lib winmm.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib /out:"$(OUTDIR)\ddrawxcl.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\ddrawobj.obj" \
	"$(INTDIR)\ddrawxcl.obj" \
	"$(INTDIR)\vidplay.obj" \
	"$(INTDIR)\ddrawxcl.res"

"$(OUTDIR)\ddrawxcl.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ddrawxcl - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\ddrawxcl.exe"


CLEAN :
	-@erase "$(INTDIR)\ddrawobj.obj"
	-@erase "$(INTDIR)\ddrawxcl.obj"
	-@erase "$(INTDIR)\ddrawxcl.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vidplay.obj"
	-@erase "$(OUTDIR)\ddrawxcl.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /GX /Zi /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "DEBUG" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\ddrawxcl.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ddrawxcl.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\baseclasses\debug\strmbasd.lib quartz.lib kernel32.lib user32.lib comdlg32.lib ole32.lib oleaut32.lib gdi32.lib ddraw.lib msvcrtd.lib uuid.lib advapi32.lib winmm.lib /nologo /subsystem:windows /pdb:none /debug /machine:I386 /nodefaultlib /out:"$(OUTDIR)\ddrawxcl.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\ddrawobj.obj" \
	"$(INTDIR)\ddrawxcl.obj" \
	"$(INTDIR)\vidplay.obj" \
	"$(INTDIR)\ddrawxcl.res"

"$(OUTDIR)\ddrawxcl.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("ddrawxcl.dep")
!INCLUDE "ddrawxcl.dep"
!ELSE 
!MESSAGE Warning: cannot find "ddrawxcl.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ddrawxcl - Win32 Release" || "$(CFG)" == "ddrawxcl - Win32 Debug"
SOURCE=.\ddrawobj.cpp

"$(INTDIR)\ddrawobj.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ddrawxcl.cpp

"$(INTDIR)\ddrawxcl.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ddrawxcl.rc

"$(INTDIR)\ddrawxcl.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\vidplay.cpp

"$(INTDIR)\vidplay.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

