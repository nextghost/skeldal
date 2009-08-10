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

static int freq = 22050;

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
