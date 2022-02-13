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
#include <cstdio>
#include <cstdlib>
#include <climits>
#include <cstring>
#include <inttypes.h>
#include <cassert>
#include <cmath>
#include "libs/memman.h"
#include "libs/sound.h"
#include "libs/wav_mem.h"
#include "libs/event.h"
#include "game/globals.h"
#include "libs/strlite.h"
#include "libs/system.h"

#define PL_RANDOM 1
#define PL_FORWARD 2
#define PL_FIRST 3

#define CHANNELS 20
#define TRACKS 512

#define SND_EFF_MAXVOL 32000
#define SND_DIST_COEF 32

#define have_loop(x) ((x)->start_loop!=(x)->end_loop)

typedef struct snd_info
  {
  TMA_SOUND *data;              //4
  int16_t xpos,ypos,side;         //10
  uint16_t volume,block;            //14
  }SND_INFO;

static short chan_state[CHANNELS];
static short track_state[TRACKS];

static char sound_enabled=1;

SND_INFO tracks[TRACKS];
SND_INFO playings[CHANNELS];
static uint16_t locks[32];

StringList cur_playlist;
StringList sound_table;
int playlist_size;
int playing_track=0;
int remain_play=0;
int play_list_mode=PL_RANDOM;

void init_tracks()
  {
  memset(tracks,0,sizeof(tracks));
  memset(playings,0,sizeof(playings));
  memset(chan_state,0xff,sizeof(chan_state));
  memset(track_state,0xff,sizeof(track_state));
  memset(locks,0,sizeof(locks));
  }

int find_free_channel(int stamp) {
	int i, j;
	int dist, vol, tdist, tvol;


	if (stamp) {
		for (i = 0; i < CHANNELS; i++) {
			if (chan_state[i] == stamp) {
				return i;
			}
		}
	}

	j = 0;
	dist = Sound_GetDistance(j);
	vol = Sound_GetVolume(j);

	for (i = 0; i < CHANNELS; i++) {
		if (!Sound_GetChannelState(i)) {
			return i;
		}

		tdist = Sound_GetDistance(i);
		tvol = Sound_GetVolume(i);

		if ((tdist > dist) || (tdist == dist && tvol < vol)) {
			j = i;
			dist = tdist;
			vol = tvol;
		}
	}

	return j;
}

void release_channel(int channel)
  {
  int i;

  i=chan_state[channel];
  if (i==-1) return;
  Sound_Mute(channel);
     {
     aunlock(playings[channel].block);
     chan_state[channel]=-1;
     track_state[i]=-1;
     }
  }

int calcul_volume(int chan, int x, int y, int side, int volume) {
	double dist, angle;
	int ds, ang;

	if (side == -1) {
		side = viewdir;
	}

	side %= 4;
	dist = sqrt(x*x + y*y);
	ds = SND_DIST_COEF * dist;

	if (ds >= 256) {
		release_channel(chan);
		return -1;
	}

	if (!x && !y) {
		ang = 0;
	} else {
		angle = 180 * acos(y / dist) / M_PI;

		if (x > 0) {
			angle = 360 - angle;
		}

		ang = angle;
		ang = (ang + 360 - side * 90) % 360;
	}

	Sound_SetVolume(chan, (volume * SND_EFF_MAXVOL) / 100, ang, ds);
	return 0;
}

DataBlock *wav_load(SeekableReadStream &stream) {
	return new SoundSample(stream);
}

