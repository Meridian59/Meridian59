# Makefile 

TOPDIR=..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = .

BGFS = rmcombat.bgf \
       rmnocombat.bgf \
       rmgcombat.bgf 

all: 
	@-for %i in ($(BGFS)) do \
      @echo copying %i to client & \
   	$(CP) %i $(CLIENTRUNDIR)\resource
	@-for %i in (snowman qormastree "rat king" "qormas gifts" qormas_textures "rat soldier" "small rat") do \
		@if EXIST %i (	cd %i & \
      @echo Make in %i & \
	 	$(MAKE) /$(MAKEFLAGS) & \
		cd .. )

!include $(TOPDIR)\rules.mak
