# makefile for blakdeco

TOPDIR=..
!include $(TOPDIR)\common.mak

# ----------------------------------------------------------------------
# Additional compiler flags (see common.mak)
# /TP           Compile as C++ code
# /arch:SSE     Use SSE instructions (VS2013+ default to SSE2)
CFLAGS = $(CFLAGS) /arch:SSE /TP

# ----------------------------------------------------------------------
# Additional linker flags (see common.mak)
# /SUBSYSTEM:CONSOLE",5.01" NO-UI Windows XP (5.01)
LINKFLAGS = $(LINKFLAGS) /SUBSYSTEM:CONSOLE",5.01"

OBJS = $(OUTDIR)\blakdeco.obj

all: makedirs $(OUTDIR)\blakdeco.exe

$(OUTDIR)\blakdeco.exe : $(OBJS)
	$(LINK) $(LINKFLAGS) $(OBJS) -OUT:$@
	$(CP) $@ $(BLAKBINDIR)

!include $(TOPDIR)\rules.mak