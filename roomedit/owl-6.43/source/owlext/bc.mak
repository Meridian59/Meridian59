#-----------------------------------------------------------------------------
# bc.mak - makefile for building OwlExt
#
# OWL specific defines (parameters):
#
#   COMPVER              compiler version (required)
#   OWLVER               OWLNext version number (required)
#   DEBUG=0|1            Build the debug version of the target. (Default: 0)
#
#   -DCOMPILE_ASM        Compile assembler files (requires tasm)
#   -DNO_EMULATED_CTRL   Builds library using only system implementation of
#                        common controls
#
#   "COMPAT=0|1|2|5"     Builds library with the API exposed for backward compatibility 
#                        with the given version of OWL (1.0, 2.0 or 5.0).
#                        Setting this variable will define the respective compiler flags.
#
#   -DNO_OBJ_STREAMING   Builds library without special code to allow
#                        some OWL objects to be persisted in a stream
#
#   -DMT                 Build multi-thread version
#   -DUNICODE            Build unicode version
#   -DDLL                Build a DLL. (no default)
#   -DDIAGS              Build the diagnostic version of the target.
#   ALIGN=<1|2|4|8>      Specifies the alignment
#   BI_DBCS_SUPPORT      builds library with DBCS support
#   BI_MSG_JAPAN         builds library with japaneze support
#
# Borland C++ specific defines:
#
#   -DBCC32I             Uses bcc32i instead of bcc32 to compile
#   -DILINK              Uses ILINK32 instead of TLINK32 to link
#   CODEGUARD=0|1|2      Build the codeguard version of the target.
#                        0=Don't use CodeGuard, 1=basic data, 2=full checking
#

!if $d(OWLEXTNAME) 
BASENAME = $(OWLEXTNAME)
!endif

!if !$d(OWLVER)
! error OWLVER not defined. Can't build.
!endif
OWL_VERSION = $(OWLVER)

!if !$d(COMPVER)
! error COMPVER not defined. Can't build.
!endif

!if !$d(PLATFORM)
PLATFORM=x86
!endif

!if !$d(BCCVER)
BCCVER = $(COMPVER)
!endif

DEFFILE  = owlext.def
RESFILE  = owlext.res
LINKLIBS =

CDEFINES       = -D_OWLEXTPCH -DINC_OLE2 -D_OWLPCH
PCHFILE		   = "owlext\pch.h"

!if $d(DLL)
DLLCDEFS     = -D_BUILDOWLEXTDLL -D_OWLDLL -D_RTLDLL 
LIBDLL_DEFS    = -D_OWLDLL -D_OWLEXTDLL
LIBDLL_UNDEFS  = -U_BUILDOWLEXTDLL
!else
LIBCDEFS     = -DBUILD_OWLEXTLIB
!endif

!if !$d(CODEGUARD)
CODEGUARD = 0
!endif

!if !$d(DEBUG)
DEBUG = 0
!endif


#-----------------------------------------------------------------------------
# Various OWL settings
#-----------------------------------------------------------------------------

#
# Compatibility settings
#
!ifndef COMPAT
COMPAT=0
!endif
!if $(COMPAT) == 5
CDEFINES=$(CDEFINES) /DOWL5_COMPAT
!else
!if $(COMPAT) != 0
!error Illegal value for COMPAT ($(COMPAT)). Must be 0 or 5.
!endif
!endif
# !message COMPAT=$(COMPAT) and CDEFINES=$(CDEFINES)

#
# Build without persistent/stream support?
#
!if $d(NO_OBJ_STREAMING)
  CDEFINES = $(CDEFINES) -DBI_NO_OBJ_STREAMING
!endif



#-----------------------------------------------------------------------------
# The make target
#-----------------------------------------------------------------------------
Make: Setup All


##############################################################################
# Start of former common.inc
##############################################################################

.autodepend
.cacheautodepend
.swap

#--------------------------------------------------------------------
# Configure options based on settings.
# Check for incompatible choices and warn user.
#--------------------------------------------------------------------

