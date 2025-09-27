#-----------------------------------------------------------------------------
# ObjectComponents - 
# Original Copyright (c) CodeGear International 1991, 1996. All Rights Reserved
# Portions Copyright (c) OWLNext project 2009. All Rights Reserved
#
# MAKEFILE for ObjectComponents BOcOle Dlls
#
#  BCROOT  = Borland compiler root directory, parent of BIN, LIB, INCLUDE
#  APP     = name of application, used for .DLL
#  DEFNAME = name of .def, .res file
#  OBJS    = list of object files for link, each with .obj extension
#  LIBS    = list of library files to be searched in addition to standard ones
#  OBJDIR  = name of build directory, child of source directory
#  DLL     = defined if making a DLL, not defined for EXE
#------------------------------------------------------------------------

#MAKEINC = ..\make.inc

.SUFFIXES: .RC .OBJ .DEF .CPP .C .ASM

!if !$d(TARGET_EXT)
!if $d(DLL)
TARGET_EXT = DLL
WSWITCH    = DE
!else
TARGET_EXT = EXE
WSWITCH    = S
!endif
!endif

!ifdef DIAGS
DEBUG = 1
TRACE = 1
WARN  = 1
!endif

!if !$d(BCROOT)
  BCROOT = $(MAKEDIR)\..   # default bcroot to parent of dir where make lives
!endif

!ifndef WIN64
OLEINC    = $(SRCDIR)\boleui32
LIBPATH   = $(RELDIR);$(SRCDIR)\boleui32;$(BCROOT)\LIB
INCLUDE   = $(BCROOT)\include\win32;$(BCROOT)\include
CONFIG    = bcc32.cfg
CPP       = @$(BCROOT)\BIN\BCC32
LINK      = $(BCROOT)\BIN\TLINK32
BRC       = $(BCROOT)\BIN\BRCC32
RLINK     = $(BCROOT)\BIN\BRC32
#64 bit version
!else
OLEINC    = $(SRCDIR)\boleui64
INCLUDE   = $(BCROOT)\include
LIBPATH   = $(RELDIR);$(SRCDIR)\boleui64;$(BCROOT)\LIB
CONFIG    = bcc64.cfg
CPP       = @$(BCROOT)\BIN\BCC32
LINK      = $(BCROOT)\BIN\TLINK32
BRC       = $(BCROOT)\BIN\BRCC32.EXE
RLINK     = $(BCROOT)\BIN\BRC32
!endif

SRCDIR    = ..
RELDIR    = $(SRCDIR)\BIN

LIB       = $(BCROOT)\BIN\TLIB
IMPLIB    = $(BCROOT)\BIN\IMPLIB
ASM       = $(BCROOT)\BIN\TASM
RC        = $(BRC)

MAPSYM    = $(BCROOT)\BIN\TMAPSYM
CPPFLAGS  = -DINC_OLE2 # -DLOOSE_HRESULT

!if $(DEBUG)
  CPPFLAGS  = $(CPPFLAGS) -v -vi -D_DEBUG
!else
  !ifndef WIN64                              # NDEBUG turns assert() off.
    CPPFLAGS  = $(CPPFLAGS) -DNDEBUG -k-     # -k- optimized stack frames.
  !else                                      # same options, will see later
    CPPFLAGS  = $(CPPFLAGS) -DNDEBUG -k-     # -k- optimized stack frames.
  !endif
!endif

!ifdef TRACE
CPPFLAGS  = $(CPPFLAGS) -D__TRACE
!endif

!ifdef WARN
CPPFLAGS  = $(CPPFLAGS) -D__WARN
!endif

!ifndef ALIGN
  !ifndef WIN64
    ALIGN  = -a4
  !else
    ALIGN  = -a8
  !endif
!endif

!ifndef WIN64
  CPPFLAGS  = $(CPPFLAGS) -DWIN32 $(ALIGN)
!else
  CPPFLAGS  = $(CPPFLAGS) -DWIN64 $(ALIGN)
!endif

#The ANSI flag should be obsolete, now code only reference to UNICODE or !UNICODE
!ifndef UNICODE
  CPPFLAGS =  $(CPPFLAGS) -DANSI
!else
  CPPFLAGS =  $(CPPFLAGS) -DUNICODE
!endif
!endif

  CPPFLAGS =  $(CPPFLAGS) -DMS_OLEUI_DEF

