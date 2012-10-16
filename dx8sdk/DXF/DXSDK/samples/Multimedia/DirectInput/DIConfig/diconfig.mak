# Microsoft Developer Studio Generated NMAKE File, Based on diconfig.dsp
!IF "$(CFG)" == ""
CFG=diconfig - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to diconfig - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "diconfig - Win32 Release" && "$(CFG)" != "diconfig - Win32 Debug" && "$(CFG)" != "diconfig - Win32 Debug Unicode" && "$(CFG)" != "diconfig - Win32 Release Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "diconfig.mak" CFG="diconfig - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "diconfig - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "diconfig - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "diconfig - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "diconfig - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "diconfig - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\diconfig.dll"


CLEAN :
	-@erase "$(INTDIR)\cbitmap.obj"
	-@erase "$(INTDIR)\cd3dsurf.obj"
	-@erase "$(INTDIR)\cdevicecontrol.obj"
	-@erase "$(INTDIR)\cdeviceui.obj"
	-@erase "$(INTDIR)\cdeviceview.obj"
	-@erase "$(INTDIR)\cdeviceviewtext.obj"
	-@erase "$(INTDIR)\cdiacpage.obj"
	-@erase "$(INTDIR)\cfguitrace.obj"
	-@erase "$(INTDIR)\cfrmwrk.obj"
	-@erase "$(INTDIR)\configwnd.obj"
	-@erase "$(INTDIR)\constants.obj"
	-@erase "$(INTDIR)\cyclestr.obj"
	-@erase "$(INTDIR)\dconfig.res"
	-@erase "$(INTDIR)\flexcheckbox.obj"
	-@erase "$(INTDIR)\flexcombobox.obj"
	-@erase "$(INTDIR)\flexinfobox.obj"
	-@erase "$(INTDIR)\flexlistbox.obj"
	-@erase "$(INTDIR)\flexmsgbox.obj"
	-@erase "$(INTDIR)\flexscrollbar.obj"
	-@erase "$(INTDIR)\flextooltip.obj"
	-@erase "$(INTDIR)\flextree.obj"
	-@erase "$(INTDIR)\flexwnd.obj"
	-@erase "$(INTDIR)\guids.obj"
	-@erase "$(INTDIR)\iclassfact.obj"
	-@erase "$(INTDIR)\ipageclassfact.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\populate.obj"
	-@erase "$(INTDIR)\privcom.obj"
	-@erase "$(INTDIR)\registry.obj"
	-@erase "$(INTDIR)\selcontroldlg.obj"
	-@erase "$(INTDIR)\uiglobals.obj"
	-@erase "$(INTDIR)\usefuldi.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\viewselwnd.obj"
	-@erase "$(OUTDIR)\diconfig.dll"
	-@erase "$(OUTDIR)\diconfig.exp"
	-@erase "$(OUTDIR)\diconfig.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GR /O2 /I "$(dxsdkroot)\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DICONFIG_EXPORTS" /Fp"$(INTDIR)\diconfig.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\dconfig.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\diconfig.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib uuid.lib dinput8.lib ddraw.lib d3d8.lib d3dx8.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\diconfig.pdb" /machine:I386 /def:".\diactfrm.def" /out:"$(OUTDIR)\diconfig.dll" /implib:"$(OUTDIR)\diconfig.lib" /libpath:"$(dxsdkroot)\lib" /stack:0x200000,0x200000
DEF_FILE= \
	".\diactfrm.def"
LINK32_OBJS= \
	"$(INTDIR)\cbitmap.obj" \
	"$(INTDIR)\cd3dsurf.obj" \
	"$(INTDIR)\cdevicecontrol.obj" \
	"$(INTDIR)\cdeviceui.obj" \
	"$(INTDIR)\cdeviceview.obj" \
	"$(INTDIR)\cdeviceviewtext.obj" \
	"$(INTDIR)\cdiacpage.obj" \
	"$(INTDIR)\cfguitrace.obj" \
	"$(INTDIR)\cfrmwrk.obj" \
	"$(INTDIR)\configwnd.obj" \
	"$(INTDIR)\constants.obj" \
	"$(INTDIR)\cyclestr.obj" \
	"$(INTDIR)\flexcheckbox.obj" \
	"$(INTDIR)\flexcombobox.obj" \
	"$(INTDIR)\flexinfobox.obj" \
	"$(INTDIR)\flexlistbox.obj" \
	"$(INTDIR)\flexmsgbox.obj" \
	"$(INTDIR)\flexscrollbar.obj" \
	"$(INTDIR)\flextooltip.obj" \
	"$(INTDIR)\flextree.obj" \
	"$(INTDIR)\flexwnd.obj" \
	"$(INTDIR)\guids.obj" \
	"$(INTDIR)\iclassfact.obj" \
	"$(INTDIR)\ipageclassfact.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\populate.obj" \
	"$(INTDIR)\privcom.obj" \
	"$(INTDIR)\registry.obj" \
	"$(INTDIR)\selcontroldlg.obj" \
	"$(INTDIR)\uiglobals.obj" \
	"$(INTDIR)\usefuldi.obj" \
	"$(INTDIR)\viewselwnd.obj" \
	"$(INTDIR)\dconfig.res"

