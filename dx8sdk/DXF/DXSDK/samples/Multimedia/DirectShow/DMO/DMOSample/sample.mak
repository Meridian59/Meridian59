# Microsoft Developer Studio Generated NMAKE File, Based on sample.dsp
!IF "$(CFG)" == ""
CFG=dmosample - Win32 Debug
!MESSAGE No configuration specified. Defaulting to dmosample - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "dmosample - Win32 Release" && "$(CFG)" != "dmosample - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sample.mak" CFG="dmosample - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dmosample - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dmosample - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "dmosample - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\dmosample.dll"


CLEAN :
	-@erase "$(INTDIR)\dmosample.obj"
	-@erase "$(INTDIR)\dmosample.res"
	-@erase "$(INTDIR)\sample.obj"
	-@erase "$(INTDIR)\state.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\dmosample.dll"
	-@erase "$(OUTDIR)\dmosample.exp"
	-@erase "$(OUTDIR)\dmosample.lib"
	-@erase "$(OUTDIR)\dmosample.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "..\..\..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DMOSAMPLE_EXPORTS" /Fp"$(INTDIR)\sample.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\dmosample.res" /i "..\..\BaseClasses" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\sample.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msdmo.lib dmoguids.lib strmiids.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\dmosample.pdb" /map:"$(INTDIR)\dmosample.map" /machine:I386 /def:"dmosample.def" /out:"$(OUTDIR)\dmosample.dll" /implib:"$(OUTDIR)\dmosample.lib" /libpath:"..\..\..\..\..\lib" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\dmosample.obj" \
	"$(INTDIR)\sample.obj" \
	"$(INTDIR)\state.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\dmosample.res"

"$(OUTDIR)\dmosample.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dmosample - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\dmosample.dll"


CLEAN :
	-@erase "$(INTDIR)\dmosample.obj"
	-@erase "$(INTDIR)\dmosample.res"
	-@erase "$(INTDIR)\sample.obj"
	-@erase "$(INTDIR)\state.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\dmosample.dll"
	-@erase "$(OUTDIR)\dmosample.exp"
	-@erase "$(OUTDIR)\dmosample.ilk"
	-@erase "$(OUTDIR)\dmosample.lib"
	-@erase "$(OUTDIR)\dmosample.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DMOSAMPLE_EXPORTS" /Fp"$(INTDIR)\sample.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\dmosample.res" /i "..\..\BaseClasses" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\sample.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msdmo.lib dmoguids.lib strmiids.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\dmosample.pdb" /debug /machine:I386 /def:"dmosample.def" /out:"$(OUTDIR)\dmosample.dll" /implib:"$(OUTDIR)\dmosample.lib" /pdbtype:sept /libpath:"..\..\..\..\..\lib" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\dmosample.obj" \
	"$(INTDIR)\sample.obj" \
	"$(INTDIR)\state.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\dmosample.res"

"$(OUTDIR)\dmosample.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("sample.dep")
!INCLUDE "sample.dep"
!ELSE 
!MESSAGE Warning: cannot find "sample.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "dmosample - Win32 Release" || "$(CFG)" == "dmosample - Win32 Debug"
SOURCE=.\dmosample.cpp

"$(INTDIR)\dmosample.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sample.cpp

"$(INTDIR)\sample.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\state.cpp

"$(INTDIR)\state.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\StdAfx.cpp

"$(INTDIR)\StdAfx.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\util.cpp

"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\dmosample.rc

"$(INTDIR)\dmosample.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

