#include <stdlib.h>
#include "meta.h"

size_t malloc_usable_size(void *p)
{
	if (!p) return 0;
	struct meta *g = getmeta(p);
	if(!g)return 0;
	return g->allocatoffset - ((uintptr_t)p)-((uintptr_t)g +METASZ); // ie; the size of the allocation, or miltipule allocation of 
}
