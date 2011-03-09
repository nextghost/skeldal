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
#include <inttypes.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <unistd.h>
#include <cstring>
#include "libs/memman.h"
#include <ctime>
#include "libs/system.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "libs/pcx.h"
#include "libs/wav_mem.h"

#define DPMI_INT 0x31
#define LOAD_BUFFER 4096

#undef malloc

#define NON_GETMEM_RESERVED (4*1024)
//char **mman_pathlist=NULL;

char mman_patch=0;

int memman_handle;
static int max_handle=0;
//static FILE *log;

void (*mman_action)(int action)=NULL;

long last_load_size;
Logger *syslog = new Logger;
/*
void get_mem_info(MEMORYSTATUS *mem)
  {
  mem->dwLength=sizeof(*mem);
  GlobalMemoryStatus(mem);
  }
*/


void standard_mem_error(size_t size)
  {
  char buff[256];
  SEND_LOG("(ERROR) Memory allocation error detected, %u bytes missing",size,0);
  Sys_Shutdown();
//  DXCloseMode();
  sprintf(buff,"Memory allocation error\n Application can't allocate %u bytes of memory (%xh)\n",size,memman_handle);
  Sys_ErrorBox(buff);
//  MessageBox(NULL,buff,NULL,MB_OK|MB_ICONSTOP);  
  exit(1);
  }

void load_error(char *filename)
  {
  char buff[256];
  SEND_LOG("(ERROR) Load error detected, system can't load file: %s",filename,0);
  Sys_Shutdown();
//  DXCloseMode();
  sprintf(buff,"Load error while loading file: %s", filename);
  Sys_ErrorBox(buff);
//  MessageBox(NULL,buff,NULL,MB_OK|MB_ICONSTOP);  
  exit(1);
  }

void *getmem(long size) {
	void *ret = malloc(size);

	if (!ret) {
		standard_mem_error(size);
	}

	return ret;
}

// FIXME: get rid of this function
void *load_file(char *filename) {
	long size, *p;
	File file;

	if (mman_action != NULL) {
		mman_action(MMA_READ);
	}

	SEND_LOG("(LOAD) Loading file '%s'", filename, 0);
	file.open(filename);

	if (!file.isOpen()) {
		load_error(filename);
	}

	size = file.size();
	p = (long*)getmem(size);

	if (file.read(p, size) != size) {
		load_error(filename);
	}

	last_load_size = size;
	return p;
}

//--------------- BLOCK MASTER OPERATION SYSTEM --------------------------
//--------------- part: MEMORY MANAGER -----------------------------------

DDLFile datafile, patchfile;
handle_groups _handles;
unsigned long bk_global_counter=0;
char *swap_path;

int8_t ReadStream::readSint8(void) {
	int8_t ret = 0;

	read(&ret, 1);
	return ret;
}

uint8_t ReadStream::readUint8(void) {
	uint8_t ret = 0;

	read(&ret, 1);
	return ret;
}

uint16_t ReadStream::readUint16LE(void) {
	uint8_t a = readUint8();
	uint8_t b = readUint8();

	return (b << 8) | a;
}

uint32_t ReadStream::readUint32LE(void) {
	uint16_t a = readUint16LE();
	uint16_t b = readUint16LE();

	return (b << 16) | a;
}

int16_t ReadStream::readSint16LE(void) {
	return (int16_t)readUint16LE();
}

int32_t ReadStream::readSint32LE(void) {
	return (int32_t)readUint32LE();
}

uint16_t ReadStream::readUint16BE(void) {
	uint8_t a = readUint8();
	uint8_t b = readUint8();

	return (a << 8) | b;
}

uint32_t ReadStream::readUint32BE(void) {
	uint16_t a = readUint16BE();
	uint16_t b = readUint16BE();

	return (a << 16) | b;
}

int16_t ReadStream::readSint16BE(void) {
	return (int16_t)readUint16BE();
}

int32_t ReadStream::readSint32BE(void) {
	return (int32_t)readUint32BE();
}

MemoryReadStream *ReadStream::readStream(unsigned size) {
	unsigned char *ptr = new unsigned char[size];
	MemoryReadStream *ret;

	size = read(ptr, size);
	ret = new MemoryReadStream(ptr, size);
	delete[] ptr;
	return ret;
}

