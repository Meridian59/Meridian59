# Microsoft Developer Studio Generated NMAKE File, Based on MazeClient.dsp
!IF "$(CFG)" == ""
CFG=MazeClient - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to MazeClient - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "MazeClient - Win32 Release" && "$(CFG)" != "MazeClient - Win32 Debug" && "$(CFG)" != "MazeClient - Win32 Debug Unicode" && "$(CFG)" != "MazeClient - Win32 Release Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MazeClient.mak" CFG="MazeClient - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MazeClient - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "MazeClient - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "MazeClient - Win32 Debug Unicode" (based on "Win32 (x86) Application")
!MESSAGE "MazeClient - Win32 Release Unicode" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "MazeClient - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\MazeClient.exe"


CLEAN :
	-@erase "$(INTDIR)\d3dfile.obj"
	-@erase "$(INTDIR)\d3dfont.obj"
	-@erase "$(INTDIR)\D3DGraphics.obj"
	-@erase "$(INTDIR)\d3dsaver.obj"
	-@erase "$(INTDIR)\d3dutil.obj"
	-@erase "$(INTDIR)\dplay8client.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\Maze.obj"
	-@erase "$(INTDIR)\MazeApp.obj"
	-@erase "$(INTDIR)\MazeClient.obj"
	-@erase "$(INTDIR)\mazeclient.res"
	-@erase "$(INTDIR)\MazeServer.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MazeClient.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\..\..\common\include" /I "..\mazecommon" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "WIN32" /D "_WIN32_DCOM" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\mazeclient.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MazeClient.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=d3dx8dt.lib d3d8.lib d3dxof.lib dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\MazeClient.pdb" /machine:I386 /out:"$(OUTDIR)\MazeClient.exe" /stack:0x10000,0x10000
LINK32_OBJS= \
	"$(INTDIR)\D3DGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\d3dfile.obj" \
	"$(INTDIR)\d3dfont.obj" \
	"$(INTDIR)\d3dsaver.obj" \
	"$(INTDIR)\d3dutil.obj" \
	"$(INTDIR)\dplay8client.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\Maze.obj" \
	"$(INTDIR)\MazeApp.obj" \
	"$(INTDIR)\MazeClient.obj" \
	"$(INTDIR)\MazeServer.obj" \
	"$(INTDIR)\mazeclient.res"

"$(OUTDIR)\MazeClient.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MazeClient - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\MazeClient.exe"


CLEAN :
	-@erase "$(INTDIR)\d3dfile.obj"
	-@erase "$(INTDIR)\d3dfont.obj"
	-@erase "$(INTDIR)\D3DGraphics.obj"
	-@erase "$(INTDIR)\d3dsaver.obj"
	-@erase "$(INTDIR)\d3dutil.obj"
	-@erase "$(INTDIR)\dplay8client.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\Maze.obj"
	-@erase "$(INTDIR)\MazeApp.obj"
	-@erase "$(INTDIR)\MazeClient.obj"
	-@erase "$(INTDIR)\mazeclient.res"
	-@erase "$(INTDIR)\MazeServer.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\MazeClient.exe"
	-@erase "$(OUTDIR)\MazeClient.ilk"
	-@erase "$(OUTDIR)\MazeClient.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\common\include" /I "..\mazecommon" /D "_WINDOWS" /D "_DEBUG" /D "WIN32" /D "_WIN32_DCOM" /Fp"$(INTDIR)\MazeClient.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\mazeclient.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MazeClient.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=d3dx8dt.lib d3d8.lib d3dxof.lib dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\MazeClient.pdb" /debug /machine:I386 /nodefaultlib:"LIBC" /out:"$(OUTDIR)\MazeClient.exe" /pdbtype:sept /stack:0x10000,0x10000
LINK32_OBJS= \
	"$(INTDIR)\D3DGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\d3dfile.obj" \
	"$(INTDIR)\d3dfont.obj" \
	"$(INTDIR)\d3dsaver.obj" \
	"$(INTDIR)\d3dutil.obj" \
	"$(INTDIR)\dplay8client.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\Maze.obj" \
	"$(INTDIR)\MazeApp.obj" \
	"$(INTDIR)\MazeClient.obj" \
	"$(INTDIR)\MazeServer.obj" \
	"$(INTDIR)\mazeclient.res"

"$(OUTDIR)\MazeClient.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MazeClient - Win32 Debug Unicode"

OUTDIR=.\Win32_Debug_Unicode
INTDIR=.\Win32_Debug_Unicode
# Begin Custom Macros
OutDir=.\Win32_Debug_Unicode
# End Custom Macros

ALL : "$(OUTDIR)\MazeClient.exe"


