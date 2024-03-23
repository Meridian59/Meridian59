# Makefile for downloadable client DLLs

TOPDIR=..
!include $(TOPDIR)\common.mak

MODULES = admin char mailnews merintr intro dm chess

# Propagate error code if any sub-build fails
all:
   @echo off
	@for %i in ($(MODULES)) do @if EXIST %i (cd %i && ($(MAKE) /$(MAKEFLAGS) || (echo FAILURE > ..\error.tmp)) && cd ..)
   @if exist error.tmp (del error.tmp && exit /b 1)

release:
	-@for %i in ($(MODULES)) do @if EXIST %i (cd %i && $(MAKE) /$(MAKEFLAGS) RELEASE=1 && cd ..)

clean:
	-@for %i in ($(MODULES)) do @if EXIST %i (cd %i && $(MAKE) /$(MAKEFLAGS) clean && cd ..)