MemoryReadStream::MemoryReadStream(const void *ptr, unsigned len) : _data(NULL),
	_length(0), _pos(0) {

	_length = len;
	_data = new unsigned char[_length + 1];
	memcpy(_data, ptr, _length);
	_data[_length] = 0;
}

MemoryReadStream::MemoryReadStream(const MemoryReadStream &src) : _data(NULL),
	_length(src._length), _pos(src._pos) {

	_data = new unsigned char[_length + 1];
	memcpy(_data, src._data, _length);
	_data[_length] = 0;
}

MemoryReadStream::~MemoryReadStream(void) {
	delete[] _data;
}

const MemoryReadStream &MemoryReadStream::operator=(const MemoryReadStream &src) {
	MemoryReadStream tmp(src);
	unsigned tmp1;
	unsigned char *ptr;

	ptr = _data;
	_data = tmp._data;
	tmp._data = ptr;

	tmp1 = _length;
	_length = tmp._length;
	tmp._length = tmp1;

	tmp1 = _pos;
	_pos = tmp._pos;
	tmp._pos = tmp1;

	return *this;
}

size_t MemoryReadStream::read(void *buf, size_t size) {
	size_t len;

	if (_pos >= _length) {
		_pos = _length + 1;
		return 0;
	}

	len = size < _length - _pos ? size : _length - _pos;
	memcpy(buf, _data + _pos, len);
	_pos = len < size ? _length + 1 : _pos + len;

	return len;
}

char *MemoryReadStream::readLine(char *buf, size_t size) {
	unsigned i;

	if (_pos >= _length) {
		_pos = _length + 1;
		return NULL;
	}

	for (i = 0; i < size - 1 && i + _pos < _length; i++) {
		if (_data[_pos + i] == '\n') {
			i++;
			break;
		}
	}

	memcpy(buf, _data + _pos, i);
	_pos = _pos + i >= _length && i < size - 1 && buf[i-1] != '\n' ? _length + 1 : _pos + i;

	buf[i] = '\0';
	if (buf[i-1] == '\n' && buf[i-2] == '\r') {
		buf[i-2] = '\n';
		buf[i-1] = '\0';
	} else if (buf[i-1] == '\r' && _pos < _length && _data[_pos] == '\n') {
		buf[i-1] = '\n';
		_pos++;
	}

	return buf;
}

const char *MemoryReadStream::readCString(void) {
	const char *ptr = (char*)(_data + _pos);
	int len;

	if (_pos >= _length) {
		_pos = _length + 1;
		return NULL;
	}

	len = strlen(ptr) + 1;
	_pos += len;

	if (_pos > _length) {
		_pos = _length;
	}

	return ptr;
}

bool MemoryReadStream::eos(void) const {
	return _pos > _length;
}

void MemoryReadStream::seek(long offset, int whence) {
	switch (whence) {
	case SEEK_SET:
		_pos = offset >= 0 && offset <= _length ? offset : _length;
		break;

	case SEEK_CUR:
		_pos += offset;
		_pos = _pos > _length ? _length : _pos;
		break;

	case SEEK_END:
		_pos = _length + offset;
		_pos = _pos > _length ? _length : _pos;
	}
}

File::File(void) : _file(NULL), _name(NULL) { }

File::File(const char *filename) : _file(NULL), _name(NULL) {
	open(filename);
}

File::~File(void) {
	close();
}

void File::open(const char *filename) {
	close();

	_file = fopen(filename, "r");

	if (_file) {
		_name = new char[strlen(filename) + 1];
		strcpy(_name, filename);
	}
}

void File::close(void) {
	if (!_file) {
		return;
	}

	fclose(_file);
	delete[] _name;

	_file = NULL;
	_name = NULL;
}

void File::seek(long offset, int whence) {
	assert(_file);
	fseek(_file, offset, whence);
}

long File::pos(void) const {
	assert(_file);
	return ftell(_file);
}

long File::size(void) const {
	long tmp, ret;
	assert(_file);

	tmp = ftell(_file);
	fseek(_file, 0, SEEK_END);
	ret = ftell(_file);
	fseek(_file, tmp, SEEK_SET);
	return ret;
}

size_t File::read(void *buf, size_t size) {
	assert(_file);
	return fread(buf, 1, size, _file);
}

