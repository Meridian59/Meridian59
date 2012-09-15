#
# makefile piece included by each kod directory makefile
#

.SUFFIXES : .kod

BCFLAGS = -d -I $(KODINCLUDEDIR) -K $(KODDIR)\kodbase.txt

.kod.bof::
	$(BC) $(BCFLAGS) $<
	@for %i in ($<) do @$(KODDIR)\bin\instbof $(BLAKSERVRUNDIR) %i
	@for %i in ($<) do @$(KODDIR)\bin\instrsc $(TOPDIR) %i

all : $(BOFS) $(BOFS2) $(BOFS3) $(BOFS4) $(BOFS5) $(BOFS6) $(BOFS7) $(BOFS8)
	@for %i in ($(BOFS:.bof=) $(BOFS2:.bof=) $(BOFS3:.bof=.) $(BOFS4:.bof=.) $(BOFS5:.bof=.) $(BOFS6:.bof=.) $(BOFS7:.bof=.) $(BOFS8:.bof=.)) do @if EXIST %i (echo Building %i & cd %i & $(MAKE) /$(MAKEFLAGS) TOPDIR=..\$(TOPDIR) & cd ..)


$(BOFS) $(BOFS2) $(BOFS3) $(BOFS4) $(BOFS5) $(BOFS6) $(BOFS7) $(BOFS8): $(DEPEND)

clean :
	@-$(RM) *.bof *.rsc kodbase.txt
	@-for %i in ($(BOFS:.bof=.)) do @if EXIST %i (cd %i & $(MAKE) /$(MAKEFLAGS) TOPDIR=..\$(TOPDIR) clean & cd .. )
