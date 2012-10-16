# Microsoft Developer Studio Generated NMAKE File, Based on actionmapper.dsp
!IF "$(CFG)" == ""
CFG=ActionMapper - Win32 Debug
!MESSAGE No configuration specified. Defaulting to ActionMapper - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "ActionMapper - Win32 Release" && "$(CFG)" != "ActionMapper - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "actionmapper.mak" CFG="ActionMapper - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ActionMapper - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ActionMapper - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "ActionMapper - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\actionmapper.exe"


CLEAN :
	-@erase "$(INTDIR)\ActionMapper.obj"
	-@erase "$(INTDIR)\ActionMapper.res"
	-@erase "$(INTDIR)\diutil.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\actionmapper.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\actionmapper.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\ActionMapper.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\actionmapper.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dsound.lib dinput8.lib dxerr8.lib d3dx8dt.lib d3d8.lib d3dxof.lib dxguid.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\actionmapper.pdb" /machine:I386 /out:"$(OUTDIR)\actionmapper.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\ActionMapper.obj" \
	"$(INTDIR)\diutil.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\ActionMapper.res"

"$(OUTDIR)\actionmapper.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ActionMapper - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\actionmapper.exe"


CLEAN :
	-@erase "$(INTDIR)\ActionMapper.obj"
	-@erase "$(INTDIR)\ActionMapper.res"
	-@erase "$(INTDIR)\diutil.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\actionmapper.exe"
	-@erase "$(OUTDIR)\actionmapper.ilk"
	-@erase "$(OUTDIR)\actionmapper.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /I "..\..\common\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\actionmapper.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\ActionMapper.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\actionmapper.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dsound.lib dinput8.lib dxerr8.lib d3dx8dt.lib d3d8.lib d3dxof.lib dxguid.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\actionmapper.pdb" /debug /machine:I386 /out:"$(OUTDIR)\actionmapper.exe" /pdbtype:sept /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\ActionMapper.obj" \
	"$(INTDIR)\diutil.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\ActionMapper.res"

"$(OUTDIR)\actionmapper.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("actionmapper.dep")
!INCLUDE "actionmapper.dep"
!ELSE 
!MESSAGE Warning: cannot find "actionmapper.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ActionMapper - Win32 Release" || "$(CFG)" == "ActionMapper - Win32 Debug"
SOURCE=.\ActionMapper.cpp

"$(INTDIR)\ActionMapper.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ActionMapper.rc

"$(INTDIR)\ActionMapper.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=..\..\common\src\diutil.cpp

"$(INTDIR)\diutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\src\dxutil.cpp

"$(INTDIR)\dxutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

