#
# Makefile include file - detects the VC++ compiler version and target platform.
# Copyright (c) 2011-2015 Vidar Hasfjord
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
# variables. If not set, report error.

!if !defined(VISUALSTUDIOVERSION)
! message vc_detection: System environment variable VISUALSTUDIOVERSION is not defined. 
! error Unable to detect the compiler.
!endif

!if [echo $(VISUALSTUDIOVERSION) | $(FINDSTR) /B /L "17." >nul] == 0

COMPILER_VERSION=1930

!elseif [echo $(VISUALSTUDIOVERSION) | $(FINDSTR) /B /L "16." >nul] == 0

COMPILER_VERSION=1920

!elseif [echo $(VISUALSTUDIOVERSION) | $(FINDSTR) /B /L "15." >nul] == 0

COMPILER_VERSION=1910

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
