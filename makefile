#
# overall makefile
#

TOPDIR=.
!include common.mak

.SILENT:

# make ignores targets if they match directory names
all: Bzlib Blibarchive Blibpng Bserver Bclient Bmodules Bkod Bdeco Bupdater Bbbgun Bkeybind Bresource

Bserver: Bresource
	echo Making $(COMMAND) in $(BLAKSERVDIR)
	cd $(BLAKSERVDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

Bclient: Butil Bresource
	echo Making $(COMMAND) in $(CLIENTDIR)
	cd $(CLIENTDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..
!if !DEFINED(NOCOPYFILES)
# Postbuild handles its own echoes
	$(POSTBUILD)
!endif NOCOPYFILES

Bmodules: Bclient
	echo Making $(COMMAND) in $(MODULEDIR)
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

Bdeco:
	echo Making $(COMMAND) in $(DECODIR)
	cd $(DECODIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

Bresource: Bmakebgf Bbbgun
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
	
Blibpng:
	echo Making $(COMMAND) in $(LIBPNGDIR)
	cd $(LIBPNGDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..
	
Blibarchive:
	echo Making $(COMMAND) in $(LIBARCHIVEDIR)
	cd $(LIBARCHIVEDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

Bzlib:
	echo Making $(COMMAND) in $(ZLIBDIR)
	cd $(ZLIBDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..
	
clean:
        set NOCOPYFILES=1
        set COMMAND=clean
        $(MAKE) /$(MAKEFLAGS)
		$(RM) $(TOPDIR)\postbuild.log >nul 2>&1
		$(RM) $(BLAKSERVDIR)\channel\*.txt 2>nul