void play_effekt(int x, int y, int xd, int yd, int side, int sided, TMA_SOUND *p) {
	int chan;
	int32_t blockid;
	SND_INFO *track;
	THANDLE_DATA *z;
	const SoundSample *s;

	if (!sound_enabled) {
		return;
	}

	chan = find_free_channel(p->soundid);
	release_channel(chan);
	track = &tracks[p->soundid];
	track->data = p;
	track->xpos = xd;
	track->ypos = yd;
	track->side = sided;
	track_state[p->soundid] = -1;

	if (p->bit16 & 0x8) {
		int vol = SND_EFF_MAXVOL * p->volume / 100;

		Sound_SetVolume(chan, vol, rnd(360), 0);
	} else if (calcul_volume(chan, x - xd, y - yd, /*side-*/sided, p->volume)) {
		return;
	}

	if (p->filename[0] == 1) {
		memcpy(&blockid, &p->filename[1], sizeof(int32_t));
	} else {
		blockid = find_handle(p->filename, wav_load);

		if (blockid == -1) {
			z = def_handle(end_ptr, p->filename, wav_load, SR_ZVUKY);
			blockid = end_ptr++;

			if (level_preload) {
				apreload(blockid);
			}
		}

		memcpy(&p->filename[1], &blockid, sizeof(int32_t));
		p->filename[0] = 1;
	}

	alock(blockid);
	s = dynamic_cast<const SoundSample*>(ablock(blockid));
	Sound_PlaySample(chan, s->data(), s->length(), p->start_loop - p->offset, p->end_loop - p->offset, p->freq, 1 + (p->bit16 & 1));
	playings[chan].data = p;
	playings[chan].xpos = xd;
	playings[chan].ypos = yd;
	playings[chan].side = sided;
	playings[chan].volume = p->volume;
	playings[chan].block = blockid;
	chan_state[chan] = p->soundid;
	track_state[p->soundid] = chan;
}

void restore_sound_name(TMA_SOUND *p)
  {
  int32_t blockid;
  THANDLE_DATA *h;

  if (p->filename[0]==1)
     {
      memcpy(&blockid,&p->filename[1],sizeof(int32_t));
     do
        {
        h=get_handle(blockid);
        if (h->status==BK_SAME_AS) blockid=h->seekpos;else blockid=-1;
        }
     while (blockid!=-1);
     strncpy(p->filename,h->src_file,12);
     }
  }

void restore_sound_names() {
	int i, j;

	for (i = 0; i < gameMap.coordCount() * 4; i++) {
		const MacroEntry &entry = gameMap.macro(i);

		for (j = 0; entry.data && j < entry.size; j++) {
			if (entry.data[j].general.action == MA_SOUND) {
				restore_sound_name(&entry.data[j].sound);
			}
		}
	}
}

void recalc_volumes(int sector, int side) {
	int i;
	int newx, newy, layer;

	if (sector >= gameMap.coordCount()) {
		return;
	}

	SEND_LOG("(SOUND) %s", "Recalculating volumes", 0);
	newx = gameMap.coord()[sector].x;
	newy = gameMap.coord()[sector].y;
	layer = gameMap.coord()[sector].layer;

	for (i = 0; i < CHANNELS; i++) {
		if (chan_state[i] >= 0 && playings[i].side >= 0) {
			calcul_volume(i, newx-playings[i].xpos, newy - playings[i].ypos, /*side-*/playings[i].side, playings[i].volume);

			if (!Sound_GetChannelState(i)) {
				release_channel(i);
			}
		} else {
			calcul_volume(i, 0, 0, -1, playings[i].volume);
		}
	}

	for (i = 1; i < TRACKS; i++) {
		if (track_state[i] < 0 && tracks[i].data != NULL) {
			if (tracks[i].side < 0) {
				if (have_loop(tracks[i].data)) {
					play_effekt(0, 0, 0, 0, -1, -1, tracks[i].data);
				}
			} else {
				int x = newx - tracks[i].xpos, y = newy - tracks[i].ypos;
				if (have_loop(tracks[i].data)) {
					play_effekt(newx, newy, tracks[i].xpos, tracks[i].ypos, side, tracks[i].side, tracks[i].data);
				}
			}
		}
	}
}

