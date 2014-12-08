#
# makefile for blakdiff
#

TOPDIR=..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(DIFFDIR)
LINKFLAGS = $(LINKFLAGS) /SUBSYSTEM:CONSOLE",5.01"

OBJS = $(OUTDIR)\blakdiff.obj

all: makedirs $(OUTDIR)\blakdiff.exe

$(OUTDIR)\blakdiff.exe : $(OBJS)
	$(LINK) $(LINKFLAGS) $(OBJS) -out:$@
	$(CP) $@ $(BLAKBINDIR)

!include $(TOPDIR)\rules.mak
