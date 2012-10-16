# Microsoft Developer Studio Generated NMAKE File, Based on MazeConsoleClient.dsp
!IF "$(CFG)" == ""
CFG=MazeConsoleClient - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to MazeConsoleClient - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "MazeConsoleClient - Win32 Release" && "$(CFG)" != "MazeConsoleClient - Win32 Debug" && "$(CFG)" != "MazeConsoleClient - Win32 Debug Unicode" && "$(CFG)" != "MazeConsoleClient - Win32 Release Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MazeConsoleClient.mak" CFG="MazeConsoleClient - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MazeConsoleClient - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "MazeConsoleClient - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "MazeConsoleClient - Win32 Debug Unicode" (based on "Win32 (x86) Console Application")
!MESSAGE "MazeConsoleClient - Win32 Release Unicode" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "MazeConsoleClient - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\MazeConsoleClient.exe"


CLEAN :
	-@erase "$(INTDIR)\ConsoleGraphics.obj"
	-@erase "$(INTDIR)\dplay8client.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\Maze.obj"
	-@erase "$(INTDIR)\MazeApp.obj"
	-@erase "$(INTDIR)\MazeClient.obj"
	-@erase "$(INTDIR)\MazeServer.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MazeConsoleClient.exe"

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MazeConsoleClient.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\MazeConsoleClient.pdb" /machine:I386 /out:"$(OUTDIR)\MazeConsoleClient.exe" /stack:0x5000,0x5000
LINK32_OBJS= \
	"$(INTDIR)\ConsoleGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\dplay8client.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\Maze.obj" \
	"$(INTDIR)\MazeApp.obj" \
	"$(INTDIR)\MazeClient.obj" \
	"$(INTDIR)\MazeServer.obj"

"$(OUTDIR)\MazeConsoleClient.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\MazeConsoleClient.exe"


CLEAN :
	-@erase "$(INTDIR)\ConsoleGraphics.obj"
	-@erase "$(INTDIR)\dplay8client.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\Maze.obj"
	-@erase "$(INTDIR)\MazeApp.obj"
	-@erase "$(INTDIR)\MazeClient.obj"
	-@erase "$(INTDIR)\MazeServer.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\MazeConsoleClient.exe"
	-@erase "$(OUTDIR)\MazeConsoleClient.ilk"
	-@erase "$(OUTDIR)\MazeConsoleClient.pdb"

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MazeConsoleClient.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\MazeConsoleClient.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MazeConsoleClient.exe" /pdbtype:sept /stack:0x5000,0x5000
LINK32_OBJS= \
	"$(INTDIR)\ConsoleGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\dplay8client.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\Maze.obj" \
	"$(INTDIR)\MazeApp.obj" \
	"$(INTDIR)\MazeClient.obj" \
	"$(INTDIR)\MazeServer.obj"

"$(OUTDIR)\MazeConsoleClient.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug Unicode"

OUTDIR=.\Win32_Debug_Unicode
INTDIR=.\Win32_Debug_Unicode
# Begin Custom Macros
OutDir=.\Win32_Debug_Unicode
# End Custom Macros

ALL : "$(OUTDIR)\MazeConsoleClient.exe"


CLEAN :
	-@erase "$(INTDIR)\ConsoleGraphics.obj"
	-@erase "$(INTDIR)\dplay8client.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\Maze.obj"
	-@erase "$(INTDIR)\MazeApp.obj"
	-@erase "$(INTDIR)\MazeClient.obj"
	-@erase "$(INTDIR)\MazeServer.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\MazeConsoleClient.exe"
	-@erase "$(OUTDIR)\MazeConsoleClient.ilk"
	-@erase "$(OUTDIR)\MazeConsoleClient.pdb"

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MazeConsoleClient.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\MazeConsoleClient.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MazeConsoleClient.exe" /pdbtype:sept /stack:0x5000,0x5000
LINK32_OBJS= \
	"$(INTDIR)\ConsoleGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\dplay8client.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\Maze.obj" \
	"$(INTDIR)\MazeApp.obj" \
	"$(INTDIR)\MazeClient.obj" \
	"$(INTDIR)\MazeServer.obj"

"$(OUTDIR)\MazeConsoleClient.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Release Unicode"

OUTDIR=.\Win32_Release_Unicode
INTDIR=.\Win32_Release_Unicode
# Begin Custom Macros
OutDir=.\Win32_Release_Unicode
# End Custom Macros

ALL : "$(OUTDIR)\MazeConsoleClient.exe" "$(OUTDIR)\MazeConsoleClient.bsc"


