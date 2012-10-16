# Microsoft Developer Studio Generated NMAKE File, Based on amcap.dsp
!IF "$(CFG)" == ""
CFG=amcap - Win32 Debug
!MESSAGE No configuration specified. Defaulting to amcap - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "amcap - Win32 Release" && "$(CFG)" != "amcap - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "amcap.mak" CFG="amcap - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "amcap - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "amcap - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "amcap - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\amcap.exe"


CLEAN :
	-@erase "$(INTDIR)\amcap.obj"
	-@erase "$(INTDIR)\amcap.pch"
	-@erase "$(INTDIR)\amcap.res"
	-@erase "$(INTDIR)\crossbar.obj"
	-@erase "$(INTDIR)\status.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\amcap.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x0400 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\amcap.res" /i "..\..\BaseClasses" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\amcap.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=quartz.lib msvcrt.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib winmm.lib msacm32.lib olepro32.lib oleaut32.lib advapi32.lib uuid.lib strmiids.lib ..\..\baseclasses\release\strmbase.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib /out:"$(OUTDIR)\amcap.exe" /libpath:"..\..\..\..\..\lib" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\amcap.obj" \
	"$(INTDIR)\crossbar.obj" \
	"$(INTDIR)\status.obj" \
	"$(INTDIR)\amcap.res"

"$(OUTDIR)\amcap.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "amcap - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\amcap.exe"


CLEAN :
	-@erase "$(INTDIR)\amcap.obj"
	-@erase "$(INTDIR)\amcap.pch"
	-@erase "$(INTDIR)\amcap.res"
	-@erase "$(INTDIR)\crossbar.obj"
	-@erase "$(INTDIR)\status.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\amcap.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /GX /Zi /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x0400 /D "DEBUG" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\amcap.res" /i "..\..\BaseClasses" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\amcap.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=quartz.lib msvcrtd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib winmm.lib msacm32.lib olepro32.lib oleaut32.lib advapi32.lib uuid.lib strmiids.lib ..\..\baseclasses\debug\strmbasd.lib /nologo /subsystem:windows /pdb:none /debug /machine:I386 /nodefaultlib /out:"$(OUTDIR)\amcap.exe" /libpath:"..\..\..\..\..\lib" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\amcap.obj" \
	"$(INTDIR)\crossbar.obj" \
	"$(INTDIR)\status.obj" \
	"$(INTDIR)\amcap.res"

"$(OUTDIR)\amcap.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("amcap.dep")
!INCLUDE "amcap.dep"
!ELSE 
!MESSAGE Warning: cannot find "amcap.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "amcap - Win32 Release" || "$(CFG)" == "amcap - Win32 Debug"
SOURCE=.\amcap.cpp

!IF  "$(CFG)" == "amcap - Win32 Release"

CPP_SWITCHES=/nologo /ML /W3 /GX /O2 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x0400 /Fp"$(INTDIR)\amcap.pch" /Yc"streams.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\amcap.obj"	"$(INTDIR)\amcap.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "amcap - Win32 Debug"

CPP_SWITCHES=/nologo /MLd /W3 /GX /Zi /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x0400 /D "DEBUG" /Fp"$(INTDIR)\amcap.pch" /Yc"streams.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\amcap.obj"	"$(INTDIR)\amcap.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\crossbar.cpp

!IF  "$(CFG)" == "amcap - Win32 Release"

CPP_SWITCHES=/nologo /ML /W3 /GX /O2 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x0400 /Fp"$(INTDIR)\amcap.pch" /Yu"streams.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\crossbar.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\amcap.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "amcap - Win32 Debug"

CPP_SWITCHES=/nologo /MLd /W3 /GX /Zi /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x0400 /D "DEBUG" /Fp"$(INTDIR)\amcap.pch" /Yu"streams.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\crossbar.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\amcap.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\status.cpp

!IF  "$(CFG)" == "amcap - Win32 Release"

CPP_SWITCHES=/nologo /ML /W3 /GX /O2 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x0400 /Fp"$(INTDIR)\amcap.pch" /Yu"streams.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\status.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\amcap.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "amcap - Win32 Debug"

CPP_SWITCHES=/nologo /MLd /W3 /GX /Zi /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D WINVER=0x0400 /D "DEBUG" /Fp"$(INTDIR)\amcap.pch" /Yu"streams.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\status.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\amcap.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\amcap.rc

"$(INTDIR)\amcap.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