#
# VCL compatibility implies use of namespace
#
!if $d(VCL)
!if $d(NONAMESPACE)
!error VCL compatibility implies use of namespace (i.e. don't define NONAMESPACE)
!endif
!endif

#
# VCL compatibility implies -D_USE_OWL_CM_EXIT
#
CDEFINES = $(CDEFINES) -D_USE_OWL_CM_EXIT
!if $d(VCL)
CDEFINES = $(CDEFINES) -DVCLRTL -DINC_VCL
!endif

#
# Unicode
#
!if $d(UNICODE) && !$d(_UNICODE)
_UNICODE = 1
!endif
!if $d(_UNICODE) && !$d(UNICODE)
UNICODE = 1
!endif

!if $d(UNICODE)
  CDEFINES = $(CDEFINES) -DUNICODE -D_UNICODE
!endif

#
# Diagnostics
#
!if $d(DIAGS)         # default diags use precondition, check, trace & warn
  __DEBUG=2
  __TRACE=1
  __WARN=1
!endif

#
# Build up compiler diag switches based on settings
#
!if $d(__DEBUG)
  CDEFINES = $(CDEFINES) -D__DEBUG=$(__DEBUG)
!endif

!if $d(__TRACE)
  CDEFINES = $(CDEFINES) -D__TRACE
!endif

!if $d(__WARN)
  CDEFINES = $(CDEFINES) -D__WARN
!endif

#
# Options for DLL
#
!if $d(DLL) && $d(DLLCDEFS)
  CDEFINES = $(CDEFINES) $(DLLCDEFS)
!endif

!if !$d(DLL) && $d(LIBCDEFS)
  CDEFINES = $(CDEFINES) $(LIBCDEFS)
!endif

#
# Options for DBCS
#
!if $d(BI_DBCS_SUPPORT)
  CFLAGS = $(CFLAGS) -DBI_DBCS_SUPPORT
!endif
!if $d(BI_MSG_JAPAN)
  CFLAGS = $(CFLAGS) -DBI_MSG_LANGUAGE=0x0411
!endif

#
# Figure out default data alignment based on target
#
!if !$d(ALIGN)
!  if $d(VCL)
     ALIGN = 8
!  else
!    if $(BCCVER) >= 6
       ALIGN = 8
!    else
       ALIGN = 1
!    endif
!  endif
!endif


#-----------------------------------------------------------------------------
# Build target name
#-----------------------------------------------------------------------------

!if !$d(BASENAME)
BASENAME=owlext
!endif

COMP=b$(COMPVER)

!if $(COMPAT) > 0
COMPATIBILITY_MODE=$(COMPAT)
!else
COMPATIBILITY_MODE=
!ENDIF

!if $(CODEGUARD) != 0
DEBUG_MODE=g
!else
! if $(DEBUG) == 1
DEBUG_MODE=d
! endif
!endif

!if "$(VCL)" == "1"
THREADING_MODE=v
!else
! if "$(MT)" == "1"
THREADING_MODE=t
! else
THREADING_MODE=s
! endif
!endif

!if "$(UNICODE)" == "1"
CHARACTERTYPE_MODE=u
!endif

BUILD_MODES = $(COMPATIBILITY_MODE)$(DEBUG_MODE)$(THREADING_MODE)$(CHARACTERTYPE_MODE)
DELIM = -

TARGETNAME = $(BASENAME)$(DELIM)$(OWL_VERSION)$(DELIM)$(COMP)$(DELIM)$(PLATFORM)$(DELIM)$(BUILD_MODES)
TEMPTARGETNAME = $(BASENAME)$(DELIM)$(COMP)$(DELIM)$(PLATFORM)$(DELIM)$(BUILD_MODES)


#-----------------------------------------------------------------------------
# Directory names
#-----------------------------------------------------------------------------

#!if !$d(ROOTDIR)
  ROOTDIR = ..\..
#!endif

!if !$d(TARGETDIR)
TARGETDIR = $(ROOTDIR)\bin
!endif

!if !$d(TARGETLIBDIR)
TARGETLIBDIR = $(ROOTDIR)\lib
!endif

!if !$d(SRCROOT)
SRCROOT   = $(ROOTDIR)\source
!endif
!if !$d(INCDIR)
INCDIR    = $(ROOTDIR)\include
!endif
!if !$d(RCINCDIR)
RCINCDIR  = $(ROOTDIR)\include
!endif
!if !$d(LIBDIR)
LIBDIR    = $(ROOTDIR)\lib
!endif
!if !$d(OBJROOT)
OBJROOT   = $(ROOTDIR)\obj
!endif
!if !$d(OBJDIR)
!if $d(DLL)
OBJDIR    = $(OBJROOT)\$(TEMPTARGETNAME)i
!else
OBJDIR    = $(OBJROOT)\$(TEMPTARGETNAME)
!endif
!endif

.PATH.obj = $(OBJDIR)
.PATH.res = $(OBJDIR)
.PATH.def = $(OBJDIR)
.PATH.lib = $(LIBDIR)


!if !$d(FULLTARGET)
FULLTARGET     = $(TARGETDIR)\$(TARGETNAME)
!endif


#-----------------------------------------------------------------------------
# Create full pathnames for DEFFILE and RESFILE
#-----------------------------------------------------------------------------

!if $d(DEFFILE)
  DEFFILE = $(OBJDIR)\$(DEFFILE)
!endif
!if $d(RESFILE)
  RESFILE = $(OBJDIR)\$(RESFILE)
!endif


#-----------------------------------------------------------------------------
# Borland specific directive (previously "bc5.inc" include file)
#-----------------------------------------------------------------------------

!if $(BCCVER) >= 3
  ILINK=1
!endif

!if $d(VCL)
  MT = 1
!endif

!if $d(MT)
  RTL_MT = -WM
  RTLMT  = mt
!else
  RTL_MT = -WM-
!endif

#
# If building for VCL, use an RTL that begins "cp" instead of "cw"
#
!if $d(VCL)
  RTLPFX=cp
!else
  RTLPFX=cw
!endif

#--------------------------------------------------------------------
# dir & search paths, & precompiled headers
#--------------------------------------------------------------------

#
# BC tools BCBINDIR can be based on MAKEDIR if no BCROOT is defined
#
!if !$d(BCROOT) && !$d(BCBROOT)
! include $(MAKEDIR)\bcroot.inc
! if !$d(BCROOT) && !$d(BCBROOT)
    BCROOT = $(MAKEDIR)\..
! endif
!endif

#
# At least one of the roots is certainly defined now.
# If only one is defined, make them both the same.
#
!if !$d(BCROOT)
  BCROOT = $(BCBROOT)
!elif !$d(BCBROOT)
  BCBROOT = $(BCROOT)
!endif


#
# Where to find BC libraries and include files
#
! if $(BCCVER) >= 13
   CG_BOOST_ROOT = $(BCROOT)\include\boost_1_39
! endif
! if $(BCCVER) >= 12
   INCDIR = $(INCDIR);"$(BCROOT)\include\windows\crtl";"$(BCROOT)\include\windows\sdk"
! endif
!if $(BCCVER) >= 9
  INCDIR = $(INCDIR);"$(CG_BOOST_ROOT)\boost\tr1\tr1\bcc32";"$(CG_BOOST_ROOT)\boost\tr1\tr1";"$(CG_BOOST_ROOT)";"$(BCROOT)\include\dinkumware";"$(BCROOT)\include"
!elif $(BCCVER) >= 8
  INCDIR = $(INCDIR);"$(BCROOT)\include\dinkumware";"$(BCROOT)\include"
!else
  INCDIR = $(INCDIR);"$(BCROOT)\include\Win32";"$(BCROOT)\include"
!endif

!if $(DEBUG) == 1
LIBDIR   = $(LIBDIR);"$(BCROOT)\lib"
! if $(BCCVER) >= 12
   LIBDIR   = $(LIBDIR);"$(BCROOT)\lib\win32\debug"
! else
   LIBDIR   = $(LIBDIR);"$(BCROOT)\lib\debug"
! endif
!else
LIBDIR   = $(LIBDIR);"$(BCROOT)\lib"
! if $(BCCVER) >= 12
   LIBDIR   = $(LIBDIR);"$(BCROOT)\lib\win32\release"
! else
   LIBDIR   = $(LIBDIR);"$(BCROOT)\lib\release"
! endif
!endif

!if $(BCCVER) >= 12
  RCINCDIR = $(RCINCDIR);"$(BCROOT)\include\windows\sdk"
!else
  RCINCDIR = $(RCINCDIR);"$(BCROOT)\include"
!endif
BCBINDIR = $(BCROOT)\bin


#
# VCL directories for object and header files
#
!if $d(VCL)
! if $(BCCVER) >= 13
    VCLOBJDIR = "$(BCBROOT)\lib\win32\release"
! else
    VCLOBJDIR = "$(BCBROOT)\lib\obj"
! endif
! if $(BCCVER) >= 12
    VCLINCDIR = "$(BCBROOT)\include\windows\vcl"
! else
    VCLINCDIR = "$(BCBROOT)\include\vcl"
! endif
INCDIR = $(INCDIR);$(VCLINCDIR)
!endif


#-----------------------------------------------------------------------------
# Borland C++ 32-bit options
#-----------------------------------------------------------------------------

!if $d(BCC32I)
   CC     = "$(BCBINDIR)\bcc32i"      # Highly optimizing compiler
!else
   CC     = "$(BCBINDIR)\bcc32"       # Regular optimizing compiler
!endif

!if $(DEBUG) == 1
  RCC      = "$(BCBINDIR)\brcc32" -w32 -DWIN32 -D_DEBUG # Resource compiler
  CPP      = "$(BCBINDIR)\cpp32" -DWIN32 -D_DEBUG # Preprocessor
!else 
  RCC      = "$(BCBINDIR)\brcc32" -w32 -DWIN32 # Resource compiler
  CPP      = "$(BCBINDIR)\cpp32" -DWIN32 # Preprocessor
!endif

RLINK    = "$(BCBINDIR)\brcc32"      # Resource binder/linker
ASM      = "$(BCBINDIR)\tasm32"      # Assembler
  
!if $d(ILINK)
  LINK     = "$(BCBINDIR)\ilink32"     # Linker
!else
  LINK     = "$(BCBINDIR)\tlink32"     # Linker
!endif

LIB      = "$(BCBINDIR)\tlib"        # Librarian
IMPLIB   = "$(BCBINDIR)\implib"      # Import library generator
TDSTRIP  = "$(BCBINDIR)\tdstrp32"    # Debug info extractor

CFLAGS   = $(CFLAGS) -d            # duplicate strings merged
!if $(DEBUG) == 1
   CFLAGS = $(CFLAGS) -v -k -Od     # debug-info on, normal stack, no opt.
!else
!  if $(BCCVER) > 8
     CFLAGS = $(CFLAGS) -3 -k- -O2   # Optimizations -O2 , stdcall
!  else
     CFLAGS = $(CFLAGS) -3 -k- -Ogmpv   # Optimizations -O1gmpv , stdcall
!  endif
!endif

!if $(BCCVER) >= 9 && $(BCCVER) <= 10
   CFLAGS = $(CFLAGS) -w-inl     # to prevent too many warnings 'Functions with exception specifications are not expanded inline'
!endif


!if $(CODEGUARD) != 0                # Enable compiler's CodeGuard support
   CFLAGS = $(CFLAGS) -vGd -vGt     # d=add data, t=check this
!  if $(CODEGUARD) == 2
     CFLAGS = $(CFLAGS) -vGc        # c=add calls
!  endif
!endif

CFGFILE  = bcc32.cfg               # Name of compiler's config file

!if $d(DLL)
!  if $(DEBUG) == 1
     LIBOPTS = /P64 #/C
!  else
     LIBOPTS = /P64 /0 #/C
!  endif
!else
!  if $(DEBUG) == 1
     LIBOPTS = /P4096 #/C
!  else
     LIBOPTS = /P1024 /0 #/C
!  endif
!endif

!if $(DEBUG) == 1
   LDBG = -v
!endif

!if $(DEBUG) == 1
   LIBOPTS = /P2048 #/C
!else
   LIBOPTS = /P256 /0 #/C
!endif

!if $d(DLL)
   CFLAGS = $(CFLAGS) -D_RTLDLL
!  if $(BCCVER) >= 6
     CFLAGS = $(CFLAGS) -WD
!  else
     CFLAGS = $(CFLAGS) -WDE
!  endif
   LFLAGS = $(LDBG) $(ILINKFLAG) -Tpd -c -aa -x -OS -V4.0
!  if $d(ILINK)
     LFLAGS = $(LFLAGS) -Gi
!  endif
!  if $d(IMAGEBASE)
!    if $d(ILINK)
       LFLAGS = $(LFLAGS) -b:$(IMAGEBASE)
!    else
       LFLAGS = $(LFLAGS) -B:$(IMAGEBASE)
!    endif
!  endif
   STDLIBS  = import32 ole2w32.lib $(RTLPFX)32$(RTLMT)i   # Libs for linking the DLL
!  if $(CODEGUARD) != 0
     STDLIBS  = cg32 $(STDLIBS)
!  endif
!  if $d(UNICODE)
     STARTUP  = c0d32w
!  else
     STARTUP  = c0d32
!  endif
!else
   CFLAGS = -W $(CFLAGS)
!endif

!if $d(ILINK)
   LFLAGS = $(LFLAGS) -Gn
!endif

!if $d(VCL)
!if $(BCCVER) >= 13
   STDLIBS = vcle rtl $(STDLIBS)
!elif $(BCCVER) >= 6
   STDLIBS = vcllink $(STDLIBS)
!else
   STDLIBS = vcl $(STDLIBS)
!endif
   CFLAGS  = -b- -Vx -Ve $(CFLAGS) -DVCLRTL -DINC_VCL
   LFLAGS  = $(LFLAGS) -j$(VCLOBJDIR)
!endif

!if $d(MT)
   CFLAGS = $(CFLAGS) -WM
!else
   CFLAGS = -WM- $(CFLAGS)
!endif

!if $d(VCL)
   CFLAGS = -a4 $(CFLAGS)
!elif $d(ALIGN)
   CFLAGS = -a$(ALIGN) $(CFLAGS)
!endif

CFGFILE = $(OBJDIR)\$(CFGFILE)
CPCH    = -H -H=$(OBJDIR)\pch.csm
!if $d(PCHFILE)
CPCH    = $(CPCH) -H$(PCHFILE)
!else
CPCH    = $(CPCH) -H"pch.h" 
!endif
!if $(BCCVER) != 8
CPCH    = $(CPCH) -Hc
!endif

CFLAGS_WIN = -W $(CFLAGS_WIN)


#-----------------------------------------------------------------------------
# Tools
#-----------------------------------------------------------------------------

CC         = $(CC) +$(CFGFILE)
CCW        = $(CC) $(CFLAGS_WIN)
LINK       = $(LINK) $(LFLAGS) -L$(LIBDIR) $(STARTUP)
LIB        = $(LIB) $(LIBOPTS)
IMPLIB_I   = $(IMPLIB) -i -w          # Ignore WEP, no warnings
#WCREV=SubWCRev_TEST_NONEXISTENCE.exe
WCREV=SubWCRev.exe

#
# Special variant of compiler,
# for objs that goes into the lib when building a dll
#
CC_LIBDLL = $(CC) -W
CC_LIBDLL = $(CC_LIBDLL) $(LIBDLL_DEFS) $(LIBDLL_UNDEFS) 

  CCNO     = $(CC)


!if $d(LINKLIBS)
  TEMP_001 = $(LINKLIBS:TARGETSFX=$(TARGETSFX))
  USERLIBS = $(TEMP_001:;= )
!endif

LINKSFX = ,$(FULLTARGET).dll, nul,$(USERLIBS) $(STDLIBS),
!if $d(DEFFILE)
  LINKSFX = $(LINKSFX) $(DEFFILE)
!endif

LIBSFX =
LINKOBJPFX =
LINKOBJSFX =
LIBOBJPFX = +"
LIBOBJSFX = "


##############################################################################
# End of former common.inc
##############################################################################


#-----------------------------------------------------------------------------
#
# Build the various file lists needed for dependency checking,
# and LIBing.
#
#   OBJS is the main list, conditionalized for the various targets
#       and options.
#
#   OBJS_LIB are the objs that only go in libs & implib, not the dll
#
#   OBJS_DLL are the objs that only go in the dll, not the libs
#
#   DEPOBJS is the list of object files for dependency checking
#
#   LIBOBJS is the list of object files for building the library
#
#-----------------------------------------------------------------------------

#
# Common, unsectioned objs go into static libs and DLLs
#

OBJS = \
 PFXbmpmenu.objSFX\
 PFXbrowse.objSFX\
 PFXbusy.objSFX\
 PFXbutappdlg.objSFX\
 PFXcalcedit.objSFX\
 PFXcaledit.objSFX\
 PFXcolorhls.objSFX\
 PFXcolpick.objSFX\
 PFXcomboexp.objSFX\
 PFXctxhelpm.objSFX\
 PFXdirdialg.objSFX\
 PFXdlgtoolb.objSFX\
 PFXdockingex.objSFX\
 PFXeditlist.objSFX\
 PFXenumwnd.objSFX\
 PFXgadgctrl.objSFX\
 PFXgadgdesc.objSFX\
 PFXgadgetex.objSFX\
 PFXgaugegad.objSFX\
 PFXharborex.objSFX\
 PFXhlinkctl.objSFX\
 PFXkeyhook.objSFX\
 PFXmenubtn.objSFX\
 PFXmousehk.objSFX\
 PFXmsghook.objSFX\
 PFXmultiwin.objSFX\
 PFXnotebook.objSFX\
 PFXowlext.objSFX\
 PFXpickedit.objSFX\
 PFXpictdeco.objSFX\
 PFXpictdlg.objSFX\
 PFXpopedit.objSFX\
 PFXprogress.objSFX\
 PFXsimplapp.objSFX\
 PFXstaticbm.objSFX\
 PFXtabwin.objSFX\
 PFXthredprg.objSFX\
 PFXtimer.objSFX\
 PFXtipdialg.objSFX\
 PFXvalidate.objSFX\
 PFXtoolcustdlg.objSFX

#
# Put together obj groups into lists for different dependencies & tools
#
LIBOBJPFX_DIR = $(LIBOBJPFX)$(OBJDIR)\\
LINKOBJPFX_DIR = $(LINKOBJPFX)$(OBJDIR)\\

!if $d(DLL)
	LIBDEPOBJS_S  = $(OBJS_LIB:PFX=)
	LINKDEPOBJS_S = $(OBJS:PFX=) $(OBJS_DLL:PFX=)
	LIBOBJS_S     = $(OBJS_LIB:PFX=$(LIBOBJPFX_DIR))
	LINKOBJS_S    = $(OBJS:PFX=$(LINKOBJPFX_DIR)) $(OBJS_DLL:PFX=$(LINKOBJPFX_DIR)) $(ASM_OBJS:PFX=$(LINKOBJPFX))
!else
	LIBDEPOBJS_S  = $(OBJS:PFX=) $(OBJS_LIB:PFX=)
	LIBOBJS_S     = $(OBJS:PFX=$(LIBOBJPFX_DIR)) $(OBJS_LIB:PFX=$(LIBOBJPFX_DIR)) $(ASM_OBJS_LIB:PFX=$(LIBOBJPFX))
!endif

LIBDEPOBJS  = $(LIBDEPOBJS_S:SFX=)
LINKDEPOBJS = $(LINKDEPOBJS_S:SFX=)
LIBOBJS     = $(LIBOBJS_S:SFX=$(LIBOBJSFX))
LINKOBJS    = $(LINKOBJS_S:SFX=$(LINKOBJSFX))

#--------------------------------------------------------------------
# Rules
#--------------------------------------------------------------------

.rc.res:
  $(RCC) -i$(RCINCDIR) -fo$@ $.

.cpp.obj:
  $(CC) {$& }

.asm.obj:
  $(ASM) /mx $& $@


#-----------------------------------------------------------------------------
# Setup - pre-build targets
#-----------------------------------------------------------------------------

Setup: CreateDirs InitCompiler

CreateDirs:
  @-if not exist $(TARGETDIR)\NUL md $(TARGETDIR)
  @-if not exist $(TARGETLIBDIR)\NUL md $(TARGETLIBDIR)
  @-if not exist $(OBJROOT)\NUL md $(OBJROOT)
  @-if not exist $(OBJDIR)\NUL md $(OBJDIR)

# Create compiler response file

InitCompiler:
  @copy &&|
-I$(INCDIR)
-c -w -g255 $(CFLAGS) $(CDBG) $(CDEFINES)
$(CPCH)
-n$(OBJDIR)
| $(CFGFILE) > NUL


#--------------------------------------------------------------------
# Resource file
#--------------------------------------------------------------------

# VH, May 2006:
# The resource file (owlext.res) needs preprocessing of "version.rc".
# Preprocessing is necessary because of the limitations of the 
# preprocessor inside the Borland Resource Compiler. The stand-alone
# preprocessor hasn't got these problems so it is used to preprocess. 
# Then "owlext.rc" will include "version.i" instead (see "owlext.rc").

version.i: version.rc $(ROOTDIR)\include\owlext\version.h
  @echo Preprocessing version.rc
  $(CPP) -DRC_INVOKED -I$(RCINCDIR) -o$@ version.rc

$(RESFILE): version.i
  $(RCC) -i$(RCINCDIR) -fo$@ $&.rc


#--------------------------------------------------------------------
# Targets
#--------------------------------------------------------------------

!if $d(DLL)
TARGETLIBNAME = $(TARGETNAME)i.lib
!else
TARGETLIBNAME = $(TARGETNAME).lib
!endif

LIBTARGET = $(TARGETLIBDIR)\$(TARGETLIBNAME)


!if $d(DLL)

DLLTARGET = $(FULLTARGET).dll

All: PrintBuildSettings $(DLLTARGET) 

PrintBuildSettings :
  @echo DLLTARGET = $(DLLTARGET)
  @echo LIBTARGET = $(LIBTARGET)
  @echo BCCVER = $(BCCVER)
  @echo OWL_VERSION = $(OWL_VERSION)
  @echo CDEFINES = $(CDEFINES)

#
# Link and implib
#
$(DLLTARGET): $(LINKDEPOBJS) $(LIBDEPOBJS) $(DEFFILE) $(RESFILE)
	@cd $(.PATH.obj)
	$(LINK) @&&|
$(LINKOBJS) $(LINKSFX),$(RESFILE)
|
! if !$d(ILINK)
		$(TDSTRIP) -s $(DLLTARGET)
		$(IMPLIB) $(LIBTARGET) $(DLLTARGET)
! else
	 -del $(LIBTARGET)
	 copy $(DLLTARGET:.dll=.lib) $(LIBTARGET)
	 @-del 	$(DLLTARGET:.dll=.lib)
! endif
  $(LIB) "$(LIBTARGET)" @&&|
$(LIBOBJS) $(LIBSFX) 
|
  @-del $(LIBTARGET:.lib=.bak)
	@cd $(.PATH.cpp)

#
# Win32 def-file
#
$(DEFFILE):
	@copy &&|
LIBRARY     "$(TARGETNAME)" INITINSTANCE
DESCRIPTION 'OWL Extensions DLL'
EXETYPE     WINDOWS
DATA        MULTIPLE
EXPORTS
| $@ > NUL

#
# Compile objs that aren't part of the Owl DLL as if they were user EXE or
# user DLL objs
#
#$(LIBDEPOBJS): 
#  $(CC_LIBDLL) {$&.cpp }


!else # ! DLL


All: PrintBuildSettings $(LIBTARGET)

PrintBuildSettings :
  @echo LIBTARGET = $(LIBTARGET)
  @echo BCCVER = $(BCCVER)
  @echo OWL_VERSION = $(OWL_VERSION)
  @echo CDEFINES = $(CDEFINES)

$(LIBTARGET): $(LIBDEPOBJS)
  @if exist $(LIBTARGET)  del $(LIBTARGET)
  $(LIB) "$(LIBTARGET)" @&&|
$(LIBOBJS) $(LIBSFX)
|
  @-del $(FULLTARGET).bak


!endif # if/else DLL










