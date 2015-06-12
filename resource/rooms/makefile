# Makefile for copying rooms

TOPDIR=..\..
!include $(TOPDIR)\common.mak

all:
	@echo Copying rooms to $(BLAKSERVRUNDIR)\rooms
	@echo Copying rooms to $(CLIENTRUNDIR)\resource
	@$(XCP) $(RESOURCEDIR)\rooms\*.roo $(CLIENTRUNDIR)\resource >nul
	@$(XCP) $(RESOURCEDIR)\rooms\*.roo $(BLAKSERVRUNDIR)\rooms >nul


clean :
	