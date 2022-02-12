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
 *  Last commit made by: $Id: FLC.H 7 2008-01-14 20:14:25Z bredysoft $
 */
//
//		Hlavickovy soubor ke knihovne FLC.C slouzici k dekompresi FLC
//

#ifndef _FLC_H
#define _FLC_H

typedef struct FLCHEADER {
	unsigned int size;		// delka souboru vcetne hlavicky
	unsigned short idflc;	// ID FLC=0AF12h
	unsigned short frames;	// pocet frejmu
	unsigned short width;	// sirka vsech obrazku
	unsigned short height;	// vyska vsech obrazku
	unsigned short color;	// hloubka barvy (bpp)
	unsigned short flag1;
	unsigned int speed;		// rychlost prehravani (v 1/1000 s)
	unsigned short reserv1;	// rezervovany
	unsigned int date1;		// datum a cas vytvoreni
	unsigned int serial;	// seriove cislo programu
	unsigned int date2;		// datum a cas posledni zmeny
	unsigned short XA;
	unsigned short YA;
	char reserv2 [42];		// rezervovano
	unsigned int offset1;	// offset prvniho frame
	unsigned int offset2;	// offset druheho frame
	char reserv3 [40];		// rezervovano
	} flcheader;

typedef struct FRAMEHEADER {
	unsigned int size;		// velikost frame v bytech
	unsigned short sign;	// znacka frame OFAF1h
	unsigned short actions;	// pocet akci v tomto frame
	char reserv [8];		// rezervovano
	} frameheader;

typedef struct ACTIONHEADER {
	unsigned int size;		// velikost akce v bytech
	unsigned short code;	// kod akce
		// 04h - predani casti nebo cele palety
		// 07h - predavani zmenenych casti obrazu
		// 0Dh - vymaze obrazovku
		// 0Fh - cela obrazovka v RLE
		// 10h - nekomprimovana kopie cele obrazovky
	// nasleduji data akce
	} actionheader;

FILE *flc;
flcheader h_flc;
frameheader h_frame;
actionheader h_action;
char frame_buffer [307205];
char *flc_buffer;
//unsigned int ladici;

void Open_FLC (char *filename);
void Close_FLC (void);
void Get_first_frame (void);
void Get_next_frame (void);
void Decompress_frame (void);

#endif
