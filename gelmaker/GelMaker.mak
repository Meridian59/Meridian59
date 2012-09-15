# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=GelMaker - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to GelMaker - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "GelMaker - Win32 Release" && "$(CFG)" !=\
 "GelMaker - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "GelMaker.mak" CFG="GelMaker - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GelMaker - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "GelMaker - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "GelMaker - Win32 Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "GelMaker - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\GelMaker.exe" "$(OUTDIR)\GelMaker.bsc"

CLEAN : 
	-@erase "$(INTDIR)\AfxEdh.obj"
	-@erase "$(INTDIR)\AfxEdh.sbr"
	-@erase "$(INTDIR)\BufferedView.obj"
	-@erase "$(INTDIR)\BufferedView.sbr"
	-@erase "$(INTDIR)\Dib.obj"
	-@erase "$(INTDIR)\Dib.sbr"
	-@erase "$(INTDIR)\DibRop.obj"
	-@erase "$(INTDIR)\DibRop.sbr"
	-@erase "$(INTDIR)\DibView.obj"
	-@erase "$(INTDIR)\DibView.sbr"
	-@erase "$(INTDIR)\GelDoc.obj"
	-@erase "$(INTDIR)\GelDoc.sbr"
	-@erase "$(INTDIR)\GelMaker.obj"
	-@erase "$(INTDIR)\GelMaker.pch"
	-@erase "$(INTDIR)\GelMaker.res"
	-@erase "$(INTDIR)\GelMaker.sbr"
	-@erase "$(INTDIR)\GelPreview.obj"
	-@erase "$(INTDIR)\GelPreview.sbr"
	-@erase "$(INTDIR)\GelView.obj"
	-@erase "$(INTDIR)\GelView.sbr"
	-@erase "$(INTDIR)\MainFrame.obj"
	-@erase "$(INTDIR)\MainFrame.sbr"
	-@erase "$(INTDIR)\NullView.obj"
	-@erase "$(INTDIR)\NullView.sbr"
	-@erase "$(INTDIR)\PaletteApp.obj"
	-@erase "$(INTDIR)\PaletteApp.sbr"
	-@erase "$(INTDIR)\PathString.obj"
	-@erase "$(INTDIR)\PathString.sbr"
	-@erase "$(INTDIR)\PersistFrame.obj"
	-@erase "$(INTDIR)\PersistFrame.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(OUTDIR)\GelMaker.bsc"
	-@erase "$(OUTDIR)\GelMaker.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/GelMaker.pch" /Yu"stdafx.h"\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\Release/
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/GelMaker.res" /d "NDEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/GelMaker.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\AfxEdh.sbr" \
	"$(INTDIR)\BufferedView.sbr" \
	"$(INTDIR)\Dib.sbr" \
	"$(INTDIR)\DibRop.sbr" \
	"$(INTDIR)\DibView.sbr" \
	"$(INTDIR)\GelDoc.sbr" \
	"$(INTDIR)\GelMaker.sbr" \
	"$(INTDIR)\GelPreview.sbr" \
	"$(INTDIR)\GelView.sbr" \
	"$(INTDIR)\MainFrame.sbr" \
	"$(INTDIR)\NullView.sbr" \
	"$(INTDIR)\PaletteApp.sbr" \
	"$(INTDIR)\PathString.sbr" \
	"$(INTDIR)\PersistFrame.sbr" \
	"$(INTDIR)\StdAfx.sbr"

"$(OUTDIR)\GelMaker.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/GelMaker.pdb" /machine:I386 /out:"$(OUTDIR)/GelMaker.exe" 
LINK32_OBJS= \
	"$(INTDIR)\AfxEdh.obj" \
	"$(INTDIR)\BufferedView.obj" \
	"$(INTDIR)\Dib.obj" \
	"$(INTDIR)\DibRop.obj" \
	"$(INTDIR)\DibView.obj" \
	"$(INTDIR)\GelDoc.obj" \
	"$(INTDIR)\GelMaker.obj" \
	"$(INTDIR)\GelMaker.res" \
	"$(INTDIR)\GelPreview.obj" \
	"$(INTDIR)\GelView.obj" \
	"$(INTDIR)\MainFrame.obj" \
	"$(INTDIR)\NullView.obj" \
	"$(INTDIR)\PaletteApp.obj" \
	"$(INTDIR)\PathString.obj" \
	"$(INTDIR)\PersistFrame.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\GelMaker.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "GelMaker - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\GelMaker.exe" "$(OUTDIR)\GelMaker.bsc"

