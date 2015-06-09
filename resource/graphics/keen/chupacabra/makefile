TOPDIR=..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\resource\graphics

.SUFFIXES: .bbg

BGFS = chupa.bgf chupaX.bgf

!include $(RESOURCEDIR)\copy.mak

clean:
	@$(RM) $(CLIENTRUNDIR)\resource\chupa.bgf 2>nul
	@$(RM) $(CLIENTRUNDIR)\resource\chupaX.bgf 2>nul

clean-frames:
	$(RM) *.bmp
