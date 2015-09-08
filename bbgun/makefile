# Makefile for bbgun

TOPDIR=..
!include $(TOPDIR)\common.mak

# ----------------------------------------------------------------------
# Additional compiler flags (see common.mak)
# /TP           Compile as C++ code
# /arch:SSE     Use SSE instructions (VS2013+ default to SSE2)
CFLAGS = $(CFLAGS) /arch:SSE /TP

# ----------------------------------------------------------------------
# Additional linker flags (see common.mak)
# /SUBSYSTEM:WINDOWS",5.01" UI Windows XP (5.01)
LINKFLAGS = $(LINKFLAGS) /SUBSYSTEM:WINDOWS",5.01"

SOURCEDIR = $(BBGUNDIR)
LIBS = user32.lib gdi32.lib comdlg32.lib

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
	$(LINK) $** $(LIBS) -OUT:$@ $(LINKFLAGS)
	$(CP) $(OUTDIR)\bbgun.exe $(TOPDIR)\resource\graphics\

$(OUTDIR)\bbgun.res : $(SOURCEDIR)\bbgun.rc
        $(RC) /fo $@ /i$(SOURCEDIR) $**

!include $(TOPDIR)\rules.mak