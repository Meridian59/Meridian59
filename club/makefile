# makefile for club

TOPDIR=..
!include $(TOPDIR)\common.mak

# ----------------------------------------------------------------------
# Additional compiler flags (see common.mak)
# /TP           Compile as C++ code
# /arch:SSE     Use SSE instructions (VS2013+ default to SSE2)
CFLAGS = $(CFLAGS) /arch:SSE /TP /D_USING_V110_SDK71_

# ----------------------------------------------------------------------
# Additional linker flags (see common.mak)
# /SUBSYSTEM:WINDOWS",5.01" UI Windows XP (5.01)
LINKFLAGS = $(LINKFLAGS) /SUBSYSTEM:WINDOWS",5.01"

SOURCEDIR = $(TOPDIR)\club

LIBS = user32.lib gdi32.lib advapi32.lib libarchive.lib zlib.lib comctl32.lib wininet.lib

all: makedirs $(OUTDIR)\club.exe

OBJS = \
    $(OUTDIR)\club.obj \
    $(OUTDIR)\clubarchive.obj \
    $(OUTDIR)\util.obj \
    $(OUTDIR)\transfer.obj

$(OUTDIR)\club.exe : $(OBJS) $(OUTDIR)\club.res 
        $(LINK) $(LINKFLAGS) $** $(LIBS) -out:$@ 
        $(CP) $@ $(CLIENTRUNDIR)

!include $(TOPDIR)\rules.mak