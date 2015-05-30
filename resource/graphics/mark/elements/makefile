# Makefile 

TOPDIR=..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = .

DIRS = fire \
       ice \
       earth

all: 
	@-for %i in ($(DIRS)) do \
		@if EXIST %i (	cd %i & \
	 	$(MAKE) /$(MAKEFLAGS) & \
		cd .. )
