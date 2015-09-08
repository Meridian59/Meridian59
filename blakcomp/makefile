# Makefile for Blakod compiler

TOPDIR=..
!include $(TOPDIR)\common.mak

# ----------------------------------------------------------------------
# Additional compiler flags (see common.mak)
# /TP           Compile as C++ code
# /arch:SSE     Use SSE instructions (VS2013+ default to SSE2)
# /wd4065       Ignore specific warning
CFLAGS = $(CFLAGS) /arch:SSE /TP /wd4065

# ----------------------------------------------------------------------
# Additional linker flags (see common.mak)
# /SUBSYSTEM:CONSOLE",5.01" NO-UI Windows XP (5.01)
LINKFLAGS = $(LINKFLAGS) /SUBSYSTEM:CONSOLE",5.01"

SOURCEDIR = $(BLAKCOMPDIR)

OBJS = \
    $(OUTDIR)\lexyy.obj \
    $(OUTDIR)\blakcomp.tab.obj \
    $(OUTDIR)\actions.obj \
    $(OUTDIR)\table.obj \
    $(OUTDIR)\kodbase.obj \
    $(OUTDIR)\codegen.obj \
    $(OUTDIR)\codeutil.obj \
    $(OUTDIR)\function.obj \
    $(OUTDIR)\util.obj \
    $(OUTDIR)\sort.obj \
    $(OUTDIR)\optimize.obj \
    $(OUTDIR)\resource.obj

all: makedirs $(OUTDIR)\bc.exe

$(OUTDIR)\bc.exe: $(OBJS)
	$(LINK) $(OBJS) setargv.obj $(LIBS) -OUT:$@ $(LINKFLAGS)
	$(CP) $@ $(BLAKBINDIR)

$(OUTDIR)\lexyy.obj: $(OUTDIR)\lexyy.c $(OUTDIR)\blakcomp.tab.h

$(OUTDIR)\lexyy.c: {$(SOURCEDIR)}blakcomp.l {$(SOURCEDIR)}blakcomp.h
	$(LEX) -o$(OUTDIR)\lexyy.c $(SOURCEDIR)\blakcomp.l

$(OUTDIR)\blakcomp.tab.c $(OUTDIR)\blakcomp.tab.h: {$(SOURCEDIR)}blakcomp.y {$(SOURCEDIR)}blakcomp.h
	$(YACC) $(SOURCEDIR)\blakcomp.y -o $(OUTDIR)\blakcomp.tab.c

!include $(TOPDIR)\rules.mak