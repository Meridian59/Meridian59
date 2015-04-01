TOPDIR=..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\resource\graphics

.SUFFIXES: .bbg

BGFS = \
      chupa.bgf \
      chupaX.bgf \

all: $(BGFS)
	@-for %i in ($(BGFS)) do \
      @echo copying %i to client & \
   	$(CP) %i $(CLIENTRUNDIR)\resource

clean:
	$(RM) $(CLIENTRUNDIR)\resource\chupa.bgf
   $(RM) $(CLIENTRUNDIR)\resource\chupaX.bgf

clean-frames:
	$(RM) *.bmp