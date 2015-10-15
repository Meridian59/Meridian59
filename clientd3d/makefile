# Makefile for Blakston client

TOPDIR=..
!include $(TOPDIR)\common.mak

# ----------------------------------------------------------------------
# Additional compiler flags (see common.mak)
# /TP           Compile as C++ code
# /arch:SSE     Use SSE instructions (VS2013+ default to SSE2)
# /DBLAKCLIENT  Specific for m59 client
# -I$(OUTDIR)   Need to put build dir in include path, because trig and palette
#               generated files are there.
CFLAGS = $(CFLAGS) /arch:SSE /TP /DBLAKCLIENT /D_USING_V110_SDK71_ -I$(OUTDIR)  

# ----------------------------------------------------------------------
# Additional linker flags (see common.mak)
# /SUBSYSTEM:WINDOWS",5.01"   UI Windows XP (5.01)
# /SUBSYSTEM:CONSOLE",5.01"   NON-UI Windows XP (5.01)
LINKFLAGS        = $(LINKFLAGS) /SUBSYSTEM:WINDOWS",5.01"
LINKFLAGSCONSOLE = $(LINKFLAGS) /SUBSYSTEM:CONSOLE",5.01"

SOURCEDIR = $(CLIENTDIR)

LIBS = \
    user32.lib gdi32.lib comdlg32.lib shell32.lib \
    ws2_32.lib comctl32.lib advapi32.lib \
    winmm.lib wininet.lib ole32.lib \
    d3d9.lib dxerr.lib irrklang.lib \
    rscload.lib libpng.lib zlib.lib libarchive.lib 

DEFFILE  = $(SOURCEDIR)\client.def

all: makedirs $(OUTDIR)\meridian.exe

OBJS = \
    $(OUTDIR)\client.obj \
    $(OUTDIR)\list.obj \
    $(OUTDIR)\modules.obj \
    $(OUTDIR)\map.obj \
    $(OUTDIR)\msgfiltr.obj \
    $(OUTDIR)\loadrsc.obj \
    $(OUTDIR)\protocol.obj \
    $(OUTDIR)\com.obj \
    $(OUTDIR)\draw3d.obj \
    $(OUTDIR)\trig.obj \
    $(OUTDIR)\palette.obj \
    $(OUTDIR)\object3d.obj \
    $(OUTDIR)\overlay.obj \
    $(OUTDIR)\moveobj.obj \
    $(OUTDIR)\tooltip.obj \
    $(OUTDIR)\annotate.obj \
    $(OUTDIR)\idlist.obj \
    $(OUTDIR)\roomanim.obj \
    $(OUTDIR)\statconn.obj \
    $(OUTDIR)\memmap.obj \
    $(OUTDIR)\mapfile.obj \
    $(OUTDIR)\fastasm.obj \
    $(OUTDIR)\boverlay.obj \
    $(OUTDIR)\web.obj \
    $(OUTDIR)\about.obj \
    $(OUTDIR)\login.obj \
    $(OUTDIR)\srvrstr.obj \
    $(OUTDIR)\effect.obj \
    $(OUTDIR)\bitmap.obj \
    $(OUTDIR)\download.obj \
    $(OUTDIR)\select.obj \
    $(OUTDIR)\logoff.obj \
    $(OUTDIR)\transfer.obj \
    $(OUTDIR)\statoffl.obj \
    $(OUTDIR)\cursor.obj \
    $(OUTDIR)\client3d.obj \
    $(OUTDIR)\md5.obj \
    $(OUTDIR)\crc.obj \
    $(OUTDIR)\xlat.obj \
    $(OUTDIR)\rops.obj \
    $(OUTDIR)\guest.obj \
    $(OUTDIR)\profane.obj \
    $(OUTDIR)\object.obj \
    $(OUTDIR)\graphics.obj \
    $(OUTDIR)\winmsg.obj \
    $(OUTDIR)\server.obj \
    $(OUTDIR)\dialog.obj \
    $(OUTDIR)\draw.obj \
    $(OUTDIR)\graphctl.obj \
    $(OUTDIR)\animate.obj \
    $(OUTDIR)\config.obj \
    $(OUTDIR)\project.obj \
    $(OUTDIR)\uselist.obj \
    $(OUTDIR)\move.obj \
    $(OUTDIR)\startup.obj \
    $(OUTDIR)\keyhook.obj \
    $(OUTDIR)\toolbar.obj \
    $(OUTDIR)\game.obj \
    $(OUTDIR)\intrface.obj \
    $(OUTDIR)\ownerdrw.obj \
    $(OUTDIR)\util.obj \
    $(OUTDIR)\table.obj \
    $(OUTDIR)\statterm.obj \
    $(OUTDIR)\statgame.obj \
    $(OUTDIR)\font.obj \
    $(OUTDIR)\textin.obj \
    $(OUTDIR)\msgbox.obj \
    $(OUTDIR)\editbox.obj \
    $(OUTDIR)\gameuser.obj \
    $(OUTDIR)\offer.obj \
    $(OUTDIR)\maindlg.obj \
    $(OUTDIR)\lookdlg.obj \
    $(OUTDIR)\color.obj \
    $(OUTDIR)\say.obj \
    $(OUTDIR)\audio.obj \
    $(OUTDIR)\key.obj \
    $(OUTDIR)\buy.obj \
    $(OUTDIR)\statstrt.obj \
    $(OUTDIR)\dibutil.obj \
    $(OUTDIR)\pal.obj \
    $(OUTDIR)\drawbsp.obj \
    $(OUTDIR)\bspload.obj \
    $(OUTDIR)\parse.obj \
    $(OUTDIR)\drawbmp.obj \
    $(OUTDIR)\winmenu.obj \
    $(OUTDIR)\cache.obj \
    $(OUTDIR)\ping.obj \
    $(OUTDIR)\objdraw.obj \
    $(OUTDIR)\lagbox.obj \
    $(OUTDIR)\regexpr.obj \
    $(OUTDIR)\fixed.obj \
    $(OUTDIR)\d3dcache.obj \
    $(OUTDIR)\d3ddriver.obj \
    $(OUTDIR)\d3dparticle.obj \
    $(OUTDIR)\d3drender.obj \
    $(OUTDIR)\matrix.obj \
    $(OUTDIR)\xform.obj


