# Microsoft Developer Studio Generated NMAKE File, Based on Textures.dsp
!IF "$(CFG)" == ""
CFG=Textures - Win32 Debug
!MESSAGE No configuration specified. Defaulting to Textures - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Textures - Win32 Release" && "$(CFG)" != "Textures - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Textures.mak" CFG="Textures - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Textures - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Textures - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "Textures - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\Texture3D.exe"


CLEAN :
	-@erase "$(INTDIR)\DShowTextures.obj"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\Textures.obj"
	-@erase "$(INTDIR)\textures.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\Texture3D.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /I "..\..\..\common\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\Textures.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\textures.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Textures.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\..\BaseClasses\Release\strmbase.lib d3dx8.lib d3d8.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\Texture3D.pdb" /machine:I386 /out:"$(OUTDIR)\Texture3D.exe" /libpath:"..\..\..\..\..\lib" /OPT:NOREF /OPT:ICF /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\DShowTextures.obj" \
	"$(INTDIR)\Textures.obj" \
	"$(INTDIR)\textures.res"

"$(OUTDIR)\Texture3D.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Textures - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\Texture3D.exe" "$(OUTDIR)\Textures.bsc"


CLEAN :
	-@erase "$(INTDIR)\DShowTextures.obj"
	-@erase "$(INTDIR)\DShowTextures.sbr"
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\dxutil.sbr"
	-@erase "$(INTDIR)\Textures.obj"
	-@erase "$(INTDIR)\textures.res"
	-@erase "$(INTDIR)\Textures.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\Texture3D.exe"
	-@erase "$(OUTDIR)\Texture3D.ilk"
	-@erase "$(OUTDIR)\Texture3D.pdb"
	-@erase "$(OUTDIR)\Textures.bsc"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\BaseClasses" /I "..\..\..\..\..\include" /I "..\..\..\common\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "DEBUG" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Textures.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\textures.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Textures.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\dxutil.sbr" \
	"$(INTDIR)\DShowTextures.sbr" \
	"$(INTDIR)\Textures.sbr"

"$(OUTDIR)\Textures.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=..\..\BaseClasses\debug\strmbasd.lib d3dx8.lib d3d8.lib winmm.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\Texture3D.pdb" /debug /machine:I386 /out:"$(OUTDIR)\Texture3D.exe" /pdbtype:sept /libpath:"..\..\..\..\..\lib" /stack:0x200000,0x200000
LINK32_OBJS= \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\DShowTextures.obj" \
	"$(INTDIR)\Textures.obj" \
	"$(INTDIR)\textures.res"

"$(OUTDIR)\Texture3D.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Textures.dep")
!INCLUDE "Textures.dep"
!ELSE 
!MESSAGE Warning: cannot find "Textures.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Textures - Win32 Release" || "$(CFG)" == "Textures - Win32 Debug"
SOURCE=..\..\..\Common\src\dxutil.cpp

!IF  "$(CFG)" == "Textures - Win32 Release"


"$(INTDIR)\dxutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Textures - Win32 Debug"


"$(INTDIR)\dxutil.obj"	"$(INTDIR)\dxutil.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\DShowTextures.cpp

!IF  "$(CFG)" == "Textures - Win32 Release"


"$(INTDIR)\DShowTextures.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Textures - Win32 Debug"


"$(INTDIR)\DShowTextures.obj"	"$(INTDIR)\DShowTextures.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\Textures.cpp

!IF  "$(CFG)" == "Textures - Win32 Release"


"$(INTDIR)\Textures.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Textures - Win32 Debug"


"$(INTDIR)\Textures.obj"	"$(INTDIR)\Textures.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\textures.rc

"$(INTDIR)\textures.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

