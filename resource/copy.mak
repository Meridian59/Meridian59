# Common rules for graphics (.bgf) makefiles.

all: $(BGFS)
	@-for %i in ($(BGFS)) do \
      @echo copying %i to client \
   	-@$(CP) %i $(CLIENTRUNDIR)\resource >nul
