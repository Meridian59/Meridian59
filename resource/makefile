# makefile for bitmaps

TOPDIR=..
!include $(TOPDIR)\common.mak

DIRS = graphics \
       #textures

CLEANDIRS = graphics \
            #textures
       
all:
	@-for %i in ($(DIRS)) do \
		@if EXIST %i (echo Building %i & \
		cd %i & \
	 	$(MAKE) /$(MAKEFLAGS) & \
		cd .. )

clean :
	@-for %i in ($(CLEANDIRS)) do @if EXIST %i (cd %i & $(MAKE) /$(MAKEFLAGS) clean & cd .. )
