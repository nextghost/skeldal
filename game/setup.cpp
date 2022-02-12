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
#include <cstring>
#include <cassert>
#include "libs/event.h"
#include "libs/memman.h"
#include "libs/devices.h"
#include "libs/bmouse.h"
#include "libs/bgraph.h"
#include "libs/sound.h"
#include "libs/strlite.h"
#include "game/engine1.h"
#include "libs/pcx.h"
#include "game/globals.h"
#include "libs/gui.h"
#include "libs/basicobj.h"
#include "game/interfac.h"

extern char enable_sort;
extern char autoattack;


char q_runsetup(char *parm)
  {
  char c[6];

  strncpy(c,parm,6);
  strupr(c);
  return !strncmp(c,"/SETUP",6);
  }


static void show_setup_desktop(WINDOW *w) {
	const Texture *tex = dynamic_cast<const Texture*>(ablock(H_SETUPBAR));

	renderer->blit(*tex, w->x, w->y, tex->palette());
}

static void checkbox_animator(THE_TIMER *t)
  {
  t;
  animate_checkbox(10,130,10);
  }

static int effects[]={SND_GVOLUME,SND_MUSIC,SND_GFX,SND_TREBL,SND_BASS,SND_XBASS};

static void do_setup_change() {
	SkeldalCheckBox *cb;
	SkeldalSlider *slider;

	switch (o_aktual->id()) {
	case 10:
		cb = dynamic_cast<SkeldalCheckBox*>(o_aktual);
		assert(cb);
		Sound_SetEffect(SND_SWAP, cb->getValue());
		break;

	case 20:
		cb = dynamic_cast<SkeldalCheckBox*>(o_aktual);
		assert(cb);
		Sound_SetEffect(SND_OUTFILTER, cb->getValue());
		break;

	default:
		slider = dynamic_cast<SkeldalSlider*>(o_aktual);
		if (slider) {
			Sound_SetEffect(effects[o_aktual->id() / 10 - 20], slider->getValue());
		}
		break;
	}
}

static void change_zoom() {
	int id = o_aktual->id();
	int i;
	SkeldalCheckBox *cb;
	WINDOW *wi;

	for(i = 30; i < 60; i += 10) {
		cb = dynamic_cast<SkeldalCheckBox*>(find_object_desktop(0, i, &wi));
		assert(cb);
		cb->setValue(i == id ? 1 : 0);
	}

	zoom_speed((id - 30) / 10);
}

static void change_turn() {
	int id = o_aktual->id();
	int i;
	SkeldalCheckBox *cb;
	WINDOW *wi;

	for (i = 60; i < 90; i += 10) {
		cb = dynamic_cast<SkeldalCheckBox*>(find_object_desktop(0, i, &wi));
		assert(cb);
		cb->setValue(i == id ? 1 : 0);
	}

	turn_speed((id-60)/10);
}

static void unwire_setup();

static EVENT_PROC(setup_keyboard) {
	WHEN_MSG(E_KEYBOARD) {
		char c;
		va_list args;

		va_copy(args, msg->data);
		c = va_arg(args, int) & 0xff;
		va_end(args);

		if (c == 27) {
			unwire_proc();
		}
	}
}

static void wire_setup()
  {
  unwire_proc();
  unwire_proc=unwire_setup;
  mute_all_tracks(0);
  cur_mode=MD_SETUP;
  send_message(E_ADD,E_KEYBOARD,setup_keyboard);
  SEND_LOG("(GAME) Starting setup",0,0);
  }

static void unwire_setup() {
	SkeldalCheckBox *cb;
	WINDOW *wi;

	cb = dynamic_cast<SkeldalCheckBox*>(find_object_desktop(0, 90, &wi));
	assert(cb);
	show_names = cb->getValue();
	cb = dynamic_cast<SkeldalCheckBox*>(find_object_desktop(0, 100, &wi));
	assert(cb);
	enable_sort = cb->getValue();
	cb = dynamic_cast<SkeldalCheckBox*>(find_object_desktop(0, 110, &wi));
	assert(cb);
	autoattack = cb->getValue();
	cb = dynamic_cast<SkeldalCheckBox*>(find_object_desktop(0, 120, &wi));
	assert(cb);
	autosave_enabled = cb->getValue();
	cb = dynamic_cast<SkeldalCheckBox*>(find_object_desktop(0, 130, &wi));
	assert(cb);
	level_preload = cb->getValue();

	delete_from_timer(TM_CHECKBOX);
	Sound_MixBack(32768);
	close_current();
	send_message(E_DONE, E_KEYBOARD, setup_keyboard);
	wire_proc();
	cancel_render = 1;
	SEND_LOG("(GAME) Setup closed", 0, 0);
}

char exit_setup(int id,int xa,int ya,int xr,int yr)
  {
  id,xa,ya,xr,yr;
  unwire_setup();
  return 0;
  }

T_CLK_MAP setup[]=
  {
  {-1,0,0,639,14,exit_setup,2,H_MS_DEFAULT},
  {-1,0,15,639,479,exit_setup,0x8,H_MS_DEFAULT},
  };


