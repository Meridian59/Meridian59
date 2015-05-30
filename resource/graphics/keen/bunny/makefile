TOPDIR=..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = $(TOPDIR)\resource\graphics

.SUFFIXES: .bbg

BGFS = bunny2.bgf bunny2X.bgf

!include $(RESOURCEDIR)\copy.mak

clean:
	$(RM) $(CLIENTRUNDIR)\resource\bunny2.bgf
	$(RM) $(CLIENTRUNDIR)\resource\bunny2X.bgf

clean-frames:
	$(RM) *.bmp
