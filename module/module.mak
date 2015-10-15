# Common makefile definitions for modules

# ----------------------------------------------------------------------
# Additional compiler flags (see common.mak)
# /TP                   Compile as C++ code
# /arch:SSE             Use SSE instructions (VS2013+ default to SSE2)
# /D_USING_V110_SDK71_  VS uses this for XP compatibility
CFLAGS = $(CFLAGS) /arch:SSE /TP /D_USING_V110_SDK71_

# ----------------------------------------------------------------------
# Additional linker flags (see common.mak)
# /SUBSYSTEM:WINDOWS",5.01"    UI Windows XP (5.01)
# /DLL                         Create DLL 
LINKFLAGS = $(LINKFLAGS) /SUBSYSTEM:WINDOWS",5.01" /DLL

LIBS            = user32.lib gdi32.lib comctl32.lib $(BLAKLIBDIR)\meridian.lib
INCLUDE         = $(INCLUDE);$(CLIENTDIR);$(CLIENTDIR)\$(OUTDIR)
SOURCEDIR       = $(MODULEDIR)\$(MODULE)
MODULECLIENTDIR = $(TOPDIR)\run\localclient\resource

.obj.dll:
	$(LINK) $(LINKFLAGS) \
	  -def:$(SOURCEDIR)\$(*B).def -out:$@ \
	  $** $(LIBS)
	$(CP) $@ $(MODULECLIENTDIR)

all: makedirs $(OUTDIR)\$(MODULE).dll
