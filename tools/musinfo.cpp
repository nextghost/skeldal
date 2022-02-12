#include <cstdio>
#include <cstdlib>
#include <getopt.h>
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
	int c;
	int encType = 0;
	File fr;

	while ((c = getopt (argc, argv, "mo")) != -1) {
		switch (c) {
        	case 'm': /* mp3 (default) */
				encType = 0;
				break;
        	case 'o': /* ogg */
				encType = 1;
				break;
		}	
	}

	for (i = optind; i < argc; i++) {
		fr.open(argv[i]);
		h.channels = fr.readSint16LE();
		h.freq = fr.readSint32LE();
		h.ssize = fr.readSint32LE();
		h.blocks = fr.readSint32LE();

#ifdef WORDS_BIGENDIAN
		switch (encType) {
			case 0:
				printf("-r -s %5.5g --bitwidth 16 --signed --big-endian -m %c\n", h.freq / 1000.0f, h.channels == 1 ? 'm' : 'j');
				break;
			case 1:
				printf("-r -s %li -B 16 --raw-endianness 1 -C %i\n", h.freq, h.channels);
				break;
		}
#else
		switch (encType) {
			case 0:
				printf("-r -s %5.5g --bitwidth 16 --signed --little-endian -m %c\n", h.freq / 1000.0f, h.channels == 1 ? 'm' : 'j');
				break;
			case 1:
				printf("-r -R %li -B 16 --raw-endianness 0 -C %i\n", h.freq, h.channels);
				break;
		}
#endif

		fr.close();
	}

	return 0;
}
