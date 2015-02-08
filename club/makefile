TOPDIR=..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\club

CLUBLIBS = user32.lib gdi32.lib archive.lib comctl32.lib wininet.lib

all: makedirs $(OUTDIR)\club.exe

OBJS = \
        $(OUTDIR)\club.obj \
        $(OUTDIR)\clubarchive.obj \
        $(OUTDIR)\util.obj \
        $(OUTDIR)\transfer.obj

$(OUTDIR)\club.exe : $(OBJS) $(OUTDIR)\club.res 
        $(LINK) $(LINKFLAGS) $** $(CLUBLIBS) -out:$@ $(LINKWINDOWSFLAGS)
        $(CP) $@ $(CLIENTRUNDIR)

!include $(TOPDIR)\rules.mak
