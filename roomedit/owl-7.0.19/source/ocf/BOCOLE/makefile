#-----------------------------------------------------------------------------
# ObjectComponents
# Original Copyright (c) CodeGear International 1991, 2009. All Rights Reserved
# Portions Copyright (c) OWLNext project 2009. All Rights Reserved
#
#  $Revision: 1.2 $
#
# MAKEFILE for BOCOLE Helper DLL
#
# Defines:
#   MODEL=<l|f>       one of: large, flat
#   WIN64             enables win64 DLL [flat]
#   WIN32             enables win32 DLL [flat]
#   WIN16             deprecated
#   MT                links to the multithreaded version of the RTL
#   VCL               builds BOCOLE using the VCL-compatible RTL (cp32mt) (implies -DMT; requires WIN32)
#   DEBUG             enables debug info for TDW & TD32
#   BOCONAME=Xxx      lib & DLL name base, Defaults to BOCOF (vs BOCOF64, BOCOFW, BOCOF64W)
#   BOCORES=Xxxx      name of .RES file [defaults to BOCONAME.res]
#   CODEGUARD         builds with CodeGuard:  1=basic data, 2=full checking
#   RTLDLL            builds BOCOLE DLL which uses DLL version of RTL
#   ALIGN=<1|2|4|8>   specifies the alignment
#   TLINK             uses TLINK32 instead of ILINK32 to link  (original default was TLINK)
#
# Define these for alternate OCF build directories:
#   BOCOLESRCDIR    defaults to: .
#   BOCOLEINCDIR    defaults to: .
#   BOCOLELIBDIR    defaults to: .
#   BOCOLEBINDIR    defaults to: .\bin
#
#   History:
#   V3.00: SL, removed Win16, added Win64
#   Tested with BC5, ie:
#                       make -DBCROOT=C:\BC5
#                       make -DBCROOT=C:\BC5 -DUNICODE
#
#-----------------------------------------------------------------------------

.autodepend
.cacheautodepend
.swap


#-----------------------------------------------------------------------------
# Fill in defaults
#-----------------------------------------------------------------------------
!if !$d(WIN64)
  WIN32 = 1
!endif

DLL = 1

#
# For multithreaded target, set the value of MT
#
!if $(MODEL) == t || $d(MT) || $d(VCL)
  MT = mt       # model t implies MT. Use MT as 'mt' to build library names
!endif

!if $d(MT) || $(VCL)
 MODEL = t
!endif

#
# Check for incompatible choices and warn user
#
!if $d(VCL)
! if !$d(WIN32) || $(MODEL) != t
!   error VCL-compatible library must be multithreaded 32-bit
! endif
!endif

!if $d(WIN16)
! error 16-bit targets not supported.
!endif


#--------------------------------------------------------------------
# Dir & name components
#--------------------------------------------------------------------
!if !$d(BOCONAME)
! if $d(WIN64)
!   if $d(UNICODE)
      BOCONAME = BOCOLE64W
!   else
      BOCONAME = BOCOLE64
!   endif
! else
!   if $d(UNICODE)
      BOCONAME = BOCOFW
!   else
      BOCONAME = BOCOF
!   endif
! endif
!endif
!if !$d(BOCORES)
  BOCORES = $(BOCONAME).res
!endif


#--------------------------------------------------------------------
# Debug & diagnostic flags
#--------------------------------------------------------------------
!if $d(DEBUG)
  CDBG = -v -vi -D_DEBUG
  LDBG = /v
!endif
!if $d(DIAGS)         # default diags use precondition, check, trace & warn
  __DEBUG=2
  __TRACE=1
  __WARN=1
!endif


#--------------------------------------------------------------------
# Dir & search paths, & precompiled headers
#--------------------------------------------------------------------

#
# BC tools BCBINDIR can be based on MAKEDIR if no BCROOT is defined
#
!if $d(BCROOT)
  BCBINDIR = $(BCROOT)\bin
!else
  BCBINDIR = $(MAKEDIR)
!endif


#
# Default bcroot to whatever is in bcroot.inc
#
!if !$d(BCROOT)
#!  include $(MAKEDIR)\bcroot.inc
  BCROOT = $(MAKEDIR)\..   # default bcroot to parent of dir where make lives
!endif


#
# Where to find BC tools, libraries and include files
#
BCSRCDIR = $(BCROOT)\source
BCINCDIR = $(BCROOT)\include
BCLIBDIR = $(BCROOT)\lib
BCBINDIR = $(BCROOT)\bin


