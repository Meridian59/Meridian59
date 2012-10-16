# Microsoft Developer Studio Generated NMAKE File, Based on optimizedmesh.dsp
!IF "$(CFG)" == ""
CFG=OptimizedMesh - Win32 Debug
!MESSAGE No configuration specified. Defaulting to OptimizedMesh - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "OptimizedMesh - Win32 Release" && "$(CFG)" != "OptimizedMesh - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "optimizedmesh.mak" CFG="OptimizedMesh - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "OptimizedMesh - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "OptimizedMesh - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "OptimizedMesh - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\optimizedmesh.exe"


CLEAN :
	-@erase "$(INTDIR)\d3dapp.obj"
	-@erase "$(INTDIR)\d3dfont.obj"
	-@erase "$(INTDIR)\d3dutil.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\OptimizedMesh.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\WinMain.res"
	-@erase "$(OUTDIR)\optimizedmesh.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\optimizedmesh.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\WinMain.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\optimizedmesh.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=d3dx8.lib d3d8.lib winmm.lib dxguid.lib d3dxof.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\optimizedmesh.pdb" /machine:I386 /out:"$(OUTDIR)\optimizedmesh.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\d3dapp.obj" \
	"$(INTDIR)\d3dfont.obj" \
	"$(INTDIR)\d3dutil.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\OptimizedMesh.obj" \
	"$(INTDIR)\WinMain.res"

"$(OUTDIR)\optimizedmesh.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "OptimizedMesh - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\optimizedmesh.exe" "$(OUTDIR)\optimizedmesh.bsc"


CLEAN :
	-@erase "$(INTDIR)\d3dapp.obj"
	-@erase "$(INTDIR)\d3dapp.sbr"
	-@erase "$(INTDIR)\d3dfont.obj"
	-@erase "$(INTDIR)\d3dfont.sbr"
	-@erase "$(INTDIR)\d3dutil.obj"
	-@erase "$(INTDIR)\d3dutil.sbr"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\dxutil.sbr"
	-@erase "$(INTDIR)\OptimizedMesh.obj"
	-@erase "$(INTDIR)\OptimizedMesh.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WinMain.res"
	-@erase "$(OUTDIR)\optimizedmesh.bsc"
	-@erase "$(OUTDIR)\optimizedmesh.exe"
	-@erase "$(OUTDIR)\optimizedmesh.ilk"
	-@erase "$(OUTDIR)\optimizedmesh.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\..\common\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\optimizedmesh.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\WinMain.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\optimizedmesh.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\d3dapp.sbr" \
	"$(INTDIR)\d3dfont.sbr" \
	"$(INTDIR)\d3dutil.sbr" \
	"$(INTDIR)\dxutil.sbr" \
	"$(INTDIR)\OptimizedMesh.sbr"

"$(OUTDIR)\optimizedmesh.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=d3dx8dt.lib d3d8.lib winmm.lib dxguid.lib d3dxof.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\optimizedmesh.pdb" /debug /machine:I386 /out:"$(OUTDIR)\optimizedmesh.exe" /pdbtype:sept /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\d3dapp.obj" \
	"$(INTDIR)\d3dfont.obj" \
	"$(INTDIR)\d3dutil.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\OptimizedMesh.obj" \
	"$(INTDIR)\WinMain.res"

"$(OUTDIR)\optimizedmesh.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("optimizedmesh.dep")
!INCLUDE "optimizedmesh.dep"
!ELSE 
!MESSAGE Warning: cannot find "optimizedmesh.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "OptimizedMesh - Win32 Release" || "$(CFG)" == "OptimizedMesh - Win32 Debug"
SOURCE=.\WinMain.rc

"$(INTDIR)\WinMain.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=..\..\common\src\d3dapp.cpp

!IF  "$(CFG)" == "OptimizedMesh - Win32 Release"


"$(INTDIR)\d3dapp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OptimizedMesh - Win32 Debug"


"$(INTDIR)\d3dapp.obj"	"$(INTDIR)\d3dapp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\src\d3dfont.cpp

!IF  "$(CFG)" == "OptimizedMesh - Win32 Release"


"$(INTDIR)\d3dfont.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OptimizedMesh - Win32 Debug"


"$(INTDIR)\d3dfont.obj"	"$(INTDIR)\d3dfont.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\src\d3dutil.cpp

!IF  "$(CFG)" == "OptimizedMesh - Win32 Release"


"$(INTDIR)\d3dutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OptimizedMesh - Win32 Debug"


"$(INTDIR)\d3dutil.obj"	"$(INTDIR)\d3dutil.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\common\src\dxutil.cpp

!IF  "$(CFG)" == "OptimizedMesh - Win32 Release"


"$(INTDIR)\dxutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "OptimizedMesh - Win32 Debug"


"$(INTDIR)\dxutil.obj"	"$(INTDIR)\dxutil.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\OptimizedMesh.cpp

!IF  "$(CFG)" == "OptimizedMesh - Win32 Release"


"$(INTDIR)\OptimizedMesh.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "OptimizedMesh - Win32 Debug"


"$(INTDIR)\OptimizedMesh.obj"	"$(INTDIR)\OptimizedMesh.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

