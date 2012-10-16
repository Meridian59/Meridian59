# Microsoft Developer Studio Generated NMAKE File, Based on mfcfog.dsp
!IF "$(CFG)" == ""
CFG=MFCFog - Win32 Debug
!MESSAGE No configuration specified. Defaulting to MFCFog - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "MFCFog - Win32 Release" && "$(CFG)" != "MFCFog - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mfcfog.mak" CFG="MFCFog - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MFCFog - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "MFCFog - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "MFCFog - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\mfcfog.exe"


CLEAN :
	-@erase "$(INTDIR)\d3dapp.obj"
	-@erase "$(INTDIR)\d3dfile.obj"
	-@erase "$(INTDIR)\d3dutil.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\fog.obj"
	-@erase "$(INTDIR)\fog.res"
	-@erase "$(INTDIR)\stdafx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\mfcfog.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "..\..\common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Fp"$(INTDIR)\mfcfog.pch" /YX"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\fog.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mfcfog.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=d3dx8.lib d3d8.lib d3dxof.lib dxguid.lib winmm.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\mfcfog.pdb" /machine:I386 /out:"$(OUTDIR)\mfcfog.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\d3dapp.obj" \
	"$(INTDIR)\d3dfile.obj" \
	"$(INTDIR)\d3dutil.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\fog.obj" \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\fog.res"

"$(OUTDIR)\mfcfog.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MFCFog - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\mfcfog.exe"


CLEAN :
	-@erase "$(INTDIR)\d3dapp.obj"
	-@erase "$(INTDIR)\d3dfile.obj"
	-@erase "$(INTDIR)\d3dutil.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\fog.obj"
	-@erase "$(INTDIR)\fog.res"
	-@erase "$(INTDIR)\stdafx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\mfcfog.exe"
	-@erase "$(OUTDIR)\mfcfog.ilk"
	-@erase "$(OUTDIR)\mfcfog.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\common\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXDLL" /Fp"$(INTDIR)\mfcfog.pch" /YX"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\fog.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mfcfog.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=d3dx8dt.lib d3d8.lib d3dxof.lib dxguid.lib winmm.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\mfcfog.pdb" /debug /machine:I386 /out:"$(OUTDIR)\mfcfog.exe" /pdbtype:sept /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\d3dapp.obj" \
	"$(INTDIR)\d3dfile.obj" \
	"$(INTDIR)\d3dutil.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\fog.obj" \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\fog.res"

"$(OUTDIR)\mfcfog.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("mfcfog.dep")
!INCLUDE "mfcfog.dep"
!ELSE 
!MESSAGE Warning: cannot find "mfcfog.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "MFCFog - Win32 Release" || "$(CFG)" == "MFCFog - Win32 Debug"
SOURCE=..\..\common\src\d3dapp.cpp

"$(INTDIR)\d3dapp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\src\d3dfile.cpp

"$(INTDIR)\d3dfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\src\d3dutil.cpp

"$(INTDIR)\d3dutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\src\dxutil.cpp

"$(INTDIR)\dxutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\fog.rc

"$(INTDIR)\fog.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\fog.cpp

"$(INTDIR)\fog.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\stdafx.cpp

"$(INTDIR)\stdafx.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