#
# Where to find OCF dirs-- can set these for special layouts
# Build search paths differently if OCF include dir is not below BC's
#
!if !$d(OCFSRCDIR)
  OCFSRCDIR = $d(OWLROOT)\source\ocf
!endif

!if !$d(BOCOLESRCDIR)
  BOCOLESRCDIR = .
!endif
!if !$d(BOCOLEINCDIR)
  BOCOLEINCDIR = $(BOCOLESRCDIR)
!endif
!if !$d(BOCOLELIBDIR)
  BOCOLELIBDIR = $(BOCOLESRCDIR)
!endif
!if !$d(BOCOLEBINDIR)
  BOCOLEBINDIR = $(BOCOLESRCDIR)\bin
!endif

!if $d(WIN64)
!  if $d(UNICODE)
     BOCOLEOBJDIR = $(BOCOLESRCDIR)\o\w64_UNICODE
!  else
     BOCOLEOBJDIR = $(BOCOLESRCDIR)\o\w64_ANSI
!  endif
!else
!  if $d(UNICODE)
     BOCOLEOBJDIR = $(BOCOLESRCDIR)\o\w32_UNICODE
!  else
     BOCOLEOBJDIR = $(BOCOLESRCDIR)\o\w32_ANSI
!  endif
!endif

INCLUDE = $(BOCOLEINCDIR);$(BCINCDIR)
!if $d(WIN64)
  INCLUDE = $(INCLUDE);$(BOCOLEINCDIR)\BOLEUI64
!else
  INCLUDE = $(INCLUDE);$(BOCOLEINCDIR)\BOLEUI32
!endif

!if $d(WIN64)
  RCINCLUDEPATH = -i$(BCINCDIR)\WIN64 -i$(BCINCDIR)
!else
  RCINCLUDEPATH = -i$(BCINCDIR)\WIN32 -i$(BCINCDIR)
!endif

!if $d(VCL) && !$d(VCLOBJDIR)
  VCLOBJDIR = $(BCLIBDIR)\obj
!endif

.PATH.obj = $(BOCOLEOBJDIR)
.PATH.res = $(BOCOLEOBJDIR)
.PATH.def = $(BOCOLEOBJDIR)
.PATH.lib = $(BOCOLELIBDIR)
.PATH.cpp = $(BOCOLESRCDIR)


#--------------------------------------------------------------------
# Library names
#--------------------------------------------------------------------

#
# If building for VCL, use an RTL that begins "cp" instead of "cw"
#
!if $d(VCL)
  RTLPFX=cp
!else
  RTLPFX=cw
!endif

!if $d(DEBUG)
  VCLLIB = vcld
!else
  VCLLIB = vcl
!endif

#--------------------------------------------------------------------
# Generic BOCOLE Options
#--------------------------------------------------------------------
CFLAGS = -DINC_OLE2 -DMS_OLEUI_DEF -c
!if !$d(DIAGS)
  CFLAGS = $(CFLAGS) -DNDEBUG
!endif
LFLAGS = -c -s


#--------------------------------------------------------------------
# Tools and options for Win64
#--------------------------------------------------------------------
!if $d(WIN64)
  CC       = $(BCBINDIR)\bcc32       # Compiler
  RCC      = $(BCBINDIR)\brcc32 -w64 # Resource compiler
  CCASM    = $(BCBINDIR)\tasm32      # Assembler

! if $d(TLINK)
  LINK     = $(BCBINDIR)\tlink32     # Linker
! else
  LINK     = $(BCBINDIR)\ilink32     # Linker
! endif

  LIB      = $(BCBINDIR)\tlib        # Librarian
  IMPLIB   = $(BCBINDIR)\implib      # Import library generator
  TDSTRIP  = $(BCBINDIR)\tdstrp32    # Debug info extractor

  CFLAGS   = $(CFLAGS) -DWIN64 -i60 -d -O -ObelS -Z -RT- -x-
  STDLIBS  = ole2w64 import64

! if $d(RTLDLL)
    STDLIBS = $(STDLIBS) $(RTLPFX)64$(MT)i
! else
    STDLIBS = $(STDLIBS) $(RTLPFX)64$(MT)
! endif

  BOLEUILIB = $(BOCOLESRCDIR)\boleui64\boleui64.lib
  BOLEUIRES = $(BOCOLESRCDIR)\boleui64\boleui64.res

!  if $d(UNICODE)
    CFLAGS = $(CFLAGS) -DUNICODE
