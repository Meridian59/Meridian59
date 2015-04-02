#
# Makefile piece included by each kod directory makefile. The top level
# makefile reproduces this with some commands to run after recursing.
#

.SUFFIXES : .kod .lkod

BCFLAGS = -d -I $(KODINCLUDEDIR) -K $(KODDIR)\kodbase.txt

.lkod.kod::
.kod.bof::
	$(BC) $(BCFLAGS) $<
	@for %i in ($<) do @$(KODDIR)\bin\instbofrsc $(TOPDIR) $(BLAKSERVRUNDIR) %i

all : $(BOFS) $(BOFS2) $(BOFS3) $(BOFS4) $(BOFS5) $(BOFS6) $(BOFS7) $(BOFS8)
	@for %i in ($(BOFS:.bof=) $(BOFS2:.bof=) $(BOFS3:.bof=.) $(BOFS4:.bof=.) $(BOFS5:.bof=.) $(BOFS6:.bof=.) $(BOFS7:.bof=.) $(BOFS8:.bof=.)) do @if EXIST %i (echo Building %i & cd %i & $(MAKE) /$(MAKEFLAGS) TOPDIR=..\$(TOPDIR) & cd ..)


$(BOFS) $(BOFS2) $(BOFS3) $(BOFS4) $(BOFS5) $(BOFS6) $(BOFS7) $(BOFS8): $(DEPEND)

clean :
	@-$(RM) *.bof *.rsc kodbase.txt >nul 2>&1
	@-for %i in ($(BOFS:.bof=.)) do @if EXIST %i (cd %i & $(MAKE) /$(MAKEFLAGS) TOPDIR=..\$(TOPDIR) clean & cd .. )
