# stuff included in blakston makefiles

# defining DYNAMIC uses multi-threaded C runtime DLL; 
# otherwise link statically with single-threaded lib

# defining RELEASE compiles optimized
# defining NODEBUG omits debugging information
# defining FINAL implies release, and also removes debugging strings from client executable 

!ifdef FINAL
RELEASE = 1
NODPRINTFS = 1
!endif

!ifdef RELEASE
!undef DEBUG
OUTDIR=release
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

BLAKBINDIR = $(TOPDIR)\bin
BLAKLIBDIR = $(TOPDIR)\lib
BLAKINCLUDEDIR = $(TOPDIR)\include
BLAKSERVRUNDIR = $(TOPDIR)\run\server
CLIENTRUNDIR = $(TOPDIR)\run\localclient

KODDIR = $(TOPDIR)\kod
KODINCLUDEDIR = $(KODDIR)\include

PALETTEFILE = $(TOPDIR)\blakston.pal

# compiler specs -- uses multi-threaded DLL C runtime library
# /TP builds C files in C++ mode
# /WX treats warnings as errors
# /GR- turns off RTTI
# /EHsc- turns off exceptions
# /wd4996  disables warning (GetVersionExA has been deprecated)
# -arch:IA32 disables SSE instructions (not supported on ancient Athlon CPUs)

CCOMMONFLAGS = -nologo -DBLAK_PLATFORM_WINDOWS -DWIN32 \
             -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE \
             -D_WINSOCK_DEPRECATED_NO_WARNINGS /wd4996 \
				 -TP -WX -GR- -EHsc- -arch:IA32

CNORMALFLAGS = $(CCOMMONFLAGS) -W2 /Ox
CDEBUGFLAGS = $(CCOMMONFLAGS) -Zi -W3 -DBLAKDEBUG
CNODEBUGFLAGS = $(CCOMMONFLAGS) -W2 -DBLAKDEBUG
LINKNORMALFLAGS =/release
LINKDEBUGFLAGS = /debug
LINKNODEBUGFLAGS =
LINKCONSOLEFLAGS = -subsystem:console,5.01
LINKWINDOWSFLAGS = -subsystem:windows,5.01

!ifdef DEBUG

!ifdef DEBUGINFO
CFLAGS = $(CDEBUGFLAGS)
LINKFLAGS = $(LINKDEBUGFLAGS)
!else
CFLAGS = $(CNODEBUGFLAGS)
LINKFLAGS = $(LINKNODEBUGFLAGS)
!endif DEBUGINFO

!else
CFLAGS = $(CNORMALFLAGS)
LINKFLAGS = $(LINKNORMALFLAGS)

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
RC     = rc -nologo

LEX = $(TOPDIR)\bin\flex -I -i
YACC = $(TOPDIR)\bin\bison -d -t
CP = copy /Y
RM = -del /Q
RMDIR = -rmdir
MV = move
LATEX = latex
MAKENSIS = "c:\program files (x86)\nsis\makensis.exe"

BC = $(BLAKBINDIR)\bc
MAKEBGF = $(BLAKBINDIR)\makebgf

# environment variables for compiler

LIB = $(LIB);$(BLAKLIBDIR);$(TOPDIR)\miles\lib
INCLUDE = $(INCLUDE);$(BLAKINCLUDEDIR);$(TOPDIR)\miles\include
