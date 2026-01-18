#
# vc.mak - Makefile for building OCFNext with Microsoft Visual C++
#
# Usage: nmake -fvc.mak [options] [targets]
#
# Targets (optional):
#
# <output-file> - Builds the specified output file, e.g. object file or executable.
# clean - Removes all intermediate files
#
# Options:
#
# "DLL=0|1" (defaults to 0)
# If this item is 0, then a static link library is generated.
# If this item is 1, a dynamic link library and a import library are generated.
#
# "COMPAT=0|1|2|5" (defaults to 0)
# If this item is 0 then the library is built without any special backward compatibility modes.
# Otherwise the library is built for backward compatibility with the given version of OWL (1.0, 2.0 or 5.0).
# Setting this variable will define the respective compiler flags.
#
# "DEBUG=0|1" (defaults to 0)
# If this item is 1, debugging support is compiled into the library.
# If this item is 0, then debugging support is disabled.
# Debug support does not include CodeView information.
#
# "STACKWALKER=0|1" (defaults to 0)
# If this item is 1, the StackWalker library is linked into the target.
#
# "CODEVIEW=0|1" (defaults to 1 for DEBUG=1, 0 for DEBUG=0)
# TODO: Test this option. Is it functional?
# If this item is 1 CodeView information is compiled into the library.
# You must use the /DEBUG:FULL and /DEBUGTYPE:cv link options when linking your executable.
# A value of 0 indicates that no CodeView information is to be generated.
#
# "BROWSEONLY=0|1" (defaults to 0)
# TODO: Test this option. Is it functional?
# Set this option to build the browse files without re-building the library itself.
# Note: This option is used internally when BROWSE=1 is selected.
#
# "BROWSE=0|1" (defaults to 0)
# TODO: Test this option. Is it functional?
# Set this option to build the browse database for the library.
# By setting BROWSE=1, both the .SBRs and the .BSC files will be built along with the .OBJ and .LIB files
# that are part of the normal build process.
#
# "MT=0|1" (defaults to 1)
# If this item is 1 the library is built for multi-threading.
# If this item is 0 the library is built for single-threading.
#
# "UNICODE=0|1" (defaults to 0)
# To build a Unicode library instead of the default (SBCS) you can use UNICODE=1.
# This option overrides the MBCS setting.
#
# "MBCS=0|1" (defaults to 0)
# TODO: Test this option. Is it functional?
# To build an MBCS library instead of the default (SBCS) you can use MBCS=1.
# This creates a slightly larger library, but the code will work in far-east markets.
# This option has no effect when UNICODE=1.
#
# "INCREMENTAL=0|1" (defaults to 0)
# TODO: Test this option. Is it functional?
# This option enables incremental/minimal compilation and incremental linking.
#
# "PCH=0|1" (defaults to 1)
# Set this item to 0 to disable the default use of precompiled headers.
#
# "PDB=0|1" (defaults to 1)
# Set this item to 0 to disable the default use of PDB files.
#
# "OPT=options" (no default value)
# This allows additional compiler options to be added to the build.
# If more than one switch is desired, use quotes, e.g. "OPT=/J /W3".
#
# "OBJDIR=path" (default specific to variant)
# This optional specification specifies where temporary object files are stored during the build process.
# The directory is created or removed as necessary.
#
# "OPTSPEED=0|1" (defaults to 0)
# Set this item to 1 to optimize the code for speed.
# The default optimize code for size.


#----------------------------------------------------
# Check for obsolete arguments.
#
!ifdef COMPVER
! error The COMPVER parameter is obsolete; the compiler version is now detected automatically.
!endif

!ifdef CODEGUARD
! error Visual C++ does not support the CodeGuard build mode.
!endif

!ifdef VCL
! error Visual C++ does not support the VCL build mode.
!endif

!ifdef USE_UNICOWS
! error The USE_UNICOWS parameter is obsolete. Use UNICOWS instead.
!endif


#----------------------------------------------------
# Check arguments and define defaults if not defined.
#
!ifndef DLL
DLL=0
!endif

!ifndef COMPAT
COMPAT=0
!endif

!ifndef DEBUG
DEBUG=0
!endif

!ifndef STACKWALKER
STACKWALKER=0
!endif

