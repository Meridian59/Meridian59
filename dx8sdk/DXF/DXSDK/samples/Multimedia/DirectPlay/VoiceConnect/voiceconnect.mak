# Microsoft Developer Studio Generated NMAKE File, Based on voiceconnect.dsp
!IF "$(CFG)" == ""
CFG=VoiceConnect - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to VoiceConnect - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "VoiceConnect - Win32 Release" && "$(CFG)" != "VoiceConnect - Win32 Debug" && "$(CFG)" != "VoiceConnect - Win32 Debug Unicode" && "$(CFG)" != "VoiceConnect - Win32 Release Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "voiceconnect.mak" CFG="VoiceConnect - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "VoiceConnect - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "VoiceConnect - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "VoiceConnect - Win32 Debug Unicode" (based on "Win32 (x86) Application")
!MESSAGE "VoiceConnect - Win32 Release Unicode" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "VoiceConnect - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\voiceconnect.exe"


CLEAN :
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\NetConnect.obj"
	-@erase "$(INTDIR)\NetVoice.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\VoiceConnect.obj"
	-@erase "$(INTDIR)\voiceconnect.res"
	-@erase "$(OUTDIR)\voiceconnect.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\..\common\include" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "WIN32" /D "_WIN32_DCOM" /Fp"$(INTDIR)\voiceconnect.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\voiceconnect.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\voiceconnect.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dplay.lib comctl32.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\voiceconnect.pdb" /machine:I386 /out:"$(OUTDIR)\voiceconnect.exe" /stack:0x10000,0x10000
LINK32_OBJS= \
	"$(INTDIR)\VoiceConnect.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\NetConnect.obj" \
	"$(INTDIR)\NetVoice.obj" \
	"$(INTDIR)\voiceconnect.res"

"$(OUTDIR)\voiceconnect.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "VoiceConnect - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\voiceconnect.exe"


CLEAN :
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\NetConnect.obj"
	-@erase "$(INTDIR)\NetVoice.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\VoiceConnect.obj"
	-@erase "$(INTDIR)\voiceconnect.res"
	-@erase "$(OUTDIR)\voiceconnect.exe"
	-@erase "$(OUTDIR)\voiceconnect.ilk"
	-@erase "$(OUTDIR)\voiceconnect.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\common\include" /D "_WINDOWS" /D "_DEBUG" /D "WIN32" /D "_WIN32_DCOM" /Fp"$(INTDIR)\voiceconnect.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\voiceconnect.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\voiceconnect.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dplay.lib comctl32.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\voiceconnect.pdb" /debug /machine:I386 /nodefaultlib:"LIBC" /out:"$(OUTDIR)\voiceconnect.exe" /pdbtype:sept /stack:0x10000,0x10000
LINK32_OBJS= \
	"$(INTDIR)\VoiceConnect.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\NetConnect.obj" \
	"$(INTDIR)\NetVoice.obj" \
	"$(INTDIR)\voiceconnect.res"

"$(OUTDIR)\voiceconnect.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "VoiceConnect - Win32 Debug Unicode"

OUTDIR=.\Win32_Debug_Unicode
INTDIR=.\Win32_Debug_Unicode
# Begin Custom Macros
OutDir=.\Win32_Debug_Unicode
# End Custom Macros

ALL : "$(OUTDIR)\voiceconnect.exe"


CLEAN :
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\NetConnect.obj"
	-@erase "$(INTDIR)\NetVoice.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\VoiceConnect.obj"
	-@erase "$(INTDIR)\voiceconnect.res"
	-@erase "$(OUTDIR)\voiceconnect.exe"
	-@erase "$(OUTDIR)\voiceconnect.ilk"
	-@erase "$(OUTDIR)\voiceconnect.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\..\common\include" /D "_DEBUG" /D "_WINDOWS" /D "WIN32" /D "_WIN32_DCOM" /D "UNICODE" /D "_UNICODE" /Fp"$(INTDIR)\voiceconnect.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\voiceconnect.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\voiceconnect.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dplay.lib comctl32.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\voiceconnect.pdb" /debug /machine:I386 /nodefaultlib:"LIBC" /out:"$(OUTDIR)\voiceconnect.exe" /pdbtype:sept /stack:0x10000,0x10000
LINK32_OBJS= \
	"$(INTDIR)\VoiceConnect.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\NetConnect.obj" \
	"$(INTDIR)\NetVoice.obj" \
	"$(INTDIR)\voiceconnect.res"

"$(OUTDIR)\voiceconnect.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "VoiceConnect - Win32 Release Unicode"

OUTDIR=.\Win32_Release_Unicode
INTDIR=.\Win32_Release_Unicode
# Begin Custom Macros
OutDir=.\Win32_Release_Unicode
# End Custom Macros

ALL : "$(OUTDIR)\voiceconnect.exe"


CLEAN :
	-@erase "$(INTDIR)\dxutil.obj"
	-@erase "$(INTDIR)\NetConnect.obj"
	-@erase "$(INTDIR)\NetVoice.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\VoiceConnect.obj"
	-@erase "$(INTDIR)\voiceconnect.res"
	-@erase "$(OUTDIR)\voiceconnect.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /I "..\..\common\include" /D "NDEBUG" /D "_MBCS" /D "_WINDOWS" /D "WIN32" /D "_WIN32_DCOM" /D "UNICODE" /D "_UNICODE" /Fp"$(INTDIR)\voiceconnect.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\voiceconnect.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\voiceconnect.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=dplay.lib comctl32.lib dxguid.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib oleaut32.lib odbc32.lib odbccp32.lib dxerr8.lib winmm.lib kernel32.lib user32.lib ole32.lib uuid.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\voiceconnect.pdb" /machine:I386 /out:"$(OUTDIR)\voiceconnect.exe" /stack:0x10000,0x10000
LINK32_OBJS= \
	"$(INTDIR)\VoiceConnect.obj" \
	"$(INTDIR)\dxutil.obj" \
	"$(INTDIR)\NetConnect.obj" \
	"$(INTDIR)\NetVoice.obj" \
	"$(INTDIR)\voiceconnect.res"

"$(OUTDIR)\voiceconnect.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("voiceconnect.dep")
!INCLUDE "voiceconnect.dep"
!ELSE 
!MESSAGE Warning: cannot find "voiceconnect.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "VoiceConnect - Win32 Release" || "$(CFG)" == "VoiceConnect - Win32 Debug" || "$(CFG)" == "VoiceConnect - Win32 Debug Unicode" || "$(CFG)" == "VoiceConnect - Win32 Release Unicode"
SOURCE=.\VoiceConnect.cpp

"$(INTDIR)\VoiceConnect.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\voiceconnect.rc

"$(INTDIR)\voiceconnect.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=..\..\common\src\dxutil.cpp

"$(INTDIR)\dxutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\src\NetConnect.cpp

"$(INTDIR)\NetConnect.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\common\src\NetVoice.cpp

"$(INTDIR)\NetVoice.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

