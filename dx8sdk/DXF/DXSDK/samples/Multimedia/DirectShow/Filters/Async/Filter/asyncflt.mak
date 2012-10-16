# Microsoft Developer Studio Generated NMAKE File, Based on asyncflt.dsp
!IF "$(CFG)" == ""
CFG=async - Win32 Debug
!MESSAGE No configuration specified. Defaulting to async - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "async - Win32 Release" && "$(CFG)" != "async - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "asyncflt.mak" CFG="async - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "async - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "async - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "async - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\asyncflt.ax"

!ELSE 

ALL : "BaseClasses - Win32 Release" "asynbase - Win32 Release" "$(OUTDIR)\asyncflt.ax"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"asynbase - Win32 ReleaseCLEAN" "BaseClasses - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\asyncflt.obj"
	-@erase "$(INTDIR)\asyncflt.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\asyncflt.ax"
	-@erase "$(OUTDIR)\asyncflt.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Gz /MD /W3 /GX /O2 /I "..\..\..\BaseClasses" /I "..\..\..\..\..\..\include" /I "..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ASYNC_EXPORTS" /Fp"$(INTDIR)\asyncflt.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\asyncflt.res" /i "..\..\..\BaseClasses" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\asyncflt.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\base\release\asynbase.lib ..\..\..\BaseClasses\release\strmbase.lib quartz.lib uuid.lib winmm.lib msvcrt.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib odbc32.lib odbccp32.lib /nologo /entry:"DllEntryPoint@12" /dll /incremental:no /pdb:"$(OUTDIR)\asyncflt.pdb" /machine:I386 /nodefaultlib /def:".\asyncflt.def" /out:"$(OUTDIR)\asyncflt.ax" /implib:"$(OUTDIR)\asyncflt.lib" /libpath:"..\..\..\..\lib" /OPT:NOREF /OPT:ICF /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\asyncflt.obj" \
	"$(INTDIR)\asyncflt.res" \
	"..\base\Release\asynbase.lib" \
	"..\..\BaseClasses\Release\STRMBASE.lib"

"$(OUTDIR)\asyncflt.ax" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "async - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\asyncflt.ax"

!ELSE 

ALL : "BaseClasses - Win32 Debug" "asynbase - Win32 Debug" "$(OUTDIR)\asyncflt.ax"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"asynbase - Win32 DebugCLEAN" "BaseClasses - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\asyncflt.obj"
	-@erase "$(INTDIR)\asyncflt.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\asyncflt.ax"
	-@erase "$(OUTDIR)\asyncflt.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Gz /MDd /W3 /Z7 /I "..\..\..\BaseClasses" /I "..\..\..\..\..\..\include" /I "..\include" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D DBG=1 /D "DEBUG" /D "_DEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\asyncflt.res" /i "..\..\..\BaseClasses" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\asyncflt.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\..\BaseClasses\debug\strmbasd.lib ..\base\debug\asynbase.lib quartz.lib uuid.lib winmm.lib msvcrtd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib odbc32.lib odbccp32.lib /nologo /entry:"DllEntryPoint@12" /dll /pdb:none /machine:I386 /nodefaultlib /def:".\asyncflt.def" /out:"$(OUTDIR)\asyncflt.ax" /implib:"$(OUTDIR)\asyncflt.lib" /libpath:"..\..\..\..\lib" /stack:0x200000,0x200000
DEF_FILE= \
	".\asyncflt.def"
LINK32_OBJS= \
	"$(INTDIR)\asyncflt.obj" \
	"$(INTDIR)\asyncflt.res" \
	"..\base\Debug\asynbase.lib" \
	"..\..\BaseClasses\debug\strmbasd.lib"

"$(OUTDIR)\asyncflt.ax" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("asyncflt.dep")
!INCLUDE "asyncflt.dep"
!ELSE 
!MESSAGE Warning: cannot find "asyncflt.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "async - Win32 Release" || "$(CFG)" == "async - Win32 Debug"
SOURCE=.\asyncflt.cpp

"$(INTDIR)\asyncflt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\asyncflt.rc

"$(INTDIR)\asyncflt.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!IF  "$(CFG)" == "async - Win32 Release"

"asynbase - Win32 Release" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\async\base"
   $(MAKE) /$(MAKEFLAGS) /F .\asynbase.mak CFG="asynbase - Win32 Release" 
   cd "..\filter"

"asynbase - Win32 ReleaseCLEAN" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\async\base"
   $(MAKE) /$(MAKEFLAGS) /F .\asynbase.mak CFG="asynbase - Win32 Release" RECURSE=1 CLEAN 
   cd "..\filter"

!ELSEIF  "$(CFG)" == "async - Win32 Debug"

"asynbase - Win32 Debug" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\async\base"
   $(MAKE) /$(MAKEFLAGS) /F .\asynbase.mak CFG="asynbase - Win32 Debug" 
   cd "..\filter"

"asynbase - Win32 DebugCLEAN" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\async\base"
   $(MAKE) /$(MAKEFLAGS) /F .\asynbase.mak CFG="asynbase - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\filter"

!ENDIF 

!IF  "$(CFG)" == "async - Win32 Release"

"BaseClasses - Win32 Release" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\BaseClasses"
   $(MAKE) /$(MAKEFLAGS) /F .\baseclasses.mak CFG="BaseClasses - Win32 Release" 
   cd "..\async\filter"

"BaseClasses - Win32 ReleaseCLEAN" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\BaseClasses"
   $(MAKE) /$(MAKEFLAGS) /F .\baseclasses.mak CFG="BaseClasses - Win32 Release" RECURSE=1 CLEAN 
   cd "..\async\filter"

!ELSEIF  "$(CFG)" == "async - Win32 Debug"

"BaseClasses - Win32 Debug" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\BaseClasses"
   $(MAKE) /$(MAKEFLAGS) /F .\baseclasses.mak CFG="BaseClasses - Win32 Debug" 
   cd "..\async\filter"

"BaseClasses - Win32 DebugCLEAN" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\BaseClasses"
   $(MAKE) /$(MAKEFLAGS) /F .\baseclasses.mak CFG="BaseClasses - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\async\filter"

!ENDIF 


!ENDIF 