void create_playlist(const char *playlist)
  {
  const char *c;
  char mode[20];
  char shift;
  int i=1,j;
  cur_playlist.clear();
  if (!playlist || !playlist[0]) return;
  c=playlist;
  while (*c && *c==32) c++;
  sscanf(c,"%s",mode);
  strupr(mode);
  shift=1;
  if (!strcmp(mode,"RANDOM")) play_list_mode=PL_RANDOM;
  else if (!strcmp(mode,"FORWARD")) play_list_mode=PL_FORWARD;
  else if (!strcmp(mode,"FIRST")) play_list_mode=PL_FIRST;
  else shift=0;
  if (shift) c+=strlen(mode);else play_list_mode=PL_RANDOM;
  while (*c && *c==32) c++;
  playlist=c;
  if (playlist=="") return;
  for (c=playlist;c!=NULL;c=strchr(c+1,' ')) i++;
  playlist_size=i-1;
  j=0;
  for (c=playlist;c!=NULL;c=strchr(c+1,' '))
     {
     char *e;
     char d[PATH_MAX+2]="!";
     strncat(d,c+j,PATH_MAX);d[PATH_MAX+1]=0;j=1;
     if ((e=strchr(d,32))!=NULL) *e=0;
     strupr(d);
     cur_playlist.insert(d);
     }
  if (play_list_mode==PL_FIRST)
     {
     char *ptr = new char[strlen(cur_playlist[0]) + 1];
     strcpy(ptr, cur_playlist[0]);
     ptr[0] = ' ';
     cur_playlist.replace(0, ptr);
     delete[] ptr;
     remain_play=1;
     play_list_mode=PL_RANDOM;
     }
  else
     {
     remain_play=0;
     }
  playing_track=-1;
  }

void play_next_music(char **c)
  {
  int i,step;
  static char d[PATH_MAX];
	char *tmp;

  *c=NULL;
  if (!cur_playlist.count()) return;
  if (!remain_play)
     for (i = 0; cur_playlist[i] != NULL; remain_play++, i++) {
		char *tmp = new char[strlen(cur_playlist[i]) + 1];
		strcpy(tmp, cur_playlist[i]);
		tmp[0] = ' ';
		cur_playlist.replace(i, tmp);
		delete[] tmp;
	}
  if (play_list_mode==PL_RANDOM)
	step = rnd(playlist_size) + 1;
  else
     step=1;
  i=playing_track;
  do
     {
     i++;
     if (cur_playlist[i]==NULL) i=0;
     if (cur_playlist[i][0]==32) step--;
     }
  while (step);
  playing_track=i;
/*
  snprintf(d,sizeof(d),"%s%s",pathtable[SR_MUSIC],cur_playlist[i]+1);
  if (access(d,0) == -1)
      snprintf(d,sizeof(d),"%s%s",pathtable[SR_ORGMUSIC],cur_playlist[i]+1);
*/

	tmp = Sys_FullPath(SR_MUSIC, cur_playlist[i] + 1);
	if (!Sys_FileExists(tmp)) {
		tmp = Sys_FullPath(SR_ORGMUSIC, cur_playlist[i] + 1);
	}
	strcpy(d, tmp);

	tmp = new char[strlen(cur_playlist[i]) + 1];
	strcpy(tmp, cur_playlist[i]);
	tmp[0] = 33;
	cur_playlist.replace(i, tmp);
	delete[] tmp;

  remain_play--;
  *c=d;
  }

void purge_playlist()
  {
	cur_playlist.clear();
  }

void play_sample_at_sector(int sample, int sector1, int sector2, int track, char loop) {
	int x, y, xd, yd, chan;
	int oldtrack;
	const SoundSample *s;

	if (!sound_enabled) {
		return;
	}

	if (gameMap.coord()[sector1].layer != gameMap.coord()[sector2].layer) {
		return;
	}

	x = gameMap.coord()[sector1].x;
	y = gameMap.coord()[sector1].y;
	xd = gameMap.coord()[sector2].x;
	yd = gameMap.coord()[sector2].y;
	chan = find_free_channel(track);
	oldtrack = track_state[track];

	if (!track || oldtrack == -1) {
		release_channel(chan);
	}

	if (calcul_volume(chan, x - xd, y - yd, viewdir, 100)) {
		return;
	}

	if (!track || oldtrack == -1) {
		alock(sample);
		s = dynamic_cast<const SoundSample*>(ablock(sample));
		Sound_PlaySample(chan, s->data(), s->length(), loop ? 0 : s->length(), s->length(), s->freq(), (s->freq() != s->bps() ? 2 : 1));
		playings[chan].data = NULL;
	}

	playings[chan].xpos = xd;
	playings[chan].ypos = yd;
	playings[chan].side = viewdir;
	playings[chan].volume = 100;
	playings[chan].block = sample;
	chan_state[chan] = track;
	track_state[track] = chan;
}

