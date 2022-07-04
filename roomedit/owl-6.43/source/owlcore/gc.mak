#-----------------------------------------------------------------------------
# gc.mak - makefile for building OWLNext
# Created by Kenneth Haley (khaley@bigfoot.com) 1999
#
#$Revision: 1.2 $
#$Author: jogybl $
#$Date: 2010-03-24 11:52:05 $
#
# OWL specific defines (parameters):
#
#   COMPVER              compiler version (currently not used)
#   OWLVER               OWLNext version number (optional)
#
#
#   NO_EMULATED_CTRL     Builds library using only system implementation of
#                        common controls
#   OWL5_COMPAT          Builds library with API exposed for backward
#                        compatibility with v5.x of ObjectWindows.
#   NO_OBJ_STREAMING     Builds library without special code to allow
#                        some OWL objects to be persisted in a stream
#
# Defines (parameters):
#   MT                   Build multi-thread version (Win32 only)
#   UNICODE              Build unicode version (Win32 only)
#   DLL                  Build a DLL. (no default)
#   DIAGS                Build the diagnostic version of the target.
#   DEBUG                Build the debug version of the target.
#   ALIGN=<1|2|4|8>      Specifies the alignment
#   BI_DBCS_SUPPORT      builds library with DBCS support
#   BI_MSG_JAPAN         builds library with japaneze support
#
# The name of the targetfile (and the directory for compiler output)
# is constructed using the following format:
#   TARGETSFX = $(VERSION)$(COMP)$(DBG)$(MODEL)$(UC)
#   TARGETNAME = $(BASENAME)$(TARGETSFX)
#
# Where:
#   BASENAME             Base name of the target (bidp, owlp, ocfp, owlx)
#   VERSION              Version number (default=630)
#   MODEL=<f|t>		 Model/target platform
#                        f=32-bit static/dynamic library
#			 t=32-bit static/dynamic multhi threaded library
#   UC=<u|>              u=Unicode                      (Win32 only)
#                        <nothing>=ANSI
#   DBG=<d|>             d=Debug/diagnostic build
#                        <nothing>=Release build
#   COMP=<,v,g>          Borland=none, MS VC++ = v, GNU = g
#-----------------------------------------------------------------------------

ifdef OWLNAME
  BASENAME = $(OWLNAME)
endif

ifndef BASENAME
  BASENAME = owl
endif

ifndef OWLVER
  VERSION  = 630
else
  VERSION  = OWLVER
endif

ifndef COMPNAME
#currentyl unused as 2010-02
#  COMPNAME = g
endif


#-----------------------------------------------------------------------------
# Tools
#-----------------------------------------------------------------------------

# This can prodece 'CreateProcess fail' in Windows, don't bother.
UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
  PLATFORM=Linux
  GNUROOT=
  SDKDIR=/usr/include/wine/windows
  LINKLIBDIR = -L /usr/lib/wine 

else ifeq ($(UNAME), Solaris)
  PLATFORM=Solaris
  GNUROOT=/usr/bin/
  SDKDIR=/usr/include/wine/windows
  LINKLIBDIR = -L /usr/lib/wine 

else ifeq ($(UNAME), Darwin)
  PLATFORM=Osx
  GNUROOT=/usr/bin/
  SDKDIR=/usr/include/wine/windows
  LINKLIBDIR = -L /usr/lib/wine 

else ifeq ($(UNAME), HP-UX)
  PLATFORM=HP-UX
  GNUROOT=/usr/bin/
  SDKDIR=/usr/include/wine/windows
  LINKLIBDIR = -L /usr/lib/wine 

else
  PLATFORM=Windows
  GNUROOT=c:\mingw\bin\
  SDKDIR=c:\mingw\include
  LINKLIBDIR = -L c:\mingw\lib

#reserved for possible future use
#  paths := $(shell echo %path%)
#  paths := $(subst ;, ,$(paths))
#  binpath := $(strip $(foreach path,$(paths),$(if $(wildcard $(path)/gcc.exe),$(path))))
#  binpath := $(subst \,/,$(binpath))
#  binpath := $(patsubst %/bin,%,$(binpath))
#  libpath := $(binpath)/lib

endif


ifeq ($(PLATFORM), Windows)
  # We use simple slash in Windows
  SLASH=\\

  # mingw32 settings
  CFLAGS+= -mno-cygwin # -mno-cygwin=applications dont require Cygwin.DLL to run (in Windows)
else
  SLASH=/
  CDEFINES += -DWINELIB
