# Makefile for downloadable client DLLs

TOPDIR=..
!include $(TOPDIR)\common.mak

MODULES = admin char mailnews merintr intro dm chess stats

all:
	-@for %i in ($(MODULES)) do @if EXIST %i (cd %i & $(MAKE) /$(MAKEFLAGS) & cd ..)

release:
	-@for %i in ($(MODULES)) do @if EXIST %i (cd %i & $(MAKE) /$(MAKEFLAGS) RELEASE=1 & cd ..)

clean:
	-@for %i in ($(MODULES)) do @if EXIST %i (cd %i & $(MAKE) /$(MAKEFLAGS) clean & cd ..)
