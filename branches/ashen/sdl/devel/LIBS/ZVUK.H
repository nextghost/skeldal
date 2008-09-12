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
 *  Last commit made by: $Id: ZVUK.H 7 2008-01-14 20:14:25Z bredysoft $
 */
#ifndef __ZVUK_H___
#define __ZVUK_H___

#ifdef __cplusplus
extern "C" {
#endif

#define BACK_BUFF_SIZE 0x40000

#define DEV_NOSOUND 0
#define DEV_SB10 1
#define DEV_SB20 2
#define DEV_SBPRO 3
#define DEV_SB16 4
#define DEV_WSS 5
#define DEV_ULTRA 6
#define DEV_DAC 7
#define DEV_PCSPEAKER 8
#define DEV_DIRECTSOUND 9     //only valid device for this module

extern int bvolume;
extern void (*konec_skladby)(char **jmeno);

int sound_detect(int *dev,int *port,int *dma, int *irq);
void set_mixing_device(int mix_dev,int mix_freq,...);
char start_mixing();
void stop_mixing();
void play_sample(int channel,void *sample,long size,long lstart,long sfreq,int type);
void set_channel_volume(int channel,int left,int right);
void init_winamp_plugins(const char *path);

void fade_music();
int mix_back_sound(int synchro);
int open_backsound(char *filename);
void change_music(const char *filename);
int get_timer_value();
char *device_name(int device);
void force_music_volume(int volume);

void set_backsnd_freq(int freq);

char get_channel_state(int channel);
void get_channel_volume(int channel,int *left,int *right);
void mute_channel(int channel);
void chan_break_loop(int channel);
void chan_break_ext(int channel,void *org_sample,long size_sample); //zrusi loop s moznosti dohrat zvuk

char set_snd_effect(int funct,int data);
char check_snd_effect(int funct);
int  get_snd_effect(int funct);

void *PrepareVideoSound(int mixfreq, int buffsize);
char LoadNextVideoFrame(void *buffer, char *data, int size, short *xlat, short *accnums, long *writepos);
void DoneVideoSound(void *buffer);


#define SND_MAXFUNCT 11
#define SND_PING    0  //Ping function
#define SND_GVOLUME 1  //SetGlobalVolume
#define SND_BASS    2  //SetBass
#define SND_TREBL   3  //SetTrebles
#define SND_SWAP    4  //SetSwapChannels
#define SND_LSWAP   5  //SetLinearSwapping
#define SND_SURROUND 6 //Surrourd
#define SND_OUTFILTER 7//Out Filter
#define SND_GFX     8  //setgfxvolume
#define SND_MUSIC   9  //setmusicvolume
#define SND_XBASS  10  //setxbassy

#ifdef __cplusplus
  }
#endif
#endif