CLEAN :
	-@erase "$(INTDIR)\ConsoleGraphics.obj"
	-@erase "$(INTDIR)\ConsoleGraphics.sbr"
	-@erase "$(INTDIR)\dplay8client.obj"
	-@erase "$(INTDIR)\dplay8client.sbr"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\dxutil.sbr"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\Main.sbr"
	-@erase "$(INTDIR)\Maze.obj"
	-@erase "$(INTDIR)\Maze.sbr"
	-@erase "$(INTDIR)\MazeApp.obj"
	-@erase "$(INTDIR)\MazeApp.sbr"
	-@erase "$(INTDIR)\MazeClient.obj"
	-@erase "$(INTDIR)\MazeClient.sbr"
	-@erase "$(INTDIR)\MazeServer.obj"
	-@erase "$(INTDIR)\MazeServer.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MazeConsoleClient.bsc"
	-@erase "$(OUTDIR)\MazeConsoleClient.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\..\..\common\include" /I "..\mazecommon" /D "NDEBUG" /D "_MBCS" /D "_CONSOLE" /D "WIN32" /D "_WIN32_DCOM" /D "UNICODE" /D "_UNICODE" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MazeConsoleClient.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ConsoleGraphics.sbr" \
	"$(INTDIR)\Main.sbr" \
	"$(INTDIR)\dplay8client.sbr" \
	"$(INTDIR)\dxutil.sbr" \
	"$(INTDIR)\Maze.sbr" \
	"$(INTDIR)\MazeApp.sbr" \
	"$(INTDIR)\MazeClient.sbr" \
	"$(INTDIR)\MazeServer.sbr"

"$(OUTDIR)\MazeConsoleClient.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\MazeConsoleClient.pdb" /machine:I386 /out:"$(OUTDIR)\MazeConsoleClient.exe" /stack:0x5000,0x5000
LINK32_OBJS= \
	"$(INTDIR)\ConsoleGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\dplay8client.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\Maze.obj" \
	"$(INTDIR)\MazeApp.obj" \
	"$(INTDIR)\MazeClient.obj" \
	"$(INTDIR)\MazeServer.obj"

"$(OUTDIR)\MazeConsoleClient.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MazeConsoleClient.dep")
!INCLUDE "MazeConsoleClient.dep"
!ELSE 
!MESSAGE Warning: cannot find "MazeConsoleClient.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "MazeConsoleClient - Win32 Release" || "$(CFG)" == "MazeConsoleClient - Win32 Debug" || "$(CFG)" == "MazeConsoleClient - Win32 Debug Unicode" || "$(CFG)" == "MazeConsoleClient - Win32 Release Unicode"
SOURCE=.\ConsoleGraphics.cpp

!IF  "$(CFG)" == "MazeConsoleClient - Win32 Release"


"$(INTDIR)\ConsoleGraphics.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug"


"$(INTDIR)\ConsoleGraphics.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug Unicode"


"$(INTDIR)\ConsoleGraphics.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Release Unicode"


"$(INTDIR)\ConsoleGraphics.obj"	"$(INTDIR)\ConsoleGraphics.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\Main.cpp

!IF  "$(CFG)" == "MazeConsoleClient - Win32 Release"


"$(INTDIR)\Main.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug"


"$(INTDIR)\Main.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug Unicode"


"$(INTDIR)\Main.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Release Unicode"


"$(INTDIR)\Main.obj"	"$(INTDIR)\Main.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\mazecommon\dplay8client.cpp

!IF  "$(CFG)" == "MazeConsoleClient - Win32 Release"


"$(INTDIR)\dplay8client.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug"


"$(INTDIR)\dplay8client.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug Unicode"


"$(INTDIR)\dplay8client.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Release Unicode"


"$(INTDIR)\dplay8client.obj"	"$(INTDIR)\dplay8client.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\..\common\src\dxutil.cpp

!IF  "$(CFG)" == "MazeConsoleClient - Win32 Release"


"$(INTDIR)\dxutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug"


"$(INTDIR)\dxutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug Unicode"


"$(INTDIR)\dxutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Release Unicode"


"$(INTDIR)\dxutil.obj"	"$(INTDIR)\dxutil.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\mazecommon\Maze.cpp

!IF  "$(CFG)" == "MazeConsoleClient - Win32 Release"


"$(INTDIR)\Maze.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug"


"$(INTDIR)\Maze.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug Unicode"


"$(INTDIR)\Maze.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Release Unicode"


"$(INTDIR)\Maze.obj"	"$(INTDIR)\Maze.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\MazeCommon\MazeApp.cpp

!IF  "$(CFG)" == "MazeConsoleClient - Win32 Release"


"$(INTDIR)\MazeApp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug"


"$(INTDIR)\MazeApp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug Unicode"


"$(INTDIR)\MazeApp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Release Unicode"


"$(INTDIR)\MazeApp.obj"	"$(INTDIR)\MazeApp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\mazecommon\MazeClient.cpp

!IF  "$(CFG)" == "MazeConsoleClient - Win32 Release"


"$(INTDIR)\MazeClient.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug"


"$(INTDIR)\MazeClient.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug Unicode"


"$(INTDIR)\MazeClient.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Release Unicode"


"$(INTDIR)\MazeClient.obj"	"$(INTDIR)\MazeClient.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\MazeCommon\MazeServer.cpp

!IF  "$(CFG)" == "MazeConsoleClient - Win32 Release"


"$(INTDIR)\MazeServer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug"


"$(INTDIR)\MazeServer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Debug Unicode"


"$(INTDIR)\MazeServer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "MazeConsoleClient - Win32 Release Unicode"


"$(INTDIR)\MazeServer.obj"	"$(INTDIR)\MazeServer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

