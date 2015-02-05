TOPDIR=..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\resource\graphics

.SUFFIXES: .bbg

BGFS = \
      wolf.bgf \
      wolfX.bgf \
      
all: $(BGFS)
	@-for %i in ($(BGFS)) do \
      @echo copying %i to client & \
   	$(CP) %i $(CLIENTRUNDIR)\resource

clean:
	$(RM) $(CLIENTRUNDIR)\resource\wolf.bgf
   $(RM) $(CLIENTRUNDIR)\resource\wolfX.bgf

clean-frames:
	$(RM) *.bmp