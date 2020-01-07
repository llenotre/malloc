#include "malloc.h"
#include "malloc_internal.h"

__attribute__((malloc))
void *malloc(const size_t size)
{
	void *ptr;

	if(size == 0)
		return NULL;
	if(size < _SMALL_BIN_MAX)
		ptr = small_alloc(size);
	else if(size < _MEDIUM_BIN_MAX)
		ptr = medium_alloc(size);
	else
		ptr = large_alloc(size);
	if(ptr)
		bzero(ptr, size);
	return ptr;
}
