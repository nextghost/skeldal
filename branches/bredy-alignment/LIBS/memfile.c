#include "skeldal_win.h"
#include "memfile.h"
#include <stdlib.h>
#include "MEMMAN.H"


static TMEMFILE rootStateFile = {0,0,"",0,0};


PMEMFILE openMemFile(const char *name) {
	PMEMFILE k = rootStateFile.next;
	while (k != 0) {
		if (strcmp(k->name, name) == 0) {
			SEND_LOG("(MEMFILE) Opened memory file: %s (size: %u)",name,k->length);
			return k;
		}
		k = k->next;
	}
	SEND_LOG("(MEMFILE) File not found: %s",name,0);
	return 0;
}

PMEMFILE createMemFile(const char *name, int initialSize) {
	PMEMFILE f = getmem(sizeof(TMEMFILE)+initialSize);
	f->next = 0;
	f->prev = 0;
	strncpy(f->name,name,24);
	f->name[23] = 0;
	f->length = 0;
	f->alloc = initialSize;
	return f;
}


PMEMFILE replaceMemFile(PMEMFILE old, PMEMFILE nw) {
	if (old->next != 0 || old->prev != 0)  {
		nw->next = old->next;
		nw->prev = old->prev;
		if (nw->next) nw->next->prev = nw;
		if (nw->prev) nw->prev->next = nw;
	}
	free(old);
	return nw;
}

void commitMemFile(PMEMFILE f) {
	PMEMFILE o = openMemFile(f->name);
	if (o == NULL) {
		f->next = rootStateFile.next;
		f->prev = &rootStateFile;
		rootStateFile.next = f;
		if (f->next) f->next->prev = f;
		SEND_LOG("(MEMFILE) Commited new memory file: %s (size: %u)",f->name,f->length);
	} else {
		replaceMemFile(o,f);
		SEND_LOG("(MEMFILE) Replaced memory file: %s (size: %u)",f->name,f->length);
	}
}


void writeMemFile(PMEMFILE *pf, const void *data, unsigned int count) {
	PMEMFILE f = *pf;
	unsigned int needsz = f->length+count;
	if (needsz > f->alloc) {
		PMEMFILE nstf = createMemFile(f->name,needsz+needsz * 3 / 2);
		writeMemFile(&nstf,f->data,f->length);
		f = *pf = replaceMemFile(f,nstf);				
	}
	memcpy(f->data+f->length,data,count);
	f->length+=count;
}

int readMemFile(PMEMFILE f, unsigned int *seekPos, void *data, unsigned int count) {
	if (seekPos == 0 || f == 0) return 0;
	if (*seekPos >= f->length) return 0;
	if (*seekPos + count >=f->length) count = f->length - *seekPos;
	memcpy(data,f->data+(*seekPos),count);
	*seekPos+=count;
	return count;
}

int __inline lengthOfMemFile(PMEMFILE f) {
	return f->length;
}

void deleteAllMemFiles() {
	while (rootStateFile.next) {
		PMEMFILE f= rootStateFile.next;
		rootStateFile.next = f->next;
		SEND_LOG("(MEMFILE) Erasing (all files): %s",f->name,0);
		free(f);
	}
}

char eraseMemFile( const char *name)
{

	PMEMFILE f ;
	SEND_LOG("(MEMFILE) Erasing: %s",name,0);

	f = openMemFile(name);
	if (f == 0) return 1;
	if (f->next != 0) f->next->prev = f->prev;
	if (f->prev != 0) f->prev->next = f->next;
	free(f);
	
}

PMEMFILE getFirstMemFile()
{
	return rootStateFile.next;
}

PMEMFILE truncateMemFile( PMEMFILE f )
{
	f->length = 0;
	return f;
}

PMEMFILE reuseMemFile( const char *name, PMEMFILE *f, int reserveSize)
{
	if (reserveSize > (*f)->length) {
		PMEMFILE old = *f;
		*f = createMemFile(name,reserveSize);
		replaceMemFile(old,*f);
	} else {
		strncpy((*f)->name,name,24);
		(*f)->name[23] = 0;
		(*f)->length = 0;
	}
	return (*f);
	
}

PMEMFILE renameMemFile( PMEMFILE f, const char *name )
{
	strncpy(f->name,name,24);
	f->name[23] = 0;


}

void closeMemFile( PMEMFILE f )
{
	if (f->next == 0 && f->prev == 0) free(f);
}

