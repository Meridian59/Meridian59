# Common makefile definitions for modules

LIBS = user32.lib gdi32.lib comctl32.lib $(BLAKLIBDIR)\meridian.lib
INCLUDE = $(INCLUDE);$(CLIENTDIR);$(CLIENTDIR)\$(OUTDIR)
CFLAGS = $(CFLAGS) /LD
LINKFLAGS = $(LINKFLAGS) /SUBSYSTEM:WINDOWS",5.01"

# directory for running local test client
MODULECLIENTDIR = $(TOPDIR)\run\localclient\resource

SOURCEDIR = $(MODULEDIR)\$(MODULE)

# On clientside: Use SSE instead of SSE2 (default for VS2013)
# Because of old CPU (Athlon XP)
CCOMMONFLAGS = $(CCOMMONFLAGS) /arch:SSE

.obj.dll:
	$(LINK) -dll $(LINKFLAGS) \
	  -def:$(SOURCEDIR)\$(*B).def -out:$@ \
	  $** $(LIBS)
	$(CP) $@ $(MODULECLIENTDIR)

all: makedirs $(OUTDIR)\$(MODULE).dll