char *File::readLine(char *buf, size_t size) {
	int idx;
	assert(_file);

	if (!fgets(buf, size, _file)) {
		return NULL;
	}

	idx = strlen(buf);

	if (buf[idx-1] == '\n' && buf[idx-2] == '\r') {
		buf[idx-2] = '\n';
		buf[idx-1] = '\0';
	} else if (buf[idx-1] == '\r') {
		unsigned tmp = readUint8();

		if (eos()) {
			return buf;
		} else if (tmp == '\n') {
			buf[idx-1] = '\n';
		} else {
			seek(-1, SEEK_CUR);
		}
	}

	return buf;
}

bool File::eos() const {
	assert(_file);
	return feof(_file);
}

void WriteStream::writeSint8(int8_t data) {
	write(&data, 1);
}

void WriteStream::writeUint8(uint8_t data) {
	write(&data, 1);
}

void WriteStream::writeUint16LE(uint16_t data) {
	writeUint8(data & 0xff);
	writeUint8(data >> 8);
}

void WriteStream::writeUint32LE(uint32_t data) {
	writeUint16LE(data & 0xffff);
	writeUint16LE(data >> 16);
}

void WriteStream::writeSint16LE(int16_t data) {
	writeUint16LE((uint16_t)data);
}

void WriteStream::writeSint32LE(int32_t data) {
	writeUint32LE((uint32_t)data);
}

void WriteStream::writeUint16BE(uint16_t data) {
	writeUint8(data >> 8);
	writeUint8(data & 0xff);
}

void WriteStream::writeUint32BE(uint32_t data) {
	writeUint16BE(data >> 16);
	writeUint16BE(data & 0xffff);
}

void WriteStream::writeSint16BE(int16_t data) {
	writeUint16BE((uint16_t)data);
}

void WriteStream::writeSint32BE(int32_t data) {
	writeUint32BE((uint32_t)data);
}

WriteFile::WriteFile(void) : _file(NULL), _name(NULL) { }

WriteFile::WriteFile(const char *filename) : _file(NULL), _name(NULL) {
	open(filename);
}

WriteFile::~WriteFile(void) {
	close();
}

void WriteFile::open(const char *filename) {
	close();

	_file = fopen(filename, "w");

	if (_file) {
		_name = new char[strlen(filename) + 1];
		strcpy(_name, filename);
	}
}

void WriteFile::close(void) {
	if (!_file) {
		return;
	}

	fclose(_file);
	delete[] _name;

	_file = NULL;
	_name = NULL;
}

size_t WriteFile::write(const void *buf, size_t size) {
	assert(_file);
	return fwrite(buf, 1, size, _file);
}

MemoryWriteStream::MemoryWriteStream(unsigned alloc) :
	_data(new unsigned char[alloc]), _size(alloc), _pos(0) {

	memset(_data, 0, _size * sizeof(unsigned char));
}

MemoryWriteStream::~MemoryWriteStream(void) {
	delete[] _data;
}

size_t MemoryWriteStream::write(const void *buf, size_t size) {
	if (_pos + size >= _size) {
		unsigned char *tmp;

		_size = _pos + size > 2 * _size ? _pos + size : 2 * _size;
		tmp = new unsigned char[_size];
		memcpy(tmp, _data, _pos);
		memset(tmp + _pos + size, 0, _size - _pos - size);
		delete[] _data;
		_data = tmp;
	}

	memcpy(_data + _pos, buf, size);
	_pos += size;
	return size;
}

BitStream::BitStream(ReadStream &stream) : _stream(stream), _lastByte(0),
	_bitsLeft(0) {

}

BitStream::~BitStream(void) {

}

unsigned BitStream::readBitsLE(unsigned bits) {
	unsigned ret = 0, shift = 0;
	static const unsigned mask[] = {0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff};
	assert(bits <= 8 * sizeof(unsigned));

	if (_bitsLeft >= bits) {
		ret = _lastByte & mask[bits];
		_lastByte >>= bits;
		_bitsLeft -= bits;
		return ret;
	}

	ret = _lastByte;
	shift = _bitsLeft;
	bits -= shift;
	_lastByte = 0;
	_bitsLeft = 0;

	for (; bits >= 8; bits -= 8, shift += 8) {
		ret |= _stream.readUint8() << shift;
	}

	if (bits) {
		_lastByte = _stream.readUint8();
		_bitsLeft = 8 - bits;
		ret |= (_lastByte & mask[bits]) << shift;
		_lastByte >>= bits;
	}

	return ret;
}

