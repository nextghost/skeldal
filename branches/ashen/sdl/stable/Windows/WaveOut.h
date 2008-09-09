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
 *  Last commit made by: $Id: WaveOut.h 7 2008-01-14 20:14:25Z bredysoft $
 */
// WaveOut.h: interface for the WaveOut class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WAVEOUT_H__1390C856_FA5C_4E84_B98D_4CC05086733D__INCLUDED_)
#define AFX_WAVEOUT_H__1390C856_FA5C_4E84_B98D_4CC05086733D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAXBUFFERS 64

#include "WAInputPlugin.h"
#include <mmsystem.h>

class WaveOut: public IWAOutput
{
	HWAVEOUT _device;
	bool _paused;
	LPWAVEHDR _buffers[MAXBUFFERS];
	int _wrpos;
	HANDLE _event;
	DWORD _played;
	DWORD _written;
	DWORD _persec;

	int NextBuffer();
	int _pending;

	public:
	WaveOut();
	virtual ~WaveOut();

	virtual int Open(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms);
	virtual void Close(); 
	virtual int Write(const char *buf, int len);
	virtual int CanWrite();
	virtual int IsPlaying();
	virtual int Pause(int pause);
	virtual void SetVolume(int volume);
	virtual void SetPan(int pan);
	virtual void Flush(int t);	
	virtual int GetOutputTime();
	virtual int GetWrittenTime();
	virtual unsigned long AddRef() {return 1;}
	virtual unsigned long Release() {return 0;}
};



#endif // !defined(AFX_WAVEOUT_H__1390C856_FA5C_4E84_B98D_4CC05086733D__INCLUDED_)
