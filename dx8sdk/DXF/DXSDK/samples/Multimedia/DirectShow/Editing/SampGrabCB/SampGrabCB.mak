# Microsoft Developer Studio Generated NMAKE File, Based on SampGrabCB.dsp
!IF "$(CFG)" == ""
CFG=SampGrabCB - Win32 Debug
!MESSAGE No configuration specified. Defaulting to SampGrabCB - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "SampGrabCB - Win32 Release" && "$(CFG)" != "SampGrabCB - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SampGrabCB.mak" CFG="SampGrabCB - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SampGrabCB - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "SampGrabCB - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "SampGrabCB - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\SampGrabCB.exe"

!ELSE 

ALL : "BaseClasses - Win32 Release" "$(OUTDIR)\SampGrabCB.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"BaseClasses - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\SampGrabCB.obj"
	-@erase "$(INTDIR)\SampGrabCB.pch"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\SampGrabCB.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "RELEASE" /Fp"$(INTDIR)\SampGrabCB.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SampGrabCB.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\BaseClasses\Release\strmbase.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\SampGrabCB.pdb" /machine:I386 /nodefaultlib:"libcmt.lib" /out:"$(OUTDIR)\SampGrabCB.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\SampGrabCB.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"..\..\BaseClasses\Release\STRMBASE.lib"

"$(OUTDIR)\SampGrabCB.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "SampGrabCB - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\SampGrabCB.exe"

!ELSE 

ALL : "BaseClasses - Win32 Debug" "$(OUTDIR)\SampGrabCB.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"BaseClasses - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\SampGrabCB.obj"
	-@erase "$(INTDIR)\SampGrabCB.pch"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\SampGrabCB.exe"
	-@erase "$(OUTDIR)\SampGrabCB.ilk"
	-@erase "$(OUTDIR)\SampGrabCB.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "DEBUG" /Fp"$(INTDIR)\SampGrabCB.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\SampGrabCB.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\BaseClasses\debug\strmbasd.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\SampGrabCB.pdb" /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /out:"$(OUTDIR)\SampGrabCB.exe" /pdbtype:sept /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\SampGrabCB.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"..\..\BaseClasses\debug\strmbasd.lib"

"$(OUTDIR)\SampGrabCB.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("SampGrabCB.dep")
!INCLUDE "SampGrabCB.dep"
!ELSE 
!MESSAGE Warning: cannot find "SampGrabCB.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "SampGrabCB - Win32 Release" || "$(CFG)" == "SampGrabCB - Win32 Debug"
SOURCE=.\SampGrabCB.cpp

"$(INTDIR)\SampGrabCB.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\SampGrabCB.pch"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "SampGrabCB - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "RELEASE" /Fp"$(INTDIR)\SampGrabCB.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\SampGrabCB.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "SampGrabCB - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "DEBUG" /Fp"$(INTDIR)\SampGrabCB.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\SampGrabCB.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

!IF  "$(CFG)" == "SampGrabCB - Win32 Release"

"BaseClasses - Win32 Release" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\BaseClasses"
   $(MAKE) /$(MAKEFLAGS) /F .\baseclasses.mak CFG="BaseClasses - Win32 Release" 
   cd "..\Editing\SampGrabCB"

"BaseClasses - Win32 ReleaseCLEAN" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\BaseClasses"
   $(MAKE) /$(MAKEFLAGS) /F .\baseclasses.mak CFG="BaseClasses - Win32 Release" RECURSE=1 CLEAN 
   cd "..\Editing\SampGrabCB"

!ELSEIF  "$(CFG)" == "SampGrabCB - Win32 Debug"

"BaseClasses - Win32 Debug" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\BaseClasses"
   $(MAKE) /$(MAKEFLAGS) /F .\baseclasses.mak CFG="BaseClasses - Win32 Debug" 
   cd "..\Editing\SampGrabCB"

"BaseClasses - Win32 DebugCLEAN" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\BaseClasses"
   $(MAKE) /$(MAKEFLAGS) /F .\baseclasses.mak CFG="BaseClasses - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\Editing\SampGrabCB"

!ENDIF 


!ENDIF 

