# Makefile for bbgun

TOPDIR=..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(BBGUNDIR)

LIBS = oldnames.lib user32.lib gdi32.lib comdlg32.lib

all: makedirs $(OUTDIR)\bbgun.exe

OBJS = \
        $(OUTDIR)\bbgun.obj \
        $(OUTDIR)\dibutil.obj \
        $(OUTDIR)\draw.obj \
        $(OUTDIR)\pal.obj \
        $(OUTDIR)\palette.obj \
        $(OUTDIR)\bmparea.obj \
        $(OUTDIR)\bbgfile.obj


$(OUTDIR)\bbgun.exe: $(OBJS) $(OUTDIR)\bbgun.res
	$(LINK) $** $(LIBS) -OUT:$@ $(LINKFLAGS) $(LINKWINDOWSFLAGS)

$(OUTDIR)\bbgun.res : $(SOURCEDIR)\bbgun.rc
        $(RC) /fo $@ /i$(SOURCEDIR) $**

!include $(TOPDIR)\rules.mak