$(OUTDIR)\trig.c $(OUTDIR)\trig.h: $(OUTDIR)\maketrig.exe
	$(OUTDIR)\maketrig.exe $(OUTDIR)

$(OUTDIR)\pal.c: $(OUTDIR)\makepal.exe $(PALETTEFILE)
	$(OUTDIR)\makepal $(PALETTEFILE) $(OUTDIR)

$(OUTDIR)\crc.obj : $(TOPDIR)\util\crc.c
	$(CC) -Fo$(OUTDIR)/ $(CFLAGS) /Fd$(OUTDIR)\vc90.pdb /FpCpch -c $**

$(OUTDIR)\md5.obj : $(TOPDIR)\util\md5.c
	$(CC) -Fo$(OUTDIR)/ $(CFLAGS) /Fd$(OUTDIR)\vc90.pdb /FpCpch -c $**

$(OUTDIR)\meridian.exe: $(OBJS) $(DEFFILE) $(OUTDIR)\client.res
	$(LINK) $(LINKFLAGS) @<<
$(OBJS) $(LIBS) -def:$(DEFFILE) $(OUTDIR)\client.res -OUT:$@ 
<<
	$(CP) $(OUTDIR)\meridian.lib $(BLAKLIBDIR)
	$(CP) $@ $(CLIENTRUNDIR)

$(OUTDIR)\maketrig.exe: $(OUTDIR)\maketrig.obj
	$(LINK) $** -out:$@ $(LINKFLAGSCONSOLE)
	$(CP) $@ $(BLAKBINDIR) 

$(OUTDIR)\makepal.exe: $(OUTDIR)\makepal.obj
	$(LINK) $** -out:$@ $(LINKFLAGSCONSOLE)
	$(CP) $@ $(BLAKBINDIR) 

profile: $(OBJS) client.res $(BLAKNTLIBS) 
	$(LINK) $(LINKFLAGS) @<<
$(OBJS) /PROFILE $(LIBS) -def:$(DEFFILE) client.res -OUT:meridian.exe
<<
#       prep /lc /excall /inc draw3d.obj meridian.exe   
	prep /ft /excall /inc drawbsp.obj meridian.exe

!include $(TOPDIR)\rules.mak