#!  else
#    CFLAGS = $(CFLAGS) -DANSI  deprecated flag, use UNICODE or !UNICODE
!  endif

!  if $d(CODEGUARD)                  # d=add data, t=check this
    CFLAGS = $(CFLAGS) -vGd -vGt
!  endif
!  if $(CODEGUARD) == 2              # c=add calls,
    CFLAGS= $(CFLAGS) -vGc
!  endif

  CFGFILE  = bcc64.cfg               # Name of compiler's config file

!  if $d(DEBUG)
    TLIBOPTS = /P512 /C
!  else
    TLIBOPTS = /P32 /0 /C
!  endif

!  if $d(DLL)
    CFLAGS = $(CFLAGS) -WDE
    LFLAGS = $(LFLAGS) $(LDBG) -Tpd -aa -s -c
    STARTUP= c0d64.obj
!  else
    CFLAGS = $(CFLAGS) -WS
    STARTUP= c0w64.obj
!  endif

!  if $d(CODEGUARD)
    STDLIBS  = cg64 $(STDLIBS)
!  endif

! if $d(VCL)
    CFLAGS  = $(CFLAGS) -b- -Vx -Ve
    LFLAGS  = $(LFLAGS) -j$(VCLOBJDIR)
    STDLIBS = $(STDLIBS) $(VCLLIB)
! endif

!  if !$d(MT)
    CFLAGS = $(CFLAGS) -WM-
!  endif

!  if $d(VCL)
    CFLAGS = $(CFLAGS) -a8
!  elif $d(ALIGN)
    CFLAGS = $(CFLAGS) -a$(ALIGN)
!  else
    CFLAGS = $(CFLAGS) -a8
!  endif

#--------------------------------------------------------------------
# Tools and options for Win32
#--------------------------------------------------------------------
!else
  CC       = $(BCBINDIR)\bcc32       # Compiler
  RCC      = $(BCBINDIR)\brcc32 -w32 # Resource compiler
  CCASM    = $(BCBINDIR)\tasm32      # Assembler

! if $d(TLINK)
  LINK     = $(BCBINDIR)\tlink32     # Linker
! else
  LINK     = $(BCBINDIR)\ilink32     # Linker
! endif

  LIB      = $(BCBINDIR)\tlib        # Librarian
  IMPLIB   = $(BCBINDIR)\implib      # Import library generator
  TDSTRIP  = $(BCBINDIR)\tdstrp32    # Debug info extractor

  CFLAGS   = $(CFLAGS) -DWIN32 -i60 -d -O -Obels -Z -RT- -x-
  STDLIBS  = ole2w32 import32

! if $d(RTLDLL)
    STDLIBS = $(STDLIBS) $(RTLPFX)32$(MT)i
! else
    STDLIBS = $(STDLIBS) $(RTLPFX)32$(MT)
! endif

  BOLEUILIB = $(BOCOLESRCDIR)\boleui32\boleui32.lib
  BOLEUIRES = $(BOCOLESRCDIR)\boleui32\boleui32.res

!  if $d(UNICODE)
    CFLAGS = $(CFLAGS) -DUNICODE
!  else
    CFLAGS = $(CFLAGS) -DANSI
!  endif

!  if $d(CODEGUARD)                  # d=add data, t=check this
    CFLAGS = $(CFLAGS) -vGd -vGt
!  endif
!  if $(CODEGUARD) == 2              # c=add calls,
    CFLAGS= $(CFLAGS) -vGc
!  endif

  CFGFILE  = bcc32.cfg               # Name of compiler's config file

!  if $d(DEBUG)
    TLIBOPTS = /P512 /C
!  else
    TLIBOPTS = /P32 /0 /C
!  endif

!  if $d(DLL)
    CFLAGS = $(CFLAGS) -WDE
    LFLAGS = $(LFLAGS) $(LDBG) -Tpd -aa -s -c
    STARTUP= c0d32.obj
!  else
    CFLAGS = $(CFLAGS) -WS
    STARTUP= c0w32.obj
!  endif

!  if $d(CODEGUARD)
    STDLIBS  = cg32 $(STDLIBS)
!  endif

! if $d(VCL)
    CFLAGS  = $(CFLAGS) -b- -Vx -Ve
    LFLAGS  = $(LFLAGS) -j$(VCLOBJDIR)
    STDLIBS = $(STDLIBS) $(VCLLIB)
! endif

!  if !$d(MT)
    CFLAGS = $(CFLAGS) -WM-
!  endif

