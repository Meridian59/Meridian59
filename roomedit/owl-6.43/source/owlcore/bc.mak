#
# bc.mak - makefile for building OWLNext
#
# Part of OWLNext - the next generation Object Windows Library 
# Copyright (c) 1998 by Mikael Lindgren (m@smtdata.se)
#
# OWL specific defines (parameters):
#
#   OWLVER=x.x           Sets the library version number (required).
#   COMPVER=xx           Sets the compiler version; see below for table.
#   DEBUG=0|1            If 1, builds the debug version (default 0).
#   CODEGUARD=0|1|2      0 = no CodeGuard (default), 1 = basic data, 2 = full checking.
#   COMPAT=0|5           If 5, builds a version with increased OWL 5.0 compatibility (default 0).
#   ALIGN=1|2|4|8        Specifies the alignment (default 8).
#   SETUP=0|1            Prepares the source code for building (default 1).
#
#   -DDLL                Build the library as a DLL.
#   -DUNICODE            Build the library with Unicode support.
#   -DMT                 Links the library with the multi-threaded RTL.
#   -DVCL                Links the library with VCL multi-threaded RTL.
#   -DNO_OBJ_STREAMING   Builds the library without persistent streams.
#
# Supported compiler versions:
#
#   COMPVER=720          C++Builder 10.1 Berlin (0x720)
#   COMPVER=710          C++Builder 10 Seattle (0x710)
#   COMPVER=19           C++Builder XE8 (0x700)
#   COMPVER=18           C++Builder XE7 (0x690)
#   COMPVER=17           C++Builder XE6 (0x680)
#   COMPVER=16           C++Builder XE5 (0x670)
#   COMPVER=15           C++Builder XE4 (0x660)
#   COMPVER=14           C++Builder XE3 (0x650)
#   COMPVER=13           C++Builder XE2 (0x640)
#   COMPVER=12           C++Builder XE (0x630)
#

#-----------------------------------------------------------------------------
# Handle arguments
#-----------------------------------------------------------------------------

#
# OWLNext version
#
!if !$d(OWLVER)
!  error OWLVER not defined. Can't build.
!endif
OWL_VERSION = $(OWLVER)

#
# Compiler version
#
!if !$d(COMPVER)
!  error COMPVER not defined. Can't build.
!elif $(COMPVER) < 12
!  error Unsupported compiler version; COMPVER must be >= 12 (0x630/XE).
!endif
BCCVER = $(COMPVER)

#
# Debugging
#
!if !$d(DEBUG)
   DEBUG = 0
!endif
!if $(DEBUG) != 0
   __DEBUG = 2
   __TRACE = 1
   __WARN = 1
   CDEFINES = $(CDEFINES) -D__DEBUG=$(__DEBUG)
   CDEFINES = $(CDEFINES) -D__TRACE
   CDEFINES = $(CDEFINES) -D__WARN
!endif

#
# CodeGuard support
#
!if !$d(CODEGUARD)
   CODEGUARD = 0
!endif

#
# Compatibility settings
#
!ifndef COMPAT
   COMPAT = 0
!endif
!if $(COMPAT) == 5
   CDEFINES = $(CDEFINES) /DOWL5_COMPAT
!else
!  if $(COMPAT) != 0
!    error Illegal value for COMPAT ($(COMPAT)). Must be 0 or 5.
!  endif
!endif

#
# Data alignment
#
!if !$d(ALIGN)
   ALIGN = 8
!endif

#
# Setup
#
!if !$d(SETUP)
   SETUP = 1
!endif

#
# Dynamic linking
#
!if $d(DLL)
   DLLCDEFS = -D_BUILDOWLDLL
   LIBDLL_DEFS = -D_OWLDLL
   LIBDLL_UNDEFS = -U_BUILDOWLDLL
!endif
!if $d(DLL) && $d(DLLCDEFS)
   CDEFINES = $(CDEFINES) $(DLLCDEFS)
!endif

#
# Unicode support
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
# Multi-threaded RTL support
#
!if $d(MT)
   RTLMT = mt
!endif

#
# VCL support
# VCL compatibility implies -D_USE_OWL_CM_EXIT
#
CDEFINES = $(CDEFINES) -D_USE_OWL_CM_EXIT
!if $d(VCL)
   CDEFINES = $(CDEFINES) -DVCLRTL -DINC_VCL
   MT = 1
   RTLPFX = cp # If building for VCL, use an RTL that begins "cp" instead of "cw".
