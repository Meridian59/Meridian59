# Microsoft Developer Studio Generated NMAKE File, Based on duelvoice.dsp
!IF "$(CFG)" == ""
CFG=duelvoice - Win32 Release
!MESSAGE No configuration specified. Defaulting to duelvoice - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "duelvoice - Win32 Release" && "$(CFG)" != "duelvoice - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "duelvoice.mak" CFG="duelvoice - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "duelvoice - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "duelvoice - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "duelvoice - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\duelvoice.exe"


CLEAN :
	-@erase "$(INTDIR)\ddutil.obj"
	-@erase "$(INTDIR)\diutil.obj"
	-@erase "$(INTDIR)\dpconnect.obj"
	-@erase "$(INTDIR)\dputil.obj"
	-@erase "$(INTDIR)\dsutil.obj"
	-@erase "$(INTDIR)\duel.obj"
	-@erase "$(INTDIR)\duelvoice.res"
	-@erase "$(INTDIR)\gameproc.obj"
	-@erase "$(INTDIR)\gfx.obj"
	-@erase "$(INTDIR)\lobby.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\duelvoice.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\duelvoice.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\duelvoice.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\duelvoice.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=uuid.lib comctl32.lib ddraw.lib dinput.lib dsound.lib dplayx.lib dxguid.lib dxerr8.lib ole32.lib winmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\duelvoice.pdb" /machine:I386 /out:"$(OUTDIR)\duelvoice.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ddutil.obj" \
	"$(INTDIR)\diutil.obj" \
	"$(INTDIR)\dpconnect.obj" \
	"$(INTDIR)\dputil.obj" \
	"$(INTDIR)\dsutil.obj" \
	"$(INTDIR)\duel.obj" \
	"$(INTDIR)\gameproc.obj" \
	"$(INTDIR)\gfx.obj" \
	"$(INTDIR)\lobby.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\duelvoice.res"

"$(OUTDIR)\duelvoice.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "duelvoice - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\duelvoice.exe" "$(OUTDIR)\duelvoice.bsc"


CLEAN :
	-@erase "$(INTDIR)\ddutil.obj"
	-@erase "$(INTDIR)\ddutil.sbr"
	-@erase "$(INTDIR)\diutil.obj"
	-@erase "$(INTDIR)\diutil.sbr"
	-@erase "$(INTDIR)\dpconnect.obj"
	-@erase "$(INTDIR)\dpconnect.sbr"
	-@erase "$(INTDIR)\dputil.obj"
	-@erase "$(INTDIR)\dputil.sbr"
	-@erase "$(INTDIR)\dsutil.obj"
	-@erase "$(INTDIR)\dsutil.sbr"
	-@erase "$(INTDIR)\duel.obj"
	-@erase "$(INTDIR)\duel.sbr"
	-@erase "$(INTDIR)\duelvoice.res"
	-@erase "$(INTDIR)\gameproc.obj"
	-@erase "$(INTDIR)\gameproc.sbr"
	-@erase "$(INTDIR)\gfx.obj"
	-@erase "$(INTDIR)\gfx.sbr"
	-@erase "$(INTDIR)\lobby.obj"
	-@erase "$(INTDIR)\lobby.sbr"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\util.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\duelvoice.bsc"
	-@erase "$(OUTDIR)\duelvoice.exe"
	-@erase "$(OUTDIR)\duelvoice.ilk"
	-@erase "$(OUTDIR)\duelvoice.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\..\common\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\duelvoice.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\duelvoice.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\duelvoice.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ddutil.sbr" \
	"$(INTDIR)\diutil.sbr" \
	"$(INTDIR)\dpconnect.sbr" \
	"$(INTDIR)\dputil.sbr" \
	"$(INTDIR)\dsutil.sbr" \
	"$(INTDIR)\duel.sbr" \
	"$(INTDIR)\gameproc.sbr" \
	"$(INTDIR)\gfx.sbr" \
	"$(INTDIR)\lobby.sbr" \
	"$(INTDIR)\util.sbr"

"$(OUTDIR)\duelvoice.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=uuid.lib comctl32.lib ddraw.lib dinput.lib dsound.lib dplayx.lib dxguid.lib dxerr8.lib ole32.lib winmm.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\duelvoice.pdb" /debug /machine:I386 /out:"$(OUTDIR)\duelvoice.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ddutil.obj" \
	"$(INTDIR)\diutil.obj" \
	"$(INTDIR)\dpconnect.obj" \
	"$(INTDIR)\dputil.obj" \
	"$(INTDIR)\dsutil.obj" \
	"$(INTDIR)\duel.obj" \
	"$(INTDIR)\gameproc.obj" \
	"$(INTDIR)\gfx.obj" \
	"$(INTDIR)\lobby.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\duelvoice.res"

"$(OUTDIR)\duelvoice.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("duelvoice.dep")
!INCLUDE "duelvoice.dep"
!ELSE 
!MESSAGE Warning: cannot find "duelvoice.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "duelvoice - Win32 Release" || "$(CFG)" == "duelvoice - Win32 Debug"
SOURCE=.\ddutil.cpp

!IF  "$(CFG)" == "duelvoice - Win32 Release"


"$(INTDIR)\ddutil.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "duelvoice - Win32 Debug"


"$(INTDIR)\ddutil.obj"	"$(INTDIR)\ddutil.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\diutil.cpp

!IF  "$(CFG)" == "duelvoice - Win32 Release"


"$(INTDIR)\diutil.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "duelvoice - Win32 Debug"


"$(INTDIR)\diutil.obj"	"$(INTDIR)\diutil.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\dpconnect.cpp

!IF  "$(CFG)" == "duelvoice - Win32 Release"


"$(INTDIR)\dpconnect.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "duelvoice - Win32 Debug"


"$(INTDIR)\dpconnect.obj"	"$(INTDIR)\dpconnect.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\dputil.cpp

!IF  "$(CFG)" == "duelvoice - Win32 Release"


"$(INTDIR)\dputil.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "duelvoice - Win32 Debug"


"$(INTDIR)\dputil.obj"	"$(INTDIR)\dputil.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\dsutil.cpp

!IF  "$(CFG)" == "duelvoice - Win32 Release"


"$(INTDIR)\dsutil.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "duelvoice - Win32 Debug"


"$(INTDIR)\dsutil.obj"	"$(INTDIR)\dsutil.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\duel.cpp

!IF  "$(CFG)" == "duelvoice - Win32 Release"


"$(INTDIR)\duel.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "duelvoice - Win32 Debug"


"$(INTDIR)\duel.obj"	"$(INTDIR)\duel.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\duelvoice.rc

"$(INTDIR)\duelvoice.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\gameproc.cpp

!IF  "$(CFG)" == "duelvoice - Win32 Release"


"$(INTDIR)\gameproc.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "duelvoice - Win32 Debug"


"$(INTDIR)\gameproc.obj"	"$(INTDIR)\gameproc.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\gfx.cpp

!IF  "$(CFG)" == "duelvoice - Win32 Release"


"$(INTDIR)\gfx.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "duelvoice - Win32 Debug"


"$(INTDIR)\gfx.obj"	"$(INTDIR)\gfx.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\lobby.cpp

!IF  "$(CFG)" == "duelvoice - Win32 Release"


"$(INTDIR)\lobby.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "duelvoice - Win32 Debug"


"$(INTDIR)\lobby.obj"	"$(INTDIR)\lobby.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\util.cpp

!IF  "$(CFG)" == "duelvoice - Win32 Release"


"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "duelvoice - Win32 Debug"


"$(INTDIR)\util.obj"	"$(INTDIR)\util.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