"$(OUTDIR)\diconfig.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "diconfig - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\diconfig.dll"


CLEAN :
	-@erase "$(INTDIR)\cbitmap.obj"
	-@erase "$(INTDIR)\cd3dsurf.obj"
	-@erase "$(INTDIR)\cdevicecontrol.obj"
	-@erase "$(INTDIR)\cdeviceui.obj"
	-@erase "$(INTDIR)\cdeviceview.obj"
	-@erase "$(INTDIR)\cdeviceviewtext.obj"
	-@erase "$(INTDIR)\cdiacpage.obj"
	-@erase "$(INTDIR)\cfguitrace.obj"
	-@erase "$(INTDIR)\cfrmwrk.obj"
	-@erase "$(INTDIR)\configwnd.obj"
	-@erase "$(INTDIR)\constants.obj"
	-@erase "$(INTDIR)\cyclestr.obj"
	-@erase "$(INTDIR)\dconfig.res"
	-@erase "$(INTDIR)\flexcheckbox.obj"
	-@erase "$(INTDIR)\flexcombobox.obj"
	-@erase "$(INTDIR)\flexinfobox.obj"
	-@erase "$(INTDIR)\flexlistbox.obj"
	-@erase "$(INTDIR)\flexmsgbox.obj"
	-@erase "$(INTDIR)\flexscrollbar.obj"
	-@erase "$(INTDIR)\flextooltip.obj"
	-@erase "$(INTDIR)\flextree.obj"
	-@erase "$(INTDIR)\flexwnd.obj"
	-@erase "$(INTDIR)\guids.obj"
	-@erase "$(INTDIR)\iclassfact.obj"
	-@erase "$(INTDIR)\ipageclassfact.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\populate.obj"
	-@erase "$(INTDIR)\privcom.obj"
	-@erase "$(INTDIR)\registry.obj"
	-@erase "$(INTDIR)\selcontroldlg.obj"
	-@erase "$(INTDIR)\uiglobals.obj"
	-@erase "$(INTDIR)\usefuldi.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\viewselwnd.obj"
	-@erase "$(OUTDIR)\diconfig.dll"
	-@erase "$(OUTDIR)\diconfig.exp"
	-@erase "$(OUTDIR)\diconfig.ilk"
	-@erase "$(OUTDIR)\diconfig.lib"
	-@erase "$(OUTDIR)\diconfig.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GR /Zi /Od /I "$(dxsdkroot)\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DICONFIG_EXPORTS" /D "DEBUG" /Fp"$(INTDIR)\diconfig.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\dconfig.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\diconfig.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib uuid.lib dinput8.lib ddraw.lib d3d8.lib d3dx8.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\diconfig.pdb" /debug /machine:I386 /def:".\diactfrm.def" /out:"$(OUTDIR)\diconfig.dll" /implib:"$(OUTDIR)\diconfig.lib" /pdbtype:sept /libpath:"$(dxsdkroot)\lib" /stack:0x200000,0x200000
DEF_FILE= \
	".\diactfrm.def"
LINK32_OBJS= \
	"$(INTDIR)\cbitmap.obj" \
	"$(INTDIR)\cd3dsurf.obj" \
	"$(INTDIR)\cdevicecontrol.obj" \
	"$(INTDIR)\cdeviceui.obj" \
	"$(INTDIR)\cdeviceview.obj" \
	"$(INTDIR)\cdeviceviewtext.obj" \
	"$(INTDIR)\cdiacpage.obj" \
	"$(INTDIR)\cfguitrace.obj" \
	"$(INTDIR)\cfrmwrk.obj" \
	"$(INTDIR)\configwnd.obj" \
	"$(INTDIR)\constants.obj" \
	"$(INTDIR)\cyclestr.obj" \
	"$(INTDIR)\flexcheckbox.obj" \
	"$(INTDIR)\flexcombobox.obj" \
	"$(INTDIR)\flexinfobox.obj" \
	"$(INTDIR)\flexlistbox.obj" \
	"$(INTDIR)\flexmsgbox.obj" \
	"$(INTDIR)\flexscrollbar.obj" \
	"$(INTDIR)\flextooltip.obj" \
	"$(INTDIR)\flextree.obj" \
	"$(INTDIR)\flexwnd.obj" \
	"$(INTDIR)\guids.obj" \
	"$(INTDIR)\iclassfact.obj" \
	"$(INTDIR)\ipageclassfact.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\populate.obj" \
	"$(INTDIR)\privcom.obj" \
	"$(INTDIR)\registry.obj" \
	"$(INTDIR)\selcontroldlg.obj" \
	"$(INTDIR)\uiglobals.obj" \
	"$(INTDIR)\usefuldi.obj" \
	"$(INTDIR)\viewselwnd.obj" \
	"$(INTDIR)\dconfig.res"

