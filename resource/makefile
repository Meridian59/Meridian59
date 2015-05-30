# makefile for bitmaps

TOPDIR=..
!include $(TOPDIR)\common.mak

DIRS = graphics \
       rooms
       #textures

CLEANDIRS = graphics \
            #textures
       
all:
	@-for %i in ($(DIRS)) do \
		@if EXIST %i (echo Building %i & \
		cd %i & \
	 	$(MAKE) /$(MAKEFLAGS) $(COMMAND) &\
		cd .. )

clean:
   @set COMMAND=clean
	@-for %i in ($(CLEANDIRS)) do @if EXIST %i (cd %i & $(MAKE) /$(MAKEFLAGS) $(COMMAND) & cd .. )
