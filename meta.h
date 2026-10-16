#ifndef MALLOC_META_H
#define MALLOC_META_H

#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include "glue.h"


#define MMAP_THRESHOLD 131052


struct meta {
	size_t magicnum; //the magic number, made on the run
	struct meta* prevchun; // privuce chunk
	
	size_t allocatoffset;
	size_t datasize; // simple and strate forward
	
	int alocatedcount;
	int    lock; // the lock mutext, one bit
	//size_t allignepadd:((sizeof(size_t)-4)*8)<0?0:((sizeof(size_t)-4)*8);
	struct meta* self;
}__attribute__((packed))  ;



struct malloc_context {

	struct meta* firstchnck;
	struct meta* lastchnck;
	struct meta* ffreemeta;

	size_t magic;
	size_t brk;
	int lock;
	int intilised;
	int deallocation_lock;
} ;



#define METASZ sizeof(struct meta)

#define UNIT METASZ

__attribute__((__visibility__("hidden")))
extern struct malloc_context ctx;

#ifdef PAGESIZE
#define PGSZ PAGESIZE
#else
#define PGSZ 0x1000
#endif

__attribute__((__visibility__("hidden")))
struct meta *alloc_meta(void);

__attribute__((__visibility__("hidden")))
int is_allzero(void *);

static struct meta* getmeta(void* p){
	if(!p || (uintptr_t )p <  METASZ || (size_t)p >= ctx.brk )return 0;
	struct meta*cnk= (struct meta*)  (*(void**) ((uintptr_t)p-sizeof(void*)) ) ;

	cnk = cnk->self;
	if(!cnk || (uintptr_t)cnk > ctx.brk)return 0;
	if (cnk->magicnum != ctx.magic){
		return 0; /*some one soing somthing sceatchy..*/
	}
	return cnk;
}

#define dbg(x) fprintf(stderr,"%s : %x\n",#x,x);


#endif