"$(OUTDIR)\diconfig.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "diconfig - Win32 Debug Unicode"

OUTDIR=.\Win32_Debug_Unicode
INTDIR=.\Win32_Debug_Unicode
# Begin Custom Macros
OutDir=.\Win32_Debug_Unicode
# End Custom Macros

ALL : "$(OUTDIR)\diconfig.dll"


CLEAN :
	-@erase "$(INTDIR)\cbitmap.obj"
	-@erase "$(INTDIR)\cd3dsurf.obj"
	-@erase "$(INTDIR)\cdevicecontrol.obj"
	-@erase "$(INTDIR)\cdeviceui.obj"
	-@erase "$(INTDIR)\cdeviceview.obj"
	-@erase "$(INTDIR)\cdeviceviewtext.obj"
	-@erase "$(INTDIR)\cdiacpage.obj"
	-@erase "$(INTDIR)\cfguitrace.obj"
	-@erase "$(INTDIR)\cfrmwrk.obj"
	-@erase "$(INTDIR)\configwnd.obj"
	-@erase "$(INTDIR)\constants.obj"
	-@erase "$(INTDIR)\cyclestr.obj"
	-@erase "$(INTDIR)\dconfig.res"
	-@erase "$(INTDIR)\flexcheckbox.obj"
	-@erase "$(INTDIR)\flexcombobox.obj"
	-@erase "$(INTDIR)\flexinfobox.obj"
	-@erase "$(INTDIR)\flexlistbox.obj"
	-@erase "$(INTDIR)\flexmsgbox.obj"
	-@erase "$(INTDIR)\flexscrollbar.obj"
	-@erase "$(INTDIR)\flextooltip.obj"
	-@erase "$(INTDIR)\flextree.obj"
	-@erase "$(INTDIR)\flexwnd.obj"
	-@erase "$(INTDIR)\guids.obj"
	-@erase "$(INTDIR)\iclassfact.obj"
	-@erase "$(INTDIR)\ipageclassfact.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\populate.obj"
	-@erase "$(INTDIR)\privcom.obj"
	-@erase "$(INTDIR)\registry.obj"
	-@erase "$(INTDIR)\selcontroldlg.obj"
	-@erase "$(INTDIR)\uiglobals.obj"
	-@erase "$(INTDIR)\usefuldi.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\viewselwnd.obj"
	-@erase "$(OUTDIR)\diconfig.dll"
	-@erase "$(OUTDIR)\diconfig.exp"
	-@erase "$(OUTDIR)\diconfig.ilk"
	-@erase "$(OUTDIR)\diconfig.lib"
	-@erase "$(OUTDIR)\diconfig.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GR /Zi /Od /I "$(dxsdkroot)\include" /D "_MBCS" /D "_USRDLL" /D "DICONFIG_EXPORTS" /D "DEBUG" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /Fp"$(INTDIR)\diconfig.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\dconfig.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\diconfig.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib uuid.lib dinput8.lib ddraw.lib d3d8.lib d3dx8.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\diconfig.pdb" /debug /machine:I386 /def:".\diactfrm.def" /out:"$(OUTDIR)\diconfig.dll" /implib:"$(OUTDIR)\diconfig.lib" /pdbtype:sept /libpath:"$(dxsdkroot)\lib" /stack:0x200000,0x200000
DEF_FILE= \
	".\diactfrm.def"
