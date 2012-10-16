# Microsoft Developer Studio Generated NMAKE File, Based on dump.dsp
!IF "$(CFG)" == ""
CFG=dump - Win32 Debug
!MESSAGE No configuration specified. Defaulting to dump - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "dump - Win32 Release" && "$(CFG)" != "dump - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dump.mak" CFG="dump - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dump - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dump - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "dump - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\dump.ax"


CLEAN :
	-@erase "$(INTDIR)\dump.obj"
	-@erase "$(INTDIR)\dump.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\dump.ax"
	-@erase "$(OUTDIR)\dump.exp"
	-@erase "$(OUTDIR)\dump.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Gz /MD /W3 /O2 /Ob0 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D "NDEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\dump.res" /i "..\..\BaseClasses" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dump.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\BaseClasses\release\strmbase.lib quartz.lib winmm.lib msvcrt.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib /nologo /entry:"DllEntryPoint@12" /dll /incremental:no /pdb:"$(OUTDIR)\dump.pdb" /machine:I386 /nodefaultlib /def:".\dump.def" /out:"$(OUTDIR)\dump.ax" /implib:"$(OUTDIR)\dump.lib" /libpath:"..\..\..\..\lib" /OPT:NOREF /OPT:ICF /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\dump.obj" \
	"$(INTDIR)\dump.res"

"$(OUTDIR)\dump.ax" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dump - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\dump.ax"


CLEAN :
	-@erase "$(INTDIR)\dump.obj"
	-@erase "$(INTDIR)\dump.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\dump.ax"
	-@erase "$(OUTDIR)\dump.exp"
	-@erase "$(OUTDIR)\dump.ilk"
	-@erase "$(OUTDIR)\dump.lib"
	-@erase "$(OUTDIR)\dump.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Gz /MDd /W3 /Zi /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D DBG=1 /D "DEBUG" /D "_DEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\dump.res" /i "..\..\BaseClasses" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dump.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\BaseClasses\debug\strmbasd.lib quartz.lib winmm.lib msvcrtd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib /nologo /entry:"DllEntryPoint@12" /dll /incremental:yes /pdb:"$(OUTDIR)\dump.pdb" /debug /machine:I386 /nodefaultlib /def:".\dump.def" /out:"$(OUTDIR)\dump.ax" /implib:"$(OUTDIR)\dump.lib" /pdbtype:sept /libpath:"..\..\..\..\lib" /stack:0x200000,0x200000
DEF_FILE= \
	".\dump.def"
LINK32_OBJS= \
	"$(INTDIR)\dump.obj" \
	"$(INTDIR)\dump.res"

"$(OUTDIR)\dump.ax" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("dump.dep")
!INCLUDE "dump.dep"
!ELSE 
!MESSAGE Warning: cannot find "dump.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "dump - Win32 Release" || "$(CFG)" == "dump - Win32 Debug"
SOURCE=.\dump.cpp

"$(INTDIR)\dump.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\dump.rc

"$(INTDIR)\dump.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

