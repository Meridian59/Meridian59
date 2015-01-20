# Makefile 

TOPDIR=..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = .

BGFS = \
	qormastree.bgf \
	qormastreebig.bgf \


all: 
	@-for %i in ($(BGFS)) do \
   @if NOT EXIST $(CLIENTRUNDIR)\resource\%i \
   @echo copying %i to client & \
	$(CP) %i $(CLIENTRUNDIR)\resource

!include $(TOPDIR)\rules.mak