LINK32_OBJS= \
	"$(INTDIR)\cbitmap.obj" \
	"$(INTDIR)\cd3dsurf.obj" \
	"$(INTDIR)\cdevicecontrol.obj" \
	"$(INTDIR)\cdeviceui.obj" \
	"$(INTDIR)\cdeviceview.obj" \
	"$(INTDIR)\cdeviceviewtext.obj" \
	"$(INTDIR)\cdiacpage.obj" \
	"$(INTDIR)\cfguitrace.obj" \
	"$(INTDIR)\cfrmwrk.obj" \
	"$(INTDIR)\configwnd.obj" \
	"$(INTDIR)\constants.obj" \
	"$(INTDIR)\cyclestr.obj" \
	"$(INTDIR)\flexcheckbox.obj" \
	"$(INTDIR)\flexcombobox.obj" \
	"$(INTDIR)\flexinfobox.obj" \
	"$(INTDIR)\flexlistbox.obj" \
	"$(INTDIR)\flexmsgbox.obj" \
	"$(INTDIR)\flexscrollbar.obj" \
	"$(INTDIR)\flextooltip.obj" \
	"$(INTDIR)\flextree.obj" \
	"$(INTDIR)\flexwnd.obj" \
	"$(INTDIR)\guids.obj" \
	"$(INTDIR)\iclassfact.obj" \
	"$(INTDIR)\ipageclassfact.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\populate.obj" \
	"$(INTDIR)\privcom.obj" \
	"$(INTDIR)\registry.obj" \
	"$(INTDIR)\selcontroldlg.obj" \
	"$(INTDIR)\uiglobals.obj" \
	"$(INTDIR)\usefuldi.obj" \
	"$(INTDIR)\viewselwnd.obj" \
	"$(INTDIR)\dconfig.res"

"$(OUTDIR)\diconfig.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "diconfig - Win32 Release Unicode"

OUTDIR=.\Win32_Release_Unicode
INTDIR=.\Win32_Release_Unicode
# Begin Custom Macros
OutDir=.\Win32_Release_Unicode
# End Custom Macros

ALL : "$(OUTDIR)\diconfig.dll"


CLEAN :
	-@erase "$(INTDIR)\cbitmap.obj"
	-@erase "$(INTDIR)\cd3dsurf.obj"
	-@erase "$(INTDIR)\cdevicecontrol.obj"
	-@erase "$(INTDIR)\cdeviceui.obj"
	-@erase "$(INTDIR)\cdeviceview.obj"
	-@erase "$(INTDIR)\cdeviceviewtext.obj"
	-@erase "$(INTDIR)\cdiacpage.obj"
	-@erase "$(INTDIR)\cfguitrace.obj"
	-@erase "$(INTDIR)\cfrmwrk.obj"
	-@erase "$(INTDIR)\configwnd.obj"
	-@erase "$(INTDIR)\constants.obj"
	-@erase "$(INTDIR)\cyclestr.obj"
	-@erase "$(INTDIR)\dconfig.res"
	-@erase "$(INTDIR)\flexcheckbox.obj"
	-@erase "$(INTDIR)\flexcombobox.obj"
	-@erase "$(INTDIR)\flexinfobox.obj"
	-@erase "$(INTDIR)\flexlistbox.obj"
	-@erase "$(INTDIR)\flexmsgbox.obj"
	-@erase "$(INTDIR)\flexscrollbar.obj"
	-@erase "$(INTDIR)\flextooltip.obj"
	-@erase "$(INTDIR)\flextree.obj"
	-@erase "$(INTDIR)\flexwnd.obj"
	-@erase "$(INTDIR)\guids.obj"
	-@erase "$(INTDIR)\iclassfact.obj"
	-@erase "$(INTDIR)\ipageclassfact.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\populate.obj"
	-@erase "$(INTDIR)\privcom.obj"
	-@erase "$(INTDIR)\registry.obj"
	-@erase "$(INTDIR)\selcontroldlg.obj"
	-@erase "$(INTDIR)\uiglobals.obj"
	-@erase "$(INTDIR)\usefuldi.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\viewselwnd.obj"
	-@erase "$(OUTDIR)\diconfig.dll"
	-@erase "$(OUTDIR)\diconfig.exp"
	-@erase "$(OUTDIR)\diconfig.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GR /O2 /I "$(dxsdkroot)\include" /D "_MBCS" /D "_USRDLL" /D "DICONFIG_EXPORTS" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /Fp"$(INTDIR)\diconfig.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\dconfig.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\diconfig.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib ole32.lib uuid.lib dinput8.lib ddraw.lib d3d8.lib d3dx8.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\diconfig.pdb" /machine:I386 /def:".\diactfrm.def" /out:"$(OUTDIR)\diconfig.dll" /implib:"$(OUTDIR)\diconfig.lib" /libpath:"$(dxsdkroot)\lib" /stack:0x200000,0x200000
