# makefile for blakdeco

TOPDIR=..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(DECODIR)

OBJS = $(OUTDIR)\blakdeco.obj

all: makedirs $(OUTDIR)\blakdeco.exe

$(OUTDIR)\blakdeco.exe : $(OBJS)
	$(LINK) -nologo /SUBSYSTEM:CONSOLE",5.01" $(OBJS) -OUT:$@
	$(CP) $@ $(BLAKBINDIR)

!include $(TOPDIR)\rules.mak