DDLFile::DDLFile(void) : _grptable(NULL), _grpsize(0), _namesize(0),
	_nametable(NULL) {
}

DDLFile::~DDLFile(void) {
	close();
}

void DDLFile::open(const char *filename) {
	int i;
	char buf[DDLEntryNameSize + 1];

	close();

	_file.open(filename);

	if (!_file.isOpen()) {
		fprintf(stderr, "Could not open file %s\n", filename);
		return;
	}

	i = _file.readUint32LE();
	_grpsize = _file.readSint32LE() / DDLGroupSize;
	_grptable = new DDLGroup[_grpsize];

	_grptable[0].id = i;
	_grptable[0].start = 0;

	for (i = 1; i < _grpsize; i++) {
		_grptable[i].id = _file.readSint32LE();
		_grptable[i].start = (_file.readSint32LE() - _grpsize * DDLGroupSize) / DDLEntrySize;
	}

	_file.read(buf, DDLEntryNameSize);
	buf[DDLEntryNameSize] = '\0';
	i = _file.readSint32LE();
	_namesize = (i - _grpsize * DDLGroupSize) / DDLEntrySize;
	_nametable = new DDLEntry[_namesize];

	strcpy(_nametable[0].name, buf);
	_nametable[0].offset = i;

	for (i = 1; i < _namesize; i++) {
		_file.read(_nametable[i].name, DDLEntryNameSize);
		_nametable[i].name[DDLEntryNameSize] = '\0';
		_nametable[i].offset = _file.readSint32LE();
	}
}

void DDLFile::close(void) {
	if (!_file.isOpen()) {
		return;
	}

	_file.close();
	delete[] _grptable;
	delete[] _nametable;

	_grptable = NULL;
	_nametable = NULL;
	_grpsize = 0;
	_namesize = 0;
}

MemoryReadStream *DDLFile::readFile(int group, const char *name) {
	assert(_nametable);
	int entry = findEntry(group, name);
	unsigned size;
	void *ret;

	assert(entry >= 0);

	_file.seek(_nametable[entry].offset, SEEK_SET);
	size = _file.readSint32LE();
	return _file.readStream(size);
}

int DDLFile::findEntry(int group, const char *name) const {
	int i;
	assert(_nametable);

	for (i = 0; i < _grpsize; i++) {
		if (_grptable[i].id == group) {
			break;
		}
	}

	if (i >= _grpsize) {
		return -1;
	}

	for (i = _grptable[i].start; i < _namesize; i++) {
		if (!strcmp(_nametable[i].name, name)) {
			return i;
		}
	}

	return -1;
}

int DDLFile::getOffset(unsigned id) const {
	assert(id < _namesize);

	return _nametable[id].offset;
}

DataBlock::~DataBlock(void) {

}

RawData::~RawData(void) {
	free(data);
}

DataBlock *preloadStream(SeekableReadStream &stream) {
	return stream.readStream(stream.size());
}

Logger::~Logger(void) {
	flush();
}

void StdLogger::write(const char *format, ...) {
	va_list args;

	va_start(args, format);
	fprintf(stderr, "%s ", get_time_str());
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end(args);
}

void StdLogger::flush(void) {
	fflush(stderr);
}

static int test_file_exist_DOS(int group, const char *filename)
  {
/*
     char *f;

     f=alloca(strlen(mman_pathlist[group])+strlen(filename)+1);
     strcpy(f,mman_pathlist[group]);
     strcat(f,filename);
     if (access(f,0)) return 0;
     return 1;
*/

	return Sys_FileExists(Sys_FullPath(group, filename));
  }

int get_file_entry(int group,char *name) {
	int i;
	char ex;

	if (mman_patch) {
		ex = test_file_exist_DOS(group, name);
	} else {
		ex = 0;
	}

	if (ex || !datafile.isOpen()) {
		return 0;
	}

	if (patchfile.isOpen()) {
		i = patchfile.findEntry(group,name);

		if (i != -1) {
			return -datafile.getOffset(i);
		}
	}

	i = datafile.findEntry(group,name);

	if (i == -1) {
		return 0;
	}

	return datafile.getOffset(i);
}

THANDLE_DATA *get_handle(int handle) {
	int group, list;

	group = handle / BK_MINOR_HANDLES;
	list = handle % BK_MINOR_HANDLES;

	if (_handles[group] == NULL) {
		_handles[group] = (handle_list *)getmem(sizeof(handle_list));
		memset(_handles[group], 0, sizeof(handle_list));
	}

	if (handle > max_handle) {
		max_handle = handle;
	}

	return *_handles[group] + list;
}