DEF_FILE= \
	".\diactfrm.def"
LINK32_OBJS= \
	"$(INTDIR)\cbitmap.obj" \
	"$(INTDIR)\cd3dsurf.obj" \
	"$(INTDIR)\cdevicecontrol.obj" \
	"$(INTDIR)\cdeviceui.obj" \
	"$(INTDIR)\cdeviceview.obj" \
	"$(INTDIR)\cdeviceviewtext.obj" \
	"$(INTDIR)\cdiacpage.obj" \
	"$(INTDIR)\cfguitrace.obj" \
	"$(INTDIR)\cfrmwrk.obj" \
	"$(INTDIR)\configwnd.obj" \
	"$(INTDIR)\constants.obj" \
	"$(INTDIR)\cyclestr.obj" \
	"$(INTDIR)\flexcheckbox.obj" \
	"$(INTDIR)\flexcombobox.obj" \
	"$(INTDIR)\flexinfobox.obj" \
	"$(INTDIR)\flexlistbox.obj" \
	"$(INTDIR)\flexmsgbox.obj" \
	"$(INTDIR)\flexscrollbar.obj" \
	"$(INTDIR)\flextooltip.obj" \
	"$(INTDIR)\flextree.obj" \
	"$(INTDIR)\flexwnd.obj" \
	"$(INTDIR)\guids.obj" \
	"$(INTDIR)\iclassfact.obj" \
	"$(INTDIR)\ipageclassfact.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\populate.obj" \
	"$(INTDIR)\privcom.obj" \
	"$(INTDIR)\registry.obj" \
	"$(INTDIR)\selcontroldlg.obj" \
	"$(INTDIR)\uiglobals.obj" \
	"$(INTDIR)\usefuldi.obj" \
	"$(INTDIR)\viewselwnd.obj" \
	"$(INTDIR)\dconfig.res"

"$(OUTDIR)\diconfig.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("diconfig.dep")
!INCLUDE "diconfig.dep"
!ELSE 
!MESSAGE Warning: cannot find "diconfig.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "diconfig - Win32 Release" || "$(CFG)" == "diconfig - Win32 Debug" || "$(CFG)" == "diconfig - Win32 Debug Unicode" || "$(CFG)" == "diconfig - Win32 Release Unicode"
SOURCE=.\cbitmap.cpp

"$(INTDIR)\cbitmap.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cd3dsurf.cpp

"$(INTDIR)\cd3dsurf.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cdevicecontrol.cpp

"$(INTDIR)\cdevicecontrol.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cdeviceui.cpp

"$(INTDIR)\cdeviceui.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cdeviceview.cpp

"$(INTDIR)\cdeviceview.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cdeviceviewtext.cpp

"$(INTDIR)\cdeviceviewtext.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cdiacpage.cpp

"$(INTDIR)\cdiacpage.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cfguitrace.cpp

"$(INTDIR)\cfguitrace.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cfrmwrk.cpp

"$(INTDIR)\cfrmwrk.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\configwnd.cpp

"$(INTDIR)\configwnd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\constants.cpp

"$(INTDIR)\constants.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\cyclestr.cpp

"$(INTDIR)\cyclestr.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\dconfig.rc

"$(INTDIR)\dconfig.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\flexcheckbox.cpp

"$(INTDIR)\flexcheckbox.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\flexcombobox.cpp

"$(INTDIR)\flexcombobox.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\flexinfobox.cpp

"$(INTDIR)\flexinfobox.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\flexlistbox.cpp

"$(INTDIR)\flexlistbox.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\flexmsgbox.cpp

"$(INTDIR)\flexmsgbox.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\flexscrollbar.cpp

"$(INTDIR)\flexscrollbar.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\flextooltip.cpp

"$(INTDIR)\flextooltip.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\flextree.cpp

"$(INTDIR)\flextree.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\flexwnd.cpp

"$(INTDIR)\flexwnd.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\guids.c

"$(INTDIR)\guids.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\iclassfact.cpp

"$(INTDIR)\iclassfact.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ipageclassfact.cpp

"$(INTDIR)\ipageclassfact.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\main.cpp

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\populate.cpp

"$(INTDIR)\populate.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\privcom.cpp

"$(INTDIR)\privcom.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\registry.cpp

"$(INTDIR)\registry.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\selcontroldlg.cpp

"$(INTDIR)\selcontroldlg.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\uiglobals.cpp

"$(INTDIR)\uiglobals.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\usefuldi.cpp

"$(INTDIR)\usefuldi.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\viewselwnd.cpp

"$(INTDIR)\viewselwnd.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

