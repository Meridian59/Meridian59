# Makefile 

TOPDIR=..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = .

BGFS = ratsold.bgf \
       ratsoldX.bgf \

all: 
	@-for %i in ($(BGFS)) do \
   @echo copying %i to client & \
	$(CP) %i $(CLIENTRUNDIR)\resource

!include $(TOPDIR)\rules.mak
