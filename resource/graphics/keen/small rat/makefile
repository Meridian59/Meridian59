# Makefile 

TOPDIR=..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = .

BGFS = \
	ratsm.bgf \
	ratsmX.bgf \

all: 
	@-for %i in ($(BGFS)) do \
   @if NOT EXIST $(CLIENTRUNDIR)\resource\%i \
   @echo copying %i to client & \
	$(CP) %i $(CLIENTRUNDIR)\resource

!include $(TOPDIR)\rules.mak
