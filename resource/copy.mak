# Common rules for graphics (.bgf) makefiles.

all:
	@-for %i in ($(BGFS)) do @$(CP) %i $(CLIENTRUNDIR)\resource >nul
