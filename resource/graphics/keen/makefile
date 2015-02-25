# Makefile 

TOPDIR=..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = .

BGFS = rmcombat.bgf \
       rmnocombat.bgf \
       rmgcombat.bgf \
       frogman.bgf \
       frogmanX.bgf \
       srtoken.bgf

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
   chupacabra \
   
all: 
	@-for %i in ($(BGFS)) do \
      @echo copying %i to client & \
   	$(CP) %i $(CLIENTRUNDIR)\resource
	@-for %i in ($(DIRS)) do \
		@if EXIST %i (	cd %i & \
      @echo Make in %i & \
	 	$(MAKE) /$(MAKEFLAGS) & \
		cd .. )

!include $(TOPDIR)\rules.mak
