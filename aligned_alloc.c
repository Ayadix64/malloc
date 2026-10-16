#include <stdlib.h>
#include <errno.h>
#include "meta.h"

void *aligned_alloc(size_t align, size_t len)
{
	if ((align & -align) != align) {
		errno = EINVAL;
		return 0;
	}

	if (len > SIZE_MAX - align || align >= (1ULL<<31)*UNIT) {
		errno = ENOMEM;
		return 0;
	}

	if (DISABLE_ALIGNED_ALLOC) {
		errno = ENOMEM;
		return 0;
	}

	if (align <= UNIT) align = UNIT;

	unsigned char *p = malloc(len + align - UNIT);
	if (!p)
		return 0;

	struct meta *g = getmeta(p);
	if(!g)return 0; //who knows how?
	p=(unsigned char*)(((uintptr_t)p + align -1)&(~align)); // TODO: mybe a bug in her...



	return p;
}