!else
   RTLPFX = cw
!endif

#
# Persistent streaming
#
!if $d(NO_OBJ_STREAMING)
   CDEFINES = $(CDEFINES) -DBI_NO_OBJ_STREAMING
!endif

#
# Undocumented (obsolete) options
#
!if $d(BI_DBCS_SUPPORT)
   CFLAGS = $(CFLAGS) -DBI_DBCS_SUPPORT
!endif
!if $d(BI_MSG_JAPAN)
   CFLAGS = $(CFLAGS) -DBI_MSG_LANGUAGE=0x0411
!endif

#-----------------------------------------------------------------------------
# Build target name
#-----------------------------------------------------------------------------

BASENAME = owl

COMP = b$(COMPVER)

PLATFORM = x86

!if $(COMPAT) > 0
   COMPATIBILITY_MODE = $(COMPAT)
!else
   COMPATIBILITY_MODE =
!ENDIF

!if $(CODEGUARD) != 0
   DEBUG_MODE = g
!else
!  if $(DEBUG) == 1
     DEBUG_MODE = d
!  endif
!endif

!if "$(VCL)" == "1"
   THREADING_MODE = v
!else
!  if "$(MT)" == "1"
     THREADING_MODE = t
!  else
     THREADING_MODE = s
!  endif
!endif

!if "$(UNICODE)" == "1"
   CHARACTERTYPE_MODE = u
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
   SRCROOT = $(ROOTDIR)\source
!endif
!if !$d(INCDIR)
   INCDIR = $(ROOTDIR)\include
!endif
!if !$d(RCINCDIR)
   RCINCDIR = $(ROOTDIR)\include
!endif
!if !$d(LIBDIR)
   LIBDIR = $(ROOTDIR)\lib
!endif
!if !$d(OBJROOT)
   OBJROOT = $(ROOTDIR)\obj
!endif
!if !$d(OBJDIR)
!  if $d(DLL)
     OBJDIR = $(OBJROOT)\$(TEMPTARGETNAME)i
!else
     OBJDIR = $(OBJROOT)\$(TEMPTARGETNAME)
!endif
!endif

.PATH.obj = $(OBJDIR)
.PATH.res = $(OBJDIR)
.PATH.def = $(OBJDIR)
.PATH.lib = $(LIBDIR)

#-----------------------------------------------------------------------------
# Create full pathnames
#-----------------------------------------------------------------------------

!if $d(DLL)
   DLLTARGET = $(TARGETDIR)\$(TARGETNAME).dll
   LIBTARGET = $(TARGETLIBDIR)\$(TARGETNAME)i.lib
!else
   LIBTARGET = $(TARGETLIBDIR)\$(TARGETNAME).lib
!endif

DEFFILE = $(OBJDIR)\owlp.def
RESFILE = $(OBJDIR)\owl.res

#--------------------------------------------------------------------
# Search paths
#--------------------------------------------------------------------

#
# BC tools BCBINDIR can be based on MAKEDIR if no BCROOT is defined.
#
!if !$d(BCROOT) && !$d(BCBROOT)
!  if $d(BDS)
     BCROOT = $(BDS)
!  endif
!  include $(MAKEDIR)\bcroot.inc
!  if !$d(BCROOT) && !$d(BCBROOT)
     BCROOT = $(MAKEDIR)\..
!  endif
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
# Where to find BC libraries and include files.
#
!if $(BCCVER) >= 13
   CG_BOOST_ROOT = $(BCROOT)\include\boost_1_39
!endif
INCDIR = $(INCDIR);"$(BCROOT)\include\windows\crtl";"$(BCROOT)\include\windows\sdk"
INCDIR = $(INCDIR);"$(CG_BOOST_ROOT)\boost\tr1\tr1\bcc32";"$(CG_BOOST_ROOT)\boost\tr1\tr1";"$(CG_BOOST_ROOT)"
INCDIR = $(INCDIR);"$(BCROOT)\include\dinkumware";"$(BCROOT)\include"

!if $(DEBUG) == 1
   LIBDIR = $(LIBDIR);"$(BCROOT)\lib"
!  if $(BCCVER) >= 12
     LIBDIR = $(LIBDIR);"$(BCROOT)\lib\win32\debug"
