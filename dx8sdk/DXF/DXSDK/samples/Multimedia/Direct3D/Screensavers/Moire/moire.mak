# Microsoft Developer Studio Generated NMAKE File, Based on Moire.dsp
!IF "$(CFG)" == ""
CFG=Moire - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Moire - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Moire - Win32 Release" && "$(CFG)" != "Moire - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Moire.mak" CFG="Moire - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Moire - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Moire - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "Moire - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\Moire.scr"


CLEAN :
	-@erase "$(INTDIR)\d3dfont.obj"
	-@erase "$(INTDIR)\d3dsaver.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\Moire.obj"
	-@erase "$(INTDIR)\Moire.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\Moire.scr"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\..\common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Moire.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Moire.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=d3d8.lib d3dx8.lib winmm.lib kernel32.lib comctl32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\Moire.pdb" /machine:I386 /out:"$(OUTDIR)\Moire.scr" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\d3dfont.obj" \
	"$(INTDIR)\d3dsaver.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\Moire.obj" \
	"$(INTDIR)\Moire.res"

"$(OUTDIR)\Moire.scr" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Moire - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\Moire.scr"


CLEAN :
	-@erase "$(INTDIR)\d3dfont.obj"
	-@erase "$(INTDIR)\d3dsaver.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\Moire.obj"
	-@erase "$(INTDIR)\Moire.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\Moire.pdb"
	-@erase "$(OUTDIR)\Moire.scr"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\common\include" /I "..\Common" /I "..\swrast" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Moire.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Moire.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dxguid.lib d3d8.lib d3dx8.lib winmm.lib kernel32.lib comctl32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\Moire.pdb" /debug /machine:I386 /out:"$(OUTDIR)\Moire.scr" /pdbtype:sept /libpath:"..\swrast" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\d3dfont.obj" \
	"$(INTDIR)\d3dsaver.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\Moire.obj" \
	"$(INTDIR)\Moire.res"

"$(OUTDIR)\Moire.scr" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Moire.dep")
!INCLUDE "Moire.dep"
!ELSE 
!MESSAGE Warning: cannot find "Moire.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Moire - Win32 Release" || "$(CFG)" == "Moire - Win32 Debug"
SOURCE=..\..\..\common\src\d3dfont.cpp

"$(INTDIR)\d3dfont.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\d3dsaver.cpp

"$(INTDIR)\d3dsaver.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\dxutil.cpp

"$(INTDIR)\dxutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Moire.cpp

"$(INTDIR)\Moire.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Moire.rc

"$(INTDIR)\Moire.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