THANDLE_DATA *kill_block(int handle) {
	THANDLE_DATA *h;

	h = get_handle(handle);

	if (h->status == BK_NOT_USED) {
		return h;
	}

	if (h->flags & BK_LOCKED) {
		SEND_LOG("(ERROR) Unable to kill block! It is LOCKED! '%-.12s' (%04X)", h->src_file, handle);
		return NULL;
	}

	SEND_LOG("(KILL) Killing block '%-.12s' (%04X)", h->src_file, handle);

	if (h->status == BK_SAME_AS) {
		return h;
	}

	if (h->status == BK_PRESENT) {
		delete h->blockdata;
	}

	h->status = BK_NOT_LOADED;
	return h;
}

THANDLE_DATA *zneplatnit_block(int handle)
  {
  THANDLE_DATA *h;

  h=kill_block(handle);
  if (h->status==BK_SAME_AS)
     return zneplatnit_block(h->seekpos);
  if (h->src_file[0]) h->seekpos=get_file_entry(h->path,h->src_file);
  return h;
  }

// filename= Jmeno datoveho souboru nebo NULL pak se pouzije DOS
// swp je cesta do TEMP adresare
void init_manager(char *filename, char *swp) {
	memset(_handles, 0, sizeof(_handles));

	if (filename) {
		datafile.open(filename);
	}
}

int find_same(const char *name,DataBlock *(*decomp)(SeekableReadStream&))
  {
  THANDLE_DATA *p;
  int i,j;

//  decomp;
  if (name[0]==0) return -1;
  for(i=0;i<BK_MAJOR_HANDLES;i++)
     if (_handles[i]!=NULL)
     {     
     p=*_handles[i];
     for(j=0;j<BK_MINOR_HANDLES;j++)
        if ((!strncmp(p[j].src_file,name,12))&& (p[j].loadproc==decomp)) return i*BK_MINOR_HANDLES+j;
     }
  return -1;
  }

int find_handle(const char *name, DataBlock *(*decomp)(SeekableReadStream&))
  {
  return find_same(name,decomp);
  }

int test_file_exist(int group,const char *filename)
  {
  if (datafile.findEntry(group,filename)==-1) return test_file_exist_DOS(group,filename);
  return 1;
  }

THANDLE_DATA *def_handle(int handle, const char *filename, DataBlock *(*decompress)(SeekableReadStream&), char path) {
	THANDLE_DATA *h;
	int i;

	i = find_same(filename, decompress);
	h = get_handle(handle);

	if (i == handle) {
		return h;
	}

	if (kill_block(handle) == NULL) {
		SEND_LOG("(ERROR) File/Block can't be registred, handle is already in use '%-.12s' handle %04X", filename, handle);
		return NULL;
	}

	if (i != -1 && i != handle) {
		h->status = BK_SAME_AS;
		h->seekpos = i;
		return h;
	}

	strncpy(h->src_file, filename, 12);
	h->src_file[12] = '\0';
	h->seekpos = 0;
	strupr(h->src_file);
	h->loadproc = decompress;

	if (filename[0]) {
		h->seekpos = get_file_entry(path, h->src_file);
	}

	SEND_LOG("(REGISTER) File/Block registred '%-.12s' handle %04X", h->src_file, handle);
	SEND_LOG("(REGISTER) Seekpos=%d", h->seekpos, 0);
	h->flags = 0;
	h->path = path;

	if (h->status != BK_DIRLIST) {
		h->status = BK_NOT_LOADED;
	}

	h->counter = bk_global_counter++;
	return h;
}