void new_setup() {
	WINDOW *w;
	CTL3D ctl;
	size_t i;
	static int textxp[] = {75, 75, 435, 435, 434, 535, 535, 535, 434, 434, 434, 434, 434, 35, 410, 510};
	static int textyp[] = {275, 305, 65, 95, 125, 65, 95, 125, 185, 215, 245, 275, 305, 235, 40, 40};
	static int  textc[] = {53, 54, 56, 57, 58, 56, 57, 58, 140, 141, 142, 143, 144, 51, 55, 59};
	const Font* font = dynamic_cast<const Font*>(ablock(H_FBOLD));
	uint8_t black[3] = {0};

	Sound_MixBack(256000 - 16384);
	memset(&ctl, 0, sizeof(ctl));
	change_click_map(setup, 4);
	renderer->setFont(font, SETUP_COL2);
	GUIObject::setDefaultFont(font, SETUP_COL2);
	w = create_window(0, SCREEN_OFFLINE, 639, 359, 0, 0, 0, &ctl);
	w->draw_event = show_setup_desktop;
	desktop_add_window(w);
	define(new SkeldalCheckBox(10, 50, 270, 190, 20, 0, Sound_GetEffect(SND_SWAP)));
	on_change(do_setup_change);

	if (Sound_CheckEffect(SND_OUTFILTER)) {
		define(new SkeldalCheckBox(20, 50, 300, 190, 20, 0, Sound_GetEffect(SND_OUTFILTER)));
		on_change(do_setup_change);
	}

	define(new SkeldalCheckBox(30, 410, 60, 90, 20, 0, zoom_speed(-1) == 0));
	on_change(change_zoom);
	define(new SkeldalCheckBox(40, 410, 90, 90, 20, 0, zoom_speed(-1) == 1));
	on_change(change_zoom);
	define(new SkeldalCheckBox(50, 410, 120, 90, 20, 0, zoom_speed(-1) == 2));
	on_change(change_zoom);
	
	define(new SkeldalCheckBox(60, 510, 60, 90, 20, 0, turn_speed(-1) == 0));
	on_change(change_turn);
	define(new SkeldalCheckBox(70, 510, 90, 90, 20, 0, turn_speed(-1) == 1));
	on_change(change_turn);
	define(new SkeldalCheckBox(80, 510, 120, 90, 20, 0, turn_speed(-1) == 2));
	on_change(change_turn);

	define(new SkeldalCheckBox(90, 410, 180, 190, 20, 0, show_names));
	define(new SkeldalCheckBox(100, 410, 210, 190, 20, 0, enable_sort));
	define(new SkeldalCheckBox(110, 410, 240, 190, 20, 0, autoattack));
	define(new SkeldalCheckBox(120, 410, 270, 190, 20, 0, autosave_enabled));
	define(new SkeldalCheckBox(130, 410, 300, 190, 20, 0, level_preload));

	for (i = 0; i < sizeof(textc) / sizeof(int); i++) {
		define(new Label(-1, textxp[i], textyp[i] - 1, 1, 1, 0, texty[textc[i]]));
	}

	for (i = 0; i < sizeof(effects) / sizeof(int); i++) {
		if (Sound_CheckEffect(effects[i])) {
			define(new SkeldalSlider(200 + i * 10, 50 + i * 60, 30, 30, 200, 0, effects[i] == SND_MUSIC ? 127 : 255, Sound_GetEffect(effects[i])));
			on_change(do_setup_change);
		}
	}

	define(new SetupOkButton(300, 559, 336, 81, 21, 0, texty[174]));
	on_change(unwire_setup);
	font = dynamic_cast<const Font*>(ablock(H_FTINY));
	property(NULL, font, 1, color_topbar, black);
	redraw_window();
	add_to_timer(TM_CHECKBOX, 4, -1, checkbox_animator);
}

void game_setup_()
  {
  wire_setup();
  new_setup();
  }

char game_setup(int id,int xa,int ya,int xr,int yr)
  {
  id;xa;ya;xr;yr;
  run_background(game_setup_);
  return 1;
  }


void GameResume(EVENT_MSG *msg,void **data)
  {
  static int volsave;
  data;
  if (msg->msg==E_INIT)
     {
     volsave=Sound_GetEffect(SND_GVOLUME);
     Sound_SetEffect(SND_GVOLUME,volsave>>1);
     }
  if (msg->msg==E_KEYBOARD)
     {
     Sound_SetEffect(SND_GVOLUME,volsave);
     wire_main_functs();
     msg->msg=-2;
     }
  }

void GamePause() {
	int i;
	const Font *font = dynamic_cast<const Font*>(ablock(H_FBOLD));

	unwire_proc();
	send_message(E_ADD, E_KEYBOARD, GameResume);
	update_mysky();
	trans_bar(0, 0, 640, 480, 0, 0, 0);
	renderer->setFont(font, 1, 0, 189, 0);
	i = renderer->textWidth(texty[5]);
	add_window(320 - (i / 2) - 10, 100, i + 40, 40, H_IDESKA, 4, 20, 20);
	redraw_window();
	renderer->drawAlignedText(320, 115, HALIGN_CENTER, VALIGN_CENTER, texty[5]);
	showview(0, 0, 0, 0);
	cancel_render = 1;
}

/*void user_setup()
  {

  initmode256(cur_xlat);
  init_mysky();
  hranice_mysky(0,0,639,479);
  mouse_set_default(H_MS_DEFAULT);
  ukaz_mysku();
  setup_dialoge();
  escape();
  schovej_mysku();
  closemode();
  }
 */
