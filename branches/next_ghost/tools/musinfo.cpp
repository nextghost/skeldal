#include <cstdio>
#include <cstdlib>
#include "tools/memman.h"
#include "config.h"

struct MusFileHeader {
	int channels;
	long  freq;
	long  ssize;
	long  blocks;
	long  reserved1;
	long  reserved2;
	int16_t ampltable[256];
};

int main(int argc, char **argv) {
	struct MusFileHeader h;
	int i;
	File fr;

	for (i = 1; i < argc; i++) {
		fr.open(argv[i]);
		h.channels = fr.readSint16LE();
		h.freq = fr.readSint32LE();
		h.ssize = fr.readSint32LE();
		h.blocks = fr.readSint32LE();

#ifdef WORDS_BIGENDIAN
		printf("-r -s %5.5g --bitwidth 16 --signed --big-endian -m %c\n", h.freq / 1000.0f, h.channels == 1 ? 'm' : 'j');
#else
		printf("-r -s %5.5g --bitwidth 16 --signed --little-endian -m %c\n", h.freq / 1000.0f, h.channels == 1 ? 'm' : 'j');
#endif

		fr.close();
	}

	return 0;
}
