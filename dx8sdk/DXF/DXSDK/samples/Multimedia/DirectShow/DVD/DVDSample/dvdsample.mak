# Microsoft Developer Studio Generated NMAKE File, Based on dvdsample.dsp
!IF "$(CFG)" == ""
CFG=DVDSample - Win32 Debug
!MESSAGE No configuration specified. Defaulting to DVDSample - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "DVDSample - Win32 Release" && "$(CFG)" != "DVDSample - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dvdsample.mak" CFG="DVDSample - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DVDSample - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "DVDSample - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "DVDSample - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\dvdsample.exe"


CLEAN :
	-@erase "$(INTDIR)\Dialogs.obj"
	-@erase "$(INTDIR)\DvdCore.obj"
	-@erase "$(INTDIR)\DVDSample.obj"
	-@erase "$(INTDIR)\dvdsample.pch"
	-@erase "$(INTDIR)\DVDSample.res"
	-@erase "$(INTDIR)\StringUtil.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\dvdsample.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\dvdsample.pch" /Yu"dshow.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\DVDSample.res" /i "..\..\BaseClasses" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dvdsample.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib quartz.lib strmiids.lib winmm.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\dvdsample.pdb" /machine:I386 /out:"$(OUTDIR)\dvdsample.exe" /libpath:"..\..\..\..\..\lib" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\Dialogs.obj" \
	"$(INTDIR)\DvdCore.obj" \
	"$(INTDIR)\DVDSample.obj" \
	"$(INTDIR)\StringUtil.obj" \
	"$(INTDIR)\DVDSample.res"

"$(OUTDIR)\dvdsample.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "DVDSample - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\dvdsample.exe"


CLEAN :
	-@erase "$(INTDIR)\Dialogs.obj"
	-@erase "$(INTDIR)\DvdCore.obj"
	-@erase "$(INTDIR)\DVDSample.obj"
	-@erase "$(INTDIR)\dvdsample.pch"
	-@erase "$(INTDIR)\DVDSample.res"
	-@erase "$(INTDIR)\StringUtil.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\dvdsample.exe"
	-@erase "$(OUTDIR)\dvdsample.ilk"
	-@erase "$(OUTDIR)\dvdsample.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "DEBUG" /Fp"$(INTDIR)\dvdsample.pch" /Yu"dshow.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\DVDSample.res" /i "..\..\BaseClasses" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dvdsample.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib quartz.lib strmiids.lib winmm.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\dvdsample.pdb" /debug /machine:I386 /out:"$(OUTDIR)\dvdsample.exe" /pdbtype:sept /libpath:"..\..\..\..\..\lib" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\Dialogs.obj" \
	"$(INTDIR)\DvdCore.obj" \
	"$(INTDIR)\DVDSample.obj" \
	"$(INTDIR)\StringUtil.obj" \
	"$(INTDIR)\DVDSample.res"

"$(OUTDIR)\dvdsample.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("dvdsample.dep")
!INCLUDE "dvdsample.dep"
!ELSE 
!MESSAGE Warning: cannot find "dvdsample.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "DVDSample - Win32 Release" || "$(CFG)" == "DVDSample - Win32 Debug"
SOURCE=.\Dialogs.cpp

"$(INTDIR)\Dialogs.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dvdsample.pch"


SOURCE=.\DvdCore.cpp

"$(INTDIR)\DvdCore.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dvdsample.pch"


SOURCE=.\DVDSample.cpp

!IF  "$(CFG)" == "DVDSample - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\dvdsample.pch" /Yc"dshow.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\DVDSample.obj"	"$(INTDIR)\dvdsample.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "DVDSample - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "DEBUG" /Fp"$(INTDIR)\dvdsample.pch" /Yc"dshow.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\DVDSample.obj"	"$(INTDIR)\dvdsample.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\StringUtil.cpp

"$(INTDIR)\StringUtil.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dvdsample.pch"


SOURCE=.\DVDSample.rc

"$(INTDIR)\DVDSample.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

