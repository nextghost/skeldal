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
//#define LOGFILE

#ifndef _MEMMAN_H_
#define _MEMMAN_H_

#include <cstdio>
#include <inttypes.h>

class MemoryReadStream;

class DataBlock {
public:
	DataBlock(void) {}
	virtual ~DataBlock(void) = 0;

	virtual size_t memsize(void) const = 0;
};

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

class MemoryReadStream : public SeekableReadStream, public DataBlock {
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
	size_t memsize(void) const { return _length + sizeof(*this); }
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
	WriteFile(const WriteFile &src);
	const WriteFile &operator=(const WriteFile &src);

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

class MemoryWriteStream : public WriteStream {
private:
	unsigned char *_data;
	unsigned _size, _pos;

	// Do not implement
	MemoryWriteStream(const MemoryWriteStream &src);
	const MemoryWriteStream &operator=(const MemoryWriteStream &src);
public:
	MemoryWriteStream(unsigned alloc = 32);
	~MemoryWriteStream(void);

	size_t write(const void *buf, size_t size);
	void *getData(void) const { return _data; }
	unsigned getDataLength(void) const { return _pos; }
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

	MemoryReadStream *readFile(int group, const char *name);

	int findEntry(int group, const char *name) const;
	int getOffset(unsigned id) const;

	bool isOpen(void) const { return _nametable; }
};

// FIXME: replace this with real classes for sound and stuff
struct RawData : public DataBlock {
	void *data;
	unsigned size;

	RawData(void) : DataBlock(), data(NULL), size(0) {}
	~RawData(void);

	size_t memsize(void) const { return size + sizeof(*this); }
};

DataBlock *preloadStream(SeekableReadStream &stream);

class Logger {
private:
	// Do not implement
	Logger(const Logger &src);
	const Logger &operator=(const Logger &src);

public:
	Logger(void) { }
	virtual ~Logger(void);

	virtual void write(const char *format, ...) { }
	virtual void flush(void) { }
};

class StdLogger : public Logger {
public:
	void write(const char *format, ...);
	void flush(void);
};

#define freemem(size) free(size);
//#define malloc(size) getmem(size)
#define New(typ) (typ *)getmem(sizeof(typ))
#define NewArr(typ,count) (typ *)getmem(sizeof(typ)*(count))

/*
typedef struct meminfo {
    unsigned LargestBlockAvail;
    unsigned MaxUnlockedPage;
    unsigned LargestLockablePage;
    unsigned LinAddrSpace;
    unsigned NumFreePagesAvail;
    unsigned NumPhysicalPagesFree;
    unsigned TotalPhysicalPages;
    unsigned FreeLinAddrSpace;
    unsigned SizeOfPageFile;
    unsigned Reserved[3];
} MEMINFO;
*/


typedef struct thandle_data {
	char src_file[13];  //12
	int32_t seekpos;       //16
	DataBlock *blockdata;	//20
	int8_t flags;        //21
	int8_t path;        //22
	int16_t status;
	DataBlock *(*loadproc)(SeekableReadStream &stream);	// 28
	uint16_t lockcount;               //32
	uint32_t counter;
	uint32_t size;
} THANDLE_DATA;

#define BK_MAJOR_HANDLES 256 // maximalni pocet skupin rukojeti
#define BK_MINOR_HANDLES 256 // pocet rukojeti v jedne skupine

typedef THANDLE_DATA handle_list[BK_MINOR_HANDLES];
typedef handle_list *handle_groups[BK_MAJOR_HANDLES];

//status
#define BK_NOT_USED 0
#define BK_NOT_LOADED 1
#define BK_PRESENT 2
#define BK_SWAPED 3
#define BK_DIRLIST 4 //znaci ze handle je docasne pouzito pro adresarovy list
           //pokud je status=4 je automaticky chapan jako 1.
#define BK_SAME_AS 5
//flags
#define BK_LOCKED 1
#define BK_SWAPABLE 2
#define BK_SHARED 4
#define BK_PRELOAD 8
#define BK_HSWAP 16

//extern char **mman_pathlist;  //tento pointer musi byt naplnen ukazatelem na tabulku cest
extern int memman_handle; //cislo handle naposled zpracovavaneho prikazem ablock
extern char mman_patch;    //jednicka zapina moznost pouziti patchu
void *getmem(long size);        //alokace pameti pres memman. alokovat pomoci malloc lze ale hrozi nebezpeci ze vrati NULL
void *grealloc(void *m,long size); //realokace pameti pres memman
void *load_file(char *filename); //obycejne natahne soubor do pameti a vrati ukazatel.
void init_manager(char *filename,char *swp); //inicializuje manager. Jmeno filename i swapname nejsou povinne (musi byt NULL kdyz nejsou pouzity)
THANDLE_DATA *def_handle(int handle, const char *filename, DataBlock *(*decompress)(SeekableReadStream&), char path); //deklaruje rukojet. promenna decompress je ukazatel na funkci ktera upravi data pred vracenim ukazatele
DataBlock *ablock(int handle);             //vraci ukazatel bloku spojeneho s handlem
void alock(int handle);               //zamyka blok
void aunlock(int handle);             //odmyka blok
void aswap(int handle);               //zapina swapovani pro blok
void aunswap(int handle);             //vypina swapovani pro blok
void apreload(int handle);            //zapina preloading pro blok (preloading proved pomoci ablock)
THANDLE_DATA *kill_block(int handle);
//void free();                          //free
void close_manager();                 //uzavre manager a uvolni veskerou pamet
void undef_handle(int handle);        //uvolni hadle k dalsimu pouziti
THANDLE_DATA *zneplatnit_block(int handle); //zneplatni data bloku
THANDLE_DATA *get_handle(int handle); //vraci informace o rukojeti
int find_handle(const char *name,DataBlock *(*decomp)(SeekableReadStream&));   //hleda mezi rukojeti stejnou definici
int test_file_exist(int group,const char *filename); //testuje zda soubor existuje v ramci mmanageru
SeekableReadStream *afile(const char *filename, int group); //nahraje do pameti soubor registrovany v ramci mmanageru
long get_handle_size(int handle);
//void get_mem_info(MEMORYSTATUS *mem); 

void apreload_sign(int handle,int max_handle);     //pripravi preloading pro nacteni dat z CD (sekvencne)
void apreload_start(void (*percent)(int cur,int max));   //provede sekvenci apreload

char *read_next_entry(char mode);     //cte adresar DDL souboru a vraci jmeno, nebo NULL. mode udava, zda se hleda od zacatku, nebo pokracuje tam kde program skoncil
int read_group(int index);
char add_patch_file(char *filename);  //pripojuje zaplatu
FILE *afiletemp(char *filename, int group);
void memstats(void);


#define MMA_READ 1
#define MMA_SWAP 2
#define MMA_SWAP_READ 3
#define MMA_FREE 4

#define MMR_FIRST 0
#define MMR_NEXT 1

extern void (*mman_action)(int action);  //udalost volajici se pri akci mmanagera.

void display_status();    //zobrazi na display status memmanageru
char *get_time_str();
extern Logger *syslog;
#define SEND_LOG(format, ...) syslog->write(format, __VA_ARGS__)

#endif