CLEAN : 
	-@erase "$(INTDIR)\AfxEdh.obj"
	-@erase "$(INTDIR)\AfxEdh.sbr"
	-@erase "$(INTDIR)\BufferedView.obj"
	-@erase "$(INTDIR)\BufferedView.sbr"
	-@erase "$(INTDIR)\Dib.obj"
	-@erase "$(INTDIR)\Dib.sbr"
	-@erase "$(INTDIR)\DibRop.obj"
	-@erase "$(INTDIR)\DibRop.sbr"
	-@erase "$(INTDIR)\DibView.obj"
	-@erase "$(INTDIR)\DibView.sbr"
	-@erase "$(INTDIR)\GelDoc.obj"
	-@erase "$(INTDIR)\GelDoc.sbr"
	-@erase "$(INTDIR)\GelMaker.obj"
	-@erase "$(INTDIR)\GelMaker.pch"
	-@erase "$(INTDIR)\GelMaker.res"
	-@erase "$(INTDIR)\GelMaker.sbr"
	-@erase "$(INTDIR)\GelPreview.obj"
	-@erase "$(INTDIR)\GelPreview.sbr"
	-@erase "$(INTDIR)\GelView.obj"
	-@erase "$(INTDIR)\GelView.sbr"
	-@erase "$(INTDIR)\MainFrame.obj"
	-@erase "$(INTDIR)\MainFrame.sbr"
	-@erase "$(INTDIR)\NullView.obj"
	-@erase "$(INTDIR)\NullView.sbr"
	-@erase "$(INTDIR)\PaletteApp.obj"
	-@erase "$(INTDIR)\PaletteApp.sbr"
	-@erase "$(INTDIR)\PathString.obj"
	-@erase "$(INTDIR)\PathString.sbr"
	-@erase "$(INTDIR)\PersistFrame.obj"
	-@erase "$(INTDIR)\PersistFrame.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\GelMaker.bsc"
	-@erase "$(OUTDIR)\GelMaker.exe"
	-@erase "$(OUTDIR)\GelMaker.ilk"
	-@erase "$(OUTDIR)\GelMaker.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/GelMaker.pch"\
 /Yu"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/GelMaker.res" /d "_DEBUG" /d "_AFXDLL" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/GelMaker.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\AfxEdh.sbr" \
	"$(INTDIR)\BufferedView.sbr" \
	"$(INTDIR)\Dib.sbr" \
	"$(INTDIR)\DibRop.sbr" \
	"$(INTDIR)\DibView.sbr" \
	"$(INTDIR)\GelDoc.sbr" \
	"$(INTDIR)\GelMaker.sbr" \
	"$(INTDIR)\GelPreview.sbr" \
	"$(INTDIR)\GelView.sbr" \
	"$(INTDIR)\MainFrame.sbr" \
	"$(INTDIR)\NullView.sbr" \
	"$(INTDIR)\PaletteApp.sbr" \
	"$(INTDIR)\PathString.sbr" \
	"$(INTDIR)\PersistFrame.sbr" \
	"$(INTDIR)\StdAfx.sbr"

"$(OUTDIR)\GelMaker.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=/nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/GelMaker.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/GelMaker.exe" 
LINK32_OBJS= \
	"$(INTDIR)\AfxEdh.obj" \
	"$(INTDIR)\BufferedView.obj" \
	"$(INTDIR)\Dib.obj" \
	"$(INTDIR)\DibRop.obj" \
	"$(INTDIR)\DibView.obj" \
	"$(INTDIR)\GelDoc.obj" \
	"$(INTDIR)\GelMaker.obj" \
	"$(INTDIR)\GelMaker.res" \
	"$(INTDIR)\GelPreview.obj" \
	"$(INTDIR)\GelView.obj" \
	"$(INTDIR)\MainFrame.obj" \
	"$(INTDIR)\NullView.obj" \
	"$(INTDIR)\PaletteApp.obj" \
	"$(INTDIR)\PathString.obj" \
	"$(INTDIR)\PersistFrame.obj" \
	"$(INTDIR)\StdAfx.obj"

"$(OUTDIR)\GelMaker.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "GelMaker - Win32 Release"
# Name "GelMaker - Win32 Debug"

!IF  "$(CFG)" == "GelMaker - Win32 Release"

!ELSEIF  "$(CFG)" == "GelMaker - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\AfxEdh.h"\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "GelMaker - Win32 Release"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/GelMaker.pch" /Yc"stdafx.h"\
 /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\GelMaker.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "GelMaker - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_AFXDLL" /D "_MBCS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/GelMaker.pch"\
 /Yc"stdafx.h" /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\GelMaker.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\GelDoc.cpp
DEP_CPP_GELDO=\
	".\AfxEdh.h"\
	".\Dib.h"\
	".\GelDoc.h"\
	".\GelMaker.h"\
	".\MainFrame.h"\
	".\PaletteApp.h"\
	".\PersistFrame.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\GelDoc.obj" : $(SOURCE) $(DEP_CPP_GELDO) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"

"$(INTDIR)\GelDoc.sbr" : $(SOURCE) $(DEP_CPP_GELDO) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\GelView.cpp
DEP_CPP_GELVI=\
	".\AfxEdh.h"\
	".\BufferedView.h"\
	".\GelDoc.h"\
	".\GelMaker.h"\
	".\GelView.h"\
	".\PaletteApp.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\GelView.obj" : $(SOURCE) $(DEP_CPP_GELVI) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"

"$(INTDIR)\GelView.sbr" : $(SOURCE) $(DEP_CPP_GELVI) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\GelMaker.rc
DEP_RSC_GELMA=\
	".\res\GelMaker.ico"\
	".\res\GelMaker.rc2"\
	".\res\geltype.ico"\
	".\res\palette.bmp"\
	".\res\reference.bmp"\
	".\res\Sample1.bmp"\
	".\res\selswatch.ico"\
	".\res\Toolbar.bmp"\
	".\res\unselswatch.ico"\
	

