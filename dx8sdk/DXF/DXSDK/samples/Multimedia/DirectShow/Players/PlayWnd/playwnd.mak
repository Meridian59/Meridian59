# Microsoft Developer Studio Generated NMAKE File, Based on PlayWnd.dsp
!IF "$(CFG)" == ""
CFG=PlayWnd - Win32 Debug
!MESSAGE No configuration specified. Defaulting to PlayWnd - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "PlayWnd - Win32 Release" && "$(CFG)" != "PlayWnd - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PlayWnd.mak" CFG="PlayWnd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PlayWnd - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PlayWnd - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "PlayWnd - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\PlayWnd.exe"


CLEAN :
	-@erase "$(INTDIR)\playwnd.obj"
	-@erase "$(INTDIR)\playwnd.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\PlayWnd.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\..\..\..\include" /I "..\..\BaseClasses" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\PlayWnd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\playwnd.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PlayWnd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=winspool.lib shell32.lib odbc32.lib odbccp32.lib quartz.lib msvcrt.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib winmm.lib msacm32.lib olepro32.lib oleaut32.lib advapi32.lib uuid.lib strmiids.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\PlayWnd.pdb" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\PlayWnd.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\playwnd.obj" \
	"$(INTDIR)\playwnd.res"

"$(OUTDIR)\PlayWnd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PlayWnd - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\PlayWnd.exe"


CLEAN :
	-@erase "$(INTDIR)\playwnd.obj"
	-@erase "$(INTDIR)\playwnd.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\PlayWnd.exe"
	-@erase "$(OUTDIR)\PlayWnd.ilk"
	-@erase "$(OUTDIR)\PlayWnd.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\..\..\..\..\include" /I "..\..\BaseClasses" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\PlayWnd.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\playwnd.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PlayWnd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=quartz.lib msvcrtd.lib winmm.lib msacm32.lib olepro32.lib strmiids.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib oleaut32.lib advapi32.lib uuid.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\PlayWnd.pdb" /debug /machine:I386 /nodefaultlib /out:"$(OUTDIR)\PlayWnd.exe" /pdbtype:sept /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\playwnd.obj" \
	"$(INTDIR)\playwnd.res"

"$(OUTDIR)\PlayWnd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("PlayWnd.dep")
!INCLUDE "PlayWnd.dep"
!ELSE 
!MESSAGE Warning: cannot find "PlayWnd.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "PlayWnd - Win32 Release" || "$(CFG)" == "PlayWnd - Win32 Debug"
SOURCE=.\playwnd.cpp

"$(INTDIR)\playwnd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\playwnd.rc

"$(INTDIR)\playwnd.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

