# Microsoft Developer Studio Generated NMAKE File, Based on PlayWndASF.dsp
!IF "$(CFG)" == ""
CFG=PlayWndASF - Win32 Debug
!MESSAGE No configuration specified. Defaulting to PlayWndASF - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "PlayWndASF - Win32 Release" && "$(CFG)" != "PlayWndASF - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PlayWndASF.mak" CFG="PlayWndASF - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PlayWndASF - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PlayWndASF - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "PlayWndASF - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\PlayWndASF.exe"


CLEAN :
	-@erase "$(INTDIR)\keyprovider.obj"
	-@erase "$(INTDIR)\PlayWndASF.obj"
	-@erase "$(INTDIR)\PlayWndASF.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\PlayWndASF.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\..\..\..\include" /I "..\..\BaseClasses" /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\PlayWndASF.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\PlayWndASF.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PlayWndASF.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=winspool.lib shell32.lib odbc32.lib odbccp32.lib ..\..\common\wmstub.lib ..\..\common\wmvcore.lib quartz.lib msvcrt.lib winmm.lib msacm32.lib olepro32.lib strmiids.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib oleaut32.lib advapi32.lib uuid.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\PlayWndASF.pdb" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\PlayWndASF.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\keyprovider.obj" \
	"$(INTDIR)\PlayWndASF.obj" \
	"$(INTDIR)\PlayWndASF.res"

"$(OUTDIR)\PlayWndASF.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PlayWndASF - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\PlayWndASF.exe"


CLEAN :
	-@erase "$(INTDIR)\keyprovider.obj"
	-@erase "$(INTDIR)\PlayWndASF.obj"
	-@erase "$(INTDIR)\PlayWndASF.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\PlayWndASF.exe"
	-@erase "$(OUTDIR)\PlayWndASF.ilk"
	-@erase "$(OUTDIR)\PlayWndASF.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\..\..\..\..\include" /I "..\..\BaseClasses" /I "..\..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\PlayWndASF.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\PlayWndASF.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PlayWndASF.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\common\wmstub.lib ..\..\common\wmvcore.lib quartz.lib msvcrtd.lib winmm.lib msacm32.lib olepro32.lib strmiids.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib ole32.lib oleaut32.lib advapi32.lib uuid.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\PlayWndASF.pdb" /debug /machine:I386 /nodefaultlib /out:"$(OUTDIR)\PlayWndASF.exe" /pdbtype:sept /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\keyprovider.obj" \
	"$(INTDIR)\PlayWndASF.obj" \
	"$(INTDIR)\PlayWndASF.res"

"$(OUTDIR)\PlayWndASF.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("PlayWndASF.dep")
!INCLUDE "PlayWndASF.dep"
!ELSE 
!MESSAGE Warning: cannot find "PlayWndASF.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "PlayWndASF - Win32 Release" || "$(CFG)" == "PlayWndASF - Win32 Debug"
SOURCE=.\keyprovider.cpp

"$(INTDIR)\keyprovider.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\PlayWndASF.cpp

"$(INTDIR)\PlayWndASF.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\PlayWndASF.rc

"$(INTDIR)\PlayWndASF.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

