#ifndef MEMMAN_H_
#define MEMMAN_H_

#include <cstdio>
#include <inttypes.h>

class MemoryReadStream;

class ReadStream {
public:
	virtual int8_t readSint8(void);
	virtual uint8_t readUint8(void);

	virtual int16_t readSint16LE(void);
	virtual uint16_t readUint16LE(void);
	virtual int32_t readSint32LE(void);
	virtual uint32_t readUint32LE(void);

	virtual int16_t readSint16BE(void);
	virtual uint16_t readUint16BE(void);
	virtual int32_t readSint32BE(void);
	virtual uint32_t readUint32BE(void);

	virtual size_t read(void *buf, size_t size) = 0;
	virtual char *readLine(char *buf, size_t size) = 0;

	virtual bool eos() const { return false; }

	virtual ~ReadStream() { }

	MemoryReadStream *readStream(unsigned size);
};

class SeekableReadStream : public ReadStream {
public:
	virtual void seek(long offset, int whence) = 0;
	virtual long pos(void) const = 0;
	virtual long size(void) const = 0;

	~SeekableReadStream() { }
};

class MemoryReadStream : public SeekableReadStream {
private:
	unsigned char *_data;	// always add extra null byte at the end
	unsigned _length, _pos;

public:
	MemoryReadStream(const void *ptr, unsigned len);
	MemoryReadStream(const MemoryReadStream &src);
	~MemoryReadStream(void);

	const MemoryReadStream &operator=(const MemoryReadStream &src);

	size_t read(void *buf, size_t size);
	char *readLine(char *buf, size_t size);
	const char *readCString(void);
	bool eos(void) const;

	void seek(long offset, int whence);
	long pos(void) const { return _pos; }
	long size(void) const { return _length; }
	const void *dataPtr(void) const { return _data; }
};

class File : public SeekableReadStream {
private:
	FILE *_file;
	char *_name;

	// Do not implement
	File(const File &src);
	const File &operator=(const File &src);
public:
	File(void);
	explicit File(const char *filename);
	~File(void);

	// inherited methods
	void seek(long offset, int whence);
	long pos(void) const;
	long size(void) const;
	size_t read(void *buf, size_t size);
	char *readLine(char *buf, size_t size);

	// new methods
	void open(const char *filename);
	void close(void);
	inline const char *getName(void) const { return _name; }
	inline bool isOpen(void) const { return _file; }
	bool eos() const;
};

class WriteStream {
public:
	virtual void writeSint8(int8_t data);
	virtual void writeUint8(uint8_t data);

	virtual void writeSint16LE(int16_t data);
	virtual void writeUint16LE(uint16_t data);
	virtual void writeSint32LE(int32_t data);
	virtual void writeUint32LE(uint32_t data);

	virtual void writeSint16BE(int16_t data);
	virtual void writeUint16BE(uint16_t data);
	virtual void writeSint32BE(int32_t data);
	virtual void writeUint32BE(uint32_t data);

	virtual size_t write(const void *buf, size_t size) = 0;

	virtual ~WriteStream() { }
};

class WriteFile : public WriteStream {
private:
	FILE *_file;
	char *_name;

	// Do not implement
	WriteFile(const File &src);
	const WriteFile &operator=(const File &src);

public:
	WriteFile(void);
	explicit WriteFile(const char *filename);
	~WriteFile(void);

	void open(const char *filename);
	void close(void);
	inline const char *getName(void) const { return _name; }
	inline bool isOpen(void) const { return _file; }

	size_t write(const void *buf, size_t size);
};

class BitStream {
private:
	ReadStream &_stream;
	uint8_t _lastByte;
	int _bitsLeft;

public:
	BitStream(ReadStream &stream);
	~BitStream(void);

	unsigned readBitsLE(unsigned bits);
	bool eos(void) const { return _stream.eos(); }
};

class DDLFile {
private:
	static const int DDLEntrySize = 16;
	static const int DDLEntryNameSize = 12;
	static const int DDLGroupSize = 8;

	typedef struct {
		char name[DDLEntryNameSize + 1];
		int offset;
	} DDLEntry;

	typedef struct {
		int id, start;
	} DDLGroup;

	int _grpsize, _namesize;
	DDLGroup *_grptable;
	DDLEntry *_nametable;
	File _file;

	// Do not implement
	DDLFile(const DDLFile &src);
	const DDLFile &operator=(const DDLFile &src);
public:
	DDLFile(void);
	~DDLFile(void);

	void open(const char *filename);
	void close(void);

	// FIXME: rewrite to return MemoryReadStream later
	void *readFile(int group, const char *name, long &size);
	MemoryReadStream *readFile(unsigned id);

	int findEntry(int group, const char *name) const;
	int getOffset(unsigned id) const;
	const char *getName(unsigned id) const;

	bool isOpen(void) const { return _nametable; }
	int fileCount(void) const { return _namesize; }
};

#endif
