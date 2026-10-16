#include "glue.h"
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#define _BSD_SOURCE
#include <stdlib.h>
#include <sys/mman.h>

#include "meta.h"


void free(void* p)
{
	if(!p)return;

	struct meta *mt  = *(void**)((uintptr_t)p - sizeof(void*));
	
	if(!mt)return;
	

	if (mt->alocatedcount) {
		if(mt->alocatedcount == 1 && mt == ctx.lastchnck && a_cas(&mt->lock,0,-1) )
		{
			if(a_cas(&ctx.lock , 0 , (1<<31) + 1)){	
				if(mt == ctx.lastchnck){
					for(;mt->prevchun &&  
							!mt->prevchun->alocatedcount&&
							a_cas(&mt->prevchun->lock,0,-1);)
					{
						ctx.brk = (uintptr_t)mt->prevchun;
						mt = mt->prevchun;
						ctx.lastchnck = mt;
					}
					mt = mt->prevchun;
					ctx.lastchnck = mt->prevchun;
					ctx.brk = (uintptr_t)mt->prevchun;

					syscall(SYS_brk,ctx.brk);
					ctx.lock = 0;
					return;	
				}
				ctx.lock = 0;
			}
			mt->lock = 0;
		}
		a_dec((int*)&mt->alocatedcount);

	}
	
	return;
}