!ifndef WIN64
LINKFLAGS   = -c -C -Tw -n -s c0w$(MODEL)+
LINKFLAGSD  =       -Tpd -n -v -s c0d32+
!else
LINKFLAGS   = -c -C -Tw  -A=16 -n -s c0w64$(MODEL)+
LINKFLAGSD  = -c -C -Twd -A=16 -n -v -s c0d64+
!endif

!ifndef WIN64
LIBS      = $(LIBS) cw$(MODEL) import32
!else
LIBS      = $(LIBS) cw$(MODEL) import64
!endif

ALL: cdobj mdbin $(CONFIG) $(APP).$(TARGET_EXT) $(OTHER_TARGETS) cdsrc

cdobj:
  @if not exist $(OBJDIR)\NUL md $(OBJDIR)
  @cd $(OBJDIR)

mdbin:
  @if not exist $(RELDIR)\NUL md $(RELDIR)

cdsrc:
  @cd $(SRCDIR)

bcc64.cfg: $(MAKEINC)
  @echo  Building bcc64.cfg
  @copy &&|
  $(CPPFLAGS)
  -I$(SRCDIR);$(SRCDIR)\..\ole2ui;$(OLEINC);$(BCROOT)\include\win64;$(BCROOT)\include
#  -I..\..\public;..\..\bole
  -w-par -w-aus -w-ef
  -c -3 -i60 -d -W$(WSWITCH) -O -Obels -Z -RT- -x-
|       bcc64.cfg
  @echo  Building tasm.cfg
  @copy &&|
  /mx/Zi
|       tasm.cfg

bcc32.cfg: $(MAKEINC)
  @echo  Building bcc32.cfg
  @copy &&|
  $(CPPFLAGS)
  -I$(SRCDIR);$(SRCDIR)\..\ole2ui;$(OLEINC);$(BCROOT)\include\win32;$(BCROOT)\include
#  -I..\..\public;..\..\bole
  -w-par -w-aus -w-ef
  -c -3 -i60 -d -W$(WSWITCH) -O -Obels -Z -RT- -x-
|       bcc32.cfg

$(APP).dll: $(OBJS) $(SRCDIR)\$(DEFNAME).def $(SRCDIR)\$(DEFNAME).res $(RESOURCES)
  $(LINK) $(LINKFLAGSD) @&&|
  -L$(LIBPATH) +
  $(OBJS)
  $@
  $(APP).map
  $(LIBS)
  $(SRCDIR)\$(DEFNAME).def
!ifndef WIN64
  $(SRCDIR)\$(DEFNAME).res $(RESOURCES)
!endif
|
!ifdef WIN64
  $(RLINK) $(SRCDIR)\$(DEFNAME).res $(RESOURCES) $@
!endif
#  copy $(APP).dll $(RELDIR)
  $(IMPLIB) $(RELDIR)\$(FILENAME).lib $(APP).dll

$(APP).exe : $(OBJS) $(DEFNAME).def $(DEFNAME).res
  $(LINK) $(LINKFLAGS) @<<$(APP).lnk
  -L$(LIBPATH) +
  $(OBJS:.obj =+)
  $@
  $(APP).map
  $(LIBS)
  $(DEFNAME).def
<<KEEP
  $(RLINK) $(DEFNAME).res $@
  copy $(APP).exe $(RELDIR)

## .DEF File Generation ##

$(DEFNAME).def : $(MAKEINC)
  copy &&|
  NAME       $(APP)
  DESCRIPTION 'OLE 2.0 $(APP) Sample'
  EXETYPE     WINDOWS
  CODE        PRELOAD MOVEABLE
  DATA        PRELOAD MOVEABLE MULTIPLE
  HEAPSIZE    4096
  STACKSIZE   8192
|       $(DEFNAME).def

## IMPLICIT RULES ##

{$(SRCDIR)}.c{}.obj:
  $(CPP) -c $<

{$(SRCDIR)}.cpp{}.obj:
  $(CPP) -c $<

{$(SRCDIR)}.asm{}.obj:
  $(ASM) /l/ml/n/q/t $<,$@

{$(SRCDIR)}.def{}.lib:
  $(IMPLIB) $@ $<

{$(SRCDIR)}.rc{}.res:
  $(RC) $< -r -i$(SRCDIR);$(BCROOT)\include -fo$@
