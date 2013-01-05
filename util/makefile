# makefile for remote debugging utils

TOPDIR=..
!include $(TOPDIR)\common.mak

OBJS3 = $(OUTDIR)\scrshot.obj $(OUTDIR)\dibutil.obj $(OUTDIR)\copy.obj $(OUTDIR)\file.obj

SOURCEDIR = $(TOPDIR)\util

NTLIBS = user32.lib gdi32.lib

all: \
       makedirs \
       $(OUTDIR)\roocrypt.exe \
       $(OUTDIR)\rscload.lib \
       $(OUTDIR)\rscprint.exe \
       $(OUTDIR)\bmpdump.exe \
       $(OUTDIR)\rscmerge.exe \
       $(OUTDIR)\paldump.exe \
       $(OUTDIR)\palcomp.exe \
       $(OUTDIR)\porttest.exe 

$(OUTDIR)\remot.exe: $(OUTDIR)\remot.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) -out:$@

$(OUTDIR)\rwatson.exe: $(OUTDIR)\rwatson.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) -out:$@

$(OUTDIR)\scrshot.exe: $(OBJS3)
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) -out:$@

$(OUTDIR)\enumwin.exe: $(OUTDIR)\enumwin.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) -out:$@

$(OUTDIR)\btnpush.exe: $(OUTDIR)\btnpush.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) -out:$@

$(OUTDIR)\inst.exe: $(OUTDIR)\inst.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) -out:$@

$(OUTDIR)\ftptest.exe: ftptest.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) wininet.lib -out:$@

$(OUTDIR)\roocrypt.exe: $(OUTDIR)\roocrypt.obj $(OUTDIR)\memmap.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) wrap.lib -out:$@
	$(CP) $@ $(BLAKBINDIR)

$(OUTDIR)\rscload.lib: $(OUTDIR)\rscload.obj $(OUTDIR)\memmap.obj
	$(LIBPRG) $** -out:$@
	$(CP) $@ $(BLAKLIBDIR)

$(OUTDIR)\rscprint.exe: $(OUTDIR)\rscprint.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) rscload.lib setargv.obj -out:$@
	$(CP) $@ $(BLAKBINDIR)

$(OUTDIR)\bmpdump.exe: $(OUTDIR)\bmpdump.obj $(OUTDIR)\memmap.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) setargv.obj -out:$@
	$(CP) $@ $(BLAKBINDIR)

$(OUTDIR)\paldump.exe: $(OUTDIR)\paldump.obj $(OUTDIR)\memmap.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) setargv.obj -out:$@
	$(CP) $@ $(BLAKBINDIR)

$(OUTDIR)\rscmerge.exe: $(OUTDIR)\rscmerge.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) rscload.lib setargv.obj -out:$@
	$(CP) $@ $(BLAKBINDIR)

$(OUTDIR)\syscolors.exe: syscolors.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) rscload.lib setargv.obj -out:$@
	$(CP) $@ $(BLAKBINDIR)

$(OUTDIR)\palcomp.exe: $(OUTDIR)\palcomp.obj $(OUTDIR)\memmap.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) setargv.obj -out:$@
	$(CP) $@ $(BLAKBINDIR)

$(OUTDIR)\porttest.exe: $(OUTDIR)\porttest.obj
	$(LINK) $(LINKFLAGS) $** $(NTLIBS) wsock32.lib -out:$@
	$(CP) $@ $(BLAKBINDIR)

!include $(TOPDIR)\rules.mak
