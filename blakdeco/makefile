# makefile for blakdeco

TOPDIR=..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(DECODIR)

OBJS = $(OUTDIR)\blakdeco.obj

all: makedirs $(OUTDIR)\blakdeco.exe

$(OUTDIR)\blakdeco.exe : $(OBJS)
	$(LINK) $(OBJS) -OUT:$@
	$(CP) $@ $(BLAKBINDIR)

!include $(TOPDIR)\rules.mak