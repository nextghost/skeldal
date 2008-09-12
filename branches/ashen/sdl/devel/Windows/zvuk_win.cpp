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
 *  Last commit made by: $Id: zvuk_win.cpp 7 2008-01-14 20:14:25Z bredysoft $
 */
#include <skeldal_win.h>
#include <zvuk.h>

extern "C" 
{

	int mix_back_sound(int synchro)
	{

		return 0;
	}

	int get_timer_value()
	{
		return GetTickCount()/TIMERSPEED;
	}


	int get_snd_effect(int funct)
	{
		return 0;
	}

	char set_snd_effect(int funct,int data)
	{
		return 0;
	}

	char check_snd_effect(int funct)
	{
		return 0;
	}

	void change_music(char *filename)
	{

	}


	void set_backsnd_freq(int freq)
	{

	}

	void fade_music()
	{

	}

	void set_mixing_device(int mix_dev,int mix_freq,...)
	{
	}
	char start_mixing()
	{
		return 0;
	}
	void stop_mixing()
	{
	}
	void play_sample(int channel,void *sample,long size,long lstart,long sfreq,int type)
	{
	}
	void set_channel_volume(int channel,int left,int right)
	{
	}
	int open_backsound(char *filename)
	{
		return 0;
	}
	char *device_name(int device)
	{
		return "";
	}
	void force_music_volume(int volume)
	{
	}
	char get_channel_state(int channel)
	{
		return 0;
	}
	void get_channel_volume(int channel,int *left,int *right)
	{
	}
	void mute_channel(int channel)
	{
	}
	void chan_break_loop(int channel)
	{
	}
	void chan_break_ext(int channel,void *org_sample,long size_sample) //zrusi loop s moznosti dohrat zvuk
	{
	}

}

