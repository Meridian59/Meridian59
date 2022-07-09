#
# Makefile include file - detects the VC++ compiler version and target platform.
# Copyright © 2011-2015 Vidar Hasfjord
# See OWLNext for license information (http://owlnext.sourceforge.net).
#
# Parameters:
#
#   MSCL - filename of the Microsoft C++ compiler (default is "cl.exe").
#
# Return values:
#
#   COMPILER_VERSION - the colloquial version of the compiler (see below).
#   TARGET_PLATFORM - the CPU ISA targeted by the compiler (x86 | x64).
#
# If detection fails, an error is reported and the make is halted.

#------------------------------------------------------------------------------
# Check parameters and define helper macros.

!if !defined(MSCL)
MSCL=cl.exe
!endif

FINDSTR="%systemroot%\system32\findstr"

#------------------------------------------------------------------------------
# The configuration scripts for Visual Studio 2012 and later define the system 
# environment variables VISUALSTUDIOVERSION and PLATFORM, which specifies the
# compiler version and target platform, respectively. So, first check for these
# variables. If not set, then we'll attempt to detect the version and platform
# by more elaborate means below.

!if defined(VISUALSTUDIOVERSION)

!if [echo $(VISUALSTUDIOVERSION) | $(FINDSTR) /B /L "14." >nul] == 0

COMPILER_VERSION=14

!elseif [echo $(VISUALSTUDIOVERSION) | $(FINDSTR) /B /L "12." >nul] == 0

COMPILER_VERSION=12

!elseif [echo $(VISUALSTUDIOVERSION) | $(FINDSTR) /B /L "11." >nul] == 0

COMPILER_VERSION=11

!elseif [echo $(VISUALSTUDIOVERSION) | $(FINDSTR) /B /L "10." >nul] == 0

COMPILER_VERSION=10

!else
! message vc_detection: Check that the version of the compiler is supported by this version of OWLNext.
! error Unable to detect the compiler version.
!endif

!if "$(PLATFORM)" == "" || "$(PLATFORM)" == "x86"

TARGET_PLATFORM=x86

!elseif "$(PLATFORM)" == "x64" || "$(PLATFORM)" == "X64" || "$(PLATFORM)" == "x86-64" || "$(PLATFORM)" == "amd64"

TARGET_PLATFORM=x64

!else
! message vc_detection: Check that the target platform is supported by this version of OWLNext.
! error Unable to detect the target platform architecture.
!endif

!else

#------------------------------------------------------------------------------
# Extract banner output (title, version, etc.) for the compiler.
# We use this text for analysis (compiler version and target platform).

MSCL_BANNER="$(TEMP)\OWL_MSCL_BANNER.tmp"

!if [$(MSCL) >$(MSCL_BANNER) 2>&1] != 0
! message vc_detection: Check that the compiler "$(MSCL)" is accessible in the PATH system environment variable.
! error Unable to generate compiler banner output for analysis.
!endif

!if [if not exist $(MSCL_BANNER) exit 2] != 0
! message vc_detection: Check that the TEMP system environment variable specifies an accessible path.
! error Failed to create temporary file $(MSCL_BANNER).
!endif

#------------------------------------------------------------------------------
# Detect the compiler version.
#
# Search the compiler output for the major version number, and translate:
#
#   CL.EXE Version 16 = 10 (Microsoft Visual Studio 2010)
#   CL.EXE Version 17 = 11 (Microsoft Visual Studio 2012)
#   CL.EXE Version 18 = 12 (Microsoft Visual Studio 2013)
#   CL.EXE Version 19 = 14 (Microsoft Visual Studio 2015)

# This helper macro contains a regular expression that matches the 2 minor 
# parts of any version number; e.g. ".00.20617" for "Version 18.00.20617". 
# Note that we need to double-escape the right angle bracket, i.e. "^^]", so
# that "^]" appears properly escaped in the if-[]-expression.
#
MINOR_RE=\.[0-9^^][0-9^^]*\.[0-9^^][0-9^^]*

!if [$(FINDSTR) "19$(MINOR_RE)" $(MSCL_BANNER) >nul] == 0

COMPILER_VERSION=14

!elseif [$(FINDSTR) "18$(MINOR_RE)" $(MSCL_BANNER) >nul] == 0

COMPILER_VERSION=12

!elseif [$(FINDSTR) "17$(MINOR_RE)" $(MSCL_BANNER) >nul] == 0

COMPILER_VERSION=11

!elseif [$(FINDSTR) "16$(MINOR_RE)" $(MSCL_BANNER) >nul] == 0

COMPILER_VERSION=10

!else
! message vc_detection: Check that the version of the compiler is supported by this version of OWLNext.
! error Unable to detect the compiler version.
!endif

#------------------------------------------------------------------------------
# Detect the target platform based on the current compiler tools.
#
# Search the compiler output for a hint about the target platform architecture.

!if [$(FINDSTR) "x64" $(MSCL_BANNER) >nul] == 0

TARGET_PLATFORM=x64

!elseif [$(FINDSTR) "x86" $(MSCL_BANNER) >nul] == 0

TARGET_PLATFORM=x86

!else
! message vc_detection: Check that the target platform is supported by this version of OWLNext.
! error Unable to detect the target platform architecture.
!endif

!endif
