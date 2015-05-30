TOPDIR=..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\resource\graphics

.SUFFIXES: .bbg

BGFS = elephant.bgf elephantX.bgf

!include $(RESOURCEDIR)\copy.mak

clean:
	$(RM) $(CLIENTRUNDIR)\resource\elephant.bgf
	$(RM) $(CLIENTRUNDIR)\resource\elephantX.bgf

clean-frames:
	$(RM) *.bmp
