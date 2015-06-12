TOPDIR=..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\resource\graphics

.SUFFIXES: .bbg

BGFS = wolf.bgf wolfX.bgf

!include $(RESOURCEDIR)\copy.mak

clean:
	@$(RM) $(CLIENTRUNDIR)\resource\wolf.bgf 2>nul
	@$(RM) $(CLIENTRUNDIR)\resource\wolfX.bgf 2>nul

clean-frames:
	$(RM) *.bmp
