# stuff included in blakston makefiles

# defining DYNAMIC uses multi-threaded C runtime DLL; 
# otherwise link statically with single-threaded lib

# defining RELEASE compiles optimized
# defining NODEBUG omits debugging information
# defining FINAL implies release, and also removes debugging strings from client executable 

!ifdef FINAL
RELEASE = 1
!endif

!ifdef RELEASE
!undef DEBUG
OUTDIR=release
NODPRINTFS = 1
!else
DEBUG = 1
OUTDIR=debug
!endif

# set to get debugging info in executable
!ifdef NODEBUG
!undef DEBUGINFO
!else
DEBUGINFO = 1
!endif

# Set source directories

# Subdirectories of components

BBGUNDIR    = $(TOPDIR)\bbgun
CLIENTDIR   = $(TOPDIR)\clientd3d
BLAKSERVDIR = $(TOPDIR)\blakserv
BLAKCOMPDIR = $(TOPDIR)\blakcomp
ROOMEDITDIR = $(TOPDIR)\roomedit
DOCDIR      = $(TOPDIR)\doc
DECODIR     = $(TOPDIR)\blakdeco
MAKEBGFDIR  = $(TOPDIR)\makebgf
RESOURCEDIR = $(TOPDIR)\resource
DIFFDIR     = $(TOPDIR)\blakdiff
MODULEDIR   = $(TOPDIR)\module
CRUSHERDIR  = $(TOPDIR)\crusher
UTILDIR     = $(TOPDIR)\util
SPROCKETDIR = $(TOPDIR)\sprocket
CLUBDIR     = $(TOPDIR)\club
KEYBINDDIR  = $(TOPDIR)\keybind
WAVEMIXDIR  = $(TOPDIR)\wavemix
LIBPNGDIR   = $(TOPDIR)\libpng

BLAKBINDIR = $(TOPDIR)\bin
BLAKLIBDIR = $(TOPDIR)\lib
BLAKINCLUDEDIR = $(TOPDIR)\include
BLAKSERVRUNDIR = $(TOPDIR)\run\server
CLIENTRUNDIR = $(TOPDIR)\run\localclient
MYSQLINCLUDEDIR = $(TOPDIR)\include\mysql
DX9INCLUDEDIR = $(TOPDIR)\include\dx9
LIBPNGINCLUDEDIR = $(TOPDIR)\include\libpng

KODDIR = $(TOPDIR)\kod
KODINCLUDEDIR = $(KODDIR)\include

PALETTEFILE = $(TOPDIR)\blakston.pal

# compiler specs -- uses multi-threaded DLL C runtime library
# /TP builds C files in C++ mode
# /WX treats warnings as errors
# /GR- turns off RTTI
# /EHsc- turns off exceptions

CCOMMONFLAGS = -nologo -DBLAK_PLATFORM_WINDOWS -DWIN32 \
             -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE \
             -D_WINSOCK_DEPRECATED_NO_WARNINGS /wd4996 /wd4390 \
			 -TP -WX -GR- -EHsc- /MP -W3 /fp:fast

CNORMALFLAGS = $(CCOMMONFLAGS) /Ox /GL /GF
CDEBUGFLAGS = $(CCOMMONFLAGS) -Zi -DBLAKDEBUG
CNODEBUGFLAGS = $(CCOMMONFLAGS) -DBLAKDEBUG

# For shared libs (dll)
LINKNORMALFLAGS = -nologo /release /machine:ix86 /LTCG /LARGEADDRESSAWARE /OPT:REF /OPT:ICF
LINKDEBUGFLAGS = -nologo /debug /machine:ix86 /LARGEADDRESSAWARE
LINKNODEBUGFLAGS = -nologo /machine:ix86 /LARGEADDRESSAWARE

# For static libs (lib)
LINKSTNORMALFLAGS = -nologo /machine:ix86 /LTCG
LINKSTDEBUGFLAGS = -nologo /machine:ix86

!ifdef DEBUG

!ifdef DEBUGINFO
CFLAGS = $(CDEBUGFLAGS)
LINKFLAGS = $(LINKDEBUGFLAGS)
LINKSTFLAGS = $(LINKSTDEBUGFLAGS)
!else
CFLAGS = $(CNODEBUGFLAGS)
LINKFLAGS = $(LINKNODEBUGFLAGS)
LINKSTFLAGS = $(LINKSTDEBUGFLAGS)
!endif DEBUGINFO

!else
CFLAGS = $(CNORMALFLAGS)
LINKFLAGS = $(LINKNORMALFLAGS)
LINKSTFLAGS = $(LINKSTNORMALFLAGS)

!endif DEBUG

!ifdef DLL
LINKFLAGS = $(LINKFLAGS) /DLL
!endif

!ifdef DYNAMIC
CFLAGS = $(CFLAGS) /MD
!else
CFLAGS = $(CFLAGS) /MT
!endif DYNAMIC

!ifdef NODPRINTFS
CFLAGS = $(CFLAGS) -DNODPRINTFS
!endif NODPRINTFS

# programs

CC     = cl
MAKE   = nmake -nologo
LIBPRG = lib -nologo
LINK   = link -nologo
LINKST = lib
RC     = rc
RSCMERGE = $(TOPDIR)\bin\rscmerge.exe -o
POSTBUILD = $(TOPDIR)\postbuild.bat
LEX = $(TOPDIR)\bin\flex -I -i
YACC = $(TOPDIR)\bin\bison -d -t
XCP = xcopy /ydi
CP = copy /Y
RM = -del /Q
RMDIR = -rmdir
MV = move
LATEX = latex
MAKENSIS = "c:\program files (x86)\nsis\makensis.exe"

BC = $(BLAKBINDIR)\bc
MAKEBGF = $(BLAKBINDIR)\makebgf

# environment variables for compiler

LIB = $(LIB);$(BLAKLIBDIR)
INCLUDE = $(INCLUDE);$(BLAKINCLUDEDIR);$(MYSQLINCLUDEDIR);$(DX9INCLUDEDIR);$(LIBPNGINCLUDEDIR)
