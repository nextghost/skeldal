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
#include <SDL/SDL_mixer.h>

#define CHANNELS 32
#define SND_EFF_MAXVOL 32000

static int freq = 22050;
static Mix_Chunk chunks[CHANNELS] = {0};

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
	Mix_CloseAudio();
}

// FIXME: resample when loading the sound file
void Sound_PlaySample(int channel, void *sample, long size, long lstart, long sfreq, int type) {
	SDL_AudioCVT cvt;

	assert(channel >= 0 && channel < CHANNELS && "Invalid channel");
	assert(((lstart == 0) || (lstart == size)) && "Lead-in not supported");

	if (Mix_Playing(channel)) {
		Mix_HaltChannel(channel);
	}

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