endif


RC=rc.exe  #currently we dont use RC.EXE
ifeq ($(PLATFORM), Windows)
  WINDRES=$(GNUROOT)windres.exe #In Windows platform we use WINDRES
else
  WINDRES=$(GNUROOT)wrc #In other platforms we use Wine Resource Compiler
endif
GCC=$(GNUROOT)gcc
LD=$(GNUROOT)ld
AR=$(GNUROOT)ar
ASM=$(GNUROOT)as
CPP=$(GNUROOT)c++
STRIP=$(GNUROOT)strip


#------------------------------------------------------------------------------


#Greg Chicare report problems with vtable-thunks, remove it
#CFLAGS = -fvtable-thunks

# current additional options
#NO_OBJ_STREAMING = 1  # ??? can we enable this ?????
NO_WIN32S_SUPPORT = 1

# Compile as DLL/Shared Object/DyLib
#DLL               = 1

#DEBUG            = 1

CFLAGS+= -D_WIN32_IE=0x0501 -DBI_SUPPRESS_OLE 
ifdef DEBUG
  CFLAGS+= -D_DEBUG  -D__DEBUG=2  -D_TRACE=1  -D_WARN=1
endif

#location of win32 headers
RCW32INC = $(SDKDIR)

CFLAGS+= -Wall   #Warnigs All

#CDEFINES =

DEFFILE  = owlp.def
RESFILE  = owl.res

ifeq ($(PLATFORM), Windows)
  WINLIBS = -lgdi32 -lgdiplus -lmsimg32 -luser32 -lkernel32 -ladvapi32 -lshell32
  RTLLIBS = -lmingw32 -lgcc -lmoldname -lmingwex -lmsvcrt -lstdc++
else
  WINLIBS = -l:gdi32.dll.so -l:gdiplus.dll.so -l:msimg32.dll.so -l:user32.dll.so -l:kernel32.dll.so -l:advapi32.dll.so -l:shell32.dll.so
  RTLLIBS =  -lc -lg #-lstdc++
endif
LINKLIBS = $(RTLLIBS) $(WINLIBS)

ifdef DLL
  DLLCDEFS       = -D_BUILDOWLDLL
  LIBDLL_DEFS    = -D_OWLDLL
  LIBDLL_UNDEFS  = -U_BUILDOWLDLL
  CFLAGS+= -fPIC
endif

ifndef CXX
  CXX = c++
endif


#-----------------------------------------------------------------------------
# Various OWL settings
#-----------------------------------------------------------------------------

#
# Emulate Common Ctrls or not
#
ifdef NO_EMULATED_CTRL
  CDEFINES += -DOWL_NATIVECTRL_ALWAYS
endif

#
# Build OWL5 compatible version?
#
ifdef OWL5_COMPAT
 CDEFINES += -DOWL5_COMPAT
endif

#
# Build without persistent/stream support?
#
ifdef NO_OBJ_STREAMING
  CDEFINES += -DBI_NO_OBJ_STREAMING
endif

#-----------------------------------------------------------------------------
# The make target
#-----------------------------------------------------------------------------
Make: Vars Setup All

Vars:
	@echo OBJROOT $(OBJROOT)
	@echo OBJDIR $(OBJDIR)
	@echo VERSION $(VERSION)
	@echo MODEL $(MODEL)
	@echo KEY $(KEY)
	@echo COMP $(COMP)
	@echo FULLTARGET $(FULLTARGET)
	@echo LIBTARGET $(LIBTARGET)
	@echo LIBTARGET_CONSOL $(LIBTARGET_CONSOL)
	@echo RESFILE $(RESFILE)
	@echo DEFFILE $(DEFFILE)
	@echo ALIGN $(ALIGN)
	@echo CFLAGS $(CFLAGS)
	@echo CDEFINES $(CDEFINES)

	@echo PLATFORM $(PLATFORM)


#-----------------------------------------------------------------------------
# Check parameters
#-----------------------------------------------------------------------------

ifndef WIN64
  WIN32 = 1
endif

#--------------------------------------------------------------------
# Options which are memory model independent
#--------------------------------------------------------------------

ifdef UNICODE
  CDEFINES += -DUNICODE
endif

ifdef DEBUG
  __DEBUG=2
endif
ifdef DIAGS         # default diags use precondition, check, trace & warn
  __DEBUG=2
  __TRACE=1
  __WARN=1
endif

