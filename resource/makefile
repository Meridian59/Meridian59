# makefile for bitmaps

TOPDIR=..
!include $(TOPDIR)\common.mak

all:
	@-for %i in (graphics textures) do \
		@if EXIST %i (echo Building %i & \
		cd %i & \
	 	$(MAKE) /$(MAKEFLAGS) & \
		cd .. )

clean :
	@-for %i in (graphics textures) do @if EXIST %i (cd %i & $(MAKE) /$(MAKEFLAGS) clean & cd .. )
