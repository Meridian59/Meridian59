# -----------------------------------------------------------------
# Switch between RELEASE (by cmdline RELEASE=1) or DEBUG (default/no param)
!ifdef RELEASE
!undef DEBUG
OUTDIR = release
!else
OUTDIR = debug
DEBUG = 1
!endif

# -----------------------------------------------------------------
# Subdirectories of components
BBGUNDIR      = $(TOPDIR)\bbgun
CLIENTDIR     = $(TOPDIR)\clientd3d
BLAKSERVDIR   = $(TOPDIR)\blakserv
BLAKCOMPDIR   = $(TOPDIR)\blakcomp
ROOMEDITDIR   = $(TOPDIR)\roomedit
DOCDIR        = $(TOPDIR)\doc
DECODIR       = $(TOPDIR)\blakdeco
MAKEBGFDIR    = $(TOPDIR)\makebgf
RESOURCEDIR   = $(TOPDIR)\resource
DIFFDIR       = $(TOPDIR)\blakdiff
MODULEDIR     = $(TOPDIR)\module
UTILDIR       = $(TOPDIR)\util
SPROCKETDIR   = $(TOPDIR)\sprocket
CLUBDIR       = $(TOPDIR)\club
KEYBINDDIR    = $(TOPDIR)\keybind
WAVEMIXDIR    = $(TOPDIR)\wavemix
LIBPNGDIR     = $(TOPDIR)\libpng
LIBARCHIVEDIR = $(TOPDIR)\libarchive
ZLIBDIR       = $(TOPDIR)\libzlib

# Other directories
BLAKBINDIR     = $(TOPDIR)\bin
BLAKLIBDIR     = $(TOPDIR)\lib
BLAKSERVRUNDIR = $(TOPDIR)\run\server
CLIENTRUNDIR   = $(TOPDIR)\run\localclient

# Include directories
BLAKINCLUDEDIR       = $(TOPDIR)\include
MYSQLINCLUDEDIR      = $(TOPDIR)\include\mysql
DX9INCLUDEDIR        = $(TOPDIR)\include\dx9
LIBPNGINCLUDEDIR     = $(TOPDIR)\include\libpng
LIBARCHIVEINCLUDEDIR = $(TOPDIR)\include\libarchive
ZLIBINCLUDEDIR       = $(TOPDIR)\include\zlib
IRRKLANGINCLUDEDIR   = $(TOPDIR)\include\irrklang

# KOD and KOD include directories
KODDIR        = $(TOPDIR)\kod
KODINCLUDEDIR = $(KODDIR)\include

# -----------------------------------------------------------------
# Palette
PALETTEFILE = $(TOPDIR)\blakston.pal

# -----------------------------------------------------------------
# Common MS VC++ compiler specs for all builds
# /nologo                 Surpress banner
# /GR-                    Turns off RTTI
# /EHsc-                  Turns off exceptions
# /MP                     Compile using multiple cpu cores
# /fp:precise             Use precise floating-point calculations
#                         /fp:fast causes issues with BSP tree calcs
# /DBLAK_PLATFORM_WINDOWS Build blakserv for Windows
# /DLIBARCHIVE_STATIC     For static libarchive build
# /DHAVE_CONFIG_H         For libarchive build
# /WX                     Treat warnings as errors
# /W3                     Warnings level
# /wdXXXX                 Disable specific warnings
CCOMMONFLAGS = /nologo /GR- /EHsc- /MP /fp:precise \
    /DBLAK_PLATFORM_WINDOWS /DWIN32 \
    /D_CRT_SECURE_NO_WARNINGS \
    /D_CRT_NONSTDC_NO_DEPRECATE \
    /D_WINSOCK_DEPRECATED_NO_WARNINGS \
    /DLIBARCHIVE_STATIC \
    /DHAVE_CONFIG_H \
    /WX /W3 /wd4996 /wd4390 

# Specific MS VC++ compiler specs, different for release and debug
# /MT           Use static multithreaded VC++ runtime
# /MTd          Use static debug multithreaded VC++ runtime
# /Ox           Maximum optimization
# /Ob2          Inline any suitable
# /Oi           Enable system internal functions
# /Ot           Prefer fast over small code
# /Oy           Suppress frame pointer
# /GL           Full optimization across modules (for /LTCG)
# /GF           Eliminate duplicate strings
# /ZI           Debug info generation (edit and continue)
# /DBLAKDEBUG   Tell blakserv to run in debug mode
# /DNODPRINTFS  Used to surpress debug output and few others in release client
CNORMALFLAGS  = $(CCOMMONFLAGS) /MT /Ox /Ob2 /Oi /Ot /Oy /GL /GF /DNODPRINTFS
CDEBUGFLAGS   = $(CCOMMONFLAGS) /MT /ZI /DBLAKDEBUG

# -----------------------------------------------------------------
# Linker settings for MS linker
# /nologo              Surpress banner
# /machine:ix86        x86 architecture
# /LARGEADDRESSAWARE   Allow using up to 3GB RAM
# /release             Sets header checksum
# /debug               Creates debug info
# /LTCG                Optimize across modules (see /GL)
# /OPT:REF             Optimization
# /OPT:ICF             Optimization

# For Shared (DLL)
LINKNORMALFLAGS = /nologo /machine:ix86 /LARGEADDRESSAWARE /release /LTCG /OPT:REF /OPT:ICF
LINKDEBUGFLAGS  = /nologo /machine:ix86 /LARGEADDRESSAWARE /debug  

# For Static (LIB)
LINKSTNORMALFLAGS = /nologo /machine:ix86 /LTCG
LINKSTDEBUGFLAGS  = /nologo /machine:ix86

# -----------------------------------------------------------------
# Pick compiler and linker flags based on DEBUG or RELEASE
!ifdef DEBUG
CFLAGS      = $(CDEBUGFLAGS)
LINKFLAGS   = $(LINKDEBUGFLAGS)
LINKSTFLAGS = $(LINKSTDEBUGFLAGS)
!else
CFLAGS      = $(CNORMALFLAGS)
LINKFLAGS   = $(LINKNORMALFLAGS)
LINKSTFLAGS = $(LINKSTNORMALFLAGS)
!endif DEBUG

# -----------------------------------------------------------------
# programs
CC        = cl
MAKE      = nmake -nologo
LINK      = link
LINKST    = lib
RC        = rc
RSCMERGE  = $(TOPDIR)\bin\rscmerge.exe -o
POSTBUILD = $(TOPDIR)\postbuild.bat
LEX       = $(TOPDIR)\bin\flex -I -i
YACC      = $(TOPDIR)\bin\bison -d -t
XCP       = xcopy /ydi
CP        = copy /Y
RM        = -del /Q
RMDIR     = -rmdir
MV        = move
LATEX     = latex
MAKENSIS  = "c:\program files (x86)\nsis\makensis.exe"
BC        = $(BLAKBINDIR)\bc
MAKEBGF   = $(BLAKBINDIR)\makebgf

# -----------------------------------------------------------------
# LIB and INCLUDE directories for compiler and linker
LIB     = $(LIB);$(BLAKLIBDIR)
INCLUDE = $(INCLUDE);$(BLAKINCLUDEDIR);$(MYSQLINCLUDEDIR);$(DX9INCLUDEDIR);$(LIBPNGINCLUDEDIR);$(ZLIBINCLUDEDIR);$(LIBARCHIVEINCLUDEDIR);$(IRRKLANGINCLUDEDIR)
