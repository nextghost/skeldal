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
 *  Last commit made by: $Id: PLAY.C 7 2008-01-14 20:14:25Z bredysoft $
 */
#include <stdio.h>
#include <conio.h>
#include <graph.h>
#include <dos.h>
#include "flc.h"
#include "vesa.h"

void Set_TEXT_mode (void);

void main (int argc, char *argv[])
{
	int x;

	Open_FLC (argv[1]);

	printf ("\nHlavicka FLC souboru %s:\n", argv[1]);
	printf ("Delka celeho souboru: %d\n", h_flc.size);
	printf ("Pocet frame: %d\n", h_flc.frames);
	printf ("Velikost filmu: %dx%d\n", h_flc.width, h_flc.height);
	printf ("Hloubka barvy: %d\n", h_flc.color);
//	printf ("Rychlost prehravani: %d fps\n", 1000/h_flc.speed);
	printf ("Offset prvniho frame: %d\n", h_flc.offset1);
	getch ();

	Set_VESA_mode (0x101);
	delay (1000);

	Get_first_frame ();
	Decompress_frame ();
	Show_screen (frame_buffer);

	for (x = 2; x<= h_flc.frames; x++)
	{
		Get_next_frame ();
		Decompress_frame ();
		Show_screen (frame_buffer);
		delay (7);
	}
	getch ();

	Set_TEXT_mode ();
	Close_FLC ();	
}

void Set_TEXT_mode (void)
{
	union REGS inr, outr;

	inr.h.ah = 0x00;
    inr.h.al = 03;
    int386 (0x10, &inr, &outr);
}

		