!  else
     LIBDIR = $(LIBDIR);"$(BCROOT)\lib\debug"
!  endif
!else
   LIBDIR = $(LIBDIR);"$(BCROOT)\lib"
!  if $(BCCVER) >= 12
     LIBDIR = $(LIBDIR);"$(BCROOT)\lib\win32\release"
!  else
     LIBDIR = $(LIBDIR);"$(BCROOT)\lib\release"
!  endif
!endif

RCINCDIR = $(RCINCDIR);"$(BCROOT)\include\windows\sdk"
BCBINDIR = $(BCROOT)\bin

#
# VCL directories for object and header files
#
!if $d(VCL)
!  if $(BCCVER) >= 13
     VCLOBJDIR = "$(BCBROOT)\lib\win32\release"
!  else
     VCLOBJDIR = "$(BCBROOT)\lib\obj"
!  endif
   VCLINCDIR = "$(BCBROOT)\include\windows\vcl"
   INCDIR = $(INCDIR);$(VCLINCDIR)
!endif

#-----------------------------------------------------------------------------
# Borland C++ 32-bit options
#-----------------------------------------------------------------------------

CC = "$(BCBINDIR)\bcc32"

!if $(DEBUG) == 1
   RCC = "$(BCBINDIR)\brcc32" -w32 -DWIN32 -D_DEBUG # Resource compiler
   CPP = "$(BCBINDIR)\cpp32" -DWIN32 -D_DEBUG # Preprocessor
!else 
   RCC = "$(BCBINDIR)\brcc32" -w32 -DWIN32 # Resource compiler
   CPP = "$(BCBINDIR)\cpp32" -DWIN32 # Preprocessor
!endif

RLINK = "$(BCBINDIR)\brcc32" # Resource binder/linker
ASM = "$(BCBINDIR)\tasm32" # Assembler
  
ILINK = 1
!if $d(ILINK)
   LINK = "$(BCBINDIR)\ilink32" # Linker
!else
   LINK = "$(BCBINDIR)\tlink32" # Linker
!endif

LIB = "$(BCBINDIR)\tlib" # Librarian
IMPLIB = "$(BCBINDIR)\implib" # Import library generator
TDSTRIP = "$(BCBINDIR)\tdstrp32" # Debug info extractor

CFLAGS = $(CFLAGS) -d # Merge duplicate strings.
!if $(DEBUG) == 1
   CFLAGS = $(CFLAGS) -v -k -Od # Debug (-v), normal stackframe (-k), and disable optimizations (-Od).
!else
   CFLAGS = $(CFLAGS) -O1 # Generate smallest code (-O1).
!endif

#
# Disable meretricious warnings.
#
CFLAGS = $(CFLAGS) -w-amp # W8001 "Superfluous & with function"; plain wrong.
CFLAGS = $(CFLAGS) -w-bcx # W8126 "Base class of exported class should have exported constructor"; seems too pedantic. TODO: Investigate.
CFLAGS = $(CFLAGS) -w-bex # W8120 "Base class of dllexport class should also be dllexport"; too pedantic; doesn't accept inheritance from system structs.
CFLAGS = $(CFLAGS) -w-ccc # W8008 "Condition is always true/false"; too pedantic, e.g. for asserts and compile-time evaluation.
CFLAGS = $(CFLAGS) -w-hid # W8022 "'%s' hides virtual function '%s'"; too pedantic, and being generated for "gdiplus.h" system header.
CFLAGS = $(CFLAGS) -w-mls # W8104 "Local Static with constructor dangerous for multi-threaded apps"; OWLNext isn't thread-safe anyway.
CFLAGS = $(CFLAGS) -w-mtx # W8122 "dllexport class member '%s' should be of exported type"; fires for template instatiations. TODO: Investigate.
CFLAGS = $(CFLAGS) -w-pin # W8061 "Initialization is only partially bracketed"; too pedantic; warns for legal well-written code.
CFLAGS = $(CFLAGS) -w-pow # W8062 "Previous options and warnings not restored"; generated in "uxtheme.h" system header.
CFLAGS = $(CFLAGS) -w-sig # W8071 "Conversion may lose significant digits"; generated in "gdiplus.h" system header.
CFLAGS = $(CFLAGS) -w-use # W8080 "'%s' is declared but never used"; generated in "gdiplus.h" system header.

!if $(CODEGUARD) != 0 # Enable compiler's CodeGuard support
   CFLAGS = $(CFLAGS) -vGd -vGt # d = add data, t = check this
!  if $(CODEGUARD) == 2
     CFLAGS = $(CFLAGS) -vGc # c = add calls
!  endif
!endif

CFGFILE = bcc32.cfg # Name of the compiler's configuration file.

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
   LIBOPTS = /P512 /0 #/C
!endif

!if $d(DLL)
   CFLAGS = $(CFLAGS) -D_RTLDLL
   CFLAGS = $(CFLAGS) -WD
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
   STDLIBS = import32 ole2w32.lib $(RTLPFX)32$(RTLMT)i   # Libs for linking the DLL
!  if $(CODEGUARD) != 0
     STDLIBS = cg32 $(STDLIBS)
!  endif
!  if $d(UNICODE)
     STARTUP = c0d32w
!  else
     STARTUP = c0d32
!  endif
!else
   CFLAGS = -W $(CFLAGS)
!endif

!if $d(ILINK)
   LFLAGS = $(LFLAGS) -Gn
!endif

!if $d(VCL)
!  if $(BCCVER) >= 13
     STDLIBS = vcle rtl $(STDLIBS)
!  else
     STDLIBS = vcllink $(STDLIBS)
!  endif
   CFLAGS = -b- -Vx -Ve $(CFLAGS) -DVCLRTL -DINC_VCL
   LFLAGS = $(LFLAGS) -j$(VCLOBJDIR)
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
CPCH = -H -H=$(OBJDIR)\pch.csm
PCHFILE = "owl\pch.h"
CPCH = $(CPCH) -H$(PCHFILE) -Hc
CDEFINES = $(CDEFINES) -D_OWLPCH

#-----------------------------------------------------------------------------
# Tools
#-----------------------------------------------------------------------------

CC = $(CC) +$(CFGFILE)
LINK = $(LINK) $(LFLAGS) -L$(LIBDIR) $(STARTUP)
LIB = $(LIB) $(LIBOPTS)
IMPLIB_I = $(IMPLIB) -i -w # Ignore WEP (-i), no warnings (-w).

#
# Special compile command for objects that goes into the LIB when building a DLL.
#
CC_LIBDLL = $(CC) -W
CC_LIBDLL = $(CC_LIBDLL) $(LIBDLL_DEFS) $(LIBDLL_UNDEFS) 

LINKLIBS =
TEMP_001 = $(LINKLIBS:TARGETSFX=$(TARGETSFX))
USERLIBS = $(TEMP_001:;= )

!if $d(DLL)
   DLL_LINKSFX = ,$(DLLTARGET), nul,$(USERLIBS) $(STDLIBS),
   DLL_LINKSFX = $(DLL_LINKSFX) $(DEFFILE)
!endif

LIBSFX =
LINKOBJPFX =
LINKOBJSFX =
LIBOBJPFX = +"
LIBOBJSFX = "

#-----------------------------------------------------------------------------
#
# Object file lists
#
# - OBJS is the main list.
# - OBJS_LIB is the list of objects that go in the LIB and not the DLL.
# - OBJS_DLL is the list of objects that go in the DLL and not the LIB.
# - DEPOBJS is the list of objects for dependency checking.
# - LIBOBJS is the list of objects for building the LIB.
#
#-----------------------------------------------------------------------------