!ifndef CODEVIEW
! if $(DEBUG) != 0
CODEVIEW=1
! else
CODEVIEW=0
! endif
!endif

!ifndef BROWSEONLY
BROWSEONLY=0
!endif

!if $(BROWSEONLY) == 1
!undef BROWSE
BROWSE=1
!endif

!ifndef BROWSE
BROWSE=0
!endif

!ifndef MT
MT=1
!endif

!ifdef UNICOWS
! error UNICOWS no longer supported
!endif

!ifndef UNICODE
UNICODE=0
!endif

!ifndef MBCS
MBCS=0
!endif

!ifndef INCREMENTAL
INCREMENTAL=0
!endif

!ifndef PCH
PCH=1
!endif

!ifndef PDB
PDB=1
!endif

!ifndef OPTSPEED
OPTSPEED=0
!endif


#-------------------------------------------------
# Tools
#
MSCL=cl
MSLINK=link
MSRC=rc
MSLIB=lib
MSML=ml
WCREV=SubWCRev.exe

!include ..\vc_detection.inc
!message COMPILER_VERSION = $(COMPILER_VERSION)
!message TARGET_PLATFORM = $(TARGET_PLATFORM)

!if $(COMPILER_VERSION) < 9
! error Visual C++ 2005 and earlier versions are no longer supported.
!endif


#-------------------------------------------------
# Build intermediate name.
#
!ifndef BASENAME
BASENAME=ocf
!endif

COMPILER=v$(COMPILER_VERSION)

!if $(COMPAT) > 0
COMPATIBILITY_MODE=$(COMPAT)
!else
COMPATIBILITY_MODE=
!ENDIF

!if $(DEBUG) != 0
DEBUG_MODE=d
!endif

!if $(MT) == 1
THREADING_MODE=t
!else
THREADING_MODE=s
!endif

!if $(UNICODE) == 1
CHARACTERTYPE_MODE=u
!endif

!if $(DLL) == 1
IMPLIB_MODE=i
!else
IMPLIB_MODE=
!endif

BUILD_MODES=$(COMPATIBILITY_MODE)$(DEBUG_MODE)$(THREADING_MODE)$(CHARACTERTYPE_MODE)
DELIM=-
INTERMEDIATENAME=$(BASENAME)$(DELIM)$(COMPILER)$(DELIM)$(TARGET_PLATFORM)$(DELIM)$(BUILD_MODES)$(IMPLIB_MODE)


#-------------------------------------------------
# Directories
#
ROOTDIR = ..\..

!ifndef BINDIR
BINDIR=$(ROOTDIR)\bin
!endif

!ifndef LIBDIR
LIBDIR=$(ROOTDIR)\lib
!endif

!ifndef OBJDIR
OBJDIR=$(ROOTDIR)\obj\$(INTERMEDIATENAME)
!endif

# Short-hand
D=$(OBJDIR)

!if [if not exist "$D" mkdir "$(D)"]
!error Unable to create intermediate directory.
!endif


#-------------------------------------------------
# Parse the OWLNext version number from the source.
#
!if [..\parse_version.bat OWL "..\..\include\owl\version.h" >$(OBJDIR)\OCF_VERSION.tmp]
! error Unable to detect the OCFNext version number.
!endif

!include $(OBJDIR)\OCF_VERSION.tmp
OCF_VERSION=$(OWL_VERSION)
OCF_PRERELEASE=$(OWL_PRERELEASE)

!if "$(OCF_VERSION)" == ""
! error OCF_VERSION is undefined (parsing failed).
!endif

!if [del "$(OBJDIR)\OCF_VERSION.tmp"]
! message Unable to remove temporary file "$(OBJDIR)\OCF_VERSION.tmp".
!endif

!message OCF_VERSION = $(OCF_VERSION)
!message OCF_PRERELEASE = $(OCF_PRERELEASE)


#-------------------------------------------------
# Build target name.
#
TARGETNAME=$(BASENAME)$(DELIM)$(OCF_VERSION)$(DELIM)$(COMPILER)$(DELIM)$(TARGET_PLATFORM)$(DELIM)$(BUILD_MODES)
TARGETLIB=$(TARGETNAME)$(IMPLIB_MODE).lib
!message TARGETLIB = $(TARGETLIB)

