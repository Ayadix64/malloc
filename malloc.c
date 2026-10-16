#include <complex.h>
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include <limits.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>

#include "glue.h"
#include "meta.h"

#include <sys/syscall.h>
#include <unistd.h>
LOCK_OBJ_DEF;

struct malloc_context ctx = { 0 };


#define PAGESZ 0x1000

#define DATALOC(_meta) (void*)((uintptr_t)(_meta) + METASZ)
#define NXTCNK(_meta)  (struct meta*)((uintptr_t)(_meta) + _meta->datasize + METASZ)
#define ALIGN ((uintptr_t)-1 << 12)


inline struct meta*  alocatenewmeta( size_t s  ){ // if malloc didnt finde a usble meta , it calls this	
	uintptr_t curuntbrk = ctx.brk;
	curuntbrk&=ALIGN;
	const size_t rlsz = ( s + METASZ + ~ALIGN -1)&ALIGN;

	ctx.brk = syscall(SYS_brk, curuntbrk + rlsz);
	
	if(ctx.brk == curuntbrk){
		return  NULL;
	}
	
	struct meta* ret = (struct meta*)curuntbrk;
		
	*ret = (struct meta){
		.magicnum = ctx.magic,
		.prevchun = ctx.lastchnck,
		.allocatoffset = s,
		.datasize = rlsz - METASZ,
		.alocatedcount = 1,
		.lock = 0,
		.self = ret
	};
	return ret;
}



void* malloc(size_t s)
{
	if(!s)return NULL;
	if(!ctx.intilised){
		LOCK(&ctx.lock);
		if(!ctx.intilised){
			ctx.magic = get_random_secret();
			ctx.brk   = syscall(SYS_brk,0);
			
			if(ctx.brk & ALIGN != ctx.brk) //mostly is aligned, but u never know..
			{
				ctx.brk = syscall(SYS_brk,ctx.brk&ALIGN);
			}
			a_store(&ctx.intilised,-1);
		}
		UNLOCK(&ctx.lock);
	}
	

	struct meta* cmt = ctx.firstchnck;
	for(;;)
	{
		struct meta* nxtm = ctx.firstchnck;
		if(!cmt || (uintptr_t)cmt+METASZ >= ctx.brk)
		{
			LOCK(&ctx.lock);
			struct meta* nmt = alocatenewmeta(s);
			if(!nmt)
			{
				UNLOCK(&ctx.lock);
				return NULL;
			}
			if(!ctx.firstchnck)
			{
				ctx.firstchnck = nmt;
			}
			ctx.lastchnck = nmt;
			UNLOCK(&ctx.lock);
			return DATALOC(nmt);

		}else if(a_cas(&cmt->lock , 0 , -1)){
			if(!cmt->alocatedcount && cmt->datasize >= s)
			{
				a_inc(&cmt->alocatedcount);
				cmt->allocatoffset=s;
				cmt->lock = 0;
				return  DATALOC(cmt);
			}/*else if(cmt->datasize - cmt->allocatoffset >= s + sizeof(uintptr_t) && ~cmt->alocatedcount)
			{
				*(void**)((uintptr_t)cmt + cmt->allocatoffset) = cmt;
				void* ret = (void*)((uintptr_t)cmt + cmt->allocatoffset + sizeof(uintptr_t));
				a_inc(&cmt->alocatedcount);
				cmt->allocatoffset += s;
				cmt->lock=0;
				return ret;

			}*/
			nxtm = NXTCNK(cmt);
			cmt->lock = 0;
		}
		cmt = nxtm;
		
	}

}
