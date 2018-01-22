# Makefile for wave mixer

TOPDIR=..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(WAVEMIXDIR)

# Build as C so that exported names aren't C++ decorated
CFLAGS = $(CFLAGS:-TP=-TC) /LD

WAVEMIXLIBS = winmm.lib gdi32.lib user32.lib 

WAVEMIXOBJS = $(OUTDIR)\wavemix.obj

all: makedirs $(OUTDIR)\waveplay.dll

$(OUTDIR)\waveplay.dll: $(WAVEMIXOBJS) $(OUTDIR)\wavemix.res
	$(LINK) -dll -def:$(SOURCEDIR)\wavmix32.def $(LINKFLAGS) \
	$(LINKWINDOWSFLAGS) -out:$@ $** $(WAVEMIXLIBS)
	$(CP) $(OUTDIR)\waveplay.lib $(BLAKLIBDIR)
	$(CP) $@ $(BLAKBINDIR)
	$(CP) $@ $(CLIENTRUNDIR)

!include $(TOPDIR)\rules.mak
