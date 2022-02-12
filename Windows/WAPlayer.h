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
 *  Last commit made by: $Id: WAPlayer.h 7 2008-01-14 20:14:25Z bredysoft $
 */
#pragma once

#include "WAInputPlugin.h"

class WAPlayer
{
	WAInputPlugin *_pluginList;
	int _nPlugins;

	WAPlayer &operator=(const WAPlayer &other);
	WAPlayer(const WAPlayer &other);
	public:
	WAPlayer(void);
	~WAPlayer(void);



	void ClearList();
	void LoadPlugins(const char *path);
	WAInputPlugin *SelectBestPlugin(const char *songName);

	bool EnumPlugins(bool (*EnumProc)(WAPlayer &player, WAInputPlugin &plugin, void *context), void *context);
};