void play_sample_at_channel(int sample, int channel, int vol) {
	const SoundSample *s;

	if (!sound_enabled) {
		return;
	}

	channel += CHANNELS;
	vol *= SND_EFF_MAXVOL / 100;
	Sound_SetVolume(channel, vol, 0, 0);

	if (locks[channel]) {
		aunlock(locks[channel]);
	}

	alock(sample);
	locks[channel] = sample;
	s = dynamic_cast<const SoundSample*>(ablock(sample));
	Sound_PlaySample(channel, s->data(), s->length(), s->length(), s->length(), s->freq(), (s->freq() != s->bps() ? 2 : 1));
}

void create_sound_table_old() {
	int i, length;
	const char *str;
	SeekableReadStream *tmp = afile("SOUND.DAT", SR_MAP);
	MemoryReadStream *stream = tmp->readStream(tmp->size());

	delete tmp;
	length = stream->readUint32LE();

	for (i = 0; i < length; i++) {
		str = stream->readCString();

		if (str && *str) {
			sound_table.replace(i, str);
		}
	}
}


void stop_track(int track)
  {
  int chan;
  chan=track_state[track];
  if (chan==-1) return;
  Sound_BreakLoop(chan);
  }

void stop_track_free(int track)
  {
  int chan;
  chan=track_state[track];
  if (chan==-1) return;
  Sound_BreakLoop(chan);
  track_state[track]=-1;
  chan_state[chan]=0;
  }

void mute_all_tracks(char all)
  {
  int i;
  for(i=0;i<CHANNELS;i++)
     if (playings[i].side!=-1 || all) release_channel(i);
  SEND_LOG("(SOUND) %s (%d)","MUTE Tracks",all);
  }


void kill_all_sounds()
  {
  int i;
  SEND_LOG("(SOUND) Killing sound tracks...",0,0);
  for (i=0;i<CHANNELS;i++) release_channel(i);
  for (i=0;i<32;i++) if (locks[i]!=0) aunlock(locks[i]);
  }

char test_playing(int track)
  {
  return track_state[track]!=-1;
  }

static int flute_canal=30;

void start_play_flute(char note) {
	int vol = 50, btr, lstart, lend;

	assert(note >= 0 && note < H_FLETNA_CNT && "Flute note out of range");

	if (Sound_CheckEffect(SND_GFX)) {
		const SoundSample *ptr = dynamic_cast<const SoundSample*>(ablock(H_FLETNA + note));
		vol *= SND_EFF_MAXVOL / 100;
		btr = ptr->bps() / ptr->freq();
		btr = btr < 1 ? 1 : btr;
		lstart = ptr->length() * 0.32229f;
		lstart -= lstart % btr;
		lend = ptr->length() * 0.664157f;
		lend -= lend % btr;
		Sound_SetVolume(flute_canal, vol, 0, 0);
		Sound_PlaySample(flute_canal, ptr->data(), ptr->length(), lstart, lend, ptr->freq(), btr, 1);
	}
}

void stop_play_flute() {
	if (Sound_CheckEffect(SND_GFX)) {
		Sound_BreakExt(flute_canal);
	}
}

char enable_sound(char enbl)
  {
  char save;

  save=sound_enabled;
  sound_enabled=enbl;
  SEND_LOG("(SOUND) Sound status (en/dis) changed: new %d, old %d",enbl,save);
  return save;
  }
