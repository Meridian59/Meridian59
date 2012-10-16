# Microsoft Developer Studio Generated NMAKE File, Based on MazeServer.dsp
!IF "$(CFG)" == ""
CFG=MazeServer - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to MazeServer - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "MazeServer - Win32 Release" && "$(CFG)" != "MazeServer - Win32 Debug" && "$(CFG)" != "MazeServer - Win32 Debug Unicode" && "$(CFG)" != "MazeServer - Win32 Release Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MazeServer.mak" CFG="MazeServer - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MazeServer - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "MazeServer - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "MazeServer - Win32 Debug Unicode" (based on "Win32 (x86) Console Application")
!MESSAGE "MazeServer - Win32 Release Unicode" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "MazeServer - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\MazeServer.exe"


CLEAN :
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\Maze.obj"
	-@erase "$(INTDIR)\MazeServer.obj"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MazeServer.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\..\..\common\include" /I "..\mazecommon" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "WIN32" /D "_WIN32_DCOM" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MazeServer.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\MazeServer.pdb" /machine:I386 /out:"$(OUTDIR)\MazeServer.exe" /stack:0x80000,0x10000
LINK32_OBJS= \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\Maze.obj" \
	"$(INTDIR)\MazeServer.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\dxutil.obj"

"$(OUTDIR)\MazeServer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MazeServer - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\MazeServer.exe"


CLEAN :
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\Maze.obj"
	-@erase "$(INTDIR)\MazeServer.obj"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\MazeServer.exe"
	-@erase "$(OUTDIR)\MazeServer.ilk"
	-@erase "$(OUTDIR)\MazeServer.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\mazecommon" /I "..\..\..\common\include" /D "_CONSOLE" /D "_DEBUG" /D "WIN32" /D "_WIN32_DCOM" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MazeServer.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\MazeServer.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MazeServer.exe" /pdbtype:sept /stack:0x80000,0x10000
LINK32_OBJS= \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\Maze.obj" \
	"$(INTDIR)\MazeServer.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\dxutil.obj"

"$(OUTDIR)\MazeServer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MazeServer - Win32 Debug Unicode"

OUTDIR=.\Win32_Debug_Unicode
INTDIR=.\Win32_Debug_Unicode
# Begin Custom Macros
OutDir=.\Win32_Debug_Unicode
# End Custom Macros

ALL : "$(OUTDIR)\MazeServer.exe"


CLEAN :
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\Maze.obj"
	-@erase "$(INTDIR)\MazeServer.obj"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\MazeServer.exe"
	-@erase "$(OUTDIR)\MazeServer.ilk"
	-@erase "$(OUTDIR)\MazeServer.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\mazecommon" /I "..\..\..\common\include" /D "_DEBUG" /D "_CONSOLE" /D "WIN32" /D "_WIN32_DCOM" /D "UNICODE" /D "_UNICODE" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MazeServer.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\MazeServer.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MazeServer.exe" /pdbtype:sept /stack:0x80000,0x10000
LINK32_OBJS= \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\Maze.obj" \
	"$(INTDIR)\MazeServer.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\dxutil.obj"

"$(OUTDIR)\MazeServer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MazeServer - Win32 Release Unicode"

OUTDIR=.\Win32_Release_Unicode
INTDIR=.\Win32_Release_Unicode
# Begin Custom Macros
OutDir=.\Win32_Release_Unicode
# End Custom Macros

ALL : "$(OUTDIR)\MazeServer.exe"


CLEAN :
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\Maze.obj"
	-@erase "$(INTDIR)\MazeServer.obj"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MazeServer.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\..\..\common\include" /I "..\mazecommon" /D "NDEBUG" /D "_MBCS" /D "_CONSOLE" /D "WIN32" /D "_WIN32_DCOM" /D "UNICODE" /D "_UNICODE" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MazeServer.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\MazeServer.pdb" /machine:I386 /out:"$(OUTDIR)\MazeServer.exe" /stack:0x80000,0x10000
LINK32_OBJS= \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\Maze.obj" \
	"$(INTDIR)\MazeServer.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\dxutil.obj"

"$(OUTDIR)\MazeServer.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MazeServer.dep")
!INCLUDE "MazeServer.dep"
!ELSE 
!MESSAGE Warning: cannot find "MazeServer.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "MazeServer - Win32 Release" || "$(CFG)" == "MazeServer - Win32 Debug" || "$(CFG)" == "MazeServer - Win32 Debug Unicode" || "$(CFG)" == "MazeServer - Win32 Release Unicode"
SOURCE=.\Main.cpp

"$(INTDIR)\Main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=..\mazecommon\Maze.cpp

"$(INTDIR)\Maze.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\mazecommon\MazeServer.cpp

"$(INTDIR)\MazeServer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\server.cpp

"$(INTDIR)\server.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=..\..\..\common\src\dxutil.cpp

"$(INTDIR)\dxutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

