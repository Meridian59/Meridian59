#
# makefile for blakserv
#

TOPDIR=..

!include $(TOPDIR)\common.mak

BLAKSERVLINKFLAGS = /STACK:0x180000 /map /debug $(LINKWINDOWSFLAGS)

SOURCEDIR = .

LIBS = gdi32.lib user32.lib wsock32.lib winmm.lib comctl32.lib

OBJS =  \
	$(OUTDIR)\main.obj \
	$(OUTDIR)\loadkod.obj \
	$(OUTDIR)\class.obj \
	$(OUTDIR)\message.obj \
	$(OUTDIR)\object.obj \
	$(OUTDIR)\sendmsg.obj \
	$(OUTDIR)\roofile.obj \
	$(OUTDIR)\bufpool.obj \
	$(OUTDIR)\ccode.obj \
	$(OUTDIR)\channel.obj \
	$(OUTDIR)\list.obj \
	$(OUTDIR)\timer.obj \
	$(OUTDIR)\session.obj \
	$(OUTDIR)\loadrsc.obj \
	$(OUTDIR)\blakres.obj \
	$(OUTDIR)\roomdata.obj \
	$(OUTDIR)\commcli.obj \
	$(OUTDIR)\string.obj \
	$(OUTDIR)\async.obj \
	$(OUTDIR)\loadgame.obj \
	$(OUTDIR)\game.obj \
	$(OUTDIR)\term.obj \
	$(OUTDIR)\account.obj \
	$(OUTDIR)\loadacco.obj \
	$(OUTDIR)\saveacco.obj \
	$(OUTDIR)\savestr.obj \
	$(OUTDIR)\loadstr.obj \
	$(OUTDIR)\nameid.obj \
	$(OUTDIR)\time.obj \
	$(OUTDIR)\dllist.obj \
	$(OUTDIR)\trysync.obj \
	$(OUTDIR)\saveall.obj \
	$(OUTDIR)\loadall.obj \
	$(OUTDIR)\synched.obj \
	$(OUTDIR)\motd.obj \
	$(OUTDIR)\admin.obj \
	$(OUTDIR)\garbage.obj \
	$(OUTDIR)\kodbase.obj \
	$(OUTDIR)\savegame.obj \
	$(OUTDIR)\user.obj \
	$(OUTDIR)\system.obj \
	$(OUTDIR)\resync.obj \
	$(OUTDIR)\gamelock.obj \
	$(OUTDIR)\config.obj \
	$(OUTDIR)\apndfile.obj \
	$(OUTDIR)\admincons.obj \
	$(OUTDIR)\builtin.obj \
	$(OUTDIR)\version.obj \
	$(OUTDIR)\systimer.obj \
	$(OUTDIR)\memory.obj \
	$(OUTDIR)\interface.obj \
	$(OUTDIR)\intrlock.obj \
	$(OUTDIR)\chanbuf.obj \
	$(OUTDIR)\debug.obj \
	$(OUTDIR)\saversc.obj \
	$(OUTDIR)\adminfn.obj \
	$(OUTDIR)\table.obj \
	$(OUTDIR)\parsecli.obj \
	$(OUTDIR)\rscload.obj \
	$(OUTDIR)\crc.obj \
	$(OUTDIR)\md5.obj \
	$(OUTDIR)\maintenance.obj \
	$(OUTDIR)\block.obj \
	$(OUTDIR)\stringinthash.obj \
	$(OUTDIR)\intstringhash.obj \
	$(OUTDIR)\files.obj \
	$(OUTDIR)\sprocket.obj \


all : makedirs $(OUTDIR)\blakserv.exe

$(OUTDIR)\rscload.obj : $(TOPDIR)\util\rscload.c
	$(CC) -Fo$(OUTDIR)/ $(CFLAGS) /Fd$(OUTDIR)\vc90.pdb /FpCpch -c $**

$(OUTDIR)\crc.obj : $(TOPDIR)\util\crc.c
	$(CC) -Fo$(OUTDIR)/ $(CFLAGS) /Fd$(OUTDIR)\vc90.pdb /FpCpch -c $**

$(OUTDIR)\md5.obj : $(TOPDIR)\util\md5.c
	$(CC) -Fo$(OUTDIR)/ $(CFLAGS) /Fd$(OUTDIR)\vc90.pdb /Fpmd5pch -c $**

$(OUTDIR)\blakserv.exe: $(OBJS) $(OUTDIR)\blakserv.res
	$(CC) $(CFLAGS) -Fo$(OUTDIR)/ /FpCpch /Fd$(OUTDIR)\vc90.pdb -c $(SOURCEDIR)\version.c
	$(LINK)	$** $(LIBS) -OUT:$@ $(BLAKSERVLINKFLAGS) $(LINKFLAGS)
	$(CP) $@ $(BLAKSERVRUNDIR)


!include $(TOPDIR)\rules.mak