!if $(DLL) == 1
TARGETDLL=$(TARGETNAME).dll
!message TARGETDLL = $(TARGETDLL)
!endif

!message

#-------------------------------------------------
# DLL specific settings
#
!if $(DLL) == 1

!if $(DEBUG) == 0
CRTDLL=$(CRTDLL) msvcrt.lib
CRTDLL=$(CRTDLL) msvcprt.lib
!else
CRTDLL=$(CRTDLL) msvcrtd.lib
CRTDLL=$(CRTDLL) msvcprtd.lib
!endif

RCFLAGS=$(RCFLAGS) /r /DWIN32
!if $(DEBUG) != 0
RCFLAGS=$(RCFLAGS) /D_DEBUG
!endif

!if $(UNICODE) == 1
RCDEFINES=$(RCDEFINES) /D_UNICODE
!endif

!if $(DEBUG) == 1
LFLAGS=$(LFLAGS) /opt:noref /opt:noicf
!else
LFLAGS=$(LFLAGS) /opt:ref /opt:icf
!endif

!if $(CODEVIEW) == 1
PDBOPTS=/Fd$(BINDIR)\$(TARGETNAME).pdb
!endif

!if $(DEBUG) == 1
LFLAGS=$(LFLAGS) /debug
LFLAGS=$(LFLAGS) /pdb:$(BINDIR)\$(TARGETNAME).pdb
!else
LFLAGS=$(LFLAGS) /pdb:none
!endif

!if "$(TARGET_PLATFORM)" == "x64"
!if $(UNICODE) == 1
LFLAGS=$(LFLAGS) /base:0x14F800000
!else
LFLAGS=$(LFLAGS) /base:0x14F400000
!endif
!else
!if $(UNICODE) == 1
LFLAGS=$(LFLAGS) /base:0x4F800000
!else
LFLAGS=$(LFLAGS) /base:0x4F400000
!endif
!endif

!if $(COMPILER_VERSION) < 11
LFLAGS=$(LFLAGS) /version:$(OCF_VERSION)
!endif

!if $(OCF_PRERELEASE) == 0
RCDEFINES=$(RCDEFINES) /DRELEASE
LFLAGS=$(LFLAGS) /release
!endif

LFLAGS=$(LFLAGS) /dll
DLL_OPT=/D_DLL /D_BUILDOCFDLL /D_OWLDLL
RCDEFINES=$(RCDEFINES) /D_DLL /D_BUILDOCFDLL /D_OWLDLL
TARGOPTS=$(TARGOPTS) /MD /GF

!if $(MT) == 1
TARGOPTS=$(TARGOPTS) /D_MT
!endif

LIBS=$(LIBS) $(CRTDLL) kernel32.lib gdi32.lib user32.lib uuid.lib shell32.lib advapi32.lib ole32.lib oleaut32.lib version.lib

!endif # !if $(DLL) == 1


#-------------------------------------------------
# Static library specific (non-DLL) settings
#
!if $(DLL) == 0

!if $(MT) == 1
!if $(DEBUG) == 0
TARGOPTS=$(TARGOPTS) /MT
!else
TARGOPTS=$(TARGOPTS) /MTd
!endif
!endif

!if $(CODEVIEW) == 1
PDBOPTS=/Fd$(LIBDIR)\$(TARGETNAME).pdb
!endif

!endif # !if $(DLL) == 0


#-------------------------------------------------
# Common settings
#
!if $(DEBUG) != 0
DEBUGSUF=d
!ifndef DEBDEFS
DEBDEFS=/D__DEBUG=2 /D__TRACE /D__WARN
!endif
DEBDEFS=$(DEBDEFS) /D_DEBUG
DEBOPTS=/Od /Oy-
!else
DEBUGSUF=
DEBDEFS=
!if $(OPTSPEED) != 0
DEBOPTS=/O2
!else
DEBOPTS=/O1 /Gy
!endif
!endif

!if $(UNICODE) == 1
TARGDEFS=$(TARGDEFS) /D_UNICODE
!else
!if $(MBCS) == 1
TARGDEFS=$(TARGDEFS) /D_MBCS
!endif
!endif

