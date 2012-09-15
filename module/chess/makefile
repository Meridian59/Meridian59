# Makefile for chess DLL

TOPDIR=..\..
!include $(TOPDIR)\common.mak

MODULE = chess

!include $(MODULEDIR)\module.mak

$(OUTDIR)\chess.dll: \
          $(OUTDIR)\chess.obj \
          $(OUTDIR)\board.obj \
          $(OUTDIR)\cmove.obj \
          $(OUTDIR)\chess.res

!include $(TOPDIR)\rules.mak
