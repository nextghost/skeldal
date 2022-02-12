/*
 *  This file is part of Skeldal project
 * 
 *  Skeldal is free software: you can redistribute 
 *  it and/or modify it under the terms of the GNU General Public 
 *  License as published by the Free Software Foundation, either 
 *  version 3 of the License, or (at your option) any later version.
 *
 *  OpenSkeldal is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Skeldal.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  --------------------
 *
 *  Project home: https://sourceforge.net/projects/skeldal/
 *  
 *  Last commit made by: $Id$
 */
#include "libs/bgraph.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <inttypes.h>
#include "libs/memman.h"
#include "libs/mgifmem.h"
#include "libs/sound.h"
#include "libs/system.h"
#include "libs/strlite.h"

static void *sound;

void loadMgifHeader(MGIF_HEADER_T &header, ReadStream &stream) {
	int i;

	stream.read(header.sign, 4);
	stream.read(header.year, 2);
	header.eof = stream.readSint8();
	header.ver = stream.readUint16LE();
	header.frames = stream.readSint32LE();
	header.snd_chans = stream.readUint16LE();
	header.snd_freq = stream.readSint32LE();

	for (i = 0; i < 256; i++) {
		header.ampl_table[i] = stream.readSint16LE();
	}

	for (i = 0; i < 32; i++) {
		header.reserved[i] = stream.readSint16LE();
	}
}

static void PlayMGFFile(ReadStream &stream, int ypos, char full) {
	unsigned state;
	MGIFReader video(stream, 320, 180);

	sound = PrepareVideoSound(22050, 256 * 1024);

	while (state = video.decodeFrame()) {
		if (state & FLAG_AUDIO) {
			while (!LoadNextAudioFrame(sound, video.getAudio(), video.getAudioSize())) {
				Timer_Sleep(5);
			}
		}

		if (state & FLAG_VIDEO) {
			renderer->videoBlit(ypos, video, video.palette());
			showview(0, ypos, 0, 360);
		}

		if (!Input_Kbhit()) {
			Sound_MixBack(0);
		} else {
			Input_ReadKey();
			break;
		}
	}

	DoneVideoSound(sound);
}

void play_animation(char *filename, char mode, int posy, char sound) {
	File mgf(filename);

	Sound_ChangeMusic(NULL);

	if (!mgf.isOpen()) {
		fprintf(stderr, "Could not open file %s\n", filename);
		return;
	}

	PlayMGFFile(mgf, posy, mode & 0x80);
}

void set_title_list(const StringList *titles)
  {

  }