SeekableReadStream *afile(const char *filename, int group) {
	char *c, *d;
	long entr;
	SeekableReadStream *ret;

	d = (char*)alloca(strlen(filename) + 1);
	strcpy(d, filename);
	strupr(d);

	if (mman_patch && test_file_exist_DOS(group, d)) {
		entr = 0;
	} else {
		entr = get_file_entry(group, d);
	}

	if (entr != 0) {
//		 int hnd;
		FILE *hnd;
		SEND_LOG("(LOAD) Afile is loading file '%s' from group %d",d,group);

		if (entr < 0) {
			ret = patchfile.readFile(group, d);
		} else {
			ret = datafile.readFile(group, d);
		}
	} else {
		SEND_LOG("(LOAD) Afile is loading file '%s' from disk", d, group);
		ret = new File(Sys_FullPath(group, filename));
	}
/*
  else if (mman_pathlist!=NULL)
     {
     SEND_LOG("(LOAD) Afile is loading file '%s' from disk",d,group);
     c=alloca(strlen(filename)+strlen(mman_pathlist[group])+2);
     c=strcat(strcpy(c,mman_pathlist[group]),filename);
     p=load_file(c);
     *blocksize=last_load_size;
     }
  else return NULL;
*/
	return ret;
}

DataBlock *ablock(int handle) {
	THANDLE_DATA *h;

sem:
	h = get_handle(handle);

	if (memman_handle != handle || h->status != BK_PRESENT) {
		h->counter = bk_global_counter++;
	}

	memman_handle = handle;

	if (h->status == BK_SAME_AS) {
		handle = h->seekpos;
		goto sem;
	}

	if (h->status == BK_PRESENT) {
		return h->blockdata;
	}

	if (h->status == BK_DIRLIST) {
		delete h->blockdata;
		h->status = BK_NOT_LOADED;
	}

	if (h->status == BK_NOT_LOADED) {
		void *p;
		long s;
		char *c;

		SEND_LOG("(LOAD) Loading file as block '%-.12s' %04X",h->src_file,handle);

		assert(h->loadproc && "Missing loadproc");

		if (h->seekpos == 0) {
			if (h->src_file[0] != 0) {
				File file;

				if (mman_action != NULL) {
					mman_action(MMA_READ);
				}

				c = Sys_FullPath(h->path, "");
				s = strlen(c);
				strcpy(c + s, h->src_file);
				c[s + 12] = '\0';
				file.open(c);
				h->blockdata = h->loadproc(file);
			} else {
				MemoryReadStream tmp(NULL, 0);
				h->blockdata = h->loadproc(tmp);
			}

			h->status = BK_PRESENT;
			return h->blockdata;
		} else {
			int entr = h->seekpos;//,hnd;
			MemoryReadStream *stream;

			if (mman_action != NULL) {
				mman_action(MMA_READ);
			}

			if (entr < 0) {
				stream = patchfile.readFile(h->path, h->src_file);
			} else {
				stream = datafile.readFile(h->path, h->src_file);
			}

			h->blockdata = h->loadproc(*stream);
			h->status = BK_PRESENT;
			delete stream;
			return h->blockdata;
		}
	}

	return NULL;
}

void alock(int handle)
  {
  THANDLE_DATA *h;

  h=get_handle(handle);
  if (!h->lockcount)
     {
     h->flags|=BK_LOCKED;
     if (h->status==BK_SAME_AS) alock(h->seekpos);
     }
  h->lockcount++;
  //SEND_LOG("(LOCK) Handle locked %04X (count %d)",handle,h->lockcount);
  }

void aunlock(int handle)
  {
  THANDLE_DATA *h;
  h=get_handle(handle);
  if (h->lockcount) h->lockcount--;else return;
  if (!h->lockcount)
     {
     h->flags&=~BK_LOCKED;
     if (h->status==BK_SAME_AS) aunlock(h->seekpos);
     }
  //SEND_LOG("(LOCK) Handle unlocked %04X (count %d)",handle,h->lockcount);
  }
void aswap(int handle)
  {
  THANDLE_DATA *h;

  h=get_handle(handle);
  h->flags|=BK_SWAPABLE;
  if (h->status==BK_SAME_AS) aswap(h->seekpos);
  }

void aunswap(int handle)
  {
  THANDLE_DATA *h;

  h=get_handle(handle);
  h->flags|=BK_SWAPABLE;
  if (h->status==BK_SAME_AS) aunswap(h->seekpos);
  }

void apreload(int handle)
  {
  THANDLE_DATA *h;


  h=get_handle(handle);
  if (h->src_file[0] && h->status!=BK_SAME_AS)
     {
     if (!(h->flags & BK_PRELOAD) || !(h->flags & BK_HSWAP)) h->flags|=BK_SWAPABLE | BK_PRELOAD;
     ablock(handle);
     }
  }

static long *apr_sign=NULL;
static long max_sign;

