# Makefile for keybind app

TOPDIR=..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(KEYBINDDIR)

CFLAGS = $(CFLAGS) /EHsc
LINKFLAGS = $(LINKFLAGS) $(LINKWINDOWSFLAGS)
LIBS = $(LIBS) nafxcw.lib

OBJS = \
	$(OUTDIR)\Assignkey.obj \
	$(OUTDIR)\Communication.obj \
	$(OUTDIR)\Interaction.obj \
	$(OUTDIR)\Keybindsps.obj \
	$(OUTDIR)\m59bind.obj \
	$(OUTDIR)\m59bind.res \
	$(OUTDIR)\m59bindDlg.obj \
	$(OUTDIR)\Modifier.obj \
	$(OUTDIR)\Mouse.obj \
	$(OUTDIR)\Movement.obj \
	$(OUTDIR)\Stdafx.obj \
	$(OUTDIR)\Targeting.obj \
	$(OUTDIR)\Themap.obj

all: makedirs $(OUTDIR)\m59bind.exe

$(OUTDIR)\m59bind.exe: $(OBJS)
	$(LINK) $(OBJS) $(LIBS) -OUT:$@ $(LINKFLAGS)
	$(CP) $@ $(CLIENTRUNDIR)

!include $(TOPDIR)\rules.mak
