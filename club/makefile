TOPDIR=..
DYNAMIC=1
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\club

CLUBLIBS = user32.lib gdi32.lib archive_static.lib comctl32.lib wininet.lib zlib.lib advapi32.lib

all: makedirs $(OUTDIR)\club.exe

OBJS = \
        $(OUTDIR)\club.obj \
        $(OUTDIR)\clubarchive.obj \
        $(OUTDIR)\util.obj \
        $(OUTDIR)\transfer.obj

$(OUTDIR)\club.exe : $(OBJS) $(OUTDIR)\club.res 
        $(LINK) $(LINKFLAGS) $** $(CLUBLIBS) -out:$@ $(LINKWINDOWSFLAGS) /NODEFAULTLIB:libc
        $(CP) $@ $(CLIENTRUNDIR)

!include $(TOPDIR)\rules.mak