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

#include <cstring>
#include <cassert>
#include <inttypes.h>
#include <SDL/SDL.h>
#include "libs/system.h"

typedef struct {
	int16_t *buff;
	unsigned size, readoff, writeoff;
	SDL_AudioSpec *hw;
} ringbuffer_t;

static void Sound_Callback(void *userdata, Uint8 *stream, int size) {
	ringbuffer_t *buff = (ringbuffer_t*)userdata;
	int len;

	len = (buff->writeoff < buff->readoff ? buff->size - buff->readoff :
		buff->writeoff - buff->readoff) * 2;
	len = size < len ? size : len;

	memcpy(stream, buff->buff + buff->readoff, len);

	stream += len;
	size -= len;
	buff->readoff += len / 2;
	buff->readoff = buff->readoff >= buff->size ? 0 : buff->readoff;

	len = size > 0 ? (buff->writeoff - buff->readoff) * 2 : 0;
	len = size < len ? size : len;

	memcpy(stream, buff->buff + buff->readoff, len);
	stream += len;
	size -= len;
	buff->readoff += len / 2;

	if (size > 0) {
		fprintf(stderr, "Sound buffer underrun (%d bytes)\n", size);
		memset(stream, buff->hw->silence, size);
	}
}

void *PrepareVideoSound(int mixfreq, int buffsize) {
	ringbuffer_t *ret = (ringbuffer_t*)malloc(sizeof(ringbuffer_t));
	SDL_AudioSpec want, *got = (SDL_AudioSpec*)malloc(sizeof(SDL_AudioSpec));

	// stupid sound buffer video timing, we can't use SDL mixer here...
	Sound_StopMixing();

	want.freq = mixfreq;
	want.format = AUDIO_S16SYS;
	want.channels = 2;
	want.samples = 1024;
	want.callback = Sound_Callback;
	want.userdata = ret;

	// start low level audio instead
	if (SDL_OpenAudio(&want, got) < 0) {
		assert(0 && "Failed to open audio");
	}

	assert(2 * got->size < buffsize && "Sound buffer too small");


	ret->buff = (int16_t*)malloc(buffsize);
	ret->size = buffsize / 2;
	ret->readoff = 0;
	ret->writeoff = got->size / 2;	// prevent initial underrun
	ret->hw = got;

	memset(ret->buff, got->silence, buffsize);
	SDL_PauseAudio(0);

	return ret;
}

int LoadNextAudioFrame(void *ptr, const int16_t *data, unsigned length) {
	ringbuffer_t *buff = (ringbuffer_t*)ptr;
	int len, i;

	SDL_LockAudio();
	len = buff->readoff <= buff->writeoff ? buff->size - buff->writeoff + buff->readoff - 1 : buff->readoff - buff->writeoff - 1;

	if (len < length) {
		SDL_UnlockAudio();
		return 0;
	}

	len = buff->size - buff->writeoff < length ? buff->size - buff->writeoff : length;

	memcpy(buff->buff + buff->writeoff, data, len * sizeof(int16_t));
	memcpy(buff->buff, data + len, (length - len) * sizeof(int16_t));
	buff->writeoff = (buff->writeoff + length) % buff->size;

	SDL_UnlockAudio();
	return 1;
}

void DoneVideoSound(void *ptr) {
	ringbuffer_t *buff = (ringbuffer_t*)ptr;

	SDL_CloseAudio();
	free(buff->buff);
	free(buff->hw);
	free(buff);

	Sound_StartMixing();
}
