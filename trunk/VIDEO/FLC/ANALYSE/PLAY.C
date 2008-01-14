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
#include <stdio.h>
#include <conio.h>
#include <graph.h>
#include "flc.h"

flcheader h_flc;
frameheader h_frame;
actionheader h_action;


void main (int argc, char *argv[])
{
	FILE *flc;
	long pozice;
	int frames, actions, x, y;

	flc = fopen (argv[1], "rb");
	fread (&h_flc, sizeof(flcheader), 1, flc);
	frames=h_flc.frames;

	printf ("\nHlavicka FLC souboru %s:\n", argv[1]);
	printf ("Delka celeho souboru: %d\n", h_flc.size);
	printf ("Pocet frame: %d\n", h_flc.frames);
	printf ("Velikost filmu: %dx%d\n", h_flc.width, h_flc.height);
	printf ("Hloubka barvy: %d\n", h_flc.color);
	printf ("Rychlost prehravani: %d fps\n", 1000/h_flc.speed);
	printf ("Offset prvniho frame: %d\n", h_flc.offset1);

	fseek (flc, h_flc.offset1, SEEK_SET);
	for (x=0; x<=(frames-1); x++)
	{
		fread (&h_frame, sizeof(frameheader), 1, flc);
		actions=h_frame.actions;
		printf ("\nHlavicka %d framu:\n", x+1);
		printf ("Velikost framu: %d\n", h_frame.size);
		printf ("Pocet akci ve framu: %d\n", h_frame.actions);

		for (y=0; y<=(actions-1); y++)
		{
			pozice = ftell (flc);
			fread (&h_action, sizeof(actionheader), 1, flc);
			fseek (flc, (pozice+h_action.size), SEEK_SET);
			printf ("\nHlavicka %d akce:\n", y+1);
			printf ("Velikost dat pro tuto akci: %d\n", h_action.size);
			printf ("Kod akce: %x\n", h_action.code);
		}
	getch ();
	_clearscreen (_GCLEARSCREEN);
	}

	fclose (flc);
}

		