OBJS = PFXanimctrl.objSFX\
   PFXappdict.objSFX\
   PFXapplicat.objSFX\
   PFXbardescr.objSFX\
   PFXbitmap.objSFX\
   PFXbitmapga.objSFX\
   PFXbitset.objSFX\
   PFXbrush.objSFX\
   PFXbtntextg.objSFX\
   PFXbutton.objSFX\
   PFXbuttonga.objSFX\
   PFXcastable.objSFX\
   PFXcelarray.objSFX\
   PFXcheckbox.objSFX\
   PFXchecklst.objSFX\
   PFXchecks.objSFX\
   PFXchooseco.objSFX\
   PFXchoosefo.objSFX\
   PFXclipboar.objSFX\
   PFXclipview.objSFX\
   PFXcmdline.objSFX\
   PFXcolmnhdr.objSFX\
   PFXcolor.objSFX\
   PFXcombobex.objSFX\
   PFXcombobox.objSFX\
   PFXcommctrl.objSFX\
   PFXcommdial.objSFX\
   PFXcommview.objSFX\
   PFXconfigfl.objSFX\
   PFXcontrol.objSFX\
   PFXcontrolb.objSFX\
   PFXcontrolg.objSFX\
   PFXcoolbar.objSFX\
   PFXcreatedc.objSFX\
   PFXcursor.objSFX\
   PFXdate.objSFX\
   PFXdateio.objSFX\
   PFXdatep.objSFX\
   PFXdatetime.objSFX\
   PFXdc.objSFX\
   PFXdecframe.objSFX\
   PFXdecmdifr.objSFX\
   PFXdiaginit.objSFX\
   PFXdialog.objSFX\
   PFXdib.objSFX\
   PFXdibdc.objSFX\
   PFXdibitmap.objSFX\
   PFXdocking.objSFX\
   PFXdocmanag.objSFX\
   PFXdoctpl.objSFX\
   PFXdocument.objSFX\
   PFXdraglist.objSFX\
   PFXdrawitem.objSFX\
   PFXdumpstack.objSFX\
   PFXedit.objSFX\
   PFXeditfile.objSFX\
   PFXeditsear.objSFX\
   PFXeditview.objSFX\
   PFXeventhan.objSFX\
   PFXexbase.objSFX\
   PFXexcept.objSFX\
   PFXfile.objSFX\
   PFXfiledoc.objSFX\
   PFXfilename.objSFX\
   PFXfiltval.objSFX\
   PFXfindrepl.objSFX\
   PFXflatctrl.objSFX\
   PFXfloatfra.objSFX\
   PFXfont.objSFX\
   PFXframewin.objSFX\
   PFXfscroll.objSFX\
   PFXfunctor.objSFX\
   PFXgadget.objSFX\
   PFXgadgetli.objSFX\
   PFXgadgetwi.objSFX\
   PFXgauge.objSFX\
   PFXgdibase.objSFX\
   PFXgdiobjec.objSFX\
   PFXgeometrp.objSFX\
   PFXgeometrs.objSFX\
   PFXgeometry.objSFX\
   PFXglyphbtn.objSFX\
   PFXgroupbox.objSFX\
   PFXhlpmanag.objSFX\
   PFXhotkey.objSFX\
   PFXhslider.objSFX\
   PFXicon.objSFX\
   PFXimagelst.objSFX\
   PFXinputdia.objSFX\
   PFXipaddres.objSFX\
   PFXlayoutwi.objSFX\
   PFXlistbox.objSFX\
   PFXlistboxview.objSFX\
   PFXlistviewctrl.objSFX\
   PFXlocale.objSFX\
   PFXlocaleco.objSFX\
   PFXlocaleid.objSFX\
   PFXlookval.objSFX\
   PFXmailer.objSFX\
   PFXmci.objSFX\
   PFXmciwav.objSFX\
   PFXmdichild.objSFX\
   PFXmdiclien.objSFX\
   PFXmdiframe.objSFX\
   PFXmemcbox.objSFX\
   PFXmemory.objSFX\
   PFXmemorydc.objSFX\
   PFXmenu.objSFX\
   PFXmenudesc.objSFX\
   PFXmenugadg.objSFX\
   PFXmessageb.objSFX\
   PFXmetafile.objSFX\
   PFXmetafldc.objSFX\
   PFXmodegad.objSFX\
   PFXmodule.objSFX\
   PFXmodversi.objSFX\
   PFXmonthcal.objSFX\
   PFXmsgthred.objSFX\
   PFXnotetab.objSFX\
   PFXobjstrm.objSFX\
   PFXopensave.objSFX\
   PFXowl.objSFX\
   PFXpager.objSFX\
   PFXpaintdc.objSFX\
   PFXpalette.objSFX\
   PFXpanespli.objSFX\
   PFXpen.objSFX\
   PFXpicker.objSFX\
   PFXpicklist.objSFX\
   PFXpictval.objSFX\
   PFXpictwind.objSFX\
   PFXpopupgad.objSFX\
   PFXpreview.objSFX\
   PFXprevwin.objSFX\
   PFXprintdc.objSFX\
   PFXprintdia.objSFX\
   PFXprinter.objSFX\
   PFXprintout.objSFX\
   PFXprofile.objSFX\
   PFXprogressbar.objSFX\
   PFXpropsht.objSFX\
   PFXpsview.objSFX\
   PFXradiobut.objSFX\
   PFXrangeval.objSFX\
   PFXrcntfile.objSFX\
   PFXregexp.objSFX\
   PFXregheap.objSFX\
   PFXregion.objSFX\
   PFXregistry.objSFX\
   PFXreglink.objSFX\
   PFXresource.objSFX\
   PFXrichedit.objSFX\
   PFXrichedpr.objSFX\
   PFXrichedv.objSFX\
   PFXrolldial.objSFX\
   PFXscrollba.objSFX\
   PFXscroller.objSFX\
   PFXserialze.objSFX\
   PFXshellitm.objSFX\
   PFXshelwapi.objSFX\
   PFXslider.objSFX\
   PFXsplashwi.objSFX\
   PFXstatic.objSFX\
   PFXstatusba.objSFX\
   PFXswindow.objSFX\
   PFXsystem.objSFX\
   PFXtabbed.objSFX\
   PFXtabctrl.objSFX\
   PFXtextgadg.objSFX\
   PFXtheme.objSFX\
   PFXthread.objSFX\
   PFXtime.objSFX\
   PFXtimegadg.objSFX\
   PFXtimeio.objSFX\
   PFXtimep.objSFX\
   PFXtinycapt.objSFX\
   PFXtoolbox.objSFX\
   PFXtooltip.objSFX\
   PFXtracewnd.objSFX\
   PFXtransfer.objSFX\
   PFXtransferbuffer.objSFX\
   PFXtrayicon.objSFX\
   PFXtreeviewctrl.objSFX\
   PFXuiborder.objSFX\
   PFXuiface.objSFX\
   PFXuihandle.objSFX\
   PFXuimetric.objSFX\
   PFXuipart.objSFX\
   PFXupdown.objSFX\
   PFXurllink.objSFX\
   PFXustring.objSFX\
   PFXvalidate.objSFX\
   PFXversion.objSFX\
   PFXview.objSFX\
   PFXvslider.objSFX\
   PFXwindow.objSFX\
   PFXwindowdc.objSFX\
   PFXwinsock.objSFX\
   PFXwskaddr.objSFX\
   PFXwskerr.objSFX\
   PFXwskhostm.objSFX\
   PFXwskservm.objSFX\
   PFXwsksock.objSFX\
   PFXwsksockd.objSFX\
   PFXwsksockm.objSFX\
   PFXwsyscls.objSFX

