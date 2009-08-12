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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <SDL/SDL_mixer.h>
#include "libs/sound.h"
#include "libs/system.h"

#define CHANNELS 32
#define SND_EFF_MAXVOL 32000
#define GFX_MAXVOL 255
#define MUSIC_MAXVOL 255

static int freq = 22050;
static Mix_Chunk chunks[CHANNELS] = {0};
static int swapped = 0;
static int play_music = 0;
static Mix_Music *cur_music = NULL, *next_music = NULL;

void Sound_SetMixer(int mix_dev, int mix_freq, ...) {
	freq = mix_freq;
}

void Sound_StartMixing(void) {
	if (Mix_OpenAudio(freq, AUDIO_S16SYS, 2, 1024) < 0) {
		assert(0 && "Failed to start mixer");
	}

	Mix_AllocateChannels(CHANNELS);
}

void Sound_StopMixing(void) {
	int i;

	// clean up resampled buffers
	for (i = 0; i < CHANNELS; i++) {
		Sound_Mute(i);
	}

	// close music files
	Mix_HaltMusic();
	if (cur_music) {
		Mix_FreeMusic(cur_music);
	}

	if (next_music) {
		Mix_FreeMusic(next_music);
	}

	// shut down subsystem
	Mix_CloseAudio();
}

// FIXME: resample when loading the sound file
void Sound_PlaySample(int channel, void *sample, long size, long lstart, long sfreq, int type) {
	SDL_AudioCVT cvt;

	assert(channel >= 0 && channel < CHANNELS && "Invalid channel");
	assert(((lstart == 0) || (lstart == size)) && "Lead-in not supported");

	// clean up resampled buffers
	Sound_Mute(channel);

	if (!SDL_BuildAudioCVT(&cvt, type == 1 ? AUDIO_U8 : AUDIO_S16SYS, 1, sfreq, AUDIO_S16SYS, 2, freq)) {
		fprintf(stderr, "Resample from %dHz/%d bits to %dHz/16 bits failed\n", sfreq, type == 1 ? 8 : 16, freq);
		return;
	}

	cvt.buf = malloc(size * cvt.len_mult);
	cvt.len = size;
	memcpy(cvt.buf, sample, size);

	SDL_ConvertAudio(&cvt);

	chunks[channel].allocated = 1;
	chunks[channel].abuf = cvt.buf;
	chunks[channel].alen = cvt.len_cvt;
	chunks[channel].volume = 128;

	Mix_PlayChannel(channel, chunks + channel, lstart == 0 ? -1 : 0);
}

// FIXME: make the channel finnish loop
void Sound_BreakLoop(int channel) {
	Mix_HaltChannel(channel);

	if (chunks[channel].abuf && chunks[channel].allocated) {
		free(chunks[channel].abuf);
		chunks[channel].abuf = NULL;
		chunks[channel].allocated = 0;
	}
}

// FIXME: add lead-out playback
void Sound_BreakExt(int channel, void *sample, long size) {
	Sound_BreakLoop(channel);
}

void Sound_Mute(int channel) {
	Sound_BreakLoop(channel);
}

char Sound_GetChannelState(int channel) {
	return Mix_Playing(channel) != 0;
}

void Sound_SetVolume(int channel, int left, int right) {
	left = (left * 255) / SND_EFF_MAXVOL;
	right = (right * 255) / SND_EFF_MAXVOL;
	Mix_SetPanning(channel, left, right);
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

	buf = malloc((strlen(file) + 5) * sizeof(char));
	strcpy(buf, file);
	tmp = strrchr(buf, '.');

	if (tmp) {
		strcpy(tmp, ".mp3");
	} else {
		strcat(buf, ".mp3");
	}

	ret = Mix_LoadMUS(buf);

	if (!ret) {
		fprintf(stderr, "Could not load music file %s...\n", buf);
	}

	free(buf);
	return ret;
}

// fade out any playing music and schedule next one
void Sound_ChangeMusic(char *file) {
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

	if (!play_music || Mix_PlayingMusic()) {
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
