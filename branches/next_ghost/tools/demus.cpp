#include <cstdio>
#include <cstdlib>
#include "tools/memman.h"

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
//	FILE *fr;
	int i, j, k, val;
	long packed, unpacked, psize = 0, usize = 0;
	unsigned char *src = NULL;
	int16_t *dst = NULL;
	File fr;

	for (i = 1; i < argc; i++) {
//		fr = fopen(argv[i], "r");
		fr.open(argv[i]);
//		fread(&h, sizeof(struct MusFileHeader), 1, fr);
		h.channels = fr.readSint16LE();
		h.freq = fr.readSint32LE();
		h.ssize = fr.readSint32LE();
		h.blocks = fr.readSint32LE();
		h.reserved1 = fr.readSint32LE();
		h.reserved2 = fr.readSint32LE();

		for (j = 0; j < 256; j++) {
			h.ampltable[j] = fr.readSint16LE();
		}

		for (j = 0; j < h.blocks; j++) {
			short accum[2] = {0, 0};
//			fread(&packed, sizeof(long), 1, fr);
//			fread(&unpacked, sizeof(long), 1, fr);
			packed = fr.readSint32LE();
			unpacked = fr.readSint32LE();

			if (psize < packed) {
//				free(src);
//				src = malloc(packed);
				delete[] src;
				src = new unsigned char[packed];
				psize = packed;
			}

			if (usize < unpacked) {
//				free(dst);
//				dst = malloc(unpacked);
				delete[] dst;
				dst = new int16_t[packed];
				usize = unpacked;
			}

//			fread(src, psize, 1, fr);
			fr.read(src, packed);

			for (k = 0; k < packed; k++) {
				val = accum[k % 2] + h.ampltable[src[k]];

				accum[k % 2] = val;
				if (src[k] == 0) {
					val -= 32767;
				}
				dst[k] = val;
			}

			fwrite(dst, unpacked, 1, stdout);
		}

//		fclose(fr);
		fr.close();
	}

	delete[] src;
	delete[] dst;
	return 0;
}
