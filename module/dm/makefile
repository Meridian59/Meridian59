# Makefile for downloadable client DLLs

TOPDIR=..\..
!include $(TOPDIR)\common.mak

MODULE = dm

!include $(MODULEDIR)\module.mak

$(OUTDIR)\dm.dll: \
           $(OUTDIR)\dm.obj \
           $(OUTDIR)\command.obj \
           $(OUTDIR)\gchannel.obj \
           $(OUTDIR)\qeditor.obj \
           $(OUTDIR)\dm.res

!include $(TOPDIR)\rules.mak
