TOPDIR=..\..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = .
OUTDIR = .

.SUFFIXES: .bbg

BGFS = \
	fieleme.bgf \
	fielemeX.bgf \

all: $(BGFS)
	@-for %i in ($(BGFS)) do \
      @if NOT EXIST $(CLIENTRUNDIR)\resource\%i \
      @echo copying %i to client & \
   	$(CP) %i $(CLIENTRUNDIR)\resource

{$(SOURCEDIR)}.bbg.bgf:
	$(MAKEBGF) -o $(OUTDIR)\$*.bgf @$*.bbg

clean:
	$(RM) $(OUTDIR)\fieleme.bgf
	$(RM) $(CLIENTRUNDIR)\resource\fieleme.bgf
