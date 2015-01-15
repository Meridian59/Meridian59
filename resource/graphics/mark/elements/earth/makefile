TOPDIR=..\..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = .
OUTDIR = .

.SUFFIXES: .bbg

BGFS = \
	earthele.bgf \
	eartheleX.bgf \

all: $(BGFS)
	@-for %i in ($(BGFS)) do \
      @if NOT EXIST $(CLIENTRUNDIR)\resource\%i \
      @echo copying %i to client & \
   	$(CP) %i $(CLIENTRUNDIR)\resource

{$(SOURCEDIR)}.bbg.bgf:
	$(MAKEBGF) -o $(OUTDIR)\$*.bgf @$*.bbg

clean:
	$(RM) $(OUTDIR)\earthele.bgf
	$(RM) $(CLIENTRUNDIR)\resource\earthele.bgf
