TOPDIR=..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\resource\graphics

.SUFFIXES: .bbg

BGFS = \
      elephant.bgf \
      elephantX.bgf \

all: $(BGFS)
	@-for %i in ($(BGFS)) do \
      @echo copying %i to client & \
   	$(CP) %i $(CLIENTRUNDIR)\resource

clean:
	$(RM) $(CLIENTRUNDIR)\resource\elephant.bgf
   $(RM) $(CLIENTRUNDIR)\resource\elephantX.bgf

clean-frames:
	$(RM) *.bmp