# Makefile 

TOPDIR=..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = .

DIRS = elements \

all: 
	@-for %i in ($(DIRS)) do \
		@if EXIST %i (	cd %i & \
	 	$(MAKE) /$(MAKEFLAGS) & \
		cd .. )
      
clean: 
	@-for %i in ($(DIRS)) do \
		@if EXIST %i (	cd %i & \
	 	$(MAKE) /$(MAKEFLAGS) clean & \
		cd .. )
