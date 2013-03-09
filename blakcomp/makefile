# Makefile for Blakod compiler

TOPDIR=..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(BLAKCOMPDIR)

LIBS =

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
        set BISON_SIMPLE=$(SOURCEDIR)\bison.simple
	$(YACC) $(SOURCEDIR)\blakcomp.y -o $(OUTDIR)\blakcomp.tab.c

!include $(TOPDIR)\rules.mak