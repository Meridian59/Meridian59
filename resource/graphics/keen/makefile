# Makefile 

TOPDIR=..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = .

BGFS = rmcombat.bgf \
       rmnocombat.bgf \
       rmgcombat.bgf \
       frogman.bgf \
       frogmanX.bgf \
       srtoken.bgf \
       crate.bgf

DIRS = \
   snowman \
   qormastree \
   "rat king" \
   "qormas gifts" \
   qormas_textures \
   "rat soldier" \
   "small rat" \
   minotaur \
   bunny \
   wolf \
   elephant \
   chupacabra

all:
	@-for %i in ($(BGFS)) do \
   	@$(CP) %i $(CLIENTRUNDIR)\resource >nul
	@-for %i in ($(DIRS)) do \
		@if EXIST %i (	cd %i & \
      @echo Make in %i & \
	 	$(MAKE) /$(MAKEFLAGS) $(COMMAND) &\
		cd .. )

clean:
	@-for %i in ($(DIRS)) do \
		@echo Making clean in %i & \
		@if EXIST %i ( cd %i & \
	 	$(MAKE) /$(MAKEFLAGS) clean &\
		cd .. )
