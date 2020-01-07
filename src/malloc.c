#include "malloc.h"
#include "malloc_internal.h"

__attribute__((malloc))
void *malloc(const size_t size)
{
	if(size == 0)
		return NULL;
	if(size < _SMALL_BIN_MAX)
		return small_alloc(size);
	else if(size < _MEDIUM_BIN_MAX)
		return medium_alloc(size);
	else
		return large_alloc(size);
}
