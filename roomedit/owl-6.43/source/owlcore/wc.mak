.OPTIMIZE


OWLBUILDROOT=c:\mlib\owl
WCROOT=d:\wc
CC=$(WCROOT)\bin\wpp386
LINK=$(WCROOT)\bin\wlink
LIB=$(WCROOT)\bin\wlib
RC=$(WCROOT)\bin\wrc
CCFL=$(CC) $(CFLAGS)

OWLVER=618
DLL_TARGET=owl$(OWLVER)w.DLL

version=debugging          # debugging version

OBJDIR=$(OWLBUILDROOT)\OBJ\OWLW\


msg_production = linking production version ...
msg_debugging = linking debug version ...

cpp_includes = -i$(WCROOT)\h -i$(WCROOT)\h\stlport -i$(WCROOT)\h\stlport\cstd -i$(WCROOT)\h\nt -i$(OWLBUILDROOT)\include
common_cpp_options =/mf /w3 /zp8 /6 /fp6 /ei /xr /xs /bt=NT /fh=$(OBJDIR)\owl.pch /hd $(cpp_includes)
compile_options_production = /oneatx /oh /oi+
compile_options_debugging = /od /d2s
CFLAGS = $(common_cpp_options) $(compile_options_$(version))

link_options_common =
link_options_production =
link_options_debugging = debug all
LFLAGS=$(link_options_common) $(link_options_$(version))


!ifdef __LOADDLL__
!  loaddll $(CC)    $(WCROOT)\bin\wppd386
!  loaddll $(LINK)  $(WCROOT)\bin\wlink
!  loaddll $(LIB)   $(WCROOT)\bin\wlibd
!endif



all : owl61w.dll owlwfi.lib

TMPLT = &
 $(PF)animctrl$(EXT) &
 $(PF)appdict$(EXT)


PF=$(OBJDIR)
EXT=.obj
OBJS=$+ $(PF)$(TMPLT).obj $-
PF=./
EXT=.cpp
SRC=$+ $(TMPLT) $-


OBJS_LIB =

$(DLL_TARGET) : $(OBJS)
    echo $(msg_$(version))
    $(LINK) $(LFLAGS) @$^*
    $(RC) /q /bt=windows owl.res $(DLL_TARGET)


owlwfi.lib : $(OBJS_LIB)


owl.res : owl.rc .AUTODEPEND
   $(RC) /ad /q /bt=windows /r $[@ $^@

.cpp.obj: .AUTODEPEND
   $(CC) $[* $(CFLAGS)

#$(OBJDIR)animctrl.obj : animctrl.cpp
# $(CCFL) $[*
#
#$(OBJDIR)appdict.obj : appdict.cpp .AUTODEPEND
# $(CCFL) $[*
