# Microsoft Developer Studio Generated NMAKE File, Based on enumdevices.dsp
!IF "$(CFG)" == ""
CFG=EnumDevices - Win32 Debug
!MESSAGE No configuration specified. Defaulting to EnumDevices - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "EnumDevices - Win32 Release" && "$(CFG)" != "EnumDevices - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "enumdevices.mak" CFG="EnumDevices - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "EnumDevices - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "EnumDevices - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "EnumDevices - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\enumdevices.exe"


CLEAN :
	-@erase "$(INTDIR)\dsutil.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\enumdevices.obj"
	-@erase "$(INTDIR)\enumdevices.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\enumdevices.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\enumdevices.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\enumdevices.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\enumdevices.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dxerr8.lib winmm.lib dsound.lib dxguid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\enumdevices.pdb" /machine:I386 /out:"$(OUTDIR)\enumdevices.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\enumdevices.obj" \
	"$(INTDIR)\dsutil.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\enumdevices.res"

"$(OUTDIR)\enumdevices.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "EnumDevices - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\enumdevices.exe"


CLEAN :
	-@erase "$(INTDIR)\dsutil.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\enumdevices.obj"
	-@erase "$(INTDIR)\enumdevices.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\enumdevices.exe"
	-@erase "$(OUTDIR)\enumdevices.ilk"
	-@erase "$(OUTDIR)\enumdevices.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\..\common\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\enumdevices.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\enumdevices.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\enumdevices.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=comctl32.lib dxerr8.lib winmm.lib dsound.lib dxguid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\enumdevices.pdb" /debug /machine:I386 /out:"$(OUTDIR)\enumdevices.exe" /pdbtype:sept /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\enumdevices.obj" \
	"$(INTDIR)\dsutil.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\enumdevices.res"

"$(OUTDIR)\enumdevices.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("enumdevices.dep")
!INCLUDE "enumdevices.dep"
!ELSE 
!MESSAGE Warning: cannot find "enumdevices.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "EnumDevices - Win32 Release" || "$(CFG)" == "EnumDevices - Win32 Debug"
SOURCE=.\enumdevices.cpp

"$(INTDIR)\enumdevices.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\enumdevices.rc

"$(INTDIR)\enumdevices.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=..\..\common\src\dsutil.cpp

"$(INTDIR)\dsutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\src\dxutil.cpp

"$(INTDIR)\dxutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

