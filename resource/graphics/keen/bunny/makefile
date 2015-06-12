TOPDIR=..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\resource\graphics

.SUFFIXES: .bbg

BGFS = bunny2.bgf bunny2X.bgf

!include $(RESOURCEDIR)\copy.mak

clean:
	@$(RM) $(CLIENTRUNDIR)\resource\bunny2.bgf 2>nul
	@$(RM) $(CLIENTRUNDIR)\resource\bunny2X.bgf 2>nul

clean-frames:
	$(RM) *.bmp
