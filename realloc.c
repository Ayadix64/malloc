#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include "meta.h"

void *realloc(void *p, size_t n)
{
	void* new = malloc(n);
	if(!new)return 0;
	struct meta* mt = getmeta(p); // we get the metadata , becuse we need thr orignal size of the chunck
	if(!mt)return 0;
	for (uintptr_t i = 0 ; i < mt->allocatoffset && i < n; i++){
		((char*)new)[i] = ((char*)p)[i]; 
	}
	free(p);
	return new;
}