#
# Build up compiler diag switches based on settings
#
ifdef __DEBUG
  CDEFINES += -D__DEBUG=$(__DEBUG)
endif

ifdef __TRACE
  CDEFINES += -D__TRACE
endif

ifdef __WARN
  CDEFINES += -D__WARN
endif

#--------------------------------------------------------------------
# Options which are compiler independent
#--------------------------------------------------------------------

ifdef DLL
  ifdef DLLCDEFS
    CDEFINES += $(DLLCDEFS)
  endif
endif

#--------------------------------------------------------------------
# Options for DBCS
#--------------------------------------------------------------------
ifdef BI_DBCS_SUPPORT
  CDEFINES += -DBI_DBCS_SUPPORT
endif

ifdef BI_MSG_JAPAN
  CDEFINES += -DBI_MSG_LANGUAGE=0x0411
endif


#
# Figure out default data alignment based on target
#
ifndef ALIGN
ifdef WIN32
  ALIGN = 8
endif
ifdef WIN64
  ALIGN = 8
endif
ifdef LINUX
  ALIGN = 8
endif
ifdef OSX
  ALIGN = 8
endif
endif

#-----------------------------------------------------------------------------
# Build target name
#-----------------------------------------------------------------------------
ifdef MT
  MODEL=t
else
  MODEL=s
endif

ifdef UNICODE
  UC=u
endif

ifndef COMP
  COMP=g
endif


KEY = w$(MODEL)

ifdef DLL
  # 32bit flat or multithread DLL import library
  KEY:=$(KEY)i
endif

#ifdef USERDLL
# Static or import library for use by dlls
#  KEY:=$(KEY)u
#endif

ifdef DEBUG
  DBG=d
else
ifdef DIAGS
  DBG=d
endif
endif

TARGETSFX = $(VERSION)$(COMP)$(DBG)$(MODEL)$(UC)
TARGETNAME = $(BASENAME)$(TARGETSFX)

#-----------------------------------------------------------------------------
# Directory names
#-----------------------------------------------------------------------------

#ifndef OWLBUILDROOT
	OWLBUILDROOT = ../..
#endif

ifndef TARGETDIR
    TARGETDIR = $(OWLBUILDROOT)$(SLASH)bin
endif

ifndef SRCROOT
  ifeq ($(PLATFORM), Windows)
    SRCROOT   = $(OWLBUILDROOT)\source
  else
    SRCROOT   = $(OWLBUILDROOT)/source
  endif
endif

ifndef INCDIR
  ifeq ($(PLATFORM), Windows)
	INCDIR    = $(OWLBUILDROOT)\include
  else
	INCDIR    = $(OWLBUILDROOT)/include
  endif
endif

ifndef RCINCDIR
  ifeq ($(PLATFORM), Windows)
     RCINCDIR  = $(OWLBUILDROOT)\include
  else
     RCINCDIR  = $(OWLBUILDROOT)/include
  endif
endif

ifndef LIBDIR
  ifeq ($(PLATFORM), Windows)
	LIBDIR    = $(OWLBUILDROOT)\lib
  else
	LIBDIR    = $(OWLBUILDROOT)/lib
  endif
endif

ifndef OBJROOT
  ifeq ($(PLATFORM), Windows)
	OBJROOT   = $(OWLBUILDROOT)\obj
  else
	OBJROOT   = $(OWLBUILDROOT)/obj
  endif
endif

ifndef OBJDIR
  ifeq ($(PLATFORM), Windows)
	OBJDIR    = $(OBJROOT)\$(BASENAME)$(COMP)$(DBG)$(KEY)$(UC)
  else
	OBJDIR    = $(OBJROOT)/$(BASENAME)$(COMP)$(DBG)$(KEY)$(UC)
  endif
endif

ifndef FULLTARGET
  ifeq ($(PLATFORM), Windows)
	FULLTARGET 		= $(TARGETDIR)\$(TARGETNAME)
  else
	FULLTARGET 		= $(TARGETDIR)/$(TARGETNAME)
  endif
endif


#-----------------------------------------------------------------------------
# Create full pathnames for DEFFILE and RESFILE
#-----------------------------------------------------------------------------
ifdef DEFFILE
	DEFFILE := $(OBJDIR)$(SLASH)$(DEFFILE)
endif

ifdef RESFILE
	RESFILE := $(OBJDIR)$(SLASH)$(RESFILE)
endif

#-----------------------------------------------------------------------------
# Compiler specific data & rules
#-----------------------------------------------------------------------------

#INCDIR=
#LIBDIR=

