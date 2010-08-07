#include <cstdlib>
#include <cstring>
#include <cassert>

#include "memman.h"

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

	size = read(ptr, size);
	return new MemoryReadStream(ptr, size);
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

void *DDLFile::readFile(int group, const char *name, long &size) {
	assert(_nametable);
	int entry = findEntry(group, name);
	void *ret;

	assert(entry >= 0);

	_file.seek(_nametable[entry].offset, SEEK_SET);
	size = _file.readSint32LE();
	ret = malloc(size); // FIXME: change to something else later
	_file.read(ret, size);

	return ret;
}

MemoryReadStream *DDLFile::readFile(unsigned id) {
	assert(id < _namesize);
	int size;

	_file.seek(_nametable[id].offset, SEEK_SET);
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

const char *DDLFile::getName(unsigned id) const {
	assert(id < _namesize);

	return _nametable[id].name;
}
