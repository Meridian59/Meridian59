#
# overall makefile
#

TOPDIR=.
!include common.mak

.SILENT:

# make ignores targets if they match directory names
all: Bserver Bclient Bmodules Bkod Bdeco Bupdater Bbbgun Bkeybind Bresource

Bserver: Bresource
	echo Making in $(BLAKSERVDIR)
	cd $(BLAKSERVDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

Bclient: Butil Bresource
	echo Making in $(CLIENTDIR)
	cd $(CLIENTDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..
!if !DEFINED(NOCOPYFILES)
# Postbuild handles its own echoes
	$(POSTBUILD)
!endif NOCOPYFILES

Bmodules: Bclient
	echo Making in $(MODULEDIR)
	cd $(MODULEDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

Bcompiler:
	echo Making $(COMMAND) in $(BLAKCOMPDIR)
	cd $(BLAKCOMPDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

Bdiff:
	echo Making $(COMMAND) in $(DIFFDIR)
	cd $(DIFFDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

Bkod: Bdiff Bcompiler
	echo Making $(COMMAND) in $(KODDIR)
	cd $(KODDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..
!if !DEFINED(NOCOPYFILES)
	echo Copying kodbase.txt and kod include files
	-$(CP) $(KODDIR)\kodbase.txt $(BLAKSERVRUNDIR) >nul 2>&1
	-$(CP) $(KODDIR)\include\*.khd $(BLAKSERVRUNDIR) >nul 2>&1
	echo Copying .rsc and creating client .rsb file
	@-$(RM) $(CLIENTRUNDIR)\resource\*.rsc >nul 2>&1
	@-$(RM) $(CLIENTRUNDIR)\resource\*.rsb >nul 2>&1
	-$(RSCMERGE) $(CLIENTRUNDIR)\resource\rsc0000.rsb $(BLAKSERVRUNDIR)\rsc\*.rsc >nul
!endif NOCOPYFILES

Bdeco:
	echo Making $(COMMAND) in $(DECODIR)
	cd $(DECODIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

Bresource: Bmakebgf
	echo Making $(COMMAND) in $(RESOURCEDIR)
	cd $(RESOURCEDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

Bmakebgf:
	echo Making $(COMMAND) in $(MAKEBGFDIR)
	cd $(MAKEBGFDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

Butil:
	echo Making $(COMMAND) in $(UTILDIR)
	cd $(UTILDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

Bupdater:
	echo Making $(COMMAND) in $(CLUBDIR)
	cd $(CLUBDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

Bbbgun:
	echo Making $(COMMAND) in $(BBGUNDIR)
	cd $(BBGUNDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

Bkeybind:
	echo Making $(COMMAND) in $(KEYBINDDIR)
	cd $(KEYBINDDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

clean:
        set NOCOPYFILES=1
        set COMMAND=clean
        $(MAKE) /$(MAKEFLAGS)
		$(RM) $(BLAKSERVRUNDIR)\rsc\*.rsc >nul 2>&1
		$(RM) $(BLAKSERVRUNDIR)\loadkod\*.bof >nul 2>&1
		$(RM) $(BLAKSERVRUNDIR)\memmap\*.bof >nul 2>&1
		$(RM) $(BLAKSERVDIR)\channel\*.txt