ifdef DEBUG
  CFLAGS += -O0 -ggdb
else
  CFLAGS += -O3 -s
endif

#CFLAGS += -mwindows
CDEFINES += -D_OWLPCH

MCC=$(GCC) -c $(CFLAGS) $(CDEFINES) -I$(INCDIR) -I$(SDKDIR)

ifndef DEBUG
$(OBJDIR)$(SLASH)%.obj : %.cpp
	$(MCC) $< -o $@
else
MCC2=$(MCC:-O0=-O1)

$(OBJDIR)$(SLASH)%.obj : %.cpp
	-$(MCC) $< -o $@
	if ! test -e $@ ;\
	then \
	$(MCC2) $< -o $@;\
	fi
endif

#-----------------------------------------------------------------------------
# Create output directory
#-----------------------------------------------------------------------------
Setup: CreateDirs

ifeq ($(PLATFORM), Windows)
CreateDirs:
	@-if not exist $(TARGETDIR)\NUL mkdir $(TARGETDIR)
	@-if not exist $(OBJROOT)\NUL mkdir $(OBJROOT)
	@-if not exist $(OBJDIR)\NUL mkdir $(OBJDIR)
#  @-if not exist $(TARGETLIBDIR)\NUL mkdir $(TARGETLIBDIR)
	@-if not exist $(LIBDIR)\NUL mkdir $(LIBDIR)
else
CreateDirs:
	if ! test -e $(TARGETDIR);then mkdir $(TARGETDIR);fi
	if ! test -e $(OBJROOT);then mkdir $(OBJROOT);fi
	if ! test -e $(OBJDIR);then mkdir $(OBJDIR);fi
	if ! test -e $(LIBDIR);then mkdir $(LIBDIR);fi

endif


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
OBJS =\
 appdict.obj\
 bitmap.obj\
 bitmapga.obj\
 bitset.obj\
 btntextg.obj\
 buttonga.obj\
 castable.obj\
 celarray.obj\
 checks.obj\
 checklst.obj\
 chooseco.obj\
 choosefo.obj\
 clipboar.obj\
 clipview.obj\
 cmdline.obj\
 colmnhdr.obj\
 color.obj\
 commctrl.obj\
 commdial.obj\
 configfl.obj\
 controlb.obj\
 controlg.obj\
 createdc.obj\
 cursor.obj\
 date.obj\
 dateio.obj\
 datep.obj\
 dc.obj\
 diaginit.obj\
 dibdc.obj\
 dispatch.obj\
 eventhan.obj\
 exbase.obj\
 except.obj\
 flatctrl.obj\
 font.obj\
 functor.obj\
 gadget.obj\
 gadgetli.obj\
 gadgetwi.obj\
 gauge.obj\
 gdibase.obj\
 gdiobjec.obj\
 geometrp.obj\
 geometrs.obj\
 geometry.obj\
 hotkey.obj\
 icon.obj\
 imagelst.obj\
 layoutwi.obj\
 locale.obj\
 localeco.obj\
 localeid.obj\
 mailer.obj\
 mci.obj\
 mciwav.obj\
 memory.obj\
 memorydc.obj\
 menu.obj\
 menugadg.obj\
 messageb.obj\
 metafile.obj\
 metafldc.obj\
 modegad.obj\
 modversi.obj\
 msgthred.obj\
 profile.obj\
 notetab.obj\
 objstrm.obj\
 owl.obj\
 paintdc.obj\
 palette.obj\
 panespli.obj\
 pen.obj\
 picklist.obj\
 pictwind.obj\
 popupgad.obj\
 preview.obj\
 prevwin.obj\
 printdc.obj\
 progressbar.obj\
 propsht.obj\
 psview.obj\
 rcntfile.obj\
 resource.obj\
 regexp.obj\
 regheap.obj\
 region.obj\
 registry.obj\
 reglink.obj\
 rolldial.obj\
 serialze.obj\
 splashwi.obj\
 statusba.obj\
 swindow.obj\
 system.obj\
 tabctrl.obj\
 tabbed.obj\
 textgadg.obj\
 theme.obj\
 time.obj\
 timegadg.obj\
 timeio.obj\
 timep.obj\
 toolbox.obj\
 tooltip.obj\
 tracewnd.obj\
 transfer.obj\
 transferbuffer.obj\
 uiborder.obj\
 uiface.obj\
 uihandle.obj\
 uimetric.obj\
 uipart.obj\
 updown.obj\
 urllink.obj\
 ustring.obj\
 version.obj\
 window.obj\
 windowdc.obj\
 winsock.obj\
 wskaddr.obj\
 wskerr.obj\
 wskhostm.obj\
 wsksockm.obj\
 wskservm.obj\
 wsksock.obj\
 wsksockd.obj\
 wsyscls.obj

