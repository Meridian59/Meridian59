#
# makefile for blakserv protocol dll sprocket
#

TOPDIR=..

DLL = 1

!include $(TOPDIR)\common.mak

INCLUDE = $(INCLUDE);$(BLAKSERVDIR)

CFLAGS = $(CFLAGS) -LD

SOURCEDIR = .

LIBS =

OBJS =  \
	$(OUTDIR)\sprocket.obj \


all : makedirs $(OUTDIR)\sprocket.dll

$(OUTDIR)\sprocket.dll: $(OBJS)
	$(LINK) $** -OUT:$@ -subsystem:windows  $(LINKFLAGS) -def:sprocket.def
	$(CP) $@ $(BLAKSERVRUNDIR)\loadkod

!include $(TOPDIR)\rules.mak
