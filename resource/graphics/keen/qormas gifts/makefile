# Makefile 

TOPDIR=..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = .

BGFS = gift1.bgf \
       gift2.bgf \
       gift3.bgf \
       gift4.bgf   

all: 
	@-for %i in ($(BGFS)) do \
   @echo copying %i to client & \
	$(CP) %i $(CLIENTRUNDIR)\resource

!include $(TOPDIR)\rules.mak