#
# Add in these objs when not sectioning
#
#else
OBJS += \
 module.obj\
 applicat.obj\
 brush.obj\
 button.obj\
 checkbox.obj\
 combobox.obj\
 control.obj\
 decframe.obj\
 decmdifr.obj\
 dialog.obj\
 dib.obj\
 dibitmap.obj\
 docking.obj\
 docmanag.obj\
 doctpl.obj\
 document.obj\
 edit.obj\
 editfile.obj\
 editsear.obj\
 editview.obj\
 file.obj\
 filedoc.obj\
 filename.obj\
 filtval.obj\
 findrepl.obj\
 floatfra.obj\
 framewin.obj\
 glyphbtn.obj\
 groupbox.obj\
 hslider.obj\
 inputdia.obj\
 listbox.obj\
 listboxview.obj\
 lookval.obj\
 mdichild.obj\
 mdiclien.obj\
 mdiframe.obj\
 memcbox.obj\
 menudesc.obj\
 opensave.obj\
 picker.obj\
 pictval.obj\
 printdia.obj\
 printer.obj\
 printout.obj\
 radiobut.obj\
 rangeval.obj\
 scrollba.obj\
 scroller.obj\
 slider.obj\
 static.obj\
 tinycapt.obj\
 validate.obj\
 view.obj\
 vslider.obj
#endif

#
# Common, unsectioned objs go into static libs and DLLs
#
# PFXcoolbar.objSFX\

OBJS +=\
 combobex.obj\
 commview.obj\
 bardescr.obj\
 datetime.obj\
 draglist.obj\
 drawitem.obj\
 hlpmanag.obj\
 ipaddres.obj\
 listviewctrl.obj\
 monthcal.obj\
 pager.obj\
 richedit.obj\
 richedv.obj\
 richedpr.obj\
 shellitm.obj\
 shelwapi.obj\
 thread.obj\
 trayicon.obj\
 treeviewctrl.obj

#
# Add in these objs when not sectioning
#
#else  # OWLSECTION

OBJS += \
 animctrl.obj\
 fscroll.obj

#endif # OWLSECTION

OBJS += \
 unix.obj\
 unixxcpt.obj

#
# Objs that go into static libs and import libs, but not in the owl dll itself
# These also get built differently when in the import library. Note that
# classes in the modules are declared _USERCLASS instead of _OWLCLASS
#
OBJS_LIB +=\
 global.obj

#
# Lib for use by DLLs
#
OBJS_LIB +=\
 libmain.obj\
 owlmain.obj\
 initdll.obj

#
# Lib for use in EXEs
#
OBJS_LIB += winmain.obj
OBJS_LIB += main.obj

#
# Objs that go into the Owl DLL only
#
#ifdef DLL
#OBJS_DLL =\
# PFXtmplinst.objSFX
#endif

#
#add object directory prefix
#
OBJS:=$(addprefix $(OBJDIR)$(SLASH),$(OBJS))
OBJS_LIB:=$(addprefix $(OBJDIR)$(SLASH),$(OBJS_LIB))

ifdef DLL

ifeq ($(PLATFORM), Windows)
 DLLTARGET = $(FULLTARGET).dll
elif ifeq ($(PLATFORM), Darwin)
 DLLTARGET = $(FULLTARGET).dylib
else  #all other platforms
 DLLTARGET = $(FULLTARGET).dll.so
endif

LIBTARGET = $(OWLBUILDROOT)$(SLASH)lib$(SLASH)lib$(BASENAME)$(DBG)w$(MODEL)$(UC)i.a
LIBTARGET_CONSOL = $(OWLBUILDROOT)$(SLASH)lib$(SLASH)lib$(BASENAME)$(DBG)w$(MODEL)$(UC)ic.a
CDEFINES += -D_DLL

else
LIBTARGET = $(OWLBUILDROOT)$(SLASH)lib$(SLASH)lib$(BASENAME)$(DBG)w$(MODEL)$(UC).a
LIBTARGET_CONSOL = $(OWLBUILDROOT)$(SLASH)lib$(SLASH)lib$(BASENAME)$(DBG)w$(MODEL)$(UC)c.a
endif

