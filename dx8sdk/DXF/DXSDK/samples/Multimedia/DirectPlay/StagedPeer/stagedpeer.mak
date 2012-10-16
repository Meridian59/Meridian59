# Microsoft Developer Studio Generated NMAKE File, Based on stagedpeer.dsp
!IF "$(CFG)" == ""
CFG=StagedPeer - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to StagedPeer - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "StagedPeer - Win32 Release" && "$(CFG)" != "StagedPeer - Win32 Debug" && "$(CFG)" != "StagedPeer - Win32 Debug Unicode" && "$(CFG)" != "StagedPeer - Win32 Release Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "stagedpeer.mak" CFG="StagedPeer - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "StagedPeer - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "StagedPeer - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "StagedPeer - Win32 Debug Unicode" (based on "Win32 (x86) Application")
!MESSAGE "StagedPeer - Win32 Release Unicode" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "StagedPeer - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\stagedpeer.exe"


CLEAN :
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\NetConnect.obj"
	-@erase "$(INTDIR)\NetStage.obj"
	-@erase "$(INTDIR)\StagedPeer.obj"
	-@erase "$(INTDIR)\StagedPeer.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\stagedpeer.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "." /I "..\..\common\include" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "WIN32" /D "_WIN32_DCOM" /Fp"$(INTDIR)\stagedpeer.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\StagedPeer.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\stagedpeer.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\stagedpeer.pdb" /machine:I386 /out:"$(OUTDIR)\stagedpeer.exe" /stack:0x10000,0x10000
LINK32_OBJS= \
	"$(INTDIR)\NetStage.obj" \
	"$(INTDIR)\StagedPeer.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\NetConnect.obj" \
	"$(INTDIR)\StagedPeer.res"

"$(OUTDIR)\stagedpeer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "StagedPeer - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\stagedpeer.exe"


CLEAN :
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\NetConnect.obj"
	-@erase "$(INTDIR)\NetStage.obj"
	-@erase "$(INTDIR)\StagedPeer.obj"
	-@erase "$(INTDIR)\StagedPeer.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\stagedpeer.exe"
	-@erase "$(OUTDIR)\stagedpeer.ilk"
	-@erase "$(OUTDIR)\stagedpeer.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\common\include" /D "_WINDOWS" /D "_DEBUG" /D "WIN32" /D "_WIN32_DCOM" /Fp"$(INTDIR)\stagedpeer.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\StagedPeer.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\stagedpeer.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\stagedpeer.pdb" /debug /machine:I386 /nodefaultlib:"LIBC" /out:"$(OUTDIR)\stagedpeer.exe" /pdbtype:sept /stack:0x10000,0x10000
LINK32_OBJS= \
	"$(INTDIR)\NetStage.obj" \
	"$(INTDIR)\StagedPeer.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\NetConnect.obj" \
	"$(INTDIR)\StagedPeer.res"

"$(OUTDIR)\stagedpeer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "StagedPeer - Win32 Debug Unicode"

OUTDIR=.\Win32_Debug_Unicode
INTDIR=.\Win32_Debug_Unicode
# Begin Custom Macros
OutDir=.\Win32_Debug_Unicode
# End Custom Macros

ALL : "$(OUTDIR)\stagedpeer.exe"


CLEAN :
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\NetConnect.obj"
	-@erase "$(INTDIR)\NetStage.obj"
	-@erase "$(INTDIR)\StagedPeer.obj"
	-@erase "$(INTDIR)\StagedPeer.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\stagedpeer.exe"
	-@erase "$(OUTDIR)\stagedpeer.ilk"
	-@erase "$(OUTDIR)\stagedpeer.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\common\include" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "_WIN32_DCOM" /D "UNICODE" /D "_UNICODE" /Fp"$(INTDIR)\stagedpeer.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\StagedPeer.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\stagedpeer.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\stagedpeer.pdb" /debug /machine:I386 /nodefaultlib:"LIBC" /out:"$(OUTDIR)\stagedpeer.exe" /pdbtype:sept /stack:0x10000,0x10000
LINK32_OBJS= \
	"$(INTDIR)\NetStage.obj" \
	"$(INTDIR)\StagedPeer.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\NetConnect.obj" \
	"$(INTDIR)\StagedPeer.res"

"$(OUTDIR)\stagedpeer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "StagedPeer - Win32 Release Unicode"

OUTDIR=.\Win32_Release_Unicode
INTDIR=.\Win32_Release_Unicode
# Begin Custom Macros
OutDir=.\Win32_Release_Unicode
# End Custom Macros

ALL : "$(OUTDIR)\stagedpeer.exe"


CLEAN :
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\NetConnect.obj"
	-@erase "$(INTDIR)\NetStage.obj"
	-@erase "$(INTDIR)\StagedPeer.obj"
	-@erase "$(INTDIR)\StagedPeer.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\stagedpeer.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "." /I "..\..\common\include" /D "NDEBUG" /D "_MBCS" /D "_WINDOWS" /D "WIN32" /D "_WIN32_DCOM" /D "UNICODE" /D "_UNICODE" /Fp"$(INTDIR)\stagedpeer.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\StagedPeer.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\stagedpeer.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\stagedpeer.pdb" /machine:I386 /out:"$(OUTDIR)\stagedpeer.exe" /stack:0x10000,0x10000
LINK32_OBJS= \
	"$(INTDIR)\NetStage.obj" \
	"$(INTDIR)\StagedPeer.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\NetConnect.obj" \
	"$(INTDIR)\StagedPeer.res"

"$(OUTDIR)\stagedpeer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("stagedpeer.dep")
!INCLUDE "stagedpeer.dep"
!ELSE 
!MESSAGE Warning: cannot find "stagedpeer.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "StagedPeer - Win32 Release" || "$(CFG)" == "StagedPeer - Win32 Debug" || "$(CFG)" == "StagedPeer - Win32 Debug Unicode" || "$(CFG)" == "StagedPeer - Win32 Release Unicode"
SOURCE=.\NetStage.cpp

"$(INTDIR)\NetStage.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\StagedPeer.cpp

"$(INTDIR)\StagedPeer.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\StagedPeer.rc

"$(INTDIR)\StagedPeer.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=..\..\common\src\dxutil.cpp

"$(INTDIR)\dxutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\src\NetConnect.cpp

"$(INTDIR)\NetConnect.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