# Compatibility settings
!if $(COMPAT) == 5
CDEFINES=$(CDEFINES) /DOWL5_COMPAT
!elseif $(COMPAT) != 0
!error Illegal value for COMPAT ($(COMPAT)). Must be 0 or 5.
!endif

# CodeView
!if $(CODEVIEW) == 1
! if $(PDB) == 0
CVOPTS=$(CVOPTS) /Z7
! else
CVOPTS=$(CVOPTS) /Zi
! endif
!endif

# Conformity settings
CDEFINES=$(CDEFINES)\
  /D_CRT_SECURE_NO_WARNINGS\
  /D_HAS_AUTO_PTR_ETC=0

# Compiler options
CL_OPT=/std:c++17 /permissive- /W3 /Zl /EHsc /GR $(CDEFINES) $(DEBOPTS) $(CVOPTS) $(TARGOPTS) /I$(ROOTDIR)\include /I.\ 

!if $(INCREMENTAL) == 1
CL_OPT=$(CL_OPT) /Gm
LFLAGS=$(LFLAGS) /incremental:yes
!else
CL_OPT=$(CL_OPT) /Gm-
LFLAGS=$(LFLAGS) /incremental:no
!endif

!if $(BROWSE) == 1
CL_OPT=/FR$D\ $(CL_OPT)
!endif

!if $(BROWSEONLY) == 1
CL_OPT=/Zs $(CL_OPT)
!else
CL_OPT=/Fo$D\ $(CL_OPT)
!endif

DEFS=$(DEFS) $(DEBDEFS) $(TARGDEFS)
CPPFLAGS=$(CPPFLAGS) $(CL_OPT) $(PDBOPTS) $(DEFS) $(OPT) $(DLL_OPT)
NOHDRFLAGS=$(CL_OPT) $(PDBOPTS) $(DEFS) $(OPT) $(DLL_OPT) /DNOHDR

!if $(PCH) == 1
PCH_CPP=ocfpch
! if $(BROWSE) == 1
PCH_FILE_NAME=$(PCH_CPP)b
! else
PCH_FILE_NAME=$(PCH_CPP)
! endif
PCH_FILE=$D\$(PCH_FILE_NAME).pch
PCH_HEADER="ocf/pch.h"
CPPFLAGS=$(CPPFLAGS) /D_OCFPCH /D_OWLPCH /Yu$(PCH_HEADER) /Fp$(PCH_FILE)
!endif


#-------------------------------------------------
# Library Components
#
OBJECT=\
$D\ocfpch.obj\
$D\appdesc.obj\
$D\autocli.obj\
$D\autodisp.obj\
$D\autoiter.obj\
$D\autostck.obj\
$D\autosrv.obj\
$D\autosym.obj\
$D\autoval.obj\
$D\occtrl.obj\
$D\ocreg.obj\
$D\ocstorag.obj\
$D\oleutil.obj\
$D\oledialg.obj\
$D\oledoc.obj\
$D\oleview.obj\
$D\oleframe.obj\
$D\olemdifr.obj\
$D\olewindo.obj\
$D\stgdoc.obj\
$D\typelib.obj\
$D\ocapp.obj\
$D\ocdoc.obj\
$D\ocpart.obj\
$D\ocremvie.obj\
$D\ocdata.obj\
$D\oclink.obj\
$D\ocview.obj\
$D\except.obj

# These are without precompiled header.
NO_HDR_OBJ=\
$D\version.obj\
$D\ocguid.obj

NO_HDR_CPP=\
version.cpp\
ocguid.cpp

# Collect them all
CPP_OBJS= $(OBJECT) $(NO_HDR_OBJ) $(OBJS_DLL)
OBJS=$(CPP_OBJS)


#-------------------------------------------------
# Goals to build
#
TARGETS=CreateDirs

!if $(BROWSEONLY) == 0
!if $(DLL) == 1
TARGETS=$(TARGETS) $(LIBDIR)\$(TARGETLIB) $(BINDIR)\$(TARGETDLL)
!else
TARGETS=$(TARGETS) $(LIBDIR)\$(TARGETLIB)
!endif
!endif

!if $(BROWSE) == 1
TARGETS=$(TARGETS) $(TARGETNAME).bsc
!endif

All: $(TARGETS)