OBJS_LIB =\
   PFXglobal.objSFX\
   PFXlibmain.objSFX\
   PFXowlmain.objSFX\
   PFXinitdll.objSFX\
   PFXwinmain.objSFX\
   PFXmain.objSFX

LIBOBJPFX_DIR = $(LIBOBJPFX)$(OBJDIR)\\
LINKOBJPFX_DIR = $(LINKOBJPFX)$(OBJDIR)\\

!if $d(DLL)
   LIBDEPOBJS_S = $(OBJS_LIB:PFX=)
   LINKDEPOBJS_S = $(OBJS:PFX=) $(OBJS_DLL:PFX=)
   LIBOBJS_S = $(OBJS_LIB:PFX=$(LIBOBJPFX_DIR))
   LINKOBJS_S = $(OBJS:PFX=$(LINKOBJPFX_DIR)) $(OBJS_DLL:PFX=$(LINKOBJPFX_DIR)) $(ASM_OBJS:PFX=$(LINKOBJPFX_DIR))
!else
   LIBDEPOBJS_S = $(OBJS:PFX=) $(OBJS_LIB:PFX=)
   LIBOBJS_S = $(OBJS:PFX=$(LIBOBJPFX_DIR)) $(OBJS_LIB:PFX=$(LIBOBJPFX_DIR)) $(ASM_OBJS:PFX=$(LIBOBJPFX_DIR))
!endif

LIBDEPOBJS = $(LIBDEPOBJS_S:SFX=)
LINKDEPOBJS = $(LINKDEPOBJS_S:SFX=)
LIBOBJS = $(LIBOBJS_S:SFX=$(LIBOBJSFX))
LINKOBJS = $(LINKOBJS_S:SFX=$(LINKOBJSFX))

#--------------------------------------------------------------------
# Rules
#--------------------------------------------------------------------

Make: Init All

.autodepend
.cacheautodepend
.swap

.rc.res:
   $(RCC) -i$(RCINCDIR) -fo$@ $.

.cpp.obj:
   $(CC) {$& }

