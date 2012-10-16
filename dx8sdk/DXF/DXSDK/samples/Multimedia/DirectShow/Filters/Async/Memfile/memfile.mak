# Microsoft Developer Studio Generated NMAKE File, Based on memfile.dsp
!IF "$(CFG)" == ""
CFG=memfile - Win32 Debug
!MESSAGE No configuration specified. Defaulting to memfile - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "memfile - Win32 Release" && "$(CFG)" != "memfile - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "memfile.mak" CFG="memfile - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "memfile - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "memfile - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "memfile - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\memfile.exe"

!ELSE 

ALL : "BaseClasses - Win32 Release" "asynbase - Win32 Release" "$(OUTDIR)\memfile.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"asynbase - Win32 ReleaseCLEAN" "BaseClasses - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\memfile.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\memfile.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GX /O2 /I "..\..\..\BaseClasses" /I "..\..\..\..\..\..\include" /I "..\include" /D "_CONSOLE" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D "NDEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /Fp"$(INTDIR)\memfile.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\memfile.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\..\BaseClasses\release\strmbase.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib asynbase.lib msvcrt.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\memfile.pdb" /machine:I386 /nodefaultlib /out:"$(OUTDIR)\memfile.exe" /libpath:"..\..\..\..\lib" /libpath:"..\base\release" /OPT:NOREF /OPT:ICF /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\memfile.obj" \
	"..\base\Release\asynbase.lib" \
	"..\..\BaseClasses\Release\STRMBASE.lib"

"$(OUTDIR)\memfile.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "memfile - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\memfile.exe"

!ELSE 

ALL : "BaseClasses - Win32 Debug" "asynbase - Win32 Debug" "$(OUTDIR)\memfile.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"asynbase - Win32 DebugCLEAN" "BaseClasses - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\memfile.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\memfile.exe"
	-@erase "$(OUTDIR)\memfile.ilk"
	-@erase "$(OUTDIR)\memfile.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /I "..\..\..\BaseClasses" /I "..\..\..\..\..\..\include" /I "..\include" /D "_CONSOLE" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D DBG=1 /D "DEBUG" /D "_DEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /Fp"$(INTDIR)\memfile.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\memfile.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\..\BaseClasses\debug\strmbasd.lib strmiids.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib asynbase.lib msvcrtd.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\memfile.pdb" /debug /machine:I386 /nodefaultlib /out:"$(OUTDIR)\memfile.exe" /pdbtype:sept /libpath:"..\..\..\..\lib" /libpath:"..\base\debug" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\memfile.obj" \
	"..\base\Debug\asynbase.lib" \
	"..\..\BaseClasses\debug\strmbasd.lib"

"$(OUTDIR)\memfile.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("memfile.dep")
!INCLUDE "memfile.dep"
!ELSE 
!MESSAGE Warning: cannot find "memfile.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "memfile - Win32 Release" || "$(CFG)" == "memfile - Win32 Debug"
SOURCE=.\memfile.cpp

"$(INTDIR)\memfile.obj" : $(SOURCE) "$(INTDIR)"


!IF  "$(CFG)" == "memfile - Win32 Release"

"asynbase - Win32 Release" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\async\base"
   $(MAKE) /$(MAKEFLAGS) /F .\asynbase.mak CFG="asynbase - Win32 Release" 
   cd "..\memfile"

"asynbase - Win32 ReleaseCLEAN" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\async\base"
   $(MAKE) /$(MAKEFLAGS) /F .\asynbase.mak CFG="asynbase - Win32 Release" RECURSE=1 CLEAN 
   cd "..\memfile"

!ELSEIF  "$(CFG)" == "memfile - Win32 Debug"

"asynbase - Win32 Debug" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\async\base"
   $(MAKE) /$(MAKEFLAGS) /F .\asynbase.mak CFG="asynbase - Win32 Debug" 
   cd "..\memfile"

"asynbase - Win32 DebugCLEAN" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\async\base"
   $(MAKE) /$(MAKEFLAGS) /F .\asynbase.mak CFG="asynbase - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\memfile"

!ENDIF 

!IF  "$(CFG)" == "memfile - Win32 Release"

!ELSEIF  "$(CFG)" == "memfile - Win32 Debug"

!ENDIF 

!IF  "$(CFG)" == "memfile - Win32 Release"

"BaseClasses - Win32 Release" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\BaseClasses"
   $(MAKE) /$(MAKEFLAGS) /F .\baseclasses.mak CFG="BaseClasses - Win32 Release" 
   cd "..\async\memfile"

"BaseClasses - Win32 ReleaseCLEAN" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\BaseClasses"
   $(MAKE) /$(MAKEFLAGS) /F .\baseclasses.mak CFG="BaseClasses - Win32 Release" RECURSE=1 CLEAN 
   cd "..\async\memfile"

!ELSEIF  "$(CFG)" == "memfile - Win32 Debug"

"BaseClasses - Win32 Debug" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\BaseClasses"
   $(MAKE) /$(MAKEFLAGS) /F .\baseclasses.mak CFG="BaseClasses - Win32 Debug" 
   cd "..\async\memfile"

"BaseClasses - Win32 DebugCLEAN" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\BaseClasses"
   $(MAKE) /$(MAKEFLAGS) /F .\baseclasses.mak CFG="BaseClasses - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\async\memfile"

!ENDIF 


!ENDIF 