void apreload_sign(int handle,int max_handle)
  {
  THANDLE_DATA *h;
  if (apr_sign==NULL)
     {
     apr_sign=NewArr(long,max_handle);
     memset(apr_sign,0x7f,sizeof(long)*max_handle);
     max_sign=max_handle;
     }
  if (handle>=max_sign)
     {
     apreload(handle);
     return;
     }
  h=get_handle(handle);
  if (h->src_file[0] && h->status!=BK_SAME_AS && h->status!=BK_NOT_USED)
     if (!(h->flags & BK_PRELOAD) || !(h->flags & BK_HSWAP)) apr_sign[handle]=h->seekpos;
  }

static int apreload_sort(const void *val1,const void *val2)
  {
  long vl1,vl2;

  vl1=apr_sign[*(uint16_t *)val1];
  vl2=apr_sign[*(uint16_t *)val2];
  return (vl1>vl2)-(vl1<vl2);
  }

void apreload_start(void (*percent)(int cur, int max)) {
	uint16_t *p;
	int i;
	int c, z;

	p = NewArr(uint16_t, max_sign);

	for (i = 0; i < max_sign; i++) {
		p[i] = i;
	}

	qsort(p, max_sign, sizeof(uint16_t), apreload_sort);

	for (i = 0, c = 0; i < max_sign; i++) {
		if (apr_sign[p[i]] == 0x7f7f7f7f) {
			p[i] = 0xffff;
		} else {
			c++;
		}
	}

	for (i = 0, z = 0; i < max_sign; i++) {
		if (p[i] != 0xffff) {
			apreload(p[i]);
			percent(z++, c);
		}
	}

	free(apr_sign);
	free(p);
	apr_sign=NULL;
}

void undef_handle(int handle)
  {
  THANDLE_DATA *h;

  h=get_handle(handle);
  if (h->status!=BK_NOT_USED)
     {
     if (kill_block(handle)==NULL) return;
     SEND_LOG("(REGISTER) File/Block unregistred %04X (%-.12s)",handle,h->src_file);
     }
  h->src_file[0]=0;
  h->seekpos=0;
  h->flags=0;
  h->status=BK_NOT_USED;
  }

void close_manager()
  {
  int i,j;

  for(i=0;i<BK_MAJOR_HANDLES;i++) if (_handles[i]!=NULL)
     {
     for(j=0;j<BK_MINOR_HANDLES;j ++) undef_handle(i*BK_MINOR_HANDLES+j);
     free(_handles[i]);
     }
//  fclose(log);
  max_handle=0;
  }

void memstats(void) {
	unsigned i, major, minor;
	size_t tmp, total, fadetex, paltex, fulltex, sound;
	unsigned totalcnt, fadetexcnt, paltexcnt, fulltexcnt, soundcnt;
	DataBlock *block;
	Texture *tex;

	total = fadetex = paltex = fulltex = sound = 0;
	totalcnt = fadetexcnt = paltexcnt = fulltexcnt = soundcnt = 0;

	for (i = 0; i <= max_handle; i++) {
		major = i / BK_MINOR_HANDLES;
		minor = i % BK_MINOR_HANDLES;

		if (!_handles[major]) {
			i += BK_MINOR_HANDLES - minor - 1;
			continue;
		}

		if (_handles[major][0][minor].status != BK_PRESENT) {
			continue;
		}

		block = _handles[major][0][minor].blockdata;
		tmp = block->memsize();
		total += tmp;
		totalcnt++;

		if (dynamic_cast<TextureFade*>(block)) {
				fadetex += tmp;
				fadetexcnt++;
		} else if ((tex = dynamic_cast<Texture*>(block))) {
			if (tex->palette()) {
				paltex += tmp;
				paltexcnt++;
			} else {
				fulltex += tmp;
				fulltexcnt++;
			}
		} else if (dynamic_cast<SoundSample*>(block)) {
			sound += tmp;
			soundcnt++;
		}
	}

	fprintf(stderr, "Total memory used by data: %u items, %z bytes\n", totalcnt, total);
	fprintf(stderr, "Fade textures: %u items, %z bytes\n", fadetexcnt, fadetex);
	fprintf(stderr, "Palette textures: %u items, %z bytes\n", paltexcnt, paltex);
	fprintf(stderr, "Full color textures: %u items, %z bytes\n", fulltexcnt, fulltex);
	fprintf(stderr, "Sound samples: %u items, %z bytes\n", soundcnt, sound);
	fprintf(stderr, "Other: %u items, %z bytes\n", totalcnt - fadetexcnt - paltexcnt - fulltexcnt - soundcnt, total - fadetex - paltex - fulltex - sound);
}

