# makefile for makebgf

TOPDIR=..
!include $(TOPDIR)\common.mak

OBJS = \
        $(OUTDIR)\makebgf.obj \
        $(OUTDIR)\dibutil.obj \
        $(OUTDIR)\writebgf.obj

SOURCEDIR = $(MAKEBGFDIR)
LINKFLAGS = $(LINKFLAGS) /SUBSYSTEM:CONSOLE",5.01"

LIBS = gdi32.lib zdll.lib setargv

all: makedirs $(OUTDIR)\makebgf.exe

$(OUTDIR)\makebgf.exe: $(OBJS)
	$(LINK) $(LINKFLAGS) $** $(LIBS) -out:$@
	$(CP) $@ $(BLAKBINDIR)

!include $(TOPDIR)\rules.mak