CLEAN :
	-@erase "$(INTDIR)\d3dfile.obj"
	-@erase "$(INTDIR)\d3dfont.obj"
	-@erase "$(INTDIR)\D3DGraphics.obj"
	-@erase "$(INTDIR)\d3dsaver.obj"
	-@erase "$(INTDIR)\d3dutil.obj"
	-@erase "$(INTDIR)\dplay8client.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\Maze.obj"
	-@erase "$(INTDIR)\MazeApp.obj"
	-@erase "$(INTDIR)\MazeClient.obj"
	-@erase "$(INTDIR)\mazeclient.res"
	-@erase "$(INTDIR)\MazeServer.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\MazeClient.exe"
	-@erase "$(OUTDIR)\MazeClient.ilk"
	-@erase "$(OUTDIR)\MazeClient.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\..\common\include" /I "..\mazecommon" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "_WIN32_DCOM" /D "UNICODE" /D "_UNICODE" /Fp"$(INTDIR)\MazeClient.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\mazeclient.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MazeClient.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=d3dx8dt.lib d3d8.lib d3dxof.lib dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\MazeClient.pdb" /debug /machine:I386 /nodefaultlib:"LIBC" /out:"$(OUTDIR)\MazeClient.exe" /pdbtype:sept /stack:0x10000,0x10000
LINK32_OBJS= \
	"$(INTDIR)\D3DGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\d3dfile.obj" \
	"$(INTDIR)\d3dfont.obj" \
	"$(INTDIR)\d3dsaver.obj" \
	"$(INTDIR)\d3dutil.obj" \
	"$(INTDIR)\dplay8client.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\Maze.obj" \
	"$(INTDIR)\MazeApp.obj" \
	"$(INTDIR)\MazeClient.obj" \
	"$(INTDIR)\MazeServer.obj" \
	"$(INTDIR)\mazeclient.res"

"$(OUTDIR)\MazeClient.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "MazeClient - Win32 Release Unicode"

OUTDIR=.\Win32_Release_Unicode
INTDIR=.\Win32_Release_Unicode
# Begin Custom Macros
OutDir=.\Win32_Release_Unicode
# End Custom Macros

ALL : "$(OUTDIR)\MazeClient.exe"


CLEAN :
	-@erase "$(INTDIR)\d3dfile.obj"
	-@erase "$(INTDIR)\d3dfont.obj"
	-@erase "$(INTDIR)\D3DGraphics.obj"
	-@erase "$(INTDIR)\d3dsaver.obj"
	-@erase "$(INTDIR)\d3dutil.obj"
	-@erase "$(INTDIR)\dplay8client.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\Main.obj"
	-@erase "$(INTDIR)\Maze.obj"
	-@erase "$(INTDIR)\MazeApp.obj"
	-@erase "$(INTDIR)\MazeClient.obj"
	-@erase "$(INTDIR)\mazeclient.res"
	-@erase "$(INTDIR)\MazeServer.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MazeClient.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\..\..\common\include" /I "..\mazecommon" /D "NDEBUG" /D "_MBCS" /D "_WINDOWS" /D "WIN32" /D "_WIN32_DCOM" /D "UNICODE" /D "_UNICODE" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\mazeclient.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\MazeClient.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=d3dx8dt.lib d3d8.lib d3dxof.lib dplay.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\MazeClient.pdb" /machine:I386 /out:"$(OUTDIR)\MazeClient.exe" /stack:0x10000,0x10000
LINK32_OBJS= \
	"$(INTDIR)\D3DGraphics.obj" \
	"$(INTDIR)\Main.obj" \
	"$(INTDIR)\d3dfile.obj" \
	"$(INTDIR)\d3dfont.obj" \
	"$(INTDIR)\d3dsaver.obj" \
	"$(INTDIR)\d3dutil.obj" \
	"$(INTDIR)\dplay8client.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\Maze.obj" \
	"$(INTDIR)\MazeApp.obj" \
	"$(INTDIR)\MazeClient.obj" \
	"$(INTDIR)\MazeServer.obj" \
	"$(INTDIR)\mazeclient.res"

"$(OUTDIR)\MazeClient.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MazeClient.dep")
!INCLUDE "MazeClient.dep"
!ELSE 
!MESSAGE Warning: cannot find "MazeClient.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "MazeClient - Win32 Release" || "$(CFG)" == "MazeClient - Win32 Debug" || "$(CFG)" == "MazeClient - Win32 Debug Unicode" || "$(CFG)" == "MazeClient - Win32 Release Unicode"
SOURCE=.\D3DGraphics.cpp

"$(INTDIR)\D3DGraphics.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\Main.cpp

"$(INTDIR)\Main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=..\..\..\common\src\d3dfile.cpp

"$(INTDIR)\d3dfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\d3dfont.cpp

"$(INTDIR)\d3dfont.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\d3dsaver.cpp

"$(INTDIR)\d3dsaver.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\d3dutil.cpp

"$(INTDIR)\d3dutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\mazecommon\dplay8client.cpp

"$(INTDIR)\dplay8client.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\..\common\src\dxutil.cpp

"$(INTDIR)\dxutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\mazecommon\Maze.cpp

"$(INTDIR)\Maze.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\MazeCommon\MazeApp.cpp

"$(INTDIR)\MazeApp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\mazecommon\MazeClient.cpp

"$(INTDIR)\MazeClient.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\mazecommon\MazeServer.cpp

"$(INTDIR)\MazeServer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\mazeclient.rc

"$(INTDIR)\mazeclient.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

