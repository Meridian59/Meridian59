# makefile for remote debugging utils

TOPDIR=..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\util

NTLIBS = user32.lib gdi32.lib

all: \
       makedirs \
       $(OUTDIR)\rscload.lib \
       $(OUTDIR)\rscprint.exe \
       $(OUTDIR)\rscmerge.exe \
       $(OUTDIR)\paldump.exe \
       $(OUTDIR)\palcomp.exe \
       $(OUTDIR)\porttest.exe 

$(OUTDIR)\rscload.lib: $(OUTDIR)\rscload.obj $(OUTDIR)\memmap.obj
	$(LIBPRG) $** -out:$@
	$(CP) $@ $(BLAKLIBDIR)

$(OUTDIR)\rscprint.exe: $(OUTDIR)\rscprint.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) rscload.lib setargv.obj -out:$@
	$(CP) $@ $(BLAKBINDIR)

$(OUTDIR)\paldump.exe: $(OUTDIR)\paldump.obj $(OUTDIR)\memmap.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) setargv.obj -out:$@
	$(CP) $@ $(BLAKBINDIR)

$(OUTDIR)\rscmerge.exe: $(OUTDIR)\rscmerge.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) rscload.lib setargv.obj -out:$@
	$(CP) $@ $(BLAKBINDIR)

$(OUTDIR)\palcomp.exe: $(OUTDIR)\palcomp.obj $(OUTDIR)\memmap.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) setargv.obj -out:$@
	$(CP) $@ $(BLAKBINDIR)

$(OUTDIR)\porttest.exe: $(OUTDIR)\porttest.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) wsock32.lib -out:$@
	$(CP) $@ $(BLAKBINDIR)

!include $(TOPDIR)\rules.mak
