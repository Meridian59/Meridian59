TOPDIR=..\..\..\..\..
!include $(TOPDIR)\common.mak

SOURCEDIR = .
OUTDIR = .

.SUFFIXES: .bbg

BGFS = iceleme.bgf \
       icelemeX.bgf \

all: $(BGFS)
	@-for %i in ($(BGFS)) do \
      @echo copying %i to client & \
   	$(CP) %i $(CLIENTRUNDIR)\resource

{$(SOURCEDIR)}.bbg.bgf:
	$(MAKEBGF) -o $(OUTDIR)\$*.bgf @$*.bbg
   
clean:
	$(RM) $(OUTDIR)\iceleme.bgf
	$(RM) $(CLIENTRUNDIR)\resource\iceleme.bgf
