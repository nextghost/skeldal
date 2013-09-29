#ifndef SKELDAL_MEMFILE
#define SKELDAL_MEMFILE

#include <stdint.h>

typedef struct game_statefile {

	struct game_statefile *next;
	struct game_statefile *prev;
	char name[24];
	unsigned int length;
	unsigned int alloc;
	uint8_t data[1];
}TMEMFILE;

typedef TMEMFILE *PMEMFILE;

PMEMFILE openMemFile(const char *name);
PMEMFILE createMemFile(const char *name, int initialSize);
PMEMFILE reuseMemFile(const char *name, PMEMFILE *f, int reserveSize);
PMEMFILE truncateMemFile(PMEMFILE f);
PMEMFILE renameMemFile(PMEMFILE f, const char *name);
//PMEMFILE replaceMemFile(PMEMFILE old, PMEMFILE nw);
void commitMemFile(PMEMFILE f);
void writeMemFile(PMEMFILE *pf, const void *data, unsigned int count);
int readMemFile(PMEMFILE f, unsigned int *seekPos, void *data, unsigned int count);
int lengthOfMemFile(PMEMFILE f);
void deleteAllMemFiles();
char eraseMemFile(const char *name);
PMEMFILE getFirstMemFile();
__inline PMEMFILE getNextMemFile(PMEMFILE f) {return f->next;}
void closeMemFile(PMEMFILE f);

#endif