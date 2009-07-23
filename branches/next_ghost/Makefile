CFLAGS= -I.
CXXFLAGS= -I.
LDFLAGS= -lm

OBJS=cztable.o game/automap.o game/builder.o game/clk_map.o game/dialogs.o \
	game/dump.o game/enemy.o game/engine1.o game/engine2.o game/gamesave.o \
	game/globmap.o game/chargen.o game/interfac.o game/inv.o game/kniha.o \
	game/kouzla.o game/macros.o game/menu.o game/realgame.o game/setup.o \
	game/skeldal.o game/sndandmus.o game/souboje.o game/specproc.o \
	libs/basicobj.o libs/bgraph2a.o libs/bgraph2.o libs/bmouse.o \
	libs/devices.o libs/event.o libs/gui.o libs/inicfg.o libs/memman.o \
	libs/mgifmapmem.o libs/mgifmem.o libs/mgifplaya.o libs/pcx.o \
	libs/strlite.o libs/wav_mem.o libs/zvuk.o
	
UNIXOBJS=unix/system.o

.PHONY: all clean

all: skeldal

clean:
	rm -f skeldal $(OBJS) $(UNIXOBJS)

skeldal : $(OBJS) $(UNIXOBJS)
	$(CC) $(LDFLAGS) -o skeldal $^
