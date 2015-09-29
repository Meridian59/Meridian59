# makefile for skyboxen

TOPDIR=..\..
!include $(TOPDIR)\common.mak

DIRS = skya skyb skyc skyd skye redsky
         

CLEANDIRS = skya skyb skyc skyd skye redsky

all:
	@-for %i in ($(DIRS)) do \
		@if EXIST %i (echo Building %i & \
		cd %i & \
	 	$(MAKE) /$(MAKEFLAGS) $(COMMAND) &\
		cd .. )

clean:
	@-for %i in ($(CLEANDIRS)) do @echo Making clean in %i &\
	@if EXIST %i (cd %i & $(MAKE) /$(MAKEFLAGS) clean & cd .. )
