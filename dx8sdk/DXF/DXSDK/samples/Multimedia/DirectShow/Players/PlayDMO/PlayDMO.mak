# Microsoft Developer Studio Generated NMAKE File, Based on PlayDMO.dsp
!IF "$(CFG)" == ""
CFG=PlayDMO - Win32 Debug
!MESSAGE No configuration specified. Defaulting to PlayDMO - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "PlayDMO - Win32 Release" && "$(CFG)" != "PlayDMO - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PlayDMO.mak" CFG="PlayDMO - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PlayDMO - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PlayDMO - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "PlayDMO - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\PlayDMO.exe"


CLEAN :
	-@erase "$(INTDIR)\dshowutil.obj"
	-@erase "$(INTDIR)\mfcdmoutil.obj"
	-@erase "$(INTDIR)\mfcutil.obj"
	-@erase "$(INTDIR)\namedguid.obj"
	-@erase "$(INTDIR)\PlayDMO.obj"
	-@erase "$(INTDIR)\PlayDMO.pch"
	-@erase "$(INTDIR)\PlayDMO.res"
	-@erase "$(INTDIR)\PlayDMODlg.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\PlayDMO.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "..\..\common" /I "..\..\..\..\..\include" /I "..\..\baseclasses" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\PlayDMO.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\PlayDMO.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PlayDMO.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dmoguids.lib msdmo.lib ..\..\BaseClasses\Release\strmbase.lib winmm.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\PlayDMO.pdb" /machine:I386 /nodefaultlib:"libcmt.lib" /out:"$(OUTDIR)\PlayDMO.exe" /OPT:NOREF /OPT:ICF /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\dshowutil.obj" \
	"$(INTDIR)\mfcdmoutil.obj" \
	"$(INTDIR)\mfcutil.obj" \
	"$(INTDIR)\namedguid.obj" \
	"$(INTDIR)\PlayDMO.obj" \
	"$(INTDIR)\PlayDMODlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\PlayDMO.res"

"$(OUTDIR)\PlayDMO.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PlayDMO - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\PlayDMO.exe" "$(OUTDIR)\PlayDMO.bsc"


CLEAN :
	-@erase "$(INTDIR)\dshowutil.obj"
	-@erase "$(INTDIR)\dshowutil.sbr"
	-@erase "$(INTDIR)\mfcdmoutil.obj"
	-@erase "$(INTDIR)\mfcdmoutil.sbr"
	-@erase "$(INTDIR)\mfcutil.obj"
	-@erase "$(INTDIR)\mfcutil.sbr"
	-@erase "$(INTDIR)\namedguid.obj"
	-@erase "$(INTDIR)\namedguid.sbr"
	-@erase "$(INTDIR)\PlayDMO.obj"
	-@erase "$(INTDIR)\PlayDMO.pch"
	-@erase "$(INTDIR)\PlayDMO.res"
	-@erase "$(INTDIR)\PlayDMO.sbr"
	-@erase "$(INTDIR)\PlayDMODlg.obj"
	-@erase "$(INTDIR)\PlayDMODlg.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\PlayDMO.bsc"
	-@erase "$(OUTDIR)\PlayDMO.exe"
	-@erase "$(OUTDIR)\PlayDMO.ilk"
	-@erase "$(OUTDIR)\PlayDMO.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\common" /I "..\..\..\..\..\include" /I "..\..\baseclasses" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\PlayDMO.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\PlayDMO.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PlayDMO.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\dshowutil.sbr" \
	"$(INTDIR)\mfcdmoutil.sbr" \
	"$(INTDIR)\mfcutil.sbr" \
	"$(INTDIR)\namedguid.sbr" \
	"$(INTDIR)\PlayDMO.sbr" \
	"$(INTDIR)\PlayDMODlg.sbr" \
	"$(INTDIR)\StdAfx.sbr"

"$(OUTDIR)\PlayDMO.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=dmoguids.lib msdmo.lib ..\..\BaseClasses\Debug\strmbasd.lib winmm.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\PlayDMO.pdb" /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /out:"$(OUTDIR)\PlayDMO.exe" /pdbtype:sept /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\dshowutil.obj" \
	"$(INTDIR)\mfcdmoutil.obj" \
	"$(INTDIR)\mfcutil.obj" \
	"$(INTDIR)\namedguid.obj" \
	"$(INTDIR)\PlayDMO.obj" \
	"$(INTDIR)\PlayDMODlg.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\PlayDMO.res"

"$(OUTDIR)\PlayDMO.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("PlayDMO.dep")
!INCLUDE "PlayDMO.dep"
!ELSE 
!MESSAGE Warning: cannot find "PlayDMO.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "PlayDMO - Win32 Release" || "$(CFG)" == "PlayDMO - Win32 Debug"
SOURCE=..\..\Common\dshowutil.cpp

!IF  "$(CFG)" == "PlayDMO - Win32 Release"


"$(INTDIR)\dshowutil.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\PlayDMO.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PlayDMO - Win32 Debug"


"$(INTDIR)\dshowutil.obj"	"$(INTDIR)\dshowutil.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\PlayDMO.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Common\mfcdmoutil.cpp

!IF  "$(CFG)" == "PlayDMO - Win32 Release"


"$(INTDIR)\mfcdmoutil.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\PlayDMO.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PlayDMO - Win32 Debug"


"$(INTDIR)\mfcdmoutil.obj"	"$(INTDIR)\mfcdmoutil.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\PlayDMO.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Common\mfcutil.cpp

!IF  "$(CFG)" == "PlayDMO - Win32 Release"


"$(INTDIR)\mfcutil.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\PlayDMO.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PlayDMO - Win32 Debug"


"$(INTDIR)\mfcutil.obj"	"$(INTDIR)\mfcutil.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\PlayDMO.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\Common\namedguid.cpp

!IF  "$(CFG)" == "PlayDMO - Win32 Release"


"$(INTDIR)\namedguid.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\PlayDMO.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "PlayDMO - Win32 Debug"


"$(INTDIR)\namedguid.obj"	"$(INTDIR)\namedguid.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\PlayDMO.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\PlayDMO.cpp

!IF  "$(CFG)" == "PlayDMO - Win32 Release"


"$(INTDIR)\PlayDMO.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\PlayDMO.pch"


!ELSEIF  "$(CFG)" == "PlayDMO - Win32 Debug"


"$(INTDIR)\PlayDMO.obj"	"$(INTDIR)\PlayDMO.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\PlayDMO.pch"


!ENDIF 

SOURCE=.\PlayDMO.rc

"$(INTDIR)\PlayDMO.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\PlayDMODlg.cpp

!IF  "$(CFG)" == "PlayDMO - Win32 Release"


"$(INTDIR)\PlayDMODlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\PlayDMO.pch"


!ELSEIF  "$(CFG)" == "PlayDMO - Win32 Debug"


"$(INTDIR)\PlayDMODlg.obj"	"$(INTDIR)\PlayDMODlg.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\PlayDMO.pch"


!ENDIF 

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "PlayDMO - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "..\..\common" /I "..\..\..\..\..\include" /I "..\..\baseclasses" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\PlayDMO.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\PlayDMO.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "PlayDMO - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\common" /I "..\..\..\..\..\include" /I "..\..\baseclasses" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\PlayDMO.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\StdAfx.sbr"	"$(INTDIR)\PlayDMO.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

