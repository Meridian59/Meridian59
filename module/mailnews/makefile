# Makefile for mail and news DLL

TOPDIR=..\..
!include $(TOPDIR)\common.mak

MODULE = mailnews

!include $(MODULEDIR)\module.mak

$(OUTDIR)\mailnews.dll: \
        $(OUTDIR)\mailnews.obj \
        $(OUTDIR)\mailread.obj \
        $(OUTDIR)\mailsend.obj \
        $(OUTDIR)\mailfile.obj \
	$(OUTDIR)\newsread.obj \
        $(OUTDIR)\newssend.obj \
        $(OUTDIR)\mailnews.res

!include $(TOPDIR)\rules.mak
