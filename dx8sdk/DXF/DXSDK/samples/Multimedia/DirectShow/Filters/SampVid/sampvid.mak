# Microsoft Developer Studio Generated NMAKE File, Based on sampvid.dsp
!IF "$(CFG)" == ""
CFG=sampvid - Win32 Debug
!MESSAGE No configuration specified. Defaulting to sampvid - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "sampvid - Win32 Release" && "$(CFG)" != "sampvid - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sampvid.mak" CFG="sampvid - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sampvid - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "sampvid - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "sampvid - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\sampvid.ax"


CLEAN :
	-@erase "$(INTDIR)\sampvid.obj"
	-@erase "$(INTDIR)\sampvid.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\videotxt.obj"
	-@erase "$(INTDIR)\vidprop.obj"
	-@erase "$(OUTDIR)\sampvid.ax"
	-@erase "$(OUTDIR)\sampvid.exp"
	-@erase "$(OUTDIR)\sampvid.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Gz /MD /W3 /Gi /O2 /Ob0 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D "NDEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\sampvid.res" /i "..\..\BaseClasses" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\sampvid.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\BaseClasses\release\strmbase.lib quartz.lib winmm.lib msvcrt.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib /nologo /entry:"DllEntryPoint@12" /dll /incremental:no /pdb:"$(OUTDIR)\sampvid.pdb" /machine:I386 /nodefaultlib /def:".\sampvid.def" /out:"$(OUTDIR)\sampvid.ax" /implib:"$(OUTDIR)\sampvid.lib" /libpath:"..\..\..\..\lib" /stack:0x200000,0x200000
DEF_FILE= \
	".\sampvid.def"
LINK32_OBJS= \
	"$(INTDIR)\sampvid.obj" \
	"$(INTDIR)\videotxt.obj" \
	"$(INTDIR)\vidprop.obj" \
	"$(INTDIR)\sampvid.res"

"$(OUTDIR)\sampvid.ax" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "sampvid - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\sampvid.ax"


CLEAN :
	-@erase "$(INTDIR)\sampvid.obj"
	-@erase "$(INTDIR)\sampvid.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\videotxt.obj"
	-@erase "$(INTDIR)\vidprop.obj"
	-@erase "$(OUTDIR)\sampvid.ax"
	-@erase "$(OUTDIR)\sampvid.exp"
	-@erase "$(OUTDIR)\sampvid.ilk"
	-@erase "$(OUTDIR)\sampvid.lib"
	-@erase "$(OUTDIR)\sampvid.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Gz /MDd /W3 /Gm /Zi /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /D "INC_OLE2" /D "STRICT" /D _WIN32_WINNT=0x0400 /D "WIN32" /D "_WIN32" /D "_MT" /D "_DLL" /D _X86_=1 /D WINVER=0x0400 /D DBG=1 /D "DEBUG" /D "_DEBUG" /D try=__try /D except=__except /D leave=__leave /D finally=__finally /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\sampvid.res" /i "..\..\BaseClasses" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\sampvid.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\BaseClasses\debug\strmbasd.lib quartz.lib winmm.lib msvcrtd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib /nologo /entry:"DllEntryPoint@12" /dll /incremental:yes /pdb:"$(OUTDIR)\sampvid.pdb" /debug /machine:I386 /nodefaultlib /def:".\sampvid.def" /out:"$(OUTDIR)\sampvid.ax" /implib:"$(OUTDIR)\sampvid.lib" /pdbtype:sept /libpath:"..\..\..\..\lib" /stack:0x200000,0x200000
DEF_FILE= \
	".\sampvid.def"
LINK32_OBJS= \
	"$(INTDIR)\sampvid.obj" \
	"$(INTDIR)\videotxt.obj" \
	"$(INTDIR)\vidprop.obj" \
	"$(INTDIR)\sampvid.res"

"$(OUTDIR)\sampvid.ax" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("sampvid.dep")
!INCLUDE "sampvid.dep"
!ELSE 
!MESSAGE Warning: cannot find "sampvid.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "sampvid - Win32 Release" || "$(CFG)" == "sampvid - Win32 Debug"
SOURCE=.\sampvid.cpp

"$(INTDIR)\sampvid.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sampvid.rc

"$(INTDIR)\sampvid.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\videotxt.cpp

"$(INTDIR)\videotxt.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vidprop.cpp

"$(INTDIR)\vidprop.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

