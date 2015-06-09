#
# Makefile for compiling the Blakod. This is the 'main' makefile, so kod.mak
# is reproduced here with some extra commands to run after the makefile
# recurse is done (notably creating the client rsb file). We need to do this
# here instead of the root directory makefile in case anyone builds the blakod
# from this directory.
#

TOPDIR=..
!include $(TOPDIR)\common.mak

BOFS = util.bof object.bof

.SUFFIXES : .kod

BCFLAGS = -d -I $(KODINCLUDEDIR) -K $(KODDIR)\kodbase.txt

.kod.bof::
	$(BC) $(BCFLAGS) $<
	@for %i in ($<) do @$(KODDIR)\bin\instbofrsc $(TOPDIR) $(BLAKSERVRUNDIR) %i

all : $(BOFS) $(BOFS2) $(BOFS3) $(BOFS4) $(BOFS5) $(BOFS6) $(BOFS7) $(BOFS8)
	@for %i in ($(BOFS:.bof=) $(BOFS2:.bof=) $(BOFS3:.bof=.) $(BOFS4:.bof=.) $(BOFS5:.bof=.) $(BOFS6:.bof=.) $(BOFS7:.bof=.) $(BOFS8:.bof=.)) do @if EXIST %i (echo Building %i & cd %i & $(MAKE) /$(MAKEFLAGS) TOPDIR=..\$(TOPDIR) & cd ..)
	@echo Copying kodbase.txt and kod include files
	-@$(CP) $(KODDIR)\kodbase.txt $(BLAKSERVRUNDIR) 2>&1
	-@$(CP) $(KODDIR)\include\*.khd $(BLAKSERVRUNDIR) 2>&1
	@echo Copying .rsc and creating client .rsb file
	@-$(RM) $(CLIENTRUNDIR)\resource\*.rsc >nul 2>&1
	@-$(RM) $(CLIENTRUNDIR)\resource\*.rsb >nul 2>&1
	-@$(RSCMERGE) $(CLIENTRUNDIR)\resource\rsc0000.rsb $(BLAKSERVRUNDIR)\rsc\*.rsc


$(BOFS) $(BOFS2) $(BOFS3) $(BOFS4) $(BOFS5) $(BOFS6) $(BOFS7) $(BOFS8): $(DEPEND)

clean :
	@-$(RM) *.bof *.rsc kodbase.txt >nul 2>&1
	@-for %i in ($(BOFS:.bof=.)) do @if EXIST %i (cd %i & $(MAKE) /$(MAKEFLAGS) TOPDIR=..\$(TOPDIR) clean & cd .. )
