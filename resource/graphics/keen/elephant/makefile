TOPDIR=..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\resource\graphics

.SUFFIXES: .bbg

BGFS = elephant.bgf elephantX.bgf

!include $(RESOURCEDIR)\copy.mak

clean:
	@$(RM) $(CLIENTRUNDIR)\resource\elephant.bgf 2>nul
	@$(RM) $(CLIENTRUNDIR)\resource\elephantX.bgf 2>nul

clean-frames:
	$(RM) *.bmp
