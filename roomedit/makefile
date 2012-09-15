# Makefile for room editor

#
# Borland C++ tools
#
COMPILERDIR = d:\bc45
TOPDIR=..

COMPILERFLAGS = -v -vi -H -H=obj\WINDEU.CSM -N -a2 -Od -y -W -I$(COMPILERDIR)\include;include;res;$(TOPDIR)\include -D_OWLPCH

CC   = $(COMPILERDIR)\bin\Bcc32
LINK = $(COMPILERDIR)\bin\TLink32
RC   = $(COMPILERDIR)\bin\Brc32


#
# Options
#
IDE_LFLAGS32 =  -L$(COMPILERDIR)\LIB;.;$(TOPDIR)\lib -c -A:12 -Tpe -aa -c -x
IDE_RFLAGS32 =  -w32
CNIEAT_RESbwindeappdrc = -I$(COMPILERDIR)\INCLUDE;RES;INCLUDE -D_OWLPCH;

#
# Dependency List
#

all : windeu32.exe

OBJS = \
   OBJ\save.obj\
   OBJ\load.obj\
   OBJ\textures.obj\
   OBJ\bspmake.obj\
   OBJ\dibutil.obj\
   OBJ\move.obj\
   OBJ\pal.obj\
   OBJ\buildtim.obj\
   OBJ\gfx.obj\
   OBJ\memory.obj\
   OBJ\names.obj\
   OBJ\objects.obj\
   OBJ\things.obj\
   OBJ\undo.obj\
   OBJ\wads.obj\
   OBJ\windeu.obj\
   OBJ\aboutdlg.obj\
   OBJ\cnflddlg.obj\
   OBJ\cnfsedlg.obj\
   OBJ\cnfthdlg.obj\
   OBJ\cnfvedlg.obj\
   OBJ\entrydlg.obj\
   OBJ\episdlg.obj\
   OBJ\inpt2dlg.obj\
   OBJ\lineedit.obj\
   OBJ\lprogdlg.obj\
   OBJ\mastdlg.obj\
   OBJ\prefdlg.obj\
#   OBJ\roomid.obj\
   OBJ\seditdlg.obj\
   OBJ\statdlg.obj\
   OBJ\thingdlg.obj\
   OBJ\vertdlg.obj\
   OBJ\viewbmp.obj\
   OBJ\wadlidlg.obj\
   OBJ\workdlg.obj\
   OBJ\newers.obj\
   OBJ\bmp256ct.obj\
   OBJ\editcli.obj\
   OBJ\editscro.obj\
   OBJ\infocont.obj\
   OBJ\maincli.obj\
   OBJ\mainfram.obj\
   OBJ\mapdc.obj\
   OBJ\tooltip.obj\
   OBJ\windeapp.obj

LIBS = \
	bidsf.lib \
	owlwf.lib \
	import32.lib \
	cw32.lib \
	$(TOPDIR)\lib\wrap.lib

windeu32.exe : makedirs $(OBJS) OBJ\windeapp.res
  $(LINK) /v $(IDE_LFLAGS32) @<<
$(COMPILERDIR)\LIB\c0w32.obj $(OBJS), $@,,$(LIBS),,OBJ\windeapp.res
<<

makedirs:
  -@mkdir OBJ

{source}.cpp{obj}.obj:
	$(CC) $(COMPILERFLAGS) -c -o$@ $<

{source}.c{obj}.obj:
	$(CC) $(COMPILERFLAGS) -c -o$@ $<

OBJ\windeapp.res :  RES\windeapp.rc
  $(RC) $(IDE_RFLAGS32) $(CNIEAT_RESbwindeappdrc) -R -FO$@ RES\windeapp.rc

clean:
  -@del /q obj\*
  -@rmdir obj