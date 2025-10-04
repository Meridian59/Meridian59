# Makefile for downloadable client DLLs

TOPDIR=..
!include $(TOPDIR)\common.mak

MODULES = admin char mailnews merintr dm chess

all: $(MODULES)

clean:
    @$(MAKE) -$(MAKEFLAGS) $(MODULES) TARGET=$@

# This structure correctly propagates any failure from a subbuild to the top level
$(MODULES): FORCE
    @if exist $@\ cd $@ && $(MAKE) -$(MAKEFLAGS) $(TARGET)

FORCE:
