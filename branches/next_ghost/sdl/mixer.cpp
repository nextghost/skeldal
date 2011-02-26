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

#include <cassert>
#include <cstdio>
#include <cstring>
#include <SDL/SDL_mixer.h>
#include "libs/sound.h"
#include "libs/system.h"

#define CHANNELS 32
#define SND_EFF_MAXVOL 32000
#define GFX_MAXVOL 255
#define MUSIC_MAXVOL 255

typedef struct {
	int lvolume, rvolume, loopstart, loopend;
	Uint8 *data;
	unsigned size;
	Mix_Chunk chunk;
} channel_t;

static int freq = 22050;
static channel_t chans[CHANNELS] = {0};
static int swapped = 0;
static int play_music = 0, active = 0;
static Mix_Music *cur_music = NULL, *next_music = NULL;

// empty string terminated
static const char *music_ext[] = {".ogg", ".mp3", ""};

void Sound_SetMixer(int mix_dev, int mix_freq, ...) {
	freq = mix_freq;
}

void Sound_Callback(int channel) {
	if (!chans[channel].data) {
		return;
	}

	if (chans[channel].loopstart < chans[channel].loopend) {
		// channel is looping
		chans[channel].chunk.abuf = chans[channel].data + chans[channel].loopstart;
		chans[channel].chunk.alen = chans[channel].loopend - chans[channel].loopstart;
		Mix_PlayChannel(channel, &chans[channel].chunk, 0);
	} else if (chans[channel].loopstart < chans[channel].size) {
		// loop leadout
		chans[channel].chunk.abuf = chans[channel].data + chans[channel].loopstart;
		chans[channel].chunk.alen = chans[channel].size - chans[channel].loopstart;
		chans[channel].loopstart = chans[channel].loopend = chans[channel].size;
		Mix_PlayChannel(channel, &chans[channel].chunk, 0);
	}
}

void Sound_StartMixing(void) {
	if (Mix_OpenAudio(freq, AUDIO_S16SYS, 2, 1024) < 0) {
		assert(0 && "Failed to start mixer");
	}

	Mix_AllocateChannels(CHANNELS);
	Mix_ChannelFinished(Sound_Callback);
	active = 1;
}

void Sound_StopMixing(void) {
	int i;

	active = 0;
	play_music = 0;

	// clean up resampled buffers
	for (i = 0; i < CHANNELS; i++) {
		Sound_Mute(i);
	}

	// close music files
	Mix_HaltMusic();
	if (cur_music) {
		Mix_FreeMusic(cur_music);
		cur_music = NULL;
	}

	if (next_music) {
		Mix_FreeMusic(next_music);
		next_music = NULL;
	}

	// shut down subsystem
	Mix_ChannelFinished(NULL);
	Mix_CloseAudio();
}

// FIXME: resample when loading the sound file
void Sound_PlaySample(int channel, const void *sample, long size, long lstart, long lend, long sfreq, int type) {
	double coef;
	SDL_AudioCVT cvt;

	assert(active && "Sound system not initialized");
	assert(channel >= 0 && channel < CHANNELS && "Invalid channel");

	// clean up resampled buffers
	Sound_Mute(channel);

	if (!SDL_BuildAudioCVT(&cvt, type == 1 ? AUDIO_U8 : AUDIO_S16LSB, 1, sfreq, AUDIO_S16SYS, 2, freq)) {
		fprintf(stderr, "Resample from %ldHz/%d bits to %dHz/16 bits failed\n", sfreq, type == 1 ? 8 : 16, freq);
		return;
	}

	cvt.buf = (Uint8*)malloc(size * cvt.len_mult);
	cvt.len = size;
	memcpy(cvt.buf, sample, size);

	SDL_ConvertAudio(&cvt);

	chans[channel].data = cvt.buf;
	chans[channel].size = cvt.len_cvt;
	coef = (double)chans[channel].size / size;
	chans[channel].loopstart = lstart * coef;
	chans[channel].loopend = lend * coef;

	chans[channel].chunk.allocated = 1;
	chans[channel].chunk.abuf = chans[channel].data;
	chans[channel].chunk.alen = chans[channel].loopend;
	chans[channel].chunk.volume = 128;

	Mix_PlayChannel(channel, &chans[channel].chunk, 0);
}

// stop channel and free resampled buffers
void Sound_Mute(int channel) {
	// stop the channel
	SDL_LockAudio();
	chans[channel].loopstart = chans[channel].size;
	Mix_HaltChannel(channel);

	// free resampled buffer
	if (chans[channel].data) {
		free(chans[channel].data);
		chans[channel].data = NULL;
	}

	SDL_UnlockAudio();
}