CreateDirs:
  @-if not exist $D\*.* mkdir $D

Clean:
  -if exist $D\*.obj erase $D\*.obj
  -if exist $D\*.pch erase $D\*.pch
  -if exist $D\*.res erase $D\*.res
  -if exist $D\*.rsc erase $D\*.rsc
  -if exist $D\*.map erase $D\*.map
  -if exist $D\*.exp erase $D\*.exp
  -if exist $D\*.lib erase $D\*.lib
  -@if not exist $D\*.* if exist $D rmdir $D
  -if exist $(LIBDIR)\$(TARGETNAME).pdb del $(LIBDIR)\$(TARGETNAME).pdb
  -if exist $(LIBDIR)\$(TARGETNAME).idb del $(LIBDIR)\$(TARGETNAME).idb
  -if exist $(LIBDIR)\$(TARGETNAME).rep del $(LIBDIR)\$(TARGETNAME).rep
  -if exist $(LIBDIR)\$(TARGETNAME).exp del $(LIBDIR)\$(TARGETNAME).exp
  -if exist $(LIBDIR)\$(TARGETNAME).ilk del $(LIBDIR)\$(TARGETNAME).ilk
  -if exist $(LIBDIR)\$(TARGETNAME)i.pdb del $(LIBDIR)\$(TARGETNAME)i.pdb
  -if exist $(LIBDIR)\$(TARGETNAME)i.idb del $(LIBDIR)\$(TARGETNAME)i.idb
  -if exist $(LIBDIR)\$(TARGETNAME)i.rep del $(LIBDIR)\$(TARGETNAME)i.rep
  -if exist $(LIBDIR)\$(TARGETNAME)i.exp del $(LIBDIR)\$(TARGETNAME)i.exp
  -if exist $(LIBDIR)\$(TARGETNAME)i.ilk del $(LIBDIR)\$(TARGETNAME)i.ilk
  -if exist $(BINDIR)\$(TARGETNAME).pdb del $(BINDIR)\$(TARGETNAME).pdb
  -if exist $(BINDIR)\$(TARGETNAME).idb del $(BINDIR)\$(TARGETNAME).idb
  -if exist $(BINDIR)\$(TARGETNAME).rep del $(BINDIR)\$(TARGETNAME).rep
  -if exist $(BINDIR)\$(TARGETNAME).exp del $(BINDIR)\$(TARGETNAME).exp
  -if exist $(BINDIR)\$(TARGETNAME).ilk del $(BINDIR)\$(TARGETNAME).ilk


#-------------------------------------------------
# Suffix rules
#
.SUFFIXES: .cpp

!if $(BROWSE) != 0
.cpp{$D}.obj:
  $(MSCL) @<<
$(CPPFLAGS) /c $<
<<
  copy /b $*.sbr+pchmark.bin $*.sbr >NUL
!else
.cpp{$D}.obj::
  $(MSCL) @<<
$(CPPFLAGS) /c $<
<<
!endif

.cpp{$D}.sbr:
  $(MSCL) @<<
$(CPPFLAGS) /c $<
<<
  copy /b $*.sbr+pchmark.bin $*.sbr >NUL


#-------------------------------------------------
# Rule for precompiled header file
#
!if $(PCH) == 1

HDRS=$(ROOTDIR)\include\owl\*.h $(ROOTDIR)\include\owl\private\*.h $(ROOTDIR)\include\ocf\*.h

PCH_TARGETS=$D\$(PCH_CPP).obj $(PCH_FILE)
!if $(BROWSEONLY) == 1
PCH_TARGETS=$(PCH_TARGETS) $D\$(PCH_CPP).sbr
!endif

$(PCH_TARGETS): msc\$(PCH_CPP).cpp $(HDRS)
  $(MSCL) @<<
/Yc /Fp$(PCH_FILE) $(CL_OPT) $(DLL_OPT) $(PDBOPTS) $(DEFS) $(OPT) /D_OWLPCH /D_OCFPCH /c msc\$(PCH_CPP).cpp
<<
!if $(BROWSE) == 1
  copy /b $D\$(PCH_CPP).sbr+pchmark.bin $D\$(PCH_CPP).sbr>NUL
!endif

