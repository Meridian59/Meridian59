# Microsoft Developer Studio Generated NMAKE File, Based on JukeboxASF.dsp
!IF "$(CFG)" == ""
CFG=JukeboxASF - Win32 Debug
!MESSAGE No configuration specified. Defaulting to JukeboxASF - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "JukeboxASF - Win32 Release" && "$(CFG)" != "JukeboxASF - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "JukeboxASF.mak" CFG="JukeboxASF - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "JukeboxASF - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "JukeboxASF - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "JukeboxASF - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\JukeboxASF.exe"


CLEAN :
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\JukeboxASF.obj"
	-@erase "$(INTDIR)\JukeboxASF.pch"
	-@erase "$(INTDIR)\JukeboxASF.res"
	-@erase "$(INTDIR)\JukeboxASFDlg.obj"
	-@erase "$(INTDIR)\keyprovider.obj"
	-@erase "$(INTDIR)\playvideo.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\JukeboxASF.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Gi /GX /O2 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\JukeboxASF.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\JukeboxASF.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\JukeboxASF.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=strmiids.lib ..\..\common\wmstub.lib ..\..\common\wmvcore.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\JukeboxASF.pdb" /machine:I386 /out:"$(OUTDIR)\JukeboxASF.exe" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\JukeboxASF.obj" \
	"$(INTDIR)\JukeboxASFDlg.obj" \
	"$(INTDIR)\keyprovider.obj" \
	"$(INTDIR)\playvideo.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\JukeboxASF.res"

"$(OUTDIR)\JukeboxASF.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "JukeboxASF - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\JukeboxASF.exe"


CLEAN :
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\JukeboxASF.obj"
	-@erase "$(INTDIR)\JukeboxASF.pch"
	-@erase "$(INTDIR)\JukeboxASF.res"
	-@erase "$(INTDIR)\JukeboxASFDlg.obj"
	-@erase "$(INTDIR)\keyprovider.obj"
	-@erase "$(INTDIR)\playvideo.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\JukeboxASF.exe"
	-@erase "$(OUTDIR)\JukeboxASF.ilk"
	-@erase "$(OUTDIR)\JukeboxASF.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /Gi /GX /Zi /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /I "..\..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\JukeboxASF.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\JukeboxASF.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\JukeboxASF.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=strmiids.lib ..\..\common\wmstub.lib ..\..\common\wmvcore.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\JukeboxASF.pdb" /debug /machine:I386 /out:"$(OUTDIR)\JukeboxASF.exe" /pdbtype:sept /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\JukeboxASF.obj" \
	"$(INTDIR)\JukeboxASFDlg.obj" \
	"$(INTDIR)\keyprovider.obj" \
	"$(INTDIR)\playvideo.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\JukeboxASF.res"

"$(OUTDIR)\JukeboxASF.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("JukeboxASF.dep")
!INCLUDE "JukeboxASF.dep"
!ELSE 
!MESSAGE Warning: cannot find "JukeboxASF.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "JukeboxASF - Win32 Release" || "$(CFG)" == "JukeboxASF - Win32 Debug"
SOURCE=.\globals.cpp

"$(INTDIR)\globals.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\JukeboxASF.pch"


SOURCE=.\JukeboxASF.cpp

"$(INTDIR)\JukeboxASF.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\JukeboxASF.pch"


SOURCE=.\JukeboxASFDlg.cpp

"$(INTDIR)\JukeboxASFDlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\JukeboxASF.pch"


SOURCE=.\keyprovider.cpp

"$(INTDIR)\keyprovider.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\JukeboxASF.pch"


SOURCE=.\playvideo.cpp

"$(INTDIR)\playvideo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\JukeboxASF.pch"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "JukeboxASF - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Gi /GX /O2 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\JukeboxASF.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\JukeboxASF.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "JukeboxASF - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /Gi /GX /Zi /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /I "..\..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\JukeboxASF.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\JukeboxASF.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\JukeboxASF.rc

"$(INTDIR)\JukeboxASF.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