"$(INTDIR)\GelMaker.res" : $(SOURCE) $(DEP_RSC_GELMA) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PersistFrame.cpp
DEP_CPP_PERSI=\
	".\AfxEdh.h"\
	".\PersistFrame.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\PersistFrame.obj" : $(SOURCE) $(DEP_CPP_PERSI) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"

"$(INTDIR)\PersistFrame.sbr" : $(SOURCE) $(DEP_CPP_PERSI) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PathString.cpp
DEP_CPP_PATHS=\
	".\AfxEdh.h"\
	".\PathString.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\PathString.obj" : $(SOURCE) $(DEP_CPP_PATHS) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"

"$(INTDIR)\PathString.sbr" : $(SOURCE) $(DEP_CPP_PATHS) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\AfxEdh.cpp
DEP_CPP_AFXED=\
	".\AfxEdh.h"\
	".\PathString.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\AfxEdh.obj" : $(SOURCE) $(DEP_CPP_AFXED) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"

"$(INTDIR)\AfxEdh.sbr" : $(SOURCE) $(DEP_CPP_AFXED) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=".\(c)3do.txt"

!IF  "$(CFG)" == "GelMaker - Win32 Release"

!ELSEIF  "$(CFG)" == "GelMaker - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\NullView.cpp
DEP_CPP_NULLV=\
	".\AfxEdh.h"\
	".\NullView.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\NullView.obj" : $(SOURCE) $(DEP_CPP_NULLV) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"

"$(INTDIR)\NullView.sbr" : $(SOURCE) $(DEP_CPP_NULLV) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\DibView.cpp
DEP_CPP_DIBVI=\
	".\AfxEdh.h"\
	".\Dib.h"\
	".\DibView.h"\
	".\PaletteApp.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\DibView.obj" : $(SOURCE) $(DEP_CPP_DIBVI) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"

"$(INTDIR)\DibView.sbr" : $(SOURCE) $(DEP_CPP_DIBVI) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Dib.cpp
DEP_CPP_DIB_C=\
	".\AfxEdh.h"\
	".\Dib.h"\
	".\DibRop.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\Dib.obj" : $(SOURCE) $(DEP_CPP_DIB_C) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"

"$(INTDIR)\Dib.sbr" : $(SOURCE) $(DEP_CPP_DIB_C) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PaletteApp.cpp
DEP_CPP_PALET=\
	".\AfxEdh.h"\
	".\Dib.h"\
	".\PaletteApp.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\PaletteApp.obj" : $(SOURCE) $(DEP_CPP_PALET) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"

"$(INTDIR)\PaletteApp.sbr" : $(SOURCE) $(DEP_CPP_PALET) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\DibRop.cpp
DEP_CPP_DIBRO=\
	".\AfxEdh.h"\
	".\DibRop.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\DibRop.obj" : $(SOURCE) $(DEP_CPP_DIBRO) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"

"$(INTDIR)\DibRop.sbr" : $(SOURCE) $(DEP_CPP_DIBRO) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\GelPreview.cpp
DEP_CPP_GELPR=\
	".\AfxEdh.h"\
	".\Dib.h"\
	".\DibView.h"\
	".\GelMaker.h"\
	".\GelPreview.h"\
	".\PaletteApp.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\GelPreview.obj" : $(SOURCE) $(DEP_CPP_GELPR) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"

"$(INTDIR)\GelPreview.sbr" : $(SOURCE) $(DEP_CPP_GELPR) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\MainFrame.cpp
DEP_CPP_MAINF=\
	".\AfxEdh.h"\
	".\BufferedView.h"\
	".\Dib.h"\
	".\DibView.h"\
	".\GelDoc.h"\
	".\GelMaker.h"\
	".\GelPreview.h"\
	".\GelView.h"\
	".\MainFrame.h"\
	".\PaletteApp.h"\
	".\PersistFrame.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\MainFrame.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"

"$(INTDIR)\MainFrame.sbr" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\BufferedView.cpp
DEP_CPP_BUFFE=\
	".\AfxEdh.h"\
	".\BufferedView.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\BufferedView.obj" : $(SOURCE) $(DEP_CPP_BUFFE) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"

"$(INTDIR)\BufferedView.sbr" : $(SOURCE) $(DEP_CPP_BUFFE) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\GelMaker.cpp
DEP_CPP_GELMAK=\
	".\AfxEdh.h"\
	".\Dib.h"\
	".\GelDoc.h"\
	".\GelMaker.h"\
	".\MainFrame.h"\
	".\PaletteApp.h"\
	".\PersistFrame.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\GelMaker.obj" : $(SOURCE) $(DEP_CPP_GELMAK) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"

"$(INTDIR)\GelMaker.sbr" : $(SOURCE) $(DEP_CPP_GELMAK) "$(INTDIR)"\
 "$(INTDIR)\GelMaker.pch"


# End Source File
# End Target
# End Project
################################################################################