!  if $d(VCL)
    CFLAGS = $(CFLAGS) -a4
!  elif $d(ALIGN)
    CFLAGS = $(CFLAGS) -a$(ALIGN)
!  else
    CFLAGS = $(CFLAGS) -a4
!  endif
!endif

#--------------------------------------------------------------------
# Rules
#--------------------------------------------------------------------
.rc.res:
  $(RCC) $(RCINCLUDEPATH) -fo$@ $.

.cpp.obj:
  $(CC) {$& }

#--------------------------------------------------------------------
# Various file lists needed for dependency checking and LIBing.
#
#   OBJS_CMN are the objs common to static libs and the dll
#   OBJS_LIB are the objs that go into libs and implibs only, not the dlls
#   OBJS_DLL are the objs that go into the dll only
#   LIBDEPOBJS is the list of object files for lib/implib dependency checking
#   LIBOBJS is the list of object files for building/addingto the libraries
#   LINKOBJS is the list of object files for building the DLL
#--------------------------------------------------------------------

# Common, unsectioned objs go into static libs and DLLs
#
OBJS_CMN = \
 PFXbolecomp.obj \
 PFXbolesvc.obj  \
 PFXbolecman.obj \
 PFXbolecont.obj \
 PFXbolesite.obj \
 PFXbolectrl.obj \
 PFXbolefont.obj \
 PFXboleips.obj  \
 PFXbolefact.obj \
 PFXboledoc.obj  \
 PFXbolepart.obj \
 PFXbolepict.obj \
 PFXboleclip.obj \
 PFXbhatch.obj   \
 PFXolenew.obj   \
 PFXboledata.obj \
 PFXbolehelp.obj \
 PFXboleguid.obj \
 PFXbolever.obj  \
 PFXbconnpnt.obj \
 PFXbocxctrl.obj \
 PFXmovewin.obj

!if $d(WIN32)
  OBJS_CMN = $(OBJS_CMN) \
    PFXutils.obj
!endif

OBJS_DLL =  \
 PFXoledll.obj 

# Put together obj groups into lists for different targets
#
_OBJDIR_ = $(BOCOLEOBJDIR)\\
!if $d(DLL)
 LIBDEPOBJS= $(OBJS_CMN:PFX=) $(OBJS_DLL:PFX=)
 LIBOBJS   = $(OBJS_CMN:PFX=+) 
 LINKOBJS  = $(OBJS_CMN:PFX=$(_OBJDIR_)) $(OBJS_DLL:PFX=$(_OBJDIR_))
!endif


#--------------------------------------------------------------------
# targets
#--------------------------------------------------------------------
all: cfgfile $(BOCOLEBINDIR)\$(BOCONAME).DLL

cfgfile:
  @-if not exist $(.PATH.obj)\..\NUL md $(.PATH.obj)\..
  @-if not exist $(.PATH.obj)\NUL md $(.PATH.obj)
  @-if not exist $(BOCOLEBINDIR)\NUL md $(BOCOLEBINDIR)
  @copy &&|
-I$(INCLUDE)
-c -w $(CFLAGS) $(CDBG) $(CDIAG)
-n$(.PATH.obj)
| $(CFGFILE) > NUL

$(BOCOLEBINDIR)\$(BOCONAME).DLL : $(LIBDEPOBJS) \
                                  $(.PATH.obj)\$(BOCONAME).DEF \
                                  $(.PATH.obj)\$(BOCORES) \
                                  $(BOLEUILIB) $(BOLEUIRES)
  $(LINK) @&&|
$(LFLAGS) -L$(BCLIBDIR) $(STARTUP) +
$(LINKOBJS)
$(BOCOLEBINDIR)\$(BOCONAME).DLL
$(.PATH.obj)\$(BOCONAME).map
$(BOLEUILIB) $(STDLIBS)
$(.PATH.obj)\$(BOCONAME).def
!if $d(WIN32)
$(.PATH.obj)\$(BOCORES) $(BOLEUIRES)
!endif
|
!if $d(WIN64)
  $(RLINK) @&&|
-v -31 -k -fi $(.PATH.obj)\$(BOCORES) $(BOLEUIRES) $(BOCOLEBINDIR)\$(BOCONAME).DLL
|
!endif
  $(TDSTRIP) -s $(BOCOLEBINDIR)\$(BOCONAME).DLL
  $(IMPLIB) $(BOCOLEBINDIR)\$(BOCONAME).LIB $(BOCOLEBINDIR)\$(BOCONAME).DLL

