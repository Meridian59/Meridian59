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
COMPILERFLAGS = -4 -v -vi -H -H=obj\windeu.csm -N -WM- -Od -y -W -I$(COMPILERDIR)\INCLUDE;include;res -D_OWLPCH

# tlink32
# -L    Specify library search path
# -c    Case sensitive link
# -Tpe  Output file type: EXE
# -aa   Application Type: Windowing API (Subsystem Windows)
# -x    No map
# -v    Full symbolic debug information
# -V4.0 Linker subsystem NT 4.0 (is 3.5 by default)
LINKERFLAGS = -L$(COMPILERDIR)\LIB -c -Tpe -aa -x -v -V4.0

# brc32
# -32   build 32-bit Windows compatible res/exe files
RESOURCEFLAGS = -32

#
# Dependency List
#

all : windeu32.exe

# alphabetical ordered like in .IDE project
OBJS = \
   obj\aboutdlg.obj\
   obj\bmp256ct.obj\
   obj\bspmake.obj\
   obj\buildtim.obj\
   obj\cnflddlg.obj\
   obj\cnfsedlg.obj\
   obj\cnfthdlg.obj\
   obj\cnfvedlg.obj\
   obj\dibutil.obj\
   obj\editcli.obj\
   obj\editscro.obj\
   obj\entrydlg.obj\
   obj\episdlg.obj\
   obj\gfx.obj\
   obj\infocont.obj\
   obj\inpt2dlg.obj\
   obj\lineedit.obj\
   obj\load.obj\
   obj\lprogdlg.obj\
   obj\maincli.obj\
   obj\mainfram.obj\
   obj\mapdc.obj\
   obj\mastdlg.obj\
   obj\memory.obj\
   obj\move.obj\
   obj\names.obj\
   obj\newers.obj\
   obj\objects.obj\
   obj\pal.obj\
   obj\prefdlg.obj\
   obj\save.obj\
   obj\seditdlg.obj\
   obj\statdlg.obj\
   obj\textures.obj\
   obj\thingdlg.obj\
   obj\things.obj\
   obj\tooltip.obj\
   obj\undo.obj\
   obj\vertdlg.obj\
   obj\viewbmp.obj\
   obj\wadlidlg.obj\
   obj\wads.obj\
   obj\windeapp.obj\
   obj\windeu.obj\
   obj\workdlg.obj

LIBS = \
	bidsf.lib \
	owlwf.lib \
	import32.lib \
	cw32.lib

windeu32.exe: makedirs $(OBJS) obj\windeapp.res
  $(LINK) $(LINKERFLAGS) @<<
$(COMPILERDIR)\LIB\c0w32.obj $(OBJS), $@,,$(LIBS),,obj\windeapp.res
<<

makedirs:
  -@mkdir obj

{source}.cpp{obj}.obj:
  $(CC) $(COMPILERFLAGS) -c -o$@ $<

{source}.c{obj}.obj:
  $(CC) $(COMPILERFLAGS) -c -o$@ $<

obj\windeapp.res:  res\windeapp.rc
  $(RC) $(RESOURCEFLAGS) -R -FO$@ res\windeapp.rc

clean:
  -@del /q obj\*
  -@rmdir obj