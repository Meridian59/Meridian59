# Makefile for room editor

#
# Borland C++ 5.02 compiler package required
#
#
COMPILERDIR = c:\bc5
TOPDIR=..

CC   = $(COMPILERDIR)\bin\Bcc32
LINK = $(COMPILERDIR)\bin\TLink32
RC   = $(COMPILERDIR)\bin\Brc32

# bcc32
# -4      80486 instructions (default -3 for 80386)
# -v      Turn on source debugging
# -vi     Generate inline functions inline (use -vi- for out-of-line)
# -H      Generate and use precompiled headers
# -H=     Set the name of the file to use for precompiled headers
# -N      Check stack overflow
# -WM     Target is a 32-bit multi-threaded target (-WM- = single)
# -Od     Disable all optimizations
# -y      Debug line numbers on
# -W      Target is a Windows application
# -I      Set the include file search path
COMPILERFLAGS = -4 -v -vi -H -H=obj\WINDEU.CSM -N -WM- -Od -y -W -I$(COMPILERDIR)\include;include;res -D_OWLPCH

# tlink32
# -L    Specify library search path
# -c    Case sensitive link
# -Tpe  Output file type: EXE
# -aa   Application Type: Windowing API (Subsystem Windows)
# -x    No map
# -v    Full symbolic debug information
# -V4.0 Linker subsystem NT 4.0 (is 3.5 by default)
IDE_LFLAGS32 =  -L$(COMPILERDIR)\LIB;. -c -Tpe -aa -x -v -V4.0

# brc32
# -32   build 32-bit Windows compatible res/exe files
# -w32  provided for downward compatibility
IDE_RFLAGS32 =  -32
CNIEAT_RESbwindeappdrc = -I$(COMPILERDIR)\INCLUDE;RES;INCLUDE -D_OWLPCH;

#
# Dependency List
#

all : windeu32.exe

# alphabetical ordered like in .IDE project
OBJS = \
   OBJ\aboutdlg.obj\
   OBJ\bmp256ct.obj\
   OBJ\bspmake.obj\
   OBJ\buildtim.obj\
   OBJ\cnflddlg.obj\
   OBJ\cnfsedlg.obj\
   OBJ\cnfthdlg.obj\
   OBJ\cnfvedlg.obj\
   OBJ\dibutil.obj\
   OBJ\editcli.obj\
   OBJ\editscro.obj\
   OBJ\entrydlg.obj\
   OBJ\episdlg.obj\
   OBJ\gfx.obj\
   OBJ\infocont.obj\
   OBJ\inpt2dlg.obj\
   OBJ\lineedit.obj\
   OBJ\load.obj\
   OBJ\lprogdlg.obj\
   OBJ\maincli.obj\
   OBJ\mainfram.obj\
   OBJ\mapdc.obj\
   OBJ\mastdlg.obj\
   OBJ\memory.obj\
   OBJ\move.obj\
   OBJ\names.obj\
   OBJ\newers.obj\
   OBJ\objects.obj\
   OBJ\pal.obj\
   OBJ\prefdlg.obj\
   OBJ\save.obj\
   OBJ\seditdlg.obj\
   OBJ\statdlg.obj\
   OBJ\textures.obj\
   OBJ\thingdlg.obj\
   OBJ\things.obj\
   OBJ\tooltip.obj\
   OBJ\undo.obj\
   OBJ\vertdlg.obj\
   OBJ\viewbmp.obj\
   OBJ\wadlidlg.obj\
   OBJ\wads.obj\
   OBJ\windeapp.obj\
   OBJ\windeu.obj\
   OBJ\workdlg.obj

LIBS = \
	bidsf.lib \
	owlwf.lib \
	import32.lib \
	cw32.lib

windeu32.exe : makedirs $(OBJS) OBJ\windeapp.res
  $(LINK) $(IDE_LFLAGS32) @<<
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