.asm.obj:
   $(ASM) /mx $& $@

#-----------------------------------------------------------------------------
# Setup - pre-build targets
#-----------------------------------------------------------------------------

INIT_DEPENDENCIES=CreateDirs InitCompiler
!if $(SETUP) == 1
   INIT_DEPENDENCIES=$(INIT_DEPENDENCIES) Setup
!endif

Init: $(INIT_DEPENDENCIES)
   @if not exist "$(ROOTDIR)\include\owl\version.h" (@echo. & @echo Error: "include/owl/version.h" is missing. & @if not "$(SETUP)" == "1" @echo Perform build setup.) & exit 2

CreateDirs:
   @-if not exist $(TARGETDIR)\NUL md $(TARGETDIR)
   @-if not exist $(TARGETLIBDIR)\NUL md $(TARGETLIBDIR)
   @-if not exist $(OBJROOT)\NUL md $(OBJROOT)
   @-if not exist $(OBJDIR)\NUL md $(OBJDIR)

#
# Create compiler response file
#
InitCompiler:
   @copy &&|
-I$(INCDIR)
-c -w -g255 $(CFLAGS) $(CDBG) $(CDEFINES)
$(CPCH)
-n$(OBJDIR)
| $(CFGFILE) > NUL

Setup:
   @echo. 
   @$(ROOTDIR)\source\update_version_header.bat "$(ROOTDIR)" .\version.h "$(ROOTDIR)\include\owl\version.h"
   @echo.

#--------------------------------------------------------------------
# Resource file target
#--------------------------------------------------------------------

# VH, May 2006:
# The resource file (owl.res) needs preprocessing of "version.rc".
# Preprocessing is necessary because of the limitations of the 
# preprocessor inside the Borland Resource Compiler. The stand-alone
# preprocessor hasn't got these problems so it is used to preprocess. 
# Then "owl.rc" will include "version.i" instead (see "owl.rc").

version.i: version.rc $(ROOTDIR)\include\owl\version.h
   @echo Preprocessing version.rc
   $(CPP) -DRC_INVOKED -I$(RCINCDIR) -o$@ version.rc

$(RESFILE): version.i
   $(RCC) -i$(RCINCDIR) -fo$@ $&.rc

#--------------------------------------------------------------------
# DLL target
#--------------------------------------------------------------------

!if $d(DLL)

All: PrintBuildSettings $(DLLTARGET) 

PrintBuildSettings :
   @echo.
   @echo DLLTARGET = $(DLLTARGET)
   @echo LIBTARGET = $(LIBTARGET)
   @echo BCCVER = $(BCCVER)
   @echo OWL_VERSION = $(OWL_VERSION)
   @echo.

#
# Link and implib
#
$(DLLTARGET): $(LINKDEPOBJS) $(LIBDEPOBJS) $(DEFFILE) $(RESFILE)
   @cd $(.PATH.obj)
   $(LINK) @&&|
$(LINKOBJS) $(DLL_LINKSFX),$(RESFILE)
|
!if !$d(ILINK)
   $(TDSTRIP) -s $(DLLTARGET)
   $(IMPLIB) $(LIBTARGET) $(DLLTARGET)
!else
   -del $(LIBTARGET)
   copy $(DLLTARGET:.dll=.lib) $(LIBTARGET)
   @-del $(DLLTARGET:.dll=.lib)
!endif
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
DESCRIPTION 'ObjectWindows DLL'
EXETYPE     WINDOWS
DATA        MULTIPLE
EXPORTS
| $@ > NUL

#
# Compile objects that are exclusive to the import library.
#
$(LIBDEPOBJS): 
   $(CC_LIBDLL) {$&.cpp }

!else

#--------------------------------------------------------------------
# Static LIB target
#--------------------------------------------------------------------

#
# Static target
#
All: PrintBuildSettings $(LIBTARGET)

PrintBuildSettings :
   @echo.
   @echo LIBTARGET = $(LIBTARGET)
   @echo BCCVER = $(BCCVER)
   @echo OWL_VERSION = $(OWL_VERSION)
   @echo.

$(LIBTARGET): $(LIBDEPOBJS)
   @if exist $(LIBTARGET)  del $(LIBTARGET)
   $(LIB) "$(LIBTARGET)" @&&|
$(LIBOBJS) $(LIBSFX)
|

!endif // DLL
