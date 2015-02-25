# Makefile for stats DLL

TOPDIR=..\..
!include $(TOPDIR)\common.mak

MODULE = stats

!include $(MODULEDIR)\module.mak

$(OUTDIR)\stats.dll: \
        $(OUTDIR)\stats.obj \
        $(OUTDIR)\statsmake.obj \
        $(OUTDIR)\statsstat.obj \
        $(OUTDIR)\stats.res

!include $(TOPDIR)\rules.mak
