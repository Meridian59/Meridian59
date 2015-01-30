# Makefile 

TOPDIR=..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = .

BGFS = grd61001.bgf \
       grd61002.bgf \
       grd61003.bgf \
       grd61004.bgf \
       grd61005.bgf \
       grd61006.bgf \
       grd61007.bgf \
       grd61008.bgf \
       grd61009.bgf \
       grd61010.bgf \
       grd61011.bgf \
       grd61012.bgf \
       grd61013.bgf \
       grd61014.bgf \
       grd61015.bgf \
       grd61016.bgf \

all: 
	@-for %i in ($(BGFS)) do \
   @echo copying %i to client & \
	$(CP) %i $(CLIENTRUNDIR)\resource

!include $(TOPDIR)\rules.mak
