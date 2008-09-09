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
 *  Last commit made by: $Id: WinAmpInterface.cpp 7 2008-01-14 20:14:25Z bredysoft $
 */
// WinAmpInterface.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "WAInputPlugin.h"
#include "WaveOut.h"
#include "waplayer.h"


int main(int argc, char* argv[])
{
	WAPlayer player;
	player.LoadPlugins("C:\\Program Files\\Winamp\\Plugins");
	const char *name ="\\\\bredy-doma\\d$\\MUSIC\\S3M\\s3m\\CTGOBLIN.S3M";
	WAInputPlugin *plug = player.SelectBestPlugin(name);

	WaveOut output;
	plug->AttachOutput(&output);
	plug->Play(name);
	while (kbhit() == 0 && !plug->IsFinished()) Sleep(500);
	plug->Stop();
	plug->AttachOutput(0);



	return 0;
}
