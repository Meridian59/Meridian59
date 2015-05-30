TOPDIR=..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\resource\graphics

.SUFFIXES: .bbg

BGFS = minotaur.bgf minotaurX.bgf

!include $(RESOURCEDIR)\copy.mak

minotaur.bgf:
	$(MAKEBGF) -o minotaur.bgf @minotaur.bbg

clean:
	$(RM) $(CLIENTRUNDIR)\resource\minotaur.bgf
	$(RM) $(CLIENTRUNDIR)\resource\minotaurX.bgf

clean-frames:
	$(RM) *.bmp
