# Microsoft Developer Studio Generated NMAKE File, Based on dxtex.dsp
!IF "$(CFG)" == ""
CFG=dxtex - Win32 Debug
!MESSAGE No configuration specified. Defaulting to dxtex - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "dxtex - Win32 Release" && "$(CFG)" != "dxtex - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dxtex.mak" CFG="dxtex - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dxtex - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "dxtex - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "dxtex - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\dxtex.exe"


CLEAN :
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\dialogs.obj"
	-@erase "$(INTDIR)\Dxtex.obj"
	-@erase "$(INTDIR)\Dxtex.res"
	-@erase "$(INTDIR)\DxtexDoc.obj"
	-@erase "$(INTDIR)\DxtexView.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\dxtex.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\dxtex.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Dxtex.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dxtex.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=ddraw.lib d3d8.lib d3dx8.lib dxguid.lib version.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\dxtex.pdb" /machine:I386 /out:"$(OUTDIR)\dxtex.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\dialogs.obj" \
	"$(INTDIR)\Dxtex.obj" \
	"$(INTDIR)\DxtexDoc.obj" \
	"$(INTDIR)\DxtexView.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Dxtex.res"

"$(OUTDIR)\dxtex.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dxtex - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\dxtex.exe" "$(OUTDIR)\dxtex.bsc"


CLEAN :
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\ChildFrm.sbr"
	-@erase "$(INTDIR)\dialogs.obj"
	-@erase "$(INTDIR)\dialogs.sbr"
	-@erase "$(INTDIR)\Dxtex.obj"
	-@erase "$(INTDIR)\Dxtex.res"
	-@erase "$(INTDIR)\Dxtex.sbr"
	-@erase "$(INTDIR)\DxtexDoc.obj"
	-@erase "$(INTDIR)\DxtexDoc.sbr"
	-@erase "$(INTDIR)\DxtexView.obj"
	-@erase "$(INTDIR)\DxtexView.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\dxtex.bsc"
	-@erase "$(OUTDIR)\dxtex.exe"
	-@erase "$(OUTDIR)\dxtex.ilk"
	-@erase "$(OUTDIR)\dxtex.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\dxtex.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\Dxtex.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dxtex.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ChildFrm.sbr" \
	"$(INTDIR)\dialogs.sbr" \
	"$(INTDIR)\Dxtex.sbr" \
	"$(INTDIR)\DxtexDoc.sbr" \
	"$(INTDIR)\DxtexView.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\StdAfx.sbr"

"$(OUTDIR)\dxtex.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=d3d8.lib d3dx8dt.lib dxguid.lib version.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\dxtex.pdb" /debug /machine:I386 /out:"$(OUTDIR)\dxtex.exe" /pdbtype:sept /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\dialogs.obj" \
	"$(INTDIR)\Dxtex.obj" \
	"$(INTDIR)\DxtexDoc.obj" \
	"$(INTDIR)\DxtexView.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\Dxtex.res"

"$(OUTDIR)\dxtex.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("dxtex.dep")
!INCLUDE "dxtex.dep"
!ELSE 
!MESSAGE Warning: cannot find "dxtex.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "dxtex - Win32 Release" || "$(CFG)" == "dxtex - Win32 Debug"
SOURCE=.\ChildFrm.cpp

!IF  "$(CFG)" == "dxtex - Win32 Release"


"$(INTDIR)\ChildFrm.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dxtex - Win32 Debug"


"$(INTDIR)\ChildFrm.obj"	"$(INTDIR)\ChildFrm.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\dialogs.cpp

!IF  "$(CFG)" == "dxtex - Win32 Release"


"$(INTDIR)\dialogs.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dxtex - Win32 Debug"


"$(INTDIR)\dialogs.obj"	"$(INTDIR)\dialogs.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\Dxtex.cpp

!IF  "$(CFG)" == "dxtex - Win32 Release"


"$(INTDIR)\Dxtex.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dxtex - Win32 Debug"


"$(INTDIR)\Dxtex.obj"	"$(INTDIR)\Dxtex.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\Dxtex.rc

"$(INTDIR)\Dxtex.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\DxtexDoc.cpp

!IF  "$(CFG)" == "dxtex - Win32 Release"


"$(INTDIR)\DxtexDoc.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dxtex - Win32 Debug"


"$(INTDIR)\DxtexDoc.obj"	"$(INTDIR)\DxtexDoc.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\DxtexView.cpp

!IF  "$(CFG)" == "dxtex - Win32 Release"


"$(INTDIR)\DxtexView.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dxtex - Win32 Debug"


"$(INTDIR)\DxtexView.obj"	"$(INTDIR)\DxtexView.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\MainFrm.cpp

!IF  "$(CFG)" == "dxtex - Win32 Release"


"$(INTDIR)\MainFrm.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dxtex - Win32 Debug"


"$(INTDIR)\MainFrm.obj"	"$(INTDIR)\MainFrm.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "dxtex - Win32 Release"


"$(INTDIR)\StdAfx.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "dxtex - Win32 Debug"


"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

