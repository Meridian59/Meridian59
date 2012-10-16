# Microsoft Developer Studio Generated NMAKE File, Based on AudioCap.dsp
!IF "$(CFG)" == ""
CFG=AudioCap - Win32 Debug
!MESSAGE No configuration specified. Defaulting to AudioCap - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "AudioCap - Win32 Release" && "$(CFG)" != "AudioCap - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AudioCap.mak" CFG="AudioCap - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AudioCap - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "AudioCap - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "AudioCap - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\AudioCap.exe"

!ELSE 

ALL : "BaseClasses - Win32 Release" "$(OUTDIR)\AudioCap.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"BaseClasses - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\AudioCap.obj"
	-@erase "$(INTDIR)\AudioCap.pch"
	-@erase "$(INTDIR)\AudioCap.res"
	-@erase "$(INTDIR)\AudioCapDlg.obj"
	-@erase "$(INTDIR)\dshowutil.obj"
	-@erase "$(INTDIR)\mfcutil.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\AudioCap.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\AudioCap.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\AudioCap.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\AudioCap.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\BaseClasses\Release\strmbase.lib winmm.lib dmoguids.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\AudioCap.pdb" /machine:I386 /nodefaultlib:"libcmt.lib" /out:"$(OUTDIR)\AudioCap.exe" /OPT:NOREF /OPT:ICF /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\AudioCap.obj" \
	"$(INTDIR)\AudioCapDlg.obj" \
	"$(INTDIR)\dshowutil.obj" \
	"$(INTDIR)\mfcutil.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\AudioCap.res" \
	"..\BaseClasses\Release\STRMBASE.lib"

"$(OUTDIR)\AudioCap.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "AudioCap - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\AudioCap.exe"

!ELSE 

ALL : "BaseClasses - Win32 Debug" "$(OUTDIR)\AudioCap.exe"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"BaseClasses - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\AudioCap.obj"
	-@erase "$(INTDIR)\AudioCap.pch"
	-@erase "$(INTDIR)\AudioCap.res"
	-@erase "$(INTDIR)\AudioCapDlg.obj"
	-@erase "$(INTDIR)\dshowutil.obj"
	-@erase "$(INTDIR)\mfcutil.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\AudioCap.exe"
	-@erase "$(OUTDIR)\AudioCap.ilk"
	-@erase "$(OUTDIR)\AudioCap.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /I "..\..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\AudioCap.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\AudioCap.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\AudioCap.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\BaseClasses\Debug\strmbasd.lib winmm.lib dmoguids.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\AudioCap.pdb" /debug /machine:I386 /nodefaultlib:"libcmtd.lib" /out:"$(OUTDIR)\AudioCap.exe" /pdbtype:sept /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\AudioCap.obj" \
	"$(INTDIR)\AudioCapDlg.obj" \
	"$(INTDIR)\dshowutil.obj" \
	"$(INTDIR)\mfcutil.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\AudioCap.res" \
	"..\BaseClasses\debug\strmbasd.lib"

"$(OUTDIR)\AudioCap.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("AudioCap.dep")
!INCLUDE "AudioCap.dep"
!ELSE 
!MESSAGE Warning: cannot find "AudioCap.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "AudioCap - Win32 Release" || "$(CFG)" == "AudioCap - Win32 Debug"
SOURCE=.\AudioCap.cpp

"$(INTDIR)\AudioCap.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\AudioCap.pch"


SOURCE=.\AudioCap.rc

"$(INTDIR)\AudioCap.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\AudioCapDlg.cpp

"$(INTDIR)\AudioCapDlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\AudioCap.pch"


SOURCE=..\..\Common\dshowutil.cpp

"$(INTDIR)\dshowutil.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\AudioCap.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\Common\mfcutil.cpp

"$(INTDIR)\mfcutil.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\AudioCap.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "AudioCap - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /I "..\..\common" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\AudioCap.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\AudioCap.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "AudioCap - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /I "..\..\common" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Fp"$(INTDIR)\AudioCap.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\AudioCap.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

!IF  "$(CFG)" == "AudioCap - Win32 Release"

"BaseClasses - Win32 Release" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\BaseClasses"
   $(MAKE) /$(MAKEFLAGS) /F .\baseclasses.mak CFG="BaseClasses - Win32 Release" 
   cd "..\audiocap"

"BaseClasses - Win32 ReleaseCLEAN" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\BaseClasses"
   $(MAKE) /$(MAKEFLAGS) /F .\baseclasses.mak CFG="BaseClasses - Win32 Release" RECURSE=1 CLEAN 
   cd "..\audiocap"

!ELSEIF  "$(CFG)" == "AudioCap - Win32 Debug"

"BaseClasses - Win32 Debug" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\BaseClasses"
   $(MAKE) /$(MAKEFLAGS) /F .\baseclasses.mak CFG="BaseClasses - Win32 Debug" 
   cd "..\audiocap"

"BaseClasses - Win32 DebugCLEAN" : 
   cd "\ntdev\multimedia\DirectX\dxsdk\samples\multimedia\dshow\BaseClasses"
   $(MAKE) /$(MAKEFLAGS) /F .\baseclasses.mak CFG="BaseClasses - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\audiocap"

!ENDIF 


!ENDIF 