void Sound_BreakExt(int channel) {
	SDL_LockAudio();
	chans[channel].loopstart = chans[channel].loopend;
	SDL_UnlockAudio();
}

void Sound_BreakLoop(int channel) {
	SDL_LockAudio();
	chans[channel].loopstart = chans[channel].size;
	chans[channel].loopend = chans[channel].size;
	SDL_UnlockAudio();
}

char Sound_GetChannelState(int channel) {
	return chans[channel].data != NULL;
}

void Sound_SetVolume(int channel, int left, int right) {
	chans[channel].lvolume = left;
	chans[channel].rvolume = right;

	left = (left * 255) / SND_EFF_MAXVOL;
	right = (right * 255) / SND_EFF_MAXVOL;
	Mix_SetPanning(channel, left, right);
}

void Sound_GetVolume(int channel, int *left, int *right) {
	*left = chans[channel].lvolume;
	*right = chans[channel].rvolume;
}

char Sound_IsActive(void) {
	int f, ch;
	Uint16 form;

	return Mix_QuerySpec(&f, &form, &ch) != 0;
}

char Sound_SetEffect(int filter, int data) {
	switch (filter) {
	case SND_PING:
		return 1;

	case SND_SWAP:
		if (Mix_SetReverseStereo(MIX_CHANNEL_POST, data & 1)) {
			swapped = data & 1;
			return 1;
		}

		return 0;

	case SND_GFX:
		Mix_Volume(-1, (data * MIX_MAX_VOLUME) / GFX_MAXVOL);
		return 1;

	case SND_MUSIC:
		Mix_VolumeMusic((data * MIX_MAX_VOLUME) / MUSIC_MAXVOL);
		return 1;

	default:
		return 0;
	}
}

int Sound_GetEffect(int filter) {
	switch (filter) {
	case SND_PING:
		return 1;

	case SND_SWAP:
		return swapped;

	case SND_GFX:
		return (Mix_Volume(-1, -1) * GFX_MAXVOL) / MIX_MAX_VOLUME;

	case SND_MUSIC:
		return (Mix_VolumeMusic(-1) * MUSIC_MAXVOL) / MIX_MAX_VOLUME;

	default:
		return 0;
	}
}

char Sound_CheckEffect(int filter) {
	switch (filter) {
	case SND_PING:
	case SND_SWAP:
	case SND_GFX:
	case SND_MUSIC:
		return 1;

	default:
		return 0;
	}
}

// load SDL music descriptor
static Mix_Music *Sound_LoadFile(const char *file) {
	Mix_Music *ret;
	char *tmp, *buf;

	if (!file) {
		return NULL;
	}

	buf = (char*)malloc((strlen(file) + 5) * sizeof(char));
	strcpy(buf, file);
	tmp = strrchr(buf, '.');
	tmp = tmp ? tmp : buf + strlen(buf);

	// try to load all possible music formats until success or failure
	for (int i = 0; music_ext[i][0]; i++) {
		strcpy(tmp, music_ext[i]);
		ret = Mix_LoadMUS(buf);

		if (ret) {
			break;
		}
	}

	if (!ret) {
		tmp[0] = '\0';
		fprintf(stderr, "Could not load music file %s...\n", buf);
	}

	free(buf);
	return ret;
}

// fade out any playing music and schedule next one
void Sound_ChangeMusic(const char *file) {
	if (next_music) {
		Mix_FreeMusic(next_music);
		next_music = NULL;
	}

	if (!file || !strcmp(file, "?")) {
		play_music = 0;
	} else {
		next_music = Sound_LoadFile(file);
		play_music = 1;
	}

	Mix_FadeOutMusic(1000);
}

// start next music when the last one finished
int Sound_MixBack(int sync) {
	char *buf;
	unsigned i;

	for (i = 0; i < CHANNELS; i++) {
		if (!chans[i].data || Mix_Playing(i)) {
			continue;
		}

		// loop leadout ended, clear the channel
		if (chans[i].loopstart >= chans[i].size) {
			Sound_Mute(i);
		}
	}

	if (!active || !play_music || Mix_PlayingMusic()) {
		return 0;
	}

	if (cur_music) {
		Mix_FreeMusic(cur_music);
	}

	if (!next_music) {
		konec_skladby(&buf);
		cur_music = Sound_LoadFile(buf);
	} else {
		cur_music = next_music;
		next_music = NULL;
	}

	if (cur_music) {
		Mix_FadeInMusic(cur_music, 1, 1000);
	}

	return 0;
}
