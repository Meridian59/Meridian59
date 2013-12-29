TOPDIR=..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\club

CLUBLIBS = user32.lib gdi32.lib wrap.lib comctl32.lib wininet.lib

all: makedirs $(OUTDIR)\club.exe

OBJS = \
        $(OUTDIR)\club.obj \
        $(OUTDIR)\clubarchive.obj \
        $(OUTDIR)\util.obj \
        $(OUTDIR)\transfer.obj

$(OUTDIR)\club.exe : $(OBJS) $(OUTDIR)\club.res 
        $(LINK) $(LINKFLAGS) $** $(CLUBLIBS) -out:$@ /subsystem:windows
        $(CP) $@ $(CLIENTRUNDIR)

!include $(TOPDIR)\rules.mak