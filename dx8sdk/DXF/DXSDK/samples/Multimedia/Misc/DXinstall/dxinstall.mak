# Microsoft Developer Studio Generated NMAKE File, Based on dxinstall.dsp
!IF "$(CFG)" == ""
CFG=DXInstall - Win32 Debug
!MESSAGE No configuration specified. Defaulting to DXInstall - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "DXInstall - Win32 Release" && "$(CFG)" != "DXInstall - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dxinstall.mak" CFG="DXInstall - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DXInstall - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "DXInstall - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "DXInstall - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\dxinstall.exe"


CLEAN :
	-@erase "$(INTDIR)\dxinstall.obj"
	-@erase "$(INTDIR)\dxinstall.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\winmain.obj"
	-@erase "$(OUTDIR)\dxinstall.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\dxinstall.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\dxinstall.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dxinstall.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dsetup.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\dxinstall.pdb" /machine:I386 /out:"$(OUTDIR)\dxinstall.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\dxinstall.obj" \
	"$(INTDIR)\winmain.obj" \
	"$(INTDIR)\dxinstall.res"

"$(OUTDIR)\dxinstall.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "DXInstall - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\dxinstall.exe"


CLEAN :
	-@erase "$(INTDIR)\dxinstall.obj"
	-@erase "$(INTDIR)\dxinstall.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\winmain.obj"
	-@erase "$(OUTDIR)\dxinstall.exe"
	-@erase "$(OUTDIR)\dxinstall.ilk"
	-@erase "$(OUTDIR)\dxinstall.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\dxinstall.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\dxinstall.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dxinstall.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dsetup.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\dxinstall.pdb" /debug /machine:I386 /out:"$(OUTDIR)\dxinstall.exe" /pdbtype:sept /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\dxinstall.obj" \
	"$(INTDIR)\winmain.obj" \
	"$(INTDIR)\dxinstall.res"

"$(OUTDIR)\dxinstall.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("dxinstall.dep")
!INCLUDE "dxinstall.dep"
!ELSE 
!MESSAGE Warning: cannot find "dxinstall.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "DXInstall - Win32 Release" || "$(CFG)" == "DXInstall - Win32 Debug"
SOURCE=.\dxinstall.rc

"$(INTDIR)\dxinstall.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\dxinstall.cpp

"$(INTDIR)\dxinstall.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\winmain.cpp

"$(INTDIR)\winmain.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

