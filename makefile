#
# overall makefile
#

TOPDIR=.
!include common.mak

.SILENT:

# make ignores targets if they match directory names
all: Bserver Bclient Bmodules Bkod Bdeco Bupdater Bbbgun Bresource Broomedit

Bserver:
	echo Making $(COMMAND) in $(BLAKSERVDIR)
	cd $(BLAKSERVDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

Bclient: Butil Blibpng Blibarchive
	echo Making $(COMMAND) in $(CLIENTDIR)
	cd $(CLIENTDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

Bmodules: Bclient
	echo Making $(COMMAND) in $(MODULEDIR)
	cd $(MODULEDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

Blibpng: Bzlib
	echo Making $(COMMAND) in $(LIBPNGDIR)
	cd $(LIBPNGDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..\..

Bzlib:
	echo Making $(COMMAND) in $(ZLIBDIR)
	cd $(ZLIBDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..\..

Blibarchive: Bzlib
	echo Making $(COMMAND) in $(LIBARCHIVEDIR)
	cd $(LIBARCHIVEDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..\..



Bcompiler:
	echo Making $(COMMAND) in $(BLAKCOMPDIR)
	cd $(BLAKCOMPDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

Bkod: Bcompiler
	echo Making $(COMMAND) in $(KODDIR)
	cd $(KODDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

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

Broomedit:
	echo Making $(COMMAND) in $(ROOMEDITDIR)
	cd $(ROOMEDITDIR)
	$(MAKE) /$(MAKEFLAGS) $(COMMAND)
	cd ..

clean:
        set COMMAND=clean
        $(MAKE) /$(MAKEFLAGS)