!if $d(WIN64)
#--------------------------------------------------------------------
# DEF file [WIN64]
#--------------------------------------------------------------------
$(.PATH.obj)\$(BOCONAME).DEF:
  @copy &&|
LIBRARY      $(BOCONAME)
DESCRIPTION  'Aggregable Generic OLE2 Container and Object Implementation'
EXETYPE      WINDOWS

;CODE         LOADONCALL MOVEABLE DISCARDABLE
DATA         MULTIPLE NONSHARED

SEGMENTS
  _TEXT      PRELOAD MOVEABLE DISCARDABLE
;  WEP_TEXT   PRELOAD MOVEABLE DISCARDABLE

;HEAPSIZE     4096

EXPORTS

    ; Explicitly exported initialization routine
    ;
    DLLGETCLASSOBJECT             @2
    CREATECLASSMGR                @3
    DLLCANUNLOADNOW               @4
;   DLLENTRYPOINT                 @5

   _IID_IBPart                    @260
   _IID_IBSite                    @261
   _IID_IBContainer               @262
   _IID_IBDocument                @263
   _IID_IBService                 @264
   _IID_IBApplication             @265
   _IID_IBClassMgr                @266
   _IID_IBLinkInfo                @267
   _IID_IBLinkable                @268
   _IID_IBRootLinkable            @269
   _IID_IBContains                @270
   _IID_BOleDocument              @271
   _IID_BOlePart                  @272
   _IID_IBDataConsumer            @273
   _IID_IBDataProvider            @274
   _IID_IBWindow                  @275
   _IID_IBDataNegotiator          @276
   _IID_IBDropDest                @277
   _IID_BOleShadowData            @278
   _IID_IBClass                   @279
   _IID_IBOverrideBrowseLinks     @280
   _IID_IBOverrideHelpButton      @281
   _IID_IBSite2                   @282
   _IID_BOleFont                  @283
   _IID_BOlePicture               @284
   _IID_IBPicture                 @285
| $(.PATH.obj)\$(BOCONAME).def  > NUL

#--------------------------------------------------------------------
# DEF file [win32]
#--------------------------------------------------------------------
!else

$(.PATH.obj)\$(BOCONAME).DEF:
  @copy &&|
LIBRARY      $(BOCONAME)
DESCRIPTION  'Aggregable Generic OLE2 Container and Object Implementation'
EXETYPE      WINDOWS

;CODE        LOADONCALL MOVEABLE DISCARDABLE
DATA         MULTIPLE NONSHARED

SEGMENTS
  _TEXT      PRELOAD MOVEABLE DISCARDABLE

;HEAPSIZE    1024

EXPORTS
  DllGetClassObject  @2
  CreateClassMgr     @3
  DllCanUnloadNow    @4
  DllEntryPoint      @5

  _IID_IBPart                    @260
  _IID_IBSite                    @261
  _IID_IBContainer               @262
  _IID_IBDocument                @263
  _IID_IBService                 @264
  _IID_IBApplication             @265
  _IID_IBClassMgr                @266
  _IID_IBLinkInfo                @267
  _IID_IBLinkable                @268
  _IID_IBRootLinkable            @269
  _IID_IBContains                @270
  _IID_BOleDocument              @271
  _IID_BOlePart                  @272
  _IID_IBDataConsumer            @273
  _IID_IBDataProvider            @274
  _IID_IBWindow                  @275
  _IID_IBDataNegotiator          @276
  _IID_IBDropDest                @277
  _IID_BOleShadowData            @278
  _IID_IBClass                   @279
  _IID_IBOverrideBrowseLinks     @280
  _IID_IBOverrideHelpButton      @281
  _IID_IBSite2                   @282
  _IID_BOleFont                  @283
  _IID_BOlePicture               @284
  _IID_IBPicture                 @285

| $(.PATH.obj)\$(BOCONAME).def  > NUL

!endif

$(.PATH.obj)\$(BOCORES):
  $(RCC) $&.rc @&&|
$(RCINCLUDEPATH)
| -fo$@

$(BOLEUILIB):
 @if not exist $(BOLEUILIB) echo $(BOLEUILIB) must exist

$(BOLEUIRES):
 @if not exist $(BOLEUIRES) echo $(BOLEUIRES) must exist

clean:
  @echo Removing .OBJs from $(.PATH.obj)
  @if exist $(.PATH.obj)\*.obj del $(.PATH.obj)\*.obj
  @if exist $(.PATH.obj)\*.csm del $(.PATH.obj)\*.csm


