# Common makefile definitions for modules

LIBS = user32.lib gdi32.lib comctl32.lib $(BLAKLIBDIR)\meridian.lib
INCLUDE = $(INCLUDE);$(CLIENTDIR);$(CLIENTDIR)\$(OUTDIR)
CFLAGS = $(CFLAGS) /LD

# directory for running local test client
MODULECLIENTDIR = $(TOPDIR)\run\localclient\resource

SOURCEDIR = $(MODULEDIR)\$(MODULE)

.obj.dll:
	$(LINK) -dll $(LINKFLAGS) \
	  $(LINKWINDOWSFLAGS) -def:$(SOURCEDIR)\$(*B).def -out:$@ \
	  $** $(LIBS)
	$(CP) $@ $(MODULECLIENTDIR)

all: makedirs $(OUTDIR)\$(MODULE).dll