//------------------------------------------------------------
/*static void block()
  {
  static MEMINFO inf;
  void *c;
  static counter=0;

  get_mem_info(&inf);
  c=malloc(inf.LargestBlockAvail-1024);
  counter++;
  printf("%d. %d\n",counter,inf.LargestBlockAvail);
  if (c!=NULL) block();
  counter--;
  free(c);
  }*/

/*
void display_status()
  {
  int i,j;
  THANDLE_DATA *h;
  char names[][5]={"MISS","*ok*","SWAP","????","PTR "};
  char flags[]={"LS*PH"};
  char copys[6]="     ";
  char nname[14];
  long total_data=0;
  long total_mem=0;
  MEMORYSTATUS mem;
  int ln=0;

  //block();
  //getchar();
  memset(nname,0,sizeof(nname));
  for(i=0;i<BK_MAJOR_HANDLES;i++)
     if (_handles[i]!=NULL)
     {
     for(j=0;j<BK_MINOR_HANDLES;j++)
        {
        h=get_handle(i*BK_MINOR_HANDLES+j);
        if (h->status!=BK_NOT_USED && h->status!=BK_SAME_AS)
           {
           int k;

           for(k=0;k<5;k++) copys[k]=h->flags & (1<<k)?flags[k]:'.';
           if (h->src_file[0]) strncpy(nname,h->src_file,12);else strcpy(nname,"<local>");
           printf("%04Xh ... %12s %s %s %08Xh %6d %10d %6d \n",i*BK_MINOR_HANDLES+j,
           nname,names[h->status-1],
           copys,(int)h->blockdata,h->size,h->counter,h->lockcount);
           ln++;
           total_data+=h->size;
           if(h->status==BK_PRESENT)total_mem+=h->size;
           if (ln%24==23)
              {
              char c;
              printf("Enter to continue, q-quit:");
              c=getchar();
              if (c=='q' || c=='Q')
                 {
                 while (getchar()!='\n');
                 return;
                 }
              }
           }
        }

     }
  get_mem_info(&mem);
  printf("Data: %7d KB, Loaded: %7d KB, Largest free: %7d KB",total_data/1024,total_mem/1024,mem.dwAvailPageFile/1024);
  while (getchar()!='\n');
  }
*/

void *grealloc(void *p,long size)
  {
// TODO: I think it's time to get rid of this crap
/*
  void *q;
  long scop;

  if (!size)
     {
     free(p);
     return NULL;
     }*/
  /*q=realloc(p,size);
  if (q!=NULL)
     {
     SEND_LOG("(ALLOC) **** Realloc: New %p size %d\n",q,*((long *)q-1));
     return q;
     }*/
/*  q=getmem(size);
  if (p==NULL) return q;
  scop=_msize(p);
  if (scop>size) scop=size;
  memmove(q,p,scop);
  free(p);
  return q;
*/
	void *ret = realloc(p, size);
	if (!ret) {
		standard_mem_error(size);
	}

	return ret;
  }

char add_patch_file(char *filename) {
	if (patchfile.isOpen()) {
		return 2;
	}

	if (!datafile.isOpen()) {
		return 3;
	}

	patchfile.open(filename);

	if (!patchfile.isOpen()) {
		return 1;
	}

	return 0;
}

char *get_time_str()
  {
  time_t long_time;
  struct tm *newtime;
  static char text[20];


  time( &long_time );                /* Get time as long integer. */
  newtime = localtime( &long_time ); /* Convert to local time. */

  sprintf(text,"%02d:%02d:%02d",newtime->tm_hour,newtime->tm_min,newtime->tm_sec);
  return text;
  }

long get_handle_size(int handle) {
	THANDLE_DATA *h;

	h = get_handle(handle);

	if (h->status == BK_NOT_USED) {
		return -1;
	}

	return h->size;
}

/*void *malloc(size_t size)
  {
  static char zavora=1;
  void *c;

  c=_nmalloc(size);
  if (log!=NULL && zavora)
     {
     zavora=0;
     fprintf(log,"Alloc: %p size %d\n",c,*((long *)c-1));
     zavora=1;
     }
  return c;
  }
 */