RCDEF =/d__GNUC__ /dWIN32
ifndef DLL
RCDEF += /dMAKELIB
endif
RCTEMP= $(RESFILE:.res=.rso)

ifeq ($(PLATFORM), Windows)
RCFLAGS= -O coff
else
RCFLAGS= -D WINELIB
endif

$(RESFILE): owl.rc
#	$(RC) /i$(RCW32INC) /i$(INCDIR) /i$(INCDIR)/owl $(RCDEF) /fo$(RCTEMP) $<
#	$(WINDRES) -I $(RCW32INC) -I $(INCDIR) $(RCFLAGS) -i $(RCTEMP) -o $@
#	rm $(RCTEMP)
	$(WINDRES) -I $(RCW32INC) -I $(INCDIR) $(RCFLAGS) -i $< -o $@

#for what is used LIBTARGET_CONSOL and how to build it?
ifdef DLL
#All: $(DLLTARGET) $(LIBTARGET) $(LIBTARGET_CONSOL)
All: $(DLLTARGET) $(LIBTARGET) 
else
#All: $(DLLTARGET) $(LIBTARGET) $(LIBTARGET_CONSOL)
All: $(DLLTARGET) $(LIBTARGET) 
endif
#--export-all
#DLLWRAP_FLAGS = --output-def $(DEFFILE) \
#	--implib $(LIBTARGET) \
#	--driver-name $(CXX) \
#	--target mingw32
MCC4=$(MCC:-c=)

ifdef DLL

ifeq ($(PLATFORM), Windows)
  LDFLAGS= -shared  --out-implib $(LIBTARGET) --subsystem windows 
  MCC4FLAGS= -shared -Wl,--out-implib,$(LIBTARGET) -Wl,--subsystem,windows
else
  LDFLAGS= -shared #--verbose
  MCC4FLAGS= -shared 
endif


$(DLLTARGET): $(OBJS) $(OBJS_LIB) $(OBJS_DLL) $(RESFILE) 
	$(LD) -o $(DLLTARGET) $(LDFLAGS) $(LINKLIBDIR) $(LINKLIBS) $(OBJS) $(OBJS_DLL) #$(RESFILE) 
#	$(MCC4) $(MCC4FLAGS) -o $(DLLTARGET) $(OBJS) $(OBJS_DLL) $(RESFILE) 
	$(STRIP) $(DLLTARGET)


MCC5=$(MCC:-D_DLL=)
MCC5:=$(MCC5:-D_BUILDOWLDLL=)

$(OBJDIR)$(SLASH)global.obj : global.cpp
	$(MCC5) $(LIBDLL_DEFS) $(LIBDLL_UNDEFS) -o $(OBJDIR)$(SLASH)global.obj global.cpp

$(OBJDIR)$(SLASH)libmain.obj : libmain.cpp
	$(MCC5) $(LIBDLL_DEFS) $(LIBDLL_UNDEFS) -o $(OBJDIR)$(SLASH)libmain.obj libmain.cpp

$(OBJDIR)$(SLASH)owlmain.obj : owlmain.cpp
	$(MCC5) $(LIBDLL_DEFS) $(LIBDLL_UNDEFS) -o $(OBJDIR)$(SLASH)owlmain.obj owlmain.cpp

$(OBJDIR)$(SLASH)initdll.obj : initdll.cpp
	$(MCC5) $(LIBDLL_DEFS) $(LIBDLL_UNDEFS) -D_DLL -o $(OBJDIR)$(SLASH)initdll.obj initdll.cpp

else #making lib

#used for -frepo  (Note: -frepo: Enable automatic template instantiation at link time. This option also implies -fno-implicit-templates.)
#RPO=$(MCC:-c=)

#the ld command forces the generation of template instances for -frepo
#it will always fail
$(LIBTARGET): $(OBJS) $(OBJS_LIB) $(RESFILE) 
#	@-$(RPO) -o /tmp/foo.tmp $(OBJS) $(OBJS_LIB)
#	$(AR) rs $(LIBTARGET) $?
	$(AR) rs $(LIBTARGET) $(OBJS) $(OBJS_LIB) $(RESFILE)


endif

MCC3=$(MCC:-O2=-O0) -fno-omit-frame-pointer
MCC3:=$(MCC3:-O1=-O0)

$(OBJDIR)/unixxcpt.obj : unixxcpt.cpp
	$(MCC3) -o $(OBJDIR)/unixxcpt.obj unixxcpt.cpp

##################################################
