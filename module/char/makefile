# Makefile for char DLL

TOPDIR=..\..
!include $(TOPDIR)\common.mak

MODULE = char

!include $(MODULEDIR)\module.mak

$(OUTDIR)\char.dll: \
        $(OUTDIR)\charpick.obj \
        $(OUTDIR)\charface.obj \
        $(OUTDIR)\char.obj \
        $(OUTDIR)\charmake.obj \
        $(OUTDIR)\charstat.obj \
        $(OUTDIR)\charname.obj \
        $(OUTDIR)\charspel.obj \
        $(OUTDIR)\charskil.obj \
        $(OUTDIR)\char.res

!include $(TOPDIR)\rules.mak