!if $(BROWSEONLY) == 1
$D\$(PCH_CPP).sbr: msc\$(PCH_CPP).cpp $(PCH_FILE)
!endif

!endif # !if $(PCH) == 1


#-------------------------------------------------
# Rule for no-precompiled-header object files
#
$(NO_HDR_OBJ): $(NO_HDR_CPP) $(HDRS)
  $(MSCL) @<<
$(NOHDRFLAGS) /c $(NO_HDR_CPP)
<<
!if $(BROWSE) == 1
  copy /b $*.sbr+pchmark.bin $*.sbr >NUL
!endif


#-------------------------------------------------
# Rule for the final library (static library option)
#
!if $(DLL) == 0

SBRS=$(CPP_OBJS:.obj=.sbr)

!if $(BROWSEONLY) == 1  # Set if the makefile is called recursively; see below.

# Browse database target
$(TARGETNAME).bsc: $(PCH_TARGETS) $(SBRS)
  bscmake /n /Iu /El /o$@ @<<
$(SBRS)
<<

!else # !if $(BROWSEONLY) == 1

# Final library target
$(LIBDIR)\$(TARGETLIB): $(PCH_TARGETS) $(OBJS) $(NO_HDR_OBJ)
  @-if exist $@ erase $@
  @$(MSLIB) /out:$@ @<<
$(OBJS)
<<

# Recursive target to build browse database
$(TARGETNAME).bsc: $(PCH_TARGETS) $(SBRS)
  $(MAKE) -fvc.mak @<<
BROWSEONLY=1 DEBUG=$(DEBUG) CODEVIEW=$(CODEVIEW) DLL=$(DLL) PCH=$(PCH) OBJDIR=$(OBJDIR) OPT=$(OPT)
<<

!endif # !if $(BROWSEONLY) == 1

!endif # !if $(DLL) == 0


#-------------------------------------------------
# Rule for the final library (DLL option)
#
!if $(DLL) == 1

# Statically compiled modules in DLL
PCH_STATIC=ocfpchs
CPPFLAGS_STATIC=$(CL_OPT) $(DEFS) $(DEBDEF) /I$(ROOTDIR)\include /D_OWLDLL /D_OWLPCH /D_OCFDLL /D_OCFPCH $(OPT) $(EH) /Gy /Fd$(LIBDIR)\$(TARGETNAME)i.pdb /Fp$D\$(PCH_STATIC).pch
!if $(DEBUG) != 0
CPPFLAGS_STATIC=$(CPPFLAGS_STATIC) /D_DEBUG
!endif
!if $(UNICODE) == 1
CPPFLAGS_STATIC=$(CPPFLAGS_STATIC) /D_UNICODE
!endif

$D\$(PCH_STATIC).obj $D\$(PCH_STATIC).pch: msc\$(PCH_STATIC).cpp
  $(MSCL) $(CPPFLAGS_STATIC) /Yc$(PCH_HEADER) /c msc\$(PCH_STATIC).cpp

CPP_STATIC=$(MSCL) /nologo $(CPPFLAGS_STATIC) /Yu$(PCH_HEADER) /c

# Resources
$D\$(BASENAME).res: ocfall.rc $(ROOTDIR)\include\ocf\version.h
  $(MSRC) $(RCFLAGS) $(RCDEFINES) /i$(ROOTDIR)\include;$(ROOTDIR)\include\owl;$(ROOTDIR)\include\ocf /fo $D\$(BASENAME).res ocfall.rc

# Final DLL target
DLL_OBJS=$(NO_HDR_OBJ) $(OBJECT) $(OBJS_DLL)
DLL_RESOURCES=$D\$(BASENAME).res

$(BINDIR)\$(TARGETDLL) $(LIBDIR)\$(TARGETLIB): $(DLL_OBJS) $(DLL_RESOURCES)
  $(MSLINK) @<<
$(LFLAGS) /libpath:$(LIBDIR)
$(LIBS)
$(DLL_OBJS)
$(DLL_RESOURCES)
/out:$(BINDIR)\$(TARGETDLL)
/pdb:$(BINDIR)\$(TARGETNAME).pdb
/map:$D\$(BASENAME).map
/implib:$(LIBDIR)\$(TARGETLIB)
<<

!endif # !if $(DLL) == 1
